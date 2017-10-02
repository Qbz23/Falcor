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
  float padding;
  float3 eyePos;
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
  float3 relativeEyePos = vOut.posW - eyePos;
  float3 normalW = -normalize(cross(ddx(relativeEyePos), ddy(relativeEyePos)));
  float3 normal = GetNormalSample(normalW, -vOut.bitangentW, vOut.texC);

#ifdef WORLDNORMAL
  vOut.normalW = (vOut.normalW + 1) * 0.5f;
  return float4(vOut.normalW, 1.0f);
#endif

#ifdef CALCNORMAL
  normalW = (normalW + 1) * 0.5f;
  return float4(normalW, 1.0f);
#endif 

#ifdef SAMPLENORMAL
  normal = (normal + 1) * 0.5f;
  return float4(normal, 1.0f);
#endif 

#ifdef DIFFUSE
  float diffuse = max(dot(normalize(normalW), normalize(-lightDir)), 0);
  float3 diffuseColor = diffuse * gDiffuseMap.Sample(gSampler, vOut.texC).xyz;
  return float4(diffuseColor, 1.0f);
#endif

#ifdef DIFFUSENORMALMAP
  float diffuse = max(dot(normalize(normal), normalize(-lightDir)), 0);
  float3 diffuseColor = diffuse * gDiffuseMap.Sample(gSampler, vOut.texC).xyz;
  return float4(diffuseColor, 1.0f);
#endif 
}
