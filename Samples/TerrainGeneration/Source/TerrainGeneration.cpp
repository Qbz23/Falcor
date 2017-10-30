
#include "TerrainGeneration.h"

const std::string TerrainGeneration::mHeightmapNames[kNumHeightmaps] =
{ "Redmond", "Colorado", "Philadelphia", "Arizona", "Maine" };

const Gui::DropdownList TerrainGeneration::kHeightmapDropdown
{
  { 0, mHeightmapNames[0] },
  { 1, mHeightmapNames[1] },
  { 2, mHeightmapNames[2] },
  { 3, mHeightmapNames[3] },
  { 4, mHeightmapNames[4] },
};

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
  mCamController.setCameraSpeed(kInitialCameraSpeed);

  auto rsDesc = RasterizerState::Desc();
  mpDefaultRs = RasterizerState::create(rsDesc);
  rsDesc.setFillMode(RasterizerState::FillMode::Wireframe);
  mpWireframeRs = RasterizerState::create(rsDesc);

  mpState = GraphicsState::create();
  mpState->setProgram(program);
  
  mpState->setFbo(pDefaultFbo);
  CreatePatchVao(kInitialNumRows, kInitialNumCols, kInitialPatchW);
  LoadHeightmaps();

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
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();
}

void TerrainGeneration::onGuiRender(Gui* mpGui)
{
  if (mpGui->beginGroup("Terrain Generation"))
  {
    static float cameraSpeed = kInitialCameraSpeed;
    if (mpGui->addFloatVar("Camera Speed", cameraSpeed, 0.1f))
    {
      mCamController.setCameraSpeed(cameraSpeed);
    }
    static bool isWireframe = false; //initially not in wireframe
    if (mpGui->addCheckBox("Wireframe", isWireframe))
    {
      if (isWireframe)
        mpState->setRasterizerState(mpWireframeRs);
      else
        mpState->setRasterizerState(mpDefaultRs);
    }

    if (mpGui->beginGroup("Patch Geometry"))
    {
      //Maybe these should be stored somewhere else but this is only place used
      static int numRows = kInitialNumRows;
      static int numColumns = kInitialNumCols;
      static float patchWidth = kInitialPatchW;

      mpGui->addIntVar("Num Rows", numRows, 1, kMaxRows);
      mpGui->addIntVar("Num Cols", numColumns, 1, kMaxCols);
      mpGui->addFloatVar("Patch Wdith", patchWidth, kMinPatchW);

      if (mpGui->addButton("Re-generate Patches"))
      {
        mpState->setVao(nullptr);
        CreatePatchVao(numRows, numColumns, patchWidth);
      }
      mpGui->endGroup();
    }

    if (mpGui->beginGroup("Displacement"))
    {
      mpGui->addDropdown("Heightmap", kHeightmapDropdown, mHeightmapIndex);
      mpGui->addFloatVar("MaxHeight", mDsPerFrame.maxHeight, kMinHeight);
      mPsPerFrame.maxHeight = mDsPerFrame.maxHeight;
      mpGui->endGroup();
    }

    if (mpGui->beginGroup("Tessellation"))
    {
      mpGui->addFloatVar("Tess Near Distance", mHsPerFrame.minDistance, kSmallestMinDistance);
      mpGui->addFloatVar("Tess Far Distance", mHsPerFrame.maxDistance, kSmallestMaxDistance);
      mpGui->addIntVar("Near Tess Factor", mHsPerFrame.minTessFactor, kSmallestTessFactor, kLargestTessFactor);
      mpGui->addIntVar("Far Tess Factor", mHsPerFrame.maxTessFactor, kSmallestTessFactor, kLargestTessFactor);
      mpGui->endGroup();
    }

    if (mpGui->beginGroup("Color"))
    {
      mpGui->addFloatVar("Height Color Offset", mPsPerFrame.heightColorOffset);
      mpGui->endGroup();
    }
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
  mpWireframeRs.reset();
  mpDefaultRs.reset();

  for (auto i = 0; i < kNumHeightmaps; ++i)
    mHeightmaps[i].reset();

  mpVars.reset();
  mpState.reset();
}

void TerrainGeneration::CreatePatchVao(int numRows, int numCols, float patchSize)
{
  const float halfW = (numCols * patchSize) / 2.0f;
  const float halfH = (numRows * patchSize) / 2.0f;
  const float deltaU = 1.0f / (numCols - 1);
  const float deltaV = 1.0f / (numRows - 1);

  std::vector<Vertex> verts;
  verts.resize(numRows * numCols);
  for (int i = 0; i < numRows; ++i)
  {
    float zPos = -halfH + patchSize * i;
    for (int j = 0; j < numCols; ++j)
    {
      float xPos = -halfW + patchSize * j;

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
  mHsPerFrame.eyePos = camPos;
  mDsPerFrame.viewProj = mpCamera->getViewProjMatrix();
  mPsPerFrame.eyePos = camPos;

  mpVars->getConstantBuffer("HSPerFrame")->setBlob(&mHsPerFrame, 0, sizeof(HsPerFrame));
  mpVars->getConstantBuffer("DSPerFrame")->setBlob(&mDsPerFrame, 0, sizeof(DsPerFrame));
  mpVars->getConstantBuffer("PSPerFrame")->setBlob(&mPsPerFrame, 0, sizeof(PsPerFrame));
  mpVars->setTexture("gHeightmap", mHeightmaps[mHeightmapIndex]);
}

void TerrainGeneration::LoadHeightmaps()
{
  for (int i = 0; i < kNumHeightmaps; ++i)
  {
    std::string heightmapFile = mHeightmapNames[i] + "Heightmap.png";
    mHeightmaps[i] = createTextureFromFile(heightmapFile, true, false);
  }
}