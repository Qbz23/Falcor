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

bool GuiUint(Gui* pGui, const char* label, uint32_t* pUint, uint32_t min, uint32_t max)
{
    assert(*pUint <= INT_MAX && min >= 0 && max <= INT_MAX);

    int i = (int)*pUint;
    int iMin = (int)min;
    int iMax = (int)max;
    if (pGui->addIntVar(label, i, iMin, iMax))
    {
        *pUint = (uint32_t)i;
        return true;
    }
    else
    {
        return false;
    }
}

void GrfxSandbox::onGuiRender(SampleCallbacks* pSample, Gui* pGui)
{
    if (pGui->beginGroup("Grfx Sandbox"))
    {
        Gui::DropdownList modeList;
        modeList.push_back({ 0, "Hello Falcor" });
        modeList.push_back({ 1, "Edge Rendering" });
        modeList.push_back({ 2, "Voronoi" });
        modeList.push_back({ 3, "Mosaic" });
        modeList.push_back({ 4, "Caustics" });
        if (pGui->addDropdown("Mode", modeList, (uint32_t&)mMode))
        {
            if (mMode == Voronoi || mMode == Mosaic)
            {
                // redraw the voronoi if switching into this mode
                mVoronoiResources.bStale = true;
            }
        }

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
        case Voronoi:
        {
            updateVoronoiUI(pGui);
            break;
        }
        case Mosaic:
        {
            updateVoronoiUI(pGui);
            break;
        }
        case Caustics:
        {
            if (pGui->beginGroup("Fake Caustics"))
            {
                Gui::DropdownList mixList;
                mixList.push_back({ 0, "Multiply" });
                mixList.push_back({ 1, "Add" });
                mixList.push_back({ 2, "Min" });
                mixList.push_back({ 3, "Max" });
                mixList.push_back({ 4, "Avg" });
                if (pGui->addDropdown("Mix Type", mixList, mCausticResources.mixTypeIndex))
                {
                    std::string kAddDef = "_ADD";
                    std::string kMinDef = "_MIN";
                    std::string kMaxDef = "_MAX";
                    std::string kAvgDef = "_AVG";

                    Program::DefineList mixDefines;
                    mixDefines.add(kAddDef);
                    mixDefines.add(kMinDef);
                    mixDefines.add(kMaxDef);
                    mixDefines.add(kAvgDef);
                    mCausticResources.mpState->getProgram()->removeDefines(mixDefines);

                    switch (mCausticResources.mixTypeIndex)
                    {
                    case 1: mCausticResources.mpState->getProgram()->addDefine(kAddDef); break;
                    case 2: mCausticResources.mpState->getProgram()->addDefine(kMinDef); break;
                    case 3: mCausticResources.mpState->getProgram()->addDefine(kMaxDef); break;
                    case 4: mCausticResources.mpState->getProgram()->addDefine(kAvgDef); break;
                    }

                    pGui->endGroup();
                }

                pGui->addFloatVar("Color Split Offset", mCausticResources.mPerFrame.colorSplitOffset);
                pGui->addFloatVar("Intensity Scale", mCausticResources.mPerFrame.intensityScale);

                if (GuiUint(pGui, "Num Textures", &mCausticResources.numCausticTex, 1, 16))
                {
                    Program::DefineList numTexDefine;
                    numTexDefine.add("_NUM_CAUST_TEX", std::to_string(mCausticResources.numCausticTex));
                    mCausticResources.mpState->getProgram()->setDefines(numTexDefine);
                }


                GuiUint(pGui, "Tex Property UI Index", &mCausticResources.texUiIndex, 0, mCausticResources.numCausticTex - 1);

                vec4 causticVector = mCausticResources.mPerFrame.causticVector[mCausticResources.texUiIndex];
                vec2 causticScale = vec2(causticVector.x, causticVector.y);
                vec2 causticOffset = vec2(causticVector.z, causticVector.w);
                std::string indexStr = std::to_string(mCausticResources.texUiIndex);
                pGui->addFloat2Var((std::string("Caustic Scale") + indexStr).c_str(), causticScale, 0.01f);
                pGui->addFloat2Var((std::string("Caustic Offset") + indexStr).c_str(), causticOffset, 0.0f);
                mCausticResources.mPerFrame.causticVector[mCausticResources.texUiIndex] = vec4(
                        causticScale.x,
                        causticScale.y,
                        causticOffset.x,
                        causticOffset.y
                );
                float timeScale = mCausticResources.mPerFrame.timeScale[mCausticResources.texUiIndex].x;
                pGui->addFloatVar(
                    (std::string("Time Scale") + indexStr).c_str(),
                    timeScale,
                    0.f
                );
                mCausticResources.mPerFrame.timeScale[mCausticResources.texUiIndex].x = timeScale;
            }
            break;
        }
        }

        pGui->endGroup();
    }
}

