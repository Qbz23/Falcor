#include "DisplacementMapping.h"

const std::string DisplacementMapping::mTextureNames[kNumTextures] =
{ "Checkerboard", "Links", "Quilt", "Spiral", "Tiles", "Clouds" };

const Gui::DropdownList DisplacementMapping::kTexturesDropdown
{
  { 0, mTextureNames[0] },
  { 1, mTextureNames[1] },
  { 2, mTextureNames[2] },
  { 3, mTextureNames[3] },
  { 4, mTextureNames[4] },
  { 5, mTextureNames[5] }
};

const Gui::DropdownList DisplacementMapping::kShadingModeDropdown
{
  { (int32_t)ShadingMode::Diffuse, "Diffuse" },
  { (int32_t)ShadingMode::DiffuseWithNormalMap, "Diffuse using Normal Map" },
  { (int32_t)ShadingMode::WorldNormal, "Simple World Normal" },
  { (int32_t)ShadingMode::CalcNormal, "Derivitve Calc'd Normal" },
  { (int32_t)ShadingMode::SampleNormal, "Sampled Normal Map Normal" }
};

void DisplacementMapping::LoadTextures()
{
  for (uint32_t i = 0; i < kNumTextures; ++i)
  {
    mDiffuseMaps[i] = createTextureFromFile(mTextureNames[i] + ".png", true, false);
    mNormalMaps[i] = createTextureFromFile(mTextureNames[i] + "Normal.png", true, false);
    mDisplacementMaps[i] = createTextureFromFile(mTextureNames[i] + "Displacement.png", true, false);
  }
}

void DisplacementMapping::LoadModel(std::string filename)
{
  auto model = Model::createFromFile(filename.c_str(), Model::LoadFlags::ForcePatchTopology);
  if(model)
  {
    mpScene->deleteAllModels();
    mpScene->addModelInstance(model, "MainModel");
    mpModelInst = mpScene->getModelInstance(0, 0);
  }
}

//Not sure if passing this in is necessary, try without 
void DisplacementMapping::OnLoad(Fbo::SharedPtr& pDefaultFbo)
{
  //Scene/resources
  LoadTextures();
  mpScene = Scene::create();
  mpScene->addCamera(Camera::create());
  mpSceneRenderer = SceneRenderer::create(mpScene);
  mCamController.attachCamera(mpScene->getActiveCamera());

  //Rasterizer State
  mpDefaultRS = RasterizerState::create(RasterizerState::Desc());
	RasterizerState::Desc wireframeDesc;
	wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
	wireframeDesc.setCullMode(RasterizerState::CullMode::None);
	mpWireframeRS = RasterizerState::create(wireframeDesc);

  //Shader
  Program::DefineList defines;
  defines.add("DIFFUSE");
  auto program = GraphicsProgram::createFromFile(
    "",
    appendShaderExtension("Displacement.ps"),
    "",
    appendShaderExtension("Displacement.hs"),
    appendShaderExtension("Displacement.ds"),
    defines);
  mpVars = GraphicsVars::create(program->getActiveVersion()->getReflector());

  //State
  mpState = GraphicsState::create();
  mpState->setFbo(pDefaultFbo);
  mpState->setProgram(program);
  Sampler::Desc samplerDesc;
  samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear);
  mpVars->setSampler("gSampler", Sampler::create(samplerDesc));
}

void DisplacementMapping::PreFrameRender(RenderContext::SharedPtr pCtx)
{
  mCamController.update();
  UpdateVars();
  pCtx->pushGraphicsState(mpState);
  //pCtx->setGraphicsState(mpState);
  pCtx->pushGraphicsVars(mpVars);
}

void DisplacementMapping::OnFrameRender(RenderContext::SharedPtr pCtx)
{
  mpSceneRenderer->renderScene(pCtx.get());
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();
}

