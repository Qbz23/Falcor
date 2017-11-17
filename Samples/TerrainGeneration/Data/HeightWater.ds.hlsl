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
	float2 uv : TEXCOORD;
};

Texture2D gHeightTex;
SamplerState gSampler;

float SampleKernel(float2 tex)
{
  //TODO fix this, actually do 1 texel, send up h/w
  const float dTex = 1.0f / 1081.0f;
  const float distances[5] = {-2, -1, 0, 1, 2};
  const float weights[25] = {
    0.003765f,	0.015019f,	0.023792f,	0.015019f,	0.003765f,
    0.015019f,	0.059912f,	0.094907f,	0.059912f,	0.015019f,
    0.023792f,	0.094907f,	0.150342f,	0.094907f,	0.023792f,
    0.015019f,	0.059912f,	0.094907f,	0.059912f,	0.015019f,  
    0.003765f,	0.015019f,	0.023792f,	0.015019f,	0.003765f};

  float result = 0.f;
  [unroll(5)]
  for(int i = 0; i < 5; ++i)
  {
    [unroll(5)]
    for(int j = 0; j < 5; ++j)
    {
      float2 sampleTex = tex + float2(dTex * distances[i], dTex * distances[j]);
      float sampleVal = gHeightTex.SampleLevel(gSampler, sampleTex, 0).x;
      result += weights[5 * i + j] * 25 * sampleVal;
    }
  }

  return result;
}

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

  float heightmapColor = SampleKernel(tex);
  output.pos.y += heightmapColor.x; // maxHeight * heightmapColor.x;
  output.posW = output.pos.xyz;
  output.pos = mul(output.pos, viewProj);

  output.uv = tex;
	return output;
}