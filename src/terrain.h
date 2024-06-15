#pragma once
#include <FastNoise/FastNoise.h>
#include <glm/glm.hpp>

struct Terrain {
    int seed_;

    FastNoise::SmartNode<FastNoise::DomainWarpGradient> elevationNoise_;

    Terrain() = default;
    Terrain(int seed);

    std::vector<float> GetElevationNoiseForChunk(int x, int z);
};