#include "mesh.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "logging.h"

std::unordered_map<MeshType, MeshTypeCommonData> Mesh::commonData_;

bool Vertex::operator==(Vertex const& vertex) const
{
	bool isEqual = x == vertex.x && y == vertex.y && z == vertex.z &&
				   normalX == vertex.normalX && normalY == vertex.normalY && normalZ == vertex.normalZ &&
		           s == vertex.s && t == vertex.t && vertex.textureAtlasZ == textureAtlasZ;
	return isEqual;
}

Mesh::Mesh()
{}

Mesh::Mesh(Texture2DArray* texture, const MeshType& type)
{
	vertices_ = std::vector<Vertex>();
	indices_ = std::vector<unsigned int>();
	texture_ = texture;
	type_ = type;

	glUseProgram(commonData_.at(type_).shaderProgram);

	// Tell the shader which texture unit to use.
	unsigned int textureLoc = glGetUniformLocation(commonData_.at(type_).shaderProgram, "texture1");
	glUniform1i(textureLoc, 0);

	// Create the vao, vbo and ebo
	glCreateVertexArrays(1, &vao_);

	glBindVertexArray(vao_);
	glCreateBuffers(1, &vbo_);
	glCreateBuffers(1, &ebo_);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	// Position of the vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)0);
	// Normal vector for the vertex
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
	// Texture coordinates for the vertex
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
	// Texture Atlas Index
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(Vertex), (void*)(8 * sizeof(float)));

	// Enable the vertex attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	shouldUpdateOnGPU.store(false);
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

void Mesh::AddVertex(Vertex vertex, bool onlyAddUnique)
{
	bool alreadyExists = false;

	if (onlyAddUnique) {
		for (const auto& conVertex : vertices_)
		{
			if (conVertex == vertex)
			{
				alreadyExists = true;
			}
		}
	}

	if (!alreadyExists) {
		vertices_.push_back(vertex);
		shouldUpdateOnGPU.store(true);
	}
}

void Mesh::AddVertices(std::vector<Vertex> vertices)
{
	vertices_.insert(vertices_.end(), vertices.begin(), vertices.end());
	shouldUpdateOnGPU.store(true);
}

void Mesh::AddFace(std::vector<Vertex> vertices)
{
	for (auto const& vertex : vertices)
	{
		bool wasFound = false;

		for (int i = 0; i < vertices_.size(); i++)
		{
			if (vertex == vertices_.at(i) && !wasFound)
			{
				indices_.push_back(i);
				wasFound = true;
			}
		}

		if (!wasFound)
			LOG("Couldn't find vertex point: (%f, %f, %f, %f, %f)\n", vertex.x, vertex.y, vertex.z, vertex.s, vertex.t);
	}

	shouldUpdateOnGPU.store(true);
}

void Mesh::SetVertices(std::vector<Vertex>& vertices)
{
	vertices_ = vertices;
	shouldUpdateOnGPU.store(true);
}

void Mesh::SetIndices(std::vector<unsigned int>& indices)
{
	indices_ = indices;
	shouldUpdateOnGPU.store(true);
}

void Mesh::AddFace(std::vector<unsigned int> indices)
{
	indices_.insert(indices_.end(), indices.begin(), indices.end());
}

int Mesh::GetNumVertices()
{
	return vertices_.size();
}

void Mesh::SetModel(glm::mat4 const& model)
{
	shader shaderProgram = commonData_.at(type_).shaderProgram;
	glUseProgram(shaderProgram);
	unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
}

void Mesh::StartDrawBatch(const MeshType& type)
{
	const MeshTypeCommonData& commonData = commonData_.at(type);
	glUseProgram(commonData.shaderProgram);
}

void Mesh::EndDrawBatch()
{
	glUseProgram(0);
}

void Mesh::Draw(glm::mat4 const& model)
{
	// Since I use batching, this needs to be specified :-
	unsigned int textureLoc = glGetUniformLocation(commonData_.at(type_).shaderProgram, "texture1");
	glUniform1i(textureLoc, 0);

	SetModel(model);

	texture_->Bind(GL_TEXTURE0);

	glBindVertexArray(vao_);

	if (shouldUpdateOnGPU.load())
	{
		glNamedBufferData(vbo_, vertices_.size() * sizeof(Vertex), vertices_.data(), GL_DYNAMIC_DRAW);
		glNamedBufferData(ebo_, indices_.size() * sizeof(unsigned int), indices_.data(), GL_DYNAMIC_DRAW);

		shouldUpdateOnGPU.store(false);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, nullptr);

	texture_->Unbind(GL_TEXTURE0);
}

void Mesh::Unload()
{
	glNamedBufferData(vbo_, vertices_.size() * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
	glNamedBufferData(ebo_, indices_.size() * sizeof(unsigned int), NULL, GL_DYNAMIC_DRAW);
	vertices_.clear();
	indices_.clear();
}

std::vector<Vertex>& Mesh::GetVertices()
{
	return vertices_;
}

const MeshType& Mesh::GetMeshType()
{
	return type_;
}

void Mesh::CreateCommonData(MeshType type)
{
	MeshTypeCommonData commonData{};
	commonData.shaderProgram = CreateShader("./Assets/mesh.vert", "./Assets/mesh.frag");

	SetCommonData(type, commonData);
}

void Mesh::SetCommonData(MeshType type, MeshTypeCommonData commonData)
{
	shader shaderProgram = commonData.shaderProgram;
	glUseProgram(shaderProgram);

	// Update View Position (For Lighting in Fragment Shader)
	GLuint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
	glUniform3f(viewPosLoc, commonData.viewPos.x, commonData.viewPos.y, commonData.viewPos.z);


	// Update View Matrix (For Vertex Shader)
	unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(commonData.view));

	// Update Projection Matrix (For Vertex Shader)
	unsigned int projectionlLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projectionlLoc, 1, GL_FALSE, glm::value_ptr(commonData.projection));

	// Pass Data to the Static HashMap
	commonData_.insert_or_assign(type, commonData);
}

const MeshTypeCommonData& Mesh::GetCommonData(const MeshType& type)
{
	return commonData_.at(type);
}

shader Mesh::GetShaderProgram()
{
	return commonData_.at(type_).shaderProgram;
}