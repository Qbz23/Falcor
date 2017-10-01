#include "TessellationIntro.h"

void TessellationIntro::OnLoad(Fbo::SharedPtr& pDefaultFbo)
{
  auto program = GraphicsProgram::createFromFile(
    appendShaderExtension("TesellationIntro.vs"),
    appendShaderExtension("TesellationIntro.ps"),
    "",
    appendShaderExtension("TesellationIntro.hs"),
    appendShaderExtension("TesellationIntro.ds"));
  mpVars = GraphicsVars::create(program->getActiveVersion()->getReflector());

  //Rasterizer State
  RasterizerState::Desc wireframeDesc;
  wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
  wireframeDesc.setCullMode(RasterizerState::CullMode::None);
  auto wireframeRS = RasterizerState::create(wireframeDesc);

  mpState = GraphicsState::create();
  mpState->setFbo(pDefaultFbo);
  mpState->setProgram(program);
  mpState->setRasterizerState(wireframeRS);
  CreateQuad();
}

void TessellationIntro::PreFrameRender(RenderContext::SharedPtr pCtx)
{
  UpdateVars();
  pCtx->pushGraphicsState(mpState);
  pCtx->pushGraphicsVars(mpVars);
}

void TessellationIntro::OnFrameRender(RenderContext::SharedPtr pCtx)
{
  pCtx->draw(4, 0);
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();
}

void TessellationIntro::OnGuiRender(Gui::UniquePtr& mpGui)
{
  mpGui->addFloat4Var("Edge Factors", mHullPerFrame.edgeFactors, 1, 64);
  mpGui->addFloat2Var("Inside Factors", mHullPerFrame.insideFactors, 1, 64);
}

bool TessellationIntro::onKeyEvent(const KeyboardEvent& keyEvent)
{
  //TODO Arrow keys control tessellation factor 
  return true;
}

void TessellationIntro::CreateQuad()
{
  //Just in NDC. Not doing any transformation
  const Vertex kQuadVertices[] =
  {	//Only work for DX. See FullScreenPass.cpp line 60
    //for macro so that it also works with vulkan
    { glm::vec2(-1, 1), glm::vec2(0, 0) },
    { glm::vec2(-1, -1), glm::vec2(0, 1) },
    { glm::vec2(1, 1), glm::vec2(1, 0) },
    { glm::vec2(1, -1), glm::vec2(1, 1) },
  };

  //create VB
  const uint32_t vbSize = (uint32_t)(sizeof(Vertex)*arraysize(kQuadVertices));
  auto vertexBuffer = Buffer::create(vbSize, Buffer::BindFlags::Vertex,
    Buffer::CpuAccess::Write, (void*)kQuadVertices);

  //create input layout
  VertexLayout::SharedPtr pLayout = VertexLayout::create();
  VertexBufferLayout::SharedPtr pBufLayout = VertexBufferLayout::create();
  pBufLayout->addElement("POSITION", 0, ResourceFormat::RG32Float, 1, 0);
  pBufLayout->addElement("TEXCOORD", 8, ResourceFormat::RG32Float, 1, 1);
  pLayout->addBufferLayout(0, pBufLayout);

  //create vao
  Vao::BufferVec buffers{ vertexBuffer };
  auto vao = Vao::create(Vao::Topology::Patch4, pLayout, buffers);
  //Set it into graphics state
  mpState->setVao(vao);
}

void TessellationIntro::UpdateVars()
{
  auto cbuf = mpVars->getConstantBuffer(0, 0, 0);
  cbuf->setBlob(&mHullPerFrame, 0, sizeof(HullPerFrame));
}
