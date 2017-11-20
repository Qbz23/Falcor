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
  float4 flowColor = gFlowTex.Sample(gSampler, dOut.uv);
  return float4(float3(0.1f, 0.1f, 0.4f) * dot(normalW, float3(0, -1, 0)), 1.0f);
  //return float4(flowColor.xyz * dot(normalW, float3(0, -1, 0)), 1.0f);
   //return float4(flowColor.xyz, 1.0f);
  //return float4(dOut.uv, 0, 1);
}
