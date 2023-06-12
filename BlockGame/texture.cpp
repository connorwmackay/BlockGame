#include "texture.h"
#include "logging.h"

Texture::Texture(TextureData textureData, GLenum textureTarget, GLint textureMinFilter, GLint textureMagFilter)
{
	textureTarget_ = textureTarget;
	glCreateTextures(textureTarget, 1, &textureId_);

	if (!glIsTexture(textureId_))
	{
		LOG("Couldn't create OpenGL texture\n");
	}
}

void Texture::Bind(GLenum textureUnit)
{
	glActiveTexture(textureUnit);
	glBindTexture(textureTarget_, textureId_);
}

void Texture::Unbind(GLenum textureUnit)
{
	glActiveTexture(textureUnit);
	glBindTexture(textureTarget_, 0);
}

TextureData Texture::LoadTextureDataFromFile(const char* file)
{
	TextureData textureData{};

	textureData.data = stbi_load(file, &textureData.width, &textureData.height, &textureData.numChannels, 0);

	if (!textureData.data)
	{
		LOG("Couldn't load texture data from %s\n", file);
	}

	return textureData;
}

void Texture::FreeTextureData(TextureData& textureData)
{
	stbi_image_free(textureData.data);
}

GLenum Texture::GetTextureUnitFromInt(int index)
{
	switch(index)
	{
	case 0:
		return GL_TEXTURE0;
	case 1:
		return GL_TEXTURE1;
	case 2:
		return GL_TEXTURE2;
	case 3:
		return GL_TEXTURE3;
	case 4:
		return GL_TEXTURE4;
	case 5:
		return GL_TEXTURE5;
	// TODO: Add branches for the rest of the texture units
	default:
		return GL_TEXTURE31; // The highest texture unit...
	}
}

Texture2D::Texture2D(TextureData textureData, GLenum textureTarget, GLint textureMinFilter, GLint textureMagFilter)
	: Texture(textureData, textureTarget, textureMinFilter, textureMagFilter)
{
	Bind(GL_TEXTURE0);

	glTexParameteri(textureTarget_, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(textureTarget_, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(textureTarget_, GL_TEXTURE_MIN_FILTER, textureMinFilter);
	glTexParameteri(textureTarget_, GL_TEXTURE_MAG_FILTER, textureMagFilter);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureData.width, textureData.height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.data);
	glGenerateMipmap(textureTarget_);
}

SubTexture GetSubTextureFromTextureAtlas(int rowInd, int colInd, TextureAtlas const& textureAtlas)
{
	SubTexture subTexture{};

	if (rowInd < 0 && rowInd >= textureAtlas.numRows)
	{
		LOG("Row Index of %d is not in texture Atlas bounds of %d\n", rowInd, textureAtlas.numRows);
		return subTexture;
	}

	if (colInd < 0 && colInd >= textureAtlas.numCols)
	{
		LOG("Col Index of %d is not in texture Atlas bounds of %d\n", colInd, textureAtlas.numCols);
		return subTexture;
	}

	float widthPerSubTex = 1.0f / static_cast<float>(textureAtlas.numCols);
	float heightPerSubTex = 1.0f / static_cast<float>(textureAtlas.numRows);

	subTexture.startS = static_cast<float>(colInd+1) * widthPerSubTex;
	subTexture.startT = static_cast<float>(rowInd+1) * heightPerSubTex;
	subTexture.endS = static_cast<float>(colInd) * widthPerSubTex;
	subTexture.endT = static_cast<float>(rowInd) * heightPerSubTex;

	return subTexture;
}