#pragma once
#include <cstdint>
#include <FastNoise/FastNoise.h>
#include <FastNoise/SmartNode.h>
#include <FastNoise/Generators/Simplex.h>
#include <glm/glm.hpp>

#include "entity.h"
#include "mesh.h"

#define BLOCK_TYPE_AIR 0
#define BLOCK_TYPE_GRASS 1
#define BLOCK_TYPE_DIRT 2
#define BLOCK_TYPE_STONE 3

class Chunk : public Entity
{
	Texture2D* texture_;

	int size_;

	int seed_;

	// The blocks in the chunk (ordered z, then x, then y).
	std::vector<std::vector<std::vector<uint8_t>>> blocks_;
protected:
	void UseNoise(FastNoise::SmartNode<FastNoise::Simplex> noiseGenerator);
	void GenerateMesh();

	bool IsInChunk(int x, int y, int z);
public:
	Chunk(FastNoise::SmartNode<FastNoise::Simplex> noiseGenerator, glm::vec3 startingPosition, int size, int seed);
	void Draw(glm::mat4 view, glm::mat4 projection);
};
