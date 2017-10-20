
#include "TerrainGeneration.h"

void TerrainGeneration::onLoad(const Fbo::SharedPtr& pDefaultFbo)
{
  mpState = GraphicsState::create();

  CreatePatch(vec2(1024, 1024));
}

void TerrainGeneration::preFrameRender(RenderContext::SharedPtr pCtx)
{
}

void TerrainGeneration::onFrameRender(RenderContext::SharedPtr pCtx)
{
}

void TerrainGeneration::onGuiRender(Gui* mpGui)
{
  if (mpGui->beginGroup("Terrain Generation"))
  {
    mpGui->endGroup();
  }
}

bool TerrainGeneration::onKeyEvent(const KeyboardEvent& keyEvent)
{
  return true;
}

void TerrainGeneration::onShutdown()
{
  //Not 100% sure why I need to do this. Maybe b/c of the 
  //subproject effect sample setup? But if i dont do this,
  //there are live objects on exit
  mpVars.reset();
  mpState.reset();
}

void TerrainGeneration::CreatePatch(vec2 heightmapDimensions)
{
  CreatePatchVAO(heightmapDimensions);
}

void TerrainGeneration::CreatePatchVAO(vec2 heightmapDimensions)
{
  static const uint32_t sCellsPerPatch = 64;

  //How many patches there should be
  int numRows = (int)(((heightmapDimensions.y - 1) / sCellsPerPatch) + 1);
  int numCols = (int)(((heightmapDimensions.x - 1) / sCellsPerPatch) + 1);

  std::vector<Vertex> verts;
  verts.resize(numRows * numCols);

  float patchW = heightmapDimensions.x / (numCols - 1);
  float patchH = heightmapDimensions.y / (numRows - 1);
  float halfW = heightmapDimensions.x / 2.0f;
  float halfH = heightmapDimensions.y / 2.0f;
  float deltaU = heightmapDimensions.x / numCols;
  float deltaV = heightmapDimensions.y / numRows;

  for (int i = 0; i < numRows; ++i)
  {
    float zPos = -halfH + patchH * i;
    for (int j = 0; j < numCols; ++j)
    {
      float xPos = -halfW + patchW * j;

      Vertex newVert;
      newVert.uv = vec2(j * deltaU, i * deltaV);
      newVert.pos = vec4(xPos, 0, zPos, 1.0f);

      verts[numCols * i + j] = newVert;
    }
  }

  //Get IB
  auto ib = CreatePatchIndexBuffer(numRows, numCols);

  //create VB
  const uint32_t vbSize = (uint32_t)(sizeof(Vertex)*arraysize(verts.data()));
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
  auto vao = Vao::create(Vao::Topology::Patch4, pLayout, buffers, 
    ib, Falcor::ResourceFormat::R32Uint);
  //Set it into graphics state
  mpState->setVao(vao);
}

Buffer::SharedPtr TerrainGeneration::CreatePatchIndexBuffer(int numRows, int numCols)
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
      //bot left
      indices[k + 2] = (i + 1) * numCols + j;
      //bot right
      indices[k + 3] = indices[k + 2] + 1;

      k += 4;
    }
  }

  return Buffer::create(
    indices.size() * sizeof(uint32_t),
    Buffer::BindFlags::Index,
    Buffer::CpuAccess::None,
    indices.data());
}