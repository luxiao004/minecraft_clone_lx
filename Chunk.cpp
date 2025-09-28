#include "Chunk.h"
#include <iostream>

const float TEXTURE_SIZE = 1.0f/16.0f;
std::vector<BlockTexture> blockTextures = {
	BlockTexture{0,0,0,0,0,0},   // AIR
	BlockTexture{0,15,1,15,2,15},   // GRASS
	BlockTexture{2,15,2,15,2,15},   // DIRT
	BlockTexture{3,15,3,15,3,15},   // STONE
	BlockTexture{3,14,2,14,3,14},   // WOOD
	BlockTexture{6,12,6,12,6,12},   // LEAVES
	BlockTexture{0,0,0,0,0,0},      // WATER (��ʱ͸��)
	BlockTexture{0,14,0,14,0,14}    // SAND
};
Chunk::Chunk(const ChunkCoord& coord) :coord(coord),terrainNoise(2025) {
	// ��ʼ�����з���Ϊ����
	blocks.fill(BlockType::AIR);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	
	//���ɵ���
	generateTerrain();

	//��������
	generateMesh();
}

Chunk::~Chunk() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
void Chunk::generateTerrain() {
	const int BASE_HEIGHT = 64;   // ƽ������߶�
	const int MAX_VARIATION = 5;  // �߶ȱ仯��Χ

	std::cout << "Generating procedural terrain for chunk at ("
		<< coord.x << ", " << coord.z << ")" << std::endl;

	int blocksPlaced = 0;

	for (int x = 0; x < SIZE; x++) {
		for (int z = 0; z < SIZE; z++) {
			int worldX = coord.x * SIZE + x;
			int worldZ = coord.z * SIZE + z;

			// �������ɸ߶ȣ���Χ��Լ�� BASE_HEIGHT �� MAX_VARIATION
			float n = terrainNoise.octaveNoise(worldX * 0.05f, 0.0f, worldZ * 0.05f, 2, 0.5f);
			n = (n + 1.0f) / 2.0f;  // ת���� [0,1]
			int terrainHeight = BASE_HEIGHT + (int)(n * MAX_VARIATION);

			for (int y = 0; y < HEIGHT; y++) {
				BlockType blockType = BlockType::AIR;

				if (y <= 2) {
					blockType = BlockType::STONE; // ����
				}
				else if (y < terrainHeight) {
					blockType = BlockType::DIRT;  // ����
				}
				else if (y == terrainHeight) {
					blockType = BlockType::GRASS; // �����
				}

				setBlock(x, y, z, blockType);
				blocksPlaced++;
			}
		}
	}

	std::cout << "Placed " << blocksPlaced << " blocks in chunk at ("
		<< coord.x << ", " << coord.z << ")" << std::endl;

	markDirty();
}



/*void Chunk::generateTerrain() {
	std::cout << "Generating TEST terrain for chunk at ("
		<< coord.x << ", " << coord.z << ")" << std::endl;

	int blocksPlaced = 0;

	for (int x = 0; x < SIZE; x++) {
		for (int z = 0; z < SIZE; z++) {
			for (int y = 0; y < HEIGHT; y++) {
				BlockType blockType = BlockType::AIR; // Ĭ�Ͽ���

				if (y == 0) {
					blockType = BlockType::STONE; // ��׻���
				}
				else if (y > 0 && y < 50) {
					blockType = BlockType::STONE; // ����ʯͷ
				}
				else if (y >= 50 && y < 63) {
					blockType = BlockType::DIRT; // �ر��¼�������
				}
				else if (y == 63) {
					blockType = BlockType::GRASS; // �����
				}
				// y>63Ĭ�Ͼ���AIR

				setBlock(x, y, z, blockType);
				blocksPlaced++;
			}
		}
	}

	std::cout << "Placed " << blocksPlaced << " blocks in chunk at ("
		<< coord.x << ", " << coord.z << ")" << std::endl;

	markDirty();
}*/