//
// https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
//
float3 hsvToRgb(float h, float s, float v)
{
    int hI = (int)(h * 6);
    float hF = h * 6 - hI;

    float p = v * (1 - s);
    float q = v * (1 - hF * s);
    float t = (1 - (1 - hF) * s);

    switch (hI)
    {
    case 0: return float3(v, t, p);
    case 1: return float3(q, v, p);
    case 2: return float3(p, v, t);
    case 3: return float3(p, q, v);
    case 4: return float3(t, p, v);
    case 5: return float3(v, p, q);
    default: assert(false); return float3(-1, -1, -1);
    }
}

float2 randFloat2(float2 magnitude)
{
    float unitRandX = (float)rand() / RAND_MAX;
    float unitRandY = (float)rand() / RAND_MAX;
    // [0,1] -> [0, 2] -> [-1, 1]
    unitRandX = (unitRandX * 2.f) - 1.f;
    unitRandY = (unitRandY * 2.f) - 1.f;
    return float2(magnitude.x * unitRandX, magnitude.y * unitRandY);
}

void GrfxSandbox::updateVoronoiUI(Gui* pGui)
{
    if (pGui->beginGroup("Voronoi"))
    {
        pGui->addCheckBox("Constantly Update", mVoronoiResources.bConstantlyUpdate);

        if (!mVoronoiResources.bConstantlyUpdate)
        {
            if (pGui->addButton("Regenerate"))
            {
                mVoronoiResources.bStale = true;
            }
        }

        mVoronoiResources.bStale |= pGui->addCheckBox(
            "Manhattan Distance",
            mVoronoiResources.bManhattan);

        mVoronoiResources.bStale |= GuiUint(
            pGui,
            "NumCells",
            &mVoronoiResources.mNumCells,
            1,
            mVoronoiResources.mkMaxNumCells);
        float maxJitter = (1.f / (float)sqrt(mVoronoiResources.mNumCells)) * mVoronoiResources.kUiJitterScale;
        mVoronoiResources.bStale |= pGui->addFloat2Var(
            "Jitter",
            mVoronoiResources.jitter,
            0.f,
            maxJitter);

        // Only need color controls in voronoi mode, other modes are just using this as input
        if (mMode == Voronoi)
        {
            mVoronoiResources.bStale |= pGui->addFloatVar(
                "ColorSaturation",
                mVoronoiResources.colorSaturation,
                0.f,
                1.f);
            mVoronoiResources.bStale |= pGui->addFloatVar(
                "ColorValue",
                mVoronoiResources.colorValue,
                0.f,
                1.f);
        }

        pGui->endGroup();
    }
}

