#pragma once
#include <glm/glm.hpp>

#include "shader.h"
#include "texture.h"

struct ImageVertex
{
	glm::vec3 position;
	glm::vec2 textureCoords;
};

class Image
{
public:
	Image(const char* imageFile);

	void Draw(glm::mat4 orthoProjection, glm::vec2 pos, glm::vec2 rotation, glm::vec2 scale);
protected:
	GLuint vao_, vbo_;
	shader imageShader_;
	Texture2D* texture_;
};