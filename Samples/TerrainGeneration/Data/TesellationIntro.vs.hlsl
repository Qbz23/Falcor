struct VS_IN
{
	float2 pos : POSITION;
	float2 tex : TEXCOORD0;
};

struct VS_OUT
{
	float4 position : Position;
};

VS_OUT main(VS_IN vIn)
{
	VS_OUT output;
	output.position = float4(vIn.pos, 0, 1);
	return output;
}
