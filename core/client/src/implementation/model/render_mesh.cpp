// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :model.render_mesh;

import :engine;

using namespace pragma;

SceneMesh::SceneMesh() : m_vertexBuffer(nullptr), m_vertexWeightBuffer(nullptr), m_alphaBuffer(nullptr), m_indexBuffer(nullptr), m_indexType {pragma::geometry::IndexType::UInt16} {}
SceneMesh::SceneMesh(const SceneMesh &other) : m_renderBuffers {other.m_renderBuffers}, m_vertexBuffer {other.m_vertexBuffer}, m_vertexWeightBuffer {other.m_vertexWeightBuffer}, m_alphaBuffer {other.m_alphaBuffer}, m_indexBuffer {other.m_indexBuffer}, m_indexType {other.m_indexType} {}
SceneMesh::~SceneMesh() {}
SceneMesh &SceneMesh::operator=(const SceneMesh &other)
{
	m_renderBuffers = other.m_renderBuffers;
	m_vertexBuffer = other.m_vertexBuffer;
	m_vertexWeightBuffer = other.m_vertexWeightBuffer;
	m_alphaBuffer = other.m_alphaBuffer;
	m_indexBuffer = other.m_indexBuffer;
	return *this;
}
const std::shared_ptr<prosper::IBuffer> &SceneMesh::GetVertexBuffer() const { return m_vertexBuffer; }
const std::shared_ptr<prosper::IBuffer> &SceneMesh::GetVertexWeightBuffer() const { return m_vertexWeightBuffer; }
const std::shared_ptr<prosper::IBuffer> &SceneMesh::GetAlphaBuffer() const { return m_alphaBuffer; }
const std::shared_ptr<prosper::IBuffer> &SceneMesh::GetIndexBuffer() const { return m_indexBuffer; }

void SceneMesh::SetVertexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer)
{
	m_vertexBuffer = buffer;
	SetDirty();
}
void SceneMesh::SetVertexWeightBuffer(const std::shared_ptr<prosper::IBuffer> &buffer)
{
	m_vertexWeightBuffer = buffer;
	SetDirty();
}
void SceneMesh::SetAlphaBuffer(const std::shared_ptr<prosper::IBuffer> &buffer)
{
	m_alphaBuffer = buffer;
	SetDirty();
}
void SceneMesh::SetIndexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer, pragma::geometry::IndexType indexType)
{
	m_indexBuffer = buffer;
	SetDirty();
	m_indexType = indexType;
}
void SceneMesh::ClearBuffers()
{
	m_vertexBuffer = nullptr;
	m_vertexWeightBuffer = nullptr;
	m_alphaBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_renderBuffers.clear();
}

void SceneMesh::SetDirty() { m_renderBuffers.clear(); }
const std::shared_ptr<prosper::IRenderBuffer> &SceneMesh::GetRenderBuffer(CModelSubMesh &mesh, pragma::ShaderEntity &shader, uint32_t pipelineIdx)
{
	prosper::PipelineID pipelineId;
	if(shader.GetPipelineId(pipelineId, pipelineIdx) == false) {
		static std::shared_ptr<prosper::IRenderBuffer> nptr = nullptr;
		return nptr;
	}
	std::unique_lock lock {m_renderBufferMutex};
	auto it = std::find_if(m_renderBuffers.begin(), m_renderBuffers.end(), [pipelineId](const std::pair<prosper::PipelineID, std::shared_ptr<prosper::IRenderBuffer>> &pair) { return pair.first == pipelineId; });
	if(it != m_renderBuffers.end())
		return it->second;
	auto renderBuffer = shader.CreateRenderBuffer(mesh, pipelineIdx);
	if(renderBuffer == nullptr) {
		static std::shared_ptr<prosper::IRenderBuffer> nptr = nullptr;
		return nptr;
	}
	m_renderBuffers.push_back(std::make_pair(pipelineId, renderBuffer));
	return m_renderBuffers.back().second;
}
