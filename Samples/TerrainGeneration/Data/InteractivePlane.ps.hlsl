cbuffer PsPerFrame : register(b1)
{
  float2 clickCoords;
}

struct VS_OUT
{
	float4 pos : SV_POSITION;
  float2 tex : TEXCOORD0;
};

float4 main(VS_OUT vOut) : SV_TARGET
{
  float2 color = saturate(abs(vOut.tex - clickCoords));
  return float4(color, 0, 1);
}
