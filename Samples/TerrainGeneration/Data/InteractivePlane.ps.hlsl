cbuffer PsPerFrame : register(b1)
{
  float2 clickCoords;
}

struct DS_OUT
{
  float4 pos : SV_POSITION;
  float4 color : COLOR;
};

SamplerState gSampler;
Texture2D gHeightmap;

float4 main(DS_OUT vOut) : SV_TARGET
{
  //float3 color = gHeightmap.SampleLevel(gSampler, vOut.tex, 0).xyz * 0.01f;
  return vOut.color;
}