void DisplacementMapping::OnGuiRender(Gui::UniquePtr& mpGui)
{
  static const float floatMax = std::numeric_limits<float>().max();

  //Model Loading
  if(mpGui->addButton("Load Model"))
  {
    std::string filename;
    if(openFileDialog(Model::kSupportedFileFormatsStr, filename))
      LoadModel(filename);
  }

  //Wireframe 
  if (mpGui->addCheckBox("Wireframe", usingWireframeRS))
  {
    //sets it in graphics state whenever it changes 
    if (usingWireframeRS)
    {
      mpState->setRasterizerState(mpWireframeRS);
    }
    else
    {
      mpState->setRasterizerState(mpDefaultRS);
    }
  }

  //Model Rotation
  if(mpModelInst)
  {
    auto rot = mpModelInst->getRotation();
    mpGui->addFloatVar("RotX", rot.x);
    mpGui->addFloatVar("RotY", rot.y);
    mpGui->addFloatVar("RotZ", rot.z);
    mpModelInst->setRotation(rot);
  }

  //Tess Factors
  mpGui->addFloat3Var("Edge Factors", mHullPerFrame.edgeFactors, -floatMax, floatMax);
  mpGui->addFloatVar("Inside Factor", mHullPerFrame.insideFactor, -floatMax, floatMax);

  mpGui->addFloatVar("HeightScale", mDomainPerFrame.heightScale);
  mpGui->addDropdown("Texture", kTexturesDropdown, textureIndex);

  uint32_t mode = (uint32_t)mShadingMode;
  if(mpGui->addDropdown("ShadingMode", kShadingModeDropdown, mode))
  {
    auto program = mpState->getProgram();
    program->clearDefines();

    mShadingMode = (ShadingMode)mode;
    switch(mShadingMode)
    {
      case ShadingMode::Diffuse:
        program->addDefine("DIFFUSE");
        break;
      case ShadingMode::DiffuseWithNormalMap:
        program->addDefine("DIFFUSENORMALMAP");
        break;
      case ShadingMode::WorldNormal:
        program->addDefine("WORLDNORMAL");
        break;
      case ShadingMode::CalcNormal:
        program->addDefine("CALCNORMAL");
        break;
      case ShadingMode::SampleNormal:
        program->addDefine("SAMPLENORMAL");
        break;
      default:  
        should_not_get_here();
    }
  }
  mpGui->addFloat3Var("LightDir", mDispPixelPerFrame.lightDir);

}

bool DisplacementMapping::onKeyEvent(const KeyboardEvent& keyEvent)
{
  bool handled = mCamController.onKeyEvent(keyEvent);
  //Check for R for camera reset
  if (handled == false)
  {
    if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
    {
      switch (keyEvent.key)
      {
      case KeyboardEvent::Key::R:
        //resets camera
        mpScene->getActiveCamera()->setPosition(vec3(0, 0, 5));
        mpScene->getActiveCamera()->setTarget(vec3(0, 0, -1));
        handled = true;
        break;
      }
    }
  }
  return handled;
}

bool DisplacementMapping::onMouseEvent(const MouseEvent& mouseEvent)
{
  return mCamController.onMouseEvent(mouseEvent);
}

void DisplacementMapping::onResizeSwapChain()
{
  auto pFbo = mpState->getFbo();
  float width = (float)pFbo->getWidth();
  float height = (float)pFbo->getHeight();
  auto pCam = mpScene->getActiveCamera();
  pCam->setFocalLength(21.0f);
  pCam->setAspectRatio(width / height); 
}

void DisplacementMapping::UpdateVars()
{
  //hs
  auto hsCbuf = mpVars->getConstantBuffer(0, 1, 0);
  hsCbuf->setBlob(&mHullPerFrame, 0, sizeof(mHullPerFrame));
  //ds
  //Todo get with buffer registers, not string
  auto cBuf = mpVars->getConstantBuffer("DsPerFrame");
  mDomainPerFrame.viewProj = mpScene->getActiveCamera()->getViewProjMatrix();
  
  //uncomment to animate height
  //static float t = 0.0f;
  //t += 0.005f;
  //mDispDomainPerFrame.heightScale = 2 * sin(t) + 1.5f;
  cBuf->setBlob(&mDomainPerFrame, 0, sizeof(DomainPerFrame));
  //ps
  auto psCbuf = mpVars->getConstantBuffer(0, 0, 0);
  mDispPixelPerFrame.eyePos = mpScene->getActiveCamera()->getPosition();
  psCbuf->setBlob(&mDispPixelPerFrame, 0, sizeof(vec3));
  mpVars->setSrv(0, 0, 0, mDiffuseMaps[textureIndex]->getSRV());
  mpVars->setSrv(0, 1, 0, mNormalMaps[textureIndex]->getSRV());
  mpVars->setSrv(0, 2, 0, mDisplacementMaps[textureIndex]->getSRV());
}
