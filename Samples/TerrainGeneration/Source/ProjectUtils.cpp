#include "ProjectUtils.h"

void ProjectUtils::onLoad()
{
  //Wireframe RS
  RasterizerState::Desc rsDesc = RasterizerState::Desc();
  rsDesc.setFillMode(RasterizerState::FillMode::Wireframe);
  mpWireframeRs = RasterizerState::create(rsDesc);

  //Trilinear Wrap Sampler
  Sampler::Desc samplerDesc;
  samplerDesc.setFilterMode(
    Sampler::Filter::Linear,
    Sampler::Filter::Linear,
    Sampler::Filter::Linear);
  samplerDesc.setAddressingMode(
    Sampler::AddressMode::Wrap,
    Sampler::AddressMode::Wrap,
    Sampler::AddressMode::Wrap);
  mpTrilinearWrapSampler = Sampler::create(samplerDesc);
  //Trilinear clamp sampler
  samplerDesc.setAddressingMode(
    Sampler::AddressMode::Clamp,
    Sampler::AddressMode::Clamp,
    Sampler::AddressMode::Clamp);
  mpTrilinearClampSampler = Sampler::create(samplerDesc);

  //Quad vao
  mpUnitQuadPatchVao = CreateUnitQuadPatchVao();
}

void ProjectUtils::onShutdown()
{

}

std::pair<Vao::SharedPtr, uint32_t> ProjectUtils::GetGridPatchVao(int numRows, int numCols, float patchSize)
{
  const float halfW = (numCols * patchSize) / 2.0f;
  const float halfH = (numRows * patchSize) / 2.0f;
  const float deltaU = 1.0f / (numCols - 1);
  const float deltaV = 1.0f / (numRows - 1);

  std::vector<PatchVertex> verts;
  verts.resize(numRows * numCols);
  for (int i = 0; i < numRows; ++i)
  {
    float zPos = -halfH + patchSize * i;
    for (int j = 0; j < numCols; ++j)
    {
      float xPos = -halfW + patchSize * j;

      PatchVertex newVert;
      newVert.uv = vec2(j * deltaU, i * deltaV);
      newVert.pos = vec4(xPos, 0, zPos, 1.0f);

      verts[numCols * i + j] = newVert;
    }
  }

  //Get IB
  auto ib = CreatePatchIndexBuffer(numRows, numCols);

  //create VB
  const uint32_t vbSize = (uint32_t)(sizeof(PatchVertex)*verts.size());
  auto vertexBuffer = Buffer::create(vbSize, Buffer::BindFlags::Vertex,
    Buffer::CpuAccess::Write, (void*)verts.data());

  //create input layout
  VertexLayout::SharedPtr pLayout = VertexLayout::create();
  VertexBufferLayout::SharedPtr pBufLayout = VertexBufferLayout::create();
  pBufLayout->addElement("POSITION", 0, ResourceFormat::RGBA32Float, 1, 0);
  pBufLayout->addElement("TEXCOORD", 16, ResourceFormat::RG32Float, 1, 1);
  pLayout->addBufferLayout(0, pBufLayout);

  //create vao
  Vao::BufferVec buffers{ vertexBuffer };
  //Return pair of vao and index count
  return std::make_pair(
    Vao::create(Vao::Topology::Patch4, pLayout, buffers, ib, Falcor::ResourceFormat::R32Uint),
    4 * (numRows - 1) * (numCols - 1));
}

Buffer::SharedPtr ProjectUtils::CreatePatchIndexBuffer(int numRows, int numCols)
{
  int numFaces = (numRows - 1) * (numCols - 1);
  std::vector<uint32_t> indices;
  indices.resize(4 * numFaces);

  //index buffer index
  int k = 0;
  for (int i = 0; i < numRows - 1; ++i)
  {
    for (int j = 0; j < numCols - 1; ++j)
    {
      //top left
      indices[k] = i * numCols + j;
      //top right
      indices[k + 1] = indices[k] + 1;
      //bot right
      indices[k + 3] = (i + 1) * numCols + j + 1;
      //bot left
      indices[k + 2] = (i + 1) * numCols + j;

      k += 4;
    }
  }

  return Buffer::create(
    4 * numFaces * sizeof(uint32_t),
    Buffer::BindFlags::Index,
    Buffer::CpuAccess::None,
    indices.data());
}

Vao::SharedPtr ProjectUtils::CreateUnitQuadPatchVao()
{
  //Just in NDC. Not doing any transformation
  const PatchVertex kQuadVertices[] =
  {	
    { glm::vec4(-1, 1, 0, 1), glm::vec2(0, 0) },
    { glm::vec4(-1, -1, 0, 1), glm::vec2(0, 1) },
    { glm::vec4(1, 1, 0, 1), glm::vec2(1, 0) },
    { glm::vec4(1, -1, 0, 1), glm::vec2(1, 1) },
  };

  //create VB
  const uint32_t vbSize = (uint32_t)(sizeof(PatchVertex)*arraysize(kQuadVertices));
  auto vertexBuffer = Buffer::create(vbSize, Buffer::BindFlags::Vertex,
    Buffer::CpuAccess::Write, (void*)kQuadVertices);

  //create input layout
  VertexLayout::SharedPtr pLayout = VertexLayout::create();
  VertexBufferLayout::SharedPtr pBufLayout = VertexBufferLayout::create();
  pBufLayout->addElement("POSITION", 0, ResourceFormat::RGBA32Float, 1, 0);
  pBufLayout->addElement("TEXCOORD", 16, ResourceFormat::RG32Float, 1, 1);
  pLayout->addBufferLayout(0, pBufLayout);

  //create vao
  Vao::BufferVec buffers{ vertexBuffer };
  return Vao::create(Vao::Topology::Patch4, pLayout, buffers);
}