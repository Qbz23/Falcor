
#include "TerrainGeneration.h"

void TerrainGeneration::onLoad(const Fbo::SharedPtr& pDefaultFbo)
{
  auto program = GraphicsProgram::createFromFile(
    appendShaderExtension("TerrainGeneration.vs"),
    appendShaderExtension("TerrainGeneration.ps"),
    "",
    appendShaderExtension("TerrainGeneration.hs"),
    appendShaderExtension("TerrainGeneration.ds"));
  mpVars = GraphicsVars::create(program->getActiveVersion()->getReflector());

  mpCamera = Camera::create();
  mCamController.attachCamera(mpCamera);

  auto rsDesc = RasterizerState::Desc();
  rsDesc.setFillMode(RasterizerState::FillMode::Wireframe);
  auto rs = RasterizerState::create(rsDesc);

  mpState = GraphicsState::create();
  mpState->setProgram(program);
  //mpState->setRasterizerState(rs);
  mpState->setFbo(pDefaultFbo);
  CreatePatch(vec2(256, 256));

  mpHeightmap = createTextureFromFile("RedmondHeightMap.png", true, false);

  //Trilinear Wrap Sampler
  Sampler::Desc samplerDesc;
  samplerDesc.setFilterMode(
    Sampler::Filter::Linear,
    Sampler::Filter::Linear,
    Sampler::Filter::Linear);
  samplerDesc.setAddressingMode(
    Sampler::AddressMode::Clamp,
    Sampler::AddressMode::Clamp,
    Sampler::AddressMode::Clamp);
  mpVars->setSampler("gSampler", Sampler::create(samplerDesc));
}

void TerrainGeneration::preFrameRender(RenderContext::SharedPtr pCtx)
{
  mCamController.update();

  UpdateVars();
  pCtx->pushGraphicsState(mpState);
  pCtx->pushGraphicsVars(mpVars);
}

void TerrainGeneration::onFrameRender(RenderContext::SharedPtr pCtx)
{
  pCtx->drawIndexed(mIndexCount, 0, 0);
  //pCtx->draw(1024, 0);
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();
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
  return mCamController.onKeyEvent(keyEvent);
}

bool TerrainGeneration::onMouseEvent(const MouseEvent& mouseEvent)
{
  return mCamController.onMouseEvent(mouseEvent);
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
  //Probably hook this up to a ui eventually
  static const int numRows = 32;
  static const int numCols = 32;
  static const float patchW = 4.0f;
  static const float patchH = 4.0f;
  static const float halfW = (numCols * patchW) / 2.0f;
  static const float halfH = (numRows * patchH) / 2.0f;
  static const float deltaU = 1.0f / (numCols - 1);
  static const float deltaV = 1.0f / (numRows - 1);

  std::vector<Vertex> verts;
  verts.resize(numRows * numCols);
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
  const uint32_t vbSize = (uint32_t)(sizeof(Vertex)*verts.size());
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
  //auto vao = Vao::create(Vao::Topology::Patch4, pLayout, buffers);
  //Set it into graphics state
  mpState->setVao(vao);
}

Buffer::SharedPtr TerrainGeneration::CreatePatchIndexBuffer(int numRows, int numCols)
{
  int numFaces = (numRows - 1) * (numCols - 1);
  std::vector<uint32_t> indices;
  mIndexCount = 4 * numFaces;
  indices.resize(mIndexCount);

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
    mIndexCount * sizeof(uint32_t),
    Buffer::BindFlags::Index,
    Buffer::CpuAccess::None,
    indices.data());
}

void TerrainGeneration::UpdateVars()
{
  vec3 camPos = mpCamera->getPosition();
  glm::mat4 viewProj = mpCamera->getViewProjMatrix();
  mpVars->getConstantBuffer("HSPerFrame")->setBlob(&camPos, 0, sizeof(glm::vec3));
  mpVars->getConstantBuffer("PSPerFrame")->setBlob(&camPos, 0, sizeof(glm::vec3));
  mpVars->getConstantBuffer("DSPerFrame")->setBlob(&viewProj, 0, sizeof(glm::mat4));
  mpVars->setTexture("gHeightmap", mpHeightmap);
}