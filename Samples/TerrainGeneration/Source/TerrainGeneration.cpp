
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

  //Set state properties
  mpState = GraphicsState::create();
  mpState->setProgram(program);  
  mpState->setFbo(pDefaultFbo);

  //Get vao
  auto vaoPair = mpUtils->GetGridPatchVao(kInitialNumRows, kInitialNumCols, kInitialPatchW);
  mpState->setVao(vaoPair.first);
  mIndexCount = vaoPair.second;  LoadHeightmaps();

  //Get Sampler
  mpVars->setSampler("gSampler", mpUtils->GetTrilienarClampSampler());
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
        mpState->setRasterizerState(mpUtils->GetWireframeRS());
      else
       mpState->setRasterizerState(nullptr); //nullptr is default
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
        auto vaoPair = mpUtils->GetGridPatchVao(numRows, numColumns, patchWidth);
        mpState->setVao(vaoPair.first);
        mIndexCount = vaoPair.second;
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
  //Todo, on R, reset camera, on right/left, change map, on up/down, change height
  return mCamController.onKeyEvent(keyEvent);
}

bool TerrainGeneration::onMouseEvent(const MouseEvent& mouseEvent)
{
  return mCamController.onMouseEvent(mouseEvent);
}

void TerrainGeneration::onShutdown()
{
  //TODO CHECK. Shit I think I just need a virtual dtor to not 
  //need to do this. Derived dtor never being called and releasing 
  //references to shared ptrs

  //Not 100% sure why I need to do this. Maybe b/c of the 
  //subproject effect sample setup? But if i dont do this,
  //there are live objects on exit
  for (auto i = 0; i < kNumHeightmaps; ++i)
    mHeightmaps[i].reset();

  mpVars.reset();
  mpState.reset();
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