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
#include "Voxel.h"

void Voxel::onGuiRender(SampleCallbacks* pSample, Gui* pGui)
{
    if (pGui->beginGroup("Voxels"))
    {
        std::string sSize = std::to_string(VoxelChunk::skChunkSideLength);
        pGui->addText(std::string("Chunk Size " + sSize + "x" + sSize + "x" + sSize).c_str());
        pGui->addInt3Var("Voxel Index", m_UIData.currentVoxelIndex, 0, VoxelChunk::skChunkSideLength - 1);

        m_UIData.currentVoxelColor = m_VoxelChunk.GetVoxelStatus(m_UIData.currentVoxelIndex);
        m_UIData.currentVoxelIsVisible = m_UIData.currentVoxelColor != 0;

        pGui->addCheckBox("Visible", m_UIData.currentVoxelIsVisible);
        if(m_UIData.currentVoxelIsVisible)
        {
            // This would be better as a dropdown
            pGui->addIntVar("ColorIndex", m_UIData.currentVoxelColor, 1, VoxelChunk::VoxelColor::VoxelColorCount - 1);
            m_VoxelChunk.SetVoxelStatus(
                (VoxelChunk::VoxelColor)m_UIData.currentVoxelColor,
                m_UIData.currentVoxelIndex.x,
                m_UIData.currentVoxelIndex.y,
                m_UIData.currentVoxelIndex.z
            );
        }
        else
        {
            m_VoxelChunk.SetVoxelStatus(
                VoxelChunk::VoxelColor::VoxelInvisible,
                m_UIData.currentVoxelIndex.x,
                m_UIData.currentVoxelIndex.y,
                m_UIData.currentVoxelIndex.z
            );
        }



        pGui->endGroup();
    }
    
}

void Voxel::Init()
{
    m_pCamera = Camera::create();

    // Taken from
    // http://cs.wellesley.edu/~cs307/readings/obj-ojects.html
    m_pCube = Model::createFromFile("RoyaltyFreeBox.obj");

    m_pScene = Scene::create();
    m_pScene->addCamera(m_pCamera);
    m_pSceneRenderer = SceneRenderer::create(m_pScene);

    m_pState = GraphicsState::create();
    auto shader = GraphicsProgram::createFromFile("Voxel.slang", "", "ps");
    m_pState->setProgram(shader);

    m_pVars = GraphicsVars::create(shader->getReflector());

    m_VoxelChunk.InitVoxels(m_pScene, m_pCube, true);
}

void Voxel::onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext)
{
    Init();
}

void Voxel::onFrameRender(SampleCallbacks* pSample, RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    const glm::vec4 clearColor(0.38f, 0.52f, 0.10f, 1);
    pRenderContext->clearFbo(pTargetFbo.get(), clearColor, 1.0f, 0, FboAttachmentType::All);
    m_pSceneRenderer->update(pSample->getCurrentTime());
    m_pState->setFbo(pTargetFbo);

    pRenderContext->setGraphicsState(m_pState);
    pRenderContext->setGraphicsVars(m_pVars);

    m_VoxelChunk.RenderVoxels(pRenderContext, m_pSceneRenderer);
}

void Voxel::onShutdown(SampleCallbacks* pSample)
{
}

bool Voxel::onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent)
{
    return m_pSceneRenderer->onKeyEvent(keyEvent);
}

bool Voxel::onMouseEvent(SampleCallbacks* pSample, const MouseEvent& mouseEvent)
{
    return m_pSceneRenderer->onMouseEvent(mouseEvent);
}

void Voxel::onDataReload(SampleCallbacks* pSample)
{
    Init();
}

void Voxel::onResizeSwapChain(SampleCallbacks* pSample, uint32_t width, uint32_t height)
{
    m_pCamera->setFocalLength(60.0f);
    m_pCamera->setAspectRatio((float)width / (float)height);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    Voxel::UniquePtr pRenderer = std::make_unique<Voxel>();
    SampleConfig config;
    config.windowDesc.title = "Voxel Engine";
    config.windowDesc.resizableWindow = true;
    Sample::run(config, pRenderer);
    return 0;
}
