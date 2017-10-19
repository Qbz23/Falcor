#pragma once
#include "Falcor.h"
#include  "EffectSample.h"

using namespace Falcor;

class InteractivePlane : public EffectSample
{
  public: 
    void onLoad(Fbo::SharedPtr& pDefaultFbo) override;
    void preFrameRender(RenderContext::SharedPtr pCtx) override;
    void onFrameRender(RenderContext::SharedPtr pCtx) override;
    void onGuiRender(Gui* mpGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onShutdown() override;

  private:
    //Vertices in the VAO of the fullscreen quad
    struct Vertex
    {
      glm::vec4 screenPos;
      glm::vec2 texCoord;
    };

    //Height pass cbuffer
    struct HeightPsPerFrame
    {
      vec2 clickCoords;
      float radius = 0.25f;
      float heightIncrease = 0.01f;
      int sign = 1;
    };

    struct HeightmapPass
    {
      FullScreenPass::UniquePtr mpPass;
      GraphicsVars::SharedPtr mpVars;
      GraphicsState::SharedPtr mpState;
      HeightPsPerFrame psPerFrameData;
      ConstantBuffer::SharedPtr psPerFrame;
      Fbo::SharedPtr mpFbo;
      bool shouldUpdateThisFrame = false;
      bool shouldResetHeight = false;
    } mHeightmapPass;

    void RenderHeightChange(RenderContext::SharedPtr pCtx);

    //Loads plane obj and sets it into the scene
    void CreatePlane();
    //returns tex coords of point of click on object
    float2 ClickRayPlane(float2 mouseCoords);

    Camera::SharedPtr mpCamera;
    FirstPersonCameraController mCamController;

    GraphicsState::SharedPtr mpState;
    GraphicsVars::SharedPtr mpVars;
    Texture::SharedPtr mpHeightmap;

    void UpdateVars();

};