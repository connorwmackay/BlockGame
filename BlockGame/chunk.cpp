#include "chunk.h"

#include <GLFW/glfw3.h>

#include "logging.h"
#include "meshComponent.h"
#include "transformComponent.h"

Chunk::Chunk(std::vector<float> chunkSectionNoise, int minY, int maxY, glm::vec3 startingPosition, int size, int seed)
	: Entity()
{
	size_.store(size);
	blocks_ = std::vector<std::vector<std::vector<uint8_t>>>();
	seed_.store(seed);
	isUnloaded.store(false);

	AddComponent("transform", new TransformComponent(this, startingPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
	transformComponent = static_cast<TransformComponent*>(GetComponentByName("transform"));
	UseNoise(chunkSectionNoise, minY, maxY);

	TextureData textureData = Texture::LoadTextureDataFromFile("./Assets/textureAtlas.png");
	texture_ = new Texture2D(textureData, GL_TEXTURE_2D, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
	Texture::FreeTextureData(textureData);

	AddComponent("mesh", new MeshComponent(this, new Mesh(texture_)));
	meshComponent = static_cast<MeshComponent*>(GetComponentByName("mesh"));
	TextureAtlas textureAtlas = { 3, 6 };

	grassSubTextures_ = GetSubTexturesOfRowFromTextureAtlas(0, textureAtlas);
	dirtSubTextures_ = GetSubTexturesOfRowFromTextureAtlas(1, textureAtlas);
	stoneSubTextures_ = GetSubTexturesOfRowFromTextureAtlas(2, textureAtlas);

	GenerateMesh();
}

void Chunk::GenerateMesh(bool isOnMainThread)
{

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
						adjacentBlockUp = blocks_[z][x][y+1];
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

					// Add each block face that faces an air block
					if (adjacentBlockUp == BLOCK_TYPE_AIR)
					{
						int subTextureCol = 0;
						SubTexture subTexture;

						switch (currentBlock)
						{
						case BLOCK_TYPE_GRASS:
							subTexture = grassSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_DIRT:
							subTexture = dirtSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_STONE:
							subTexture = stoneSubTextures_.at(subTextureCol);
							break;
						}

						vertices.push_back({ meshX,		meshY + 1,	meshZ, subTexture.startS, subTexture.startT }); // Bottom-Left
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ, subTexture.endS, subTexture.startT }); // Bottom-Right
						vertices.push_back({ meshX,		meshY + 1,	meshZ - 1, subTexture.startS, subTexture.endT }); // Top-left
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ - 1, subTexture.endS, subTexture.endT }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(), 
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
							}
						);
					}

					if (adjacentBlockDown == BLOCK_TYPE_AIR)
					{
						int subTextureCol = 1;

						SubTexture subTexture;

						switch (currentBlock)
						{
						case BLOCK_TYPE_GRASS:
							subTexture = grassSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_DIRT:
							subTexture = dirtSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_STONE:
							subTexture = stoneSubTextures_.at(subTextureCol);
							break;
						}

						vertices.push_back({ meshX,		meshY,		meshZ, subTexture.startS, subTexture.startT }); // Bottom-Left
						vertices.push_back({ meshX + 1,	meshY,		meshZ, subTexture.endS, subTexture.startT }); // Bottom-Right
						vertices.push_back({ meshX,		meshY,		meshZ - 1, subTexture.startS, subTexture.endT }); // Top-left
						vertices.push_back({ meshX + 1,	meshY,		meshZ - 1, subTexture.endS, subTexture.endT }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(),
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
							}
						);
					}

					if (adjacentBlockRight == BLOCK_TYPE_AIR)
					{
						int subTextureCol = 2;

						SubTexture subTexture;

						switch (currentBlock)
						{
						case BLOCK_TYPE_GRASS:
							subTexture = grassSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_DIRT:
							subTexture = dirtSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_STONE:
							subTexture = stoneSubTextures_.at(subTextureCol);
							break;
						}

						vertices.push_back({ meshX + 1,	meshY,		meshZ, subTexture.startS, subTexture.startT }); // Bottom-Left
						vertices.push_back({ meshX + 1,	meshY,		meshZ - 1, subTexture.endS, subTexture.startT }); // Bottom-Right
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ, subTexture.startS, subTexture.endT }); // Top-left
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ - 1, subTexture.endS, subTexture.endT }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(),
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
							}
						);
					}

					if (adjacentBlockLeft == BLOCK_TYPE_AIR)
					{
						int subTextureCol = 3;

						SubTexture subTexture;

						switch (currentBlock)
						{
						case BLOCK_TYPE_GRASS:
							subTexture = grassSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_DIRT:
							subTexture = dirtSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_STONE:
							subTexture = stoneSubTextures_.at(subTextureCol);
							break;
						}

						vertices.push_back({ meshX,		meshY,		meshZ, subTexture.startS, subTexture.startT }); // Bottom-Left
						vertices.push_back({ meshX,		meshY,		meshZ - 1, subTexture.endS, subTexture.startT }); // Bottom-Right
						vertices.push_back({ meshX,		meshY + 1,	meshZ, subTexture.startS, subTexture.endT }); // Top-left
						vertices.push_back({ meshX,		meshY + 1,	meshZ - 1, subTexture.endS, subTexture.endT }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(),
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
							}
						);
					}

					if (adjacentBlockFront == BLOCK_TYPE_AIR)
					{
						int subTextureCol = 4;

						SubTexture subTexture;

						switch (currentBlock)
						{
						case BLOCK_TYPE_GRASS:
							subTexture = grassSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_DIRT:
							subTexture = dirtSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_STONE:
							subTexture = stoneSubTextures_.at(subTextureCol);
							break;
						}

						vertices.push_back({ meshX,		meshY,		meshZ, subTexture.startS, subTexture.startT }); // Bottom-Left
						vertices.push_back({ meshX + 1,	meshY,		meshZ, subTexture.endS, subTexture.startT }); // Bottom-Right
						vertices.push_back({ meshX,		meshY + 1,	meshZ, subTexture.startS, subTexture.endT }); // Top-left
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ, subTexture.endS, subTexture.endT }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(),
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
							}
						);
					}

					if (adjacentBlockBack == BLOCK_TYPE_AIR)
					{
						int subTextureCol = 5;

						SubTexture subTexture;

						switch (currentBlock)
						{
						case BLOCK_TYPE_GRASS:
							subTexture = grassSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_DIRT:
							subTexture = dirtSubTextures_.at(subTextureCol);
							break;
						case BLOCK_TYPE_STONE:
							subTexture = stoneSubTextures_.at(subTextureCol);
							break;
						}

						vertices.push_back({ meshX,		meshY,		meshZ - 1, subTexture.startS, subTexture.startT }); // Bottom-Left
						vertices.push_back({ meshX + 1,	meshY,		meshZ - 1, subTexture.endS, subTexture.startT }); // Bottom-Right
						vertices.push_back({ meshX,		meshY + 1,	meshZ - 1, subTexture.startS, subTexture.endT }); // Top-left
						vertices.push_back({ meshX + 1,	meshY + 1,	meshZ - 1, subTexture.endS, subTexture.endT }); // Top-Right

						unsigned int offsetStart = vertices.size() - 1;

						indices.insert(indices.end(),
							{
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
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
	if (!isUnloaded.load()) {
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

				if ((int)currentY < (int)ySurface)
				{
					currentBlock = BLOCK_TYPE_DIRT;
				}
				else if ((int)currentY > (int)ySurface)
				{
					currentBlock = BLOCK_TYPE_AIR;
				}
				else
				{
					currentBlock = BLOCK_TYPE_GRASS;
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
}

void Chunk::Recreate(std::vector<float> chunkSectionNoise, int minY, int maxY, glm::vec3 newStartingPosition, int seed, bool isOnMainThread)
{
	seed_.store(seed);
	blocks_.clear();
	transformComponent->SetTranslation(newStartingPosition);
	UseNoise(chunkSectionNoise, minY, maxY);
	GenerateMesh(isOnMainThread);
	isUnloaded.store(false);
}