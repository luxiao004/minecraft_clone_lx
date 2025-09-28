#include "ChunkManager.h"
#include <iostream>
#include <algorithm>

ChunkManager::ChunkManager(int renderDistance) :renderDistance(renderDistance) {
	std::cout << "ChunkManager initialized with render distance: " << renderDistance << std::endl;
	startAsyncLoading();
}

ChunkManager::~ChunkManager() {
	stopAsyncLoading();//停止后台线程，不让它继续加载区块（否则可能在析构时还在访问 chunks）
	//清理所有区块
	std::lock_guard<std::mutex> lock(chunksMutex);//进入临界区，加锁，防止同时有别的线程访问 chunks
	for (auto& pair : chunks) {
		delete pair.second;
	}
	chunks.clear();
	std::cout << "ChunkManager destroyed" << std::endl;
}

void ChunkManager::update(const glm::vec3& cameraPosition) {
	//计算相机所在的区块坐标
	ChunkCoord cameraChunk = {
		static_cast<int>(std::floor(cameraPosition.x / Chunk::SIZE)),
		static_cast<int>(std::floor(cameraPosition.z / Chunk::SIZE))
	};

	//如果相机没有移动到新区块，就不更新
	if (cameraChunk==lastCameraChunk) {
		return;
	}
	lastCameraChunk = cameraChunk;
	std::cout << "Camera moved to chunk: (" << cameraChunk.x << ", " << cameraChunk.z << ")" << std::endl;

	// 确定需要加载的区块范围
	//???????????????????
	std::vector<ChunkCoord> chunksToLoad;
	for (int x = cameraChunk.x - renderDistance;x <= cameraChunk.x + renderDistance;x++) {
		for (int z = cameraChunk.z - renderDistance; z <= cameraChunk.z + renderDistance; z++) {
			ChunkCoord coord{ x,z };
			if (shouldLoadChunk(coord, cameraChunk)) {
				chunksToLoad.push_back(coord);
			}
		}
	}
	std::cout << "hi xjj" << std::endl;
	//异步加载区块:保证同时只有一个线程在修改 loadQueue
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		for (const auto& coord : chunksToLoad) {
			//检查是否已经加载或正在加载
			bool shouldLoad = true;
			{
				std::lock_guard<std::mutex> chunkLock(chunksMutex);//加锁，保证线程安全
				//下面这段代码只允许一个线程进入
				if (chunks.find(coord) != chunks.end()) {// 在 map 里能找到这个坐标
					shouldLoad = false;// 已经加载过了，就不需要再加载
				}
			}

			if (shouldLoad) {
				loadQueue.push(coord);
			}
		}
		if (!loadQueue.empty()) {
			queueCV.notify_one();//notify:通报，告知
		}
	}

	//同步卸载远离区块
	std::vector<ChunkCoord> chunksToUnload;
	{
		std::lock_guard<std::mutex> lock(chunksMutex);
		for (const auto& pair : chunks) {
			if (shouldUnloadChunk(pair.first, cameraChunk)) {
				chunksToUnload.push_back(pair.first);
			}
		}
	}

	for (const auto& coord : chunksToUnload) {
		unloadChunk(coord);
	}
	std::cout << "Loaded chunks: " << chunks.size() << std::endl;
}

void ChunkManager::render()const {
	std::lock_guard<std::mutex> lock(chunksMutex);
	for (const auto& pair : chunks) {
		if (pair.second->isMeshGenerated()) {
			pair.second->render();
		}
	}
	
}

//根据坐标 coord 找到对应的区块指针
Chunk* ChunkManager::getChunk(const ChunkCoord& coord)const {
	std::lock_guard<std::mutex> lock(chunksMutex);
	auto it = chunks.find(coord);
	return it != chunks.end() ? it->second : nullptr;
}

BlockType ChunkManager::getBlock(const WorldPos& worldPos)const {
	ChunkCoord chunkCoord = worldPos.toChunkCoord(Chunk::SIZE);
	glm::ivec3 localPos = worldPos.toLocalPos(Chunk::SIZE);

	Chunk* chunk = getChunk(chunkCoord);
	if (chunk) {
		return chunk->getBlock(localPos.x, localPos.y, localPos.z);
	}
	return BlockType::AIR;
}

