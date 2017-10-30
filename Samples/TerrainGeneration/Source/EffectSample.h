#pragma once
#include "Falcor.h"
#include "ProjectUtils.h"

using namespace Falcor;

//This is like a mini sample class, where my main sample lets you switch
//between these smaller samples.
class EffectSample
{
  public:
    using UniquePtr = std::unique_ptr<EffectSample>;

    EffectSample(ProjectUtils* pu) : mpUtils(pu) {}
    virtual ~EffectSample() {};
    virtual void onLoad(const Fbo::SharedPtr& pFbo) {}
    virtual void preFrameRender(RenderContext::SharedPtr pCtx) {}
    virtual void onFrameRender(RenderContext::SharedPtr pCtx) {}
    virtual void onShutdown() {}
    virtual void onGuiRender(Gui* mpGui) {}
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) { return false; }
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) { return false; }
    virtual void onResizeSwapChain() {}

  protected:
    ProjectUtils* mpUtils;

};