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
	BlockTexture{0,0,0,0,0,0},      // WATER (暂时透明)
	BlockTexture{0,14,0,14,0,14}    // SAND
};
Chunk::Chunk(const ChunkCoord& coord) :coord(coord),terrainNoise(2025) {
	// 初始化所有方块为空气
	blocks.fill(BlockType::AIR);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	
	//生成地形
	generateTerrain();

	//生成网格
	generateMesh();
}

Chunk::~Chunk() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
void Chunk::generateTerrain() {
	const int BASE_HEIGHT = 64;   // 平均地面高度
	const int MAX_VARIATION = 5;  // 高度变化范围

	std::cout << "Generating procedural terrain for chunk at ("
		<< coord.x << ", " << coord.z << ")" << std::endl;

	int blocksPlaced = 0;

	for (int x = 0; x < SIZE; x++) {
		for (int z = 0; z < SIZE; z++) {
			int worldX = coord.x * SIZE + x;
			int worldZ = coord.z * SIZE + z;

			// 噪声生成高度，范围大约在 BASE_HEIGHT ± MAX_VARIATION
			float n = terrainNoise.octaveNoise(worldX * 0.05f, 0.0f, worldZ * 0.05f, 2, 0.5f);
			n = (n + 1.0f) / 2.0f;  // 转换到 [0,1]
			int terrainHeight = BASE_HEIGHT + (int)(n * MAX_VARIATION);

			for (int y = 0; y < HEIGHT; y++) {
				BlockType blockType = BlockType::AIR;

				if (y <= 2) {
					blockType = BlockType::STONE; // 基岩
				}
				else if (y < terrainHeight) {
					blockType = BlockType::DIRT;  // 泥土
				}
				else if (y == terrainHeight) {
					blockType = BlockType::GRASS; // 顶层草
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
				BlockType blockType = BlockType::AIR; // 默认空气

				if (y == 0) {
					blockType = BlockType::STONE; // 最底基岩
				}
				else if (y > 0 && y < 50) {
					blockType = BlockType::STONE; // 地下石头
				}
				else if (y >= 50 && y < 63) {
					blockType = BlockType::DIRT; // 地表下几层泥土
				}
				else if (y == 63) {
					blockType = BlockType::GRASS; // 顶层草
				}
				// y>63默认就是AIR

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
	const int WATER_LEVEL = 62;// 海平面高度
	const int BASE_HEIGHT = 68;// 基础高度

	std::cout << "Generating terrain for chunk at (" << coord.x << ", " << coord.z << ")" << std::endl;

	int blocksPlaced = 0;

	for (int x = 0; x < SIZE; x++) {
		for (int z = 0; z < SIZE; z++) {
			// 计算世界坐标
			int worldX = coord.x * SIZE + x;
			int worldZ = coord.z * SIZE + z;

			// 使用Perlin噪声生成高度
			// 使用不同频率的噪声组合创建更自然的地形
			float continentNoise = (terrainNoise.octaveNoise(
				worldX * 0.001f, 0.0f, worldZ * 0.001f, 4, 0.5f) + 1.0f) / 2.0f;
			

			float mountainNoise = (terrainNoise.octaveNoise(
				worldX * 0.005f, 0.0f, worldZ * 0.005f, 6, 0.6f) + 1.0f) / 2.0f;

			float detailNoise = (terrainNoise.octaveNoise(
				worldX * 0.02f, 0.0f, worldZ * 0.02f, 2, 0.3f) + 1.0f) / 2.0f;
		

			// 组合噪声值
			float combinedNoise = continentNoise * 0.6f + mountainNoise * 0.3f + detailNoise * 0.1f;

			// 将噪声值转换为高度 (40-120的范围)
			int terrainHeight = BASE_HEIGHT + (int)(combinedNoise * 40.0f);
			terrainHeight = std::max(0, std::min(HEIGHT - 1, terrainHeight));

			// 使用额外的噪声决定生物群系
			float biomeNoise = (terrainNoise.noise(worldX * 0.01f, 0.0f, worldZ * 0.01f)+1.0f)/2.0f;

			// 生成地形层
			for (int y = 0; y <= terrainHeight; y++) {
				BlockType blockType = BlockType::STONE;  // 默认是石头

				if (y == terrainHeight) {
					// 顶层
					if (terrainHeight < WATER_LEVEL - 2) {
						blockType = BlockType::SAND;  // 水下沙滩
					}
					else if (biomeNoise > 0.3f) {
						blockType = BlockType::GRASS; // 草地
					}
					else {
						blockType = BlockType::SAND;  // 沙漠
					}
				}
				else if (y > terrainHeight - 4) {
					// 上面几层
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
					// 深层是基岩
					blockType = BlockType::STONE;
				}

				setBlock(x, y, z, blockType);
				blocksPlaced++;
			}

			// 生成洞穴
			for (int y = 20; y < terrainHeight - 5; y++) {
				float caveNoise = terrainNoise.octaveNoise(
					worldX * 0.05f, y * 0.1f, worldZ * 0.05f, 3, 0.5f
				);

				// 如果噪声值在特定范围内，创建洞穴（空气）
				if (std::abs(caveNoise) < 0.2f) {
					setBlock(x, y, z, BlockType::AIR);
				}
			}
		}
	}

	std::cout << "Placed " << blocksPlaced << " blocks in chunk at ("
		<< coord.x << ", " << coord.z << ")" << std::endl;

	// 标记需要重新生成网格
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
	if (dir == UP) { texX = tex.topX; texY = tex.topY; }     // 上
	else if (dir == DOWN) { texX = tex.bottomX; texY = tex.bottomY; } // 下
	else { texX = tex.sideX; texY = tex.sideY; }            // 侧面

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

	// 纹理坐标 (注意 v 轴要不要翻转，取决于加载纹理的方式)
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
	//简单的面剔除：只生成可见的面
	for (int x = 0; x < SIZE; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			for (int z = 0; z < SIZE; z++) {
				BlockType block = getBlock(x, y, z);
				if (block == BlockType::AIR)continue;

				//检查6个方向
				const int directions[6][3] = {
					{1, 0, 0}, {-1, 0, 0},  // 东、西
					{0, 1, 0}, {0, -1, 0},  // 上、下  
					{0, 0, -1}, {0, 0, 1}   // 北、南
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

	//如果相邻位置在区块内
	if (nx >= 0 && nx < SIZE && ny >= 0 && ny < HEIGHT && nz >= 0 && nz < SIZE) {
		return getBlock(nx, ny, nz) == BlockType::AIR;
	}
	// 相邻位置在其他区块，需要查询世界管理器
	// 这里简化处理，视为需要生成面
	return true;
}

void Chunk::setupBuffers() {
	// 设置OpenGL缓冲区
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

	// 顶点属性：位置 (vec3)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// 顶点属性：纹理坐标 (vec2)
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