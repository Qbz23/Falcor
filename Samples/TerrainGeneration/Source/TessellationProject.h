#pragma once
#include "Falcor.h"
#include "EffectSample.h"

using namespace Falcor;

class TerrainGeneration : public Sample
{
public:
	void onLoad() override;
	void onFrameRender() override;
	void onShutdown() override;
	void onResizeSwapChain() override;
	bool onKeyEvent(const KeyboardEvent& keyEvent) override;
	bool onMouseEvent(const MouseEvent& mouseEvent) override;
	void onGuiRender() override;

private:
  enum Mode { Intro = 0, Displacement = 1, DynamicPlane = 2, Count = 3 };
  static const Gui::DropdownList kModeDropdown;
  Mode mMode = Displacement;
  EffectSample* effects[Mode::Count];

	vec4 mClearColor = vec4(0.0f, 0.35f, 0.35f, 1.0f);
};
