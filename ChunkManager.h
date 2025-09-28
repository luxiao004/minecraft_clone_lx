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

	//���¼��ص����飨�������λ�ã�
	void update(const glm::vec3& cameraPosition);

	//��Ⱦ���м��ص�����
	void render()const;

	//��ȡ����
	Chunk* getChunk(const ChunkCoord& coord)const;

	//��ȡ�����еķ���
	BlockType getBlock(const WorldPos& worldPos)const;

	//���������еķ���
	void setBlock(const WorldPos& worldPos, BlockType type);
private:
	//�������
	void loadChunk(const ChunkCoord& coord);
	void unloadChunk(const ChunkCoord& coord);
	void generateChunkMesh(Chunk* chunk);

	//�첽����
	void startAsyncLoading();
	void stopAsyncLoading();
	void asyncLoadWorker();
	// ��������
	bool shouldLoadChunk(const ChunkCoord& coord, const ChunkCoord& cameraChunk) const;
	bool shouldUnloadChunk(const ChunkCoord& coord, const ChunkCoord& cameraChunk) const;

private:
	mutable std::mutex chunksMutex;//mutable ��ԭ�򣺼�ʹ�� const ������Ҳ���Լ�����
	std::unordered_map<ChunkCoord, Chunk*, ChunkCoord::Hash> chunks;

	// �첽�������
	bool asyncLoading = true;//�����Ƿ������첽����
	std::thread asyncThread;//��̨�̶߳���ר��ȥ������ض��� loadQueue
	std::queue<ChunkCoord> loadQueue;//�����ص�������У��̰߳�ȫ�ش洢��Щ������Ҫ����
	std::mutex queueMutex;//���� loadQueue �Ļ�����
	std::condition_variable queueCV;//����������condition_variable���������̵߳ȴ���֪ͨ
	bool stopAsync = false;

	int renderDistance;
	ChunkCoord lastCameraChunk{ 0, 0 };
};