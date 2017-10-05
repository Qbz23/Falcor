static const float3 colors[6] = {
  float3(0.6f, 0.96f, 0.69f),
  float3(0.37f, 0.74f, 0.25f),
  float3(0.81f, 0.68f, 0.1f),
  float3(0.96f, 0.62f, 0.11f),
  float3(0.55f, 0.27f, 0.07f),
  float3(1.0f, 1.0f, 1.0f)
  };

float3 GetColorFromHeight(float height)
{
  height += 0.5f; //to cover range [-0.5, 2.5]. Made easier moving to [0, 3]
  height /= 0.5f; //color intervals 0.5 each
  height = max(height, 0); //not deal with negative, anything below 0 is 0.
  float t = frac(height);
  int index = floor(height);
  if(index < 5)
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
};

float4 main(DS_OUT dOut) : SV_TARGET
{
  float3 normal = (cross(ddx(dOut.posW), ddy(dOut.posW)));
  if(!any(normal)) //if allcomponents are 0
    normal = float3(0, 1, 0);
  else
    normal = -normalize(normal);

  float3 elevationColor = GetColorFromHeight(dOut.posW.y);
  float diffuse = dot(normal, normalize(float3(0, 0.5f, 1)));
  return float4(diffuse * elevationColor, 1.0f);
}
