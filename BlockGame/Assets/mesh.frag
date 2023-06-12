#version 450

out vec4 fColour;
in vec2 TexCoord;

uniform sampler2D texture1;

void main() {
	fColour = texture(texture1, TexCoord);
}