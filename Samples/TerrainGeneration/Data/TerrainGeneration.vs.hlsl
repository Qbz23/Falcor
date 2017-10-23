struct VS_IN
{
	float4 pos : POSITION;
	float2 tex : TEXCOORD0;
};

struct VS_OUT
{
  float3 posW : POS;
  float2 tex : TEXCOORD0;
};

SamplerState gSampler;
Texture2D gHeightmap;

VS_OUT main(VS_IN vIn)
{
	VS_OUT output;
  output.tex = vIn.tex;
	output.posW = vIn.pos.xyz;
  output.posW.y = gHeightmap.SampleLevel(gSampler, vIn.tex, 0).x;
	return output;
}
