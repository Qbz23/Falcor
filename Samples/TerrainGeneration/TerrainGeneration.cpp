#include "TerrainGeneration.h"

void TerrainGeneration::onGuiRender()
{
	static const int intMax = std::numeric_limits<int>().max();

	mpGui->addFloat4Var("Clear Color", mClearColor, 0, 1.0f);

	//Edge factors
	//for (int i = 0; i < 4; ++i)
	//{
	//	std::string name = "Edge" + std::to_string(i);
	//	mpGui->addIntVar(name.c_str(), mHullPerFrame.edgeFactors[i], -intMax, intMax, 1, false);
	//}

	////Inside Factors 
	//for (int j = 0; j < 2; ++j)
	//{
	//	std::string name = "Inside" + std::to_string(j);
	//	mpGui->addIntVar(name.c_str(), mHullPerFrame.insideFactors[j], -intMax, intMax, 1, false);
	//}

	//Send up cbuffer
	int size = sizeof(HullPerFrame);
	auto cbuf = mpProgramVars->getConstantBuffer(0, 0, 0); 
	int edgeFactorSize = arraysize(mHullPerFrame.edgeFactors);
	cbuf->setBlob(&mHullPerFrame.edgeFactors[0], 0, sizeof(float));
	cbuf->setBlob(&mHullPerFrame.edgeFactors[1], sizeof(float), sizeof(float));
	cbuf->setBlob(&mHullPerFrame.edgeFactors[2], 2 * sizeof(float), sizeof(float));
	cbuf->setBlob(&mHullPerFrame.edgeFactors[3], 3 * sizeof(float), sizeof(float));

	//cbuf->setBlob(&mHullPerFrame.insideFactors, 4 * sizeof(float), arraysize(mHullPerFrame.edgeFactors));
	/*mpProgramVars->getConstantBuffer(0, 2, 0)->setVariableArray("edgeFactor",
		mHullPerFrame.edgeFactors, arraysize(mHullPerFrame.edgeFactors));
	mpProgramVars->getConstantBuffer(0, 2, 0)->setVariableArray("insideFactor",
		mHullPerFrame.insideFactors, arraysize(mHullPerFrame.insideFactors));*/
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
	mpQuadVertexBuffer = Buffer::create(vbSize, Buffer::BindFlags::Vertex, 
						Buffer::CpuAccess::Write, (void*)kQuadVertices);

	//create VAO
	VertexLayout::SharedPtr pLayout = VertexLayout::create();
	VertexBufferLayout::SharedPtr pBufLayout = VertexBufferLayout::create();
	pBufLayout->addElement("POSITION", 0, ResourceFormat::RG32Float, 1, 0);
	pBufLayout->addElement("TEXCOORD", 8, ResourceFormat::RG32Float, 1, 1);
	pLayout->addBufferLayout(0, pBufLayout);
	Vao::BufferVec buffers{ mpQuadVertexBuffer };
	mpQuadVao = Vao::create(Vao::Topology::Patch, pLayout, buffers);
}

void TerrainGeneration::onLoad()
{
	//Full Screen Quad VAO
	CreateQuad();

	//Camera
	mpCamera = Camera::create();
	mCamController.attachCamera(mpCamera);

	//Shader
	mpProgram = GraphicsProgram::createFromFile(
		appendShaderExtension("TesellationIntro.vs"), 
		appendShaderExtension("TesellationIntro.ps"),
		"", 
		appendShaderExtension("TesellationIntro.hs"),
		appendShaderExtension("TesellationIntro.ds"));

	//create a wireframe rasterizer state
	RasterizerState::Desc wireframeDesc;
	wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
	wireframeDesc.setCullMode(RasterizerState::CullMode::None);
	mpWireframeRS = RasterizerState::create(wireframeDesc);

	//other misc needed things 
	mpProgramVars = GraphicsVars::create(mpProgram->getActiveVersion()->getReflector());
	mpGraphicsState = GraphicsState::create();
	mpGraphicsState->setRasterizerState(mpWireframeRS);
	mpGraphicsState->setVao(mpQuadVao);
}

void TerrainGeneration::onFrameRender()
{
	mpRenderContext->clearFbo(mpDefaultFBO.get(), mClearColor, 1.0f, 0, FboAttachmentType::All);
	mpGraphicsState->setFbo(mpDefaultFBO);
	mCamController.update();

	if (mpQuadVao)
	{
		mpGraphicsState->setProgram(mpProgram);
		mpRenderContext->setGraphicsState(mpGraphicsState);
		mpRenderContext->setGraphicsVars(mpProgramVars);
		mpRenderContext->draw(4, 0);
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
