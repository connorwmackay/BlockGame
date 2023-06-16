#include "world.h"

#include <future>

#include "logging.h"

World::World(glm::vec3 currentPlayerPos, int renderDistance)
{
	srand(time(NULL));
	seed_ = rand();
	renderDistance_ = renderDistance;

	simplexNoise_ = FastNoise::New<FastNoise::Simplex>();

	int startZ = World::FindClosestPosition(currentPlayerPos.z, 16) - ((16 * (renderDistance-1)) / 2);
	int startX = World::FindClosestPosition(currentPlayerPos.x, 16) - ((16 * (renderDistance-1)) / 2);

	for (int z = 0; z < renderDistance; z++)
	{
		for (int x = 0; x < renderDistance; x++)
		{
			chunks_.push_back(new Chunk(std::atomic(&simplexNoise_), glm::vec3(startX + (x * 16.0f), 0.0f, startZ + (z * 16.0f)), 16, seed_));
		}
	}

	lastKnownPlayerPos_ = currentPlayerPos;
}

void World::Update(glm::vec3 currentPlayerPos)
{
	int oldZ = World::FindClosestPosition(lastKnownPlayerPos_.z, 16);
	int oldX = World::FindClosestPosition(lastKnownPlayerPos_.x, 16);
	int newZ = World::FindClosestPosition(currentPlayerPos.z, 16);
	int newX = World::FindClosestPosition(currentPlayerPos.x, 16);

	// TODO: Perform a raycast from player's forward vector to the nearest chunk

	if (oldZ != newZ || oldX != newX)
	{
		// Determine startX, endX, startZ, endZ
		// i.e. so that all chunks are from the start of player pos onwards...


		// Load the new chunks asynchronously
		//loadingChunks_ = std::async(std::launch::async, &World::LoadNewChunksAsync, this, positionsToLoad, unloadedChunkIndexes);
	}
	lastKnownPlayerPos_ = currentPlayerPos;
}

std::vector<Chunk*> World::GetWorld()
{
	return chunks_;
}

int World::FindClosestPosition(int val, int multiple)
{
	int quotient = val / multiple;

	int option1 = multiple * quotient;
	int option2 = multiple * (quotient + 1);

	if (abs(val - option1) < abs(val - option2))
	{
		return option1;
	}

	return option2;
}

bool World::LoadNewChunksAsync(std::vector<glm::vec3> positions, std::vector<int> chunkIndexes)
{
	for (int& chunkIndex : chunkIndexes)
	{
		if (positions.size() > 0) {
			glm::vec3 newPosition = positions.at(0);
			positions.erase(positions.begin());

			if (chunkIndex >= 0 && chunkIndex < chunks_.size()) {
				chunks_[chunkIndex]->Recreate(&simplexNoise_, newPosition, seed_, false);
			}
		}
		else
		{
			LOG("Error: There weren't enough positions to fill the unloaded chunks\n");
		}
	}

	return true;
}