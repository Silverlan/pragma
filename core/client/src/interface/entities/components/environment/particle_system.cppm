// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:entities.components.particle_system;

export import :entities.base_entity;
export import :entities.components.entity;
export import :particle_system.enums;
export import :particle_system.modifier;
export import :particle_system.particle;
export import :rendering.enums;
export import pragma.cmaterialsystem;

export namespace pragma::ecs {
	struct DLLCLIENT ParticleSystemFileHeader {
		uint32_t version = 0;
		uint32_t numParticles = 0;
		std::vector<std::string> particleSystemNames {};
		std::vector<uint64_t> particleSystemOffsets {};
	};
	DLLCLIENT std::unordered_map<std::string, std::string> get_particle_key_values(lua::State *l, const luabind::map<std::string, void> &keyValues);
	class DLLCLIENT CParticleSystemComponent final : public BaseEnvParticleSystemComponent, public CBaseNetComponent, public pts::CParticleSystemBaseKeyValues {
	  public:
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		static const uint32_t PARTICLE_DATA_SIZE;
		static const uint32_t VERTEX_COUNT;
		static bool Save(const std::string &fileName, const std::vector<CParticleSystemComponent *> &particleSystems);
		static bool Save(fs::VFilePtrReal &f, const std::vector<CParticleSystemComponent *> &particleSystems);
		static bool Save(const std::vector<CParticleSystemComponent *> &particleSystems, udm::AssetData &outData, std::string &outErr);
		static bool IsParticleFilePrecached(const std::string &fname);
		static void InitializeBuffers();
		static void ClearBuffers();
		static std::optional<ParticleSystemFileHeader> ReadHeader(NetworkState &nw, const std::string &fileName);
		static std::optional<ParticleSystemFileHeader> ReadHeader(fs::VFilePtr &f);
		static bool Precache(std::string fname, bool bReload = false);
		static bool PrecacheLegacy(std::string fname, bool bReload = false);
		static bool InitializeFromAssetData(const std::string &ptName, const udm::LinkedPropertyWrapper &udm, std::string &outErr);
		static const std::vector<std::string> &GetPrecachedParticleSystemFiles();
		static std::optional<std::string> FindParticleSystemFile(const std::string ptName);
		static const std::unordered_map<std::string, std::unique_ptr<asset::ParticleSystemData>> &GetCachedParticleSystemData();
		static void ClearCache();
		static const std::shared_ptr<prosper::IDynamicResizableBuffer> &GetGlobalParticleBuffer();
		static const std::shared_ptr<prosper::IDynamicResizableBuffer> &GetGlobalAnimationStartBuffer();
		static const std::shared_ptr<prosper::IDynamicResizableBuffer> &GetGlobalAnimationBuffer();

		static CParticleSystemComponent *Create(const std::string &fname, CParticleSystemComponent *parent = nullptr, bool bRecordKeyValues = false, bool bAutoSpawn = true);
		static CParticleSystemComponent *Create(const std::unordered_map<std::string, std::string> &values, CParticleSystemComponent *parent = nullptr, bool bRecordKeyValues = false, bool bAutoSpawn = true);
		static CParticleSystemComponent *Create(CParticleSystemComponent *parent = nullptr, bool bAutoSpawn = true);
		static std::shared_ptr<asset::Model> GenerateModel(Game &game, const std::vector<const CParticleSystemComponent *> &particleSystems);

		enum class Flags : uint32_t {
			None = 0u,
			SoftParticles = 1u,
			TextureScrollingEnabled = SoftParticles << 1u,
			RendererBufferUpdateRequired = TextureScrollingEnabled << 1u,
			HasMovingParticles = RendererBufferUpdateRequired << 1u,
			MoveWithEmitter = HasMovingParticles << 1u,
			RotateWithEmitter = MoveWithEmitter << 1u,
			SortParticles = RotateWithEmitter << 1u,
			Dying = SortParticles << 1u,
			RandomStartFrame = Dying << 1u,
			PremultiplyAlpha = RandomStartFrame << 1u,
			AlwaysSimulate = PremultiplyAlpha << 1u,
			CastShadows = AlwaysSimulate << 1u,
			Setup = CastShadows << 1u, /* Has this system been set up already? */
			AutoSimulate = Setup << 1u,
			MaterialDescriptorSetInitialized = AutoSimulate << 1u
		};

