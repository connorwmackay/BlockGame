#include "world.h"

#include <future>

#include "logging.h"

World::World(glm::vec3 currentPlayerPos, int renderDistance)
{
	srand(time(NULL));
	seed_ = rand();
	renderDistance_ = renderDistance;

	simplexNoise_ = FastNoise::New<FastNoise::Simplex>();

	int startZ = World::FindClosestPosition(currentPlayerPos.z, 16) - (16 * glm::floor(renderDistance_-1));
	int startX = World::FindClosestPosition(currentPlayerPos.x, 16) - (16 * glm::floor(renderDistance_-1));

	// Double render distance since it pertains to all sides
	for (int z = 0; z < renderDistance*2+1; z++)
	{
		for (int x = 0; x < renderDistance*2+1; x++)
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

	if ((oldZ != newZ || oldX != newX))
	{
		int startZ = newZ - (int)(16 * glm::floor(renderDistance_));
		int startX = newX - (int)(16 * glm::floor(renderDistance_));
		int endZ = newZ + (int)(16 * glm::floor(renderDistance_));
		int endX = newX + (int)(16 * glm::floor(renderDistance_));

		// Unload all chunks outside the bounds set just above
		std::vector<Chunk*> unloadedChunks = std::vector<Chunk*>();
		std::vector<glm::vec3> loadedChunkPositions = std::vector<glm::vec3>();
		for (Chunk* chunk : chunks_)
		{
			TransformComponent* transformComponent = static_cast<TransformComponent*>(chunk->GetComponentByName("transform"));
			glm::vec3 chunkLoc = transformComponent->GetTranslation();
			int chunkX = chunkLoc.x;
			int chunkZ = chunkLoc.z;

			// If chunk is outside the new render bounds
			if (chunkZ < startZ || chunkZ > endZ || chunkX < startX || chunkX > endX)
			{
				// Unload the chunk
				chunk->Unload();

				// Add to unloaded chunks list
				unloadedChunks.push_back(chunk);
			}
			else
			{
				loadedChunkPositions.push_back(chunkLoc);
			}
		}

		std::vector<glm::vec3> positionsToLoad = std::vector<glm::vec3>();

		int zIncrement = 16;
		if (endZ < startZ)
		{
			zIncrement = -16;
		}

		int xIncrement = 16;
		if (endX < startX)
		{
			xIncrement = -16;
		}

		for (int z=startZ; z <= endZ; z += zIncrement)
		{
			for (int x=startX; x <= endX; x += xIncrement)
			{
				bool isAlreadyLoaded = false;
				for (glm::vec3 loadedChunkPos : loadedChunkPositions)
				{
					int loadedChunkZ = static_cast<int>(loadedChunkPos.z);
					int loadedChunkX = static_cast<int>(loadedChunkPos.x);
					
					if (loadedChunkX == x && loadedChunkZ == z)
					{
						isAlreadyLoaded = true;
					}
				}

				if (!isAlreadyLoaded)
				{
					positionsToLoad.push_back(glm::vec3(x, 0.0f, z));
				}
			}
		}

		// Load the new chunks asynchronously
		loadingChunks_ = std::async(std::launch::async, &World::LoadNewChunksAsync, this, positionsToLoad, unloadedChunks);
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

bool World::LoadNewChunksAsync(std::vector<glm::vec3> positions, std::vector<Chunk*> chunkIndexes)
{
	loadingChunksMutex_.lock();
	for (int i = 0; i < chunkIndexes.size(); i++)
	{
		if (positions.size() > 0) {
			glm::vec3 newPosition = positions.at(0);
			positions.erase(positions.begin());

			chunkIndexes[i]->Recreate(&simplexNoise_, newPosition, seed_, false);
		}
	}
	loadingChunksMutex_.unlock();
	return true;
}