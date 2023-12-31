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

#include <random>

#define BLOCK_TYPE_AIR 0
#define BLOCK_TYPE_GRASS 1
#define BLOCK_TYPE_DIRT 2
#define BLOCK_TYPE_STONE 3
#define BLOCK_TYPE_SAND 4
#define BLOCK_TYPE_SNOW 5
#define BLOCK_TYPE_FORESTGRASS 6
#define BLOCK_TYPE_TREEBARK 7
#define BLOCK_TYPE_TREELEAVES 8

class World;

enum class Biome
{
	Snow,
	Grassland,
	Desert,
	Rock,
	Forest
};

class Chunk : public Entity
{
	Texture2DArray texture_;

	std::atomic<int> size_;

	std::atomic<int> seed_;

	// The blocks in the chunk (ordered z, then x, then y).
	std::vector<std::vector<std::vector<uint8_t>>> blocks_;

	MeshComponent* meshComponent;
	TransformComponent* transformComponent;

	std::atomic<bool> isUnloaded{false};


	Biome biome_;

	std::vector<glm::vec3> treeTrunkPositions_;
	std::vector<glm::vec3> treeLeavePositions_;

	World* world_;

	bool shouldDraw_;
protected:
	bool IsInChunk(int x, int y, int z);
	
public:
	Chunk(World* world, Biome biome, Texture2DArray texture, std::vector<float> chunkSectionNoise, int minY, int maxY, glm::vec3 startingPosition, int size, int seed);

	void Draw();

	void UseNoise(std::vector<float> chunkSectionNoise, int minY, int maxY);

	void GenerateMesh(bool isOnMainThread = true);

	// Returns true if any blocks were updated
	bool UpdateBlocks();

	void Unload();
	void Recreate(Biome biome, std::vector<float> chunkSectionNoise, int minY, int maxY, glm::vec3 newStartingPosition, int seed, bool isOnMainThread = true);

	void Update() override;

	bool IsUnloaded();
	Biome GetBiome();

	void AddTreeLeavePositions();
	void AddTreeTrunkPositions();

	void SetShouldDraw(bool shouldDraw);
	bool GetShouldDraw();

	TransformComponent* GetTransformComponent();
};
