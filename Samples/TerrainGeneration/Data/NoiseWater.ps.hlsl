cbuffer PSPerFrame : register(b2)
{
  float3 eyePos;
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
	return float4(float3(0.1f, 0.1f, 1.0f) * dot(normalW, float3(0, -1, 0)), 1.0f);
}
