#include "game.h"

#include <sstream>
#include <string>
#include <vector>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "logging.h"
#include "mesh.h"
#include "chunk.h"
#include "entity.h"
#include "meshComponent.h"
#include "transformComponent.h"

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
	debugInfo = DebugInfo();

	float start = -1.0f;
	float end = 1.0f;

	stbi_set_flip_vertically_on_load(false);

	TextureData textureData = Texture::LoadTextureDataFromFile("./Assets/textureAtlas.png");
	Texture2D* meshTexture = new Texture2D(textureData, GL_TEXTURE_2D, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST);
	Texture::FreeTextureData(textureData);

	TextureAtlas textureAtlas = { 3, 6 };
	SubTexture grassSubTexture = GetSubTextureFromTextureAtlas(0, 2, textureAtlas);

	Mesh mesh = Mesh(meshTexture);
	mesh.AddVertex({ start, start, end, grassSubTexture.startS, grassSubTexture.startT });
	mesh.AddVertex({ end, start, end, grassSubTexture.endS, grassSubTexture.startT });
	mesh.AddVertex({ start, end, end, grassSubTexture.startS, grassSubTexture.endT });
	mesh.AddVertex({ end, end, end, grassSubTexture.endS, grassSubTexture.endT });
	mesh.AddFace({
		{ start, start, end, grassSubTexture.startS, grassSubTexture.startT },
        {end, start, end, grassSubTexture.endS, grassSubTexture.startT },
		{ start, end, end, grassSubTexture.startS, grassSubTexture.endT },
		{ start, end, end, grassSubTexture.startS, grassSubTexture.endT },
        { end, end, end, grassSubTexture.endS, grassSubTexture.endT },
		{end, start, end, grassSubTexture.endS, grassSubTexture.startT}
	});

	auto fastNoiseSimplex = FastNoise::New<FastNoise::Simplex>();

	srand(time(NULL));
	int seed = rand();
	Chunk chunk1 = Chunk(fastNoiseSimplex, glm::vec3(0.0f, 0.0f, 0.0f), 16, seed);
	Chunk chunk2 = Chunk(fastNoiseSimplex, glm::vec3(16.0f, 0.0f, 0.0f), 16, seed);

	Entity grassPlane = Entity();
	grassPlane.AddComponent("mesh", new MeshComponent(mesh));
	grassPlane.AddComponent("transform", new TransformComponent(
		glm::vec3(10.0f, 6.0f, 10.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(8.0f, 8.0f, 1.0f)
	));

	grassPlane.Start();
	chunk1.Start();
	chunk2.Start();

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

		TransformComponent* planeTransformComponent = static_cast<TransformComponent*>(grassPlane.GetComponentByName("transform"));

		//TransformComponent* chunkTransformComponent = static_cast<TransformComponent*>(chunk.GetComponentByName("transform"));
		//chunkTransformComponent->RotateY(0.0001f);

		grassPlane.Update();
		chunk1.Update();
		chunk2.Update();
		 
		// Display Game
		ImGui::Render();
		glClearColor(0.0f, 0.3f, 0.5f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		int width, height;
		glfwGetWindowSize(window, &width, &height);

		glm::mat4 perspective = glm::mat4(1.0f);
		perspective = glm::perspective(glm::radians(60.0f), (GLfloat)((float)width / (float)height), 0.1f, 200.0f);

		glm::vec3 cameraTranslation = glm::vec3(0.0f, 0.0f, -5.0f);

		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 18.0f, -32), glm::vec3(8.0f, 8.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(20.0f), glm::vec3(1.0f, 0.3f, 0.5f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

		chunk1.Draw(view, perspective);
		chunk2.Draw(view, perspective);
		MeshComponent* planeMeshComponent = static_cast<MeshComponent*>(grassPlane.GetComponentByName("mesh"));
		planeMeshComponent->Draw(planeTransformComponent->GetModel(), view, perspective);

		glfwSwapBuffers(window);

		debugInfo.EndFrame();
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
	startSecondTime = glfwGetTime();
	fpsMax = 0.0f;
	fpsAverage = 0.0f;
	fpsMin = 0.0f;
	swapInterval = 0;
	glMajorVersion = -1;
	glMinorVersion = -1;
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

		fpsAverage /= fpsCounts.size();
		fpsCounts.clear();
	}
}

void DebugInfo::Display()
{
	ImGui::Begin("Debug Info", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

	std::stringstream glVersion;
	glVersion << "OpenGL Version: " << glMajorVersion << "." << glMinorVersion;

	std::stringstream fpsData;
	fpsData << "FPS Max: " << fpsMax;
	fpsData << "\nFPS Average: " << fpsAverage;
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