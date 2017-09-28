#include "TerrainGeneration.h"

const Gui::DropdownList TerrainGeneration::kModeDropdown
{
  { (int32_t)Mode::Intro, "Intro to Tessellation" },
  { (int32_t)Mode::Displacement, "Displacement Mapping" }
};

const std::string TerrainGeneration::mTextureNames[kNumTextures] =
{ "Checkerboard", "Links", "Quilt", "Spiral", "Tiles" };

const Gui::DropdownList TerrainGeneration::kTexturesDropdown
{
  { 0, mTextureNames[0] },
  { 1, mTextureNames[1] },
  { 2, mTextureNames[2] },
  { 3, mTextureNames[3] },
  { 4, mTextureNames[4] }
};

void TerrainGeneration::LoadModel(std::string filename)
{
  Model::LoadFlags flags = Model::LoadFlags();
  mpModel = Model::createFromFile(filename.c_str());
}

void TerrainGeneration::LoadTextures()
{
  for(uint32_t i = 0; i < kNumTextures; ++i)
  {
    mDiffuseMaps[i] = createTextureFromFile(mTextureNames[i] + ".png", true, false);
    mNormalMaps[i] = createTextureFromFile(mTextureNames[i] + "Normal.png", true, false);
    mDisplacementMaps[i] = createTextureFromFile(mTextureNames[i] + "Displacement.png", true, false);
  }
}

void TerrainGeneration::onGuiRender()
{
	static const int intMax = std::numeric_limits<int>().max();

  uint32_t iMode = (uint32_t)mMode;
  varsDirty = mpGui->addDropdown("Mode", kModeDropdown, iMode);
  mMode = (Mode)iMode;
  if(mpGui->addCheckBox("Wireframe", hasWireframeRs))
  {
    if(hasWireframeRs)
    {
      mpGraphicsState->setRasterizerState(mpWireframeRS);
    }
    else
    {
      mpGraphicsState->setRasterizerState(mpDefaultRS);
    }
  }
	mpGui->addFloat4Var("Clear Color", mClearColor, 0, 1.0f);
  mpGui->addSeparator();

  switch(mMode)
  {
    case Intro:
      varsDirty = mpGui->addFloat4Var("edge", mHullPerFrame.edgeFactors, 1, 64) || varsDirty;
      varsDirty = mpGui->addFloat2Var("inside", mHullPerFrame.insideFactors, 1, 64) || varsDirty;
      break;
    case Displacement:
      if(mpGui->addButton("Load Model"))
      {
        std::string filename;
        if (openFileDialog(Model::kSupportedFileFormatsStr, filename))
        {
          mpModel = Model::createFromFile(filename.c_str());
        }
      }
      varsDirty = mpGui->addDropdown("Texture", kTexturesDropdown, textureIndex) || varsDirty;
      varsDirty = mpGui->addFloat3Var("LightDir", mLightDir) || varsDirty;

      break;
    default:
      should_not_get_here();
  }
}

void TerrainGeneration::CreateQuad()
{
	//Just in NDC. Not doing any transformation
	const Vertex kQuadVertices[] =
	{	//Only work for DX. See FullScreenPass.cpp line 60
		//for macro so that it also works with vulkan
		{ glm::vec2(-1, 1), glm::vec2(0, 0) },
		{ glm::vec2(-1, -1), glm::vec2(0, 1) },
		{ glm::vec2(1, 1), glm::vec2(1, 0) },
		{ glm::vec2(1, -1), glm::vec2(1, 1) },
	};

	//create VB
	const uint32_t vbSize = (uint32_t)(sizeof(Vertex)*arraysize(kQuadVertices));
	auto vertexBuffer = Buffer::create(vbSize, Buffer::BindFlags::Vertex, 
						Buffer::CpuAccess::Write, (void*)kQuadVertices);

	//create input layout
	VertexLayout::SharedPtr pLayout = VertexLayout::create();
	VertexBufferLayout::SharedPtr pBufLayout = VertexBufferLayout::create();
	pBufLayout->addElement("POSITION", 0, ResourceFormat::RG32Float, 1, 0);
	pBufLayout->addElement("TEXCOORD", 8, ResourceFormat::RG32Float, 1, 1);
	pLayout->addBufferLayout(0, pBufLayout);

  //create vao
	Vao::BufferVec buffers{ vertexBuffer };
	mpQuadVao = Vao::create(Vao::Topology::Patch, pLayout, buffers);
}

