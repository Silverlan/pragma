// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:particle_system.initializer_lua;

export import :entities.components.particle_system;
export import :particle_system.modifier;
export import :particle_system.enums;
export import :particle_system.particle;

export namespace pragma::pts {
	class DLLCLIENT CParticleModifierLua : public LuaObjectBase {
	  public:
		void Initialize(const luabind::object &o);

		void SetIdentifier(const std::string &identifier);
		const std::string &GetIdentifier() const;
	  private:
		std::string m_identifier = "";
	};

	template<class TModifier>
	class TParticleModifierLua : public TModifier, public CParticleModifierLua {
	  public:
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override
		{
			TModifier::Initialize(pSystem, values);
			// RecordKeyValues(values);
			CallLuaMember("Initialize");
		}
		virtual void OnParticleCreated(CParticle &particle) override
		{
			TModifier::OnParticleCreated(particle);
			CallLuaMember<void, std::reference_wrapper<CParticle>>("OnParticleCreated", std::ref(particle));
		}
		virtual void OnParticleDestroyed(CParticle &particle) override
		{
			TModifier::OnParticleDestroyed(particle);
			CallLuaMember<void, std::reference_wrapper<CParticle>>("OnParticleDestroyed", std::ref(particle));
		}
		virtual void OnParticleSystemStarted() override
		{
			TModifier::OnParticleSystemStarted();
			CallLuaMember<void>("OnParticleSystemStarted");
		}
		virtual void OnParticleSystemStopped() override
		{
			TModifier::OnParticleSystemStopped();
			CallLuaMember<void>("OnParticleSystemStopped");
		}

		void Lua_Initialize() {}
		static void Lua_default_Initialize(lua::State *l, TParticleModifierLua<TModifier> &mod) { mod.Lua_Initialize(); }

		void Lua_OnParticleSystemStarted() {}
		static void Lua_default_OnParticleSystemStarted(lua::State *l, TParticleModifierLua<TModifier> &mod) { mod.Lua_OnParticleSystemStarted(); }

		void Lua_OnParticleSystemStopped() {}
		static void Lua_default_OnParticleSystemStopped(lua::State *l, TParticleModifierLua<TModifier> &mod) { mod.Lua_OnParticleSystemStopped(); }

		void Lua_OnParticleCreated(CParticle &pt) {}
		static void Lua_default_OnParticleCreated(lua::State *l, TParticleModifierLua<TModifier> &mod, CParticle &pt) { mod.Lua_OnParticleCreated(pt); }

		void Lua_OnParticleDestroyed(CParticle &pt) {}
		static void Lua_default_OnParticleDestroyed(lua::State *l, TParticleModifierLua<TModifier> &mod, CParticle &pt) { mod.Lua_OnParticleDestroyed(pt); }
	};

	class DLLCLIENT CParticleInitializerLua : public TParticleModifierLua<CParticleInitializer> {
	  public:
		CParticleInitializerLua() = default;
	};

	class DLLCLIENT CParticleOperatorLua : public TParticleModifierLua<CParticleOperator> {
	  public:
		CParticleOperatorLua() = default;
		virtual void PreSimulate(CParticle &particle, double tDelta) override;
		virtual void Simulate(CParticle &particle, double tDelta, float strength) override;
		virtual void PostSimulate(CParticle &particle, double tDelta) override;
		virtual void Simulate(double tDelta) override;

		void Lua_Simulate(CParticle &pt, float dt, float strength) {}
		static void Lua_default_Simulate(lua::State *l, CParticleOperatorLua &mod, CParticle &pt, float dt, float strength) { mod.Lua_Simulate(pt, dt, strength); }
	};

	class DLLCLIENT CParticleRendererLua : public TParticleModifierLua<CParticleRenderer> {
	  public:
		CParticleRendererLua() = default;
		virtual void RecordRender(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ParticleRenderFlags renderFlags) override;
		virtual void RecordRenderShadow(prosper::ICommandBuffer &drawCmd, CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CLightComponent &light, uint32_t layerId = 0) override;
		virtual ShaderParticleBase *GetShader() const override;
		void SetShader(ShaderParticleBase *shader);

		void Lua_Render(prosper::ICommandBuffer &drawCmd, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, bool bloom) {}
		static void Lua_default_Render(lua::State *l, CParticleRendererLua &mod, prosper::ICommandBuffer &drawCmd, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, bool bloom) { mod.Lua_Render(drawCmd, scene, renderer, bloom); }
	  private:
		ShaderParticleBase *m_shader = nullptr;
	};
};
