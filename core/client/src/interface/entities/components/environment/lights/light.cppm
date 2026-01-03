// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.lights.light;

export import :entities.base_entity;
export import :entities.components.entity;
export import :model.mesh;
export import :rendering.enums;
export import :rendering.light_data;

export namespace pragma {
	struct DLLCLIENT CEShouldPassEntity : public ComponentEvent {
		CEShouldPassEntity(const ecs::CBaseEntity &entity, uint32_t &renderFlags);
		virtual void PushArguments(lua::State *l) override;
		const ecs::CBaseEntity &entity;
		uint32_t &renderFlags;
		bool shouldPass = true;
	};
	struct DLLCLIENT CEShouldPassEntityMesh : public ComponentEvent {
		CEShouldPassEntityMesh(const ecs::CBaseEntity &ent, const geometry::CModelMesh &mesh, uint32_t &renderFlags);
		virtual void PushArguments(lua::State *l) override;
		const ecs::CBaseEntity &entity;
		const geometry::CModelMesh &mesh;
		uint32_t &renderFlags;
		bool shouldPass = true;
	};
	struct DLLCLIENT CEShouldPassMesh : public ComponentEvent {
		CEShouldPassMesh(const asset::Model &model, const geometry::CModelSubMesh &mesh);
		virtual void PushArguments(lua::State *l) override;
		const asset::Model &model;
		const geometry::CModelSubMesh &mesh;
		bool shouldPass = true;
	};
	struct DLLCLIENT CEShouldUpdateRenderPass : public ComponentEvent {
		CEShouldUpdateRenderPass();
		virtual void PushArguments(lua::State *l) override;
		bool shouldUpdate = false;
	};
	struct DLLCLIENT CEGetTransformationMatrix : public ComponentEvent {
		CEGetTransformationMatrix(uint32_t index);
		virtual void PushArguments(lua::State *l) override;
		uint32_t index;
		Mat4 *transformation = nullptr;
	};
	struct DLLCLIENT CEHandleShadowMap : public ComponentEvent {
		CEHandleShadowMap();
		virtual void PushArguments(lua::State *l) override;
		BaseEntityComponent *resultShadow = nullptr;
	};
	struct DLLCLIENT CEOnShadowBufferInitialized : public ComponentEvent {
		CEOnShadowBufferInitialized(prosper::IBuffer &shadowBuffer);
		virtual void PushArguments(lua::State *l) override;
		prosper::IBuffer &shadowBuffer;
	};
	class DLLCLIENT CBaseLightComponent : public BaseEnvLightComponent, public CBaseNetComponent {
	  public:
		CBaseLightComponent(ecs::BaseEntity &ent);
		virtual ~CBaseLightComponent() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;

		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntitySpawn() override;

		void InitializeLightSource();
	};

