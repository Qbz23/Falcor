cbuffer VsPerFrame
{
  matrix rotation;
};

struct VS_IN
{
	float4 pos : POSITION;
	float2 tex : TEXCOORD0;
};

struct VS_OUT
{
	float4 pos : SV_POSITION;
  float2 tex : TEXCOORD0;
};

VS_OUT main(VS_IN vIn)
{
	VS_OUT output;
  output.pos = mul(vIn.pos, rotation);
  output.tex = vIn.tex;
	return output;
}
