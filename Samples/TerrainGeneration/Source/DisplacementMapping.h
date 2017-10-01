#pragma once
#include "Falcor.h"
#include "EffectSample.h"

using namespace Falcor;
using ModelIstnace = ObjectInstance<Model>;

class DisplacementMapping : public EffectSample
{
public:
    void OnLoad(Fbo::SharedPtr& pFbo) override;
    void PreFrameRender(RenderContext::SharedPtr pCtx) override;
    void OnFrameRender(RenderContext::SharedPtr pCtx) override;
    void OnGuiRender(Gui::UniquePtr& mpGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(const MouseEvent& mouseEvent) override;
    void onResizeSwapChain();

  private:
    static const Gui::DropdownList kTexturesDropdown;

    struct HullPerFrame
    {
      glm::vec3 edgeFactors = vec3(2, 2, 2);
      float insideFactor = 2;

    } mHullPerFrame;

    struct DomainPerFrame
    {
      glm::mat4 viewProj = glm::mat4();
      float heightScale = 1.0f;
    } mDomainPerFrame;

    struct PixelPerFrame
    {
      vec3 lightDir = vec3(0, 0, -1);
      float padding;
      float3 eyePos = vec3(0, 0, 5);
    } mDispPixelPerFrame;

    //Texture stuff
    //Loads all available diff/normal/displacement textures
    void LoadTextures();
    static const uint32_t kNumTextures = 6;
    uint32_t textureIndex = 0;
    // {"Checkerboard", "Links", "Quilt", "Spiral", "Tiles", "Clouds"}
    static const std::string mTextureNames[kNumTextures];
    Texture::SharedPtr mDiffuseMaps[kNumTextures];
    Texture::SharedPtr mNormalMaps[kNumTextures];
    Texture::SharedPtr mDisplacementMaps[kNumTextures];

    //Clears scene, creates model inst, and sets it into scene
    void LoadModel(std::string filename);
    ModelIstnace::SharedPtr mpModelInst;
    Scene::SharedPtr mpScene;
    SceneRenderer::SharedPtr mpSceneRenderer;
    FirstPersonCameraController mCamController;

    //Rasterizer State
    bool usingWireframeRS = false;
    RasterizerState::SharedPtr mpDefaultRS;
    RasterizerState::SharedPtr mpWireframeRS;

    GraphicsState::SharedPtr mpState;
    GraphicsVars::SharedPtr mpVars;
    void UpdateVars();
};