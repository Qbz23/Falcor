#include "InteractivePlane.h"

void InteractivePlane::OnLoad(Fbo::SharedPtr& pDefaultFbo)
{
  auto program = GraphicsProgram::createFromFile(
    appendShaderExtension("InteractivePlane.vs"),
    appendShaderExtension("InteractivePlane.ps"));
  mpVars = GraphicsVars::create(program->getActiveVersion()->getReflector());

  //Rasterizer State
  RasterizerState::Desc wireframeDesc;
  //wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
  wireframeDesc.setCullMode(RasterizerState::CullMode::Front);
  auto wireframeRS = RasterizerState::create(wireframeDesc);

  mpScene = Scene::create();
  mpSceneRenderer = SceneRenderer::create(mpScene);
  auto camera = Camera::create();
  mpScene->addCamera(camera);
  mCamController.attachCamera(camera);

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
  mpSceneRenderer->renderScene(pCtx.get());
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
  //Could use the picking class, and probably will need to for terrain gen
  //but since its only 1 model/mesh and it guarunteed to be the plane y = 0, 
  //can do simple janky check

  if(mouseEvent.type == MouseEvent::Type::LeftButtonDown)
  {
    vec2 clickTex = ClickRayPlane(mouseEvent.pos); 
    auto cbuf = mpVars->getConstantBuffer(0, 1, 0);
    cbuf->setBlob(&clickTex, 0, sizeof(vec2));
  }

  return mCamController.onMouseEvent(mouseEvent);
}

float2 InteractivePlane::ClickRayPlane(float2 mouseCoords)
{
  //[0, 1], y down
  float2 screenPos = mouseCoords;
  //2d [-1, 1] y up
  float2 ndcPos2 = screenPos * vec2(2, -2) + vec2(-1, 1);
  //4d [-1, 1]
  float4 ndcPos = float4(ndcPos2.x, ndcPos2.y, -1.0f, 1.0f);
  //ndc -> viewspace
  auto proj = mpScene->getActiveCamera()->getProjMatrix();
  float4 viewRay = glm::inverse(proj) * ndcPos;
  viewRay.z = -1.0f;
  viewRay.w = 0.0f;
  //viewspace -> worldpsace
  auto view = mpScene->getActiveCamera()->getViewMatrix();
  float4 worldRay = glm::inverse(view) * viewRay;
  float3 worldRay3 = float3(worldRay.x, worldRay.y, worldRay.z);
  worldRay3 = glm::normalize(worldRay3);

  auto eyePos = mpScene->getActiveCamera()->getPosition();
  //looking to hit plane y = 0
  float t = -eyePos.y / worldRay3.y;
  float3 hitPos = eyePos + worldRay3 * t;
  //lol apparently quad1x1 is actually quad10x10 
  //convert from [-10, 10] to [0, 1] and swap y for texC
  hitPos += float3(10.0f, 0.0f, -10.0f);
  hitPos /= float3(20.0f, 1.0f, -20.0f);
  return float2(hitPos.x, hitPos.z);
}

void InteractivePlane::CreatePlane()
{
  auto model = Model::createFromFile("quad1x1.obj");
  mpScene->addModelInstance(model, "Plane");
}

void InteractivePlane::UpdateVars()
{
  //b/c im not going through default vs, just apply 90 degree rotation here.
  static const glm::mat4 rotMat = glm::yawPitchRoll(0.f, 3.1415f / 2.f, 0.f);
  auto cbuf = mpVars->getConstantBuffer(0, 0, 0);
  glm::mat4 viewProj = mpScene->getActiveCamera()->getViewProjMatrix();
  viewProj = viewProj * rotMat;
  cbuf->setBlob(&viewProj, 0, sizeof(glm::mat4));
}
