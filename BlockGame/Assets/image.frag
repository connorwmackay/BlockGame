#version 450

out vec4 fColour;
in vec2 TexCoord;

uniform sampler2D imageTexture;

void main() {
	fColour = texture(imageTexture, TexCoord);
}