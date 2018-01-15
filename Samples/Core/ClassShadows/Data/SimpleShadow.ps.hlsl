#include "DefaultVS.slang"

float4 main(VS_OUT vOut) : SV_Target
{
  float nDotL = dot(float3(0, 1, 0), float3(0, -1, -1));
  return float4(nDotL, nDotL, nDotL, 1.0f);
}