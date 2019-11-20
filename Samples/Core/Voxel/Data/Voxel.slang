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
}

struct PsOut
{
    float4 color : SV_TARGET0;
};

// Must be passed along from vertex shader
// This will fail on pso creation bc the signatures are
// incompatibler bc SV_InstanceID isnt provided from the vs
PsOut ps(VertexOut vOut, uint id : SV_InstanceID)
{
    PsOut psOut;

    UINT colorIndex = id % 3;
    float4 finalColor = float4(0.3, 0.3, 0.3, 1);
    switch(colorIndex)
    {
        case 0:
        {
            finalColor = float4(1.0f, 0.2f, 0.2f, 1.0f);
            break;
        }
        case 1:
        {
            finalColor = float4(0.2f, 1.0f, 0.2f, 1.0f);
            break;
        }
        case 2:
        {
            finalColor = float4(0.2f, 0.2f, 1.0f, 1.0f);
            break;
        }
        default:
        {
            finalColor = float4(0.2f, 0.0f, 0.0f, 1.0f);
            break;
        }
    }

    // unnecessary
    //ShadingData sd = prepareShadingData(vOut, gMaterial, gCamera.posW);


    //Diffuse 
    finalColor.rgb += float3(0.5f, 0.5f, 0.5f) * saturate(dot(normalize(lightDir), vOut.normalW));

    psOut.color = finalColor;
    return psOut;
}
