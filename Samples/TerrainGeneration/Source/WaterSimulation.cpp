
#include "WaterSimulation.h"

void WaterSimulation::onLoad(const Fbo::SharedPtr& pDefaultFbo)
{
  auto program = GraphicsProgram::createFromFile(
    appendShaderExtension("TerrainGeneration.vs"),
    appendShaderExtension("NoiseWater.ps"),
    "",
    appendShaderExtension("TerrainGeneration.hs"),
    appendShaderExtension("NoiseWater.ds"));
  mpVars = GraphicsVars::create(program->getActiveVersion()->getReflector());

  mpCamera = Camera::create();
  mpCamera->setPosition(mInitialCamPos);
  mpCamera->setTarget(mInitialCamTarget);
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
  mpNoiseTex = Texture::create2D(
    noiseDim,
    noiseDim,
    ResourceFormat::R32Float, 
    1u, 
    Resource::kMaxPossible,
    nullptr,
    Resource::BindFlags::ShaderResource | Resource::BindFlags::RenderTarget);
  mpNoiseTex->generateMips();
  mNoisePass.mpState = GraphicsState::create();
  mNoisePass.mpPass = FullScreenPass::create(appendShaderExtension("Noise.ps"));
  mNoisePass.mpVars = GraphicsVars::create(
    mNoisePass.mpPass->getProgram()->getActiveVersion()->getReflector());
  mNoisePass.psPerFrame = mNoisePass.mpVars->getConstantBuffer("PsPerFrame");
  mNoisePass.mpFbo = Fbo::create();
  mNoisePass.mpFbo->attachColorTarget(mpNoiseTex, 0);
  mNoisePass.mpState->setFbo(mNoisePass.mpFbo);

  //Heightfield water stuff
  mpWaterHeightTex = Texture::create2D(
	  noiseDim,
	  noiseDim,
	  ResourceFormat::R32Float,
	  1u,
	  Resource::kMaxPossible,
	  nullptr,
	  Resource::BindFlags::ShaderResource | Resource::BindFlags::UnorderedAccess);
  mpWaterFlowTex = Texture::create2D(
	  noiseDim,
	  noiseDim,
	  ResourceFormat::RGBA32Float,
	  1u,
	  Resource::kMaxPossible,
	  nullptr,
	  Resource::BindFlags::ShaderResource | Resource::BindFlags::UnorderedAccess);
  
  ComputeProgram::SharedPtr cs = ComputeProgram::createFromFile("SimulateWater.cs.hlsl");
  mpComputeVars = ComputeVars::create(cs->getActiveVersion()->getReflector());
  mpComputeState = ComputeState::create();
  mpComputeState->setProgram(cs);
}

void WaterSimulation::RenderNoiseTex(RenderContext::SharedPtr pCtx)
{
  mNoisePass.psPerFrameData.time += dt * mNoisePass.timeScale;
  //Set Vars
  mNoisePass.psPerFrame->setBlob(&mNoisePass.psPerFrameData, 0, sizeof(NoisePsPerFrame));

  //Render
  pCtx->pushGraphicsState(mNoisePass.mpState);
  pCtx->pushGraphicsVars(mNoisePass.mpVars);
  mNoisePass.mpPass->execute(pCtx.get());
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();

  //Save result
  mpNoiseTex = mNoisePass.mpFbo->getColorTexture(0);
}

void WaterSimulation::preFrameRender(RenderContext::SharedPtr pCtx)
{
  mCamController.update();

  RenderNoiseTex(pCtx);

  //This should go in its own function. Should be structs. 
  //Maybe a whole new effectsample
  mpComputeVars->setTexture("newFlow", mpWaterFlowTex);
  pCtx->pushComputeState(mpComputeState);
  pCtx->pushComputeVars(mpComputeVars);
  pCtx->dispatch(1024/32, 1024 / 32, 1);
  pCtx->popComputeVars();
  pCtx->popComputeState();

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

    if (mpGui->beginGroup("Noise"))
    {
      mpGui->addIntVar("Octaves", mNoisePass.psPerFrameData.octaves, 1);
      mpGui->addFloatVar("Amplitude", mNoisePass.psPerFrameData.amplitude, 0.001f);
      mpGui->addFloatVar("Initial Frequency", mNoisePass.psPerFrameData.initialFreq);
      mpGui->addFloatVar("Time Scale", mNoisePass.timeScale);
      mpGui->addIntVar("Input Scale", mNoisePass.psPerFrameData.noiseInputScale);
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
        mNoisePass.timeScale *= 0.9f;
        return true;
      case KeyboardEvent::Key::Right:
        mNoisePass.timeScale *= 1.1f;
        return true;
      case KeyboardEvent::Key::Down:
        mNoisePass.psPerFrameData.amplitude *= 0.9f;
        return true;
      case KeyboardEvent::Key::Up:
        mNoisePass.psPerFrameData.amplitude *= 1.1f;
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
  mNoisePass.mpPass.release();
  mNoisePass.mpVars.reset();
  mNoisePass.mpState.reset();
  mNoisePass.psPerFrame.reset();
  mNoisePass.mpFbo.reset();

  mpNoiseTex.reset();
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
  mpVars->setTexture("gNoiseTex", mpWaterFlowTex);

}
