#ifndef __C_ENV_LIGHT_H__
#define __C_ENV_LIGHT_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include "pragma/rendering/lighting/c_light_data.hpp"

enum class LightType : uint32_t
{
	Invalid = 0,
	Directional = 1,
	Point = 2,
	Spot = 3
};

namespace pragma
{
	class CLightSpotComponent;
	class CLightPointComponent;
	class CLightDirectionalComponent;
	struct DLLCLIENT CEShouldPassEntity
		: public ComponentEvent
	{
		CEShouldPassEntity(const CBaseEntity &entity,uint32_t &renderFlags);
		virtual void PushArguments(lua_State *l) override;
		const CBaseEntity &entity;
		uint32_t &renderFlags;
		bool shouldPass = true;
	};
	struct DLLCLIENT CEShouldPassEntityMesh
		: public ComponentEvent
	{
		CEShouldPassEntityMesh(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags);
		virtual void PushArguments(lua_State *l) override;
		const CBaseEntity &entity;
		const CModelMesh &mesh;
		uint32_t &renderFlags;
		bool shouldPass = true;
	};
	struct DLLCLIENT CEShouldPassMesh
		: public ComponentEvent
	{
		CEShouldPassMesh(const Model &model,const CModelSubMesh &mesh);
		virtual void PushArguments(lua_State *l) override;
		const Model &model;
		const CModelSubMesh &mesh;
		bool shouldPass = true;
	};
	struct DLLCLIENT CEShouldUpdateRenderPass
		: public ComponentEvent
	{
		CEShouldUpdateRenderPass();
		virtual void PushArguments(lua_State *l) override;
		bool shouldUpdate = false;
	};
	struct DLLCLIENT CEGetTransformationMatrix
		: public ComponentEvent
	{
		CEGetTransformationMatrix(uint32_t index);
		virtual void PushArguments(lua_State *l) override;
		uint32_t index;
		Mat4 *transformation = nullptr;
	};
	struct DLLCLIENT CEHandleShadowMap
		: public ComponentEvent
	{
		CEHandleShadowMap(std::unique_ptr<ShadowMap> &shadowMap);
		virtual void PushArguments(lua_State *l) override;
		std::unique_ptr<ShadowMap> &shadowMap;
	};
	struct DLLCLIENT CEOnShadowBufferInitialized
		: public ComponentEvent
	{
		CEOnShadowBufferInitialized(prosper::Buffer &shadowBuffer);
		virtual void PushArguments(lua_State *l) override;
		prosper::Buffer &shadowBuffer;
	};
	class DLLCLIENT CBaseLightComponent
		: public BaseEnvLightComponent,
		public CBaseNetComponent
	{
	public:
		CBaseLightComponent(BaseEntity &ent);
		virtual ~CBaseLightComponent() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;

		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;

		void SetLight(CLightSpotComponent &light);
		void SetLight(CLightPointComponent &light);
		void SetLight(CLightDirectionalComponent &light);

		BaseEntityComponent *GetLight(LightType &outType) const;
		BaseEntityComponent *GetLight() const;

		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		virtual void InitializeLight(BaseEntityComponent &component);
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntitySpawn() override;

		util::WeakHandle<BaseEntityComponent> m_hLight = {};
		void InitializeLightSource();
	};

	class DLLCLIENT CLightComponent final
		: public CBaseLightComponent
	{
	public:
		static pragma::ComponentEventId EVENT_SHOULD_PASS_ENTITY;
		static pragma::ComponentEventId EVENT_SHOULD_PASS_ENTITY_MESH;
		static pragma::ComponentEventId EVENT_SHOULD_PASS_MESH;
		static pragma::ComponentEventId EVENT_SHOULD_UPDATE_RENDER_PASS;
		static pragma::ComponentEventId EVENT_GET_TRANSFORMATION_MATRIX;
		static pragma::ComponentEventId EVENT_HANDLE_SHADOW_MAP;
		static pragma::ComponentEventId EVENT_ON_SHADOW_BUFFER_INITIALIZED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		static const prosper::UniformResizableBuffer &GetGlobalRenderBuffer();
		static const prosper::UniformResizableBuffer &GetGlobalShadowBuffer();
		static CLightComponent *GetLightByBufferIndex(uint32_t idx);
		static CLightComponent *GetLightByShadowBufferIndex(uint32_t idx);
		static uint32_t GetMaxLightCount();
		static uint32_t GetMaxShadowCount();
		static uint32_t GetLightCount();
		static void InitializeBuffers();
		static void ClearBuffers();
		const std::shared_ptr<prosper::Buffer> &GetRenderBuffer() const;
		const std::shared_ptr<prosper::Buffer> &GetShadowBuffer() const;

		enum class RenderPass : uint8_t
		{
			Static = 0,
			Dynamic = 1
		};
		enum class StateFlags : uint32_t
		{
			None = 0u,
			StaticUpdateRequired = 1u,
			FullUpdateRequired = StaticUpdateRequired<<1u,
			UseDualTextureSet = FullUpdateRequired<<1u,
			RenderScheduled = UseDualTextureSet<<1u, // TODO: Deprecated? Remove this flag!
			AddToGameScene = RenderScheduled<<1u
		};
		struct MeshInfo
		{
			MeshInfo(CModelSubMesh *mesh,uint32_t renderFlags=0);
			CModelSubMesh *mesh;
			uint32_t renderFlags;
		};
		struct EntityInfo
		{
			EntityInfo(CLightComponent *light,BaseEntity *ent,uint32_t renderFlags=0);
			~EntityInfo();
			EntityInfo(const EntityInfo&)=delete;
			EntityInfo &operator=(const EntityInfo&)=delete;
			EntityHandle hEntity;
			CallbackHandle hCbMaterialsLoaded;
			uint32_t renderFlags;
			double tLastMoved;
			std::vector<MeshInfo> meshes;
		};
		struct ParticleInfo
		{
			ParticleInfo(pragma::CParticleSystemComponent &pt);
			ParticleInfo(const ParticleInfo&)=delete;
			ParticleInfo &operator=(const ParticleInfo&)=delete;
			util::WeakHandle<pragma::CParticleSystemComponent> hParticle;
		};
		struct ShadowInfoSet
		{
			std::vector<std::shared_ptr<EntityInfo>>::iterator FindEntity(CBaseEntity *ent);
			std::vector<std::shared_ptr<EntityInfo>> meshInfo;
		};
		/*class BufferUpdateInfo
		{
		private:
			std::function<void(const Vulkan::Context &context,const Vulkan::Buffer&)> m_fUpdate;
			Vulkan::SwapBufferObject *m_swapBuffer;
			uint32_t m_swapchainUpdateFlags; // 1 Bit per swapchain image
		public:
			BufferUpdateInfo(const Vulkan::SwapBufferObject *swapBuffer,const std::function<void(const Vulkan::Context&,const Vulkan::Buffer&)> &f);
			bool ExecSwapchainUpdate();
			bool IsComplete() const;
		};*/ // prosper TODO

		CLightComponent(BaseEntity &ent);
		virtual ~CLightComponent() override;
		Mat4 &GetTransformationMatrix(unsigned int j);
		virtual void Initialize() override;
		bool ShouldUpdateRenderPass(RenderPass rp) const;
		void PostRenderShadow(); // Called once the light's shadows have been rendered
		void SetStaticResolved(bool b=true);
		virtual bool ShouldPass(const CBaseEntity &ent,uint32_t &renderFlags);
		virtual bool ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags);
		virtual bool ShouldPass(const Model &mdl,const CModelSubMesh &mesh);
		ShadowMap *GetShadowMap();
		void UpdateCulledMeshes();
		std::vector<std::shared_ptr<EntityInfo>> &GetCulledMeshes(RenderPass rp);
		std::vector<std::shared_ptr<ParticleInfo>> &GetCulledParticleSystems();
		bool ShouldRender();
		void UpdateTransformationMatrix(const Mat4 &biasMatrix,const Mat4 &viewMatrix,const Mat4 &projectionMatrix);
		virtual void OnEntitySpawn() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		void SetStateFlag(StateFlags flag,bool enabled);

