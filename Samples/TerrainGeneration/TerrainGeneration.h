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

  enum Mode { Intro, Displacement };
  static const Gui::DropdownList kModeDropDown;
  Mode mMode = Intro;

	void CreateQuad();
  //makes sure they get set at least once on first frame
  bool varsDirty = true;
  void UpdateVars();

	Buffer::SharedPtr mpQuadVertexBuffer;
	Vao::SharedPtr mpQuadVao;

	Camera::SharedPtr mpCamera;
	FirstPersonCameraController mCamController;
	
	Model::SharedPtr mpModel;
	DirectionalLight::SharedPtr mpLight;

	GraphicsState::SharedPtr mpGraphicsState;
	GraphicsVars::SharedPtr mpProgramVars;
	GraphicsProgram::SharedPtr mpProgram;
	RasterizerState::SharedPtr mpWireframeRS;

	vec4 mClearColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	int tessellationFactor = 32;
};
