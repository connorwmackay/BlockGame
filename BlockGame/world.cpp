#include "world.h"

#include <future>

#include "logging.h"
#include <unordered_map>

World::World(glm::vec3 currentPlayerPos, int renderDistance)
{
	srand(time(NULL));
	seed_ = rand();
	renderDistance_ = renderDistance;

	auto fastNoiseSimplex = FastNoise::New<FastNoise::Simplex>();
	fractalNoise_ = FastNoise::New<FastNoise::FractalFBm>();
	fractalNoise_->SetSource(fastNoiseSimplex);
	fractalNoise_->SetOctaveCount(2);
	fractalNoise_->SetLacunarity(0);
	fractalNoise_->SetGain(0.5f);

	worldWorker_ = new WorldWorker(1);

	int startZ = World::FindClosestPosition(currentPlayerPos.z, 16) - (16 * glm::floor(renderDistance_-1));
	int startX = World::FindClosestPosition(currentPlayerPos.x, 16) - (16 * glm::floor(renderDistance_-1));

	// Double render distance since it pertains to all sides
	for (int z = 0; z < renderDistance*2+1; z++)
	{
		for (int x = 0; x < renderDistance*2+1; x++)
		{
			std::vector<float> chunkSectionNoise = GetNoiseForChunkSection(startX + x * 16.0f, startZ + z * 16.0f, 16);

			for (int y = yMin; y <= yMax; y++) {
				chunks_.push_back(new Chunk(chunkSectionNoise, yMin, yMax, glm::vec3(startX + (x * 16.0f), y * 16.0f, startZ + (z * 16.0f)), 16, seed_));
			}
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

		// Load the new chunks asynchronously
		if (worldWorker_->AreAnyThreadsAvailable()) {
			worldWorker_->QueueFunction({ &World::LoadNewChunksAsync, this, startX, endX, startZ, endZ, loadedChunkPositions, unloadedChunks });
		}
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

bool World::LoadNewChunksAsync(int startX, int endX, int startZ, int endZ, std::vector<glm::vec3> loadedChunkPositions, std::vector<Chunk*> chunkIndexes)
{
	std::vector<glm::vec3> positionsToLoad = std::vector<glm::vec3>();
	std::vector<ChunkNoiseSection> chunkNoiseSections = std::vector<ChunkNoiseSection>();

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

	for (int z = startZ; z <= endZ; z += zIncrement)
	{
		for (int x = startX; x <= endX; x += xIncrement)
		{
			bool shouldAddNoise = true;
			for (glm::vec3 loadedChunkPos : loadedChunkPositions)
			{
				int loadedChunkZ = static_cast<int>(loadedChunkPos.z);
				int loadedChunkX = static_cast<int>(loadedChunkPos.x);

				if (loadedChunkX == x && loadedChunkZ == z)
				{
					shouldAddNoise = false;
				}
			}

			for (int y = yMin; y <= yMax; y++) {
				bool shouldAddPosition = true;
				for (glm::vec3 loadedChunkPos : loadedChunkPositions)
				{
					int loadedChunkZ = static_cast<int>(loadedChunkPos.z);
					int loadedChunkX = static_cast<int>(loadedChunkPos.x);
					int loadedChunkY = static_cast<int>(loadedChunkPos.y);

					if (loadedChunkX == x && loadedChunkZ == z && loadedChunkY == (y * 16))
					{
						shouldAddPosition = false;
					}
				}

				if (shouldAddPosition)
				{
					positionsToLoad.push_back(glm::vec3(x, y * 16, z));
				}
			}

			if (shouldAddNoise) {
				chunkNoiseSections.push_back({ glm::vec3(x, 0, z), GetNoiseForChunkSection(x, z, 16)});
			}
		}
	}

	for (int i = 0; i < chunkIndexes.size(); i++)
	{
		if (positionsToLoad.size() > 0) {
			glm::vec3 newPosition = positionsToLoad.at(0);
			positionsToLoad.erase(positionsToLoad.begin());

			int chunkNoiseSectionInd = 0;

			for (int i=0; i < chunkNoiseSections.size(); i++)
			{
				ChunkNoiseSection& chunkNoiseSection = chunkNoiseSections.at(i);

				if (chunkNoiseSection.position.x == newPosition.x && chunkNoiseSection.position.z == newPosition.z)
				{
					chunkNoiseSectionInd = i;
				}
			}

			chunkIndexes[i]->Recreate(chunkNoiseSections.at(chunkNoiseSectionInd).noise, yMin, yMax, newPosition, seed_, false);
		}
	}
	return true;
}

std::vector<float> World::GetNoiseForChunkSection(int x, int z, int size)
{
	auto noiseOutput = std::vector<float>(size * size);
	fractalNoise_->GenUniformGrid2D(
		noiseOutput.data(),
		x,
		z,
		size,
		size,
		0.01f,
		seed_
	);

	return noiseOutput;
}