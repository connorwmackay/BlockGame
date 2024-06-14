#pragma once
#include <glad/gl.h>

typedef unsigned int shader;
const int SHADER_ERROR = -1;

shader CreateShader(const char* vertexFile, const char* fragmentFile);