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

cbuffer PsPerFrame : register(b0)
{
  float3 lightDir;
}
 
SamplerState gSampler;
Texture2D gDiffuseMap : register(t0);
Texture2D gNormalMap : register(t1);


float3 GetNormalSample(float3 normalW, float3 bitangentW, float2 uv)
{
  float3 tan = cross(normalW, bitangentW);
  float3x3 tbn = float3x3(tan, bitangentW, normalW);
  //sample and go from [0, 1] to [-1, 1]
  float3 normalTanSpace = 2 * gNormalMap.Sample(gSampler, uv).xyz - 1;
  return mul(normalTanSpace, tbn);
}

float4 main(VS_OUT vOut) : SV_TARGET
{
  float3 normal = GetNormalSample(vOut.normalW, vOut.bitangentW, vOut.texC);
  float diffuse = max(dot(normalize(normal), normalize(-lightDir)), 0);
  float3 diffuseColor = diffuse * gDiffuseMap.Sample(gSampler, vOut.texC).xyz;
  //uncomment to debug normals
  //back from [-1, 1] to [0, 1] for debug drawing
  //normal = (normal + 1) * 0.5f;
  //return float4(normal, 1.0f);
  //return float4(diffuse, diffuse, diffuse, 1.0f);
  return float4(diffuseColor, 1.0f);
}