void GrfxSandbox::updateVoronoiInputs()
{
    // Update shader define
    Program::DefineList defines;
    defines.add("_NUM_SEED", std::to_string(mVoronoiResources.mNumCells));
    if (mMode == Voronoi)
    {
        defines.add("_COLOR_CELLS");
        if (mVoronoiResources.bManhattan)
        {
            defines.add("_MANHATTAN");
        }
    }
    else
    {
        mMosiacResources.mPerFrame.numCells = mVoronoiResources.mNumCells;
        defines.add("_MANHATTAN");
    }

    mVoronoiResources.mpVoronoiPass = FullScreenPass::create("Voronoi.slang", defines);
    mVoronoiResources.mpVars = GraphicsVars::create(mVoronoiResources.mpVoronoiPass->getProgram()->getActiveVersion()->getReflector());

    // Recreate buffers bc theyre based on reflector
    mVoronoiResources.mpSeedPoints = StructuredBuffer::create(
        mVoronoiResources.mpVoronoiPass->getProgram(),
        "SeedPoints",
        mVoronoiResources.mkMaxNumCells,
        Resource::BindFlags::ShaderResource);

    mVoronoiResources.mpCellColors = StructuredBuffer::create(
        mVoronoiResources.mpVoronoiPass->getProgram(),
        "Colors",
        mVoronoiResources.mkMaxNumCells,
        Resource::BindFlags::ShaderResource);

    // Points
    // Just start with evenly distributed points
    std::vector<float2> seedPoints(mVoronoiResources.mNumCells);
    uint32_t sideSize = (uint32_t)sqrt(mVoronoiResources.mNumCells);
    float delta = 1.0f / sideSize;
    // Dealing with really small numbers in UI sucks.
    float2 scaledJitter = float2(
            mVoronoiResources.jitter.x / mVoronoiResources.kUiJitterScale,
            mVoronoiResources.jitter.y / mVoronoiResources.kUiJitterScale);
    for (uint32_t i = 0; i < sideSize; ++i)
    {
        for (uint32_t j = 0; j < sideSize; ++j)
        {
            float x = (i + 0.5f) * delta;
            float y = (j + 0.5f) * delta;
            seedPoints[i * sideSize + j] = float2(x, y) + randFloat2(scaledJitter);
        }
    }
    mVoronoiResources.mpSeedPoints->setBlob(seedPoints.data(), 0, seedPoints.size() * sizeof(float2));

    // Colors
    std::vector<float3> colors(mVoronoiResources.mNumCells);
    float goldenRatioConjugate = 0.618033988749895f;
    float h = (float)rand();
    float s = (float)rand();
    float v = (float)rand();
    for (uint32_t i = 0; i < mVoronoiResources.mNumCells; ++i)
    {
        h += goldenRatioConjugate;
        h = fmodf(h, 1.f);
        colors[i] = hsvToRgb(h, mVoronoiResources.colorSaturation, mVoronoiResources.colorValue);
    }
    mVoronoiResources.mpCellColors->setBlob(colors.data(), 0, colors.size() * sizeof(float3));
}

