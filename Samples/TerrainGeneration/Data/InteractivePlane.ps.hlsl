struct VS_OUT
{
	float4 pos : SV_POSITION;
  float2 tex : TEXCOORD0;
};

float4 main(VS_OUT vOut) : SV_TARGET
{
  return float4(vOut.tex, 0, 1);
}
