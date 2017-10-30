struct VS_IN
{
	float4 pos : POSITION;
	float2 tex : TEXCOORD0;
};

struct VS_OUT
{
	float4 position : Position;
};

VS_OUT main(VS_IN vIn)
{
	VS_OUT output;
	output.position = float4(vIn.pos.xyz, 1);
	return output;
}
