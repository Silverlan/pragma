// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

#define ENTITY_RENDER_BUFFER_USE_STORAGE_BUFFER 1

export module pragma.client:entities.components.render;

export import :entities.components.entity;
export import :entities.components.enums;
export import :model.render_mesh_group;
export import :rendering.enums;
import :rendering.entity_instance_data;
export import :rendering.model_render_buffer_data;

export class CBaseEntity;
export namespace pragma {
	class CCameraComponent;
	class CSceneComponent;
	class CModelComponent;
	class CAttachmentComponent;
	class CAnimatedComponent;
	class CLightMapReceiverComponent;
	class CWorldComponent;
};
export namespace pragma {
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

		static ComponentEventId EVENT_ON_UPDATE_RENDER_DATA_MT;
		static ComponentEventId EVENT_ON_RENDER_BUFFERS_INITIALIZED;
		static ComponentEventId EVENT_ON_RENDER_BOUNDS_CHANGED;
		static ComponentEventId EVENT_ON_RENDER_MODE_CHANGED;
		static ComponentEventId EVENT_SHOULD_DRAW;
		static ComponentEventId EVENT_SHOULD_DRAW_SHADOW;
		static ComponentEventId EVENT_ON_UPDATE_RENDER_BUFFERS;
		static ComponentEventId EVENT_ON_UPDATE_RENDER_MATRICES;
		static ComponentEventId EVENT_UPDATE_INSTANTIABILITY;
		static ComponentEventId EVENT_ON_CLIP_PLANE_CHANGED;
		static ComponentEventId EVENT_ON_DEPTH_BIAS_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		CRenderComponent(pragma::ecs::BaseEntity &ent);
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

		std::vector<std::shared_ptr<pragma::ModelSubMesh>> &GetRenderMeshes();
		const std::vector<std::shared_ptr<pragma::ModelSubMesh>> &GetRenderMeshes() const;
		std::vector<rendering::RenderBufferData> &GetRenderBufferData();
		const std::vector<rendering::RenderBufferData> &GetRenderBufferData() const { return const_cast<CRenderComponent *>(this)->GetRenderBufferData(); }
		std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes();
		const std::vector<std::shared_ptr<ModelMesh>> &GetLODMeshes() const;

		RenderMeshGroup &GetLodRenderMeshGroup(uint32_t lod);
		const RenderMeshGroup &GetLodRenderMeshGroup(uint32_t lod) const;
		RenderMeshGroup &GetLodMeshGroup(uint32_t lod);
		const RenderMeshGroup &GetLodMeshGroup(uint32_t lod) const;

		uint64_t GetLastRenderFrame() const;
		void SetLastRenderFrame(unsigned long long &t);

		void SetLocalRenderBounds(Vector3 min, Vector3 max);

		const bounding_volume::AABB &GetLocalRenderBounds() const;
		const Sphere &GetLocalRenderSphere() const;

		const bounding_volume::AABB &GetAbsoluteRenderBounds() const;
		const Sphere &GetAbsoluteRenderSphere() const;

		// Note: These mustn't be called during rendering!
		const bounding_volume::AABB &GetUpdatedAbsoluteRenderBounds() const;
		const Sphere &GetUpdatedAbsoluteRenderSphere() const;

		bounding_volume::AABB CalcAbsoluteRenderBounds() const;
		Sphere CalcAbsoluteRenderSphere() const;

		pragma::rendering::SceneRenderPass GetSceneRenderPass() const;
		void SetSceneRenderPass(pragma::rendering::SceneRenderPass pass);
		const util::PEnumProperty<pragma::rendering::SceneRenderPass> &GetSceneRenderPassProperty() const;

		bool IsInRenderGroup(pragma::rendering::RenderGroup group) const;
		bool AddToRenderGroup(const std::string &name);
		void AddToRenderGroup(pragma::rendering::RenderGroup group);
		bool RemoveFromRenderGroup(const std::string &name);
		void RemoveFromRenderGroup(pragma::rendering::RenderGroup group);
		void SetRenderGroups(pragma::rendering::RenderGroup group);
		pragma::rendering::RenderGroup GetRenderGroups() const;
		const util::PEnumProperty<pragma::rendering::RenderGroup> &GetRenderGroupsProperty() const;

		Mat4 &GetTransformationMatrix();
		const umath::ScaledTransform &GetRenderPose() const;

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
		std::optional<Intersection::LineMeshResult> CalcRayIntersection(const Vector3 &start, const Vector3 &dir, bool precise = false) const;

		bool IsInstantiable() const;
		void SetInstaniationEnabled(bool enabled);
		void UpdateInstantiability();

		void UpdateShouldDrawState();

		void SetRenderOffsetTransform(const umath::ScaledTransform &t);
		void ClearRenderOffsetTransform();
		const umath::ScaledTransform *GetRenderOffsetTransform() const;

		bool IsInPvs(const Vector3 &camPos, const CWorldComponent &world) const;
		bool IsInPvs(const Vector3 &camPos) const;

		const pragma::rendering::InstanceData &GetInstanceData() const;

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

		std::optional<umath::ScaledTransform> m_renderOffset {};
		umath::ScaledTransform m_renderPose {};
		Mat4 m_matTransformation = umat::identity();
		util::PEnumProperty<pragma::rendering::RenderGroup> m_renderGroups = nullptr;
		util::PEnumProperty<rendering::SceneRenderPass> m_renderPass = nullptr;

		// Used for quick access to avoid having to do a lookup on the entity's components
		mutable CAttachmentComponent *m_attachmentComponent = nullptr;
		mutable CAnimatedComponent *m_animComponent = nullptr;
		mutable CLightMapReceiverComponent *m_lightMapReceiverComponent = nullptr;

		bounding_volume::AABB m_localRenderBounds {};
		Sphere m_localRenderSphere {};

		bounding_volume::AABB m_absoluteRenderBounds {};
		Sphere m_absoluteRenderSphere {};

		std::optional<Vector4> m_renderClipPlane {};
		std::optional<Vector2> m_depthBias {};

		StateFlags m_stateFlags
		  = static_cast<StateFlags>(umath::to_integral(StateFlags::RenderBufferDirty) | umath::to_integral(StateFlags::EnableDepthPass) | umath::to_integral(StateFlags::RenderBoundsDirty) | umath::to_integral(StateFlags::ShouldDraw) | umath::to_integral(StateFlags::ShouldDrawShadow));
		std::atomic<uint64_t> m_lastRender = 0ull;
		std::mutex m_renderDataMutex;
		static std::vector<CRenderComponent *> s_ocExemptEntities;
	  private:
		void UpdateAbsoluteRenderBounds();
		void UpdateAbsoluteSphereRenderBounds();
		void UpdateAbsoluteAABBRenderBounds();
		pragma::rendering::InstanceData m_instanceData {};
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
		CEOnUpdateRenderMatrices(umath::ScaledTransform &pose, Mat4 &transformation);
		virtual void PushArguments(lua::State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua::State *l) override;
		umath::ScaledTransform &pose;
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
		CEOnRenderBoundsChanged(const Vector3 &min, const Vector3 &max, const Sphere &sphere);
		virtual void PushArguments(lua::State *l) override;
		const Vector3 &min;
		const Vector3 &max;
		const Sphere &sphere;
	};
	using namespace umath::scoped_enum::bitwise;
};
export {
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<pragma::CRenderComponent::StateFlags> : std::true_type {};
	}
};
