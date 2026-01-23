// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.animated;

export import :entities.components.entity;
export import pragma.prosper;

export namespace pragma {
	void initialize_articulated_buffers();
	void clear_articulated_buffers();
	const std::shared_ptr<prosper::IUniformResizableBuffer> &get_instance_bone_buffer();

	namespace cAnimatedComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_SKELETON_UPDATED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_BONE_MATRICES_UPDATED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_BONE_BUFFER_INITIALIZED;
	}
	namespace cAnimatedComponent {
		using namespace baseAnimatedComponent;
	}
	class DLLCLIENT CAnimatedComponent final : public BaseAnimatedComponent, public CBaseNetComponent {
	  public:
		enum class StateFlags : uint8_t { None = 0u, BoneBufferDirty = 1u, EnableSkeletonUpdateCallbacks = BoneBufferDirty << 1u };

		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		CAnimatedComponent(ecs::BaseEntity &ent) : BaseAnimatedComponent(ent) {}

		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void PlayAnimation(int animation, FPlayAnim flags = FPlayAnim::Default) override;
		bool HasBones() const;
		using BaseAnimatedComponent::PlayAnimation;

		virtual bool UpdateBonePoses() override;
		const prosper::IBuffer *GetBoneBuffer() const;
		const std::vector<Mat4> &GetBoneMatrices() const;
		std::vector<Mat4> &GetBoneMatrices();
		void UpdateBoneMatricesMT();
		void UpdateBoneBuffer(prosper::IPrimaryCommandBuffer &commandBuffer, bool flagAsDirty = false);
		void InitializeBoneBuffer();
		std::optional<Mat4> GetVertexTransformMatrix(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, Vector3 *optOutNormalOffset = nullptr, float *optOutDelta = nullptr) const;
		virtual std::optional<Mat4> GetVertexTransformMatrix(const geometry::ModelSubMesh &subMesh, uint32_t vertexId) const override;
		virtual bool GetVertexTransformMatrix(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, math::ScaledTransform &outPose) const override;

		uint32_t OnSkeletonUpdated();
		bool MaintainAnimations(double dt) override;

		void SetSkeletonUpdateCallbacksEnabled(bool enabled);
		bool AreSkeletonUpdateCallbacksEnabled() const;
		void SetBoneBufferDirty();
	  protected:
		virtual void OnModelChanged(const std::shared_ptr<asset::Model> &mdl) override;
		virtual void ResetAnimation(const std::shared_ptr<asset::Model> &mdl) override;
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	  private:
		std::shared_ptr<prosper::IBuffer> m_boneBuffer = nullptr;
		std::vector<Mat4> m_boneMatrices;
		StateFlags m_stateFlags = StateFlags::BoneBufferDirty;
	};

	// Events

	struct DLLCLIENT CEOnSkeletonUpdated : public ComponentEvent {
		CEOnSkeletonUpdated(uint32_t &physRootBoneId);
		virtual void PushArguments(lua::State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua::State *l) override;
		uint32_t &physRootBoneId;
	};

	struct DLLCLIENT CEOnBoneBufferInitialized : public ComponentEvent {
		CEOnBoneBufferInitialized(const std::shared_ptr<prosper::IBuffer> &buffer);
		virtual void PushArguments(lua::State *l) override;
		std::shared_ptr<prosper::IBuffer> buffer;
	};
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::CAnimatedComponent::StateFlags)
};
