struct DS_OUT
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

float4 main(DS_OUT dOut) : SV_TARGET
{
	return dOut.color;
}
