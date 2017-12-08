
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
  //Heightmap
  mHeightResources.mpHeightTex = Texture::create2D(
    mHeightResources.kTextureDimensions,
    mHeightResources.kTextureDimensions,
	  ResourceFormat::R32Float,
	  1u,
	  1u,
	  nullptr,
	  Resource::BindFlags::ShaderResource | Resource::BindFlags::UnorderedAccess);

  //Initialize zero flow
  mHeightResources.zeroFlow.resize(
    mHeightResources.kTextureDimensions * mHeightResources.kTextureDimensions, vec4(0, 0, 0, 0));
  //Flow
  mHeightResources.mpFlowTex = Texture::create2D(
    mHeightResources.kTextureDimensions,
    mHeightResources.kTextureDimensions,
    ResourceFormat::RGBA32Float,
    1u,
    1u,
    mHeightResources.zeroFlow.data(),
    Resource::BindFlags::ShaderResource | Resource::BindFlags::UnorderedAccess);

  ComputeProgram::SharedPtr cs = ComputeProgram::createFromFile("SimulateWater.cs.hlsl");
  mHeightResources.mSimulatePass.mpComputeVars = ComputeVars::create(cs->getActiveVersion()->getReflector());
  mHeightResources.mSimulatePass.mpComputeState = ComputeState::create();
  mHeightResources.mSimulatePass.mpComputeState->setProgram(cs);

  auto heightProgram = GraphicsProgram::createFromFile(
    appendShaderExtension("TerrainGeneration.vs"),
    appendShaderExtension("HeightWater.ps"),
    "",
    appendShaderExtension("TerrainGeneration.hs"),
    appendShaderExtension("HeightWater.ds"));
  mHeightResources.mpVars = GraphicsVars::create(heightProgram->getActiveVersion()->getReflector());

  mHeightResources.mpState = GraphicsState::create();
  mHeightResources.mpState->setProgram(heightProgram);
  mHeightResources.mpState->setVao(vaoPair.first);

  mHeightResources.mpVars->setSampler("gSampler", mpUtils->GetTrilienarClampSampler());
}

void WaterSimulation::RenderNoiseTex(RenderContext::SharedPtr pCtx, Texture::SharedPtr dstTex)
{
  mNoiseResources.mNoisePass.psPerFrameData.time += dt * timeScale;
  //Set Vars
  mNoiseResources.mNoisePass.psPerFrame->setBlob(&mNoiseResources.mNoisePass.psPerFrameData, 0, sizeof(mNoiseResources.mNoisePass.psPerFrameData));

  //Render
  pCtx->pushGraphicsState(mNoiseResources.mNoisePass.mpState);
  pCtx->pushGraphicsVars(mNoiseResources.mNoisePass.mpVars);
  mNoiseResources.mNoisePass.mpPass->execute(pCtx.get());
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();

  //Save result
  //It's giving me a warning about the state not being correct but copyresource does resource barriers 
  //and it clearly is functioning... so?
  pCtx->copyResource(dstTex.get(), mNoiseResources.mNoisePass.mpFbo->getColorTexture(0).get()); 
}

void WaterSimulation::SimulateHeightWater(RenderContext::SharedPtr pCtx)
{
  mHeightResources.mSimulatePass.mpComputeVars->setTexture(
    "flowTex", mHeightResources.mpFlowTex);
  mHeightResources.mSimulatePass.mpComputeVars->setTexture(
    "heightTex", mHeightResources.mpHeightTex);

  //need a constant dt. I can kinda understand why framerate spikes mess stuff up, 
  //but I feel like that's wrong and shouldn't completely destroy the simulation, 
  //but I don't have time to investigate further this late in the semester
  float actualDt = 0.00016f * timeScale;
  mHeightResources.mCsPerFrame.dt = actualDt;
  auto cb = mHeightResources.mSimulatePass.mpComputeVars->getConstantBuffer("CsPerFrame");
  cb->setBlob(&mHeightResources.mCsPerFrame, 0, sizeof(HeightWaterResources::CsPerFrame));

  pCtx->pushComputeState(mHeightResources.mSimulatePass.mpComputeState);
  pCtx->pushComputeVars(mHeightResources.mSimulatePass.mpComputeVars);
  int numThreadGroups = mHeightResources.kTextureDimensions / mHeightResources.kNumThreadsPerGroup;
  pCtx->dispatch(numThreadGroups, numThreadGroups, 1);
  pCtx->popComputeVars();
  pCtx->popComputeState();
}

void WaterSimulation::preFrameRender(RenderContext::SharedPtr pCtx)
{
  mCamController.update();

  if(isInNoiseMode)
    RenderNoiseTex(pCtx, mNoiseResources.mpNoiseTex);
  else 
  {
    if(!mHeightResources.generatedFirstHeight)
    {
      pCtx->updateTexture(mHeightResources.mpFlowTex.get(), mHeightResources.zeroFlow.data());
      RenderNoiseTex(pCtx, mHeightResources.mpHeightTex);
      mHeightResources.generatedFirstHeight = true;
    }
    SimulateHeightWater(pCtx);
  }

  UpdateVars();

  if (isInNoiseMode)
  {
    pCtx->pushGraphicsState(mNoiseResources.mpState);
    pCtx->pushGraphicsVars(mNoiseResources.mpVars);
  }
  else
  {
    pCtx->pushGraphicsState(mHeightResources.mpState);
    pCtx->pushGraphicsVars(mHeightResources.mpVars);
  }
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
      {
        mNoiseResources.mpState->setRasterizerState(mpUtils->GetWireframeRS());
        mHeightResources.mpState->setRasterizerState(mpUtils->GetWireframeRS());
      }
      else
      {
        mNoiseResources.mpState->setRasterizerState(nullptr); //nullptr is default
        mHeightResources.mpState->setRasterizerState(nullptr);
      }
    }

    mpGui->addCheckBox("Noise Mode", isInNoiseMode);
    mpGui->addFloatVar("Time Scale", timeScale);
    if (mpGui->beginGroup("Noise"))
    {
      if (!isInNoiseMode)
      {
        if(mpGui->addButton("Reset Initial State (Generate Noise)"))
        {
          mHeightResources.generatedFirstHeight = false;
        }
      }
      mpGui->addIntVar("Octaves", mNoiseResources.mNoisePass.psPerFrameData.octaves, 1);
      mpGui->addFloatVar("Amplitude", mNoiseResources.mNoisePass.psPerFrameData.amplitude, 0.001f);
      mpGui->addFloatVar("Initial Frequency", mNoiseResources.mNoisePass.psPerFrameData.initialFreq);
      mpGui->addIntVar("Input Scale", mNoiseResources.mNoisePass.psPerFrameData.noiseInputScale);
      mpGui->endGroup();
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
        mHeightResources.mpState->setVao(nullptr);
        mHeightResources.mpState->setVao(vaoPair.first);
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
        timeScale *= 0.9f;
        return true;
      case KeyboardEvent::Key::Right:
        timeScale *= 1.1f;
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
  mNoiseResources.mNoisePass.mpPass.release();
  mNoiseResources.mNoisePass.mpVars.reset();
  mNoiseResources.mNoisePass.mpState.reset();
  mNoiseResources.mNoisePass.psPerFrame.reset();
  mNoiseResources.mNoisePass.mpFbo.reset();
  mNoiseResources.mpVars.reset();
  mNoiseResources.mpState.reset();
  mNoiseResources.mpNoiseTex.reset();

  mHeightResources.mSimulatePass.mpComputeVars.reset();
  mHeightResources.mSimulatePass.mpComputeState.reset();
  mHeightResources.mpHeightTex.reset();
  mHeightResources.mpFlowTex.reset();
  mHeightResources.mpVars.reset();
  mHeightResources.mpState.reset();
}

void WaterSimulation::UpdateVars()
{
  vec3 camPos = mpCamera->getPosition();
  mHsPerFrame.eyePos = camPos;
  if (isInNoiseMode)
  {
    mNoiseResources.mDsPerFrame.viewProj = mpCamera->getViewProjMatrix();
    mNoiseResources.mPsPerFrame.eyePos = camPos;

    mNoiseResources.mpVars->getConstantBuffer("HSPerFrame")->setBlob(&mHsPerFrame, 0, sizeof(HsPerFrame));
    mNoiseResources.mpVars->getConstantBuffer("DSPerFrame")->setBlob(&mNoiseResources.mDsPerFrame, 0, sizeof(NoiseWaterResources::DsPerFrame));
    mNoiseResources.mpVars->getConstantBuffer("PSPerFrame")->setBlob(&mNoiseResources.mPsPerFrame, 0, sizeof(NoiseWaterResources::PsPerFrame));
    mNoiseResources.mpVars->setTexture("gNoiseTex", mNoiseResources.mpNoiseTex);
  }
  else
  {
    mHeightResources.mDsPerFrame.viewProj = mpCamera->getViewProjMatrix();
    mHeightResources.mPsPerFrame.eyePos = camPos;

    mHeightResources.mpVars->getConstantBuffer("HSPerFrame")->setBlob(&mHsPerFrame, 0, sizeof(HsPerFrame));
    mHeightResources.mpVars->getConstantBuffer("DSPerFrame")->setBlob(&mHeightResources.mDsPerFrame, 0, sizeof(HeightWaterResources::DsPerFrame));
    mHeightResources.mpVars->getConstantBuffer("PSPerFrame")->setBlob(&mHeightResources.mPsPerFrame, 0, sizeof(HeightWaterResources::PsPerFrame));
    mHeightResources.mpVars->setTexture("gFlowTex", mHeightResources.mpFlowTex);
    mHeightResources.mpVars->setTexture("gHeightTex", mHeightResources.mpHeightTex);
  }
}
