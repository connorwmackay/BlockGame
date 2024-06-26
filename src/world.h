#pragma once
#include <future>
#include <glm/vec3.hpp>
#include <FastNoise/FastNoise.h>

#include "chunk.h"
#include "entity.h"
#include "worker.h"

#include "frustum.h"
#include "terrain.h"

struct ChunkNoiseSection
{
	glm::vec3 position; // Y should always be zero here
	std::vector<float> noise;
};

class World
{
	glm::vec3 lastKnownPlayerPos_;
	std::vector<Chunk*> chunks_;

    Terrain terrain_;

	FastNoise::SmartNode<FastNoise::FractalFBm> temperatureNoise_;

	int seed_;
	int renderDistance_;
	WorldWorker* worldWorker_;
	std::mutex loadingChunksMutex_;
	std::mutex generatingNoiseMutex_;

	int yMin = -1; // num. chunks
	int yMax = 2; // num. chunks (i.e. max - min would be the number of chunks high)
protected:
	static Biome GetBiomeFromTemperature(float temperature);
public:
	std::vector<glm::vec3> TreeTrunkPositions;
	std::vector<glm::vec3> TreeLeavePositions;
	World(glm::vec3 currentPlayerPos, int renderDistance);

	void Update(glm::vec3 currentPlayerPos);
	std::vector<Chunk*> GetWorld();

	// TODO: Have a second Worker with multiple internal threads that recreates multiple chunks at once
	bool LoadNewChunksAsync(int startX, int endX, int startZ, int endZ, std::vector<glm::vec3> loadedChunkPositions, std::vector<Chunk*> chunkIndexes);

	// Finds the closest position that's a multiple of the passed
	// parameter, i.e. closest x pos for a multiple of 16
	static int FindClosestPosition(int val, int multiple);

	void SetTreeBlocksForChunk(Biome biome, int x, int z, int minY, int maxY, std::vector<float>& chunkSectionNoise, int size);

	std::vector<float> GetNoiseForChunkSection(int x, int z, int size);

	void FrustumCullChunks(const Frustum& frustum);
	int NumChunksCulled();

	std::vector<Chunk*> GetChunksInsideArea(glm::vec3 origin, glm::vec3 size);

	bool IsCollidingWithWorld(CollisionDetection::CollisionBox collisionBox, CollisionDetection::CollisionBox& hitBoxOut);
	bool PerformRaycast(CollisionDetection::RaycastHit& hitOut, glm::vec3 hitStart, glm::vec3 direction, float distance, float stepColliderSize, int numSteps);

    void PlaceBlock(glm::vec3 worldLocation, uint8_t blockType);
    void BreakBlock(glm::vec3 worldLocation);

	std::vector<Chunk*>& GetChunks();
};