		using ControlPointIndex = uint32_t;
		struct DLLCLIENT ControlPoint {
			float simTimestamp = 0.f;
			math::Transform pose = {};
			EntityHandle hEntity = {};
		};

		struct DLLCLIENT ChildData {
			util::WeakHandle<CParticleSystemComponent> child {};
			float delay = 0.f;
		};

#pragma pack(push, 1)
		struct DLLCLIENT ParticleData {
			// Padding for std140 alignment rules (16-byte alignment), required for use in storage buffer
			Vector3 position = {};
			float radius = 0.f;
			Vector3 prevPos = {};
			float age = 0.f;
			std::array<uint16_t, 4> color;
			float rotation = 0.f;
			uint16_t length = 0;        // Encoded float
			uint16_t rotationYaw = 0.f; // Encoded float

			Color GetColor() const;
		};
		struct DLLCLIENT ParticleAnimationData {
			uint16_t frameIndex0 = 0;
			uint16_t frameIndex1 = 0;
			float interpFactor = 0.f;
		};
#pragma pack(pop)

		// Component methods
		CParticleSystemComponent(BaseEntity &ent) : BaseEnvParticleSystemComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual ~CParticleSystemComponent() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void SetRemoveOnComplete(bool b) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void OnEntitySpawn() override;
		virtual void SetParticleFile(const std::string &fileName) override;
		void ToParticleSystemData(asset::ParticleSystemData &outData);
		static bool LoadFromAssetData(asset::ParticleSystemData &ptData, const udm::AssetData &data, std::string &outErr);
		static bool LoadFromAssetData(asset::ParticleSystemData &ptData, const udm::LinkedPropertyWrapper &data, std::string &outErr);

		// Particle
		// Returns the buffer index for the specified particle. Only particles which are alive have a valid buffer index!
		std::size_t TranslateParticleIndex(std::size_t particleIdx) const;
		// Translates a buffer index to a particle index
		std::size_t TranslateBufferIndex(std::size_t bufferIdx) const;
		pts::CParticleInitializer *AddInitializer(std::string identifier, const std::unordered_map<std::string, std::string> &values);
		pts::CParticleOperator *AddOperator(std::string identifier, const std::unordered_map<std::string, std::string> &values);
		pts::CParticleRenderer *AddRenderer(std::string identifier, const std::unordered_map<std::string, std::string> &values);
		void RemoveInitializer(const std::string &name);
		void RemoveOperator(const std::string &name);
		void RemoveRenderer(const std::string &name);
		void RemoveInitializersByType(const std::string &type);
		void RemoveOperatorsByType(const std::string &type);
		void RemoveRenderersByType(const std::string &type);
		void SetRadius(float r);
		void SetExtent(float ext);
		float GetRadius() const;
		float GetExtent() const;
		void Clear();
		const std::vector<ParticleData> &GetRenderParticleData() const;
		const std::vector<ParticleAnimationData> &GetParticleAnimationData() const;
		void SetMaterial(material::Material *mat);
		void SetMaterial(const char *mat);
		material::Material *GetMaterial() const;
		pts::ParticleOrientationType GetOrientationType() const;
		void SetOrientationType(pts::ParticleOrientationType type);
		void SetNodeTarget(uint32_t node, CBaseEntity *ent);
		void SetNodeTarget(uint32_t node, const Vector3 &pos);
		uint32_t GetNodeCount() const;
		Vector3 GetNodePosition(uint32_t node) const;
		CBaseEntity *GetNodeTarget(uint32_t node) const;
		Vector3 GetParticlePosition(uint32_t ptIdx) const;
		void SetCastShadows(bool b);
		bool GetCastShadows() const;
		float GetStaticWorldScale() const;
		void SetStaticWorldScale(float scale);
		bool IsStatic() const;
		void SetSceneRenderPass(rendering::SceneRenderPass pass);
		rendering::SceneRenderPass GetSceneRenderPass() const;
		bool IsRendererBufferUpdateRequired() const;
		CallbackHandle AddRenderCallback(const std::function<void(void)> &cb);
		void AddRenderCallback(const CallbackHandle &hCb);
		rendering::ParticleAlphaMode GetAlphaMode() const;
		rendering::ParticleAlphaMode GetEffectiveAlphaMode() const;
		void SetAlphaMode(rendering::ParticleAlphaMode alphaMode);
		void SetTextureScrollingEnabled(bool b);
		bool IsTextureScrollingEnabled() const;

