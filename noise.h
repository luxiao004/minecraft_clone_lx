#pragma once
#include <glm/glm.hpp>
#include <cmath>
#include <array> 

using glm::vec3;

class PerlinNoise {
private:
    std::array<int, 512> p; // 置换表，重复两次方便索引

	static float fade(float t) {
		return t * t * t * (t * (t * 6 - 15) + 10);
	}
	static float lerp(float t, float a, float b) {
		return a + t * (b - a);
	}
	static float grad(int hash, float x, float y, float z) {
		int h = hash & 15;
		float u = h < 8 ? x : y;
		float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}
public:
    // 构造函数，初始化置换表
    PerlinNoise(unsigned int seed = 2025) {
        std::array<int, 256> permutation;
        for (int i = 0; i < 256; ++i)
            permutation[i] = i;

        // 简单随机打乱
        std::srand(seed);
        for (int i = 255; i > 0; --i) {
            int j = std::rand() % (i + 1);
            std::swap(permutation[i], permutation[j]);
        }

        for (int i = 0; i < 256; ++i) {
            p[i] = permutation[i];
            p[i + 256] = permutation[i]; // 重复一遍
        }
    }

    // 3D Perlin 噪声
    float noise(float x, float y, float z) const {
        int X = (int)std::floor(x) & 255;
        int Y = (int)std::floor(y) & 255;
        int Z = (int)std::floor(z) & 255;

        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);

        float u = fade(x);
        float v = fade(y);
        float w = fade(z);

        int A = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        return lerp(w,
            lerp(v,
                lerp(u, grad(p[AA], x, y, z),
                    grad(p[BA], x - 1, y, z)),
                lerp(u, grad(p[AB], x, y - 1, z),
                    grad(p[BB], x - 1, y - 1, z))),
            lerp(v,
                lerp(u, grad(p[AA + 1], x, y, z - 1),
                    grad(p[BA + 1], x - 1, y, z - 1)),
                lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                    grad(p[BB + 1], x - 1, y - 1, z - 1))));
    }

    // 分层噪声
    float octaveNoise(float x, float y, float z, int octaves, float persistence) const {
        float total = 0;
        float frequency = 1;
        float amplitude = 1;
        float maxValue = 0;

        for (int i = 0; i < octaves; i++) {
            total += noise(x * frequency, y * frequency, z * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= persistence;
            frequency *= 2;
        }

        return total / maxValue;
    }
};

