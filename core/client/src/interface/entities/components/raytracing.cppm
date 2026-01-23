// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.raytracing;

export import :rendering.enums;
export import pragma.cmaterialsystem;
export import pragma.prosper;
export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CRaytracingComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static bool InitializeBuffers();
		static void ClearBuffers();
		static bool IsRaytracingEnabled();

		static const std::shared_ptr<prosper::IUniformResizableBuffer> &GetEntityMeshInfoBuffer();
		static const std::shared_ptr<material::MaterialDescriptorArrayManager> &GetMaterialDescriptorArrayManager();
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
			static_assert(math::to_integral(rendering::SceneRenderPass::Count) == 4);
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

		CRaytracingComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual ~CRaytracingComponent() override;

		virtual void InitializeLuaObject(lua::State *l) override;
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
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::CRaytracingComponent::StateFlags)
}
export {
	REGISTER_ENUM_FLAGS(pragma::CRaytracingComponent::SubMeshRenderInfoBufferData::Flags)
}
