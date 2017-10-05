cbuffer PsPerFrame : register(b1)
{
  float2 clickCoords;
}

struct VS_OUT
{
	float4 pos : SV_POSITION;
  float2 tex : TEXCOORD0;
};

SamplerState gSampler;
Texture2D gHeightmap;

float4 main(VS_OUT vOut) : SV_TARGET
{
  float3 color = gHeightmap.SampleLevel(gSampler, vOut.tex, 0).xyz * 0.01f;
  return float4(color, 1.0f);
}
