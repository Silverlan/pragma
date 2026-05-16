// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :model.mesh;
import :engine;

pragma::geometry::CModelMesh::CModelMesh() : ModelMesh() {}
std::shared_ptr<pragma::geometry::ModelMesh> pragma::geometry::CModelMesh::Copy() const { return pragma::util::make_shared<CModelMesh>(*this); }

void pragma::geometry::CModelMesh::AddSubMesh(const std::shared_ptr<ModelSubMesh> &subMesh)
{
	assert(typeid(*subMesh.get()) == typeid(pragma::geometry::CModelSubMesh));
	AddSubMesh(std::dynamic_pointer_cast<CModelSubMesh>(subMesh));
}
void pragma::geometry::CModelMesh::AddSubMesh(const std::shared_ptr<CModelSubMesh> &subMesh) { m_subMeshes.push_back(subMesh); }

///////////////////////////////////////////

// These have to be separate buffers, because the base alignment of the sub-buffers has to match
// the underlying data structure (otherwise the buffers would not be usable as storage buffers).
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_vertexBuffer = nullptr;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_vertexWeightBuffer = nullptr;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_alphaBuffer = nullptr;
static std::shared_ptr<prosper::IDynamicResizableBuffer> s_indexBuffer = nullptr;
pragma::geometry::CModelSubMesh::CModelSubMesh() : ModelSubMesh(), m_sceneMesh(pragma::util::make_shared<rendering::SceneMesh>()) {}

pragma::geometry::CModelSubMesh::CModelSubMesh(const CModelSubMesh &other) : ModelSubMesh(other), m_sceneMesh(pragma::util::make_shared<rendering::SceneMesh>(*other.m_sceneMesh)) {}
const std::shared_ptr<prosper::IDynamicResizableBuffer> &pragma::geometry::CModelSubMesh::GetGlobalVertexBuffer() { return s_vertexBuffer; }
const std::shared_ptr<prosper::IDynamicResizableBuffer> &pragma::geometry::CModelSubMesh::GetGlobalVertexWeightBuffer() { return s_vertexWeightBuffer; }
const std::shared_ptr<prosper::IDynamicResizableBuffer> &pragma::geometry::CModelSubMesh::GetGlobalAlphaBuffer() { return s_alphaBuffer; }
const std::shared_ptr<prosper::IDynamicResizableBuffer> &pragma::geometry::CModelSubMesh::GetGlobalIndexBuffer() { return s_indexBuffer; }
std::shared_ptr<pragma::geometry::ModelSubMesh> pragma::geometry::CModelSubMesh::Copy(bool fullCopy) const
{
	auto cpy = pragma::util::make_shared<CModelSubMesh>(*this);
	ModelSubMesh::Copy(*cpy, fullCopy);
	return cpy;
}

