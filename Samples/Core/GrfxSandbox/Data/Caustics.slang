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
__import ShaderCommon;
__import DefaultVS;
__import Shading;
__import Helpers;
__import BRDF;

Texture2DArray gCausticTex : register(t0);
SamplerState gSampler;

static const uint kMaxCaustTex = 16;

#ifndef _NUM_CAUST_TEX
#define _NUM_CAUST_TEX 2
#endif

layout(binding = 0) cbuffer PerFrameCB : register(b0)
{
    float3 lightDir;
    float time;
    float4 causticVectors[kMaxCaustTex]; // x = ScaleX
                                        // y = ScaleY
                                        // z = OffsetX
                                        // w = OffsetY
    float4 timeScales[kMaxCaustTex];
    float colorSplitOffset;
    float causticIntensityScale;
}

float3 sampleCaustic(float3 texC, float timeScale, float4 causticVector, float2 scrollDir)
{
    float2 uv = texC.xy * causticVector.xy + causticVector.zw;
    uv += timeScale * time * scrollDir;

    float3 uvr = float3((uv + float2(colorSplitOffset, colorSplitOffset)), texC.z);
    float3 uvg = float3((uv + float2(colorSplitOffset, -colorSplitOffset)), texC.z);
    float3 uvb = float3((uv + float2(-colorSplitOffset, -colorSplitOffset)), texC.z);

    float r = gCausticTex.Sample(gSampler, uvr).r;
    float g = gCausticTex.Sample(gSampler, uvg).g;
    float b = gCausticTex.Sample(gSampler, uvb).b;

    return float3(r, g, b) * (causticIntensityScale + 1);
}

float3 getCausticColor(float2 texC)
{
    const float2 scrollDirections[4] = {
        float2(1, 1),
        float2(1, -1),
        float2(-1, 1),
        float2(-1, -1),
    };

    float3 causticColor = float3(0.f, 0.f, 0.f);
    for(uint i = 0; i < _NUM_CAUST_TEX; ++i)
    {
        float2 scrollDirection = scrollDirections[i % 4];
        float4 causticVector = causticVectors[i];
        float timeScale = timeScales[i].x;

        float3 color = sampleCaustic(float3(texC, i), timeScale, causticVector, scrollDirection);
        if(i == 0)
        {
            causticColor = color;
        }
        else
        {
            causticColor = min(causticColor, color);
        }
    }

    return causticColor;
}

float4 ps(VertexOut vOut, float4 pixelCrd : SV_POSITION) : SV_TARGET0
{
    ShadingData sd = prepareShadingData(vOut, gMaterial, gCamera.posW);

    float3 causticColor = getCausticColor(vOut.texC);

    float4 finalColor = float4(0, 0, 0, 1);

    // Diffuse 
    finalColor.rgb += sd.diffuse;

    // outside of normal range messes up some mixes
    finalColor.rgb = saturate(finalColor.rgb);
    causticColor = saturate(causticColor);

#ifdef _ADD
    finalColor.rgb += causticColor;
#elif defined _MIN
    finalColor.r = min(finalColor.r, causticColor.r);
    finalColor.g = min(finalColor.g, causticColor.g);
    finalColor.b = min(finalColor.b, causticColor.b);
#elif defined _MAX
    finalColor.r = max(finalColor.r, causticColor.r);
    finalColor.g = max(finalColor.g, causticColor.g);
    finalColor.b = max(finalColor.b, causticColor.b);
#elif defined _AVG
    finalColor.rgb = 0.5f * (finalColor.rgb + causticColor);
#else // mult
    finalColor.rgb *= causticColor;
#endif

    finalColor.rgb *= dot(normalize(lightDir), vOut.normalW);

    // Add the emissive component
    finalColor.rgb += sd.emissive;
    
    return finalColor;
}
