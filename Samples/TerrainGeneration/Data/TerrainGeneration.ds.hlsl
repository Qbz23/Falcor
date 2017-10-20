//RN just copies of tessellation intro shaders

struct HS_OUT
{
	float4 position : Position;
};

struct HS_CONST_OUT
{
	float edges[4]  : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

struct DS_OUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

[domain("quad")]
DS_OUT main(HS_CONST_OUT input, float2 UV: SV_DomainLocation,
	const OutputPatch<HS_OUT, 4> patch)
{
	DS_OUT output;
	
	float3 topMid = lerp(patch[0].position.xyz, patch[1].position.xyz, UV.x);
	float3 botMid = lerp(patch[2].position.xyz, patch[3].position.xyz, UV.x);

	output.pos = float4(lerp(topMid, botMid, UV.y), 1);
	output.color = float4(UV.yx, 1 - UV.x, 1);

	return output;
}