		bool IsInRange(const CBaseEntity &ent) const;
		bool IsInRange(const CBaseEntity &ent,const CModelMesh &mesh) const;
		bool IsInCone(const CBaseEntity &ent,const Vector3 &dir,float angle) const;

		// Used for debug purposes only
		void UpdateBuffers();

		uint64_t GetLastTimeShadowRendered() const;
		void SetLastTimeShadowRendered(uint64_t t);

		const LightBufferData &GetBufferData() const;
		LightBufferData &GetBufferData();
		const ShadowBufferData *GetShadowBufferData() const;
		ShadowBufferData *GetShadowBufferData();

		void SetShadowMapIndex(uint32_t idx);
		uint32_t GetShadowMapIndex() const;
		virtual void SetShadowType(ShadowType type) override;

		virtual void SetFalloffExponent(float falloffExponent) override;

		// For internal use only!
		void SetRenderBuffer(const std::shared_ptr<prosper::Buffer> &renderBuffer);
		void SetShadowBuffer(const std::shared_ptr<prosper::Buffer> &renderBuffer);
	protected:
		static std::size_t s_lightCount;
		void InitializeRenderBuffer();
		void InitializeShadowBuffer();
		void DestroyRenderBuffer();
		void DestroyShadowBuffer();
		void InitializeLight(BaseEntityComponent &component) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;

		LightBufferData m_bufferData {};
		std::unique_ptr<ShadowBufferData> m_shadowBufferData = nullptr;
		std::shared_ptr<prosper::Buffer> m_renderBuffer = nullptr;
		std::shared_ptr<prosper::Buffer> m_shadowBuffer = nullptr;

		enum class DataSlot : uint32_t
		{
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

		//std::vector<std::unique_ptr<BufferUpdateInfo>> m_bufferUpdateInfo; // Index corresponds to LIGHT_BUFFER_OFFSET_* ID // prosper TODO
		//virtual void ScheduleBufferUpdate(DataSlot offsetId,const std::function<void(const Vulkan::Context&,const Vulkan::Buffer&)> &f); // prosper TODO
		void ExecSwapchainUpdate();
		uint64_t m_lastThink = std::numeric_limits<uint64_t>::max();
		uint64_t m_lastShadowRendered = std::numeric_limits<uint64_t>::max();
		std::array<ShadowInfoSet,2> m_shadowInfoSets;
		std::vector<std::shared_ptr<ParticleInfo>> m_particleInfo;
		uint64_t m_nextDynamicUpdate = 0ull;

		// Entity Info
		CallbackHandle m_onModelChanged;
		void ClearCache();
		virtual void UpdateEntity(CBaseEntity *ent);
		void UpdateAllEntities();
		virtual void UpdateParticleSystem(pragma::CParticleSystemComponent &pt);
		void UpdateAllParticleSystems();
		void UpdateMeshes(CBaseEntity *ent,std::vector<MeshInfo> &meshes);
		//
		StateFlags m_stateFlags;
		double m_tTurnedOff = 0.0;
		std::unique_ptr<ShadowMap> m_shadow;
		bool ShouldCastShadows() const;
		void UpdateShadowTypes();
		void InitializeShadowMap(ShadowMap &sm);
		virtual void InitializeShadowMap();
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CLightComponent::StateFlags);

class DLLCLIENT CEnvLight
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif