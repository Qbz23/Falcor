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

  struct DispHullPerFrame
  {
    glm::vec3 edgeFactors = vec3(2, 2, 2);
    float insideFactor = 2;

  } mDispHullPerFrame;

  struct DispDomainPerFrame
  {
    glm::mat4 viewProj = glm::mat4();
    float heightScale = 1.0f;
  } mDispDomainPerFrame;

  struct DispPixelPerFrame
  {
    vec3 lightDir = vec3(0, 0, -1);
    float padding;
    float3 eyePos = vec3(0, 0, 5);
  } mDispPixelPerFrame;

  enum Mode { Intro, Displacement };
  static const Gui::DropdownList kModeDropdown;
  Mode mMode = Displacement;
  static const Gui::DropdownList kTexturesDropdown;

  void LoadModel(std::string filename);
  void LoadTextures();

	void CreateQuad();
  //makes sure they get set at least once on first frame
  bool varsDirty = true;
  void UpdateVars();

  //Geometry stuff
	Vao::SharedPtr mpQuadVao;
  Model::SharedPtr mpModel;

  //Texture stuff
  static const uint32_t kNumTextures = 6;
  uint32_t textureIndex = 0;
  // {"Checkerboard", "Links", "Quilt", "Spiral", "Tiles", "Clouds"}
  static const std::string mTextureNames[kNumTextures];
  Texture::SharedPtr mDiffuseMaps[kNumTextures];
  Texture::SharedPtr mNormalMaps[kNumTextures];
  Texture::SharedPtr mDisplacementMaps[kNumTextures];

  Scene::SharedPtr mpScene;
  SceneRenderer::SharedPtr mpSceneRenderer;
	FirstPersonCameraController mCamController;

	GraphicsState::SharedPtr mpGraphicsState;
  RasterizerState::SharedPtr mpWireframeRS;
  RasterizerState::SharedPtr mpDefaultRS;
  bool hasWireframeRs = true;

  GraphicsProgram::SharedPtr mpDisplacementProgram;
  GraphicsVars::SharedPtr mpDisplacementVars;

	GraphicsProgram::SharedPtr mpTessIntroProgram;
  GraphicsVars::SharedPtr mpTessIntroProgramVars;

	vec4 mClearColor = vec4(0.0f, 0.35f, 0.35f, 1.0f);
};
