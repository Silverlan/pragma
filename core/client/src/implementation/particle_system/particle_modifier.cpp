// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.particle_system;
import :particle_system.lua_particle_modifier_manager;

namespace pragma::pts {
	class DLLCLIENT CParticleInitializerLifetimeRandom : public CParticleInitializer {
	  private:
		float m_lifeMin = 0.f;
		float m_lifeMax = 0.f;
	  public:
		CParticleInitializerLifetimeRandom() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override
		{
			CParticleInitializer::Initialize(pSystem, values);
			for(auto it = values.begin(); it != values.end(); it++) {
				std::string key = it->first;
				string::to_lower(key);
				if(key == "lifetime_min")
					m_lifeMin = util::to_float(it->second);
				else if(key == "lifetime_max")
					m_lifeMax = util::to_float(it->second);
			}
		}
		virtual void OnParticleCreated(CParticle &particle) override { particle.SetLife(math::random(m_lifeMin, m_lifeMax)); }
	};

	class DLLCLIENT CParticleInitializerColorRandom : public CParticleInitializer {
	  private:
		Color m_colorA = colors::White;
		Color m_colorB = colors::White;
		std::unique_ptr<Color> m_colorC = nullptr;
	  public:
		CParticleInitializerColorRandom() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override
		{
			CParticleInitializer::Initialize(pSystem, values);
			for(auto it = values.begin(); it != values.end(); it++) {
				std::string key = it->first;
				string::to_lower(key);
				if(key == "color1")
					m_colorA = Color(it->second);
				else if(key == "color2")
					m_colorB = Color(it->second);
				else if(key == "color3")
					m_colorC = std::make_unique<Color>(it->second);
			}
		}
		virtual void OnParticleCreated(CParticle &particle) override
		{
			auto col = m_colorA.Lerp(m_colorB, math::random(0.f, 1.f));
			if(m_colorC != nullptr)
				col = col.Lerp(*m_colorC, math::random(0.f, 1.f));
			particle.SetColor(col);
		}
	};

	class DLLCLIENT CParticleInitializerAlphaRandom : public CParticleInitializer {
	  private:
		float m_alphaMin = 0.f;
		float m_alphaMax = 255.f;
	  public:
		CParticleInitializerAlphaRandom() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override
		{
			CParticleInitializer::Initialize(pSystem, values);
			for(auto it = values.begin(); it != values.end(); it++) {
				std::string key = it->first;
				string::to_lower(key);
				if(key == "alpha_min")
					m_alphaMin = util::to_float(it->second);
				else if(key == "alpha_max")
					m_alphaMax = util::to_float(it->second);
			}
		}
		virtual void OnParticleCreated(CParticle &particle) override
		{
			auto &col = particle.GetColor();
			col.a = math::random(m_alphaMin, m_alphaMax) / 255.f;
		}
	};

	class DLLCLIENT CParticleInitializerRotationRandom : public CParticleInitializer {
	  private:
		EulerAngles m_rotMin = EulerAngles(-180.f, -180.f, -180.f);
		EulerAngles m_rotMax = EulerAngles(180.f, 180.f, 180.f);
		Quat m_rot = {};
		float m_planarRotMin = 0.f;
		float m_planarRotMax = 0.f;
		bool m_bUseQuaternionRotation = false;
	  public:
		CParticleInitializerRotationRandom() = default;
		virtual void Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override
		{
			CParticleInitializer::Initialize(pSystem, values);
			for(auto it = values.begin(); it != values.end(); it++) {
				std::string key = it->first;
				string::to_lower(key);
				if(key == "rotation_quat") {
					m_rot = uquat::create(it->second);
					m_bUseQuaternionRotation = true;
				}
				else if(key == "rotation_min") {
					m_rotMin = EulerAngles(it->second);
					m_planarRotMin = util::to_float(it->second);
				}
				else if(key == "rotation_max") {
					m_rotMax = EulerAngles(it->second);
					m_planarRotMax = util::to_float(it->second);
				}
			}
		}
		virtual void OnParticleCreated(CParticle &particle) override
		{
			if(m_bUseQuaternionRotation == true) {
				particle.SetWorldRotation(m_rot);
				return;
			}
			particle.SetWorldRotation(uquat::create(EulerAngles(math::random(m_rotMin.p, m_rotMax.p), math::random(m_rotMin.y, m_rotMax.y), math::random(m_rotMin.r, m_rotMax.r))));
			particle.SetRotation(math::random(m_planarRotMin, m_planarRotMax));
		}
	};
}

///////////////////////

