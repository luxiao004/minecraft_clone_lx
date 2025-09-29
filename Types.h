#pragma once
#include <glm/glm.hpp>
#include <cstdint>
//�������Ͷ���
using BlockID = uint16_t;

//�������꣨��ά
struct ChunkCoord {
	int x, z;
	bool operator==(const ChunkCoord& other)const {
		return x == other.x && z == other.z;
	}
	//����unordered_map�Ĺ�ϣ����:size_t �����Ŀ���ǣ��㹻�󣬿��Դ����ڴ����ܷ�����������С
	struct Hash {
		size_t operator()(const ChunkCoord& coord)const {
			return ((size_t)coord.x << 32) | (size_t)coord.z;
		}
	};
};
//��������
struct WorldPos {
	int x, y, z;

	ChunkCoord toChunkCoord(int chunkSize)const {
		return { x / chunkSize,z / chunkSize };
	}
	//ת��Ϊ�����ڵľֲ�����
	glm::ivec3 toLocalPos(int chunkSize)const {
		return{
			(x % chunkSize + chunkSize) % chunkSize,
			y,
			(z % chunkSize + chunkSize) % chunkSize
		};
	}
};
//��������ö��
enum class BlockType :BlockID {
	AIR=0,
	GRASS=1,
	DIRT=2,
	STONE=3,
	WOOD=4,      // ľͷ
	LEAVES=5,    // ��Ҷ
	WATER=6,     // ˮ
	SAND=7       // ɳ��
};