		bool ShouldParticlesRotateWithEmitter() const;
		bool ShouldParticlesMoveWithEmitter() const;

		void SetAutoSimulate(bool b);
		bool ShouldAutoSimulate() const;

		bool IsAlphaPremultiplied() const;
		void SetAlphaPremultiplied(bool b);

		uint32_t GetEmissionRate() const;
		void SetEmissionRate(uint32_t emissionRate);
		// Can be used to set the exact amount of particles that should be spawned
		// in the next frame. This value will be reset right after it has been applied!
		void SetNextParticleEmissionCount(uint32_t count);

		void PauseEmission();
		void ResumeEmission();

		// If set to true, the system will be simulated even if there are no active particles
		void SetAlwaysSimulate(bool b);

		Vector3 PointToParticleSpace(const Vector3 &p, bool bRotateWithEmitter) const;
		Vector3 PointToParticleSpace(const Vector3 &p) const;
		Vector3 DirectionToParticleSpace(const Vector3 &p, bool bRotateWithEmitter) const;
		Vector3 DirectionToParticleSpace(const Vector3 &p) const;

		// Returns the time the particle system has been alive
		double GetLifeTime() const;
		float GetSimulationTime() const;

		double GetStartTime() const;

		void SetSoftParticles(bool bSoft);
		bool GetSoftParticles() const;

		void SetSortParticles(bool sort);
		bool GetSortParticles() const;

		const Color &GetInitialColor() const;
		void SetInitialColor(const Color &col);

		const Vector4 &GetBloomColorFactor() const;
		void SetBloomColorFactor(const Vector4 &colorFactor);
		std::optional<Vector4> GetEffectiveBloomColorFactor() const;
		bool IsBloomEnabled() const;

		const Vector4 &GetColorFactor() const;
		void SetColorFactor(const Vector4 &colorFactor);

		void Simulate(double tDelta);
		void RecordRender(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, pts::ParticleRenderFlags renderFlags);
		void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CLightComponent *light, uint32_t layerId = 0);
		uint32_t GetParticleCount() const;
		// Same as m_numParticles, minus particles with a radius of 0, alpha of 0 or similar (Invisible particles)
		uint32_t GetRenderParticleCount() const;
		uint32_t GetMaxParticleCount() const;
		const std::vector<pts::CParticle> &GetParticles() const;
		pts::CParticle *GetParticle(size_t idx);
		void Start();
		void Stop();
		void Die(float maxRemainingLifetime = 5.f);
		bool IsDying() const;
		bool IsActive() const;
		bool IsActiveOrPaused() const;
		bool IsEmissionPaused() const;
		void SetParent(CParticleSystemComponent *particle);
		const std::vector<ChildData> &GetChildren() const;
		std::vector<ChildData> &GetChildren();
		void AddChild(CParticleSystemComponent &particle, float delay = 0.f);
		CParticleSystemComponent *AddChild(const std::string &name);
		void RemoveChild(CParticleSystemComponent *particle);
		bool HasChild(CParticleSystemComponent &particle);
		const CParticleSystemComponent *GetParent() const;
		CParticleSystemComponent *GetParent();

		const std::string &GetParticleSystemName() const;
		void SetParticleSystemName(const std::string &name);

		bool SetupParticleSystem(std::string fname, CParticleSystemComponent *parent = nullptr, bool bRecordKeyValues = false);
		bool SetupParticleSystem(const std::unordered_map<std::string, std::string> &values, CParticleSystemComponent *parent = nullptr, bool bRecordKeyValues = false);
		bool SetupParticleSystem(CParticleSystemComponent *parent = nullptr);

		const std::shared_ptr<prosper::IBuffer> &GetParticleBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetParticleAnimationBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetSpriteSheetBuffer() const;
		prosper::IDescriptorSet *GetAnimationDescriptorSet();
		const std::shared_ptr<prosper::IDescriptorSetGroup> &GetAnimationDescriptorSetGroup() const;
		bool IsAnimated() const;
		const std::pair<Vector3, Vector3> &GetRenderBounds() const;
		std::pair<Vector3, Vector3> CalcRenderBounds() const;
		virtual void SetContinuous(bool b) override;

