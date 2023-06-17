#pragma once
#include <future>
#include <glm/vec3.hpp>

#include "chunk.h"
#include "entity.h"

class World
{
	glm::vec3 lastKnownPlayerPos_;
	std::vector<Chunk*> chunks_;
	FastNoise::SmartNode<FastNoise::Simplex> simplexNoise_;
	int seed_;
	int renderDistance_;
	std::future<bool> loadingChunks_;
	std::mutex loadingChunksMutex_;
public:
	World(glm::vec3 currentPlayerPos, int renderDistance);

	void Update(glm::vec3 currentPlayerPos);
	std::vector<Chunk*> GetWorld();

	bool LoadNewChunksAsync(std::vector<glm::vec3> positions, std::vector<Chunk*> chunkIndexes);

	// Finds the closest position that's a multiple of the passed
	// parameter, i.e. closest x pos for a multiple of 16
	static int FindClosestPosition(int val, int multiple);
};
