#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in int textureAtlasIndex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;
flat out int TextureAtlasIndex;
out vec3 Normal;
out vec3 FragPos;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0f);
	TexCoord = texCoord;
	TextureAtlasIndex = textureAtlasIndex;
	Normal = normal;
	FragPos = vec3(model * vec4(position, 1.0));
}