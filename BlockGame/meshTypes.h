#pragma once
#include <glm/glm.hpp>

#include "shader.h"

/*
 * You need to add the mesh type to here so batching can occur
 * i.e. if you add a pig mesh, it should be added here :-
 */
enum class MeshType
{
	Chunk
};

struct MeshTypeCommonData
{
	glm::vec3 viewPos;
	glm::mat4 view;
	glm::mat4 projection;
	shader shaderProgram;
};