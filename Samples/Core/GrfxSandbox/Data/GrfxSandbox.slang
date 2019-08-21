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

layout(binding = 0) cbuffer PerFrameCB : register(b0)
{
    float3 lightDir;
    uint colorSwapIndex;
}

struct PsOut
{
    float4 color : SV_TARGET0;
    float4 normal : SV_TARGET1;
};

PsOut ps(VertexOut vOut, float4 pixelCrd : SV_POSITION)
{
    PsOut psOut;

    ShadingData sd = prepareShadingData(vOut, gMaterial, gCamera.posW);

    float4 finalColor = float4(0, 0, 0, 1);

    //Diffuse 
    finalColor.rgb += sd.diffuse * dot(normalize(lightDir), vOut.normalW);

    // Add the emissive component
    finalColor.rgb += sd.emissive;
    
    switch(colorSwapIndex)
    {
        case 1: //RBG
        {
            finalColor.rgb = finalColor.rbg;
            break;
        }
        case 2: //GRB
        {
            finalColor.rgb = finalColor.grb;
            break;
        }
        case 3: //GBR
        {
            finalColor.rgb = finalColor.gbr;
            break;
        }
        case 4: //BRG
        {
            finalColor.rgb = finalColor.brg;
            break;
        }
        case 5: //BGR
        {
            finalColor.rgb = finalColor.bgr;
            break;
        }
    }    

    psOut.color = finalColor;
    psOut.normal = float4(vOut.normalW * 0.5f + 0.5f, 1.0f);
    return psOut;
}
