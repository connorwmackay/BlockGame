#include "worker.h"
#include "world.h"

WorldWorker::WorldWorker(int numThreads)
{
	threadPool_ = std::vector<ThreadData>();
	queue_ = std::vector<RecreateChunksFuncWithArgs>();

	for (int i = 0; i < numThreads; i++) {
		ThreadData threadData = {
			new std::thread(&WorldWorker::RunThread, this, i),
			false,
			true
		};
		threadData.t->detach();
		threadPool_.push_back(threadData);
	}
}

WorldWorker::~WorldWorker()
{
	for (ThreadData& threadData : threadPool_)
	{
		threadData.shouldDie = true;
	}
}

void WorldWorker::QueueFunction(RecreateChunksFuncWithArgs funcWithArgs)
{
	queue_.push_back(funcWithArgs);
}


void WorldWorker::RunThread(int index)
{
	ThreadData& threadData = threadPool_.at(index);

	while (!threadData.shouldDie)
	{
		if (!queue_.empty())
		{
			threadData.isAvailable = false;
			RecreateChunksFuncWithArgs funcWithArgs;
			queueLock.lock();
			funcWithArgs = queue_.at(0);
			queue_.erase(queue_.begin());
			queueLock.unlock();

			funcWithArgs.func(
				funcWithArgs.world,
				funcWithArgs.startX,
				funcWithArgs.endX,
				funcWithArgs.startZ,
				funcWithArgs.endZ,
				funcWithArgs.loadedChunkPositions,
				funcWithArgs.unloadedChunks
			);
			threadData.isAvailable = true;
		}
	}
}

bool WorldWorker::AreAnyThreadsAvailable() {
	bool isAvailable = false;

	for (ThreadData& threadData : threadPool_) {
		if (threadData.isAvailable) {
			isAvailable = true;
		}
	}

	return isAvailable;
}