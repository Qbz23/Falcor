static const float3 colors[6] = {
  float3(0.1f, 0.1f, 1.0f),
  float3(0.37f, 0.94f, 0.0f),
  float3(0.81f, 0.68f, 0.1f),
  float3(0.96f, 0.62f, 0.11f),
  float3(0.55f, 0.27f, 0.07f),
  float3(1.0f, 1.0f, 1.0f)
};

cbuffer PSPerFrame : register(b2)
{
  float3 eyePos;
  float maxHeight;
  float heightColorOffset;
}

float3 GetColorFromHeight(float height)
{
  height += heightColorOffset;
  //put height into 5 intervals
  height /= (maxHeight / 5.0f);
  //not deal with negative, anything below 0 is 0.
  height = max(height, 0); 
  float t = frac(height);
  int index = floor(height);
  if (index < 5)
  {
    return (1 - t) * colors[index] + t * colors[index + 1];
  }
  else
  {
    return colors[5];
  }
}

struct DS_OUT
{
	float4 pos : SV_POSITION;
  float3 posW : POSITION;
	float4 color : COLOR;
};

float4 main(DS_OUT dOut) : SV_TARGET
{
  float3 relativeEyePos = dOut.posW - eyePos;
  float3 normalW = normalize(cross(ddx(relativeEyePos), ddy(relativeEyePos)));
	return float4(GetColorFromHeight(dOut.posW.y) * dot(normalW, float3(0, -1, 0)), 1.0f);
}
