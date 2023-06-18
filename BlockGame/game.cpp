#include "game.h"

#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "logging.h"
#include "mesh.h"
#include "chunk.h"
#include "entity.h"
#include "freeFormController.h"
#include "meshComponent.h"
#include "transformComponent.h"
#include "world.h"

void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	if (severity != 0x9147 && severity != 0x826b) {
		LOG("GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
	}
}

Game::Game()
{
	if (glfwInit() != GLFW_TRUE)
	{
		LOG("Couldn't initialise GLFW\n");
		return;
	}

	window = glfwCreateWindow(1280, 720, "Block Game", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		LOG("Couldn't load GLAD\n");
		return;
	}

	glViewport(0, 0, 1280, 720);
	glfwSetFramebufferSizeCallback(window, ResizeViewportCallback);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = "#version 330";
	ImGui_ImplOpenGL3_Init(glsl_version);

	glEnable(GL_DEBUG_OUTPUT);

	glEnable(GL_DEPTH_TEST);
	glDebugMessageCallback(MessageCallback, 0);
}

void Game::Run()
{
	glClearColor(0.0f, 0.3f, 0.5f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);

	debugInfo = DebugInfo();

	float start = -1.0f;
	float end = 1.0f;

	stbi_set_flip_vertically_on_load(false);

	srand(time(NULL));
	int seed = rand();

	glm::mat4 perspective = glm::mat4(1.0f);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	perspective = glm::perspective(glm::radians(60.0f), (GLfloat)((float)width / (float)height), 0.1f, 400.0f);

	World world = World(glm::vec3(0.0f, 0.0f, 0.0f), 5);

	glm::mat4 oldView = glm::mat4(1.0f);
	for (Chunk* chunk : world.GetWorld())
	{
		chunk->Start();
		MeshComponent* meshComponent = static_cast<MeshComponent*>(chunk->GetComponentByName("mesh"));
		meshComponent->SetProjection(perspective);
		meshComponent->SetView(oldView);
	}

	FreeFormController freeFormController = FreeFormController(window, glm::vec3(0.0f, 16.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	freeFormController.Start();

	TransformComponent* freeFormTransform = static_cast<TransformComponent*>(freeFormController.GetComponentByName("transform"));

	std::thread regenerateThread;
	while (!glfwWindowShouldClose(window))
	{
		debugInfo.StartFrame();

		// Handle Input
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		// Update Game
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		debugInfo.Display();

		world.Update(freeFormTransform->GetTranslation());

		for (Chunk* chunk : world.GetWorld())
		{
			chunk->Update();
		}

		freeFormController.Update();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		glClearColor(0.0f, 0.3f, 0.5f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		CameraComponent* cameraComponent = static_cast<CameraComponent*>(freeFormController.GetComponentByName("camera"));
		TransformComponent* transformComponent = static_cast<TransformComponent*>(freeFormController.GetComponentByName("transform"));
		glm::mat4 view = cameraComponent->GetView(transformComponent);

		bool shouldUpdateViews = false;
		if (view != oldView)
		{
			shouldUpdateViews = true;
		}

		for (auto chunk : world.GetWorld())
		{
			if (shouldUpdateViews)
			{
				MeshComponent* chunkMeshComponent = static_cast<MeshComponent*>(chunk->GetComponentByName("mesh"));
				chunkMeshComponent->SetView(view);
			}

			chunk->Draw();
		}

		// Display Game
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);

		debugInfo.EndFrame();

		oldView = view;
	}
}

Game::~Game()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void ResizeViewportCallback(GLFWwindow* window, int width, int height)
{
	if (width > 0 && height > 0) {
		glViewport(0, 0, width, height);
	}
}

DebugInfo::DebugInfo()
{
	fpsCounts = std::vector<double>();
	frameTimes = std::vector<double>();
	startSecondTime = glfwGetTime();
	fpsMax = 0.0f;
	fpsAverage = 0.0f;
	fpsMin = 0.0f;
	swapInterval = 0;
	glMajorVersion = -1;
	glMinorVersion = -1;
	averageFrameTime = 0.0f;
}

void DebugInfo::StartFrame()
{
	if (glMajorVersion == -1 || glMinorVersion == -1)
	{
		glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
		glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
	}

	startFrameTime = glfwGetTime();
}

void DebugInfo::EndFrame()
{
	endFrameTime = glfwGetTime();
	double timeDifference = endFrameTime - startFrameTime;
	frameTimes.push_back(endFrameTime - startFrameTime);
	fpsCounts.push_back(1 / timeDifference);

	if ((endFrameTime - startSecondTime) >= 0.1f)
	{
		startSecondTime = glfwGetTime();
		fpsMin = fpsCounts[0];
		fpsMax = fpsCounts[0];

		for (double fpsCount : fpsCounts)
		{
			fpsAverage += fpsCount;

			if (fpsCount < fpsMin)
			{
				fpsMin = fpsCount;
			}

			if (fpsCount > fpsMax) {
				fpsMax = fpsCount;
			}
		}

		for (double frameTime : frameTimes)
		{
			averageFrameTime += frameTime;
		}

		fpsAverage /= fpsCounts.size();
		averageFrameTime /= frameTimes.size();
		fpsCounts.clear();
		frameTimes.clear();
	}
}

void DebugInfo::Display()
{
	ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

	std::stringstream glVersion;
	glVersion << "OpenGL Version: " << glMajorVersion << "." << glMinorVersion;

	std::stringstream fpsData;
	fpsData << "Frame Time Avg.: " << averageFrameTime * 1000.0f << "ms";
	fpsData << "\nFPS Max: " << fpsMax;
	fpsData << "\nFPS Avg.: " << fpsAverage;
	fpsData << "\nFPS Min: " << fpsMin;

	std::stringstream vsync;
	if (swapInterval == 0)
	{
		vsync << "VSync: Off";
	}
	else
	{
		vsync << "VSync: On";
	}

	ImGui::Text(glVersion.str().c_str());
	ImGui::Text(fpsData.str().c_str());
	ImGui::Text(vsync.str().c_str());
	ImGui::End();
}