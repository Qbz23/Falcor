

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
  output.pos = vIn.pos;
	//output.pos = mul(vIn.pos, viewProj);
  output.tex = vIn.tex;
	return output;
}
