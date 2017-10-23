#pragma once
#include "Falcor.h"
#include  "EffectSample.h"

using namespace Falcor;

class TerrainGeneration : public EffectSample
{
  public: 
    void onLoad(const Fbo::SharedPtr& pDefaultFbo) override;
    void preFrameRender(RenderContext::SharedPtr pCtx) override;
    void onFrameRender(RenderContext::SharedPtr pCtx) override;
    void onGuiRender(Gui* mpGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onShutdown() override;

  private:
    struct Vertex
    {
      vec4 pos;
      vec2 uv;
    };

    void CreatePatch(vec2 heightmapDimensions);
    //Creates Patch VAO and sets it in to gfx state
    void CreatePatchVAO(vec2 heightmapDimensions);
    //Create patch index buffer
    Buffer::SharedPtr CreatePatchIndexBuffer(int numRows, int numColumns);

    GraphicsState::SharedPtr mpState;
    GraphicsVars::SharedPtr mpVars;

    Camera::SharedPtr mpCamera;
    FirstPersonCameraController mCamController;

    void UpdateVars();
    uint32_t mIndexCount;

    Texture::SharedPtr mpHeightmap;
};