#include "InteractivePlane.h"

void InteractivePlane::OnLoad(Fbo::SharedPtr& pDefaultFbo)
{
  auto program = GraphicsProgram::createFromFile(
    appendShaderExtension("InteractivePlane.vs"),
    appendShaderExtension("InteractivePlane.ps"));
  mpVars = GraphicsVars::create(program->getActiveVersion()->getReflector());

  //Rasterizer State
  RasterizerState::Desc wireframeDesc;
  wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
  wireframeDesc.setCullMode(RasterizerState::CullMode::None);
  auto wireframeRS = RasterizerState::create(wireframeDesc);

  mpCamera = Camera::create();
  mCamController.attachCamera(mpCamera);

  mpState = GraphicsState::create();
  mpState->setFbo(pDefaultFbo);
  mpState->setProgram(program);
  mpState->setRasterizerState(wireframeRS);
  CreatePlane();
}

void InteractivePlane::PreFrameRender(RenderContext::SharedPtr pCtx)
{
  mCamController.update();

  UpdateVars();
  pCtx->pushGraphicsState(mpState);
  pCtx->pushGraphicsVars(mpVars);
}

void InteractivePlane::OnFrameRender(RenderContext::SharedPtr pCtx)
{
  pCtx->draw(4, 0);
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();
}

void InteractivePlane::OnGuiRender(Gui::UniquePtr& mpGui)
{
  mpGui->addFloat4Var("Edge Factors", mHullPerFrame.edgeFactors, 1, 64);
  mpGui->addFloat2Var("Inside Factors", mHullPerFrame.insideFactors, 1, 64);
}

bool InteractivePlane::onKeyEvent(const KeyboardEvent& keyEvent)
{
  return mCamController.onKeyEvent(keyEvent);
}

bool InteractivePlane::onMouseEvent(const MouseEvent& mouseEvent)
{
  return mCamController.onMouseEvent(mouseEvent);
}

void InteractivePlane::CreatePlane()
{
  const Vertex kPlaneVertices[] =
  {	
    { glm::vec4(-1.0f, 0.f, 1.0f, 1.0f), glm::vec2(0, 0) },
    { glm::vec4(-1.0f, 0.f, -1.0f, 1.0f), glm::vec2(0, 1) },
    { glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec2(1, 0) },
    { glm::vec4(1.0f, 0.0f, -1.0f, 1.0f), glm::vec2(1, 1) },
  };

  //create VB
  const uint32_t vbSize = (uint32_t)(sizeof(Vertex)*arraysize(kPlaneVertices));
  auto vertexBuffer = Buffer::create(vbSize, Buffer::BindFlags::Vertex,
    Buffer::CpuAccess::Write, (void*)kPlaneVertices);

  //create input layout
  VertexLayout::SharedPtr pLayout = VertexLayout::create();
  VertexBufferLayout::SharedPtr pBufLayout = VertexBufferLayout::create();
  pBufLayout->addElement("POSITION", 0, ResourceFormat::RGBA32Float, 1, 0);
  pBufLayout->addElement("TEXCOORD", 16, ResourceFormat::RG32Float, 1, 1);
  pLayout->addBufferLayout(0, pBufLayout);

  //create vao
  Vao::BufferVec buffers{ vertexBuffer };
  //Change this to patch4 for tess
  auto vao = Vao::create(Vao::Topology::TriangleStrip, pLayout, buffers);
  //Set it into graphics state
  mpState->setVao(vao);
}

void InteractivePlane::UpdateVars()
{
  auto cbuf = mpVars->getConstantBuffer(0, 0, 0);
  glm::mat4 viewProj = mpCamera->getViewProjMatrix();
  cbuf->setBlob(&viewProj, 0, sizeof(glm::mat4));
}
