// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:particle_system.modifier;

export import :particle_system.enums;
export import :particle_system.particle;
export import pragma.prosper;
export import pragma.shared;

#pragma warning(push)
#pragma warning(disable : 4251)
export {
	namespace pragma {
		class CLightComponent;
		class CSceneComponent;
		class CRasterizationRendererComponent;
		class ShaderParticleBase;
		namespace ecs {
			class CParticleSystemComponent;
		}
	};
	namespace pragma::pts {
		class DLLCLIENT CParticleModifier : public CParticleSystemBaseKeyValues {
		public:
			const std::string &GetName() const;
			void SetName(const std::string &name);
			const std::string &GetType() const;
			void SetType(const std::string &type);
			// Called when a new particle has been created
			virtual void OnParticleCreated(pragma::pts::CParticle &particle);
			// Called when the particle system has been started
			virtual void OnParticleSystemStarted();
			// Called when a particle has been destroyed
			virtual void OnParticleDestroyed(pragma::pts::CParticle &particle);
			// Called when the particle system has been stopped
			virtual void OnParticleSystemStopped();
			pragma::ecs::CParticleSystemComponent &GetParticleSystem() const;
			virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values);

			int32_t GetPriority() const;
			void SetPriority(int32_t priority);
		protected:
			virtual ~CParticleModifier() = default;
			mutable pragma::ecs::CParticleSystemComponent *m_particleSystem = nullptr;
			CParticleModifier() = default;
		private:
			std::string m_name;
			std::string m_type;
			int32_t m_priority = 0;
		};

		///////////////////////

		class DLLCLIENT CParticleInitializer : public CParticleModifier {
		public:
			CParticleInitializer() = default;
		};

		///////////////////////

		class DLLCLIENT CParticleOperator : public CParticleModifier {
		public:
			CParticleOperator() = default;
			virtual void PreSimulate(pragma::pts::CParticle &particle, double tDelta);
			void Simulate(pragma::pts::CParticle &particle, double tDelta);
			virtual void PostSimulate(pragma::pts::CParticle &particle, double tDelta);
			virtual void Simulate(double tDelta);
			virtual void Simulate(pragma::pts::CParticle &particle, double tDelta, float strength);
			virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override;
			float CalcStrength(float curTime) const;
		private:
			float m_opStartFadein = 0.f;
			float m_opEndFadein = 0.f;
			float m_opStartFadeout = 0.f;
			float m_opEndFadeout = 0.f;
			float m_opFadeOscillate = 0.f;
		};

		class DLLCLIENT CParticleOperatorLifespanDecay : public CParticleOperator {
		public:
			CParticleOperatorLifespanDecay() = default;
			virtual void Simulate(pragma::pts::CParticle &particle, double tDelta, float strength) override;
		};

		///////////////////////

		class DLLCLIENT CParticleRenderer : public CParticleModifier {
		public:
			CParticleRenderer() = default;
			virtual void RecordRender(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, ParticleRenderFlags renderFlags) = 0;
			virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, pragma::CLightComponent &light, uint32_t layerId = 0) = 0;
			virtual void PostSimulate(double tDelta);
			virtual void PreRender(prosper::ICommandBuffer &cmd);
			virtual std::pair<Vector3, Vector3> GetRenderBounds() const;
			virtual pragma::ShaderParticleBase *GetShader() const = 0;
			virtual bool RequiresDepthPass() const { return false; }
		};
	}
};
#pragma warning(pop)

///////////////////////

namespace pragma::pts {
	template<class T>
	using TParticleModifierFactory = std::function<std::unique_ptr<T, void (*)(T *)>(pragma::ecs::CParticleSystemComponent &, const std::unordered_map<std::string, std::string> &)>;
	DLLCLIENT void LinkParticleInitializerToFactory(std::string name, const TParticleModifierFactory<pragma::pts::CParticleInitializer> &fc);
	DLLCLIENT void LinkParticleOperatorToFactory(std::string name, const TParticleModifierFactory<pragma::pts::CParticleOperator> &fc);
	DLLCLIENT void LinkParticleRendererToFactory(std::string name, const TParticleModifierFactory<pragma::pts::CParticleRenderer> &fc);

	class DLLCLIENT __reg_particle_modifier {
	  public:
		__reg_particle_modifier(std::string name, const TParticleModifierFactory<pragma::pts::CParticleInitializer> &fc)
		{
			LinkParticleInitializerToFactory(name, fc);
			delete this;
		}
		__reg_particle_modifier(std::string name, const TParticleModifierFactory<pragma::pts::CParticleOperator> &fc)
		{
			LinkParticleOperatorToFactory(name, fc);
			delete this;
		}
		__reg_particle_modifier(std::string name, const TParticleModifierFactory<pragma::pts::CParticleRenderer> &fc)
		{
			LinkParticleRendererToFactory(name, fc);
			delete this;
		}
	};

#pragma warning(push)
#pragma warning(disable : 4251)
	class DLLCLIENT ParticleModifierMap {
	  private:
		std::unordered_map<std::string, TParticleModifierFactory<pragma::pts::CParticleInitializer>> m_initializers;
		std::unordered_map<std::string, TParticleModifierFactory<pragma::pts::CParticleOperator>> m_operators;
		std::unordered_map<std::string, TParticleModifierFactory<pragma::pts::CParticleRenderer>> m_renderers;
	  public:
		void AddInitializer(std::string name, const TParticleModifierFactory<pragma::pts::CParticleInitializer> &fc);
		void AddOperator(std::string name, const TParticleModifierFactory<pragma::pts::CParticleOperator> &fc);
		void AddRenderer(std::string name, const TParticleModifierFactory<pragma::pts::CParticleRenderer> &fc);
		TParticleModifierFactory<pragma::pts::CParticleInitializer> FindInitializer(std::string classname);
		TParticleModifierFactory<pragma::pts::CParticleOperator> FindOperator(std::string classname);
		TParticleModifierFactory<pragma::pts::CParticleRenderer> FindRenderer(std::string classname);

		const std::unordered_map<std::string, TParticleModifierFactory<pragma::pts::CParticleInitializer>> &GetInitializers() const;
		const std::unordered_map<std::string, TParticleModifierFactory<pragma::pts::CParticleOperator>> &GetOperators() const;
		const std::unordered_map<std::string, TParticleModifierFactory<pragma::pts::CParticleRenderer>> &GetRenderers() const;
	};
#pragma warning(pop)

	DLLCLIENT ParticleModifierMap *GetParticleModifierMap();
};