void pragma::pts::CParticleModifier::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	m_particleSystem = &static_cast<ecs::CParticleSystemComponent &>(pSystem);
	RecordKeyValues(values);
}

int32_t pragma::pts::CParticleModifier::GetPriority() const { return m_priority; }
void pragma::pts::CParticleModifier::SetPriority(int32_t priority) { m_priority = priority; }

pragma::ecs::CParticleSystemComponent &pragma::pts::CParticleModifier::GetParticleSystem() const { return *m_particleSystem; }

void pragma::pts::CParticleModifier::OnParticleCreated(CParticle &) {}
void pragma::pts::CParticleModifier::OnParticleSystemStarted() {}
void pragma::pts::CParticleModifier::OnParticleDestroyed(CParticle &) {}
void pragma::pts::CParticleModifier::OnParticleSystemStopped() {}
void pragma::pts::CParticleModifier::SetName(const std::string &name) { m_name = name; }
const std::string &pragma::pts::CParticleModifier::GetType() const { return m_type; }
void pragma::pts::CParticleModifier::SetType(const std::string &type) { m_type = type; }
const std::string &pragma::pts::CParticleModifier::GetName() const { return m_name; }

///////////////////////

void pragma::pts::CParticleOperator::Initialize(BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	CParticleModifier::Initialize(pSystem, values);
	for(auto &pair : values) {
		if(pragma::string::compare<std::string>(pair.first, "op_start_fadein"))
			m_opStartFadein = util::to_float(pair.second);
		else if(pragma::string::compare<std::string>(pair.first, "op_start_fadeout"))
			m_opStartFadeout = util::to_float(pair.second);
		else if(pragma::string::compare<std::string>(pair.first, "op_end_fadein"))
			m_opEndFadein = util::to_float(pair.second);
		else if(pragma::string::compare<std::string>(pair.first, "op_end_fadeout"))
			m_opEndFadeout = util::to_float(pair.second);
		else if(pragma::string::compare<std::string>(pair.first, "op_fade_oscillate_period"))
			m_opFadeOscillate = util::to_float(pair.second);
	}
}
float pragma::pts::CParticleOperator::CalcStrength(float curTime) const
{
	auto flTime = curTime;
	if(m_opFadeOscillate > 0.f)
		flTime = fmodf(curTime * (1.0 / m_opFadeOscillate), 1.f);
	return math::fade_in_out(m_opStartFadein, m_opEndFadein, m_opStartFadeout, m_opEndFadeout, flTime);
}
void pragma::pts::CParticleOperator::Simulate(double) {}

void pragma::pts::CParticleOperator::PreSimulate(CParticle &particle, double tDelta) {}

void pragma::pts::CParticleOperator::Simulate(CParticle &pt, double dt)
{
	auto strength = CalcStrength(pt.GetTimeAlive());
	Simulate(pt, dt, strength);
}

void pragma::pts::CParticleOperator::Simulate(CParticle &particle, double tDelta, float strength) {}

void pragma::pts::CParticleOperator::PostSimulate(CParticle &particle, double tDelta) {}

void pragma::pts::CParticleOperatorLifespanDecay::Simulate(CParticle &, double, float strength) {}

///////////////////////

void pragma::pts::CParticleRenderer::PostSimulate(double tDelta) {}

void pragma::pts::CParticleRenderer::PreRender(prosper::ICommandBuffer &cmd) {}

std::pair<Vector3, Vector3> pragma::pts::CParticleRenderer::GetRenderBounds() const { return {uvec::PRM_ORIGIN, uvec::PRM_ORIGIN}; }

///////////////////////

DLLCLIENT pragma::pts::ParticleModifierMap g_ParticleModifierFactories;
DLLCLIENT void pragma::pts::LinkParticleInitializerToFactory(std::string name, const TParticleModifierFactory<CParticleInitializer> &fc) { g_ParticleModifierFactories.AddInitializer(name, fc); }
DLLCLIENT void pragma::pts::LinkParticleOperatorToFactory(std::string name, const TParticleModifierFactory<CParticleOperator> &fc) { g_ParticleModifierFactories.AddOperator(name, fc); }
DLLCLIENT void pragma::pts::LinkParticleRendererToFactory(std::string name, const TParticleModifierFactory<CParticleRenderer> &fc) { g_ParticleModifierFactories.AddRenderer(name, fc); }
pragma::pts::ParticleModifierMap &pragma::pts::get_particle_modifier_map() { return g_ParticleModifierFactories; }

