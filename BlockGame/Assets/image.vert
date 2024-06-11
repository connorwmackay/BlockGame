#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 textureCoords;

uniform mat4 model;
uniform mat4 projection;

out vec2 TexCoord;

void main() {
	gl_Position = projection * model * vec4(position, 1.0f);
	TexCoord = textureCoords;
}