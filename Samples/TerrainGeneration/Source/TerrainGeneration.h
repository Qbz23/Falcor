#pragma once
#include "Falcor.h"
#include  "EffectSample.h"

using namespace Falcor;

class TerrainGeneration : public EffectSample
{
  public: 
    TerrainGeneration(ProjectUtils* pu) : EffectSample(pu) {}
    void onLoad(const Fbo::SharedPtr& pDefaultFbo) override;
    void preFrameRender(RenderContext::SharedPtr pCtx) override;
    void onFrameRender(RenderContext::SharedPtr pCtx) override;
    void onGuiRender(Gui* mpGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onShutdown() override;

  private:
    const int kInitialNumRows = 32;
    const int kMaxRows = 256;
    const int kInitialNumCols = 32;
    const int kMaxCols = 256;
    const float kInitialPatchW = 4.0f;
    const float kMinPatchW = 0.1f;
    const float kInitialCameraSpeed = 5.0f;

    struct HsPerFrame
    {
      vec3 eyePos;
      float minDistance = 0.01f;
      float maxDistance = 350.0f;
      int minTessFactor = 4;
      int maxTessFactor = 64;
    } mHsPerFrame;
    const float kSmallestMinDistance = 0.001f;
    const float kSmallestMaxDistance = 0.01f;
    const int kSmallestTessFactor = 2;
    const int kLargestTessFactor = 64;

    struct DsPerFrame
    {
      glm::mat4 viewProj;
      float maxHeight = 10.0f; //This should be a var
    } mDsPerFrame;
    const float kMinHeight = 0.01f;

    struct PsPerFrame
    {
      vec3 eyePos;
      float maxHeight = 10.0f;
      //Add or subtract first before determining color
      float heightColorOffset = 0.0f;
    } mPsPerFrame;

    GraphicsState::SharedPtr mpState;
    GraphicsVars::SharedPtr mpVars;

    Camera::SharedPtr mpCamera;
    FirstPersonCameraController mCamController;

    void UpdateVars();
    uint32_t mIndexCount;

    //Textures
    static const Gui::DropdownList kHeightmapDropdown;
    static const int kNumHeightmaps = 5;
    //{ "Redmond", "Colorado", "Philadelphia", "Arizona", "Maine" };
    static const std::string mHeightmapNames[kNumHeightmaps];
    void LoadHeightmaps();
    uint32_t mHeightmapIndex = 0;
    Texture::SharedPtr mHeightmaps[kNumHeightmaps];
};
