#pragma once
#include "Falcor.h"

using namespace Falcor;

//This is like a mini sample class, where my main sample lets you switch
//between these smaller samples.
class EffectSample
{
  public:
    virtual void onLoad(Fbo::SharedPtr& pFbo) {}
    virtual void preFrameRender(RenderContext::SharedPtr pCtx) {}
    virtual void onFrameRender(RenderContext::SharedPtr pCtx) {}
    virtual void onShutdown() {}
    virtual void onGuiRender(Gui* mpGui) {}
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) { return false; }
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) { return false; }
    virtual void onResizeSwapChain() {}
};