#pragma once
#include <FastNoise/FastNoise.h>
#include <glm/glm.hpp>

struct Terrain {
    int seed_;

    FastNoise::SmartNode<FastNoise::DomainScale> mountainTerrain_;

    Terrain() = default;
    Terrain(int seed);

    std::vector<float> GetNoiseForChunk(int x, int z);
};