void pragma::geometry::CModelSubMesh::InitializeBuffers()
{
	if(s_vertexBuffer != nullptr)
		return;
	// Initialize global vertex buffer
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	createInfo.size = console::get_con_var_value_bytes(*get_client_state(), "render_global_mesh_vertex_buffer_initial_capacity", GLOBAL_MESH_VERTEX_BUFFER_DEFAULT_INITIAL_SIZE);
	createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif
	createInfo.debugName = "mesh_vertex_data_buf";
	s_vertexBuffer = get_cengine()->GetRenderContext().CreateDynamicResizableBuffer(createInfo);
	s_vertexBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);
	s_vertexBuffer->SetReallocationBehavior(prosper::IBaseResizableBuffer::ReallocationBehavior::SafelyFreeOldBuffer);

	// Initialize global vertex weight buffer
	createInfo.size = console::get_con_var_value_bytes(*get_client_state(), "render_global_mesh_vertex_weight_buffer_initial_capacity", GLOBAL_MESH_VERTEX_WEIGHT_BUFFER_DEFAULT_INITIAL_SIZE);
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif
	createInfo.debugName = "mesh_vertex_weight_data_buf";
	s_vertexWeightBuffer = get_cengine()->GetRenderContext().CreateDynamicResizableBuffer(createInfo);
	s_vertexWeightBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);
	s_vertexWeightBuffer->SetReallocationBehavior(prosper::IBaseResizableBuffer::ReallocationBehavior::SafelyFreeOldBuffer);

	// Initialize global alpha buffer
	createInfo.size = console::get_con_var_value_bytes(*get_client_state(), "render_global_mesh_alpha_buffer_initial_capacity", GLOBAL_MESH_ALPHA_BUFFER_DEFAULT_INITIAL_SIZE);
	createInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif
	createInfo.debugName = "mesh_alpha_data_buf";
	s_alphaBuffer = get_cengine()->GetRenderContext().CreateDynamicResizableBuffer(createInfo);
	s_alphaBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);
	s_alphaBuffer->SetReallocationBehavior(prosper::IBaseResizableBuffer::ReallocationBehavior::SafelyFreeOldBuffer);

	// Initialize global index buffer
	createInfo.size = console::get_con_var_value_bytes(*get_client_state(), "render_global_mesh_index_buffer_initial_capacity", GLOBAL_MESH_INDEX_BUFFER_DEFAULT_INITIAL_SIZE);
	createInfo.usageFlags = prosper::BufferUsageFlags::IndexBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
#endif
	createInfo.debugName = "mesh_index_data_buf";
	s_indexBuffer = get_cengine()->GetRenderContext().CreateDynamicResizableBuffer(createInfo);
	s_indexBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);
	s_indexBuffer->SetReallocationBehavior(prosper::IBaseResizableBuffer::ReallocationBehavior::SafelyFreeOldBuffer);
}
void pragma::geometry::CModelSubMesh::ClearBuffers()
{
	s_vertexBuffer = nullptr;
	s_vertexWeightBuffer = nullptr;
	s_alphaBuffer = nullptr;
	s_indexBuffer = nullptr;
}

const std::shared_ptr<pragma::rendering::SceneMesh> &pragma::geometry::CModelSubMesh::GetSceneMesh() const { return m_sceneMesh; }

const std::shared_ptr<prosper::IRenderBuffer> &pragma::geometry::CModelSubMesh::GetRenderBuffer(ShaderEntity &shader, uint32_t pipelineIdx)
{
	if(m_sceneMesh == nullptr) {
		static std::shared_ptr<prosper::IRenderBuffer> nptr = nullptr;
		return nptr;
	}
	return m_sceneMesh->GetRenderBuffer(*this, shader, pipelineIdx);
}

void pragma::geometry::CModelSubMesh::UpdateVertexBuffer()
{
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	std::vector<VertexType> vertexBufferData {};
	vertexBufferData.reserve(m_vertices->size());
	for(auto &v : *m_vertices)
		vertexBufferData.push_back({v});
#else
	auto &vertexBufferData = *m_vertices;
#endif
	auto size = util::size_of_container(*m_vertices);
	auto vertexBuffer = m_sceneMesh->GetVertexBuffer();
	auto bufferSize = vertexBufferData.size() * sizeof(vertexBufferData.front());
	if(vertexBuffer == nullptr || bufferSize != vertexBuffer->GetSize()) {
		m_sceneMesh->SetVertexBuffer(nullptr); // Clear the old vertex buffer
		vertexBuffer = s_vertexBuffer->AllocateBuffer(bufferSize, vertexBufferData.data());
		if(!vertexBuffer) {
			spdlog::error("Failed to allocate mesh vertex buffer of size {}", util::get_pretty_bytes(bufferSize));
			return;
		}
	}
	else
		vertexBuffer->Write(0ull, bufferSize, vertexBufferData.data());
	m_sceneMesh->SetVertexBuffer(std::move(vertexBuffer));
}

