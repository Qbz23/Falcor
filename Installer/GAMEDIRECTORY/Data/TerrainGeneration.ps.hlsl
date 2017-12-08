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

Texture2D gDirtTex;
Texture2D gRockyTex;
SamplerState gSampler;

float3 GetColorFromHeight(float height, float2 tex)
{
  height += heightColorOffset;
  //put height into 5 intervals
  height /= (maxHeight / 5.0f);
  //not deal with negative, anything below 0 is 0.
  height = max(height, 0); 
  float t = frac(height);
  int index = floor(height);

  float4 dirtColor = gDirtTex.Sample(gSampler, tex);
  float4 rockyColor = gRockyTex.Sample(gSampler, tex);

  if (index < 5)
  {
    //determine how much dirt/rock there should be 
    //below 2, complete dirt. 
    //2-4. lerp
    float texT = (index - 2) + t;
    //[-2, 3] -> [0, 3]
    texT = max(texT, 0);
    //[0, 3] -> [0, 1]
    texT *= 0.3333;
    float3 texColor = lerp(dirtColor.xyz, rockyColor.xyz, texT);
    float3 heightTint = lerp(colors[index], colors[index + 1], t);
    return texColor * heightTint;
  }
  else
  {
    //5 rocky
    return rockyColor.xyz * colors[5];
  }
}

struct DS_OUT
{
	float4 pos : SV_POSITION;
  float3 posW : POSITION;
	float4 color : COLOR;
  float2 tex : TEXCOORD;
};

float4 main(DS_OUT dOut) : SV_TARGET
{
  float3 relativeEyePos = dOut.posW - eyePos;
  float3 normalW = normalize(cross(ddx(relativeEyePos), ddy(relativeEyePos)));
	return float4(GetColorFromHeight(dOut.posW.y, dOut.tex) * dot(normalW, float3(0, -1, 0)), 1.0f);
}
