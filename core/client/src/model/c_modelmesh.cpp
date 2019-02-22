#include "stdafx_client.h"
#include "pragma/model/c_modelmesh.h"
#include <mathutil/umath.h>
#include "pragma/model/vk_mesh.h"
#include <buffers/prosper_dynamic_resizable_buffer.hpp>

// 256 MiB
#define MODEL_MESH_VERTEX_BUFFER_SIZE 268'435'456
// 32 MiB
#define MODEL_MESH_INDEX_BUFFER_SIZE 33'554'432

extern DLLCENGINE CEngine *c_engine;

CModelMesh::CModelMesh()
	: ModelMesh()
{}
std::shared_ptr<ModelMesh> CModelMesh::Copy() const {return std::make_shared<CModelMesh>(*this);}

void CModelMesh::AddSubMesh(const std::shared_ptr<ModelSubMesh> &subMesh)
{
	assert(typeid(*subMesh.get()) == typeid(CModelSubMesh));
	AddSubMesh(std::dynamic_pointer_cast<CModelSubMesh>(subMesh));
}
void CModelMesh::AddSubMesh(const std::shared_ptr<CModelSubMesh> &subMesh) {m_subMeshes.push_back(subMesh);}

///////////////////////////////////////////

static std::shared_ptr<prosper::DynamicResizableBuffer> s_vertexBuffer = nullptr;
static std::shared_ptr<prosper::DynamicResizableBuffer> s_indexBuffer = nullptr;
CModelSubMesh::CModelSubMesh()
	: ModelSubMesh(),NormalMesh(),m_vkMesh(std::make_shared<pragma::VkMesh>())
{}

CModelSubMesh::CModelSubMesh(const CModelSubMesh &other)
	: ModelSubMesh(other),m_vkMesh(std::make_shared<pragma::VkMesh>(*other.m_vkMesh))
{}
std::shared_ptr<ModelSubMesh> CModelSubMesh::Copy() const {return std::make_shared<CModelSubMesh>(*this);}

void CModelSubMesh::InitializeBuffers()
{
	if(s_vertexBuffer != nullptr)
		return;
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::DeviceLocal;
	createInfo.size = MODEL_MESH_VERTEX_BUFFER_SIZE;
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::VERTEX_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_SRC_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	s_vertexBuffer = prosper::util::create_dynamic_resizable_buffer(*c_engine,createInfo,createInfo.size *4u,0.05f);
	s_vertexBuffer->SetDebugName("mesh_vertex_data_buf");

	createInfo.size = MODEL_MESH_INDEX_BUFFER_SIZE;
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::INDEX_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_SRC_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	s_indexBuffer = prosper::util::create_dynamic_resizable_buffer(*c_engine,createInfo,createInfo.size *4u,0.025f);
	s_indexBuffer->SetDebugName("mesh_index_data_buf");
}
void CModelSubMesh::ClearBuffers()
{
	s_vertexBuffer = nullptr;
	s_indexBuffer = nullptr;
}

const std::shared_ptr<pragma::VkMesh> &CModelSubMesh::GetVKMesh() const {return m_vkMesh;}

void CModelSubMesh::UpdateVertexBuffer()
{
	m_vkMesh->SetVertexBuffer(nullptr); // Clear the old vertex buffer
	auto vertexBuffer = s_vertexBuffer->AllocateBuffer(m_vertices->size() *sizeof(Vertex),m_vertices->data());
	m_vkMesh->SetVertexBuffer(std::move(vertexBuffer));
}

void CModelSubMesh::Centralize(const Vector3 &origin)
{
	ModelSubMesh::Centralize(origin);
	UpdateVertexBuffer();
}

void CModelSubMesh::Update(ModelUpdateFlags flags)
{
	ModelSubMesh::Update(flags);
	auto bHasAlphas = (GetAlphaCount() > 0) ? true : false;
	auto bAnimated = !m_vertexWeights->empty() ? true : false;

	if((flags &ModelUpdateFlags::UpdateTangents) != ModelUpdateFlags::None)
		ComputeTangentBasis(*m_vertices,*m_triangles);

	//auto &renderState = c_engine->GetRenderContext();
	//auto &context = c_engine->GetRenderContext();
	
	if((flags &ModelUpdateFlags::UpdateIndexBuffer) != ModelUpdateFlags::None)
	{
		m_vkMesh->SetIndexBuffer(nullptr); // Clear the old index buffer
		auto indexBuffer = s_indexBuffer->AllocateBuffer(m_triangles->size() *sizeof(uint16_t),m_triangles->data());
		m_vkMesh->SetIndexBuffer(std::move(indexBuffer));
	}
	if((flags &ModelUpdateFlags::UpdateVertexBuffer) != ModelUpdateFlags::None)
		UpdateVertexBuffer();

	// Alpha buffer!
	//m_tangents.resize(m_vertices.size());
	//assert(m_tangents.size() == m_vertices.size());
	//m_biTangents.resize(m_vertices.size());
	//assert(m_biTangents.size() == m_vertices.size());

	if(bAnimated == true && (flags &ModelUpdateFlags::UpdateWeightBuffer) != ModelUpdateFlags::None)
	{
		auto weightBuffer = s_vertexBuffer->AllocateBuffer(m_vertexWeights->size() *sizeof(VertexWeight),m_vertexWeights->data());
		m_vkMesh->SetVertexWeightBuffer(std::move(weightBuffer));
	}
	
	if(bHasAlphas == true && (flags &ModelUpdateFlags::UpdateAlphaBuffer) != ModelUpdateFlags::None)
	{
		auto alphaBuffer = s_vertexBuffer->AllocateBuffer(m_alphas->size() *sizeof(Vector2),m_alphas->data());
		m_vkMesh->SetAlphaBuffer(std::move(alphaBuffer));
	}
}
