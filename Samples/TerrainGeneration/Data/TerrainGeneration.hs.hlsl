cbuffer HSPerFrame : register(b0)
{
  float3 eyePos;
  float minDistance;
  float maxDistance;
  int minTessFactor;
  int maxTessFactor;
};

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

int calcTessFactor(float3 pos)
{
  float dist = distance(pos, eyePos);
  float t = 1 - saturate((dist - minDistance) / (maxDistance - minDistance));
  return (int)lerp(minTessFactor, maxTessFactor, t);
}

HS_CONST_OUT HSConstant(InputPatch<VS_OUT, 4> ip, uint pid : SV_PrimitiveID)
{
	HS_CONST_OUT output;

  float3 left = (ip[0].posW + ip[2].posW) / 2.0f;
  float3 top = (ip[0].posW + ip[1].posW) / 2.0f;
  float3 right = (ip[1].posW + ip[3].posW) / 2.0f;
  float3 bot = (ip[2].posW + ip[3].posW) / 2.0f;
  float3 center = (top + bot) / 2.0f;
  
  output.edges[0] = calcTessFactor(left);
  output.edges[1] = calcTessFactor(top);
  output.edges[2] = calcTessFactor(right);
  output.edges[3] = calcTessFactor(bot);
  int centerTess = calcTessFactor(center);

	[unroll(2)]
	for (int j = 0; j < 2; ++j)
		output.inside[j] = centerTess;

	return output;
}
