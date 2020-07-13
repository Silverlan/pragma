/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_ENV_PARTICLE_SYSTEM_H__
#define __C_ENV_PARTICLE_SYSTEM_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/effects/env_particle_system.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/particlesystem/c_particle.h"
#include "pragma/rendering/c_alpha_mode.hpp"
#include "pragma/particlesystem/c_particlemodifier.h"
#include <mathutil/transform.hpp>
#include <fsys/vfileptr.h>
#include <optional>

class CParticleSystemData;
class CGame;
namespace pragma
{
	struct DLLCLIENT ParticleSystemFileHeader
	{
		uint32_t version = 0;
		uint32_t numParticles = 0;
		std::vector<std::string> particleSystemNames {};
		std::vector<uint64_t> particleSystemOffsets {};
	};
	enum class ParticleRenderFlags : uint32_t
	{
		None = 0u,
		Bloom = 1u,
		DepthOnly = Bloom<<1u
	};
	class CParticleSystemComponent;
	class DLLCLIENT CParticleSystemComponent final
		: public BaseEnvParticleSystemComponent,
		public CBaseNetComponent,
		public CParticleSystemBaseKeyValues
	{
	public:
		static const uint32_t PARTICLE_DATA_SIZE;
		static const uint32_t VERTEX_COUNT;
		static bool Save(const std::string &fileName,const std::vector<pragma::CParticleSystemComponent*> &particleSystems);
		static bool Save(VFilePtrReal &f,const std::vector<pragma::CParticleSystemComponent*> &particleSystems);
		static bool IsParticleFilePrecached(const std::string &fname);
		static void InitializeBuffers();
		static void ClearBuffers();
		static std::optional<ParticleSystemFileHeader> ReadHeader(NetworkState &nw,const std::string &fileName);
		static std::optional<ParticleSystemFileHeader> ReadHeader(VFilePtr &f);
		static bool Precache(std::string fname,bool bReload=false);
		static const std::vector<std::string> &GetPrecachedParticleSystemFiles();
		static std::optional<std::string> FindParticleSystemFile(const std::string ptName);
		static const std::unordered_map<std::string,std::unique_ptr<CParticleSystemData>> &GetCachedParticleSystemData();
		static void ClearCache();

		static CParticleSystemComponent *Create(const std::string &fname,CParticleSystemComponent *parent=nullptr,bool bRecordKeyValues=false,bool bAutoSpawn=true);
		static CParticleSystemComponent *Create(const std::unordered_map<std::string,std::string> &values,CParticleSystemComponent *parent=nullptr,bool bRecordKeyValues=false,bool bAutoSpawn=true);
		static CParticleSystemComponent *Create(CParticleSystemComponent *parent=nullptr,bool bAutoSpawn=true);
		static std::shared_ptr<Model> GenerateModel(CGame &game,const std::vector<const CParticleSystemComponent*> &particleSystems);

		enum class OrientationType : uint8_t
		{
			Aligned = 0,
			Upright,
			Static,
			World,
			Billboard,

			Velocity // Velocity is a special enum that shouldn't be set directly
		};
		enum class Flags : uint32_t
		{
			None = 0u,
			SoftParticles = 1u,
			TextureScrollingEnabled = SoftParticles<<1u,
			RendererBufferUpdateRequired = TextureScrollingEnabled<<1u,
			HasMovingParticles = RendererBufferUpdateRequired<<1u,
			MoveWithEmitter = HasMovingParticles<<1u,
			RotateWithEmitter = MoveWithEmitter<<1u,
			SortParticles = RotateWithEmitter<<1u,
			Dying = SortParticles<<1u,
			RandomStartFrame = Dying<<1u,
			PremultiplyAlpha = RandomStartFrame<<1u,
			AlwaysSimulate = PremultiplyAlpha<<1u,
			CastShadows = AlwaysSimulate<<1u,
			Setup = CastShadows<<1u, /* Has this system been set up already? */
			AutoSimulate = Setup<<1u
		};

		using ControlPointIndex = uint32_t;
		struct DLLCLIENT ControlPoint
		{
			float simTimestamp = 0.f;
			umath::Transform pose = {};
			EntityHandle hEntity = {};
		};

		struct DLLCLIENT ChildData
		{
			util::WeakHandle<CParticleSystemComponent> child {};
			float delay = 0.f;
		};

#pragma pack(push,1)
		struct DLLCLIENT ParticleData
		{
			// Padding for std140 alignment rules (16-byte alignment), required for use in storage buffer
			Vector3 position = {};
			float radius = 0.f;
			Vector3 prevPos = {};
			float age = 0.f;
			std::array<uint16_t,4> color;
			float rotation = 0.f;
			uint16_t length = 0; // Encoded float
			uint16_t rotationYaw = 0.f; // Encoded float

			Color GetColor() const;
		};
		struct DLLCLIENT ParticleAnimationData
		{
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
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void SetRemoveOnComplete(bool b) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void OnEntitySpawn() override;
		virtual void SetParticleFile(const std::string &fileName) override;

		// Particle
		// Returns the buffer index for the specified particle. Only particles which are alive have a valid buffer index!
		std::size_t TranslateParticleIndex(std::size_t particleIdx) const;
		// Translates a buffer index to a particle index
		std::size_t TranslateBufferIndex(std::size_t bufferIdx) const;
		CParticleInitializer *AddInitializer(std::string identifier,const std::unordered_map<std::string,std::string> &values);
		CParticleOperator *AddOperator(std::string identifier,const std::unordered_map<std::string,std::string> &values);
		CParticleRenderer *AddRenderer(std::string identifier,const std::unordered_map<std::string,std::string> &values);
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
		const std::vector<ParticleData> &GetRenderParticleData() const;
		const std::vector<ParticleAnimationData> &GetParticleAnimationData() const;
		void SetMaterial(Material *mat);
		void SetMaterial(const char *mat);
		Material *GetMaterial() const;
		OrientationType GetOrientationType() const;
		void SetOrientationType(OrientationType type);
		void SetNodeTarget(uint32_t node,CBaseEntity *ent);
		void SetNodeTarget(uint32_t node,const Vector3 &pos);
		uint32_t GetNodeCount() const;
		Vector3 GetNodePosition(uint32_t node) const;
		CBaseEntity *GetNodeTarget(uint32_t node) const;
		Vector3 GetParticlePosition(uint32_t ptIdx) const;
		void SetCastShadows(bool b);
		bool GetCastShadows() const;
		float GetStaticWorldScale() const;
		void SetStaticWorldScale(float scale);
		bool IsStatic() const;
		void SetRenderMode(RenderMode mode);
		RenderMode GetRenderMode() const;
		bool IsRendererBufferUpdateRequired() const;
		CallbackHandle AddRenderCallback(const std::function<void(void)> &cb);
		void AddRenderCallback(const CallbackHandle &hCb);
		pragma::ParticleAlphaMode GetAlphaMode() const;
		pragma::ParticleAlphaMode GetEffectiveAlphaMode() const;
		void SetAlphaMode(pragma::ParticleAlphaMode alphaMode);
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

		Vector3 PointToParticleSpace(const Vector3 &p,bool bRotateWithEmitter) const;
		Vector3 PointToParticleSpace(const Vector3 &p) const;
		Vector3 DirectionToParticleSpace(const Vector3 &p,bool bRotateWithEmitter) const;
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
		void Render(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,ParticleRenderFlags renderFlags);
		void RenderShadow(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,const pragma::rendering::RasterizationRenderer &renderer,pragma::CLightComponent *light,uint32_t layerId=0);
		uint32_t GetParticleCount() const;
		// Same as m_numParticles, minus particles with a radius of 0, alpha of 0 or similar (Invisible particles)
		uint32_t GetRenderParticleCount() const;
		uint32_t GetMaxParticleCount() const;
		const std::vector<CParticle> &GetParticles() const;
		CParticle *GetParticle(size_t idx);
		void Start();
		void Stop();
		void Die(float maxRemainingLifetime=5.f);
		bool IsDying() const;
		bool IsActive() const;
		bool IsActiveOrPaused() const;
		bool IsEmissionPaused() const;
		void SetParent(CParticleSystemComponent *particle);
		const std::vector<ChildData> &GetChildren() const;
		std::vector<ChildData> &GetChildren();
		void AddChild(CParticleSystemComponent &particle,float delay=0.f);
		CParticleSystemComponent *AddChild(const std::string &name);
		void RemoveChild(CParticleSystemComponent *particle);
		bool HasChild(CParticleSystemComponent &particle);
		const CParticleSystemComponent *GetParent() const;
		CParticleSystemComponent *GetParent();

		const std::string &GetParticleSystemName() const;
		void SetParticleSystemName(const std::string &name);

		bool SetupParticleSystem(std::string fname,CParticleSystemComponent *parent=nullptr,bool bRecordKeyValues=false);
		bool SetupParticleSystem(const std::unordered_map<std::string,std::string> &values,CParticleSystemComponent *parent=nullptr,bool bRecordKeyValues=false);
		bool SetupParticleSystem(CParticleSystemComponent *parent=nullptr);

		const std::shared_ptr<prosper::IBuffer> &GetParticleBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetParticleAnimationBuffer() const;
		const std::shared_ptr<prosper::IBuffer> &GetSpriteSheetBuffer() const;
		prosper::IDescriptorSet *GetAnimationDescriptorSet();
		const std::shared_ptr<prosper::IDescriptorSetGroup> &GetAnimationDescriptorSetGroup() const;
		bool IsAnimated() const;
		const std::pair<Vector3,Vector3> &GetRenderBounds() const;
		std::pair<Vector3,Vector3> CalcRenderBounds() const;
		virtual void SetContinuous(bool b) override;

		template<class TInitializer>
			void GetInitializers(std::vector<TInitializer*> &initializers);
		template<class TOperator>
			void GetOperators(std::vector<TOperator*> &operators);
		template<class TRenderer>
			void GetRenderers(std::vector<TRenderer*> &renderers);

		void SetControlPointEntity(ControlPointIndex idx,CBaseEntity &ent);
		void SetControlPointPosition(ControlPointIndex idx,const Vector3 &pos);
		void SetControlPointRotation(ControlPointIndex idx,const Quat &rot);
		void SetControlPointPose(ControlPointIndex idx,const umath::Transform &pose,float *optTimestamp=nullptr);

		CBaseEntity *GetControlPointEntity(ControlPointIndex idx) const;
		std::optional<umath::Transform> GetControlPointPose(ControlPointIndex idx,float *optOutTimestamp=nullptr) const;
		std::optional<umath::Transform> GetPrevControlPointPose(ControlPointIndex idx,float *optOutTimestamp=nullptr) const;
		std::optional<umath::Transform> GetControlPointPose(ControlPointIndex idx,float t) const;

		const std::vector<std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)>> &GetInitializers() const;
		const std::vector<std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)>> &GetOperators() const;
		const std::vector<std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)>> &GetRenderers() const;
		std::vector<std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)>> &GetInitializers();
		std::vector<std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)>> &GetOperators();
		std::vector<std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)>> &GetRenderers();

		SpriteSheetAnimation *GetSpriteSheetAnimation();
		const SpriteSheetAnimation *GetSpriteSheetAnimation() const;

		std::shared_ptr<Model> GenerateModel() const;
	protected:
		util::EventReply HandleKeyValue(const std::string &key,const std::string &value);

		virtual void CreateParticle();
	private:
		static std::unordered_map<std::string,std::unique_ptr<CParticleSystemData>> s_particleData;
		static std::vector<std::string> s_precached;
		ControlPoint &InitializeControlPoint(ControlPointIndex idx);

		struct DLLCLIENT Node
		{
			Node(CBaseEntity *ent);
			Node(const Vector3 &pos);
			bool bEntity;
			EntityHandle hEntity;
			Vector3 position;
		};
		enum class State : uint32_t
		{
			Initial = 0u,
			Active,
			Complete,
			Paused
		};
		std::vector<ChildData> m_childSystems;
		util::WeakHandle<CParticleSystemComponent> m_hParent = {};
		std::vector<Node> m_nodes;
		std::vector<CParticle> m_particles;
		std::vector<std::size_t> m_sortedParticleIndices;
		std::vector<std::size_t> m_particleIndicesToBufferIndices;
		std::vector<std::size_t> m_bufferIndicesToParticleIndices;
		bool FindFreeParticle(uint32_t *idx);

		RenderMode m_renderMode = RenderMode::World;
		std::string m_particleSystemName;
		Quat m_particleRot = uquat::identity();
		uint32_t m_idxLast = 0u;
		uint32_t m_emissionRate = 0u;
		uint32_t m_nextParticleEmissionCount = std::numeric_limits<uint32_t>::max();
		Color m_initialColor = Color::White;
		Flags m_flags = static_cast<Flags>(umath::to_integral(Flags::SoftParticles) | umath::to_integral(Flags::AutoSimulate));
		std::vector<CallbackHandle> m_renderCallbacks;
		std::pair<Vector3,Vector3> m_renderBounds = {{},{}};
		uint32_t m_maxParticles = 0u;
		uint32_t m_particleLimit  = std::numeric_limits<uint32_t>::max();
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
		pragma::ParticleAlphaMode m_alphaMode = pragma::ParticleAlphaMode::Additive;
		std::vector<std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)>> m_initializers;
		std::vector<std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)>> m_operators;
		std::vector<std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)>> m_renderers;

		std::vector<ControlPoint> m_controlPoints {};
		std::vector<ControlPoint> m_controlPointsPrev {};
		Material *m_material = nullptr;
		float m_tNextEmission = 0.f;
		double m_tLastEmission = 0.0;
		double m_tLifeTime = 0.0;
		double m_tStartTime = 0.0;
		float m_radius = 0.f;
		float m_extent = 0.f;
		OrientationType m_orientationType = OrientationType::Aligned;

		std::shared_ptr<prosper::IBuffer> m_bufParticles = nullptr;
		std::shared_ptr<prosper::IBuffer> m_bufParticleAnimData = nullptr;

		std::unique_ptr<SpriteSheetAnimation> m_spriteSheetAnimationData = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupAnimation = nullptr;
		std::shared_ptr<prosper::IBuffer> m_bufSpriteSheet = nullptr;

		std::vector<ParticleData> m_instanceData;
		std::vector<ParticleAnimationData> m_particleAnimData = {};
		Vector4 m_colorFactor = {1.f,1.f,1.f,1.f};
		Vector4 m_bloomColorFactor = {1.f,1.f,1.f,1.f};
		State m_state = State::Initial;

		// Only with OrientationType::World
		float m_worldScale = 1.f;

		void SortParticles();
		CParticle &CreateParticle(uint32_t idx,float timeCreated,float timeAlive);
		uint32_t CreateParticles(uint32_t count,double tSimDelta,float tStart,float tDtPerParticle);
		void OnParticleDestroyed(CParticle &particle);
		void OnComplete();
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::CParticleSystemComponent::Flags)
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ParticleRenderFlags)

template<class TInitializer>
	void pragma::CParticleSystemComponent::GetInitializers(std::vector<TInitializer*> &initializers)
{
	const std::type_info &info = typeid(TInitializer);
	for(unsigned int i=0;i<m_initializers.size();i++)
	{
		if(typeid(*(m_initializers[i])) == info)
			initializers.push_back(static_cast<TInitializer*>(m_initializers[i].get()));
	}
}
template<class TOperator>
	void pragma::CParticleSystemComponent::GetOperators(std::vector<TOperator*> &operators)
{
	const std::type_info &info = typeid(TOperator);
	for(unsigned int i=0;i<m_operators.size();i++)
	{
		if(typeid(*(m_operators[i])) == info)
			operators.push_back(static_cast<TOperator*>(m_operators[i].get()));
	}
}
template<class TRenderer>
	void pragma::CParticleSystemComponent::GetRenderers(std::vector<TRenderer*> &renderers)
{
	const std::type_info &info = typeid(TRenderer);
	for(unsigned int i=0;i<m_renderers.size();i++)
	{
		if(typeid(*(m_renderers[i])) == info)
			renderers.push_back(static_cast<TRenderer*>(m_renderers[i].get()));
	}
}

class DLLCLIENT CEnvParticleSystem
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif