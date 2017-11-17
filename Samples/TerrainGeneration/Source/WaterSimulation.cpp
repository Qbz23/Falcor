
#include "WaterSimulation.h"

void WaterSimulation::onLoad(const Fbo::SharedPtr& pDefaultFbo)
{
  auto program = GraphicsProgram::createFromFile(
    appendShaderExtension("TerrainGeneration.vs"),
    appendShaderExtension("NoiseWater.ps"),
    "",
    appendShaderExtension("TerrainGeneration.hs"),
    appendShaderExtension("NoiseWater.ds"));
  mNoiseResources.mpVars = GraphicsVars::create(program->getActiveVersion()->getReflector());

  mpCamera = Camera::create();
  mpCamera->setPosition(mInitialCamPos);
  mpCamera->setTarget(mInitialCamTarget);
  mCamController.attachCamera(mpCamera);
  mCamController.setCameraSpeed(kInitialCameraSpeed);

  //Set state properties
  mNoiseResources.mpState = GraphicsState::create();
  mNoiseResources.mpState->setProgram(program);
  mNoiseResources.mpState->setFbo(pDefaultFbo);

  //Get vao
  auto vaoPair = mpUtils->GetGridPatchVao(kInitialNumRows, kInitialNumCols, kInitialPatchW);
  mNoiseResources.mpState->setVao(vaoPair.first);
  mIndexCount = vaoPair.second;

  //Get Sampler
  mNoiseResources.mpVars->setSampler("gSampler", mpUtils->GetTrilinearWrapSampler());

  const int noiseDim = 1024;

  //Noise pass stuf
  mNoiseResources.mpNoiseTex = Texture::create2D(
    noiseDim,
    noiseDim,
    ResourceFormat::R32Float, 
    1u, 
    Resource::kMaxPossible,
    nullptr,
    Resource::BindFlags::ShaderResource | Resource::BindFlags::RenderTarget);
  mNoiseResources.mpNoiseTex->generateMips();
  mNoiseResources.mNoisePass.mpState = GraphicsState::create();
  mNoiseResources.mNoisePass.mpPass = FullScreenPass::create(appendShaderExtension("Noise.ps"));
  mNoiseResources.mNoisePass.mpVars = GraphicsVars::create(
    mNoiseResources.mNoisePass.mpPass->getProgram()->getActiveVersion()->getReflector());
  mNoiseResources.mNoisePass.psPerFrame = 
    mNoiseResources.mNoisePass.mpVars->getConstantBuffer("PsPerFrame");
  mNoiseResources.mNoisePass.mpFbo = Fbo::create();
  mNoiseResources.mNoisePass.mpFbo->attachColorTarget(mNoiseResources.mpNoiseTex, 0);
  mNoiseResources.mNoisePass.mpState->setFbo(mNoiseResources.mNoisePass.mpFbo);

  //Heightfield water stuff
  mHeightResources.mpHeightTex = Texture::create2D(
    mHeightResources.kTextureDimensions,
    mHeightResources.kTextureDimensions,
	  ResourceFormat::R32Float,
	  1u,
	  Resource::kMaxPossible,
	  nullptr,
	  Resource::BindFlags::ShaderResource | Resource::BindFlags::UnorderedAccess);
  mHeightResources.mpFlowTex = Texture::create2D(
    mHeightResources.kTextureDimensions,
    mHeightResources.kTextureDimensions,
	  ResourceFormat::RGBA32Float,
	  1u,
	  Resource::kMaxPossible,
	  nullptr,
	  Resource::BindFlags::ShaderResource | Resource::BindFlags::UnorderedAccess);
  
  ComputeProgram::SharedPtr cs = ComputeProgram::createFromFile("SimulateWater.cs.hlsl");
  mHeightResources.mSimulatePass.mpComputeVars = ComputeVars::create(cs->getActiveVersion()->getReflector());
  mHeightResources.mSimulatePass.mpComputeState = ComputeState::create();
  mHeightResources.mSimulatePass.mpComputeState->setProgram(cs);
}

void WaterSimulation::RenderNoiseTex(RenderContext::SharedPtr pCtx)
{
  mNoiseResources.mNoisePass.psPerFrameData.time += dt * mNoiseResources.mNoisePass.timeScale;
  //Set Vars
  mNoiseResources.mNoisePass.psPerFrame->setBlob(&mNoiseResources.mNoisePass.psPerFrameData, 0, sizeof(mNoiseResources.mNoisePass.psPerFrameData));

  //Render
  pCtx->pushGraphicsState(mNoiseResources.mNoisePass.mpState);
  pCtx->pushGraphicsVars(mNoiseResources.mNoisePass.mpVars);
  mNoiseResources.mNoisePass.mpPass->execute(pCtx.get());
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();

  //Save result
  mNoiseResources.mpNoiseTex = mNoiseResources.mNoisePass.mpFbo->getColorTexture(0);
}

