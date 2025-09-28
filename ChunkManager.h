#pragma once
#include "Chunk.h"
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <glm/glm.hpp>

class ChunkManager {
public:
	ChunkManager(int renderDistance = 2);
	~ChunkManager();

	//更新加载的区块（根据相机位置）
	void update(const glm::vec3& cameraPosition);

	//渲染所有加载的区块
	void render()const;

	//获取区块
	Chunk* getChunk(const ChunkCoord& coord)const;

	//获取区块中的方块
	BlockType getBlock(const WorldPos& worldPos)const;

	//设置区块中的方块
	void setBlock(const WorldPos& worldPos, BlockType type);
private:
	//区块操作
	void loadChunk(const ChunkCoord& coord);
	void unloadChunk(const ChunkCoord& coord);
	void generateChunkMesh(Chunk* chunk);

	//异步加载
	void startAsyncLoading();
	void stopAsyncLoading();
	void asyncLoadWorker();
	// 辅助函数
	bool shouldLoadChunk(const ChunkCoord& coord, const ChunkCoord& cameraChunk) const;
	bool shouldUnloadChunk(const ChunkCoord& coord, const ChunkCoord& cameraChunk) const;

private:
	mutable std::mutex chunksMutex;//mutable 的原因：即使在 const 函数里也可以加锁。
	std::unordered_map<ChunkCoord, Chunk*, ChunkCoord::Hash> chunks;

	// 异步加载相关
	bool asyncLoading = true;//控制是否启用异步加载
	std::thread asyncThread;//后台线程对象，专门去处理加载队列 loadQueue
	std::queue<ChunkCoord> loadQueue;//待加载的区块队列，线程安全地存储哪些区块需要生成
	std::mutex queueMutex;//保护 loadQueue 的互斥锁
	std::condition_variable queueCV;//条件变量（condition_variable），用于线程等待或被通知
	bool stopAsync = false;

	int renderDistance;
	ChunkCoord lastCameraChunk{ 0, 0 };
};