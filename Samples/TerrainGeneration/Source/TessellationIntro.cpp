#include "TessellationIntro.h"

void TessellationIntro::onLoad(const Fbo::SharedPtr& pDefaultFbo)
{
  auto program = GraphicsProgram::createFromFile(
    appendShaderExtension("TessellationIntro.vs"),
    appendShaderExtension("TessellationIntro.ps"),
    "",
    appendShaderExtension("TessellationIntro.hs"),
    appendShaderExtension("TessellationIntro.ds"));
  mpVars = GraphicsVars::create(program->getActiveVersion()->getReflector());

  mpState = GraphicsState::create();
  mpState->setFbo(pDefaultFbo);
  mpState->setProgram(program);
  mpState->setRasterizerState(mpUtils->GetWireframeRS());
  mpState->setVao(mpUtils->GetUnitQuadPatchVao());
}

void TessellationIntro::preFrameRender(RenderContext::SharedPtr pCtx)
{
  UpdateVars();
  pCtx->pushGraphicsState(mpState);
  pCtx->pushGraphicsVars(mpVars);
}

void TessellationIntro::onFrameRender(RenderContext::SharedPtr pCtx)
{
  pCtx->draw(4, 0);
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();
}

void TessellationIntro::onGuiRender(Gui* mpGui)
{
  if (mpGui->beginGroup("Intro to Tessellation"))
  {
    mpGui->addFloat4Var("Edge Factors", mHullPerFrame.edgeFactors, 1, 64);
    mpGui->addFloat2Var("Inside Factors", mHullPerFrame.insideFactors, 1, 64);
    mpGui->endGroup();
  }
}

bool TessellationIntro::onKeyEvent(const KeyboardEvent& keyEvent)
{
  if (keyEvent.type == KeyboardEvent::Type::KeyReleased)
    return false;

  switch (keyEvent.key)
  {
  case KeyboardEvent::Key::Left:
    mHullPerFrame.edgeFactors -= vec4(1, 1, 1, 1);
    break;
  case KeyboardEvent::Key::Right:
    mHullPerFrame.edgeFactors += vec4(1, 1, 1, 1);
    break;
  case KeyboardEvent::Key::Up:
    mHullPerFrame.insideFactors += vec2(1, 1);
    break;
  case KeyboardEvent::Key::Down:
    mHullPerFrame.insideFactors -= vec2(1, 1);
    break;
  default:
    return false;
  }

  //Dont move tess factors below 1 or above 64
  mHullPerFrame.edgeFactors = max(mHullPerFrame.edgeFactors, vec4(1, 1, 1, 1));
  mHullPerFrame.edgeFactors = min(mHullPerFrame.edgeFactors, vec4(64, 64, 64, 64));
  mHullPerFrame.insideFactors = max(mHullPerFrame.insideFactors, vec2(1, 1));
  mHullPerFrame.insideFactors = min(mHullPerFrame.insideFactors, vec2(64, 64));
  return true;
}

void TessellationIntro::UpdateVars()
{
  auto cbuf = mpVars->getConstantBuffer(0, 0, 0);
  cbuf->setBlob(&mHullPerFrame, 0, sizeof(HullPerFrame));
}

void TessellationIntro::onShutdown()
{
  //Not 100% sure why I need to do this. Maybe b/c of the 
  //subproject effect sample setup? But if i dont do this,
  //there are live objects on exit
  mpVars.reset();
  mpState.reset();
}
