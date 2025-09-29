#pragma once
#include <glm/glm.hpp>
#include <cstdint>
//基础类型定义
using BlockID = uint16_t;

//区块坐标（二维
struct ChunkCoord {
	int x, z;
	bool operator==(const ChunkCoord& other)const {
		return x == other.x && z == other.z;
	}
	//用于unordered_map的哈希函数:size_t 的设计目标是：足够大，可以存下内存里能分配的最大对象大小
	struct Hash {
		size_t operator()(const ChunkCoord& coord)const {
			return ((size_t)coord.x << 32) | (size_t)coord.z;
		}
	};
};
//世界坐标
struct WorldPos {
	int x, y, z;

	ChunkCoord toChunkCoord(int chunkSize)const {
		return { x / chunkSize,z / chunkSize };
	}
	//转换为区块内的局部坐标
	glm::ivec3 toLocalPos(int chunkSize)const {
		return{
			(x % chunkSize + chunkSize) % chunkSize,
			y,
			(z % chunkSize + chunkSize) % chunkSize
		};
	}
};
//方块类型枚举
enum class BlockType :BlockID {
	AIR=0,
	GRASS=1,
	DIRT=2,
	STONE=3,
	WOOD=4,      // 木头
	LEAVES=5,    // 树叶
	WATER=6,     // 水
	SAND=7       // 沙子
};