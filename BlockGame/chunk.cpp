#include "chunk.h"

#include <GLFW/glfw3.h>

#include "logging.h"
#include "meshComponent.h"
#include "transformComponent.h"

Chunk::Chunk(FastNoise::SmartNode<FastNoise::Simplex>& noiseGenerator, glm::vec3 startingPosition, int size, int seed)
	: Entity()
{
	size_ = size;
	blocks_ = std::vector<std::vector<std::vector<uint8_t>>>();
	seed_ = seed;

	AddComponent("transform", new TransformComponent(this, startingPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
	transformComponent = static_cast<TransformComponent*>(GetComponentByName("transform"));
	UseNoise(noiseGenerator);

	TextureData textureData = Texture::LoadTextureDataFromFile("./Assets/textureAtlas.png");
	texture_ = new Texture2D(textureData, GL_TEXTURE_2D, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
	Texture::FreeTextureData(textureData);

	Mesh mesh = Mesh(texture_);

	AddComponent("mesh", new MeshComponent(this, mesh));
	meshComponent = static_cast<MeshComponent*>(GetComponentByName("mesh"));

	GenerateMesh();
}

void Chunk::GenerateMesh()
{
	Mesh* mesh = meshComponent->GetMesh();

	float meshStartX = -1.0f * (size_ / 2.0f);
	float meshStartY = -1.0f * (size_ / 2.0f);
	float meshStartZ = -1.0f * (size_ / 2.0f);

	TextureAtlas textureAtlas = { 3, 6 };

	int numBlocksRendered = 0;

	std::vector<Vertex> faceVertices = std::vector<Vertex>();
	faceVertices.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
	faceVertices.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
	faceVertices.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
	faceVertices.push_back({ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });

	for (int z = 0; z < size_; z++)
	{
		for (int x = 0; x < size_; x++)
		{
			for (int y = 0; y < size_; y++)
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

					int subTextureRow, subTextureCol;
					switch(currentBlock)
					{
					case BLOCK_TYPE_GRASS:
						subTextureRow = 0;
						break;
					case BLOCK_TYPE_DIRT:
						subTextureRow = 1;
						break;
					case BLOCK_TYPE_STONE:
						subTextureRow = 2;
						break;
					}

					// Add each block face that faces an air block
					if (adjacentBlockUp == BLOCK_TYPE_AIR)
					{
						subTextureCol = 0;

						SubTexture subTexture = GetSubTextureFromTextureAtlas(subTextureRow, subTextureCol, textureAtlas);

						faceVertices[0] = { meshX,		meshY + 1,	meshZ, subTexture.startS, subTexture.startT }; // Bottom-Left
						faceVertices[1] = { meshX + 1,	meshY + 1,	meshZ, subTexture.endS, subTexture.startT }; // Bottom-Right
						faceVertices[2] = { meshX,		meshY + 1,	meshZ - 1, subTexture.startS, subTexture.endT }; // Top-left
						faceVertices[3] = { meshX + 1,	meshY + 1,	meshZ - 1, subTexture.endS, subTexture.endT }; // Top-Right

						mesh->AddVertex(faceVertices[0]);
						mesh->AddVertex(faceVertices[1]);
						mesh->AddVertex(faceVertices[2]);
						mesh->AddVertex(faceVertices[3]);

						unsigned int offsetStart = mesh->GetNumVertices() - 1;

						mesh->AddFace({
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
						});
					}

					if (adjacentBlockDown == BLOCK_TYPE_AIR)
					{
						subTextureCol = 1;

						SubTexture subTexture = GetSubTextureFromTextureAtlas(subTextureRow, subTextureCol, textureAtlas);

						faceVertices[0] = { meshX,		meshY,		meshZ, subTexture.startS, subTexture.startT }; // Bottom-Left
						faceVertices[1] = { meshX + 1,	meshY,		meshZ, subTexture.endS, subTexture.startT }; // Bottom-Right
						faceVertices[2] = { meshX,		meshY,		meshZ - 1, subTexture.startS, subTexture.endT }; // Top-left
						faceVertices[3] = { meshX + 1,	meshY,		meshZ - 1, subTexture.endS, subTexture.endT }; // Top-Right

						mesh->AddVertex(faceVertices[0]);
						mesh->AddVertex(faceVertices[1]);
						mesh->AddVertex(faceVertices[2]);
						mesh->AddVertex(faceVertices[3]);

						unsigned int offsetStart = mesh->GetNumVertices() - 1;

						mesh->AddFace({
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
							});
					}

					if (adjacentBlockRight == BLOCK_TYPE_AIR)
					{
						subTextureCol = 2;

						SubTexture subTexture = GetSubTextureFromTextureAtlas(subTextureRow, subTextureCol, textureAtlas);

						faceVertices[0] = { meshX + 1,	meshY,		meshZ, subTexture.startS, subTexture.startT }; // Bottom-Left
						faceVertices[1] = { meshX + 1,	meshY,		meshZ - 1, subTexture.endS, subTexture.startT }; // Bottom-Right
						faceVertices[2] = { meshX + 1,	meshY + 1,	meshZ, subTexture.startS, subTexture.endT }; // Top-left
						faceVertices[3] = { meshX + 1,	meshY + 1,	meshZ - 1, subTexture.endS, subTexture.endT }; // Top-Right

						mesh->AddVertex(faceVertices[0]);
						mesh->AddVertex(faceVertices[1]);
						mesh->AddVertex(faceVertices[2]);
						mesh->AddVertex(faceVertices[3]);

						unsigned int offsetStart = mesh->GetNumVertices() - 1;

						mesh->AddFace({
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
							});
					}

					if (adjacentBlockLeft == BLOCK_TYPE_AIR)
					{
						subTextureCol = 3;

						SubTexture subTexture = GetSubTextureFromTextureAtlas(subTextureRow, subTextureCol, textureAtlas);

						faceVertices[0] = { meshX,		meshY,		meshZ, subTexture.startS, subTexture.startT }; // Bottom-Left
						faceVertices[1] = { meshX,		meshY,		meshZ - 1, subTexture.endS, subTexture.startT }; // Bottom-Right
						faceVertices[2] = { meshX,		meshY + 1,	meshZ, subTexture.startS, subTexture.endT }; // Top-left
						faceVertices[3] = { meshX,		meshY + 1,	meshZ - 1, subTexture.endS, subTexture.endT }; // Top-Right

						mesh->AddVertex(faceVertices[0]);
						mesh->AddVertex(faceVertices[1]);
						mesh->AddVertex(faceVertices[2]);
						mesh->AddVertex(faceVertices[3]);

						unsigned int offsetStart = mesh->GetNumVertices() - 1;

						mesh->AddFace({
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
							});
					}

					if (adjacentBlockFront == BLOCK_TYPE_AIR)
					{
						subTextureCol = 4;

						SubTexture subTexture = GetSubTextureFromTextureAtlas(subTextureRow, subTextureCol, textureAtlas);

						faceVertices[0] = { meshX,		meshY,		meshZ, subTexture.startS, subTexture.startT }; // Bottom-Left
						faceVertices[1] = { meshX + 1,	meshY,		meshZ, subTexture.endS, subTexture.startT }; // Bottom-Right
						faceVertices[2] = { meshX,		meshY + 1,	meshZ, subTexture.startS, subTexture.endT }; // Top-left
						faceVertices[3] = { meshX + 1,	meshY + 1,	meshZ, subTexture.endS, subTexture.endT }; // Top-Right

						mesh->AddVertex(faceVertices[0]);
						mesh->AddVertex(faceVertices[1]);
						mesh->AddVertex(faceVertices[2]);
						mesh->AddVertex(faceVertices[3]);

						unsigned int offsetStart = mesh->GetNumVertices() - 1;

						mesh->AddFace({
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
							});
					}

					if (adjacentBlockBack == BLOCK_TYPE_AIR)
					{
						subTextureCol = 5;

						SubTexture subTexture = GetSubTextureFromTextureAtlas(subTextureRow, subTextureCol, textureAtlas);

						faceVertices[0] = { meshX,		meshY,		meshZ - 1, subTexture.startS, subTexture.startT }; // Bottom-Left
						faceVertices[1] = { meshX + 1,	meshY,		meshZ - 1, subTexture.endS, subTexture.startT }; // Bottom-Right
						faceVertices[2] = { meshX,		meshY + 1,	meshZ - 1, subTexture.startS, subTexture.endT }; // Top-left
						faceVertices[3] = { meshX + 1,	meshY + 1,	meshZ - 1, subTexture.endS, subTexture.endT }; // Top-Right

						mesh->AddVertex(faceVertices[0]);
						mesh->AddVertex(faceVertices[1]);
						mesh->AddVertex(faceVertices[2]);
						mesh->AddVertex(faceVertices[3]);

						unsigned int offsetStart = mesh->GetNumVertices() - 1;

						mesh->AddFace({
							offsetStart - 0,
							offsetStart - 1,
							offsetStart - 2,
							offsetStart - 2,
							offsetStart - 3,
							offsetStart - 1
							});
					}
				}
			}
		}
	}

	meshComponent->SetModel(transformComponent->GetModel());
}

bool Chunk::IsInChunk(int x, int y, int z)
{
	// Ensure the chunk actually has blocks in it.
	if (blocks_.empty())
	{
		LOG("There are no blocks in the chunk: Z\n");
		return false;
	}

	if (blocks_[0].empty())
	{
		LOG("There are no blocks in the chunk: X\n");
		return false;
	}

	if (blocks_[0][0].empty())
	{
		LOG("There are no blocks in the chunk: Y\n");
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
	meshComponent->Draw();
}

void Chunk::UseNoise(FastNoise::SmartNode<FastNoise::Simplex>& noiseGenerator)
{
	glm::vec3 position = static_cast<TransformComponent*>(GetComponentByName("transform"))->GetTranslation();

	// Fill the blocks_ container using noise
	std::vector<float> simplexNoiseOutput(size_ * size_);
	noiseGenerator->GenUniformGrid2D(
		simplexNoiseOutput.data(),
		(int)glm::floor(position.x),
		(int)glm::floor(position.z),
		size_,
		size_,
		0.02f,
		seed_
	);

	float beforeSettingBlocks = glfwGetTime();
	int currentBlockIndex = 0;
	int currentNoiseIndex = 0;
	for (int z = 0; z < size_; z++)
	{
		std::vector<std::vector<uint8_t>> zRow = std::vector<std::vector<uint8_t>>();

		for (int x = 0; x < size_; x++)
		{
			std::vector<uint8_t> xRow = std::vector<uint8_t>();

			float currentNoiseVal = simplexNoiseOutput[currentNoiseIndex];

			int yHeight = 8 + (currentNoiseVal * size_/2);

			for (int y = 0; y < size_; y++)
			{
				uint8_t currentBlock = BLOCK_TYPE_AIR;

				if (y < yHeight && y >= (yHeight-3))
				{
					currentBlock = BLOCK_TYPE_DIRT;
				}
				else if (y < yHeight-3)
				{
					currentBlock = BLOCK_TYPE_STONE;
				}
				else if (y == yHeight)
				{
					currentBlock = BLOCK_TYPE_GRASS;
				}

				xRow.push_back(currentBlock);
				currentBlockIndex++;
			}

			zRow.push_back(xRow);

			currentNoiseIndex++;
		}

		blocks_.push_back(zRow);
	}
}

void Chunk::Unload()
{
	meshComponent->GetMesh()->Unload();
}

void Chunk::Recreate(FastNoise::SmartNode<FastNoise::Simplex>& noiseGenerator, glm::vec3 newStartingPosition, int seed)
{
	seed_ = seed;
	blocks_.clear();
	transformComponent->SetTranslation(newStartingPosition);
	UseNoise(noiseGenerator);
	GenerateMesh();
}