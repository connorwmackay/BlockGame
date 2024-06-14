#include "chunk.h"

#include <GLFW/glfw3.h>

#include "logging.h"
#include "meshComponent.h"
#include "transformComponent.h"

#include <noise/noise.h>
#include "noiseutils.h"

#include "world.h"

Chunk::Chunk(World* world, Biome biome, Texture2DArray texture, std::vector<float> chunkSectionNoise, int minY, int maxY, glm::vec3 startingPosition, int size, int seed)
	: Entity()
{
	needsUpdated = false;
	shouldDraw_ = true;
	world_ = world;
	texture_ = texture;

	size_.store(size);
	blocks_ = std::vector<std::vector<std::vector<uint8_t>>>();
	seed_.store(seed);
	isUnloaded.store(false);
	biome_ = biome;

	AddComponent("transform", new TransformComponent(this, startingPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
	transformComponent = static_cast<TransformComponent*>(GetComponentByName("transform"));

	treeTrunkPositions_ = std::vector<glm::vec3>(size * size * size);
	treeLeavePositions_ = std::vector<glm::vec3>(size * size * size);

	UseNoise(chunkSectionNoise, minY, maxY);
	UpdateBlocks();

	AddComponent("mesh", new MeshComponent(this, new Mesh(&texture_, MeshType::Chunk)));
	meshComponent = static_cast<MeshComponent*>(GetComponentByName("mesh"));
	TextureAtlas textureAtlas = { 8, 6 };

	GenerateMesh();
}

void Chunk::GenerateMesh(bool isOnMainThread)
{
	SubTexture textureAtlasSubTexture = GetSubTextureFromTextureAtlas(0, 0, { 1, 1 });

	Mesh* mesh = meshComponent->GetMesh();

	float meshStartX = -1.0f * (size_.load() / 2.0f);
	float meshStartY = -1.0f * (size_.load() / 2.0f);
	float meshStartZ = -1.0f * (size_.load() / 2.0f);

	int numBlocksRendered = 0;

	// We want to declare this inside the function because
	// chunks can be called on other threads, therefore
	// it's better just to replace the vertices and indices
	// on the mesh rather than pass them between classes etc.
	std::vector<Vertex> vertices = std::vector<Vertex>(blocks_.size() * 4 * 6);
	std::vector<unsigned int> indices = std::vector<unsigned int>(blocks_.size() * 6 * 6);

	for (int z = 0; z < size_.load(); z++)
	{
		for (int x = 0; x < size_.load(); x++)
		{
			for (int y = 0; y < size_.load(); y++)
			{
				uint8_t currentBlock = blocks_[z][x][y];

				// Only add to mesh if the block can be rendered
				if (currentBlock != BLOCK_TYPE_AIR) {
					numBlocksRendered++;

					// Get the adjacent blocks
					uint8_t adjacentBlockUp = BLOCK_TYPE_AIR;
					if (IsInChunk(x, y + 1, z)) {
						adjacentBlockUp = blocks_[z][x][y + 1];
					}

					uint8_t adjacentBlockDown = BLOCK_TYPE_AIR;
					if (IsInChunk(x, y - 1, z))
					{
						adjacentBlockDown = blocks_[z][x][y-1];
					}

					uint8_t adjacentBlockRight = BLOCK_TYPE_AIR;
					if (IsInChunk(x + 1, y, z))
					{
						adjacentBlockRight = blocks_[z][x+1][y];
					}

					uint8_t adjacentBlockLeft = BLOCK_TYPE_AIR;
					if (IsInChunk(x - 1, y, z))
					{
						adjacentBlockLeft = blocks_[z][x-1][y];
					}

					uint8_t adjacentBlockFront = BLOCK_TYPE_AIR;
					if (IsInChunk(x, y, z + 1))
					{
						adjacentBlockFront = blocks_[z + 1][x][y];
					}

					uint8_t adjacentBlockBack = BLOCK_TYPE_AIR;
					if (IsInChunk(x, y, z - 1))
					{
						adjacentBlockBack = blocks_[z - 1][x][y];
					}

					// The bottom-left corner of the current block in the mesh
					float meshX = meshStartX + x;
					float meshY = meshStartY + y;
					float meshZ = meshStartZ + z;

					int currentRow = currentBlock-1;

					// Add each block face that faces an air block
					if (adjacentBlockUp == BLOCK_TYPE_AIR)
					{
						int textureAtlasIndex = currentRow * texture_.GetNumCols() + 0;

						vertices.push_back({ meshX,		meshY + 1,	meshZ, 0.0f, 1.0f, 0.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Left
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ, 0.0f, 1.0f, 0.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Right
						vertices.push_back({ meshX,		meshY + 1,	meshZ - 1, 0.0f, 1.0f, 0.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-left
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ - 1, 0.0f, 1.0f, 0.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(), 
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 1,
							offsetStart - 3,
							offsetStart - 2,
							}
						);
					}

					if (adjacentBlockDown == BLOCK_TYPE_AIR)
					{
						int textureAtlasIndex = currentRow * texture_.GetNumCols() + 1;

						vertices.push_back({ meshX,		meshY,		meshZ, 0.0f, -1.0f, 0.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Left
						vertices.push_back({ meshX + 1,	meshY,		meshZ, 0.0f, -1.0f, 0.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Right
						vertices.push_back({ meshX,		meshY,		meshZ - 1, 0.0f, -1.0f, 0.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-left
						vertices.push_back({ meshX + 1,	meshY,		meshZ - 1, 0.0f, -1.0f, 0.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(),
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 1,
							offsetStart - 3,
							offsetStart - 2,
							}
						);
					}

					if (adjacentBlockRight == BLOCK_TYPE_AIR)
					{
						int textureAtlasIndex = currentRow * texture_.GetNumCols() + 2;

						vertices.push_back({ meshX + 1,	meshY,		meshZ, 1.0f, 0.0f, 0.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Left
						vertices.push_back({ meshX + 1,	meshY,		meshZ - 1, 1.0f, 0.0f, 0.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Right
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ, 1.0f, 0.0f, 0.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-left
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ - 1, 1.0f, 0.0f, 0.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(),
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 1,
							offsetStart - 3,
							offsetStart - 2,
							}
						);
					}

					if (adjacentBlockLeft == BLOCK_TYPE_AIR)
					{
						int textureAtlasIndex = currentRow * texture_.GetNumCols() + 3;

						vertices.push_back({ meshX,		meshY,		meshZ, -1.0f, 0.0f, 0.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Left
						vertices.push_back({ meshX,		meshY,		meshZ - 1, -1.0f, 0.0f, 0.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Right
						vertices.push_back({ meshX,		meshY + 1,	meshZ, -1.0f, 0.0f, 0.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-left
						vertices.push_back({ meshX,		meshY + 1,	meshZ - 1, -1.0f, 0.0f, 0.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(),
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 1,
							offsetStart - 3,
							offsetStart - 2,
							}
						);
					}

					if (adjacentBlockFront == BLOCK_TYPE_AIR)
					{
						int textureAtlasIndex = currentRow * texture_.GetNumCols() + 4;

						vertices.push_back({ meshX,		meshY,		meshZ, 0.0f, 0.0f, 1.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Left
						vertices.push_back({ meshX + 1,	meshY,		meshZ, 0.0f, 0.0f, 1.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Right
						vertices.push_back({ meshX,		meshY + 1,	meshZ, 0.0f, 0.0f, 1.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-left
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ, 0.0f, 0.0f, 1.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(),
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 1,
							offsetStart - 3,
							offsetStart - 2,
							}
						);
					}

					if (adjacentBlockBack == BLOCK_TYPE_AIR)
					{
						int textureAtlasIndex = currentRow * texture_.GetNumCols() + 5;

						vertices.push_back({ meshX,		meshY,		meshZ - 1, 0.0f, 0.0f, -1.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Left
						vertices.push_back({ meshX + 1,	meshY,		meshZ - 1, 0.0f, 0.0f, -1.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.startT, textureAtlasIndex }); // Bottom-Right
						vertices.push_back({ meshX,		meshY + 1,	meshZ - 1, 0.0f, 0.0f, -1.0f, textureAtlasSubTexture.startS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-left
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ - 1, 0.0f, 0.0f, -1.0f, textureAtlasSubTexture.endS, textureAtlasSubTexture.endT, textureAtlasIndex }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(),
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 1,
							offsetStart - 3,
							offsetStart - 2,
							}
						);
					}
				}
			}
		}
	}

	mesh->SetVertices(vertices);
	mesh->SetIndices(indices);

	if (isOnMainThread) {
		meshComponent->SetModel(transformComponent->GetModel());
	}
}

bool Chunk::IsInChunk(int x, int y, int z)
{
	// Ensure the chunk actually has blocks in it.
	if (blocks_.empty())
	{
		LOG("There are no blocks in the chunk: (%d, %d, %d)\n", x, y ,z);
		return false;
	}

	if (blocks_[0].empty())
	{
		LOG("There are no blocks in the chunk: (%d, %d, %d)\n", x, y, z);
		return false;
	}

	if (blocks_[0][0].empty())
	{
		LOG("There are no blocks in the chunk: (%d, %d, %d)\n", x, y, z);
		return false;
	}

	// Define what constitutes being inside the chunk.
	int minZ = 0;
	int maxZ = blocks_.size()-1;
	int minX = 0;
	int maxX = blocks_[0].size()-1;
	int minY = 0;
	int maxY = blocks_[0][0].size() - 1;

	// If it's within the bounds
	if ((z >= minZ && z <= maxZ) &&
		(x >= minX && x <= maxX) &&
		(y >= minY && y <= maxY))
	{
		return true;
	}

	return false;
}

void Chunk::Update()
{
	if (transformComponent->GetHasChanged())
	{
		meshComponent->SetModel(transformComponent->GetModel());
		transformComponent->SetHasChanged(false);
	}
}

void Chunk::Draw()
{
	if (!isUnloaded.load() && shouldDraw_) {
		meshComponent->Draw();
	}
}

void Chunk::UseNoise(std::vector<float> chunkSectionNoise, int minY, int maxY)
{
	std::vector<std::vector<std::vector<uint8_t>>> blocks = std::vector<std::vector<std::vector<uint8_t>>>();

	if (!transformComponent)
	{
		LOG("Error: Transform Component was nullptr\n");
		return;
	}

	glm::vec3 position = transformComponent->GetTranslation();

	// Default values
	uint8_t surfaceBlock = BLOCK_TYPE_GRASS;
	uint8_t subSurfaceBlockHigh = BLOCK_TYPE_DIRT;
	uint8_t subSurfaceBlockLow = BLOCK_TYPE_STONE;

	switch (biome_)
	{
	case Biome::Desert:
		surfaceBlock = BLOCK_TYPE_SAND;
		subSurfaceBlockHigh = BLOCK_TYPE_SAND;
		subSurfaceBlockLow = BLOCK_TYPE_SAND;
		break;
	case Biome::Grassland:
		surfaceBlock = BLOCK_TYPE_GRASS;
		subSurfaceBlockHigh = BLOCK_TYPE_DIRT;
		subSurfaceBlockLow = BLOCK_TYPE_STONE;
		break;
	case Biome::Snow:
		surfaceBlock = BLOCK_TYPE_SNOW;
		subSurfaceBlockHigh = BLOCK_TYPE_DIRT;
		subSurfaceBlockLow = BLOCK_TYPE_STONE;
		break;
	case Biome::Rock:
		surfaceBlock = BLOCK_TYPE_STONE;
		subSurfaceBlockHigh = BLOCK_TYPE_STONE;
		subSurfaceBlockLow = BLOCK_TYPE_STONE;
		break;
	case Biome::Forest:
		surfaceBlock = BLOCK_TYPE_FORESTGRASS;
		subSurfaceBlockHigh = BLOCK_TYPE_DIRT;
		subSurfaceBlockLow = BLOCK_TYPE_STONE;
		break;
	}

	int currentBlockIndex = 0;
	int currentNoiseIndex = 0;
	for (int z = 0; z < size_.load(); z++)
	{
		std::vector<std::vector<uint8_t>> zRow = std::vector<std::vector<uint8_t>>();

		for (int x = 0; x < size_.load(); x++)
		{
			std::vector<uint8_t> xRow = std::vector<uint8_t>();

			float currentNoiseVal = chunkSectionNoise[currentNoiseIndex];
			float ySize = glm::abs(maxY - minY) * size_;
			float ySurface = (ySize / 2) + (currentNoiseVal * ySize / 2);

			for (int y = 0; y < size_.load(); y++)
			{
				uint8_t currentBlock = BLOCK_TYPE_AIR;

				float currentY = position.y + (float)y;

				if ((int)currentY < (int)ySurface && currentY >(int)ySurface - 3)
				{
					currentBlock = subSurfaceBlockHigh;
				}
				else if ((int)currentY < (int)ySurface)
				{
					currentBlock = subSurfaceBlockLow;
				}
				else if ((int)currentY > (int)ySurface)
				{
					currentBlock = BLOCK_TYPE_AIR;
				}
				else
				{
					currentBlock = surfaceBlock;
				}

				xRow.push_back(currentBlock);
				currentBlockIndex++;
			}
			currentNoiseIndex++;
			zRow.push_back(xRow);
		}

		blocks.push_back(zRow);
	}

	blocks_ = blocks;
}

void Chunk::Unload()
{
	meshComponent->GetMesh()->Unload();
	isUnloaded.store(true);
	shouldDraw_ = true;
}

void Chunk::Recreate(Biome biome, std::vector<float> chunkSectionNoise, int minY, int maxY, glm::vec3 newStartingPosition, int seed, bool isOnMainThread)
{
	seed_.store(seed);
	blocks_.clear();
	biome_ = biome;
	transformComponent->SetTranslation(newStartingPosition);
	UseNoise(chunkSectionNoise, minY, maxY);
	UpdateBlocks();
	GenerateMesh(isOnMainThread);
	isUnloaded.store(false);
}

bool Chunk::IsUnloaded()
{
	return isUnloaded;
}

Biome Chunk::GetBiome()
{
	return biome_;
}

void Chunk::AddTreeLeavePositions()
{
	world_->TreeLeavePositions.insert(world_->TreeLeavePositions.end(), treeLeavePositions_.begin(), treeLeavePositions_.end());
}

void Chunk::AddTreeTrunkPositions()
{
	world_->TreeTrunkPositions.insert(world_->TreeTrunkPositions.end(), treeTrunkPositions_.begin(), treeTrunkPositions_.end());
}


TransformComponent* Chunk::GetTransformComponent()
{
	return transformComponent;
}

bool Chunk::UpdateBlocks()
{
	bool hasUpdatedBlocks = false;

	glm::vec3 position = transformComponent->GetTranslation();

	auto localTreeTrunkPositions = std::vector<glm::vec3*>();
	auto localTreeLeavePositions = std::vector<glm::vec3*>();

	for (glm::vec3& treeLeavePos : world_->TreeLeavePositions)
	{
		if (treeLeavePos.x >= position.x && treeLeavePos.x <= position.x + size_ &&
			treeLeavePos.y >= position.y && treeLeavePos.y <= position.y + size_ &&
			treeLeavePos.z >= position.z && treeLeavePos.z <= position.z + size_)
		{
			localTreeLeavePositions.push_back(&treeLeavePos);
		}
	}

	for (glm::vec3& treeTrunkPos : world_->TreeTrunkPositions)
	{
		if (treeTrunkPos.x >= position.x && treeTrunkPos.x <= position.x + size_ &&
			treeTrunkPos.y >= position.y && treeTrunkPos.y <= position.y + size_ &&
			treeTrunkPos.z >= position.z && treeTrunkPos.z <= position.z + size_)
		{
			localTreeTrunkPositions.push_back(&treeTrunkPos);
		}
	}

	for (int z = 0; z < size_.load(); z++)
	{
		for (int x = 0; x < size_.load(); x++)
		{
			for (int y = 0; y < size_.load(); y++)
			{
				uint8_t currentBlock = BLOCK_TYPE_AIR;

				for (glm::vec3* treeLeavePos : localTreeLeavePositions)
				{
					if (treeLeavePos != nullptr) {
						if ((int)position.x + x == (int)treeLeavePos->x && (int)position.y + y == (int)treeLeavePos->y && (int)position.z + z == (int)treeLeavePos->z)
						{
							currentBlock = BLOCK_TYPE_TREELEAVES;
						}
					}
				}

				for (glm::vec3* treeTrunkPos : localTreeTrunkPositions)
				{
					if (treeTrunkPos != nullptr) {
						if ((int)position.x + x == (int)treeTrunkPos->x && (int)position.y + y == (int)treeTrunkPos->y && (int)position.z + z == (int)treeTrunkPos->z)
						{
							currentBlock = BLOCK_TYPE_TREEBARK;
						}
					}
				}

				if (currentBlock != BLOCK_TYPE_AIR)
				{
					blocks_.at(z).at(x).at(y) = currentBlock;
					hasUpdatedBlocks = true;
				}
			}
		}
	}

	UpdateCollisionData();

	return hasUpdatedBlocks;
}

void Chunk::UpdateCollisionData()
{
	// Update Collision Data
	collisionBoxes.clear();
	glm::vec3 pos = transformComponent->GetTranslation();
	pos.x -= size_ / 2;
	pos.y -= size_ / 2;
	pos.z -= size_ / 2;

	for (int z = 0; z < size_.load(); z++)
	{
		for (int x = 0; x < size_.load(); x++)
		{
			for (int y = 0; y < size_.load(); y++)
			{
				uint8_t blockType = blocks_.at(z).at(x).at(y);
				if (blockType != BLOCK_TYPE_AIR) {

					collisionBoxes.push_back({
						glm::vec3(x + pos.x, y + pos.y, z + pos.z),
						glm::vec3(1.0f, 1.0f, 1.0f)
						});
				}
			}
		}
	}
}

void Chunk::SetShouldDraw(bool shouldDraw)
{
	shouldDraw_ = shouldDraw;
}

bool Chunk::GetShouldDraw()
{
	return shouldDraw_;
}

std::vector<CollisionDetection::CollisionBox>& Chunk::GetCollisionBoxes() {
	return collisionBoxes;
}

bool Chunk::RemoveBlockAt(glm::vec3 worldPosition)
{
	glm::vec3 chunkPosition = worldPosition - transformComponent->GetTranslation();

	chunkPosition.x += size_ / 2;
	chunkPosition.y += size_ / 2;
	chunkPosition.z += size_ / 2;

	if ((chunkPosition.x >= 0.0f && chunkPosition.x < size_) &&
		(chunkPosition.y >= 0.0f && chunkPosition.y < size_) &&
		(chunkPosition.z >= 0.0f && chunkPosition.z < size_)) {

		blocks_.at(chunkPosition.z).at(chunkPosition.x).at(chunkPosition.y) = BLOCK_TYPE_AIR;
		Reload();

        LOG("Block Position <Inside Chunk>: (%f, %f, %f)\n", chunkPosition.x, chunkPosition.y, chunkPosition.z);
		return true;
	}
	else
	{
        LOG("Block Position <Outside Chunk>: (%f, %f, %f)\n", chunkPosition.x, chunkPosition.y, chunkPosition.z);
	}

	return false;
}

bool Chunk::PlaceBlockNextTo(glm::vec3 worldPosition, glm::vec3 playerPos, uint8_t blockType)
{

	glm::vec3 chunkPosition = worldPosition - transformComponent->GetTranslation();

	chunkPosition.x += size_ / 2;
	chunkPosition.y += size_ / 2;
	chunkPosition.z += size_ / 2;

	if ((chunkPosition.x >= 0.0f && chunkPosition.x < size_) &&
		(chunkPosition.y >= 0.0f && chunkPosition.y < size_) &&
		(chunkPosition.z >= 0.0f && chunkPosition.z < size_)) {

		// Block That Was Hit
		uint8_t hitBlock = blocks_.at(chunkPosition.z).at(chunkPosition.x).at(chunkPosition.y);

		glm::vec3 nearestAirBlockPosition;
		float minDistance = 1000.0f;

		// Check Adjacent Blocks If Required
		if ((chunkPosition.z - 1.0f) >= 0.0f)
		{
			uint8_t adjacentBlock = blocks_.at(chunkPosition.z - 1.0f).at(chunkPosition.x).at(chunkPosition.y);
			if (adjacentBlock == BLOCK_TYPE_AIR)
			{
				glm::vec3 worldPos = glm::vec3(worldPosition.x, worldPosition.y, worldPosition.z - 1.0f);
				glm::vec3 pos = glm::vec3(chunkPosition.x, chunkPosition.y, chunkPosition.z - 1.0f);
				float curDistance = glm::distance(playerPos, worldPos);

				if (curDistance < minDistance)
				{
					minDistance = curDistance;
					nearestAirBlockPosition = pos;
				}
			}
		}

		if ((chunkPosition.z + 1.0f) < size_)
		{
			uint8_t adjacentBlock = blocks_.at(chunkPosition.z + 1.0f).at(chunkPosition.x).at(chunkPosition.y);
			if (adjacentBlock == BLOCK_TYPE_AIR)
			{
				glm::vec3 worldPos = glm::vec3(worldPosition.x, worldPosition.y, worldPosition.z + 1.0f);
				glm::vec3 pos = glm::vec3(chunkPosition.x, chunkPosition.y, chunkPosition.z + 1.0f);
				float curDistance = glm::distance(playerPos, worldPos);

				if (curDistance < minDistance)
				{
					minDistance = curDistance;
					nearestAirBlockPosition = pos;
				}
			}
		}

		if ((chunkPosition.x - 1.0f) >= 0.0f)
		{
			uint8_t adjacentBlock = blocks_.at(chunkPosition.z).at(chunkPosition.x - 1.0f).at(chunkPosition.y);
			if (adjacentBlock == BLOCK_TYPE_AIR)
			{
				glm::vec3 worldPos = glm::vec3(worldPosition.x - 1.0f, worldPosition.y, worldPosition.z);
				glm::vec3 pos = glm::vec3(chunkPosition.x - 1.0f, chunkPosition.y, chunkPosition.z);
				float curDistance = glm::distance(playerPos, worldPos);

				if (curDistance < minDistance)
				{
					minDistance = curDistance;
					nearestAirBlockPosition = pos;
				}
			}
		}

		if ((chunkPosition.x + 1.0f) < size_)
		{
			uint8_t adjacentBlock = blocks_.at(chunkPosition.z).at(chunkPosition.x + 1.0f).at(chunkPosition.y);
			if (adjacentBlock == BLOCK_TYPE_AIR)
			{
				glm::vec3 worldPos = glm::vec3(worldPosition.x + 1.0f, worldPosition.y, worldPosition.z);
				glm::vec3 pos = glm::vec3(chunkPosition.x + 1.0f, chunkPosition.y, chunkPosition.z);
				float curDistance = glm::distance(playerPos, worldPos);

				if (curDistance < minDistance)
				{
					minDistance = curDistance;
					nearestAirBlockPosition = pos;
				}
			}
		}

		if ((chunkPosition.y - 1.0f) >= 0.0f)
		{
			uint8_t adjacentBlock = blocks_.at(chunkPosition.z).at(chunkPosition.x).at(chunkPosition.y - 1.0f);
			if (adjacentBlock == BLOCK_TYPE_AIR)
			{
				glm::vec3 worldPos = glm::vec3(worldPosition.x, worldPosition.y - 1.0f, worldPosition.z);
				glm::vec3 pos = glm::vec3(chunkPosition.x, chunkPosition.y - 1.0f, chunkPosition.z);
				float curDistance = glm::distance(playerPos, worldPos);

				if (curDistance < minDistance)
				{
					minDistance = curDistance;
					nearestAirBlockPosition = pos;
				}
			}
		}

		if ((chunkPosition.y + 1.0f) < size_)
		{
			uint8_t adjacentBlock = blocks_.at(chunkPosition.z).at(chunkPosition.x).at(chunkPosition.y + 1.0f);
			if (adjacentBlock == BLOCK_TYPE_AIR)
			{
				glm::vec3 worldPos = glm::vec3(worldPosition.x, worldPosition.y + 1.0f, worldPosition.z);
				glm::vec3 pos = glm::vec3(chunkPosition.x, chunkPosition.y + 1.0f, chunkPosition.z);
				float curDistance = glm::distance(playerPos, worldPos);

				if (curDistance < minDistance)
				{
					minDistance = curDistance;
					nearestAirBlockPosition = pos;
				}
			}
		}

		if (minDistance < 1000.0f)
		{
			blocks_.at(nearestAirBlockPosition.z).at(nearestAirBlockPosition.x).at(nearestAirBlockPosition.y) = blockType;
			Reload();
			return true;
		}

        LOG("Block Position <Inside Chunk>: (%f, %f, %f)\n", chunkPosition.x, chunkPosition.y, chunkPosition.z);
	}
	else
	{
        LOG("Block Position <Outside Chunk>: (%f, %f, %f)\n", chunkPosition.x, chunkPosition.y, chunkPosition.z);
	}

	return false;
}

void Chunk::Reload()
{
	UpdateCollisionData();
	GenerateMesh(true);
}