//let's just get this in [0,1] so you can just use texc
StructuredBuffer<float2> SeedPoints;
StructuredBuffer<float3> Colors; 

// Brute force voronoi calc
//http://nullprogram.com/blog/2014/06/01/
float4 main(float2 texC : TEXCOORD) : SV_TARGET
{
    float smallestDistance = distance(SeedPoints[0], texC);
    float3 color = Colors[0];

#ifdef _NUM_SEED
    for (uint i = 1; i < _NUM_SEED; ++i)
#else
    for (uint i = 1; i < 64; ++i)
#endif
    {
        float currentDistance = distance(SeedPoints[i], texC);
        if (currentDistance < smallestDistance)
        {
            smallestDistance = currentDistance;
            color = Colors[i];
        }
    }

    return float4(color, 1.0f);
}
    
