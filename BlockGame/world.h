#pragma once
#include <future>
#include <glm/vec3.hpp>

#include "chunk.h"
#include "entity.h"
#include "worker.h"

struct ChunkNoiseSection
{
	glm::vec3 position; // Y should always be zero here
	std::vector<float> noise;
};

class World
{
	glm::vec3 lastKnownPlayerPos_;
	std::vector<Chunk*> chunks_;

	FastNoise::SmartNode<FastNoise::FractalFBm> terrainNoise_;
	FastNoise::SmartNode<FastNoise::FractalRidged> heightScaleNoise_;
	FastNoise::SmartNode<FastNoise::FractalFBm> temperatureNoise_;
	FastNoise::SmartNode<FastNoise::FractalRidged> insetNoise_;

	int seed_;
	int renderDistance_;
	WorldWorker* worldWorker_;
	std::mutex loadingChunksMutex_;
	std::mutex generatingNoiseMutex_;
	int yMin = -1; // num. chunks
	int yMax = 4; // num. chunks (i.e. max - min would be the number of chunks high)
protected:
	static Biome GetBiomeFromTemperature(float temperature);
public:
	World(glm::vec3 currentPlayerPos, int renderDistance);

	void Update(glm::vec3 currentPlayerPos);
	std::vector<Chunk*> GetWorld();

	bool LoadNewChunksAsync(int startX, int endX, int startZ, int endZ, std::vector<glm::vec3> loadedChunkPositions, std::vector<Chunk*> chunkIndexes);

	// Finds the closest position that's a multiple of the passed
	// parameter, i.e. closest x pos for a multiple of 16
	static int FindClosestPosition(int val, int multiple);

	std::vector<float> GetNoiseForChunkSection(int x, int z, int size);
};
