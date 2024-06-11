#include "image.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Image::Image(const char* imageFile)
{
	TextureData textureData = Texture2D::LoadTextureDataFromFile(imageFile, 4);
	texture_ = new Texture2D(textureData, GL_TEXTURE_2D, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	// Pass the Vertex Data
	ImageVertex vertices[6] = {
		{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f} }, // TL
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // BL
		{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // BR

		{{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f} }, // TL
		{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f} }, // TR
		{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // BR
	};

	glCreateVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glCreateBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ImageVertex) * 6, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ImageVertex), (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImageVertex), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Create the Shader
	imageShader_ = CreateShader("./Assets/image.vert", "./Assets/image.frag");

	glUseProgram(imageShader_);
	glUniform1i(glGetAttribLocation(imageShader_, "imageTexture"), 0);
	glUseProgram(0);

	if (textureData.data)
		free(textureData.data);
}

void Image::Draw(glm::mat4 orthoProjection, glm::vec2 pos, glm::vec2 rotation, glm::vec2 scale)
{
	glUseProgram(imageShader_);
	glBindVertexArray(vao_);
	texture_->Bind(GL_TEXTURE0);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(pos.x, pos.y, -1.0f));
	model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(scale, 1.0f));

	glUniformMatrix4fv(glGetUniformLocation(imageShader_, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProjection));
	glUniformMatrix4fv(glGetUniformLocation(imageShader_, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glDrawArrays(GL_TRIANGLES, 0, 6);

	texture_->Unbind(GL_TEXTURE0);
	glBindVertexArray(0);
	glUseProgram(0);
}