	using LightBufferIndex = uint32_t;
	using ShadowBufferIndex = uint32_t;
	namespace cLightComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_SHOULD_PASS_ENTITY;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_SHOULD_PASS_ENTITY_MESH;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_SHOULD_PASS_MESH;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_SHOULD_UPDATE_RENDER_PASS;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_GET_TRANSFORMATION_MATRIX;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_HANDLE_SHADOW_MAP;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_SHADOW_BUFFER_INITIALIZED;
	}
	class DLLCLIENT CLightComponent final : public CBaseLightComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		static prosper::IUniformResizableBuffer &GetGlobalRenderBuffer();
		static prosper::IUniformResizableBuffer &GetGlobalShadowBuffer();
		static CLightComponent *GetLightByBufferIndex(LightBufferIndex idx);
		static CLightComponent *GetLightByShadowBufferIndex(ShadowBufferIndex idx);
		static uint32_t GetMaxLightCount();
		static uint32_t GetMaxShadowCount();
		static uint32_t GetLightCount();
		static void InitializeBuffers();
		static void ClearBuffers();
		const std::shared_ptr<prosper::IBuffer> &GetRenderBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetShadowBuffer() const;
		virtual void SetBaked(bool baked) override;

		enum class StateFlags : uint32_t { None = 0u, StaticUpdateRequired = 1u, DynamicUpdateRequired = StaticUpdateRequired << 1u, FullUpdateRequired = StaticUpdateRequired << 1u, AddToGameScene = FullUpdateRequired << 1u, EnableMorphTargetsInShadows = AddToGameScene << 1u };

		CLightComponent(ecs::BaseEntity &ent);
		virtual ~CLightComponent() override;
		template<typename TCPPM>
		TCPPM *GetShadowComponent();
		template<typename TCPPM>
		const TCPPM *GetShadowComponent() const;
		bool HasShadowsEnabled() const;
		Mat4 &GetTransformationMatrix(unsigned int j);
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		bool ShouldUpdateRenderPass(rendering::ShadowMapType smType) const;
		virtual bool ShouldPass(const ecs::CBaseEntity &ent, uint32_t &renderFlags);
		virtual bool ShouldPass(const ecs::CBaseEntity &ent, const geometry::CModelMesh &mesh, uint32_t &renderFlags);
		virtual bool ShouldPass(const asset::Model &mdl, const geometry::CModelSubMesh &mesh);
		template<typename TCPPM>
		ComponentHandle<TCPPM> GetShadowMap(rendering::ShadowMapType type) const;
		bool ShouldRender();
		void UpdateTransformationMatrix(const Mat4 &biasMatrix, const Mat4 &viewMatrix, const Mat4 &projectionMatrix);
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		void SetStateFlag(StateFlags flag, bool enabled);

		void SetMorphTargetsInShadowsEnabled(bool enabled);
		bool AreMorphTargetsInShadowsEnabled() const;

		bool ShouldCastShadows() const;
		bool ShouldCastDynamicShadows() const;
		bool ShouldCastStaticShadows() const;
		bool IsInRange(const ecs::CBaseEntity &ent) const;
		bool IsInRange(const ecs::CBaseEntity &ent, const geometry::CModelMesh &mesh) const;
		bool IsInCone(const ecs::CBaseEntity &ent, const Vector3 &dir, float angle) const;

		// A shadowed light source may only be assigned to one scene / one scene slot
		CSceneComponent *FindShadowScene() const;
		template<typename TCPPM>
		TCPPM *FindShadowOcclusionCuller() const;

		virtual void SetLightIntensityType(LightIntensityType type) override;
		virtual void SetLightIntensity(float intensity, LightIntensityType type) override;
		using CBaseLightComponent::SetLightIntensity;
		void UpdateLightIntensity();

		// Used for debug purposes only
		void UpdateBuffers();

		const LightBufferData &GetBufferData() const;
		LightBufferData &GetBufferData();
		const ShadowBufferData *GetShadowBufferData() const;
		ShadowBufferData *GetShadowBufferData();

		void SetShadowMapIndex(uint32_t idx, rendering::ShadowMapType smType);
		uint32_t GetShadowMapIndex(rendering::ShadowMapType smType) const;
		virtual void SetShadowType(ShadowType type) override;

		virtual void SetFalloffExponent(float falloffExponent) override;

		// For internal use only!
		void SetRenderBuffer(const std::shared_ptr<prosper::IBuffer> &renderBuffer, bool freeBuffer = true);
		void SetShadowBuffer(const std::shared_ptr<prosper::IBuffer> &renderBuffer, bool freeBuffer = true);
		void UpdateShadowTypes();
	  protected:
		static std::size_t s_lightCount;
		void InitializeRenderBuffer();
		void InitializeShadowBuffer();
		void DestroyRenderBuffer(bool freeBuffer = true);
		void DestroyShadowBuffer(bool freeBuffer = true);
		void UpdatePos();
		void UpdateDir();
		void UpdateColor();
		void UpdateRadius();
		void InitializeLight(BaseEntityComponent &component) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;

		LightBufferData m_bufferData {};
		std::unique_ptr<ShadowBufferData> m_shadowBufferData = nullptr;
		std::shared_ptr<prosper::IBuffer> m_renderBuffer = nullptr;
		std::shared_ptr<prosper::IBuffer> m_shadowBuffer = nullptr;

		enum class DataSlot : uint32_t {
			Flags = 0u,
			Position,
			Color,
			Distance,
			Direction,
			CutoffOuter,
			CutoffInner,
			ShadowIndex,
			ShadowMapIndex,
			// Light Buffer End

			ShadowStart,
			DepthMVP = ShadowStart,
			ViewMatrix,
			ProjectionMatrix
			// Shadow Buffer End
		};

		StateFlags m_stateFlags;
		double m_tTurnedOff = 0.0;
		uint64_t m_lastThink = std::numeric_limits<uint64_t>::max();
		ComponentHandle<BaseEntityComponent> m_shadowMapStatic = {};
		ComponentHandle<BaseEntityComponent> m_shadowMapDynamic = {};
		BaseEntityComponent *m_shadowComponent = nullptr;
		template<typename TCPPM>
		void InitializeShadowMap(TCPPM &sm);
		virtual void InitializeShadowMap();
	};
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::CLightComponent::StateFlags)
};

export class DLLCLIENT CEnvLight : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
