#include "terrain.h"

Terrain::Terrain(int seed) {
    auto mountainSimplex = FastNoise::New<FastNoise::Simplex>();
    auto mountainFractal = FastNoise::New<FastNoise::FractalFBm>();
    mountainFractal->SetSource(mountainSimplex);
    mountainFractal->SetOctaveCount(5);
    mountainFractal->SetLacunarity(1);

    mountainTerrain_ = FastNoise::New<FastNoise::DomainScale>();
    mountainTerrain_->SetSource(mountainFractal);
    mountainTerrain_->SetScale(0.35f);

    seed_ = seed;
}

std::vector<float> Terrain::GetNoiseForChunk(int x, int z) {
    std::vector<float> mountainNoise (16 * 16);
    mountainTerrain_->GenUniformGrid2D(mountainNoise.data(), x, z, 16, 16, 0.05f, seed_);
   return mountainNoise;
}