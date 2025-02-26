/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_RAYTRACING_COMPONENT_HPP__
#define __C_RAYTRACING_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_rendermode.h"
#include <pragma/entities/components/base_entity_component.hpp>
#include <shader/prosper_descriptor_array_manager.hpp>
#include <buffers/prosper_buffer.hpp>

class MaterialDescriptorArrayManager;
namespace pragma {
	class DLLCLIENT CRaytracingComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static bool InitializeBuffers();
		static void ClearBuffers();
		static bool IsRaytracingEnabled();

		static const std::shared_ptr<prosper::IUniformResizableBuffer> &GetEntityMeshInfoBuffer();
		static const std::shared_ptr<MaterialDescriptorArrayManager> &GetMaterialDescriptorArrayManager();
		static const std::shared_ptr<prosper::IDescriptorSetGroup> &GetGameSceneDescriptorSetGroup();
		static uint32_t GetBufferMeshCount();

		enum class StateFlags : uint32_t { None = 0u, RenderBufferDirty = 1u, BoneBufferDirty = RenderBufferDirty << 1u };

#pragma pack(push, 1)
		struct SubMeshRenderInfoBufferData {
			enum class Flags : uint32_t {
				None = 0,
				Visible = 1,

				RenderModeWorld = Visible << 1u,
				RenderModeView = RenderModeWorld << 1u,
				RenderModeSkybox = RenderModeView << 1u,

				UseNormalMap = RenderModeSkybox << 1u
			};
			static_assert(umath::to_integral(pragma::rendering::SceneRenderPass::Count) == 4);
			// Bounds for the sub-mesh. w-component is unused.
			Vector4 aabbMin = {};
			Vector4 aabbMax = {};

			Flags flags = Flags::None;
			prosper::DescriptorArrayManager::ArrayIndex materialArrayIndex = prosper::DescriptorArrayManager::INVALID_ARRAY_INDEX; // Index into global material array

			prosper::IBuffer::SmallOffset vertexBufferStartIndex = prosper::IBuffer::INVALID_SMALL_OFFSET; // Index into global vertex buffer
			prosper::IBuffer::SmallOffset indexBufferStartIndex = prosper::IBuffer::INVALID_SMALL_OFFSET;  // Index into global index buffer

			prosper::IBuffer::SmallOffset vertexWeightBufferIndex = prosper::IBuffer::INVALID_SMALL_OFFSET; // Index into global vertex weight buffer
			prosper::IBuffer::SmallOffset entityBufferIndex = prosper::IBuffer::INVALID_SMALL_OFFSET;       // Index into global entity render buffer
			prosper::IBuffer::SmallOffset boneBufferStartIndex = prosper::IBuffer::INVALID_SMALL_OFFSET;    // Index into global entity animation/bone buffer
			uint32_t numTriangles = 0;
		};
#pragma pack(pop)

		CRaytracingComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual ~CRaytracingComponent() override;

		virtual void InitializeLuaObject(lua_State *l) override;
	  private:
		void InitializeModelRaytracingBuffers();
		void InitializeBufferUpdateCallback();
		void SetRenderBufferDirty();
		void SetBoneBufferDirty();
		void UpdateBuffers(prosper::IPrimaryCommandBuffer &cmd);

		std::vector<std::shared_ptr<prosper::IBuffer>> m_subMeshBuffers = {};
		CallbackHandle m_cbUpdateBuffers = {};
		StateFlags m_stateFlags = StateFlags::None;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CRaytracingComponent::StateFlags)
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags)

#endif
