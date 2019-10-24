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
#pragma once
#include <Falcor.h>

using namespace Falcor;

// I wanna do watercolor npr
// This is achieved through tiling RT into voronoi
// See
// http://www.cs.cmu.edu/afs/cs.cmu.edu/academic/class/15463-f11/www/final_proj/www/mteh/
// https://en.wikipedia.org/wiki/Lloyd's_algorithm
// https://www.cc.gatech.edu/~turk/my_papers/npr_ar_2008.pdf < main paper, above are about voronoi step
// ^ above paper cites below 
// http://www.dgp.toronto.edu/papers/ahausner_SIGGRAPH2001.pdf
class GrfxSandbox : public Renderer
{
public:
    void onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext) override;
    void onFrameRender(SampleCallbacks* pSample, RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo) override;
    void onShutdown(SampleCallbacks* pSample) override;
    void onResizeSwapChain(SampleCallbacks* pSample, uint32_t width, uint32_t height) override;
    bool onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(SampleCallbacks* pSample, const MouseEvent& mouseEvent) override;
    void onDataReload(SampleCallbacks* pSample) override;
    void onGuiRender(SampleCallbacks* pSample, Gui* pGui) override;

private:

    enum Modes
    {
        HelloFalcor = 0,
        EdgeRender = 1,
        Voronoi = 2,
        Mosaic = 3,
        Caustics = 4, // https://www.alanzucconi.com/2019/09/13/believable-caustics-reflections/
        ModeCount = 5
    } mMode;

    struct CoreResources //For HelloFalcor and input to other modes
    {
        Scene::SharedPtr mpScene;
        SceneRenderer::SharedPtr mpSceneRenderer;

        GraphicsProgram::SharedPtr mpBasicShader;
        GraphicsState::SharedPtr mpState;
        GraphicsVars::SharedPtr mpVars;

        Texture::SharedPtr mpColorBuffer;
        Texture::SharedPtr mpNormalBuffer;
        Texture::SharedPtr mpDepthBuffer;
        Fbo::SharedPtr mpFbo;

        struct HelloPerFrame
        {
            vec3 lightDir = vec3(-0.25f, 0.75f, 1.0f);
            uint32_t colorSwapIndex = 0;
        } mPerFrame;

    } mCoreResources;

    struct EdgeResources
    {
        FullScreenPass::UniquePtr mpEdgePass;
        GraphicsState::SharedPtr mpState;
        GraphicsVars::SharedPtr mpVars;

        uint32_t mOperatorIndex = 0;
        struct EdgePerFrame
        {
            u32vec2 texDim;
            float normalThreshold = 1.25f;
            float depthThreshold = 0.001f;
        } mPerFrame;

    } mEdgeResources;

    struct VoronoiResources
    {
        const uint32_t mkMaxNumCells = 1024;
        uint32_t mNumCells = 64;
        bool bConstantlyUpdate = false;
        // This is only for visualization, auto true for intermediate
        bool bManhattan = false;
        float2 jitter = float2(7.5f, 7.5f);
        float colorSaturation = 0.5f;
        float colorValue = 0.95f;

        // dealing with small numbers in UI sucks. div by this
        const float kUiJitterScale = 100.0f;

        bool bStale = true;

        Texture::SharedPtr mpColorTex;
        Texture::SharedPtr mpCellTex;
        Fbo::SharedPtr mpFbo;

        FullScreenPass::UniquePtr mpVoronoiPass;
        GraphicsState::SharedPtr mpState;
        GraphicsVars::SharedPtr mpVars;
        StructuredBuffer::SharedPtr mpSeedPoints;
        StructuredBuffer::SharedPtr mpCellColors;
    } mVoronoiResources;
    // In voronoi case, pass target and render directly to it
    // In intermediate case, pass voronoi fbo
    void renderVoronoi(RenderContext* pRenderContext);
    void updateVoronoiUI(Gui* pGui);
    void updateVoronoiInputs();

    struct MosiacResources
    {
        ComputeState::SharedPtr mpState;
        ComputeVars::SharedPtr mpVars;
        ComputeProgram::SharedPtr mpComputeShader;
        Texture::SharedPtr mpOutputTex;

        struct MosiacPerFrame
        {
            int2 textureDimensions;
            uint numCells;
        } mPerFrame;
    } mMosiacResources;

    static const uint32_t kMaxCausticTex = 16;
    struct CausticResources
    {
        GraphicsState::SharedPtr mpState;
        GraphicsVars::SharedPtr mpVars;
        GraphicsProgram::SharedPtr mpShader;
        Texture::SharedPtr mpInTex;
        Texture::SharedPtr mpOutTex;
        Texture::SharedPtr mpDepthBuffer;
        Sampler::SharedPtr mpSampler;
        Fbo::SharedPtr mpFbo;
        uint32_t mixTypeIndex = 0;
        uint32_t numCausticTex = 2;
        uint32_t texUiIndex = 0;

        struct CausticPerFrame
        {
            vec3 lightDir = vec3(-0.25f, 0.75f, 1.0f);
            float time = 0.0f;
            vec4 causticVector[kMaxCausticTex];
            // hlsl won't let you have an array of float4 it pads it to float
            vec4 timeScale[kMaxCausticTex];
            float colorSplitOffset = 0.02f;
            float intensityScale = 1.25f;
        } mPerFrame;
    } mCausticResources;
};
