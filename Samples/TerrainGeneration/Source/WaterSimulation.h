#pragma once
#include "Falcor.h"
#include "EffectSample.h"

using namespace Falcor;

class WaterSimulation : public EffectSample
{
  public: 
    WaterSimulation(ProjectUtils* pu) : EffectSample(pu) {}
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
    const float kInitialPatchW = 16.0f;
    const float kMinPatchW = 0.1f;
    const float kInitialCameraSpeed = 5.0f;

    struct HsPerFrame
    {
      vec3 eyePos;
      float minDistance = 0.01f;
      float maxDistance = 500.0f;
      int minTessFactor = 64;
      int maxTessFactor = 64;
    } mHsPerFrame;
    const float kSmallestMinDistance = 0.001f;
    const float kSmallestMaxDistance = 0.01f;
    const int kSmallestTessFactor = 2;
    const int kLargestTessFactor = 64;

    struct DsPerFrame
    {
      glm::mat4 viewProj;
      float time = 10.0f; //This should be a var
    } mDsPerFrame;
    const float kMinHeight = 0.01f;

    struct PsPerFrame
    {
      vec3 eyePos;
    } mPsPerFrame;

    struct NoisePsPerFrame
    {
      int octaves = 4;
      float initialFreq = 1;
      float amplitude = 1;
      float time = 0;
      int noiseInputScale = 10;
    };

    struct NoisePass
    {
      FullScreenPass::UniquePtr mpPass;
      GraphicsVars::SharedPtr mpVars;
      GraphicsState::SharedPtr mpState;
      NoisePsPerFrame psPerFrameData;
      ConstantBuffer::SharedPtr psPerFrame;
      Fbo::SharedPtr mpFbo;
      float timeScale = 0.1f;
    } mNoisePass;
    void RenderNoiseTex(RenderContext::SharedPtr pCtx);

    GraphicsState::SharedPtr mpState;
    GraphicsVars::SharedPtr mpVars;

    Camera::SharedPtr mpCamera;
    FirstPersonCameraController mCamController;

    void UpdateVars();
    uint32_t mIndexCount;

    Texture::SharedPtr mpNoiseTex;
};
