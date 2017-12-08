cbuffer VsPerFrame : register(b0)
{
  matrix mvp; 
};

struct VS_IN
{
	float4 pos : POSITION;
};

struct VS_OUT
{
	float4 position : SV_POSITION;
};

VS_OUT main(VS_IN vIn)
{
	VS_OUT output;
	output.position = mul(mvp, vIn.pos);
	return output;
}
