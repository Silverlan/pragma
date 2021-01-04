/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <buffers/prosper_render_buffer.hpp>
#include <shader/prosper_pipeline_create_info.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;

SceneMesh::SceneMesh()
	: m_vertexBuffer(nullptr),m_vertexWeightBuffer(nullptr),
	m_alphaBuffer(nullptr),m_indexBuffer(nullptr)
{}
SceneMesh::SceneMesh(const SceneMesh &other)
	: m_renderBuffers{other.m_renderBuffers},m_vertexBuffer{other.m_vertexBuffer},
	m_vertexWeightBuffer{other.m_vertexWeightBuffer},m_alphaBuffer{other.m_alphaBuffer},
	m_indexBuffer{other.m_indexBuffer},m_lightmapUvBuffer{other.m_lightmapUvBuffer}
{}
SceneMesh &SceneMesh::operator=(const SceneMesh &other)
{
	m_renderBuffers = other.m_renderBuffers;
	m_vertexBuffer = other.m_vertexBuffer;
	m_vertexWeightBuffer = other.m_vertexWeightBuffer;
	m_alphaBuffer = other.m_alphaBuffer;
	m_indexBuffer = other.m_indexBuffer;
	m_lightmapUvBuffer = other.m_lightmapUvBuffer;
	return *this;
}
const std::shared_ptr<prosper::IBuffer> &SceneMesh::GetVertexBuffer() const {return m_vertexBuffer;}
const std::shared_ptr<prosper::IBuffer> &SceneMesh::GetVertexWeightBuffer() const {return m_vertexWeightBuffer;}
const std::shared_ptr<prosper::IBuffer> &SceneMesh::GetAlphaBuffer() const {return m_alphaBuffer;}
const std::shared_ptr<prosper::IBuffer> &SceneMesh::GetIndexBuffer() const {return m_indexBuffer;}
const std::shared_ptr<prosper::IBuffer> &SceneMesh::GetLightmapUvBuffer() const {return m_lightmapUvBuffer;}

void SceneMesh::SetVertexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer) {m_vertexBuffer = buffer; SetDirty();}
void SceneMesh::SetVertexWeightBuffer(const std::shared_ptr<prosper::IBuffer> &buffer) {m_vertexWeightBuffer = buffer; SetDirty();}
void SceneMesh::SetAlphaBuffer(const std::shared_ptr<prosper::IBuffer> &buffer) {m_alphaBuffer = buffer; SetDirty();}
void SceneMesh::SetIndexBuffer(const std::shared_ptr<prosper::IBuffer> &buffer) {m_indexBuffer = buffer; SetDirty();}
void SceneMesh::SetLightmapUvBuffer(const std::shared_ptr<prosper::IBuffer> &lightmapUvBuffer) {m_lightmapUvBuffer = lightmapUvBuffer; SetDirty();}

void SceneMesh::SetDirty() {m_renderBuffers.clear();}
const std::shared_ptr<prosper::IRenderBuffer> &SceneMesh::GetRenderBuffer(CModelSubMesh &mesh,pragma::ShaderEntity &shader,uint32_t pipelineIdx)
{
	prosper::PipelineID pipelineId;
	if(shader.GetPipelineId(pipelineId,pipelineIdx) == false)
	{
		static std::shared_ptr<prosper::IRenderBuffer> nptr = nullptr;
		return nptr;
	}
	std::unique_lock lock {m_renderBufferMutex};
	auto it = std::find_if(m_renderBuffers.begin(),m_renderBuffers.end(),[pipelineId](const std::pair<prosper::PipelineID,std::shared_ptr<prosper::IRenderBuffer>> &pair) {return pair.first == pipelineId;});
	if(it != m_renderBuffers.end())
		return it->second;
	auto renderBuffer = shader.CreateRenderBuffer(mesh,pipelineIdx);
	if(renderBuffer == nullptr)
	{
		static std::shared_ptr<prosper::IRenderBuffer> nptr = nullptr;
		return nptr;
	}
	m_renderBuffers.push_back(std::make_pair(pipelineId,renderBuffer));
	return m_renderBuffers.back().second;
}
