#pragma once
#include <thread>
#include <vector>

// TODO: Implement this.
class Worker
{
	std::vector<std::thread> threadPool_;

public:
	Worker(int numThreads) = default;
};
