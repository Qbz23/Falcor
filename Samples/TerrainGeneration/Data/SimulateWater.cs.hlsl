RWTexture2D heightTex;
RWTexture2D flowTex;

//just hardcoding an area here is fine 
//But ideally, should be able to calculate the "width"
//associated with 1 texel 
static const float area = 1.0f;
static const float density = 1.0f;
static const float dt = 0.000016f;

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

//Need 16(+1)x16(+1) groupshared array
// 16x16 block  
//Calculate newFlow.xyzw
//if not on the right edge 
//   Calc flow.x
//   if not on the bottom edge
//       Calc flow.y
//if not on the bottom edge 
//   Calc flow.z 
//   if not on the left edge 
//       Calc flow.w
//Calculate newFlow = (flow * fluidAccel + prevFlow) * damping facor
//damping is just friction type deal, see book
//fluid accel is const, see book. 
//  = (min(dt, minTimeStep) * pipeArea * gravity) / (pipeLength * colArea)
//Save your new flow into the groupshared memory 
//Also, save into prev texture. They use a structured buffer but i probs wont
//
//Group Memory Barrier with Group Sync
//Get height from left, top left, top, top right. 
//  height += (newFlow.x + .y + .z + .w)
//  height -= (left.x + topleft.y + top.z + 

static const uint groupWidth = 16;
static const uint paddedWidth = 1 + 16 + 1;

struct WaterData
{
  float4 flow;
  float height;
};

groupshared WaterData groupData[paddedWidth][paddedWidth];

//An extra thread row/col on both sides to handle calc of 
//cells adjacent to this groupW x groupW section we actually care about 
[numthreads(paddedWidth, paddedWidth, 1)]
void main(int3 groupID: SV_GroupID, uint3 threadID : SV_GroupThreadID)
{
  // i-1,j-1.y  j-1.z     i+1,j-1.W
  // i-1.x      SELF      X
  // W          Z         Y

  //-1 to offset the perimeter properly 
	int xIndex = groupWidth * groupID.x + (threadID.x - 1);
	int yIndex = groupWidth * groupID.y + (threadID.y - 1);
	int2 index = int2(xIndex, yIndex);
  
  //Save current data into groupshared memory
  float currentHeight = heightTex[index].x;
  //No check req here b/c out of bounds will just be 0 which is ok
  groupData[threadID.x][threadID.y].height = currentHeight;
  groupData[threadID.x][threadID.y].flow = flowTex[index];

  //Wait until every thread in the group saves data into groupshared memory
  GroupMemoryBarrierWithGroupSync();
  
  float4 newFlow = float4(0.f, 0.f, 0.f, 0.f);
  //If this isn't a cell on the right extra perimiter of this subgrid 
  //And also not on the right edge of the entire grid 
  //Dont want to update flow for stuff not in the actual texture, 
  //otherwise fluid would "leak" from the edges. Hence the additional 
  //check for index > -1 here.
  if(threadID.x < paddedWidth - 1 && /*index.x > -1 && */ index.x < 1023)
  { 
    float rightHeight = groupData[threadID.x + 1][threadID.y].height;
    newFlow.x = rightHeight - currentHeight;

    //Right confirmed ok in here. Need to check bot for bot right
    if(threadID.y < paddedWidth - 1 && /*index.y > -1 && */ index.y < 1023)
    {
      float botRightHeight = groupData[threadID.x + 1][threadID.y + 1].height;
      newFlow.y = botRightHeight - currentHeight;
    }
  }

  //Check the bottom edge, checked in nested above but if not right 
  //dont know if can do bot yet 
  if(threadID.y < paddedWidth - 1 && /*index.y > -1 &&*/ index.y < 1023)
  {
    float botHeight = groupData[threadID.x][threadID.y + 1].height;
    newFlow.z = botHeight - currentHeight;

    //Check left
    if(threadID.x > 0 && index.x > 0 /*&& index.x < 1023*/)
    {
      float botLeftHeight = groupData[threadID.x - 1][threadID.y + 1].height;
      newFlow.w = botLeftHeight - currentHeight;
    }
  }

  float4 prevFlow = groupData[threadID.x][threadID.y].flow;
  newFlow = newFlow * dt + prevFlow;
  groupData[threadID.x][threadID.y].flow = newFlow;

  //Wait until everyone calculates their flow
  GroupMemoryBarrierWithGroupSync();

  //Calc new height
  float height = currentHeight + newFlow.x + newFlow.y + newFlow.z + newFlow.w;
  //Dont need to bounds check, flow will be 0 oob
  //from left
  height -= groupData[threadID.x - 1][threadID.y].flow.x;
  //from top left
  height -= groupData[threadID.x - 1][threadID.y - 1].flow.y;
  //from top 
  height -= groupData[threadID.x][threadID.y - 1].flow.z;
  //From top Right
  height -= groupData[threadID.x + 1][threadID.y - 1].flow.w;
  
  if(threadID.x > 0 && threadID.x < paddedWidth - 1 &&
    threadID.y > 0 && threadID.y < paddedWidth - 1)
  {
    heightTex[index] = float4(height, 0, 0, 1);
    flowTex[index] = newFlow;
  }
}