#pragma once
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class World;

struct DebugInfo
{
	double startSecondTime;
	double startFrameTime;
	double endFrameTime;

	double startUpdateFrameTime;
	double endUpdateFrameTime;

	double startRenderFrameTime;
	double endRenderFrameTime;

	double fpsMax;
	double fpsAverage;
	double fpsMin;

	double averageFrameTime;
	double averageUpdateFrameTime;
	double averageRenderFrameTime;

	std::vector<double> fpsCounts;
	std::vector<double> frameTimes;
	std::vector<double> updateFrameTimes;
	std::vector<double> renderFrameTimes;

	int glMajorVersion;
	int glMinorVersion;
	int swapInterval;

	DebugInfo();
	void Display(const glm::vec3& playerPos, World* world);
	void StartFrame();
	void EndFrame();
	void StartRender();
	void EndRender();
	void StartUpdate();
	void EndUpdate();

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