template<class TModifier, class TBaseType>
static std::unique_ptr<TBaseType, void (*)(TBaseType *)> create_modifier(pragma::ecs::CParticleSystemComponent &system, const std::unordered_map<std::string, std::string> &values)
{
	auto r = std::unique_ptr<TBaseType, void (*)(TBaseType *)>(new TModifier {}, [](TBaseType *p) { delete p; });
	r->Initialize(system, values);
	return r;
}

void pragma::pts::register_particle_operators()
{
	auto &map = get_particle_modifier_map();
	map.AddInitializer("alpha_random", &create_modifier<CParticleInitializerAlphaRandom, CParticleInitializer>);
	// TODO: Why do we have CParticleInitializerColorRandom *and* CParticleInitializerColor?
	map.AddInitializer("color_random", &create_modifier<CParticleInitializerColorRandom, CParticleInitializer>);
	// map.AddInitializer("color_random", &create_modifier<CParticleInitializerColor, CParticleInitializer>);
	map.AddInitializer("initial_angular_velocity", &create_modifier<CParticleInitializerInitialAngularVelocity, CParticleInitializer>);
	map.AddInitializer("initial_animation_frame", &create_modifier<CParticleInitializerInitialAnimationFrame, CParticleInitializer>);
	map.AddInitializer("initial_velocity", &create_modifier<CParticleInitializerInitialVelocity, CParticleInitializer>);
	map.AddInitializer("length_random", &create_modifier<CParticleInitializerLengthRandom, CParticleInitializer>);
	map.AddInitializer("lifetime_random", &create_modifier<CParticleInitializerLifetimeRandom, CParticleInitializer>);
	map.AddInitializer("position_random_box", &create_modifier<CParticleInitializerPositionRandomBox, CParticleInitializer>);
	map.AddInitializer("position_random_circle", &create_modifier<CParticleInitializerPositionRandomCircle, CParticleInitializer>);
	map.AddInitializer("position_random_sphere", &create_modifier<CParticleInitializerPositionRandomSphere, CParticleInitializer>);
	map.AddInitializer("radius_random", &create_modifier<CParticleInitializerRadiusRandom, CParticleInitializer>);
	map.AddInitializer("rotation_random", &create_modifier<CParticleInitializerRotationRandom, CParticleInitializer>);
	map.AddInitializer("shoot_cone", &create_modifier<CParticleInitializerShootCone, CParticleInitializer>);
	map.AddInitializer("shoot_outward", &create_modifier<CParticleInitializerShootOutward, CParticleInitializer>);
	map.AddInitializer("speed", &create_modifier<CParticleInitializerSpeed, CParticleInitializer>);

	map.AddOperator("angular_acceleration", &create_modifier<CParticleOperatorAngularAcceleration, CParticleOperator>);
	map.AddOperator("animation_playback", &create_modifier<CParticleOperatorAnimationPlayback, CParticleOperator>);
	map.AddOperator("color_fade", &create_modifier<CParticleOperatorColorFade, CParticleOperator>);
	map.AddOperator("cylindrical_vortex", &create_modifier<CParticleOperatorCylindricalVortex, CParticleOperator>);
	map.AddOperator("emission_rate_random", &create_modifier<CParticleOperatorRandomEmissionRate, CParticleOperator>);
	map.AddOperator("gravity", &create_modifier<CParticleOperatorGravity, CParticleOperator>);
	map.AddOperator("jitter", &create_modifier<CParticleOperatorJitter, CParticleOperator>);
	map.AddOperator("length_fade", &create_modifier<CParticleOperatorLengthFade, CParticleOperator>);
	map.AddOperator("linear_drag", &create_modifier<CParticleOperatorLinearDrag, CParticleOperator>);
	map.AddOperator("pause_child_emission", &create_modifier<CParticleOperatorPauseChildEmission, CParticleOperator>);
	map.AddOperator("pause_emission", &create_modifier<CParticleOperatorPauseEmission, CParticleOperator>);
	map.AddOperator("physics_box", &create_modifier<CParticleOperatorPhysicsBox, CParticleOperator>);
	map.AddOperator("physics_cylinder", &create_modifier<CParticleOperatorPhysicsCylinder, CParticleOperator>);
	map.AddOperator("physics_model", &create_modifier<CParticleOperatorPhysicsModel, CParticleOperator>);
	map.AddOperator("physics_sphere", &create_modifier<CParticleOperatorPhysicsSphere, CParticleOperator>);
	map.AddOperator("quadratic_drag", &create_modifier<CParticleOperatorQuadraticDrag, CParticleOperator>);
	map.AddOperator("radius_fade", &create_modifier<CParticleOperatorRadiusFade, CParticleOperator>);
	map.AddOperator("texture_scrolling", &create_modifier<CParticleOperatorTextureScrolling, CParticleOperator>);
	map.AddOperator("toroidal_vortex", &create_modifier<CParticleOperatorToroidalVortex, CParticleOperator>);
	map.AddOperator("trail", &create_modifier<CParticleOperatorTrail, CParticleOperator>);
	map.AddOperator("velocity", &create_modifier<CParticleOperatorVelocity, CParticleOperator>);
	map.AddOperator("wander", &create_modifier<CParticleOperatorWander, CParticleOperator>);
	map.AddOperator("wind", &create_modifier<CParticleOperatorWind, CParticleOperator>);

	map.AddRenderer("beam", &create_modifier<CParticleRendererBeam, CParticleRenderer>);
	map.AddRenderer("blob", &create_modifier<CParticleRendererBlob, CParticleRenderer>);
	map.AddRenderer("model", &create_modifier<CParticleRendererModel, CParticleRenderer>);
	map.AddRenderer("source_render_animated_sprites", &create_modifier<CParticleRendererAnimatedSprites, CParticleRenderer>);
	map.AddRenderer("source_render_sprite_trail", &create_modifier<CParticleRendererSpriteTrail, CParticleRenderer>);
	map.AddRenderer("sprite", &create_modifier<CParticleRendererSprite, CParticleRenderer>);
}

