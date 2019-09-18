//let's just get this in [0,1] so you can just use texc
StructuredBuffer<float2> SeedPoints;
StructuredBuffer<float3> Colors; 

float calcDistance(float2 a, float2 b)
{
#ifdef _MANHATTAN
    return abs(a.x - b.x) + abs(a.y - b.y);
#else
    return distance(a, b);
#endif
}

// Brute force voronoi calc
//http://nullprogram.com/blog/2014/06/01/
#ifdef _COLOR_CELLS
float4 main(float2 texC : TEXCOORD) : SV_TARGET0
#else
uint main(float2 texC : TEXCOORD) : SV_TARGET1
#endif
{
    float smallestDistance = calcDistance(SeedPoints[0], texC);
#ifdef _COLOR_CELLS
    float3 color = Colors[0];
#else
    uint index = 0;
#endif

#ifdef _NUM_SEED
    for (uint i = 1; i < _NUM_SEED; ++i)
#else
    for (uint i = 1; i < 64; ++i)
#endif
    {
        float currentDistance = calcDistance(SeedPoints[i], texC);
        if (currentDistance < smallestDistance)
        {
            smallestDistance = currentDistance;
#ifdef _COLOR_CELLS
            color = Colors[i];
#else
            index = i;
#endif
        }
    }

#ifdef _COLOR_CELLS
    return float4(color, 1.0f);
#else
    return index;
#endif
}
    
