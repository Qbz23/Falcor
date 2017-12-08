cbuffer HSPerFrame : register(b0)
{
  float4 edgeFactors;
	float2 insideFactors;
};

struct VS_OUT
{
	float4 position : Position;
};

struct HS_OUT
{
	float4 position : Position;
};

//#ifdef TRI
//static const int kNumEdges = 3;
//static const int kNumInside = 1;
//static const string kTessType = "tri";
//#else
static const int kNumEdges = 4;
static const int kNumInside = 2;
static const string kTessType = "quad";
//#endif

struct HS_CONST_OUT
{
  float edges[4] : SV_TessFactor;
  float inside[2] : SV_Inside
#endif
};

[domain("quad")]
[outputcontrolpoints(4)]
[partitioning("integer")]
[outputtopology("triangle_cw")]
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

	[unroll(kNumEdges)]
	for (int i = 0; i < 4; ++i)
		output.edges[i] = edgeFactors[i];

	[unroll(kNumInside)]
	for (int j = 0; j < 2; ++j)
		output.inside[j] = insideFactors[j];

	return output;
}
