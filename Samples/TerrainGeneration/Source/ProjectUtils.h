#pragma once
#include "Falcor.h"

using namespace Falcor;

class ProjectUtils
{
public:
  //Creates one copy of all the things to return
  void onLoad();
  void onShutdown();

  RasterizerState::SharedPtr GetWireframeRS() { return mpWireframeRs; }
  Sampler::SharedPtr GetTrilinearWrapSampler() { return mpTrilinearWrapSampler;  }
  Sampler::SharedPtr GetTrilienarClampSampler() { return mpTrilinearClampSampler;  }
  Vao::SharedPtr GetUnitQuadPatchVao() { return mpUnitQuadPatchVao;  }
  //Pair of vao and index count
  //Actually creates instead of returning already created b/c varies due to params
  std::pair<Vao::SharedPtr, uint32_t> GetGridPatchVao(int numRows, int numCols, float patchSize);
  std::pair<Vao::SharedPtr, uint32_t> GridPatchVaoGui(Gui* pGui, int& numRows, int& numCols, float& patchW);

  //This and helpers based on Ken Perlin's Java Reference Implementation
  float PerlinNoise(vec3 input, int octaves, float amplitude, float frequency = 1.0f);

private:
  struct PatchVertex
  {
    vec4 pos;
    vec2 uv;
  };

  float CalcPerlinNoise(vec3 input);
  float PerlinFade(float x);
  float PerlinGrad(int hash, vec3 input);
  void InitPerlinNoise();
  uint32_t perlinValues[512];

  Buffer::SharedPtr CreatePatchIndexBuffer(int numRows, int numCols);
  Vao::SharedPtr CreateUnitQuadPatchVao();

  RasterizerState::SharedPtr mpWireframeRs;
  Sampler::SharedPtr mpTrilinearWrapSampler;
  Sampler::SharedPtr mpTrilinearClampSampler;
  Vao::SharedPtr mpUnitQuadPatchVao;
};
