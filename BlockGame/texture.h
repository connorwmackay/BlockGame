#pragma once
#include <glad/glad.h>
#include <stb_image.h>

struct TextureData
{
	int width, height;
	int numChannels;
	unsigned char* data;
};

/*
 * This class is intended to be an interface that
 * other classes can inherit from and build upon.
 *
 * i.e Texture2D and TextureArray2D (which would both
 * inherit from this class).
 */
class Texture
{
protected:
	// The id of the texture assigned by OpenGL
	unsigned int textureId_;

	// The target of the texture i.e GL_TEXTURE_2D
	GLenum textureTarget_;
	
public:
	Texture(TextureData textureData, GLenum textureTarget, GLint textureMinFilter, GLint textureMagFilter);
	virtual ~Texture() = default;

	static TextureData LoadTextureDataFromFile(const char* file);
	static void FreeTextureData(TextureData& textureData);
	static GLenum GetTextureUnitFromInt(int index);

	// Binds the texture
	void Bind(GLenum TextureUnit);

	// Unbinds the texture
	void Unbind(GLenum TextureUnit);
};

class Texture2D : public Texture
{
public:
	Texture2D(TextureData textureData, GLenum textureTarget, GLint textureMinFilter, GLint textureMagFilter);
};

struct TextureAtlas
{
	int numRows;
	int numCols;
};

struct SubTexture
{
	float startS;
	float startT;
	float endS;
	float endT;
};

SubTexture GetSubTextureFromTextureAtlas(int rowInd, int colInd, TextureAtlas const& textureAtlas);