void GrfxSandbox::onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext)
{
    srand((uint32_t)time(nullptr));

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

    // Voronoi Resources
    {
        // Shader is created in voronoi inputs fx, recreated to accomodate num cells

        mVoronoiResources.mpState = GraphicsState::create();

        // Tex to store the result and re-render it to main fbo 
        mVoronoiResources.mpColorTex = Texture::create2D(
            w,
            h,
            ResourceFormat::RGBA32Float,
            1u,
            1u,
            nullptr,
            ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource);
        mVoronoiResources.mpCellTex = Texture::create2D(
            w,
            h,
            ResourceFormat::R32Uint,
            1u,
            1u,
            nullptr,
            ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource);
        mVoronoiResources.mpFbo = Fbo::create();
        mVoronoiResources.mpFbo->attachColorTarget(mVoronoiResources.mpColorTex, 0);
        mVoronoiResources.mpFbo->attachColorTarget(mVoronoiResources.mpCellTex, 1);

        // Buffers of points and colors
        updateVoronoiInputs();
    }

    // Mosaic Resources
    {
        // Broken for now and idk why and its not my job to figure it out
        //
        // https://github.com/NVIDIAGameWorks/Falcor/issues/220
        //
        //mMosiacResources.mpState = ComputeState::create();
        //mMosiacResources.mpComputeShader = ComputeProgram::createFromFile("Mosaic.cs.hlsl", "main");
        //mMosiacResources.mpState->setProgram(mMosiacResources.mpComputeShader);
        //mMosiacResources.mpVars = ComputeVars::create(mMosiacResources.mpComputeShader->getReflector());
        //mMosiacResources.mpOutputTex = Texture::create2D(
        //    w,
        //    h,
        //    ResourceFormat::RGBA32Float,
        //    1u,
        //    1u,
        //    nullptr,
        //    ResourceBindFlags::UnorderedAccess
        //);

        //mMosiacResources.mPerFrame.textureDimensions = int2(w, h);
    }

    // Caustic Resources
    {
        mCausticResources.mpState = GraphicsState::create();
        mCausticResources.mpShader = GraphicsProgram::createFromFile("Caustics.slang", "", "ps");
        mCausticResources.mpState->setProgram(mCausticResources.mpShader);
        mCausticResources.mpVars = GraphicsVars::create(mCausticResources.mpShader->getReflector());

        Sampler::Desc samplerDesc;
        samplerDesc.setFilterMode(
            Sampler::Filter::Linear,
            Sampler::Filter::Linear,
            Sampler::Filter::Linear
        );
        samplerDesc.setAddressingMode(
            Sampler::AddressMode::Wrap,
            Sampler::AddressMode::Wrap,
            Sampler::AddressMode::Wrap
        );
        mCausticResources.mpSampler = Sampler::create(samplerDesc);
        mCausticResources.mpVars->setSampler("gSampler", mCausticResources.mpSampler);


        //const uint32_t kNumCausticTex = 2;
        //std::string causticTexPaths[kNumCausticTex] = {
        //    "caust_001.png",
        //    "caust_002.png"
        //};


        //
        // https://opengameart.org/content/water-caustics-effect-small
        //
        // Gonna pack all the tex into a tex2darray
        Texture::SharedPtr tempCausticTex[kMaxCausticTex] = { nullptr };
        uint32_t texW;
        uint32_t texH;
        for (uint32_t i = 0; i < kMaxCausticTex; ++i)
        {
            std::string texPath;
            if (i < 9)
            {
                texPath = "caust_00";
            }
            else
            {
                texPath = "caust_0";
            }
            texPath += std::to_string(i + 1) + ".png";

            tempCausticTex[i] = createTextureFromFile(texPath, true, false);
            uint32_t w = tempCausticTex[i]->getWidth();
            uint32_t h = tempCausticTex[i]->getHeight();
            if (i == 0)
            {
                texW = w;
                texH = h;
            }
            else
            {
                assert(texW == w && texH == h);
            }
        }
        mCausticResources.mpInTex = Texture::create2D(
            texW,
            texH,
            ResourceFormat::BGRX8Unorm,
            kMaxCausticTex, // arraysize
            Falcor::Texture::kMaxPossible, // num Mips
            nullptr, // inital data
            ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource
        );
        for (uint32_t i = 0; i < kMaxCausticTex; ++i)
        {
            uint32_t srcIndex = tempCausticTex[i]->getSubresourceIndex(0, 0);
            uint32_t dstIndex = mCausticResources.mpInTex->getSubresourceIndex(i, 0);
            pRenderContext->copySubresource(
                mCausticResources.mpInTex.get(),
                dstIndex,
                tempCausticTex[i].get(),
                srcIndex
            );
        }
        mCausticResources.mpInTex->generateMips(pRenderContext);

        // Initialize cb values
        vec2 causticScales[4] = {
            float2(1.0f, 1.0f),
            float2(0.5f, 1.0f),
            float2(1.0f, 0.5f),
            float2(0.5f, 0.5f)
        };
        vec2 causticOffsets[4] = {
            float2(0.1f, 0.1f),
            float2(0.25f, 0.5f),
            float2(0.5f, 0.25f),
            float2(0.25f, 0.25f)
        };
        float timeScales[8] = {
            0.04f,
            0.03f,
            0.03f,
            0.02f,
            0.02f,
            0.015f,
            0.015f,
            0.01f
        };
        for (uint32_t i = 0; i < kMaxCausticTex; ++i)
        {
            vec2 scale = causticScales[i % 4];
            vec2 offset = causticOffsets[i % 4];
            vec4 causticVec = vec4(scale.x, scale.y, offset.x, offset.y);
            float timeScale = timeScales[i % 8];
            mCausticResources.mPerFrame.causticVector[i] = causticVec;
            mCausticResources.mPerFrame.timeScale[i].x = timeScale;
        }


        mCausticResources.mpVars->setTexture("gCausticTex", mCausticResources.mpInTex);

        mCausticResources.mpOutTex = Texture::create2D(
            w,
            h,
            ResourceFormat::RGBA32Float,
            1u,
            1u,
            nullptr,
            ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource);
        mCausticResources.mpDepthBuffer = Texture::create2D(
            w,
            h,
            ResourceFormat::D32Float,
            1u,
            1u,
            nullptr,
            ResourceBindFlags::DepthStencil | ResourceBindFlags::ShaderResource);


        mCausticResources.mpFbo = Fbo::create();
        mCausticResources.mpFbo->attachColorTarget(mCausticResources.mpOutTex, 0);
        mCausticResources.mpFbo->attachDepthStencilTarget(mCausticResources.mpDepthBuffer, 0);
    }
}