/*void Chunk::generateTerrain() {
	const int WATER_LEVEL = 62;// ��ƽ��߶�
	const int BASE_HEIGHT = 68;// �����߶�

	std::cout << "Generating terrain for chunk at (" << coord.x << ", " << coord.z << ")" << std::endl;

	int blocksPlaced = 0;

	for (int x = 0; x < SIZE; x++) {
		for (int z = 0; z < SIZE; z++) {
			// ������������
			int worldX = coord.x * SIZE + x;
			int worldZ = coord.z * SIZE + z;

			// ʹ��Perlin�������ɸ߶�
			// ʹ�ò�ͬƵ�ʵ�������ϴ�������Ȼ�ĵ���
			float continentNoise = (terrainNoise.octaveNoise(
				worldX * 0.001f, 0.0f, worldZ * 0.001f, 4, 0.5f) + 1.0f) / 2.0f;
			

			float mountainNoise = (terrainNoise.octaveNoise(
				worldX * 0.005f, 0.0f, worldZ * 0.005f, 6, 0.6f) + 1.0f) / 2.0f;

			float detailNoise = (terrainNoise.octaveNoise(
				worldX * 0.02f, 0.0f, worldZ * 0.02f, 2, 0.3f) + 1.0f) / 2.0f;
		

			// �������ֵ
			float combinedNoise = continentNoise * 0.6f + mountainNoise * 0.3f + detailNoise * 0.1f;

			// ������ֵת��Ϊ�߶� (40-120�ķ�Χ)
			int terrainHeight = BASE_HEIGHT + (int)(combinedNoise * 40.0f);
			terrainHeight = std::max(0, std::min(HEIGHT - 1, terrainHeight));

			// ʹ�ö����������������Ⱥϵ
			float biomeNoise = (terrainNoise.noise(worldX * 0.01f, 0.0f, worldZ * 0.01f)+1.0f)/2.0f;

			// ���ɵ��β�
			for (int y = 0; y <= terrainHeight; y++) {
				BlockType blockType = BlockType::STONE;  // Ĭ����ʯͷ

				if (y == terrainHeight) {
					// ����
					if (terrainHeight < WATER_LEVEL - 2) {
						blockType = BlockType::SAND;  // ˮ��ɳ̲
					}
					else if (biomeNoise > 0.3f) {
						blockType = BlockType::GRASS; // �ݵ�
					}
					else {
						blockType = BlockType::SAND;  // ɳĮ
					}
				}
				else if (y > terrainHeight - 4) {
					// ���漸��
					if (terrainHeight < WATER_LEVEL - 2) {
						blockType = BlockType::SAND;
					}
					else if (biomeNoise > 0.3f) {
						blockType = BlockType::DIRT;
					}
					else {
						blockType = BlockType::SAND;
					}
				}
				else if (y < 20) {
					// ����ǻ���
					blockType = BlockType::STONE;
				}

				setBlock(x, y, z, blockType);
				blocksPlaced++;
			}

			// ���ɶ�Ѩ
			for (int y = 20; y < terrainHeight - 5; y++) {
				float caveNoise = terrainNoise.octaveNoise(
					worldX * 0.05f, y * 0.1f, worldZ * 0.05f, 3, 0.5f
				);

				// �������ֵ���ض���Χ�ڣ�������Ѩ��������
				if (std::abs(caveNoise) < 0.2f) {
					setBlock(x, y, z, BlockType::AIR);
				}
			}
		}
	}

	std::cout << "Placed " << blocksPlaced << " blocks in chunk at ("
		<< coord.x << ", " << coord.z << ")" << std::endl;

	// �����Ҫ������������
	markDirty();
}*/
BlockType Chunk::getBlock(int x, int y, int z)const {
	if (x < 0 || x >= SIZE || y < 0 || y >= HEIGHT || z < 0 || z >= SIZE) {
		return BlockType::AIR;
	}
	return blocks[x * HEIGHT*SIZE + y * SIZE + z];
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
	if (x < 0 || x >= SIZE || y < 0 || y >= HEIGHT || z < 0 || z >= SIZE) {
		return;
	}
	blocks[x * HEIGHT * SIZE + y * SIZE + z] = type;
	empty = (type == BlockType::AIR) && empty;
	markDirty();
}

