#include "TerrainGeneration.h"

void TerrainGeneration::onGuiRender()
{
	mpGui->addFloat4Var("Clear Color", mClearColor, 0, 1.0f);
}

void TerrainGeneration::onLoad()
{
	//Create needed things to view scene
	mpCamera = Camera::create();
	mCamController.attachCamera(mpCamera);
	mpProgram = GraphicsProgram::createFromFile("", appendShaderExtension("ModelViewer.ps"));
	mpModel = Model::createFromFile("teapot.obj", Model::LoadFlags::None);
	mpLight = DirectionalLight::create();

	//create a wireframe rasterizer state
	RasterizerState::Desc wireframeDesc;
	wireframeDesc.setFillMode(RasterizerState::FillMode::Wireframe);
	wireframeDesc.setCullMode(RasterizerState::CullMode::None);
	mpWireframeRS = RasterizerState::create(wireframeDesc);

	//other misc needed things 
	mpProgramVars = GraphicsVars::create(mpProgram->getActiveVersion()->getReflector());
	mpGraphicsState = GraphicsState::create();
	mpGraphicsState->setRasterizerState(mpWireframeRS);
	mpGraphicsState->setProgram(mpProgram);
}

void TerrainGeneration::onFrameRender()
{
	mpRenderContext->clearFbo(mpDefaultFBO.get(), mClearColor, 1.0f, 0, FboAttachmentType::All);
	mpGraphicsState->setFbo(mpDefaultFBO);
	mCamController.update();

	if (mpModel)
	{
		mpLight->setIntoConstantBuffer(mpProgramVars["PerFrameCB"].get(), "gDirLight");
		mpGraphicsState->setProgram(mpProgram);
		mpRenderContext->setGraphicsState(mpGraphicsState);
		mpRenderContext->setGraphicsVars(mpProgramVars);
		ModelRenderer::render(mpRenderContext.get(), mpModel, mpCamera.get());
	}

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
