#include "terrain.h"

Terrain::Terrain(int seed) {
    seed_ = seed;

    auto domainScale1 = FastNoise::New<FastNoise::DomainScale>();
    domainScale1->SetSource(FastNoise::New<FastNoise::Simplex>());
    domainScale1->SetScale(0.58f);

    auto multiply1 = FastNoise::New<FastNoise::Multiply>();
    multiply1->SetLHS(domainScale1);
    multiply1->SetRHS(0.4f);

    auto domainScale2 = FastNoise::New<FastNoise::DomainScale>();
    domainScale2->SetSource(FastNoise::New<FastNoise::Simplex>());
    domainScale2->SetScale(0.66f);

    auto multiply2 = FastNoise::New<FastNoise::Multiply>();
    multiply2->SetLHS(domainScale2);
    multiply2->SetRHS(0.66f);

    auto fade = FastNoise::New<FastNoise::Fade>();
    fade->SetA(multiply1);
    fade->SetB(multiply2);
    fade->SetFade(0.5f);

    auto domainWarpGradient = FastNoise::New<FastNoise::DomainWarpGradient>();
    domainWarpGradient->SetSource(fade);
    domainWarpGradient->SetWarpAmplitude(0.28f);
    domainWarpGradient->SetWarpFrequency(2.06f);

    elevationNoise_ = domainWarpGradient;
}

std::vector<float> Terrain::GetElevationNoiseForChunk(int x, int z) {
    std::vector<float> noise (16 * 16);
    elevationNoise_->GenUniformGrid2D(noise.data(), x, z, 16, 16, 0.011f, seed_);
    return noise;
}