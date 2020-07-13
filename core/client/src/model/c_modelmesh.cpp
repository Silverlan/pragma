/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/model/c_modelmesh.h"
#include <mathutil/umath.h>
#include "pragma/model/vk_mesh.h"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <buffers/prosper_dynamic_resizable_buffer.hpp>

static constexpr uint64_t MEGABYTE = 1'024 *1'024;
static constexpr uint64_t GLOBAL_MESH_VERTEX_BUFFER_SIZE = MEGABYTE *256; // 13'107 instances per MiB
static constexpr uint64_t GLOBAL_MESH_VERTEX_WEIGHT_BUFFER_SIZE = MEGABYTE *32; // 32'768 instances per MiB
static constexpr uint64_t GLOBAL_MESH_ALPHA_BUFFER_SIZE = MEGABYTE *16; // 131'072 instances per MiB
static constexpr uint64_t GLOBAL_MESH_INDEX_BUFFER_SIZE = MEGABYTE *32; // 524'288 instances per MiB

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

// These have to be separate buffers, because the base alignment of the sub-buffers has to match
// the underlying data structure (otherwise the buffers would not be usable as storage buffers).
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_vertexBuffer = nullptr;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_vertexWeightBuffer = nullptr;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_alphaBuffer = nullptr;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_indexBuffer = nullptr;
CModelSubMesh::CModelSubMesh()
	: ModelSubMesh(),NormalMesh(),m_vkMesh(std::make_shared<pragma::VkMesh>())
{}

CModelSubMesh::CModelSubMesh(const CModelSubMesh &other)
	: ModelSubMesh(other),m_vkMesh(std::make_shared<pragma::VkMesh>(*other.m_vkMesh))
{}
const std::shared_ptr<prosper::IDynamicResizableBuffer> &CModelSubMesh::GetGlobalVertexBuffer() {return s_vertexBuffer;}
const std::shared_ptr<prosper::IDynamicResizableBuffer> &CModelSubMesh::GetGlobalVertexWeightBuffer() {return s_vertexWeightBuffer;}
const std::shared_ptr<prosper::IDynamicResizableBuffer> &CModelSubMesh::GetGlobalAlphaBuffer() {return s_alphaBuffer;}
const std::shared_ptr<prosper::IDynamicResizableBuffer> &CModelSubMesh::GetGlobalIndexBuffer() {return s_indexBuffer;}
std::shared_ptr<ModelSubMesh> CModelSubMesh::Copy() const {return std::make_shared<CModelSubMesh>(*this);}

void CModelSubMesh::InitializeBuffers()
{
	if(s_vertexBuffer != nullptr)
		return;
	// Initialize global vertex buffer
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	createInfo.size = GLOBAL_MESH_VERTEX_BUFFER_SIZE;
	createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif
	s_vertexBuffer = c_engine->GetRenderContext().CreateDynamicResizableBuffer(createInfo,createInfo.size *4u,0.05f);
	s_vertexBuffer->SetDebugName("mesh_vertex_data_buf");
	s_vertexBuffer->SetPermanentlyMapped(true);

	// Initialize global vertex weight buffer
	createInfo.size = GLOBAL_MESH_VERTEX_WEIGHT_BUFFER_SIZE;
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif
	s_vertexWeightBuffer = c_engine->GetRenderContext().CreateDynamicResizableBuffer(createInfo,createInfo.size *4u,0.025f);
	s_vertexWeightBuffer->SetDebugName("mesh_vertex_weight_data_buf");
	s_vertexWeightBuffer->SetPermanentlyMapped(true);

	// Initialize global alpha buffer
	createInfo.size = GLOBAL_MESH_ALPHA_BUFFER_SIZE;
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif
	s_alphaBuffer = c_engine->GetRenderContext().CreateDynamicResizableBuffer(createInfo,createInfo.size *4u,0.025f);
	s_alphaBuffer->SetDebugName("mesh_alpha_data_buf");
	s_alphaBuffer->SetPermanentlyMapped(true);

	// Initialize global index buffer
	createInfo.size = GLOBAL_MESH_INDEX_BUFFER_SIZE;
	createInfo.usageFlags = prosper::BufferUsageFlags::IndexBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif
	s_indexBuffer = c_engine->GetRenderContext().CreateDynamicResizableBuffer(createInfo,createInfo.size *4u,0.025f);
	s_indexBuffer->SetDebugName("mesh_index_data_buf");
	s_indexBuffer->SetPermanentlyMapped(true);
}
void CModelSubMesh::ClearBuffers()
{
	s_vertexBuffer = nullptr;
	s_vertexWeightBuffer = nullptr;
	s_alphaBuffer = nullptr;
	s_indexBuffer = nullptr;
}

const std::shared_ptr<pragma::VkMesh> &CModelSubMesh::GetVKMesh() const {return m_vkMesh;}

void CModelSubMesh::UpdateVertexBuffer()
{
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	std::vector<VertexType> vertexBufferData {};
	vertexBufferData.reserve(m_vertices->size());
	for(auto &v : *m_vertices)
		vertexBufferData.push_back({v});
#else
	auto &vertexBufferData = *m_vertices;
#endif
	auto vertexBuffer = m_vkMesh->GetVertexBuffer();
	auto bufferSize = vertexBufferData.size() *sizeof(vertexBufferData.front());
	if(vertexBuffer == nullptr || bufferSize != vertexBuffer->GetSize())
	{
		m_vkMesh->SetVertexBuffer(nullptr); // Clear the old vertex buffer
		vertexBuffer = s_vertexBuffer->AllocateBuffer(bufferSize,vertexBufferData.data());
	}
	else
		vertexBuffer->Write(0ull,bufferSize,vertexBufferData.data());
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
		auto indexBuffer = s_indexBuffer->AllocateBuffer(m_triangles->size() *sizeof(IndexType),m_triangles->data());
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
		if(m_extendedVertexWeights->empty())
		{
			auto weightBuffer = s_vertexWeightBuffer->AllocateBuffer(m_vertexWeights->size() *sizeof(VertexWeightType),m_vertexWeights->data());
			m_vkMesh->SetVertexWeightBuffer(std::move(weightBuffer));
		}
		else
		{
			auto numVertWeights = m_vertexWeights->size() +m_extendedVertexWeights->size();
			std::vector<VertexWeight> vertWeights {};
			vertWeights.resize(numVertWeights);
			memcpy(vertWeights.data(),m_vertexWeights->data(),m_vertexWeights->size() *sizeof(m_vertexWeights->front()));
			memcpy(vertWeights.data() +m_vertexWeights->size(),m_extendedVertexWeights->data(),m_extendedVertexWeights->size() *sizeof(m_extendedVertexWeights->front()));

			auto weightBuffer = s_vertexWeightBuffer->AllocateBuffer(vertWeights.size() *sizeof(VertexWeightType),vertWeights.data());
			m_vkMesh->SetVertexWeightBuffer(std::move(weightBuffer));
		}
	}
	
	if(bHasAlphas == true && (flags &ModelUpdateFlags::UpdateAlphaBuffer) != ModelUpdateFlags::None)
	{
		auto alphaBuffer = s_alphaBuffer->AllocateBuffer(m_alphas->size() *sizeof(AlphaType),m_alphas->data());
		m_vkMesh->SetAlphaBuffer(std::move(alphaBuffer));
	}
}

