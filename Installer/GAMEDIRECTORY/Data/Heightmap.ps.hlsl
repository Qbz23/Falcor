cbuffer PsPerFrame : register(b1)
{
  float2 clickCoords; //texcoord loc of click
  float radius; //distance from click to "paint"
  float heightIncrease; //how much to increase the height at point of click
  int sign; //whether height should be increased or decreased
}

SamplerState gSampler;
Texture2D gPrevHeightMap : register(t0);

struct VS_OUT
{
  float2 tex : TEXCOORD;
};

float4 main(VS_OUT vOut) : SV_TARGET
{
  float distance = length(vOut.tex - clickCoords);
  //Gives 1 at point of click, decreasing linearly until >= radius where it = 0
  float t = 1 - saturate(distance / radius);
  float heightChange = sign * heightIncrease * t; 
  //This can probably be a one channel texture
  float oldHeight = gPrevHeightMap.Sample(gSampler, vOut.tex).x;
  float newHeight = heightChange + oldHeight;
  return float4(newHeight, newHeight, newHeight, 1.0f);
}