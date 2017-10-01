#pragma once
#include "Falcor.h"

using namespace Falcor;

//This is like a mini sample class, where my main sample lets you switch
//between these smaller samples.
class EffectSample
{
  public:
    virtual void OnLoad(Fbo::SharedPtr& pFbo) {}
    virtual void PreFrameRender(RenderContext::SharedPtr pCtx) {}
    virtual void OnFrameRender(RenderContext::SharedPtr pCtx) {}
    virtual void OnShutdown() {}
    virtual void OnGuiRender(Gui::UniquePtr& mpGui) {}
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) { return false; }
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) { return false; }
    virtual void onResizeSwapChain() {}
};