void TerrainGeneration::UpdateVars()
{
  if(varsDirty)
  {
    varsDirty = false;

    switch(mMode)
    {
      case Intro:
      {
        int size = sizeof(HullPerFrame);
        auto cbuf = mpTessIntroProgramVars->getConstantBuffer(0, 0, 0);
        cbuf->setBlob(&mHullPerFrame, 0, size);
        break;
      }
      case Displacement:
      {
        auto cbuf = mpDisplacementVars->getConstantBuffer(0, 0, 0);
        cbuf->setBlob(&mLightDir, 0, sizeof(vec3));
        mpDisplacementVars->setSrv(0, 0, 0, mNormalMaps[textureIndex]->getSRV());
        break;
      }
      default:
        should_not_get_here();
    }
  }
}

void TerrainGeneration::onLoad()
{
	//Full Screen Quad VAO
	CreateQuad();

  //load the textures
  LoadTextures();

	//Camera
	mpCamera = Camera::create();
	mCamController.attachCamera(mpCamera);

	//Intro to tess shader
  mpTessIntroProgram = GraphicsProgram::createFromFile(
		appendShaderExtension("TesellationIntro.vs"), 
		appendShaderExtension("TesellationIntro.ps"),
		"", 
		appendShaderExtension("TesellationIntro.hs"),
		appendShaderExtension("TesellationIntro.ds"));
  mpTessIntroProgramVars = GraphicsVars::create(mpTessIntroProgram->getActiveVersion()->getReflector());

  //Displacement map shader
  mpDisplacementProgram = GraphicsProgram::createFromFile(
    "",
    appendShaderExtension("Displacement.ps"));
  mpDisplacementVars = GraphicsVars::create(mpDisplacementProgram->getActiveVersion()->getReflector());

  //create default rasterizer state
  mpDefaultRS = RasterizerState::create(RasterizerState::Desc());
	//create wireframe rasterizer state
	RasterizerState::Desc wireframeDesc;
	wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
	wireframeDesc.setCullMode(RasterizerState::CullMode::None);
	mpWireframeRS = RasterizerState::create(wireframeDesc);

  //Graphics state
	mpGraphicsState = GraphicsState::create();
	mpGraphicsState->setRasterizerState(mpWireframeRS);
	mpGraphicsState->setVao(mpQuadVao);
}

void TerrainGeneration::onFrameRender()
{
	mpRenderContext->clearFbo(mpDefaultFBO.get(), mClearColor, 1.0f, 0, FboAttachmentType::All);
	mpGraphicsState->setFbo(mpDefaultFBO);
	mCamController.update();

  switch(mMode)
  {
    case Mode::Intro:
    {
	    if (mpQuadVao)
	    {
        UpdateVars();

		    mpGraphicsState->setProgram(mpTessIntroProgram);
		    mpRenderContext->pushGraphicsState(mpGraphicsState);
		    mpRenderContext->pushGraphicsVars(mpTessIntroProgramVars);
		    mpRenderContext->draw(4, 0);
        mpRenderContext->popGraphicsVars();
        mpRenderContext->popGraphicsState();
	    }
      break;
    }
    case Mode::Displacement:
    {
      if(mpModel)
      {
        varsDirty = varsDirty || mpCamera->isDirty();
        UpdateVars();

        mpGraphicsState->setProgram(mpDisplacementProgram);
        mpRenderContext->pushGraphicsState(mpGraphicsState);
        mpRenderContext->pushGraphicsVars(mpDisplacementVars);
        ModelRenderer::render(mpRenderContext.get(), mpModel, mpCamera.get()); 
        mpRenderContext->popGraphicsVars();
        mpRenderContext->popGraphicsState();
      }
      break;
    }
    default:
      should_not_get_here();
  }

	//Todo this is rendering double text. Not sure why.
	renderText(getFpsMsg(), glm::vec2(10, 30));
}

void TerrainGeneration::onShutdown()
{

}

bool TerrainGeneration::onKeyEvent(const KeyboardEvent& keyEvent)
{
	bool bHandled = mCamController.onKeyEvent(keyEvent);
	//Leaving this in for now, probably gonna want something similar
	//if (bHandled == false)
	//{
	//	if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
	//	{
	//		switch (keyEvent.key)
	//		{
	//		case KeyboardEvent::Key::R:
	//			resetCamera();
	//			bHandled = true;
	//			break;
	//		}
	//	}
	//}
	return bHandled;
}

bool TerrainGeneration::onMouseEvent(const MouseEvent& mouseEvent)
{
	return mCamController.onMouseEvent(mouseEvent);
}

void TerrainGeneration::onResizeSwapChain()
{
	float height = (float)mpDefaultFBO->getHeight();
	float width = (float)mpDefaultFBO->getWidth();

	mpCamera->setFocalLength(21.0f);
	float aspectRatio = (width / height);
	mpCamera->setAspectRatio(aspectRatio);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	TerrainGeneration tg;
	SampleConfig config;
	config.windowDesc.title = "Terrain Generation Project";
	config.windowDesc.resizableWindow = true;
	tg.run(config);
}
