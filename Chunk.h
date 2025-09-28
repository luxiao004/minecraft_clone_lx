#pragma once
#include "Types.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include "noise.h"

struct Vertex {
	glm::vec3 position;
	glm::vec2 texCoord;
};
enum FaceDir {
	EAST = 0,   // +X
	WEST = 1,   // -X
	UP = 2,   // +Y
	DOWN = 3,   // -Y
	NORTH = 4,   // -Z
	SOUTH = 5  //+z
};

struct BlockTexture {
	int topX, topY;
	int sideX, sideY;
	int bottomX, bottomY;

	BlockTexture(int a, int b, int c, int d, int e, int f)
		: topX(a), topY(b), sideX(c), sideY(d), bottomX(e), bottomY(f) {
	}
};
extern std::vector<BlockTexture> blockTextures;



class Chunk{
public:
	//constexpr 表示这个值 在编译期就能确定
	static constexpr int SIZE = 16;
	static constexpr int HEIGHT = 256;

	Chunk(const ChunkCoord& coord);
	~Chunk();

	//方块操作
	BlockType getBlock(int x, int y, int z)const;
	void setBlock(int x, int y, int z, BlockType type);

	//地形生成
	void generateTerrain();

	//网格生成
	void generateMesh();
	void render()const;
	void addFace(int x, int y, int z, FaceDir dir, BlockType block);
	
	//状态查询
	bool isMeshGenerated()const { return meshGenerated; }
	bool isEmpty()const { return empty; }
	ChunkCoord getCoord()const { return coord; }

	//标记为需要重新生成网格
	void markDirty() { meshGenerated = false; }

private:
	void setupBuffers();
	bool shouldGenerateFace(int x, int y, int z, int dx, int dy, int dz)const;

	ChunkCoord coord;
	std::array<BlockType, SIZE* HEIGHT* SIZE> blocks;
	
	//渲染数据
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	GLuint VAO, VBO, EBO;
	bool meshGenerated = false;
	bool empty=true;

	//噪声生成器
	PerlinNoise terrainNoise;
};

