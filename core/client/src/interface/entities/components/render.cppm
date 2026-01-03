// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.render;

export import :entities.components.entity;
export import :entities.components.enums;
export import :model.render_mesh_group;
export import :rendering.enums;
import :rendering.entity_instance_data;
export import :rendering.model_render_buffer_data;

export namespace pragma {
	class CCameraComponent;
	class CSceneComponent;
	class CModelComponent;
	class CAttachmentComponent;
	class CAnimatedComponent;
	class CLightMapReceiverComponent;
	class CWorldComponent;
	namespace ecs {
		class CBaseEntity;
	}
};
export namespace pragma {
	namespace cRenderComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_UPDATE_RENDER_DATA_MT;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_RENDER_BUFFERS_INITIALIZED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_RENDER_BOUNDS_CHANGED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_RENDER_MODE_CHANGED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_SHOULD_DRAW;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_SHOULD_DRAW_SHADOW;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_UPDATE_RENDER_BUFFERS;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_UPDATE_RENDER_MATRICES;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_UPDATE_INSTANTIABILITY;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_CLIP_PLANE_CHANGED;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_DEPTH_BIAS_CHANGED;
	}
	class DLLCLIENT CRenderComponent final : public BaseRenderComponent, public CBaseNetComponent {
	  public:
		enum class StateFlags : uint16_t {
			None = 0u,
			RenderBufferDirty = 1u,
			ExemptFromOcclusionCulling = RenderBufferDirty << 1u,
			EnableDepthPass = ExemptFromOcclusionCulling << 1u,
			DisableShadows = EnableDepthPass << 1u,
			IsInstantiable = DisableShadows << 1u,
			InstantiationDisabled = IsInstantiable << 1u,
			RenderBoundsDirty = InstantiationDisabled << 1u,
			ShouldDraw = RenderBoundsDirty << 1u,
			ShouldDrawShadow = ShouldDraw << 1u,
			Hidden = ShouldDrawShadow << 1u,
			AncestorHidden = Hidden << 1u,
			IgnoreAncestorVisibility = AncestorHidden << 1u,
		};
		static constexpr auto USE_HOST_MEMORY_FOR_RENDER_DATA = true;

		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		CRenderComponent(ecs::BaseEntity &ent);
		const prosper::IBuffer *GetRenderBuffer() const;
		std::optional<RenderBufferIndex> GetRenderBufferIndex() const;
		bool IsRenderBufferValid() const { return m_renderBuffer != nullptr; }
		prosper::IDescriptorSet *GetRenderDescriptorSet() const;

		static const std::vector<CRenderComponent *> &GetEntitiesExemptFromOcclusionCulling();
		static const std::shared_ptr<prosper::IUniformResizableBuffer> &GetInstanceBuffer();
		static void InitializeBuffers();
		static void ClearBuffers();

		virtual void Initialize() override;
		virtual ~CRenderComponent() override;

		std::vector<std::shared_ptr<geometry::ModelSubMesh>> &GetRenderMeshes();
		const std::vector<std::shared_ptr<geometry::ModelSubMesh>> &GetRenderMeshes() const;
		std::vector<rendering::RenderBufferData> &GetRenderBufferData();
		const std::vector<rendering::RenderBufferData> &GetRenderBufferData() const { return const_cast<CRenderComponent *>(this)->GetRenderBufferData(); }
		std::vector<std::shared_ptr<geometry::ModelMesh>> &GetLODMeshes();
		const std::vector<std::shared_ptr<geometry::ModelMesh>> &GetLODMeshes() const;

		rendering::RenderMeshGroup &GetLodRenderMeshGroup(uint32_t lod);
		const rendering::RenderMeshGroup &GetLodRenderMeshGroup(uint32_t lod) const;
		rendering::RenderMeshGroup &GetLodMeshGroup(uint32_t lod);
		const rendering::RenderMeshGroup &GetLodMeshGroup(uint32_t lod) const;

		uint64_t GetLastRenderFrame() const;
		void SetLastRenderFrame(unsigned long long &t);

		void SetLocalRenderBounds(Vector3 min, Vector3 max);

		const bounding_volume::AABB &GetLocalRenderBounds() const;
		const math::Sphere &GetLocalRenderSphere() const;

		const bounding_volume::AABB &GetAbsoluteRenderBounds() const;
		const math::Sphere &GetAbsoluteRenderSphere() const;

		// Note: These mustn't be called during rendering!
		const bounding_volume::AABB &GetUpdatedAbsoluteRenderBounds() const;
		const math::Sphere &GetUpdatedAbsoluteRenderSphere() const;

		bounding_volume::AABB CalcAbsoluteRenderBounds() const;
		math::Sphere CalcAbsoluteRenderSphere() const;

		rendering::SceneRenderPass GetSceneRenderPass() const;
		void SetSceneRenderPass(rendering::SceneRenderPass pass);
		const util::PEnumProperty<rendering::SceneRenderPass> &GetSceneRenderPassProperty() const;

		bool IsInRenderGroup(rendering::RenderGroup group) const;
		bool AddToRenderGroup(const std::string &name);
		void AddToRenderGroup(rendering::RenderGroup group);
		bool RemoveFromRenderGroup(const std::string &name);
		void RemoveFromRenderGroup(rendering::RenderGroup group);
		void SetRenderGroups(rendering::RenderGroup group);
		rendering::RenderGroup GetRenderGroups() const;
		const util::PEnumProperty<rendering::RenderGroup> &GetRenderGroupsProperty() const;

		Mat4 &GetTransformationMatrix();
		const math::ScaledTransform &GetRenderPose() const;

		virtual void ReceiveData(NetPacket &packet) override;

		// Note: Called in render thread
		void UpdateRenderDataMT(const CSceneComponent &scene, const CCameraComponent &cam, const Mat4 &vp);

		void UpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd, bool bForceBufferUpdate = false);

		bool ShouldDraw() const;
		bool ShouldDrawShadow() const;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;

		CModelComponent *GetModelComponent() const;
		CAttachmentComponent *GetAttachmentComponent() const;
		CAnimatedComponent *GetAnimatedComponent() const;
		CLightMapReceiverComponent *GetLightMapReceiverComponent() const;

		void SetExemptFromOcclusionCulling(bool exempt);
		bool IsExemptFromOcclusionCulling() const;

		StateFlags GetStateFlags() const;

		void SetDepthPassEnabled(bool enabled);
		bool IsDepthPassEnabled() const;

		void SetRenderClipPlane(const Vector4 &plane);
		void ClearRenderClipPlane();
		const Vector4 *GetRenderClipPlane() const;

		void SetDepthBias(float d, float delta);
		void ClearDepthBias();
		const Vector2 *GetDepthBias() const;

		void SetReceiveShadows(bool enabled);
		bool IsReceivingShadows() const;

		void SetRenderBufferDirty();
		void SetRenderBoundsDirty();
		std::optional<math::intersection::LineMeshResult> CalcRayIntersection(const Vector3 &start, const Vector3 &dir, bool precise = false) const;

		bool IsInstantiable() const;
		void SetInstaniationEnabled(bool enabled);
		void UpdateInstantiability();

		void UpdateShouldDrawState();

		void SetRenderOffsetTransform(const math::ScaledTransform &t);
		void ClearRenderOffsetTransform();
		const math::ScaledTransform *GetRenderOffsetTransform() const;

		bool IsInPvs(const Vector3 &camPos, const CWorldComponent &world) const;
		bool IsInPvs(const Vector3 &camPos) const;

		const rendering::InstanceData &GetInstanceData() const;

		void SetTranslucencyPassDistanceOverride(double distance);
		void ClearTranslucencyPassDistanceOverride();
		const std::optional<double> &GetTranslucencyPassDistanceOverrideSqr() const;

		void SetHidden(bool hidden);
		bool IsHidden() const;
		bool IsVisible() const;
		void SetIgnoreAncestorVisibility(bool ignoreVisibility);
		bool ShouldIgnoreAncestorVisibility() const;

		GameShaderSpecialization GetShaderPipelineSpecialization() const;
		void ClearRenderBuffers();
	  protected:
		void UpdateAncestorHiddenState();
		void PropagateHiddenState();
		void UpdateVisibility();

		void UpdateShouldDrawShadowState();
		void UpdateRenderBuffer() const;
		void UpdateMatrices();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		void UpdateRenderMeshes();
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

		void InitializeRenderBuffers();
		void UpdateBoneBuffer();

		std::optional<math::ScaledTransform> m_renderOffset {};
		math::ScaledTransform m_renderPose {};
		Mat4 m_matTransformation = umat::identity();
		util::PEnumProperty<rendering::RenderGroup> m_renderGroups = nullptr;
		util::PEnumProperty<rendering::SceneRenderPass> m_renderPass = nullptr;

		// Used for quick access to avoid having to do a lookup on the entity's components
		mutable CAttachmentComponent *m_attachmentComponent = nullptr;
		mutable CAnimatedComponent *m_animComponent = nullptr;
		mutable CLightMapReceiverComponent *m_lightMapReceiverComponent = nullptr;

		bounding_volume::AABB m_localRenderBounds {};
		math::Sphere m_localRenderSphere {};

		bounding_volume::AABB m_absoluteRenderBounds {};
		math::Sphere m_absoluteRenderSphere {};

		std::optional<Vector4> m_renderClipPlane {};
		std::optional<Vector2> m_depthBias {};

		StateFlags m_stateFlags
		  = static_cast<StateFlags>(math::to_integral(StateFlags::RenderBufferDirty) | math::to_integral(StateFlags::EnableDepthPass) | math::to_integral(StateFlags::RenderBoundsDirty) | math::to_integral(StateFlags::ShouldDraw) | math::to_integral(StateFlags::ShouldDrawShadow));
		std::atomic<uint64_t> m_lastRender = 0ull;
		std::mutex m_renderDataMutex;
		static std::vector<CRenderComponent *> s_ocExemptEntities;
	  private:
		void UpdateAbsoluteRenderBounds();
		void UpdateAbsoluteSphereRenderBounds();
		void UpdateAbsoluteAABBRenderBounds();
		rendering::InstanceData m_instanceData {};
		std::shared_ptr<prosper::IBuffer> m_renderBuffer = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_renderDescSetGroup = nullptr;
		std::optional<double> m_translucencyPassDistanceOverrideSqr {};
	};

	// Events

	struct DLLCLIENT CEUpdateInstantiability : public ComponentEvent {
		CEUpdateInstantiability(bool &instantiable);
		virtual void PushArguments(lua::State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua::State *l) override;
		bool &instantiable;
	};

	struct DLLCLIENT CEShouldDraw : public ComponentEvent {
		CEShouldDraw(bool &shouldDraw);
		virtual void PushArguments(lua::State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua::State *l) override;
		bool &shouldDraw;
	};

	struct DLLCLIENT CEOnUpdateRenderMatrices : public ComponentEvent {
		CEOnUpdateRenderMatrices(math::ScaledTransform &pose, Mat4 &transformation);
		virtual void PushArguments(lua::State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua::State *l) override;
		math::ScaledTransform &pose;
		Mat4 &transformation;
	};

	struct DLLCLIENT CEOnUpdateRenderData : public ComponentEvent {
		CEOnUpdateRenderData();
		virtual void PushArguments(lua::State *l) override;
	};

	struct DLLCLIENT CEOnUpdateRenderBuffers : public ComponentEvent {
		CEOnUpdateRenderBuffers(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &commandBuffer);
		virtual void PushArguments(lua::State *l) override;
		std::shared_ptr<prosper::IPrimaryCommandBuffer> commandBuffer;
	};

	struct DLLCLIENT CEOnRenderBoundsChanged : public ComponentEvent {
		CEOnRenderBoundsChanged(const Vector3 &min, const Vector3 &max, const math::Sphere &sphere);
		virtual void PushArguments(lua::State *l) override;
		const Vector3 &min;
		const Vector3 &max;
		const math::Sphere &sphere;
	};
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::CRenderComponent::StateFlags)
}
