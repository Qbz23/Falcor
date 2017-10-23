cbuffer DSPerFrame : register(b1)
{
  matrix viewProj;
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

struct DS_OUT
{
	float4 pos : SV_POSITION;
  float3 posW : POSITION;
	float4 color : COLOR;
};

Texture2D gHeightmap;
SamplerState gSampler;

[domain("quad")]
DS_OUT main(HS_CONST_OUT input, float2 UV: SV_DomainLocation,
	const OutputPatch<HS_OUT, 4> patch)
{
	DS_OUT output;
	
  //get center of patch to use as pos
	float3 topMid = lerp(patch[0].posW, patch[1].posW, UV.x);
	float3 botMid = lerp(patch[2].posW, patch[3].posW, UV.x);
	output.pos = float4(lerp(topMid, botMid, UV.y), 1);

  float2 texTopMid = lerp(patch[0].tex, patch[1].tex, UV.x);
  float2 texBotMid = lerp(patch[2].tex, patch[3].tex, UV.x);
  float2 tex = lerp(texTopMid, texBotMid, UV.y);

  float4 heightmapColor = gHeightmap.SampleLevel(gSampler, tex, 0);
  output.pos.y += 50 * heightmapColor.x;
  output.posW = output.pos.xyz;
  output.pos = mul(output.pos, viewProj);
  
  //output.color = float4(tex.xy, UV.x, 1);
  output.color = heightmapColor;
	return output;
}