		template<class TInitializer>
		void GetInitializers(std::vector<TInitializer *> &initializers)
		{
			const std::type_info &info = typeid(TInitializer);
			for(unsigned int i = 0; i < m_initializers.size(); i++) {
				if(typeid(*(m_initializers[i])) == info)
					initializers.push_back(static_cast<TInitializer *>(m_initializers[i].get()));
			}
		}
		template<class TOperator>
		void GetOperators(std::vector<TOperator *> &operators)
		{
			const std::type_info &info = typeid(TOperator);
			for(unsigned int i = 0; i < m_operators.size(); i++) {
				if(typeid(*(m_operators[i])) == info)
					operators.push_back(static_cast<TOperator *>(m_operators[i].get()));
			}
		}
		template<class TRenderer>
		void GetRenderers(std::vector<TRenderer *> &renderers)
		{
			const std::type_info &info = typeid(TRenderer);
			for(unsigned int i = 0; i < m_renderers.size(); i++) {
				if(typeid(*(m_renderers[i])) == info)
					renderers.push_back(static_cast<TRenderer *>(m_renderers[i].get()));
			}
		}

		void SetControlPointEntity(ControlPointIndex idx, CBaseEntity &ent);
		void SetControlPointPosition(ControlPointIndex idx, const Vector3 &pos);
		void SetControlPointRotation(ControlPointIndex idx, const Quat &rot);
		void SetControlPointPose(ControlPointIndex idx, const math::Transform &pose, float *optTimestamp = nullptr);

		CBaseEntity *GetControlPointEntity(ControlPointIndex idx) const;
		std::optional<math::Transform> GetControlPointPose(ControlPointIndex idx, float *optOutTimestamp = nullptr) const;
		std::optional<math::Transform> GetPrevControlPointPose(ControlPointIndex idx, float *optOutTimestamp = nullptr) const;
		std::optional<math::Transform> GetControlPointPose(ControlPointIndex idx, float t) const;

		const std::vector<std::unique_ptr<pts::CParticleInitializer, void (*)(pts::CParticleInitializer *)>> &GetInitializers() const;
		const std::vector<std::unique_ptr<pts::CParticleOperator, void (*)(pts::CParticleOperator *)>> &GetOperators() const;
		const std::vector<std::unique_ptr<pts::CParticleRenderer, void (*)(pts::CParticleRenderer *)>> &GetRenderers() const;
		std::vector<std::unique_ptr<pts::CParticleInitializer, void (*)(pts::CParticleInitializer *)>> &GetInitializers();
		std::vector<std::unique_ptr<pts::CParticleOperator, void (*)(pts::CParticleOperator *)>> &GetOperators();
		std::vector<std::unique_ptr<pts::CParticleRenderer, void (*)(pts::CParticleRenderer *)>> &GetRenderers();

		material::SpriteSheetAnimation *GetSpriteSheetAnimation();
		const material::SpriteSheetAnimation *GetSpriteSheetAnimation() const;

		std::shared_ptr<asset::Model> GenerateModel() const;

		uint32_t GetMaxNodes() const { return m_maxNodes; }
		void SetMaxNodes(uint32_t maxNodes) { m_maxNodes = maxNodes; }

		// For internal use only
		Flags GetFlags() const { return m_flags; }
		void SetFlags(Flags flags) { m_flags = flags; }
		void SetMaxParticleCount(uint32_t count);
	  protected:
		util::EventReply HandleKeyValue(const std::string &key, const std::string &value);

		virtual void CreateParticle();
	  private:
		static std::unordered_map<std::string, std::unique_ptr<asset::ParticleSystemData>> s_particleData;
		static std::vector<std::string> s_precached;
		ControlPoint &InitializeControlPoint(ControlPointIndex idx);

		struct DLLCLIENT Node {
			Node(CBaseEntity *ent);
			Node(const Vector3 &pos);
			bool bEntity;
			EntityHandle hEntity;
			Vector3 position;
		};
		enum class State : uint32_t { Initial = 0u, Active, Complete, Paused };
		std::vector<ChildData> m_childSystems;
		util::WeakHandle<CParticleSystemComponent> m_hParent = {};
		std::vector<Node> m_nodes;
		std::vector<pts::CParticle> m_particles;
		std::vector<std::size_t> m_sortedParticleIndices;
		std::vector<std::size_t> m_particleIndicesToBufferIndices;
		std::vector<std::size_t> m_bufferIndicesToParticleIndices;
		bool FindFreeParticle(uint32_t *idx);

