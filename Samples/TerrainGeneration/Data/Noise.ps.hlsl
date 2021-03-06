//Based on Ken Perlin's Java Reference Implementation

static const uint perlinValues[512] = { 151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
151,160,137,91,90,15,
131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 };

cbuffer PsPerFrame : register(b0)
{
  int octaves;
  float initialFreq;
  float amplitude;
  float time;
  int noiseInputScale;
};


float fade(float x)
{
  return x * x * x * (x * (x * 6 - 15) + 10);
}

float grad(int hash, float3 input)
{
  //The last 4 bits of the hash represent 12 gradient directions
  int h = hash & 15; //Get last 4 bits of the hash
                     //Sets properties of the gradient based on which bits in the hash are set.
  float u = h < 8 ? input.x : input.y;
  float v = h < 4 ? input.y : h == 12 || h == 14 ? input.x : input.z;
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float calcNoise(float3 input)
{
  //Find coords of unit cube that contains this input point
  int xCube = (int)floor(input.x) & 255;
  int yCube = (int)floor(input.y) & 255;
  int zCube = (int)floor(input.z) & 255;
  //Find the relative position within this unit cube
  float x = frac(input.x);
  float y = frac(input.y);
  float z = frac(input.z);
  //Compute Fade curves for xyz
  float u = fade(x);
  float v = fade(y);
  float w = fade(z);
  //Hash coordiunates of cube corners
  int A = perlinValues[xCube] + yCube;
  int AA = perlinValues[A] + zCube;
  int AB = perlinValues[A + 1] + zCube;
  int B = perlinValues[xCube + 1] + yCube;
  int BA = perlinValues[B] + zCube;
  int BB = perlinValues[B + 1] + zCube;

  //Return blended results from the 8 corners of the cube 
  float corner1 = grad(perlinValues[AA], float3(x, y, z));
  float corner2 = grad(perlinValues[BA], float3(x - 1, y, z));
  float corner3 = grad(perlinValues[AB], float3(x, y - 1, z));
  float corner4 = grad(perlinValues[BB], float3(x - 1, y - 1, z));
  float corner5 = grad(perlinValues[AA + 1], float3(x, y, z - 1));
  float corner6 = grad(perlinValues[BA + 1], float3(x - 1, y, z - 1));
  float corner7 = grad(perlinValues[AB + 1], float3(x, y - 1, z - 1));
  float corner8 = grad(perlinValues[BB + 1], float3(x - 1, y - 1, z - 1));
  float xLerpResult1 = lerp(corner1, corner2, u);
  float xLerpResult2 = lerp(corner3, corner4, u);
  float xLerpResult3 = lerp(corner5, corner6, u);
  float xLerpResult4 = lerp(corner7, corner8, u);
  float yLerpResult1 = lerp(xLerpResult1, xLerpResult2, v);
  float yLerpResult2 = lerp(xLerpResult3, xLerpResult4, v);
  return lerp(yLerpResult1, yLerpResult2, w);
}

struct VS_OUT
{
  float2 tex : TEXCOORD;
};

float4 main(VS_OUT vOut) : SV_TARGET
{
  float total = 0.0f;
  float frequency = initialFreq;
  for(int i = 0; i < octaves; ++i)
  {
    float3 adjustedInput = float3(noiseInputScale * vOut.tex, time) * frequency;
    total += calcNoise(adjustedInput) * amplitude;
    frequency *= 2;
  }
  //total /= octaves;
  total /= (octaves * amplitude);
  total = 0.5f * (total + 1);

  return float4(total, total, total, 1.0f);
}