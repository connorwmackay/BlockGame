#pragma once
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct DebugInfo
{
	double startSecondTime;
	double startFrameTime;
	double endFrameTime;

	double fpsMax;
	double fpsAverage;
	double fpsMin;

	std::vector<double> fpsCounts;

	int glMajorVersion;
	int glMinorVersion;
	int swapInterval;

	DebugInfo();
	void Display();
	void StartFrame();
	void EndFrame();
};

struct Game
{
	GLFWwindow* window;
	
	DebugInfo debugInfo;

	Game();
	void Run();
	~Game();
};

void ResizeViewportCallback(GLFWwindow* window, int width, int height);