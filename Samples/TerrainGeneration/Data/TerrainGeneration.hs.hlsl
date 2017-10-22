//RN just copies of tessellation intro shaders

//cbuffer HSPerFrame : register(b0)
//{
//  float4 edgeFactors;
//	float2 insideFactors;
//};

struct VS_OUT
{
	float3 posW : POS;
  float2 tex : TEXCOORD0;
};

struct HS_OUT
{
	float3 posW : POS;
  float2 tex : TEXCOORD0;
};

struct HS_CONST_OUT
{
	float edges[4]  : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HSConstant")]
HS_OUT main(InputPatch<VS_OUT, 4> ip, uint cpid : SV_OutputControlPointID, uint pid : SV_PrimitiveID)
{
	HS_OUT output;
	output.posW = ip[cpid].posW;
  output.tex = ip[cpid].tex;
	return output;
}

HS_CONST_OUT HSConstant(InputPatch<VS_OUT, 4> ip, uint pid : SV_PrimitiveID)
{
	HS_CONST_OUT output;

	[unroll(4)]
	for (int i = 0; i < 4; ++i)
		output.edges[i] = 32;//edgeFactors[i];

	[unroll(2)]
	for (int j = 0; j < 2; ++j)
		output.inside[j] = 32;//insideFactors[j];

	return output;
}
