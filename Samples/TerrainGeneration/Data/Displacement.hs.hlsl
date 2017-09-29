cbuffer HSPerFrame : register(b1)
{
  float3 edgeFactors;
	float insideFactor;
};

struct VS_OUT
{
  float3 normalW    : NORMAL;
  float3 bitangentW : BITANGENT;
  float2 texC       : TEXCRD;
  float3 posW       : POSW;
  float3 colorV     : COLOR;
  float4 prevPosH : PREVPOSH;
  float4 posH : SV_POSITION;
};

struct HS_CONST_OUT
{
	float edges[3]  : SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSConstant")]
VS_OUT main(InputPatch<VS_OUT, 3> patch, 
            uint cpid : SV_OutputControlPointID, 
            uint pid : SV_PrimitiveID)
{
  VS_OUT output;
  output.normalW = patch[cpid].normalW;
  output.bitangentW = patch[cpid].bitangentW;
	output.texC = patch[cpid].texC;
  output.posW = patch[cpid].posW;
  output.colorV = patch[cpid].colorV;
  output.prevPosH = patch[cpid].prevPosH;
  output.posH = patch[cpid].posH;
	return output;
}

HS_CONST_OUT HSConstant(
  InputPatch<VS_OUT, 3> ip, 
  uint pid : SV_PrimitiveID)
{
	HS_CONST_OUT output;

	[unroll(3)]
  for (int i = 0; i < 3; ++i)
    output.edges[i] = edgeFactors[i];

  output.inside = insideFactor;

	return output;
}
