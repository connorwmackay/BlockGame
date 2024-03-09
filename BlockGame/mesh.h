#pragma once
#include <atomic>
#include <unordered_map>
#include <vector>
#include <glad/glad.h>
#include <glm/fwd.hpp>

#include "meshTypes.h"
#include "shader.h"
#include "texture.h"

struct Vertex
{
	float x;
	float y;
	float z;
	float normalX;
	float normalY;
	float normalZ;
	float s;
	float t;
	int textureAtlasZ;

	bool operator==(Vertex const& vertex) const;
};

/*
 * Meshes now require that you use the CommonData functions.
 * This is because that results in batching for mesh types.
 *
 * If you want to create a pig mesh for example, you will
 * need to add it to the MeshTypes enum and also create
 * and use common data for the pig mesh type.
 *
 * While this means more complex code to create a mesh,
 * it results in a significant performance increase, so
 * I have decided to keep it this way.
 *
 * To see an example of how this works, please see game.cpp
 * and look out for MeshType::Chunk.
 */
class Mesh
{
	std::vector<Vertex> vertices_;
	std::vector<unsigned int> indices_;
	Texture2DArray* texture_;

	MeshType type_;

	unsigned int vao_;
	unsigned int vbo_;
	unsigned int ebo_;

	std::atomic<bool> shouldUpdateOnGPU{false};

	static std::unordered_map<MeshType, MeshTypeCommonData> commonData_;
public:
	// Default constructor, this is only to satisfy C++, you shouldn't use this
	// constructor.
	Mesh();

	Mesh(Texture2DArray* texture, const MeshType& type);
	~Mesh();

	/*
	 * Adds a vertex point to the mesh
	 */
	void AddVertex(Vertex vertex, bool onlyAddUnique = false);
	void AddVertices(std::vector<Vertex> vertices);

	/*
	 * Finds the index of each specified vertex point,
	 * then adds them to the mesh as indices. Each set
	 * of three vertices form a triangle. Each face has
	 * two of these triangles.
	 */
	void AddFace(std::vector<Vertex> vertices);
	void AddFace(std::vector<unsigned int> indices);

	void SetVertices(std::vector<Vertex>& vertices);
	void SetIndices(std::vector<unsigned int>& indices);

	int GetNumVertices();

	shader GetShaderProgram();

	/*
	 * Set the matrices
	 */
	void SetModel(glm::mat4 const& model);

	static void StartDrawBatch(const MeshType& type);
	static void EndDrawBatch();

	/*
	 * Draws the mesh to the screen.
	 * IMPORTANT: This doesn't set the shader, you need to
	 * call StartDrawBatch to do that.
	 */
	void Draw(glm::mat4 const& model);

	void Unload();

	std::vector<Vertex>& GetVertices();

	const MeshType& GetMeshType();

	static void CreateCommonData(MeshType type);
	static void SetCommonData(MeshType type, MeshTypeCommonData commonData);
	static const MeshTypeCommonData& GetCommonData(const MeshType& type);
};