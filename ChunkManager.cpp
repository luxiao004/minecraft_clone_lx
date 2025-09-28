#include "ChunkManager.h"
#include <iostream>
#include <algorithm>

ChunkManager::ChunkManager(int renderDistance) :renderDistance(renderDistance) {
	std::cout << "ChunkManager initialized with render distance: " << renderDistance << std::endl;
	startAsyncLoading();
}

ChunkManager::~ChunkManager() {
	stopAsyncLoading();//ֹͣ��̨�̣߳������������������飨�������������ʱ���ڷ��� chunks��
	//������������
	std::lock_guard<std::mutex> lock(chunksMutex);//�����ٽ�������������ֹͬʱ�б���̷߳��� chunks
	for (auto& pair : chunks) {
		delete pair.second;
	}
	chunks.clear();
	std::cout << "ChunkManager destroyed" << std::endl;
}

void ChunkManager::update(const glm::vec3& cameraPosition) {
	//����������ڵ���������
	ChunkCoord cameraChunk = {
		static_cast<int>(std::floor(cameraPosition.x / Chunk::SIZE)),
		static_cast<int>(std::floor(cameraPosition.z / Chunk::SIZE))
	};

	//������û���ƶ��������飬�Ͳ�����
	if (cameraChunk==lastCameraChunk) {
		return;
	}
	lastCameraChunk = cameraChunk;
	std::cout << "Camera moved to chunk: (" << cameraChunk.x << ", " << cameraChunk.z << ")" << std::endl;

	// ȷ����Ҫ���ص����鷶Χ
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
	//�첽��������:��֤ͬʱֻ��һ���߳����޸� loadQueue
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		for (const auto& coord : chunksToLoad) {
			//����Ƿ��Ѿ����ػ����ڼ���
			bool shouldLoad = true;
			{
				std::lock_guard<std::mutex> chunkLock(chunksMutex);//��������֤�̰߳�ȫ
				//������δ���ֻ����һ���߳̽���
				if (chunks.find(coord) != chunks.end()) {// �� map �����ҵ��������
					shouldLoad = false;// �Ѿ����ع��ˣ��Ͳ���Ҫ�ټ���
				}
			}

			if (shouldLoad) {
				loadQueue.push(coord);
			}
		}
		if (!loadQueue.empty()) {
			queueCV.notify_one();//notify:ͨ������֪
		}
	}

	//ͬ��ж��Զ������
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

//�������� coord �ҵ���Ӧ������ָ��
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
	// ж�ؾ���ȼ��ؾ����Դ󣬱���߽�Ƶ������ж��
	return distX > renderDistance + 2 || distZ > renderDistance + 2;
}

void ChunkManager::startAsyncLoading() {
	if (asyncLoading && !asyncThread.joinable()) {//�ж�����̶߳����Ƿ�������
		stopAsync = false;
		//�����߳���ִ�� this->asyncLoadWorker()
		asyncThread = std::thread(&ChunkManager::asyncLoadWorker, this);//&ChunkManager::asyncLoadWorker ���̺߳���ָ��;this �Ǵ��ݸ���Ա�����Ķ���ָ�룬��Ϊ��Ա������Ҫ����ʵ��
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

//ѭ�����У�ֱ���߳�ֹͣ
void ChunkManager::asyncLoadWorker() {
	while (!stopAsync) {
		ChunkCoord coord;

		{
			std::unique_lock<std::mutex> lock(queueMutex);//�������е���
			queueCV.wait(lock, [this]() {
				return !loadQueue.empty() || stopAsync;
			});//�߳�˯�ߵȴ���ֱ�������зǿա����ߡ��߳�Ҫֹͣ
			if (stopAsync)break;

			//�Ӷ�������һ������
			coord = loadQueue.front();//�õ�Ҫ���ص��������� coord
			loadQueue.pop();//ͬʱ�����Ӷ������Ƴ������ӣ�
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