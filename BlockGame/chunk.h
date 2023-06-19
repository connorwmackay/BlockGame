#pragma once
#include <atomic>
#include <cstdint>
#include <FastNoise/FastNoise.h>
#include <FastNoise/SmartNode.h>
#include <FastNoise/Generators/Simplex.h>
#include <glm/glm.hpp>

#include "entity.h"
#include "mesh.h"
#include "transformComponent.h"
#include "meshComponent.h"

#define BLOCK_TYPE_AIR 0
#define BLOCK_TYPE_GRASS 1
#define BLOCK_TYPE_DIRT 2
#define BLOCK_TYPE_STONE 3

class Chunk : public Entity
{
	Texture2D* texture_;

	std::atomic<int> size_;

	std::atomic<int> seed_;

	// The blocks in the chunk (ordered z, then x, then y).
	std::vector<std::vector<std::vector<uint8_t>>> blocks_;

	MeshComponent* meshComponent;
	TransformComponent* transformComponent;

	std::atomic<bool> isUnloaded{false};

	// Get all the subtextures once, so the calculation
	// only needs to happen once, when a chunk is created.
	std::vector<SubTexture> grassSubTextures_;
	std::vector<SubTexture> dirtSubTextures_;
	std::vector<SubTexture> stoneSubTextures_;
protected:
	void UseNoise(std::vector<float> chunkSectionNoise, int minY, int maxY);
	void GenerateMesh(bool isOnMainThread = true);

	bool IsInChunk(int x, int y, int z);
public:
	Chunk(std::vector<float> chunkSectionNoise, int minY, int maxY, glm::vec3 startingPosition, int size, int seed);

	void Draw();

	void Unload();
	void Recreate(std::vector<float> chunkSectionNoise, int minY, int maxY, glm::vec3 newStartingPosition, int seed, bool isOnMainThread = true);

	void Update() override;
};
