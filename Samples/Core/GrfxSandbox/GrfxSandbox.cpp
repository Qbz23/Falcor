/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#include "GrfxSandbox.h"

void GrfxSandbox::onGuiRender(SampleCallbacks* pSample, Gui* pGui)
{
    if (pGui->beginGroup("Grfx Sandbox"))
    {
        Gui::DropdownList modeList;
        modeList.push_back({ 0, "Hello Falcor" });
        modeList.push_back({ 1, "Edge Rendering" });
        pGui->addDropdown("Mode", modeList, (uint32_t&)mMode);

        switch (mMode)
        {
        case HelloFalcor:
        {
            if (pGui->beginGroup("Hello Falcor"))
            {
                pGui->addFloat3Slider("Light Dir", mCoreResources.mPerFrame.lightDir, -10, 10);

                Gui::DropdownList colorSwapList;
                colorSwapList.push_back({ 0, "RGB" });
                colorSwapList.push_back({ 1, "RBG" });
                colorSwapList.push_back({ 2, "GRB" });
                colorSwapList.push_back({ 3, "GBR" });
                colorSwapList.push_back({ 4, "BRG" });
                colorSwapList.push_back({ 5, "BGR" });
                pGui->addDropdown("Color Swap", colorSwapList, mCoreResources.mPerFrame.colorSwapIndex);

                pGui->endGroup();
            }

            break;
        }
        case EdgeRender:
        {
            if (pGui->beginGroup("Edge Rendering"))
            {
                Gui::DropdownList operatorList;
                operatorList.push_back({ 0, "Sobel" });
                operatorList.push_back({ 1, "Prewitt" });
                operatorList.push_back({ 2, "Scharr" });
                if (pGui->addDropdown("Operator", operatorList, mEdgeResources.mOperatorIndex))
                {
                    std::string kPrewittDef = "_PREWITT";
                    std::string kScharrDef = "_SCHARR";

                    // just remove everything and re-add what you want.
                    Program::DefineList imageOpDefines;
                    imageOpDefines.add(kPrewittDef);
                    imageOpDefines.add(kScharrDef);
                    mEdgeResources.mpEdgePass->getProgram()->removeDefines(imageOpDefines);

                    switch (mEdgeResources.mOperatorIndex)
                    {
                    case 1: // Prewitt
                    {
                        mEdgeResources.mpEdgePass->getProgram()->addDefine(kPrewittDef);
                        break;
                    }
                    case 2: //Scharr
                    {
                        mEdgeResources.mpEdgePass->getProgram()->addDefine(kScharrDef);
                        break;
                    }
                    }
                }

                pGui->addFloatVar("Normal Threshold", mEdgeResources.mPerFrame.normalThreshold, 0);
                pGui->addFloatVar("Depth Threshold", mEdgeResources.mPerFrame.depthThreshold, 0);


                pGui->endGroup();
            }

            break;
        }
        }

        pGui->endGroup();
    }
}

void GrfxSandbox::onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext)
{
    auto w = pSample->getWindow()->getClientAreaWidth();
    auto h = pSample->getWindow()->getClientAreaHeight();

    // Core Resources
    {
        mCoreResources.mpScene = Scene::loadFromFile("Arcade/Arcade.fscene");
        mCoreResources.mpSceneRenderer = SceneRenderer::create(mCoreResources.mpScene);
        mCoreResources.mpSceneRenderer->setCameraControllerType(SceneRenderer::CameraControllerType::FirstPerson);

        mCoreResources.mpState = GraphicsState::create();
        mCoreResources.mpBasicShader = GraphicsProgram::createFromFile("GrfxSandbox.slang", "", "ps");

        mCoreResources.mpState->setProgram(mCoreResources.mpBasicShader);
        mCoreResources.mpVars= GraphicsVars::create(mCoreResources.mpBasicShader->getActiveVersion()->getReflector());

        mCoreResources.mpColorBuffer = Texture::create2D(
            w,
            h,
            ResourceFormat::RGBA32Float,
            1u,
            1u,
            nullptr,
            ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource
        );
        mCoreResources.mpNormalBuffer = Texture::create2D(
            w,
            h,
            ResourceFormat::RGBA32Float,
            1u,
            1u,
            nullptr,
            ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource
        );
        mCoreResources.mpDepthBuffer = Texture::create2D(
            w,
            h,
            ResourceFormat::D32Float,
            1u,
            1u,
            nullptr,
            ResourceBindFlags::DepthStencil | ResourceBindFlags::ShaderResource
        );

        mCoreResources.mpFbo = Fbo::create();
        mCoreResources.mpFbo->attachColorTarget(mCoreResources.mpColorBuffer, 0);
        mCoreResources.mpFbo->attachColorTarget(mCoreResources.mpNormalBuffer, 1);
        mCoreResources.mpFbo->attachDepthStencilTarget(mCoreResources.mpDepthBuffer);
    }

    // Edge Resources
    {
        mEdgeResources.mpEdgePass = FullScreenPass::create("ImageEdge.ps.hlsl");
        mEdgeResources.mpState = GraphicsState::create();
        mEdgeResources.mpVars= GraphicsVars::create(mEdgeResources.mpEdgePass->getProgram()->getActiveVersion()->getReflector());

        mEdgeResources.mPerFrame.texDim.x = w;
        mEdgeResources.mPerFrame.texDim.y = h;
    }
}

