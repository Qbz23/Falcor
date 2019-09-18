SamplerState gSampler;
Texture2D gColorTex : register(t0);
Texture2D gCellTex: register(t1);

cbuffer PerFrame
{
  int2 textureDimensions;
  uint numCells;
};

RWTexture2D<float4> outColors : register(u0);

// 95% sure this could be better and actually use multiple threads
[numthreads(1, 1, 1)]
void main() //Dont need indexing when doing 1,1,1
{
    float3 cellColorSums[numCells];
    uint cellTexelCounts[numCells];

    for (uint i = 0; i < numCells; ++i)
    {
        cellColorSums[i] = 0;
        cellTexelCounts[i] = 0;
    }

    for (int i = 0; i < textureDimensions.x; ++i)
    {
        for (int j = 0; j < textureDimensions.y; ++j)
        {
            int2 index(i, j);
            uint cellIndex = gCellTex.Load(index);
            float4 texelColor = gColorTex.Load(index);

            cellColorSums[cellIndex] += texelColor;
            cellTexelCounts[cellIndex] += 1;
        }
    }

    for (uint i = 0; i < numCells; ++i)
    {
        float3 outColor = (float3)cellColorSums[i] / cellPixelCounts[i];
        outColors[i] = float4(outColor, 1.0f);
    }
}
