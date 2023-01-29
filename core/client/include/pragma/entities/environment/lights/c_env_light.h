/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_LIGHT_H__
#define __C_ENV_LIGHT_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/rendering/lighting/c_light_data.hpp"

class CBaseEntity;
class CModelMesh;
class Scene;
namespace pragma {
	class CLightSpotComponent;
	class CLightPointComponent;
	class CLightDirectionalComponent;
	class COcclusionCullerComponent;
	struct DLLCLIENT CEShouldPassEntity : public ComponentEvent {
		CEShouldPassEntity(const CBaseEntity &entity, uint32_t &renderFlags);
		virtual void PushArguments(lua_State *l) override;
		const CBaseEntity &entity;
		uint32_t &renderFlags;
		bool shouldPass = true;
	};
	struct DLLCLIENT CEShouldPassEntityMesh : public ComponentEvent {
		CEShouldPassEntityMesh(const CBaseEntity &ent, const CModelMesh &mesh, uint32_t &renderFlags);
		virtual void PushArguments(lua_State *l) override;
		const CBaseEntity &entity;
		const CModelMesh &mesh;
		uint32_t &renderFlags;
		bool shouldPass = true;
	};
	struct DLLCLIENT CEShouldPassMesh : public ComponentEvent {
		CEShouldPassMesh(const Model &model, const CModelSubMesh &mesh);
		virtual void PushArguments(lua_State *l) override;
		const Model &model;
		const CModelSubMesh &mesh;
		bool shouldPass = true;
	};
	struct DLLCLIENT CEShouldUpdateRenderPass : public ComponentEvent {
		CEShouldUpdateRenderPass();
		virtual void PushArguments(lua_State *l) override;
		bool shouldUpdate = false;
	};
	struct DLLCLIENT CEGetTransformationMatrix : public ComponentEvent {
		CEGetTransformationMatrix(uint32_t index);
		virtual void PushArguments(lua_State *l) override;
		uint32_t index;
		Mat4 *transformation = nullptr;
	};
	class CShadowComponent;
	struct DLLCLIENT CEHandleShadowMap : public ComponentEvent {
		CEHandleShadowMap();
		virtual void PushArguments(lua_State *l) override;
		CShadowComponent *resultShadow = nullptr;
	};
	struct DLLCLIENT CEOnShadowBufferInitialized : public ComponentEvent {
		CEOnShadowBufferInitialized(prosper::IBuffer &shadowBuffer);
		virtual void PushArguments(lua_State *l) override;
		prosper::IBuffer &shadowBuffer;
	};
	class DLLCLIENT CBaseLightComponent : public BaseEnvLightComponent, public CBaseNetComponent {
	  public:
		CBaseLightComponent(BaseEntity &ent);
		virtual ~CBaseLightComponent() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;

		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntitySpawn() override;

		void InitializeLightSource();
	};

	using LightBufferIndex = uint32_t;
	using ShadowBufferIndex = uint32_t;
	class DLLCLIENT CLightComponent final : public CBaseLightComponent {
	  public:
		static pragma::ComponentEventId EVENT_SHOULD_PASS_ENTITY;
		static pragma::ComponentEventId EVENT_SHOULD_PASS_ENTITY_MESH;
		static pragma::ComponentEventId EVENT_SHOULD_PASS_MESH;
		static pragma::ComponentEventId EVENT_SHOULD_UPDATE_RENDER_PASS;
		static pragma::ComponentEventId EVENT_GET_TRANSFORMATION_MATRIX;
		static pragma::ComponentEventId EVENT_HANDLE_SHADOW_MAP;
		static pragma::ComponentEventId EVENT_ON_SHADOW_BUFFER_INITIALIZED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

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
		enum class ShadowMapType : uint8_t { Static = 0, Dynamic };

		CLightComponent(BaseEntity &ent);
		virtual ~CLightComponent() override;
		CShadowComponent *GetShadowComponent();
		const CShadowComponent *GetShadowComponent() const;
		bool HasShadowsEnabled() const;
		Mat4 &GetTransformationMatrix(unsigned int j);
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		bool ShouldUpdateRenderPass(ShadowMapType smType) const;
		virtual bool ShouldPass(const CBaseEntity &ent, uint32_t &renderFlags);
		virtual bool ShouldPass(const CBaseEntity &ent, const CModelMesh &mesh, uint32_t &renderFlags);
		virtual bool ShouldPass(const Model &mdl, const CModelSubMesh &mesh);
		pragma::ComponentHandle<CShadowComponent> GetShadowMap(ShadowMapType type) const;
		bool ShouldRender();
		void UpdateTransformationMatrix(const Mat4 &biasMatrix, const Mat4 &viewMatrix, const Mat4 &projectionMatrix);
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		void SetStateFlag(StateFlags flag, bool enabled);

		void SetMorphTargetsInShadowsEnabled(bool enabled);
		bool AreMorphTargetsInShadowsEnabled() const;

		bool ShouldCastShadows() const;
		bool ShouldCastDynamicShadows() const;
		bool ShouldCastStaticShadows() const;
		bool IsInRange(const CBaseEntity &ent) const;
		bool IsInRange(const CBaseEntity &ent, const CModelMesh &mesh) const;
		bool IsInCone(const CBaseEntity &ent, const Vector3 &dir, float angle) const;

		// A shadowed light source may only be assigned to one scene / one scene slot
		CSceneComponent *FindShadowScene() const;
		COcclusionCullerComponent *FindShadowOcclusionCuller() const;

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

		void SetShadowMapIndex(uint32_t idx, ShadowMapType smType);
		uint32_t GetShadowMapIndex(ShadowMapType smType) const;
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
		ComponentHandle<CShadowComponent> m_shadowMapStatic = {};
		ComponentHandle<CShadowComponent> m_shadowMapDynamic = {};
		CShadowComponent *m_shadowComponent = nullptr;
		void InitializeShadowMap(CShadowComponent &sm);
		virtual void InitializeShadowMap();
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CLightComponent::StateFlags);

class DLLCLIENT CEnvLight : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
