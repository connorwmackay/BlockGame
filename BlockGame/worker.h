#pragma once
#include <functional>
#include <mutex>
#include <thread>
#include <vector>
#include <glm/vec3.hpp>

#include "chunk.h"

class World;

typedef std::function<bool(World*, int, int, int, int, std::vector<glm::vec3>, std::vector<Chunk*>)> RecreateChunksFunc;

struct RecreateChunksFuncWithArgs
{
	RecreateChunksFunc func;
	World* world;
	int startX;
	int endX;
	int startZ;
	int endZ;
	std::vector<glm::vec3> loadedChunkPositions;
	std::vector<Chunk*> unloadedChunks;
};

struct ThreadData
{
	std::thread* t;
	bool shouldDie = false;
	bool isAvailable = true;
};

class WorldWorker
{
	std::vector<ThreadData> threadPool_;
	std::vector<RecreateChunksFuncWithArgs> queue_;
	std::mutex queueLock;
public:
	WorldWorker(int numThreads);
	~WorldWorker();

	bool AreAnyThreadsAvailable();
	void QueueFunction(RecreateChunksFuncWithArgs funcWithArgs);

	void RunThread(int index);
};
