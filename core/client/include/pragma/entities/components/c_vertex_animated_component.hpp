/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_VERTEX_ANIMATED_COMPONENT_HPP__
#define __C_VERTEX_ANIMATED_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma
{
	class DLLCLIENT CVertexAnimatedComponent final
		: public BaseEntityComponent
	{
	public:
		CVertexAnimatedComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		void UpdateVertexAnimationBuffer(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		const std::shared_ptr<prosper::IBuffer> &GetVertexAnimationBuffer() const;
		bool GetVertexAnimationBufferMeshOffset(CModelSubMesh &mesh,uint32_t &offset,uint32_t &animCount) const;
		bool GetLocalVertexPosition(const ModelSubMesh &subMesh,uint32_t vertexId,Vector3 &pos,Vector3 *optOutNormal=nullptr,float *optOutDelta=nullptr) const;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		// Vertex animations
		struct VertexAnimationInfo
		{
			float playbackRate = 0.f;
			float cycle = 0.f;
		};
#pragma pack(push,1)
		struct VertexAnimationData
		{
			uint32_t srcFrameOffset = 0u;
			uint32_t dstFrameOffset = 0u;
			float blend = 0.f;
			float padding = 0.f;
		};
#pragma pack(pop)
		std::unordered_map<CModelSubMesh*,std::vector<VertexAnimationData>> m_vertexAnimationData {};
		struct VertexAnimationSlot
		{
			uint32_t vertexAnimationId = std::numeric_limits<uint32_t>::max();
			uint32_t frameId = std::numeric_limits<uint32_t>::max();
			uint32_t nextFrameId = std::numeric_limits<uint32_t>::max();
			std::weak_ptr<ModelSubMesh> mesh = {};
			float blend = 0.f;
		};
		std::vector<VertexAnimationSlot> m_vertexAnimationSlots {};

		std::unordered_map<CModelSubMesh*,std::pair<uint32_t,uint32_t>> m_vertexAnimationMeshBufferOffsets {};
		uint32_t m_maxVertexAnimations = 0u;
		uint32_t m_activeVertexAnimations = 0u;
		std::shared_ptr<prosper::IBuffer> m_vertexAnimationBuffer = nullptr;
		void InitializeVertexAnimationBuffer();
		void DestroyVertexAnimationBuffer();
	};
};

#endif
