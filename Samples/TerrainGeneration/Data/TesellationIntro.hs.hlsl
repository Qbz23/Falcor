cbuffer HSPerFrame : register(b0)
{
	float edgeFactor0;
	float edgeFactor1;
	float edgeFactor2;
	float edgeFactor3;
	float insideFactor[2];
};

struct VS_OUT
{
	float4 position : Position;
};

struct HS_OUT
{
	float4 position : Position;
};

struct HS_CONST_OUT
{
	float edges[4]  : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSConstant")]
HS_OUT main(InputPatch<VS_OUT, 4> ip, uint cpid : SV_OutputControlPointID, uint pid : SV_PrimitiveID)
{
	HS_OUT output;
	output.position = ip[cpid].position;
	return output;
}

HS_CONST_OUT HSConstant(InputPatch<VS_OUT, 4> ip, uint pid : SV_PrimitiveID)
{
	HS_CONST_OUT output;

	//[unroll(4)]
	//for (int i = 0; i < 4; ++i)
	//	output.edges[i] = edgeFactor[i];
	output.edges[0] = edgeFactor0;
	output.edges[1] = edgeFactor1;
	output.edges[2] = edgeFactor2;
	output.edges[3] = edgeFactor3;

	[unroll(2)]
	for (int j = 0; j < 2; ++j)
		output.inside[j] = 4;

	return output;
}
