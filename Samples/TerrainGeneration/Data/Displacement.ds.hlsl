cbuffer DsPerFrame
{
  matrix mvp;
  float heightScale;
};

struct HS_CONST_OUT
{
	float edges[3]  : SV_TessFactor;
	float inside : SV_InsideTessFactor;
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

SamplerState gSampler;
texture2D gDisplacementMap : register(t2);

[domain("tri")]
VS_OUT main(
  HS_CONST_OUT input, 
  float3 baryCoords: SV_DomainLocation,
	const OutputPatch<VS_OUT, 3> tri)
{
  VS_OUT output;
  //so I can do += in loop
  output.normalW = float3(0, 0, 0);
  output.bitangentW = float3(0, 0, 0);
  output.texC = float2(0, 0);
  output.posW = float3(0, 0, 0);
  output.colorV = float3(0, 0, 0);
  output.prevPosH = float4(0, 0, 0, 1.0f);
  output.posH = float4(0, 0, 0, 1.0f);

  [unroll(3)]
  for (int i = 0; i < 3; ++i)
  {
    output.normalW += baryCoords[i] * tri[i].normalW;
    output.bitangentW += baryCoords[i] * tri[i].bitangentW;
    output.texC += baryCoords[i] * tri[i].texC;
    output.posW += baryCoords[i] * tri[i].posW;
    output.colorV += baryCoords[i] * tri[i].colorV;
    output.prevPosH += baryCoords[i] * tri[i].prevPosH;
    output.posH += baryCoords[i] * tri[i].posH;
  }

  float height = gDisplacementMap.SampleLevel(gSampler, output.texC, 0).x;
  output.posW += heightScale * height * output.normalW;
  output.posH = mul(float4(output.posW, 1.0f), mvp);
  return output;
}