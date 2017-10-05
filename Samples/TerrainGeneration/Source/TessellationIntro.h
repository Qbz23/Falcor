#pragma once
#include "Falcor.h"
#include  "EffectSample.h"

using namespace Falcor;

class TessellationIntro : public EffectSample
{
  public: 
    void OnLoad(Fbo::SharedPtr& pDefaultFbo) override;
    void PreFrameRender(RenderContext::SharedPtr pCtx) override;
    void OnFrameRender(RenderContext::SharedPtr pCtx) override;
    void OnGuiRender(Gui::UniquePtr& mpGui) override;
    bool onKeyEvent(const KeyboardEvent& keyEvent) override;

  private:
    //Vertices in the VAO of the fullscreen quad
    struct Vertex
    {
      glm::vec2 screenPos;
      glm::vec2 texCoord;
    };

    struct HullPerFrame
    {
      glm::vec4 edgeFactors = vec4(32, 32, 32, 32);
      glm::vec2 insideFactors = vec2(32, 32);
      glm::vec2 padding;
    } mHullPerFrame;

    //Creates the VAO of the fullscreen quad & sets it into gfx state
    void CreateQuad();

    GraphicsState::SharedPtr mpState;
    GraphicsVars::SharedPtr mpVars;
    void UpdateVars();
};