void pragma::pts::ParticleModifierMap::AddInitializer(std::string name, const TParticleModifierFactory<CParticleInitializer> &fc)
{
	string::to_lower(name);
	m_initializers.insert(
	  std::make_pair(name, pragma::pts::TParticleModifierFactory<CParticleInitializer> {[fc, name](ecs::CParticleSystemComponent &c, const std::unordered_map<std::string, std::string> &keyvalues) -> std::unique_ptr<CParticleInitializer, void (*)(CParticleInitializer *)> {
		  auto initializer = fc(c, keyvalues);
		  if(initializer)
			  initializer->SetType(name);
		  return initializer;
	  }}));
}
void pragma::pts::ParticleModifierMap::AddOperator(std::string name, const TParticleModifierFactory<CParticleOperator> &fc)
{
	string::to_lower(name);
	m_operators.insert(std::make_pair(name, pragma::pts::TParticleModifierFactory<CParticleOperator> {[fc, name](ecs::CParticleSystemComponent &c, const std::unordered_map<std::string, std::string> &keyvalues) -> std::unique_ptr<CParticleOperator, void (*)(CParticleOperator *)> {
		auto op = fc(c, keyvalues);
		if(op)
			op->SetType(name);
		return op;
	}}));
}
void pragma::pts::ParticleModifierMap::AddRenderer(std::string name, const TParticleModifierFactory<CParticleRenderer> &fc)
{
	string::to_lower(name);
	m_renderers.insert(std::make_pair(name, pragma::pts::TParticleModifierFactory<CParticleRenderer> {[fc, name](ecs::CParticleSystemComponent &c, const std::unordered_map<std::string, std::string> &keyvalues) -> std::unique_ptr<CParticleRenderer, void (*)(CParticleRenderer *)> {
		auto renderer = fc(c, keyvalues);
		if(renderer)
			renderer->SetType(name);
		return renderer;
	}}));
}

pragma::pts::TParticleModifierFactory<pragma::pts::CParticleInitializer> pragma::pts::ParticleModifierMap::FindInitializer(std::string classname)
{
	auto it = m_initializers.find(classname);
	if(it == m_initializers.end())
		return nullptr;
	return it->second;
}
pragma::pts::TParticleModifierFactory<pragma::pts::CParticleOperator> pragma::pts::ParticleModifierMap::FindOperator(std::string classname)
{
	auto it = m_operators.find(classname);
	if(it == m_operators.end())
		return nullptr;
	return it->second;
}
pragma::pts::TParticleModifierFactory<pragma::pts::CParticleRenderer> pragma::pts::ParticleModifierMap::FindRenderer(std::string classname)
{
	auto it = m_renderers.find(classname);
	if(it == m_renderers.end())
		return nullptr;
	return it->second;
}
const std::unordered_map<std::string, pragma::pts::TParticleModifierFactory<pragma::pts::CParticleInitializer>> &pragma::pts::ParticleModifierMap::GetInitializers() const { return m_initializers; }
const std::unordered_map<std::string, pragma::pts::TParticleModifierFactory<pragma::pts::CParticleOperator>> &pragma::pts::ParticleModifierMap::GetOperators() const { return m_operators; }
const std::unordered_map<std::string, pragma::pts::TParticleModifierFactory<pragma::pts::CParticleRenderer>> &pragma::pts::ParticleModifierMap::GetRenderers() const { return m_renderers; }
