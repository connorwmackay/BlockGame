#pragma once
#include <atomic>
#include <vector>
#include <glad/glad.h>
#include <glm/fwd.hpp>

#include "shader.h"
#include "texture.h"

struct Vertex
{
	float x;
	float y;
	float z;
	float s;
	float t;
	int textureAtlasZ;

	bool operator==(Vertex const& vertex) const;
};

class Mesh
{
	std::vector<Vertex> vertices_;
	std::vector<unsigned int> indices_;
	Texture2DArray* texture_;

	unsigned int vao_;
	unsigned int vbo_;
	unsigned int ebo_;

	shader shaderProgram;

	std::atomic<bool> shouldUpdateOnGPU{false};
public:
	// Default constructor, this is only to satisfy C++, you shouldn't use this
	// constructor.
	Mesh();

	Mesh(Texture2DArray* texture);
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

	/*
	 * Set the matrices
	 */
	void SetModel(glm::mat4 const& model);
	void SetView(glm::mat4 const& view);
	void SetProjection(glm::mat4 const& projection);

	/*
	 * Draws the mesh to the screen.
	 */
	void Draw();

	void Unload();

	std::vector<Vertex>& GetVertices();
};