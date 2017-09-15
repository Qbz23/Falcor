#pragma once
#include "Falcor.h"

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
	Camera::SharedPtr mpCamera;
	FirstPersonCameraController mCamController;
	
	Model::SharedPtr mpModel;
	DirectionalLight::SharedPtr mpLight;

	GraphicsState::SharedPtr mpGraphicsState;
	GraphicsVars::SharedPtr mpProgramVars;
	GraphicsProgram::SharedPtr mpProgram;
	RasterizerState::SharedPtr mpWireframeRS;

	vec4 mClearColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
};