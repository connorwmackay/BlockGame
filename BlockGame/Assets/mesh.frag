#version 450
#extension GL_EXT_texture_array : enable

out vec4 fColour;
in vec2 TexCoord;
flat in int TextureAtlasIndex;

uniform sampler2DArray texture1;

void main() {
	fColour = texture2DArray(texture1, vec3(TexCoord, TextureAtlasIndex));
}