void ChunkManager::setBlock(const WorldPos& worldPos, BlockType type) {
	ChunkCoord chunkCoord = worldPos.toChunkCoord(Chunk::SIZE);
	glm::ivec3 localPos = worldPos.toLocalPos(Chunk::SIZE);

	Chunk* chunk = getChunk(chunkCoord);
	if (chunk) {
		chunk->setBlock(localPos.x, localPos.y, localPos.z, type);
	}
}

void ChunkManager::loadChunk(const ChunkCoord& coord) {
	std::cout << "Loading chunk at: (" << coord.x << ", " << coord.z << ")" << std::endl;

	Chunk* chunk = new Chunk(coord);
	{
		std::lock_guard<std::mutex> lock(chunksMutex);
		chunks[coord] = chunk;
		std::cout << "hi zjj" << std::endl;
	}
	std::cout << "Chunk loaded at: (" << coord.x << ", " << coord.z << ")" << std::endl;
}

void ChunkManager::unloadChunk(const ChunkCoord& coord) {
	std::cout << "Unloading chunk at: (" << coord.x << ", " << coord.z << ")" << std::endl;

	std::lock_guard<std::mutex> lock(chunksMutex);
	auto it = chunks.find(coord);
	if (it != chunks.end()) {
		delete it->second;
		chunks.erase(it);
		std::cout << "Chunk unloaded at: (" << coord.x << ", " << coord.z << ")" << std::endl;
	}
}

bool ChunkManager::shouldLoadChunk(const ChunkCoord& coord, const ChunkCoord& cameraChunk)const {
	int distX = std::abs(coord.x - cameraChunk.x);
	int distZ = std::abs(coord.z - cameraChunk.z);
	return distX <= renderDistance && distZ <= renderDistance;
}

bool ChunkManager::shouldUnloadChunk(const ChunkCoord& coord, const ChunkCoord& cameraChunk) const {
	int distX = std::abs(coord.x - cameraChunk.x);
	int distZ = std::abs(coord.z - cameraChunk.z);
	// 卸载距离比加载距离稍大，避免边界频繁加载卸载
	return distX > renderDistance + 2 || distZ > renderDistance + 2;
}

void ChunkManager::startAsyncLoading() {
	if (asyncLoading && !asyncThread.joinable()) {//判断这个线程对象是否在运行
		stopAsync = false;
		//在新线程里执行 this->asyncLoadWorker()
		asyncThread = std::thread(&ChunkManager::asyncLoadWorker, this);//&ChunkManager::asyncLoadWorker 是线程函数指针;this 是传递给成员函数的对象指针，因为成员函数需要对象实例
		std::cout << "Async loading started" << std::endl;
	}
}

void ChunkManager::stopAsyncLoading() {
	if (asyncThread.joinable()) {
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			stopAsync = true;
		}
		queueCV.notify_all();
		asyncThread.join();
		std::cout << "Async loading stopped" << std::endl;
	}
}

//循环运行，直到线程停止
void ChunkManager::asyncLoadWorker() {
	while (!stopAsync) {
		ChunkCoord coord;

		{
			std::unique_lock<std::mutex> lock(queueMutex);//保护队列的锁
			queueCV.wait(lock, [this]() {
				return !loadQueue.empty() || stopAsync;
			});//线程睡眠等待，直到“队列非空”或者“线程要停止
			if (stopAsync)break;

			//从队列里拿一个任务
			coord = loadQueue.front();//拿到要加载的区块坐标 coord
			loadQueue.pop();//同时把它从队列里移除（出队）
			std::cout << loadQueue.size() << std::endl;
		}
		bool needLoad = true;
		{
			std::lock_guard<std::mutex> lock(chunksMutex);
			if (chunks.find(coord) != chunks.end()) {
				needLoad = false;
			}
		}
		if (needLoad) {
			loadChunk(coord);
		}
	}
}