void GrfxSandbox::onFrameRender(SampleCallbacks* pSample, RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    mCoreResources.mpSceneRenderer->update(pSample->getCurrentTime());

    const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    pRenderContext->clearFbo(mCoreResources.mpFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);

    mCoreResources.mpState->setFbo(mCoreResources.mpFbo);
    pRenderContext->setGraphicsState(mCoreResources.mpState);

    auto cb = mCoreResources.mpVars->getConstantBuffer("PerFrameCB");
    cb->setBlob(&mCoreResources.mPerFrame, 0, sizeof(CoreResources::HelloPerFrame));
    pRenderContext->setGraphicsVars(mCoreResources.mpVars);

    // Idk what other passes potentially do with these, put em in right state for core Gbuffer esque pass
    pRenderContext->resourceBarrier(
        mCoreResources.mpFbo->getColorTexture(0).get(),
        Resource::State::RenderTarget);
    pRenderContext->resourceBarrier(
        mCoreResources.mpFbo->getColorTexture(1).get(),
        Resource::State::RenderTarget);
    pRenderContext->resourceBarrier(
        mCoreResources.mpFbo->getDepthStencilTexture().get(),
        Resource::State::DepthStencil);

    mCoreResources.mpSceneRenderer->renderScene(pRenderContext);

    switch (mMode)
    {
    case HelloFalcor:
    {
        pRenderContext->blit(
            mCoreResources.mpFbo->getColorTexture(0)->getSRV(),
            pTargetFbo->getColorTexture(0)->getRTV());

        break;
    }
    case EdgeRender:
    {
        pRenderContext->resourceBarrier(
            mCoreResources.mpFbo->getColorTexture(0).get(),
            Resource::State::ShaderResource);
        pRenderContext->resourceBarrier(
            mCoreResources.mpFbo->getColorTexture(1).get(),
            Resource::State::ShaderResource);
        pRenderContext->resourceBarrier(
            mCoreResources.mpFbo->getDepthStencilTexture().get(),
            Resource::State::ShaderResource);

        auto colorTex = mCoreResources.mpFbo->getColorTexture(0)->getSRV();
        auto normalTex = mCoreResources.mpFbo->getColorTexture(1)->getSRV();
        auto depthTex = mCoreResources.mpFbo->getDepthStencilTexture()->getSRV();

        mEdgeResources.mpVars->setSrv(0, 0, 0, colorTex);
        mEdgeResources.mpVars->setSrv(0, 1, 0, normalTex);
        mEdgeResources.mpVars->setSrv(0, 2, 0, depthTex);

        mEdgeResources.mpVars->getConstantBuffer("PerFrame")->setBlob(&mEdgeResources.mPerFrame, 0, sizeof(EdgeResources::EdgePerFrame));

        mEdgeResources.mpState->setFbo(pTargetFbo);
        pRenderContext->setGraphicsState(mEdgeResources.mpState);
        pRenderContext->setGraphicsVars(mEdgeResources.mpVars);

        mEdgeResources.mpEdgePass->execute(pRenderContext);
        break;
    }
    }
}

void GrfxSandbox::onShutdown(SampleCallbacks* pSample)
{
}

bool GrfxSandbox::onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent)
{
    return mCoreResources.mpSceneRenderer ? mCoreResources.mpSceneRenderer->onKeyEvent(keyEvent) : false;
}

bool GrfxSandbox::onMouseEvent(SampleCallbacks* pSample, const MouseEvent& mouseEvent)
{
    return mCoreResources.mpSceneRenderer ? mCoreResources.mpSceneRenderer->onMouseEvent(mouseEvent) : false;
}

void GrfxSandbox::onDataReload(SampleCallbacks* pSample)
{

}

void GrfxSandbox::onResizeSwapChain(SampleCallbacks* pSample, uint32_t width, uint32_t height)
{
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    GrfxSandbox::UniquePtr pRenderer = std::make_unique<GrfxSandbox>();
    SampleConfig config;
    config.windowDesc.title = "Colavell Graphics Sandbox";
    config.windowDesc.resizableWindow = true;
    Sample::run(config, pRenderer);
    return 0;
}