void pragma::geometry::CModelSubMesh::Centralize(const Vector3 &origin)
{
	ModelSubMesh::Centralize(origin);
	UpdateVertexBuffer();
}

void pragma::geometry::CModelSubMesh::Update(asset::ModelUpdateFlags flags)
{
	ModelSubMesh::Update(flags);
	auto bHasAlphas = (GetAlphaCount() > 0) ? true : false;
	auto bAnimated = !m_vertexWeights->empty() ? true : false;

	if((flags & asset::ModelUpdateFlags::CalculateTangents) != asset::ModelUpdateFlags::None)
		ComputeTangentBasis();

	//auto &renderState = pragma::get_cengine()->GetRenderContext();
	//auto &context = pragma::get_cengine()->GetRenderContext();

	if((flags & asset::ModelUpdateFlags::UpdateIndexBuffer) != asset::ModelUpdateFlags::None) {
		auto &indexData = GetIndexData();
		auto &indexBuffer = m_sceneMesh->GetIndexBuffer();
		auto newBuffer = true;
		if(indexBuffer) {
			auto alignedSize = prosper::util::get_aligned_size(indexData.size(), size_of_index(GetIndexType()));
			if(alignedSize == indexBuffer->GetSize()) {
				newBuffer = false;
				indexBuffer->Write(0ull, util::size_of_container(indexData), indexData.data());
			}
		}
		if(newBuffer) {
			m_sceneMesh->SetIndexBuffer(nullptr, IndexType::UInt16); // Clear the old index buffer
			auto indexBuffer = s_indexBuffer->AllocateBuffer(indexData.size(), size_of_index(GetIndexType()), indexData.data());
			m_sceneMesh->SetIndexBuffer(std::move(indexBuffer), GetIndexType());
		}
	}
	if((flags & asset::ModelUpdateFlags::UpdateVertexBuffer) != asset::ModelUpdateFlags::None)
		UpdateVertexBuffer();

	// Alpha buffer!
	//m_tangents.resize(m_vertices.size());
	//assert(m_tangents.size() == m_vertices.size());
	//m_biTangents.resize(m_vertices.size());
	//assert(m_biTangents.size() == m_vertices.size());

	if(bAnimated == true && (flags & asset::ModelUpdateFlags::UpdateWeightBuffer) != asset::ModelUpdateFlags::None) {
		if(m_extendedVertexWeights->empty()) {
			auto weightBuffer = s_vertexWeightBuffer->AllocateBuffer(m_vertexWeights->size() * sizeof(VertexWeightType), m_vertexWeights->data());
			m_sceneMesh->SetVertexWeightBuffer(std::move(weightBuffer));
		}
		else {
			auto numVertWeights = m_vertexWeights->size() + m_extendedVertexWeights->size();
			std::vector<math::VertexWeight> vertWeights {};
			vertWeights.resize(numVertWeights);
			memcpy(vertWeights.data(), m_vertexWeights->data(), m_vertexWeights->size() * sizeof(m_vertexWeights->front()));
			memcpy(vertWeights.data() + m_vertexWeights->size(), m_extendedVertexWeights->data(), m_extendedVertexWeights->size() * sizeof(m_extendedVertexWeights->front()));

			auto weightBuffer = s_vertexWeightBuffer->AllocateBuffer(vertWeights.size() * sizeof(VertexWeightType), vertWeights.data());
			m_sceneMesh->SetVertexWeightBuffer(std::move(weightBuffer));
		}
	}

	if(bHasAlphas == true && (flags & asset::ModelUpdateFlags::UpdateAlphaBuffer) != asset::ModelUpdateFlags::None) {
		auto alphaBuffer = s_alphaBuffer->AllocateBuffer(m_alphas->size() * sizeof(AlphaType), m_alphas->data());
		m_sceneMesh->SetAlphaBuffer(std::move(alphaBuffer));
	}
}
