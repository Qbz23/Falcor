Texture2D gFlowTex;
SamplerState gSampler;

cbuffer PSPerFrame : register(b2)
{
  float3 eyePos;
}

struct DS_OUT
{
	float4 pos : SV_POSITION;
  float3 posW : POSITION;
	float2 uv : TEXCOORD;
};

float4 main(DS_OUT dOut) : SV_TARGET
{
  float3 relativeEyePos = dOut.posW - eyePos;
  float3 normalW = normalize(cross(ddx(relativeEyePos), ddy(relativeEyePos)));
  //float4 flowColor = gFlowTex.Sample(gSampler, 1.5f);
	//return float4(flowColor.xyz * dot(normalW, float3(0, -1, 0)), 1.0f);
  //return flowColor;
  return float4(dOut.uv, 0, 1);
}
