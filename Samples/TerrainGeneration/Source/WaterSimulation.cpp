
#include "WaterSimulation.h"

void WaterSimulation::onLoad(const Fbo::SharedPtr& pDefaultFbo)
{
  auto program = GraphicsProgram::createFromFile(
    appendShaderExtension("TerrainGeneration.vs"),
    appendShaderExtension("TerrainGeneration.ps"),
    "",
    appendShaderExtension("TerrainGeneration.hs"),
    appendShaderExtension("NoiseWater.ds"));
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
  mIndexCount = vaoPair.second;

  //Get Sampler
  mpVars->setSampler("gSampler", mpUtils->GetTrilinearWrapSampler());

  const int noiseDim = 1024;

  //Noise pass stuf
  mNoiseTex = Texture::create2D(
    noiseDim,
    noiseDim,
    ResourceFormat::R32Float, 
    1u, 
    Resource::kMaxPossible,
    nullptr,
    Resource::BindFlags::ShaderResource | Resource::BindFlags::RenderTarget);
  mNoiseTex->generateMips();
  mNoisePass.mpState = GraphicsState::create();
  mNoisePass.mpPass = FullScreenPass::create(appendShaderExtension("Noise.ps"));
  mNoisePass.mpVars = GraphicsVars::create(
    mNoisePass.mpPass->getProgram()->getActiveVersion()->getReflector());
  mNoisePass.psPerFrame = mNoisePass.mpVars->getConstantBuffer("PsPerFrame");
  mNoisePass.mpFbo = Fbo::create();
  mNoisePass.mpFbo->attachColorTarget(mNoiseTex, 0);
  mNoisePass.mpState->setFbo(mNoisePass.mpFbo);
}

void WaterSimulation::RenderNoiseTex(RenderContext::SharedPtr pCtx)
{
  //Just hardcode as if 60 fps for now
  mNoisePass.psPerFrameData.time += 0.00016f;
  //Set Vars
  mNoisePass.psPerFrame->setBlob(&mNoisePass.psPerFrameData, 0, sizeof(NoisePsPerFrame));

  //Render
  pCtx->pushGraphicsState(mNoisePass.mpState);
  pCtx->pushGraphicsVars(mNoisePass.mpVars);
  mNoisePass.mpPass->execute(pCtx.get());
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();

  //Save result
  mNoiseTex = mNoisePass.mpFbo->getColorTexture(0);
}

void WaterSimulation::preFrameRender(RenderContext::SharedPtr pCtx)
{
  mCamController.update();

  RenderNoiseTex(pCtx);

  UpdateVars();
  pCtx->pushGraphicsState(mpState);
  pCtx->pushGraphicsVars(mpVars);
}

void WaterSimulation::onFrameRender(RenderContext::SharedPtr pCtx)
{
  pCtx->drawIndexed(mIndexCount, 0, 0);
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();
}

void WaterSimulation::onGuiRender(Gui* mpGui)
{
  if (mpGui->beginGroup("Water Simulation"))
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
      //Now its used elsewhere, but should water have its own version of this?
      static int numRows = kInitialNumRows;
      static int numColumns = kInitialNumCols;
      static float patchWidth = kInitialPatchW;

      auto vaoPair = mpUtils->GridPatchVaoGui(mpGui, numRows, numColumns, patchWidth);

      if (vaoPair.first != nullptr)
      {
        mpState->setVao(nullptr);
        mpState->setVao(vaoPair.first);
        mIndexCount = vaoPair.second;
      }
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

bool WaterSimulation::onKeyEvent(const KeyboardEvent& keyEvent)
{
  //Todo, on R, reset camera, on right/left, change map, on up/down, change height
  return mCamController.onKeyEvent(keyEvent);
}

bool WaterSimulation::onMouseEvent(const MouseEvent& mouseEvent)
{
  return mCamController.onMouseEvent(mouseEvent);
}

void WaterSimulation::onShutdown()
{
  //TODO CHECK. Shit I think I just need a virtual dtor to not 
  //need to do this. Derived dtor never being called and releasing 
  //references to shared ptrs;

  mpVars.reset();
  mpState.reset();
}

void WaterSimulation::UpdateVars()
{
  vec3 camPos = mpCamera->getPosition();
  mHsPerFrame.eyePos = camPos;
  mDsPerFrame.viewProj = mpCamera->getViewProjMatrix();
  mPsPerFrame.eyePos = camPos;

  mpVars->getConstantBuffer("HSPerFrame")->setBlob(&mHsPerFrame, 0, sizeof(HsPerFrame));
  mpVars->getConstantBuffer("DSPerFrame")->setBlob(&mDsPerFrame, 0, sizeof(DsPerFrame));
  mpVars->getConstantBuffer("PSPerFrame")->setBlob(&mPsPerFrame, 0, sizeof(PsPerFrame));
  mpVars->setTexture("gNoiseTex", mNoiseTex);
}
