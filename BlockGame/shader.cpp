#include "shader.h"

#include <fstream>
#include <sstream>

#include "logging.h"

shader CreateShader(const char* vertexFile, const char* fragmentFile)
{
	shader newShader = glCreateProgram();

	// Load Vertex Shader
	std::ifstream vertexFileStream;
	vertexFileStream.open(vertexFile, std::ifstream::in);

	if (!vertexFileStream.is_open())
	{
		LOG("Couldn't open vertex file.\n");
		return SHADER_ERROR;
	}

	std::stringstream vertexData;
	vertexData << vertexFileStream.rdbuf();

	std::string vertexDataStdStr = vertexData.str();
	const char* vertexDataStr = vertexDataStdStr.c_str();
	LOG("Vertex Data: %s", vertexDataStr);

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexDataStr, NULL);
	glCompileShader(vertexShader);

	// Check for Errors
	int wasSuccessful;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &wasSuccessful);
	if (!wasSuccessful)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		LOG("Vertex Shader Error: %s\n", infoLog);
		return SHADER_ERROR;
	}

	// Load Fragment Shader
	std::ifstream fragmentFileStream;
	fragmentFileStream.open(fragmentFile, std::ifstream::in);

	if (!fragmentFileStream.is_open())
	{
		LOG("Couldn't open fragment file.\n");
		return SHADER_ERROR;
	}

	std::stringstream fragmentData;
	fragmentData << fragmentFileStream.rdbuf();

	std::string fragmentDataStdStr = fragmentData.str();
	const char* fragmentDataStr = fragmentDataStdStr.c_str();
	LOG("Fragment Data: %s\n", fragmentDataStr);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentDataStr, NULL);
	glCompileShader(fragmentShader);

	// Check for Errors
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &wasSuccessful);
	if (!wasSuccessful)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		LOG("Vertex Shader Error: %s", infoLog);
		return SHADER_ERROR;
	}

	// Attach Shaders to Shader Program
	glAttachShader(newShader, vertexShader);
	glAttachShader(newShader, fragmentShader);

	// Link Program and Check for Program Errors
	glLinkProgram(newShader);

	glGetProgramiv(newShader, GL_LINK_STATUS, &wasSuccessful);
	if (!wasSuccessful)
	{
		glGetProgramInfoLog(newShader, 512, NULL, infoLog);
		LOG("Shader Linking Error: %s", infoLog);
		return SHADER_ERROR;
	}

	// Delete Vertex and Fragment Shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	LOG("Is Program: %d", glIsProgram(newShader));
	
	// Return created shader
	return newShader;
}
