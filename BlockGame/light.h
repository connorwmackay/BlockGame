#pragma once
#include <glm/vec3.hpp>
#include "shader.h"

struct DirectionalLight
{
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

inline void PassDirectionalLightToShader(shader shaderProgram, DirectionalLight light)
{
	glUseProgram(shaderProgram);
	GLint directionLoc = glGetUniformLocation(shaderProgram, "directionalLight.direction");
	GLint ambientLoc = glGetUniformLocation(shaderProgram, "directionalLight.ambient");
	GLint diffuseLoc = glGetUniformLocation(shaderProgram, "directionalLight.diffuse");
	GLint specularLoc = glGetUniformLocation(shaderProgram, "directionalLight.specular");

	glUniform3f(directionLoc, light.direction.x, light.direction.y, light.direction.z);
	glUniform3f(ambientLoc, light.ambient.x, light.ambient.y, light.ambient.z);
	glUniform3f(diffuseLoc, light.diffuse.x, light.diffuse.y, light.diffuse.z);
	glUniform3f(specularLoc, light.specular.x, light.specular.y, light.specular.z);
}