void Chunk::addFace(int x, int y, int z,FaceDir dir, BlockType block) {
	const BlockTexture& tex = blockTextures[(int)block];

	int texX, texY;
	if (dir == UP) { texX = tex.topX; texY = tex.topY; }     // ��
	else if (dir == DOWN) { texX = tex.bottomX; texY = tex.bottomY; } // ��
	else { texX = tex.sideX; texY = tex.sideY; }            // ����

	float u0 = texX * TEXTURE_SIZE;
	float v0 = texY * TEXTURE_SIZE;
	float u1 = (texX + 1) * TEXTURE_SIZE;
	float v1 = (texY + 1) * TEXTURE_SIZE;

	glm::vec3 v0p, v1p, v2p, v3p;

	switch (dir) {
	case EAST: // +X
		v0p = { x + 1, y,   z +1};
		v1p = { x + 1, y,   z };
		v2p = { x + 1, y + 1, z  };
		v3p = { x + 1, y + 1, z +1};
		break;
	case WEST: // -X
		v0p = { x, y,   z };
		v1p = { x, y,   z+1};
		v2p = { x, y + 1, z+1 };
		v3p = { x, y + 1, z  };
		break;
	case UP: // +Y
		v0p = { x,   y + 1, z+1 };
		v1p = { x + 1, y + 1, z+1 };
		v2p = { x + 1, y + 1, z };
		v3p = { x,   y + 1, z  };
		break;
	case DOWN: // -Y
		v0p = { x,   y, z  };
		v1p = { x + 1, y, z };
		v2p = { x + 1, y, z+1};
		v3p = { x,   y, z+1 };
		break;
	case NORTH: // -Z
		v0p = { x + 1, y,   z };
		v1p = { x,   y,   z };
		v2p = { x,   y + 1, z };
		v3p = { x + 1, y + 1, z };
		break;
	case SOUTH: // +Z
		v0p = { x,   y,   z + 1 };
		v1p = { x + 1, y,   z + 1 };
		v2p = { x + 1, y + 1, z + 1 };
		v3p = { x,   y + 1, z + 1 };
		break;
	}

	// �������� (ע�� v ��Ҫ��Ҫ��ת��ȡ���ڼ�������ķ�ʽ)
	glm::vec2 t0 = { u0, v0 };
	glm::vec2 t1 = { u1, v0 };
	glm::vec2 t2 = { u1, v1 };
	glm::vec2 t3 = { u0, v1 };

	int startIndex = vertices.size();

	vertices.push_back({ v0p, t0 });
	vertices.push_back({ v1p, t1 });
	vertices.push_back({ v2p, t2 });
	vertices.push_back({ v3p, t3 });

	indices.push_back(startIndex + 0);
	indices.push_back(startIndex + 1);
	indices.push_back(startIndex + 2);
	indices.push_back(startIndex + 2);
	indices.push_back(startIndex + 3);
	indices.push_back(startIndex + 0);
}
void Chunk::generateMesh() {
	vertices.clear();
	indices.clear();

	if (empty) {
		meshGenerated = true;
		return;
	}
	int totalFaces = 0;
	//�򵥵����޳���ֻ���ɿɼ�����
	for (int x = 0; x < SIZE; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			for (int z = 0; z < SIZE; z++) {
				BlockType block = getBlock(x, y, z);
				if (block == BlockType::AIR)continue;

				//���6������
				const int directions[6][3] = {
					{1, 0, 0}, {-1, 0, 0},  // ������
					{0, 1, 0}, {0, -1, 0},  // �ϡ���  
					{0, 0, -1}, {0, 0, 1}   // ������
				};
				for (int i = 0; i < 6; i++) {
					if (shouldGenerateFace(x, y, z, directions[i][0], directions[i][1], directions[i][2])) {
						addFace(x, y, z, (FaceDir)i, block);
						totalFaces++;
					}
					
				}
			}
		}
	}
	std::cout << "Generated " << totalFaces << " faces for chunk at ("
		<< coord.x << ", " << coord.z << ")" << std::endl;
	std::cout << "Vertices: " << vertices.size() << ", Indices: " << indices.size() << std::endl;
	setupBuffers();
	meshGenerated = true;
}

bool Chunk::shouldGenerateFace(int x, int y, int z, int dx, int dy, int dz)const {
	int nx = x + dx, ny = y + dy, nz = z + dz;

	//�������λ����������
	if (nx >= 0 && nx < SIZE && ny >= 0 && ny < HEIGHT && nz >= 0 && nz < SIZE) {
		return getBlock(nx, ny, nz) == BlockType::AIR;
	}
	// ����λ�����������飬��Ҫ��ѯ���������
	// ����򻯴�����Ϊ��Ҫ������
	return true;
}

void Chunk::setupBuffers() {
	// ����OpenGL������
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	// �������ԣ�λ�� (vec3)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// �������ԣ��������� (vec2)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void Chunk::render()const {
	if(meshGenerated==false||empty)return;

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	//std::cout << "render!!!" << std::endl;
}