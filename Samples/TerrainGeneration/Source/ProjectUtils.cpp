#include "ProjectUtils.h"

void ProjectUtils::onLoad()
{
  //Wireframe RS
  RasterizerState::Desc rsDesc = RasterizerState::Desc();
  rsDesc.setFillMode(RasterizerState::FillMode::Wireframe);
  mpWireframeRs = RasterizerState::create(rsDesc);

  //Trilinear Wrap Sampler
  Sampler::Desc samplerDesc;
  samplerDesc.setFilterMode(
    Sampler::Filter::Linear,
    Sampler::Filter::Linear,
    Sampler::Filter::Linear);
  samplerDesc.setAddressingMode(
    Sampler::AddressMode::Wrap,
    Sampler::AddressMode::Wrap,
    Sampler::AddressMode::Wrap);
  mpTrilinearWrapSampler = Sampler::create(samplerDesc);
  //Trilinear clamp sampler
  samplerDesc.setAddressingMode(
    Sampler::AddressMode::Clamp,
    Sampler::AddressMode::Clamp,
    Sampler::AddressMode::Clamp);
  mpTrilinearClampSampler = Sampler::create(samplerDesc);

  //Quad vao
  mpUnitQuadPatchVao = CreateUnitQuadPatchVao();

  InitPerlinNoise();
}

void ProjectUtils::onShutdown()
{

}

std::pair<Vao::SharedPtr, uint32_t> ProjectUtils::GetGridPatchVao(int numRows, int numCols, float patchSize)
{
  const float halfW = (numCols * patchSize) / 2.0f;
  const float halfH = (numRows * patchSize) / 2.0f;
  const float deltaU = 1.0f / (numCols - 1);
  const float deltaV = 1.0f / (numRows - 1);

  std::vector<PatchVertex> verts;
  verts.resize(numRows * numCols);
  for (int i = 0; i < numRows; ++i)
  {
    float zPos = -halfH + patchSize * i;
    for (int j = 0; j < numCols; ++j)
    {
      float xPos = -halfW + patchSize * j;

      PatchVertex newVert;
      newVert.uv = vec2(j * deltaU, i * deltaV);
      newVert.pos = vec4(xPos, 0, zPos, 1.0f);

      verts[numCols * i + j] = newVert;
    }
  }

  //Get IB
  auto ib = CreatePatchIndexBuffer(numRows, numCols);

  //create VB
  const uint32_t vbSize = (uint32_t)(sizeof(PatchVertex)*verts.size());
  auto vertexBuffer = Buffer::create(vbSize, Buffer::BindFlags::Vertex,
    Buffer::CpuAccess::Write, (void*)verts.data());

  //create input layout
  VertexLayout::SharedPtr pLayout = VertexLayout::create();
  VertexBufferLayout::SharedPtr pBufLayout = VertexBufferLayout::create();
  pBufLayout->addElement("POSITION", 0, ResourceFormat::RGBA32Float, 1, 0);
  pBufLayout->addElement("TEXCOORD", 16, ResourceFormat::RG32Float, 1, 1);
  pLayout->addBufferLayout(0, pBufLayout);

  //create vao
  Vao::BufferVec buffers{ vertexBuffer };
  //Return pair of vao and index count
  return std::make_pair(
    Vao::create(Vao::Topology::Patch4, pLayout, buffers, ib, Falcor::ResourceFormat::R32Uint),
    4 * (numRows - 1) * (numCols - 1));
}

Buffer::SharedPtr ProjectUtils::CreatePatchIndexBuffer(int numRows, int numCols)
{
  int numFaces = (numRows - 1) * (numCols - 1);
  std::vector<uint32_t> indices;
  indices.resize(4 * numFaces);

  //index buffer index
  int k = 0;
  for (int i = 0; i < numRows - 1; ++i)
  {
    for (int j = 0; j < numCols - 1; ++j)
    {
      //top left
      indices[k] = i * numCols + j;
      //top right
      indices[k + 1] = indices[k] + 1;
      //bot right
      indices[k + 3] = (i + 1) * numCols + j + 1;
      //bot left
      indices[k + 2] = (i + 1) * numCols + j;

      k += 4;
    }
  }

  return Buffer::create(
    4 * numFaces * sizeof(uint32_t),
    Buffer::BindFlags::Index,
    Buffer::CpuAccess::None,
    indices.data());
}

Vao::SharedPtr ProjectUtils::CreateUnitQuadPatchVao()
{
  //Just in NDC. Not doing any transformation
  const PatchVertex kQuadVertices[] =
  {	
    { glm::vec4(-1, 1, 0, 1), glm::vec2(0, 0) },
    { glm::vec4(-1, -1, 0, 1), glm::vec2(0, 1) },
    { glm::vec4(1, 1, 0, 1), glm::vec2(1, 0) },
    { glm::vec4(1, -1, 0, 1), glm::vec2(1, 1) },
  };

  //create VB
  const uint32_t vbSize = (uint32_t)(sizeof(PatchVertex)*arraysize(kQuadVertices));
  auto vertexBuffer = Buffer::create(vbSize, Buffer::BindFlags::Vertex,
    Buffer::CpuAccess::Write, (void*)kQuadVertices);

  //create input layout
  VertexLayout::SharedPtr pLayout = VertexLayout::create();
  VertexBufferLayout::SharedPtr pBufLayout = VertexBufferLayout::create();
  pBufLayout->addElement("POSITION", 0, ResourceFormat::RGBA32Float, 1, 0);
  pBufLayout->addElement("TEXCOORD", 16, ResourceFormat::RG32Float, 1, 1);
  pLayout->addBufferLayout(0, pBufLayout);

  //create vao
  Vao::BufferVec buffers{ vertexBuffer };
  return Vao::create(Vao::Topology::Patch4, pLayout, buffers);
}

