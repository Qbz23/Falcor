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

    //Default nullptr, pass in fr ptr if the sample actually needs it
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
    //It's a pain to deal with this and FrameRate() on sample with the way i did this.
    //Probably should have made multiple small samples in hindsight
    void updateDt(float newDt) {dt = newDt;}

  protected:
    ProjectUtils* mpUtils;
    float dt;

};