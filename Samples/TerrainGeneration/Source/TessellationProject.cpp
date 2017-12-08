#include "TessellationProject.h"
#include "TessellationIntro.h"
#include "DisplacementMapping.h"
#include "InteractivePlane.h"
#include "TerrainGeneration.h"
#include "WaterSimulation.h"

const Gui::DropdownList TessellationProject::kModeDropdown
{
  { (int32_t)Mode::Intro, "Intro to Tessellation" },
  { (int32_t)Mode::Displacement, "Displacement Mapping" },
  { (int32_t)Mode::DynamicPlane, "Interactive Plane" },
  { (int32_t)Mode::Terrain, "Terrain Generation" },
  { (int32_t)Mode::Water, "Water Simulation"}
};

void TessellationProject::onGuiRender()
{
  if (mpGui->beginGroup("Core"))
  {
    uint32_t iMode = (uint32_t)mMode;
    mpGui->addDropdown("Mode", kModeDropdown, iMode);
    mMode = (Mode)iMode;
    mpGui->addFloat4Var("Clear Color", mClearColor, 0, 1.0f);
    mpGui->addSeparator();
    mpGui->endGroup();
  }
  effects[mMode]->onGuiRender(mpGui.get());
}

void TessellationProject::onLoad()
{
  mProjectUtils.onLoad();
  //Needs to be ptr so it calls derived class functions
  effects[Mode::Intro] = 
    std::make_unique<TessellationIntro>(TessellationIntro(&mProjectUtils));
  effects[Mode::Displacement] = 
    std::make_unique<DisplacementMapping>(DisplacementMapping(&mProjectUtils));
  effects[Mode::DynamicPlane] = 
    std::make_unique<InteractivePlane>(InteractivePlane(&mProjectUtils));
  effects[Mode::Terrain] = 
    std::make_unique<TerrainGeneration>(TerrainGeneration(&mProjectUtils));
  effects[Mode::Water] = 
    std::make_unique<WaterSimulation>(WaterSimulation(&mProjectUtils));

  for(uint32_t i = 0; i <Mode::Count; ++i)
  {
    effects[i]->onLoad(mpDefaultFBO);
  }
}

void TessellationProject::onFrameRender()
{
  mpRenderContext->clearFbo(mpDefaultFBO.get(), mClearColor, 1.0f, 0, FboAttachmentType::All);
  effects[mMode]->updateDt(frameRate().getLastFrameTime());
  effects[mMode]->preFrameRender(mpRenderContext);

  //Get around that annoying multiple buffer swapchain d3d12 error 
  mpRenderContext->getGraphicsState()->setFbo(mpDefaultFBO);
  effects[mMode]->onFrameRender(mpRenderContext);
}

void TessellationProject::onShutdown()
{
  for (uint32_t i = 0; i < Mode::Count; ++i)
  {
    effects[i]->onShutdown();
    effects[i].release();
  }
}

bool TessellationProject::onKeyEvent(const KeyboardEvent& keyEvent)
{
  return effects[mMode]->onKeyEvent(keyEvent);
}

bool TessellationProject::onMouseEvent(const MouseEvent& mouseEvent)
{
  return effects[mMode]->onMouseEvent(mouseEvent);
}

void TessellationProject::onResizeSwapChain()
{
  for(uint32_t i = 0; i < Mode::Count; ++i)
  {
    effects[i]->onResizeSwapChain();
  }
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
  TessellationProject tp;
	SampleConfig config;
	config.windowDesc.title = "Terrain Generation Project";
	config.windowDesc.resizableWindow = true;
	tp.run(config);
}
