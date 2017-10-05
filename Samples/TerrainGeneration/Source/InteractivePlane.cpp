#include "InteractivePlane.h"

void InteractivePlane::OnLoad(Fbo::SharedPtr& pDefaultFbo)
{
  auto program = GraphicsProgram::createFromFile(
    appendShaderExtension("InteractivePlane.vs"),
    appendShaderExtension("InteractivePlane.ps"));
  mpVars = GraphicsVars::create(program->getActiveVersion()->getReflector());

  mpCamera = Camera::create();
  mCamController.attachCamera(mpCamera);

  mpState = GraphicsState::create();
  mpState->setFbo(pDefaultFbo);
  mpState->setProgram(program);

  CreatePlane();

  //Stuff for heightmap pass
  mpHeightmap = Texture::create2D(
    pDefaultFbo->getWidth(),
    pDefaultFbo->getHeight(),
    ResourceFormat::RGBA32Float, 
    1u,
    UINT32_MAX, 
    nullptr, 
    Resource::BindFlags::ShaderResource | Resource::BindFlags::RenderTarget);
  mpHeightmap->generateMips();
  mHeightmapPass.mpState = GraphicsState::create();
  mHeightmapPass.mpPass = FullScreenPass::create(
    appendShaderExtension("Heightmap.ps"));
  mHeightmapPass.mpVars = GraphicsVars::create(
    mHeightmapPass.mpPass->getProgram()->getActiveVersion()->getReflector());
  mHeightmapPass.psPerFrame = mHeightmapPass.mpVars->getConstantBuffer(0, 1, 0);
  mHeightmapPass.mpFbo = Fbo::create();
  mHeightmapPass.mpFbo->attachColorTarget(mpHeightmap, 0);
  mHeightmapPass.mpState->setFbo(mHeightmapPass.mpFbo);
}

void InteractivePlane::RenderHeightChange(RenderContext::SharedPtr pCtx)
{
  mHeightmapPass.shouldUpdateThisFrame = false;
  //Set vars
  mHeightmapPass.psPerFrame->setBlob(
    &mHeightmapPass.psPerFrameData, 0, sizeof(HeightPsPerFrame));
  mHeightmapPass.mpVars->setSrv(0, 0, 0, mpHeightmap->getSRV());

  //Render to height map
  pCtx->pushGraphicsState(mHeightmapPass.mpState);
  pCtx->pushGraphicsVars(mHeightmapPass.mpVars);
  mHeightmapPass.mpPass->execute(pCtx.get());
  pCtx->popGraphicsVars();
  pCtx->popGraphicsState();

  //Save heightmap render texture result
  mpHeightmap = mHeightmapPass.mpFbo->getColorTexture(0);
}

void InteractivePlane::PreFrameRender(RenderContext::SharedPtr pCtx)
{
  mCamController.update();

  if(mHeightmapPass.shouldUpdateThisFrame)
    RenderHeightChange(pCtx);

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
}

bool InteractivePlane::onKeyEvent(const KeyboardEvent& keyEvent)
{
  return mCamController.onKeyEvent(keyEvent);
}

bool InteractivePlane::onMouseEvent(const MouseEvent& mouseEvent)
{
  if(mouseEvent.type == MouseEvent::Type::LeftButtonDown)
  {
    //Todo, hook most of this up to a gui. 
    //Only the click and the bool and the sign needs to happen here
    mHeightmapPass.shouldUpdateThisFrame = true;
    mHeightmapPass.psPerFrameData.clickCoords = ClickRayPlane(mouseEvent.pos);
    mHeightmapPass.psPerFrameData.radius = 0.25f;
    mHeightmapPass.psPerFrameData.heightIncrease = 1.0f;
    mHeightmapPass.psPerFrameData.sign = 1;
  }
  else if(mouseEvent.type == MouseEvent::Type::RightButtonDown)
  {
    mHeightmapPass.shouldUpdateThisFrame = true;
    mHeightmapPass.psPerFrameData.clickCoords = ClickRayPlane(mouseEvent.pos);
    mHeightmapPass.psPerFrameData.radius = 0.25f;
    mHeightmapPass.psPerFrameData.heightIncrease = 1.0f;
    mHeightmapPass.psPerFrameData.sign = -1;
  }

  return mCamController.onMouseEvent(mouseEvent);
}

float2 InteractivePlane::ClickRayPlane(float2 mouseCoords)
{
  //This technically wont be correct as plane geo changes, 
  //youre actually clicking the plane of y = 0, not the rendered geo
  //Could probably fix but eh. Maybe later. TODO?

  //[0, 1], y down
  float2 screenPos = mouseCoords;
  //2d [-1, 1] y up
  float2 ndcPos2 = screenPos * vec2(2, -2) + vec2(-1, 1);
  //4d [-1, 1]
  float4 ndcPos = float4(ndcPos2.x, ndcPos2.y, -1.0f, 1.0f);
  //ndc -> viewspace
  //auto proj = mpScene->getActiveCamera()->getProjMatrix();
  auto proj = mpCamera->getProjMatrix();
  float4 viewRay = glm::inverse(proj) * ndcPos;
  viewRay.z = -1.0f;
  viewRay.w = 0.0f;
  //viewspace -> worldpsace
  //auto view = mpScene->getActiveCamera()->getViewMatrix();
  auto view = mpCamera->getViewMatrix();
  float4 worldRay = glm::inverse(view) * viewRay;
  float3 worldRay3 = float3(worldRay.x, worldRay.y, worldRay.z);
  worldRay3 = glm::normalize(worldRay3);

  //auto eyePos = mpScene->getActiveCamera()->getPosition();
  auto eyePos = mpCamera->getPosition();
  //looking to hit plane y = 0
  float t = -eyePos.y / worldRay3.y;
  float3 hitPos = eyePos + worldRay3 * t;
  //convert from [-1, 1] to [0, 1] and swap y for texC
  hitPos += float3(1.0f, 0.0f, -1.0f);
  hitPos /= float3(2.0f, 1.0f, -2.0f);
  return float2(hitPos.x, hitPos.z);
}

void InteractivePlane::CreatePlane()
{
  const Vertex kPlaneVertices[] =
  {
    { glm::vec4(-1.0f, 0.f, 1.0f, 1.0f), glm::vec2(0, 0) },
    { glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), glm::vec2(1, 0) },
    { glm::vec4(-1.0f, 0.f, -1.0f, 1.0f), glm::vec2(0, 1) },
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
  auto viewProj = mpCamera->getViewProjMatrix();
  cbuf->setBlob(&viewProj, 0, sizeof(glm::mat4));
  mpVars->setSrv(0, 0, 0, mpHeightmap->getSRV());
}