		rendering::SceneRenderPass m_renderPass = rendering::SceneRenderPass::World;
		std::string m_particleSystemName;
		Quat m_particleRot = uquat::identity();
		uint32_t m_idxLast = 0u;
		uint32_t m_emissionRate = 0u;
		uint32_t m_nextParticleEmissionCount = std::numeric_limits<uint32_t>::max();
		Color m_initialColor = colors::White;
		Flags m_flags = static_cast<Flags>(math::to_integral(Flags::SoftParticles) | math::to_integral(Flags::AutoSimulate));
		std::vector<CallbackHandle> m_renderCallbacks;
		std::pair<Vector3, Vector3> m_renderBounds = {{}, {}};
		uint32_t m_maxParticles = 0u;
		uint32_t m_particleLimit = std::numeric_limits<uint32_t>::max();
		uint32_t m_currentParticleLimit = std::numeric_limits<uint32_t>::max();
		uint32_t m_maxParticlesCur = 0u;
		uint32_t m_prevMaxParticlesCur = std::numeric_limits<uint32_t>::max();
		uint32_t m_numParticles = 0u;
		uint32_t m_numRenderParticles = 0;
		uint32_t m_numPrevRenderParticles = 0;
		uint32_t m_maxNodes = 0u;
		Vector3 m_origin = {};
		float m_lifeTime = std::numeric_limits<float>::max();
		float m_simulationTime = 0.f;
		rendering::ParticleAlphaMode m_alphaMode = rendering::ParticleAlphaMode::Additive;
		std::vector<std::unique_ptr<pts::CParticleInitializer, void (*)(pts::CParticleInitializer *)>> m_initializers;
		std::vector<std::unique_ptr<pts::CParticleOperator, void (*)(pts::CParticleOperator *)>> m_operators;
		std::vector<std::unique_ptr<pts::CParticleRenderer, void (*)(pts::CParticleRenderer *)>> m_renderers;

		std::vector<ControlPoint> m_controlPoints {};
		std::vector<ControlPoint> m_controlPointsPrev {};
		material::MaterialHandle m_material {};
		float m_tNextEmission = 0.f;
		double m_tLastEmission = 0.0;
		double m_tLifeTime = 0.0;
		double m_tStartTime = 0.0;
		float m_radius = 0.f;
		float m_extent = 0.f;
		pts::ParticleOrientationType m_orientationType = pts::ParticleOrientationType::Aligned;

		std::shared_ptr<prosper::IBuffer> m_bufParticles = nullptr;
		std::shared_ptr<prosper::IBuffer> m_bufParticleAnimData = nullptr;

		std::unique_ptr<material::SpriteSheetAnimation> m_spriteSheetAnimationData = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupAnimation = nullptr;
		std::shared_ptr<prosper::IBuffer> m_bufSpriteSheet = nullptr;

		std::vector<ParticleData> m_instanceData;
		std::vector<ParticleAnimationData> m_particleAnimData = {};
		Vector4 m_colorFactor = {1.f, 1.f, 1.f, 1.f};
		Vector4 m_bloomColorFactor = {1.f, 1.f, 1.f, 1.f};
		State m_state = State::Initial;

		// Only with OrientationType::World
		float m_worldScale = 1.f;

		void SortParticles();
		pts::CParticle &CreateParticle(uint32_t idx, float timeCreated, float timeAlive);
		uint32_t CreateParticles(uint32_t count, double tSimDelta, float tStart, float tDtPerParticle);
		void OnParticleDestroyed(pts::CParticle &particle);
		void OnComplete();
	};
};
export {
	REGISTER_ENUM_FLAGS(pragma::ecs::CParticleSystemComponent::Flags)
}
export {
	REGISTER_ENUM_FLAGS(pragma::pts::ParticleRenderFlags)
}
export {
	namespace pragma::ecs {
		class DLLCLIENT CEnvParticleSystem : public CBaseEntity {
		  public:
			virtual void Initialize() override;
		};
	};
};