void GrfxSandbox::renderVoronoi(RenderContext* pRenderContext)
{
    if (mVoronoiResources.bStale || mVoronoiResources.bConstantlyUpdate)
    {
        pRenderContext->clearFbo(mVoronoiResources.mpFbo.get(), float4(0.f, 0.f, 0.f, 0.f), 1.0f, 0, FboAttachmentType::All);

        // Re-set buffers based on input data
        updateVoronoiInputs();

        // Ensure Rt is in rt state
        pRenderContext->resourceBarrier(
            mVoronoiResources.mpFbo->getColorTexture(0).get(),
            Resource::State::RenderTarget);
        pRenderContext->resourceBarrier(
            mVoronoiResources.mpFbo->getColorTexture(1).get(),
            Resource::State::RenderTarget);

        // Update Data 
        mVoronoiResources.mpVars->setStructuredBuffer("SeedPoints", mVoronoiResources.mpSeedPoints);
        mVoronoiResources.mpVars->setStructuredBuffer("Colors", mVoronoiResources.mpCellColors);

        mVoronoiResources.mpState->setFbo(mVoronoiResources.mpFbo);
        pRenderContext->setGraphicsState(mVoronoiResources.mpState);
        pRenderContext->setGraphicsVars(mVoronoiResources.mpVars);

        mVoronoiResources.mpVoronoiPass->execute(pRenderContext);

        // This is lazy barrier-ing but whatever
        pRenderContext->resourceBarrier(
            mVoronoiResources.mpFbo->getColorTexture(0).get(),
            Resource::State::ShaderResource);
        pRenderContext->resourceBarrier(
            mVoronoiResources.mpFbo->getColorTexture(1).get(),
            Resource::State::ShaderResource);

        mVoronoiResources.bStale = false;
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
    case Voronoi:
    {
        //for debugging so you can capture the generation frame bc its every frame
        //mVoronoiResources.bStale = true; 
        renderVoronoi(pRenderContext);

        // Blit onto the render target regardless, bc its gonna get cleared
        pRenderContext->blit(
            mVoronoiResources.mpFbo->getColorTexture(0)->getSRV(),
            pTargetFbo->getColorTexture(0)->getRTV());

        break;
    }
    case Mosaic:
    {
        // Does not work, see onLoad.
        break;

        // Gonna need voronoi as an input
        renderVoronoi(pRenderContext);

        pRenderContext->resourceBarrier(
            mMosiacResources.mpOutputTex.get(),
            Resource::State::UnorderedAccess);

        auto cellTex = mVoronoiResources.mpCellTex->getSRV();
        auto colorTex = mCoreResources.mpFbo->getColorTexture(0)->getSRV();
        mMosiacResources.mpVars->setSrv(0, 0, 0, colorTex);
        mMosiacResources.mpVars->setSrv(0, 1, 0, cellTex);

        auto outTex = mMosiacResources.mpOutputTex->getUAV();
        mMosiacResources.mpVars->setUav(0, 0, 0, outTex);

        pRenderContext->setComputeState(mMosiacResources.mpState);
        pRenderContext->dispatch(1, 1, 1);

        pRenderContext->resourceBarrier(
            mMosiacResources.mpOutputTex.get(),
            Resource::State::ShaderResource);

        pRenderContext->blit(
            mMosiacResources.mpOutputTex->getSRV(),
            pTargetFbo->getColorTexture(0)->getRTV());

        break;
    }
    case Caustics:
    {
        pRenderContext->clearFbo(mCausticResources.mpFbo.get(), clearColor, 1.0f, 0);

        mCausticResources.mpState->setFbo(mCausticResources.mpFbo);
        pRenderContext->setGraphicsState(mCausticResources.mpState);

        mCausticResources.mPerFrame.time = pSample->getCurrentTime();
        auto cb = mCausticResources.mpVars->getConstantBuffer("PerFrameCB");

        cb->setBlob(&mCausticResources.mPerFrame, 0,
            sizeof(CausticResources::CausticPerFrame));
        pRenderContext->setGraphicsVars(mCausticResources.mpVars);

        mCoreResources.mpSceneRenderer->renderScene(pRenderContext);

        pRenderContext->blit(
            mCausticResources.mpOutTex->getSRV(),
            pTargetFbo->getColorTexture(0)->getRTV());

        break;
    };
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