void WaterSimulation::preFrameRender(RenderContext::SharedPtr pCtx)
{
  mCamController.update();

  RenderNoiseTex(pCtx);

  //This should go in its own function. Should be structs. 
  //Maybe a whole new effectsample
  mHeightResources.mSimulatePass.mpComputeVars->setTexture(
    "newFlow", mHeightResources.mpFlowTex);
  pCtx->pushComputeState(mHeightResources.mSimulatePass.mpComputeState);
  pCtx->pushComputeVars(mHeightResources.mSimulatePass.mpComputeVars);
  int numThreadGroups = mHeightResources.kTextureDimensions / mHeightResources.kNumThreadsPerGroup;
  pCtx->dispatch(numThreadGroups, numThreadGroups, 1);
  pCtx->popComputeVars();
  pCtx->popComputeState();

  UpdateVars();
  pCtx->pushGraphicsState(mNoiseResources.mpState);
  pCtx->pushGraphicsVars(mNoiseResources.mpVars);
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
        mNoiseResources.mpState->setRasterizerState(mpUtils->GetWireframeRS());
      else
        mNoiseResources.mpState->setRasterizerState(nullptr); //nullptr is default
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
        mNoiseResources.mpState->setVao(nullptr);
        mNoiseResources.mpState->setVao(vaoPair.first);
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

    if (mpGui->beginGroup("Noise"))
    {
      mpGui->addIntVar("Octaves", mNoiseResources.mNoisePass.psPerFrameData.octaves, 1);
      mpGui->addFloatVar("Amplitude", mNoiseResources.mNoisePass.psPerFrameData.amplitude, 0.001f);
      mpGui->addFloatVar("Initial Frequency", mNoiseResources.mNoisePass.psPerFrameData.initialFreq);
      mpGui->addFloatVar("Time Scale", mNoiseResources.mNoisePass.timeScale);
      mpGui->addIntVar("Input Scale", mNoiseResources.mNoisePass.psPerFrameData.noiseInputScale);
      mpGui->endGroup();
    }
    mpGui->endGroup();
  }
}

bool WaterSimulation::onKeyEvent(const KeyboardEvent& keyEvent)
{
  //Todo, on R, reset camera, on right/left, change map, on up/down, change height
  
  bool handledByCamera = mCamController.onKeyEvent(keyEvent);

  //If key not consumed by camera controller
  if(!handledByCamera)
  {
    switch(keyEvent.key)
    {
      case KeyboardEvent::Key::R:
        mpCamera->setPosition(mInitialCamPos);
        mpCamera->setTarget(mInitialCamTarget);
        return true;
      case KeyboardEvent::Key::Left:
        mNoiseResources.mNoisePass.timeScale *= 0.9f;
        return true;
      case KeyboardEvent::Key::Right:
        mNoiseResources.mNoisePass.timeScale *= 1.1f;
        return true;
      case KeyboardEvent::Key::Down:
        mNoiseResources.mNoisePass.psPerFrameData.amplitude *= 0.9f;
        return true;
      case KeyboardEvent::Key::Up:
        mNoiseResources.mNoisePass.psPerFrameData.amplitude *= 1.1f;
        return true;
      default:
        return false;
    }
  }
  else
    return true;
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
  mNoiseResources.mNoisePass.mpPass.release();
  mNoiseResources.mNoisePass.mpVars.reset();
  mNoiseResources.mNoisePass.mpState.reset();
  mNoiseResources.mNoisePass.psPerFrame.reset();
  mNoiseResources.mNoisePass.mpFbo.reset();

  mNoiseResources.mpNoiseTex.reset();
  mNoiseResources.mpVars.reset();
  mNoiseResources.mpState.reset();
}

void WaterSimulation::UpdateVars()
{
  vec3 camPos = mpCamera->getPosition();
  mHsPerFrame.eyePos = camPos;
  mNoiseResources.mDsPerFrame.viewProj = mpCamera->getViewProjMatrix();
  mNoiseResources.mPsPerFrame.eyePos = camPos;

  mNoiseResources.mpVars->getConstantBuffer("HSPerFrame")->setBlob(&mHsPerFrame, 0, sizeof(HsPerFrame));
  mNoiseResources.mpVars->getConstantBuffer("DSPerFrame")->setBlob(&mNoiseResources.mDsPerFrame, 0, sizeof(NoiseWaterResources::DsPerFrame));
  mNoiseResources.mpVars->getConstantBuffer("PSPerFrame")->setBlob(&mNoiseResources.mPsPerFrame, 0, sizeof(NoiseWaterResources::PsPerFrame));
  mNoiseResources.mpVars->setTexture("gNoiseTex", mNoiseResources.mpNoiseTex);
}
