Texture2D prevHeight;
RWTexture2D newHeight;
Texture2D prevFlow;
RWTexture2D newFlow; 

[numthreads(32, 32, 1)]
void main(int3 groupID: SV_GroupID, int3 threadID : SV_GroupThreadID)
{
	int xIndex = 32 * groupID.x + threadID.x;
	int yIndex = 32 * groupID.y + threadID.y;
	int2 index = int2(xIndex, yIndex);
	newHeight[index] = float4(1.0f, 1.0f, 1.0f, 1.0f);
	newFlow[index] = float4(yIndex / 64.0f, 10, 1, 1);
}