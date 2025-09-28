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
	//constexpr ��ʾ���ֵ �ڱ����ھ���ȷ��
	static constexpr int SIZE = 16;
	static constexpr int HEIGHT = 256;

	Chunk(const ChunkCoord& coord);
	~Chunk();

	//�������
	BlockType getBlock(int x, int y, int z)const;
	void setBlock(int x, int y, int z, BlockType type);

	//��������
	void generateTerrain();

	//��������
	void generateMesh();
	void render()const;
	void addFace(int x, int y, int z, FaceDir dir, BlockType block);
	
	//״̬��ѯ
	bool isMeshGenerated()const { return meshGenerated; }
	bool isEmpty()const { return empty; }
	ChunkCoord getCoord()const { return coord; }

	//���Ϊ��Ҫ������������
	void markDirty() { meshGenerated = false; }

private:
	void setupBuffers();
	bool shouldGenerateFace(int x, int y, int z, int dx, int dy, int dz)const;

	ChunkCoord coord;
	std::array<BlockType, SIZE* HEIGHT* SIZE> blocks;
	
	//��Ⱦ����
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	GLuint VAO, VBO, EBO;
	bool meshGenerated = false;
	bool empty=true;

	//����������
	PerlinNoise terrainNoise;
};

