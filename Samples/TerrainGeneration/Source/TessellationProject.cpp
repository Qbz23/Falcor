#include "TessellationProject.h"
#include "TessellationIntro.h"
#include "DisplacementMapping.h"

const Gui::DropdownList TerrainGeneration::kModeDropdown
{
  { (int32_t)Mode::Intro, "Intro to Tessellation" },
  { (int32_t)Mode::Displacement, "Displacement Mapping" }
};

void TerrainGeneration::onGuiRender()
{
  uint32_t iMode = (uint32_t)mMode;
  mpGui->addDropdown("Mode", kModeDropdown, iMode);
  mMode = (Mode)iMode;
	mpGui->addFloat4Var("Clear Color", mClearColor, 0, 1.0f);
  mpGui->addSeparator();

  effects[iMode]->OnGuiRender(mpGui);
}

void TerrainGeneration::onLoad()
{
  //Needs to be ptr so it calls derived class functions
  effects[Mode::Intro] = new TessellationIntro();
  effects[Mode::Displacement] = new DisplacementMapping();

  for(uint32_t i = 0; i < Mode::Count; ++i)
  {
    effects[i]->OnLoad(mpDefaultFBO);
  }
}

void TerrainGeneration::onFrameRender()
{
	mpRenderContext->clearFbo(mpDefaultFBO.get(), mClearColor, 1.0f, 0, FboAttachmentType::All);
  effects[mMode]->PreFrameRender(mpRenderContext);
  //Get around that annoying multiple buffer swapchain d3d12 error 
  mpRenderContext->getGraphicsState()->setFbo(mpDefaultFBO);
  effects[mMode]->OnFrameRender(mpRenderContext);
	//Todo this is rendering double text. Not sure why.
	//renderText(getFpsMsg(), glm::vec2(10, 30));
}

void TerrainGeneration::onShutdown()
{
  for(uint32_t i = 0; i < Mode::Count; ++i)
  {
    effects[i]->OnShutdown();
    delete effects[i];
  }
}

bool TerrainGeneration::onKeyEvent(const KeyboardEvent& keyEvent)
{
  return effects[mMode]->onKeyEvent(keyEvent);
}

bool TerrainGeneration::onMouseEvent(const MouseEvent& mouseEvent)
{
  return effects[mMode]->onMouseEvent(mouseEvent);
}

void TerrainGeneration::onResizeSwapChain()
{
  for(uint32_t i = 0; i < Mode::Count; ++i)
  {
    effects[i]->onResizeSwapChain();
  }
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	TerrainGeneration tg;
	SampleConfig config;
	config.windowDesc.title = "Terrain Generation Project";
	config.windowDesc.resizableWindow = true;
	tg.run(config);
}
