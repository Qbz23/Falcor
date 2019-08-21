SamplerState gSampler;
Texture2D gColorTex : register(t0);
Texture2D gNormal : register(t1);
Texture2D gDepth : register(t2);

cbuffer PerFrame
{
  int2 textureDimensions;
  float normalThreshold;
  float depthThreshold;
};

static const int kNumSamples = 8;
#ifdef _PREWITT
static const int2 weights[kNumSamples] = {
  int2(-1, -1), int2(0, -1), int2(1, -1),
  int2(-1, 0), int2(1, 0),
  int2(-1, 1), int2(0, 1), int2(1, 1) };
#elif defined _SCHARR
static const int2 weights[kNumSamples] = {
  int2(3, 3), int2(0, 10), int2(-3, 3),
  int2(10, 0), int2(-10, 0),
  int2(3, -3), int2(0, -10), int2(-3, -3) };
#else //sobel
static const int2 weights[kNumSamples] = {
  int2(-1, 1), int2(0, 2), int2(1, 1),
  int2(-2, 0), int2(2, 0),
  int2(-1, -1), int2(0, -2), int2(1, -1) };
#endif
static const int2 offsets[kNumSamples] = {
  int2(-1, -1), int2(0, -1), int2(1, -1),
  int2(-1, 0), int2(1, 0),
  int2(-1, 1), int2(0, 1), int2(1, 1)};

float4 main(float2 texC : TEXCOORD) : SV_TARGET
{
  float2 depthGradient = float2(0, 0);
  float2 normalGradient = float2(0, 0);

  int2 iCoords = texC * textureDimensions;
  [unroll(kNumSamples)]
  for(int i = 0; i < kNumSamples; ++i)
  {
    int2 sampleCoords = iCoords + offsets[i];
    float3 normalSample = gNormal[sampleCoords].xyz;
    float depthSample = gDepth[sampleCoords].x;
    int2 weight = weights[i];
    normalGradient += dot(normalSample, float3(1, 1, 1)) * weight;
    depthGradient += depthSample * weight;
  }

  float normalMagnitude = sqrt(normalGradient.x * normalGradient.x + normalGradient.y * normalGradient.y);
  float depthMagnitude = sqrt(depthGradient.x * depthGradient.x + depthGradient.y * depthGradient.y);

  int edge = (normalMagnitude > normalThreshold) || (depthMagnitude > depthThreshold);
  if(edge)
  {
    return float4(0, 0, 0, 1);
  }
  else
  {
    //return gColorTex[iCoords];
    return float4(1, 1, 1, 1);
  }
}