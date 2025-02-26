/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/model/animation/vertex_animation.hpp>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>

extern DLLCLIENT CEngine *c_engine;

void CModel::UpdateVertexAnimationBuffer()
{
	m_frameIndices.clear();
	auto &vertexAnimations = GetVertexAnimations();
	if(vertexAnimations.empty()) {
		if(m_vertexAnimationBuffer)
			c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_vertexAnimationBuffer);
		m_vertexAnimationBuffer = nullptr;
		return;
	}
	std::vector<std::array<float, 4>> vertexAnimData {};
	auto numAllVerts = 0u;
	m_frameIndices.resize(vertexAnimations.size());
	auto vaIdx = 0u;
	for(auto &va : vertexAnimations) {
		auto &anims = va->GetMeshAnimations();
		auto &vaFrameOffsets = m_frameIndices.at(vaIdx++);
		vaFrameOffsets.resize(anims.size());
		auto animIdx = 0u;
		for(auto &anim : anims) {
			auto &frames = anim->GetFrames();
			auto &meshFrameOffsets = vaFrameOffsets.at(animIdx++);
			meshFrameOffsets.resize(frames.size());
			for(auto &meshFrame : frames)
				numAllVerts += meshFrame->GetVertices().size();
		}
	}

	vertexAnimData.resize(numAllVerts);
	auto offset = 0ull;
	vaIdx = 0u;
	for(auto &va : vertexAnimations) {
		auto &vaFrameOffsets = m_frameIndices.at(vaIdx++);
		auto animIdx = 0u;
		for(auto &anim : va->GetMeshAnimations()) {
			auto &meshFrameOffsets = vaFrameOffsets.at(animIdx++);
			auto frameIdx = 0u;
			for(auto &meshFrame : anim->GetFrames()) {
				meshFrameOffsets.at(frameIdx++) = offset;
				auto &verts = meshFrame->GetVertices();
				auto &normals = meshFrame->GetNormals();
				auto hasNormals = meshFrame->IsFlagEnabled(MeshVertexFrame::Flags::HasNormals);
				std::vector<std::array<int32_t, 4>> vertexData {};
				vertexData.reserve(verts.size());
				uint32_t vertIdx = 0;
				for(auto &v : verts) {
					vertexData.push_back(std::array<int32_t, 4> {});
					auto &vdata = vertexData.back();
					vdata.at(0) = (v.at(0) << 16) | v.at(1);
					vdata.at(1) = (v.at(2) << 16) | v.at(3);
					if(hasNormals) {
						std::array<uint16_t, 4> n {};
						if(vertIdx < normals.size())
							n = normals.at(vertIdx);
						else {
							auto &dir = uvec::FORWARD;
							n = {static_cast<uint16_t>(umath::float32_to_float16_glm(dir.x)), static_cast<uint16_t>(umath::float32_to_float16_glm(dir.y)), static_cast<uint16_t>(umath::float32_to_float16_glm(dir.z)), 0};
						}
						vdata.at(2) = (n.at(0) << 16) | n.at(1);
						vdata.at(3) = (n.at(2) << 16) | n.at(3);
					}
					++vertIdx;
				}
				memcpy(vertexAnimData.data() + offset, vertexData.data(), vertexData.size() * sizeof(vertexData.front()));

				offset += verts.size();
			}
		}
	}

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit;
	createInfo.size = vertexAnimData.size() * sizeof(vertexAnimData.front());
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	if(m_vertexAnimationBuffer)
		c_engine->GetRenderContext().KeepResourceAliveUntilPresentationComplete(m_vertexAnimationBuffer);
	m_vertexAnimationBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo, vertexAnimData.data());
}
const std::shared_ptr<prosper::IBuffer> &CModel::GetVertexAnimationBuffer() const { return m_vertexAnimationBuffer; }
bool CModel::GetVertexAnimationBufferFrameOffset(uint32_t vaIdx, CModelSubMesh &subMesh, uint32_t frameId, uint64_t &offset) const
{
	if(vaIdx >= m_frameIndices.size())
		return false;
	auto &meshFrameOffsets = m_frameIndices.at(vaIdx);
	auto *va = GetVertexAnimation(vaIdx);
	if(va == nullptr)
		return false;
	auto meshAnimId = 0u;
	if((*va)->GetMeshAnimationId(subMesh, meshAnimId) == false || meshAnimId >= meshFrameOffsets.size())
		return false;
	auto &frameOffsets = meshFrameOffsets.at(meshAnimId);
	if(frameId >= frameOffsets.size())
		return false;
	offset = frameOffsets.at(frameId);
	return true;
}
