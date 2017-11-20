Texture2D prevHeightTex;
RWTexture2D newHeightTex;
Texture2D prevFlowTex;
RWTexture2D newFlowTex; 

//just hardcoding an area here is fine 
//But ideally, should be able to calculate the "width"
//associated with 1 texel 
static const float area = 1.0f;
static const float density = 1.0f;
static const float dt = 0.00016f;

//clockwise from top left
static const int2 neighborOffset[8] = {
  int2(-1, -1), //TL
  int2(0, -1), //T
  int2(1, -1), //TR
  int2(1, 0), //R
  int2(1, 1), //BR 
  int2(0, 1), //B
  int2(-1, 1), //BL
  int2(-1, 0) //L 
  };

[numthreads(32, 32, 1)]
void main(int3 groupID: SV_GroupID, int3 threadID : SV_GroupThreadID)
{
  // i-1,j-1.y  j-1.z     i+1,j-1.W
  // i-1.x      SELF      X
  // W          Z         Y

	int xIndex = 32 * groupID.x + threadID.x;
	int yIndex = 32 * groupID.y + threadID.y;
	int2 index = int2(xIndex, yIndex);

  if(xIndex == 0 || yIndex == 0 || xIndex >= 1023 || yIndex >= 1023)
    return;

  //This could be significantly improved first step put "neighborhood"
  //that this thread group deals with in groupshared memory 
  float4 storedFlow = prevFlowTex[index];
  float flowFromLeft = prevFlowTex[index + int2(-1, 0)].x;
  float flowFromTopLeft = prevFlowTex[index + int2(-1, -1)].y;
  float flowFromTop = prevFlowTex[index + int2(0, -1)].z;
  float flowFromTopRight = prevFlowTex[index + int2(1, -1)].w;

  //clockwise from topLeft
  float prevFlow[8] = {
    -flowFromTopLeft, -flowFromTop, -flowFromTopRight,
    storedFlow.x, storedFlow.y, storedFlow.z, storedFlow.w,
    -flowFromTopRight };

  float height = prevHeightTex[index].x;
  float flowResult[8];
  for(int i = 0; i < 8; ++i)
  {
    int2 neighborIndex = index + neighborOffset[i];
    float neighborHeight = prevHeightTex[neighborIndex].x;
    float vol = area * height;
    float deltaFlow = (area * (neighborHeight - height)) / (density * vol);

    flowResult[i] = prevFlow[i] + dt * area * deltaFlow;
    //flowResult[i] = deltaFlow;
    //flowResult[i] = vol;
  }

  float deltaVol = 0;
  for(int j = 0; j < 8; ++j)
  {
    deltaVol += (prevFlow[j] + flowResult[j]) / 2.0f;
  }
  deltaVol *= dt;

	//newHeightTex[index] = float4(xIndex / 64.0f, 0.f, 1.0f, 1.0f);
  newHeightTex[index] = float4(height + deltaVol, 0, 0, 1);
	//newFlow[index] = float4(xIndex / 64.0f, yIndex / 64.0f, 1, 1);
  newFlowTex[index] = float4(flowResult[3], flowResult[4], flowResult[5], flowResult[6]);
}