std::pair<Vao::SharedPtr, uint32_t> ProjectUtils::GridPatchVaoGui(Gui* pGui, int& numRows, int& numCols, float& patchW)
{
  static const int kMaxRows = 256;
  static const float kMinPatchW = 0.1f;

  pGui->addIntVar("Num Rows", numRows, 1, kMaxRows);
  pGui->addIntVar("Num Cols", numCols, 1, kMaxRows);
  pGui->addFloatVar("Patch Wdith", patchW, kMinPatchW);

  if (pGui->addButton("Re-generate Patches"))
  {
    auto vaoPair = GetGridPatchVao(numRows, numCols, patchW);
    return vaoPair;
  }
  else
    return std::make_pair(nullptr, 0);

}

float ProjectUtils::PerlinNoise(vec3 input, int octaves, float amplitude, 
  float frequency /* = 1.0f */)
{
  float total = 0.0f;
  for(int i = 0; i < octaves; ++i)
  {
    vec3 adjustedInput = input * frequency;
    total += CalcPerlinNoise(adjustedInput) * amplitude;
    frequency *= 2;
  }

  return total / octaves; 
}

float ProjectUtils::PerlinFade(float x)
{
  return x * x * x * (x * (x * 6 - 15) + 10);
}

float ProjectUtils::PerlinGrad(int hash, vec3 input)
{
  //The last 4 bits of the hash represent 12 gradient directions
  int h = hash & 15; //Get last 4 bits of the hash
  //Sets properties of the gradient based on which bits in the hash are set.
  float u = h < 8 ? input.x : input.y; 
  float v = h < 4 ? input.y : h == 12 || h == 14 ? input.x : input.z;
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float ProjectUtils::CalcPerlinNoise(vec3 input)
{
  //Find coords of unit cube that contains this input point
  int xCube = (int)floor(input.x) & 255;
  int yCube = (int)floor(input.y) & 255;
  int zCube = (int)floor(input.z) & 255;
  //Find the relative position within this unit cube
  float x = fract(input.x);
  float y = fract(input.y);
  float z = fract(input.z);
  //Compute Fade curves for xyz
  float u = PerlinFade(x);
  float v = PerlinFade(y);
  float w = PerlinFade(z);
  //Hash coordiunates of cube corners
  int A = perlinValues[xCube] + yCube;
  int AA = perlinValues[A] + zCube;
  int AB = perlinValues[A + 1] + zCube;
  int B = perlinValues[xCube + 1] + yCube;
  int BA = perlinValues[B] + zCube;
  int BB = perlinValues[B + 1] + zCube;

  //Return blended results from the 8 corners of the cube 
  float corner1 = PerlinGrad(perlinValues[AA], vec3(x, y, z));
  float corner2 = PerlinGrad(perlinValues[BA], vec3(x - 1, y, z));
  float corner3 = PerlinGrad(perlinValues[AB], vec3(x, y - 1, z));
  float corner4 = PerlinGrad(perlinValues[BB], vec3(x - 1, y - 1, z));
  float corner5 = PerlinGrad(perlinValues[AA + 1], vec3(x, y, z - 1));
  float corner6 = PerlinGrad(perlinValues[BA + 1], vec3(x - 1, y, z - 1));
  float corner7 = PerlinGrad(perlinValues[AB + 1], vec3(x, y - 1, z - 1));
  float corner8 = PerlinGrad(perlinValues[BB + 1], vec3(x - 1, y - 1, z - 1));
  float xLerpResult1 = lerp(corner1, corner2, u);
  float xLerpResult2 = lerp(corner3, corner4, u);
  float xLerpResult3 = lerp(corner5, corner6, u);
  float xLerpResult4 = lerp(corner7, corner8, u);
  float yLerpResult1 = lerp(xLerpResult1, xLerpResult2, v);
  float yLerpResult2 = lerp(xLerpResult3, xLerpResult4, v);
  return lerp(yLerpResult1, yLerpResult2, w);
}

void ProjectUtils::InitPerlinNoise()
{
  //Taken from Ken Perlin's Java Reference Implementation
  uint32_t permutation[256] = { 151,160,137,91,90,15,
    131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
    190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
    88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
    77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
    102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
    135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
    5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
    223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
    129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
    251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
    49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
  };

  for(uint32_t i = 0; i < 256; ++i)
  {
    perlinValues[256 + i] = perlinValues[i] = permutation[i];
  }
}