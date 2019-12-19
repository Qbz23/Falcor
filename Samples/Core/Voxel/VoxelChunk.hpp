#pragma once
#include "Falcor.h"

using namespace Falcor;

class VoxelChunk
{
public:
    enum VoxelColor
    {
        VoxelInvisible = 0,
        VoxelColorWhite = 1,    //
        VoxelColorRed = 2,      //
        VoxelColorGreen = 3,    //
        VoxelColorBlue = 4,     //
        VoxelColorYellow = 5,   //
        VoxelColorMagenta = 6,  //
        VoxelColorTeal = 7,
        VoxelColorBrown = 8,    // 128, 64, 0
        VoxelColorPurple = 9,   // 64, 0, 128
        VoxelColorCount = 10
    };

    static const uint32_t skChunkSideLength = 8;
    const float kVoxelScale = 0.25f;

    VoxelColor GetVoxelStatus(ivec3 index) const
    {
        return m_VoxelStatus[index.x][index.y][index.z];
    }

    void SetVoxelStatus(VoxelColor vc, uint32_t i, uint32_t j, uint32_t k)
    {
        assert(i < skChunkSideLength && j < skChunkSideLength && k < skChunkSideLength);
        m_VoxelStatus[i][j][k] = vc;
    }

    void InitVoxels(Scene::SharedPtr pScene, Model::SharedPtr pModel, bool bDebugSetup = false)
    {
        float minCoord = -(skChunkSideLength * kVoxelScale) / 2.f;
        vec3 basePosition(minCoord, minCoord, minCoord);

        for (uint i = 0; i < skChunkSideLength; ++i)
        {
            for (uint j = 0; j < skChunkSideLength; ++j)
            {
                for (uint k = 0; k < skChunkSideLength; ++k)
                {
                    vec3 pos = basePosition;
                    pos.x += 2 * kVoxelScale * (i % skChunkSideLength);
                    pos.y += 2 * kVoxelScale * (j % skChunkSideLength);
                    pos.z += 2 * kVoxelScale * (k % skChunkSideLength);

                    pScene->addModelInstance(
                        pModel,
                        "Voxel",
                        pos,                                        // Trans
                        vec3(),                                     // Rot
                        vec3(kVoxelScale, kVoxelScale, kVoxelScale) // Scale
                    );

                    if (bDebugSetup)
                    {
                        VoxelCoord vc(i, j, k);
                        VoxelColor vCol;
                        if (k == 7)
                        {
                            // This is the camera facing depth slice
                            // making it yellow so I can turn it into a smiley
                            // face as a UI test 
                            vCol = VoxelColor::VoxelColorYellow;
                        }
                        else
                        {
                            vCol = (VoxelColor)
                                ((vc.GetModelIndex() % (VoxelColorCount - 1)) + 1);
                        }
                        m_VoxelStatus[i][j][k] = vCol;
                    }
                }
            }
        }
    }

    void RenderVoxels(RenderContext* pRc, SceneRenderer::SharedPtr pSr)
    {
        std::vector<std::vector<VoxelCoord>> drawList;
        drawList.resize(VoxelColorCount);

        Scene::SharedPtr pScene = pSr->getScene();

        for (uint i = 0; i < skChunkSideLength; ++i)
        {
            for (uint j = 0; j < skChunkSideLength; ++j)
            {
                for (uint k = 0; k < skChunkSideLength; ++k)
                {
                    uint32_t modelInstanceId =
                        skChunkSideLength * skChunkSideLength * i +
                        skChunkSideLength * j + k;
                    auto pModelInstance = pScene->getModelInstance(0, modelInstanceId);
                    pModelInstance->setVisible(false);
                    VoxelColor voxelStatus = m_VoxelStatus[i][j][k];

                    if (voxelStatus != VoxelInvisible)
                    {
                        drawList[voxelStatus].emplace_back(i, j, k);
                    }
                }
            }
        }

        auto pVars = pRc->getGraphicsVars();
        auto pCb = pVars->getConstantBuffer("VoxelData");
        for (uint i = 1; i < VoxelColorCount; ++i)
        {
            if (!drawList[i].empty())
            {
                // Set color
                pCb->setBlob(&m_VoxelColors[i], 0, sizeof(vec3));

                // Set visibility
                for(auto it = drawList[i].begin(); it != drawList[i].end(); ++it)
                {
                    auto pModelInstance = pScene->getModelInstance(0, it->GetModelIndex());
                    pModelInstance->setVisible(true);
                }

                pSr->renderScene(pRc);

                // Set invisible again
                for (auto it = drawList[i].begin(); it != drawList[i].end(); ++it)
                {
                    auto pModelInstance = pScene->getModelInstance(0, it->GetModelIndex());
                    pModelInstance->setVisible(false);
                }
            }
        }
    }

private:
    // TODO replace with ivec3 
    struct VoxelCoord
    {
        VoxelCoord(uint32_t _x, uint32_t _y, uint32_t _z) :
            x(_x), y(_y), z(_z) {};
        uint32_t x;
        uint32_t y;
        uint32_t z;
        uint32_t GetModelIndex() const
        {
            return skChunkSideLength * skChunkSideLength * x +
                skChunkSideLength * y + z;
        }
    };

    vec3 m_VoxelColors[VoxelColorCount] = {
        vec3(0.f, 0.f, 0.f), // black but wont be drawn (invisible)
        vec3(1.f, 1.f, 1.f), // white 
        vec3(1.f, 0.f, 0.f), // red 
        vec3(0.f, 1.f, 0.f), // green 
        vec3(0.f, 0.f, 1.f), // blue
        vec3(1.f, 1.f, 0.f), // yellow
        vec3(1.f, 0.f, 1.f), // magenta
        vec3(0.f, 1.f, 1.f), // teal
        vec3(0.5f, 0.25f, 0.f), // brown
        vec3(0.25f, 0.f, 0.5f), // purple
    };

    VoxelColor m_VoxelStatus[skChunkSideLength][skChunkSideLength][skChunkSideLength] = { VoxelInvisible };

};
