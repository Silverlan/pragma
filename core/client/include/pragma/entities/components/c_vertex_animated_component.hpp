/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_VERTEX_ANIMATED_COMPONENT_HPP__
#define __C_VERTEX_ANIMATED_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace prosper {
	class IDynamicResizableBuffer;
	class IBuffer;
};
namespace pragma {
	DLLCLIENT void initialize_vertex_animation_buffer();
	DLLCLIENT void clear_vertex_animation_buffer();
	DLLCLIENT const std::shared_ptr<prosper::IDynamicResizableBuffer> &get_vertex_animation_buffer();
	class DLLCLIENT CVertexAnimatedComponent final : public BaseEntityComponent {
	  public:
#pragma pack(push, 1)
		struct VertexAnimationData {
			uint32_t srcFrameOffset = 0u;
			uint32_t dstFrameOffset = 0u;
			float blend = 0.f;
			std::array<float, 5> padding; // Padding to 32 byte (highest common minStorageBufferOffsetAlignment -> https://vulkan.gpuinfo.org/displaydevicelimit.php?name=minStorageBufferOffsetAlignment )
		};
#pragma pack(pop)

		CVertexAnimatedComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual ~CVertexAnimatedComponent() override;
		virtual void Initialize() override;
		void UpdateVertexAnimationDataMT();
		void UpdateVertexAnimationBuffer(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd);
		const std::shared_ptr<prosper::IBuffer> &GetVertexAnimationBuffer() const;
		bool GetVertexAnimationBufferMeshOffset(CModelSubMesh &mesh, uint32_t &offset, uint32_t &animCount) const;
		bool GetLocalVertexPosition(const ModelSubMesh &subMesh, uint32_t vertexId, Vector3 &pos, Vector3 *optOutNormal = nullptr, float *optOutDelta = nullptr) const;
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		// Vertex animations
		struct VertexAnimationInfo {
			float playbackRate = 0.f;
			float cycle = 0.f;
		};
		std::unordered_map<CModelSubMesh *, std::vector<VertexAnimationData>> m_vertexAnimationData {};
		struct VertexAnimationSlot {
			uint32_t vertexAnimationId = std::numeric_limits<uint32_t>::max();
			uint32_t frameId = std::numeric_limits<uint32_t>::max();
			uint32_t nextFrameId = std::numeric_limits<uint32_t>::max();
			std::weak_ptr<ModelSubMesh> mesh = {};
			float blend = 0.f;
		};
		std::vector<VertexAnimationSlot> m_vertexAnimationSlots {};

		std::unordered_map<CModelSubMesh *, std::pair<uint32_t, uint32_t>> m_vertexAnimationMeshBufferOffsets {};
		std::vector<VertexAnimationData> m_vertexAnimationBufferData {};
		uint32_t m_maxVertexAnimations = 0u;
		uint32_t m_activeVertexAnimations = 0u;
		uint32_t m_vertexAnimationBufferDataCount = 0;
		std::shared_ptr<prosper::IBuffer> m_vertexAnimationBuffer = nullptr;
		bool m_bufferUpdateRequired = false;
		void InitializeVertexAnimationBuffer();
		void DestroyVertexAnimationBuffer();
	};
};

#endif
