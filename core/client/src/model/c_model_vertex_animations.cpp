#include "stdafx_client.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/model/animation/vertex_animation.hpp>
#include <prosper_util.hpp>

extern DLLCENGINE CEngine *c_engine;

void CModel::UpdateVertexAnimationBuffer()
{
	m_frameIndices.clear();
	auto &vertexAnimations = GetVertexAnimations();
	if(vertexAnimations.empty())
	{
		m_vertexAnimationBuffer = nullptr;
		return;
	}
	std::vector<std::array<float,4>> vertexAnimData {};
	auto numAllVerts = 0u;
	m_frameIndices.resize(vertexAnimations.size());
	auto vaIdx = 0u;
	for(auto &va : vertexAnimations)
	{
		auto &anims = va->GetMeshAnimations();
		auto &vaFrameOffsets = m_frameIndices.at(vaIdx++);
		vaFrameOffsets.resize(anims.size());
		auto animIdx = 0u;
		for(auto &anim : anims)
		{
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
	for(auto &va : vertexAnimations)
	{
		auto &vaFrameOffsets = m_frameIndices.at(vaIdx++);
		auto animIdx = 0u;
		for(auto &anim : va->GetMeshAnimations())
		{
			auto &meshFrameOffsets = vaFrameOffsets.at(animIdx++);
			auto frameIdx = 0u;
			for(auto &meshFrame : anim->GetFrames())
			{
				meshFrameOffsets.at(frameIdx++) = offset;
				auto &verts = meshFrame->GetVertices();
				//memcpy(vertexAnimData.data() +offset,verts.data(),verts.size() *sizeof(verts.front()));
				std::vector<std::array<float,4>> floatData {};
				floatData.reserve(verts.size());
				for(auto &v : verts)
				{
					floatData.push_back(std::array<float,4>{
						umath::float16_to_float32(v.at(0)),
						umath::float16_to_float32(v.at(1)),
						umath::float16_to_float32(v.at(2)),
						umath::float16_to_float32(v.at(3))
					});
				}
				memcpy(vertexAnimData.data() +offset,floatData.data(),floatData.size() *sizeof(floatData.front()));

				offset += verts.size();
			}
		}
	}

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT;
	createInfo.size = vertexAnimData.size() *sizeof(vertexAnimData.front());
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::DeviceLocal;
	m_vertexAnimationBuffer = prosper::util::create_buffer(c_engine->GetDevice(),createInfo,vertexAnimData.data());
}
const std::shared_ptr<prosper::Buffer> &CModel::GetVertexAnimationBuffer() const {return m_vertexAnimationBuffer;}
bool CModel::GetVertexAnimationBufferFrameOffset(uint32_t vaIdx,CModelSubMesh &subMesh,uint32_t frameId,uint64_t &offset) const
{
	if(vaIdx >= m_frameIndices.size())
		return false;
	auto &meshFrameOffsets = m_frameIndices.at(vaIdx);
	auto *va = GetVertexAnimation(vaIdx);
	if(va == nullptr)
		return false;
	auto meshAnimId = 0u;
	if((*va)->GetMeshAnimationId(subMesh,meshAnimId) == false || meshAnimId >= meshFrameOffsets.size())
		return false;
	auto &frameOffsets = meshFrameOffsets.at(meshAnimId);
	if(frameId >= frameOffsets.size())
		return false;
	offset = frameOffsets.at(frameId);
	return true;
}
