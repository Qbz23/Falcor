


struct VS_INPUT
{
	float4 position   : POSITION;
	float4 color : COLOR;
};

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

cbuffer MVPConstBuff : register(b0)
{
	matrix world;
	matrix view;
	matrix proj;
}

PS_INPUT main( VS_INPUT input )
{
	PS_INPUT output;
	
	input.position.w = 1.0f;

	output.position = mul(input.position, world);
	output.position = mul(output.position, view);
	output.position = mul(output.position, proj);
	//output.position = mul(input.position, world);
	//output.position = mul(output.position, view);
	//output.position = mul(output.position, proj);

	//output.position = input.position;

	output.color = input.color;

	return output;
}