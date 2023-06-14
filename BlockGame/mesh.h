#pragma once
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

	bool operator==(Vertex const& vertex) const;
};

class Mesh
{
	std::vector<Vertex> vertices_;
	std::vector<unsigned int> indices_;
	Texture* texture_;

	unsigned int vao_;
	unsigned int vbo_;
	unsigned int ebo_;

	shader shaderProgram;

	bool shouldUpdateOnGPU;
public:
	// Default constructor, this is only to satisfy C++, you shouldn't use this
	// constructor.
	Mesh();

	Mesh(Texture* texture);
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

	int GetNumVertices();

	/*
	 * Draws the mesh to the screen.
	 */
	void Draw(glm::mat4 const& model, glm::mat4 const& view, glm::mat4 const& projection);
};