#include "world.h"

#include <future>

#include "logging.h"
#include <unordered_map>
#include <GLFW/glfw3.h>

#include <noiseutils.h>

World::World(glm::vec3 currentPlayerPos, int renderDistance)
{
	srand(time(NULL));
	seed_ = rand();
	renderDistance_ = renderDistance;

	// Setup the noise for terrain generation
	baseMountainTerrainModule_.SetSeed(seed_);
	baseMountainTerrainModule_.SetOctaveCount(3);
	baseMountainTerrainModule_.SetFrequency(0.01f);
	baseMountainTerrainModule_.SetLacunarity(1);

	mountainTerrainModule_.SetSourceModule(0, baseMountainTerrainModule_);
	mountainTerrainModule_.SetScale(0.3f);
	mountainTerrainModule_.SetBias(-0.5f);

	baseHillyTerrainModule_.SetSeed(seed_);
	baseHillyTerrainModule_.SetOctaveCount(3);
	baseHillyTerrainModule_.SetFrequency(0.01f);

	hillyTerrainModule_.SetSourceModule(0, baseHillyTerrainModule_);
	hillyTerrainModule_.SetScale(0.4f);
	hillyTerrainModule_.SetBias(-0.70f);

	baseFlatTerrainModule_.SetSeed(seed_);
	baseFlatTerrainModule_.SetOctaveCount(2);
	baseFlatTerrainModule_.SetFrequency(0.01f);

	flatTerrainModule_.SetSourceModule(0, baseFlatTerrainModule_);
	flatTerrainModule_.SetScale(0.25f);
	flatTerrainModule_.SetBias(-0.75f);

	auto temperatureNoiseSimplex = FastNoise::New<FastNoise::Simplex>();
	temperatureNoise_ = FastNoise::New<FastNoise::FractalFBm>();
	temperatureNoise_->SetSource(temperatureNoiseSimplex);
	temperatureNoise_->SetOctaveCount(1);
	temperatureNoise_->SetLacunarity(2);
	temperatureNoise_->SetGain(1.0f);

	terrainTypeModule_.SetSeed(seed_);
	terrainTypeModule_.SetFrequency(0.005f);
	terrainTypeModule_.SetPersistence(0.4f);
	terrainTypeModule_.SetOctaveCount(4);

	terrainSelectorModule_.SetSourceModule(0, flatTerrainModule_);
	terrainSelectorModule_.SetSourceModule(1, mountainTerrainModule_);
	terrainSelectorModule_.SetSourceModule(2, hillyTerrainModule_);
	terrainSelectorModule_.SetControlModule(terrainTypeModule_);
	terrainSelectorModule_.SetBounds(0.0f, 1000.0f);
	terrainSelectorModule_.SetEdgeFalloff(0.1f);

	terrainTurbulenceModule_.SetSourceModule(0, terrainSelectorModule_);
	terrainTurbulenceModule_.SetFrequency(0.03f);
	terrainTurbulenceModule_.SetPower(0.2f);

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
				float temperature = 0.0f;
				temperatureNoise_->GenUniformGrid2D(&temperature, (startX + x * 16.0f)/16.0f, (startZ + z * 16.0f)/16.0f, 1, 1, 0.05f, seed_);
				chunks_.push_back(new Chunk(World::GetBiomeFromTemperature(temperature), chunkSectionNoise, yMin, yMax, glm::vec3(startX + (x * 16.0f), y * 16.0f, startZ + (z * 16.0f)), 16, seed_));
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

	if ((oldZ != newZ || oldX != newX) && worldWorker_->AreAnyThreadsAvailable())
	{
		// Load the new chunks asynchronously
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

		worldWorker_->QueueFunction({ &World::LoadNewChunksAsync, this, startX, endX, startZ, endZ, loadedChunkPositions, unloadedChunks });
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
	float createNoiseStartTime = glfwGetTime();
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
	float createNoiseEndTime = glfwGetTime();
	LOG("Create Noise Time: %fms\n", (createNoiseEndTime - createNoiseStartTime) * 1000);

	float recreateChunksStartTime = glfwGetTime();
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

			float temperature = 0.0f;
			temperatureNoise_->GenUniformGrid2D(&temperature, newPosition.x/16.0f, newPosition.z/16.0f, 1, 1, 0.05f, seed_);
			chunkIndexes[i]->Recreate(GetBiomeFromTemperature(temperature), chunkNoiseSections.at(chunkNoiseSectionInd).noise, yMin, yMax, newPosition, seed_, false);
		}
	}
	float recreateChunksEndTime = glfwGetTime();
	LOG("Recreate Chunks Time: %fms\n", (recreateChunksEndTime - recreateChunksStartTime) * 1000);

	return true;
}

std::vector<float> World::GetNoiseForChunkSection(int x, int z, int size)
{
	auto terrainNoiseOutput = std::vector<float>();

	utils::NoiseMap chunkHeightMap;
	utils::NoiseMapBuilderPlane chunkHeightMapBuilderPlane;
	chunkHeightMapBuilderPlane.SetSourceModule(terrainTurbulenceModule_);
	chunkHeightMapBuilderPlane.SetDestSize(size, size);
	chunkHeightMapBuilderPlane.SetDestNoiseMap(chunkHeightMap);
	chunkHeightMapBuilderPlane.SetBounds(x, (x + size)-1, z, (z + size)-1);
	chunkHeightMapBuilderPlane.Build();

	chunkHeightMap.SetBorderValue(0.0f);

	for (int cz = 0; cz < size; cz++)
	{
		for (int cx=0; cx < size; cx++)
		{
			float noiseValue = chunkHeightMap.GetValue(cx, cz);
			terrainNoiseOutput.push_back(glm::clamp(noiseValue, -1.0f, 1.0f));
		}
	}

	chunkHeightMap.ReclaimMem();
	return terrainNoiseOutput;
}

Biome World::GetBiomeFromTemperature(float temperature)
{
	Biome biome = Biome::Grassland;

	if (temperature <= -0.7f)
	{
		biome = Biome::Snow;
	}
	else if (temperature > -0.7f && temperature <= -0.3f)
	{
		biome = Biome::Grassland;
	}
	else
	{
		biome = Biome::Desert;
	}

	return biome;
}