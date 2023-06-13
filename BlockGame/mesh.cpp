#include "mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "logging.h"

bool Vertex::operator==(Vertex const& vertex) const
{
	bool isEqual = x == vertex.x && y == vertex.y && z == vertex.z && s == vertex.s && t == vertex.t;
	return isEqual;
}

Mesh::Mesh()
{}

Mesh::Mesh(Texture* texture)
{
	vertices_ = std::vector<Vertex>();
	indices_ = std::vector<unsigned int>();
	texture_ = texture;

	shaderProgram = CreateShader("./Assets/mesh.vert", "./Assets/mesh.frag");

	glUseProgram(shaderProgram);

	// Tell the shader which texture unit to use.
	unsigned int textureLoc = glGetUniformLocation(shaderProgram, "texture1");
	glUniform1i(textureLoc, 0);

	// Create the vao, vbo and ebo
	glCreateVertexArrays(1, &vao_);

	glBindVertexArray(vao_);
	glCreateBuffers(1, &vbo_);
	glCreateBuffers(1, &ebo_);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	// Position of the vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)0);
	// Texture coordinates for the vertex
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

	// Enable the vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	shouldUpdateOnGPU = false;
}

Mesh::~Mesh()
{
	/*
	if (glIsBuffer(vbo_) == GL_TRUE)
		glDeleteBuffers(1, &vbo_);

	if (glIsBuffer(ebo_) == GL_TRUE)
		glDeleteBuffers(1, &ebo_);

	if (glIsVertexArray(vao_))
		glDeleteVertexArrays(1, &vao_);

	if (glIsProgram(shaderProgram))
		glDeleteProgram(shaderProgram);

	vertices_.clear();
	indices_.clear();
	*/
}

void Mesh::AddVertex(const Vertex& vertex)
{
	bool alreadyExists = false;

	for (const auto& conVertex : vertices_)
	{
		if (conVertex == vertex)
		{
			alreadyExists = true;
		}
	}

	if (!alreadyExists) {
		vertices_.push_back(vertex);
		shouldUpdateOnGPU = true;
	}
}


void Mesh::AddFace(std::vector<Vertex> vertices)
{
	for (auto vertex : vertices)
	{
		bool wasFound = false;

		for (int i = 0; i < vertices_.size(); i++)
		{
			if (vertex == vertices_[i] && !wasFound)
			{
				indices_.push_back(i);
				wasFound = true;
			}
		}

		if (!wasFound)
			LOG("Couldn't find vertex point: (%f, %f, %f, %f, %f)\n", vertex.x, vertex.y, vertex.z, vertex.s, vertex.t);
	}

	shouldUpdateOnGPU = true;
}

void Mesh::Draw(glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
	texture_->Bind(GL_TEXTURE0);

	glBindVertexArray(vao_);

	if (shouldUpdateOnGPU)
	{
		glNamedBufferData(vbo_, vertices_.size() * sizeof(Vertex), vertices_.data(), GL_DYNAMIC_DRAW);
		glNamedBufferData(ebo_, indices_.size() * sizeof(unsigned int), indices_.data(), GL_DYNAMIC_DRAW);
		shouldUpdateOnGPU = false;
	}

	glUseProgram(shaderProgram);

	unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
	unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
	unsigned int projectionlLoc = glGetUniformLocation(shaderProgram, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionlLoc, 1, GL_FALSE, glm::value_ptr(projection));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, nullptr);

	texture_->Unbind(GL_TEXTURE0);
}
