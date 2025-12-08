// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

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
		virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override
		{
			CParticleInitializer::Initialize(pSystem, values);
			for(auto it = values.begin(); it != values.end(); it++) {
				std::string key = it->first;
				ustring::to_lower(key);
				if(key == "lifetime_min")
					m_lifeMin = util::to_float(it->second);
				else if(key == "lifetime_max")
					m_lifeMax = util::to_float(it->second);
			}
		}
		virtual void OnParticleCreated(pragma::pts::CParticle &particle) override { particle.SetLife(umath::random(m_lifeMin, m_lifeMax)); }
	};

	class DLLCLIENT CParticleInitializerColorRandom : public CParticleInitializer {
	private:
		Color m_colorA = colors::White;
		Color m_colorB = colors::White;
		std::unique_ptr<Color> m_colorC = nullptr;
	public:
		CParticleInitializerColorRandom() = default;
		virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override
		{
			CParticleInitializer::Initialize(pSystem, values);
			for(auto it = values.begin(); it != values.end(); it++) {
				std::string key = it->first;
				ustring::to_lower(key);
				if(key == "color1")
					m_colorA = Color(it->second);
				else if(key == "color2")
					m_colorB = Color(it->second);
				else if(key == "color3")
					m_colorC = std::make_unique<Color>(it->second);
			}
		}
		virtual void OnParticleCreated(pragma::pts::CParticle &particle) override
		{
			auto col = m_colorA.Lerp(m_colorB, umath::random(0.f, 1.f));
			if(m_colorC != nullptr)
				col = col.Lerp(*m_colorC, umath::random(0.f, 1.f));
			particle.SetColor(col);
		}
	};

	class DLLCLIENT CParticleInitializerAlphaRandom : public CParticleInitializer {
	private:
		float m_alphaMin = 0.f;
		float m_alphaMax = 255.f;
	public:
		CParticleInitializerAlphaRandom() = default;
		virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override
		{
			CParticleInitializer::Initialize(pSystem, values);
			for(auto it = values.begin(); it != values.end(); it++) {
				std::string key = it->first;
				ustring::to_lower(key);
				if(key == "alpha_min")
					m_alphaMin = util::to_float(it->second);
				else if(key == "alpha_max")
					m_alphaMax = util::to_float(it->second);
			}
		}
		virtual void OnParticleCreated(pragma::pts::CParticle &particle) override
		{
			auto &col = particle.GetColor();
			col.a = umath::random(m_alphaMin, m_alphaMax) / 255.f;
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
		virtual void Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values) override
		{
			CParticleInitializer::Initialize(pSystem, values);
			for(auto it = values.begin(); it != values.end(); it++) {
				std::string key = it->first;
				ustring::to_lower(key);
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
		virtual void OnParticleCreated(pragma::pts::CParticle &particle) override
		{
			if(m_bUseQuaternionRotation == true) {
				particle.SetWorldRotation(m_rot);
				return;
			}
			particle.SetWorldRotation(uquat::create(EulerAngles(umath::random(m_rotMin.p, m_rotMax.p), umath::random(m_rotMin.y, m_rotMax.y), umath::random(m_rotMin.r, m_rotMax.r))));
			particle.SetRotation(umath::random(m_planarRotMin, m_planarRotMax));
		}
	};
}

///////////////////////

void pragma::pts::CParticleModifier::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	m_particleSystem = &static_cast<pragma::ecs::CParticleSystemComponent &>(pSystem);
	RecordKeyValues(values);
}

int32_t pragma::pts::CParticleModifier::GetPriority() const { return m_priority; }
void pragma::pts::CParticleModifier::SetPriority(int32_t priority) { m_priority = priority; }

pragma::ecs::CParticleSystemComponent &pragma::pts::CParticleModifier::GetParticleSystem() const { return *m_particleSystem; }

void pragma::pts::CParticleModifier::OnParticleCreated(pragma::pts::CParticle &) {}
void pragma::pts::CParticleModifier::OnParticleSystemStarted() {}
void pragma::pts::CParticleModifier::OnParticleDestroyed(pragma::pts::CParticle &) {}
void pragma::pts::CParticleModifier::OnParticleSystemStopped() {}
void pragma::pts::CParticleModifier::SetName(const std::string &name) { m_name = name; }
const std::string &pragma::pts::CParticleModifier::GetType() const { return m_type; }
void pragma::pts::CParticleModifier::SetType(const std::string &type) { m_type = type; }
const std::string &pragma::pts::CParticleModifier::GetName() const { return m_name; }

///////////////////////

void pragma::pts::CParticleOperator::Initialize(pragma::BaseEnvParticleSystemComponent &pSystem, const std::unordered_map<std::string, std::string> &values)
{
	pragma::pts::CParticleModifier::Initialize(pSystem, values);
	for(auto &pair : values) {
		if(ustring::compare<std::string>(pair.first, "op_start_fadein"))
			m_opStartFadein = util::to_float(pair.second);
		else if(ustring::compare<std::string>(pair.first, "op_start_fadeout"))
			m_opStartFadeout = util::to_float(pair.second);
		else if(ustring::compare<std::string>(pair.first, "op_end_fadein"))
			m_opEndFadein = util::to_float(pair.second);
		else if(ustring::compare<std::string>(pair.first, "op_end_fadeout"))
			m_opEndFadeout = util::to_float(pair.second);
		else if(ustring::compare<std::string>(pair.first, "op_fade_oscillate_period"))
			m_opFadeOscillate = util::to_float(pair.second);
	}
}
float pragma::pts::CParticleOperator::CalcStrength(float curTime) const
{
	auto flTime = curTime;
	if(m_opFadeOscillate > 0.f)
		flTime = fmodf(curTime * (1.0 / m_opFadeOscillate), 1.f);
	return umath::fade_in_out(m_opStartFadein, m_opEndFadein, m_opStartFadeout, m_opEndFadeout, flTime);
}
void pragma::pts::CParticleOperator::Simulate(double) {}

void pragma::pts::CParticleOperator::PreSimulate(pragma::pts::CParticle &particle, double tDelta) {}

void pragma::pts::CParticleOperator::Simulate(pragma::pts::CParticle &pt, double dt)
{
	auto strength = CalcStrength(pt.GetTimeAlive());
	Simulate(pt, dt, strength);
}

void pragma::pts::CParticleOperator::Simulate(pragma::pts::CParticle &particle, double tDelta, float strength) {}

void pragma::pts::CParticleOperator::PostSimulate(pragma::pts::CParticle &particle, double tDelta) {}

void pragma::pts::CParticleOperatorLifespanDecay::Simulate(pragma::pts::CParticle &, double, float strength) {}

///////////////////////

void pragma::pts::CParticleRenderer::PostSimulate(double tDelta) {}

void pragma::pts::CParticleRenderer::PreRender(prosper::ICommandBuffer &cmd) {}

std::pair<Vector3, Vector3> pragma::pts::CParticleRenderer::GetRenderBounds() const { return {uvec::ORIGIN, uvec::ORIGIN}; }

///////////////////////

DLLCLIENT pragma::pts::ParticleModifierMap *g_ParticleModifierFactories = nullptr;
DLLCLIENT void pragma::pts::LinkParticleInitializerToFactory(std::string name, const pragma::pts::TParticleModifierFactory<pragma::pts::CParticleInitializer> &fc)
{
	if(g_ParticleModifierFactories == nullptr) {
		static pragma::pts::ParticleModifierMap map;
		g_ParticleModifierFactories = &map;
	}
	g_ParticleModifierFactories->AddInitializer(name, fc);
}
DLLCLIENT void pragma::pts::LinkParticleOperatorToFactory(std::string name, const pragma::pts::TParticleModifierFactory<pragma::pts::CParticleOperator> &fc)
{
	if(g_ParticleModifierFactories == nullptr) {
		static pragma::pts::ParticleModifierMap map;
		g_ParticleModifierFactories = &map;
	}
	g_ParticleModifierFactories->AddOperator(name, fc);
}
DLLCLIENT void pragma::pts::LinkParticleRendererToFactory(std::string name, const pragma::pts::TParticleModifierFactory<pragma::pts::CParticleRenderer> &fc)
{
	if(g_ParticleModifierFactories == nullptr) {
		static pragma::pts::ParticleModifierMap map;
		g_ParticleModifierFactories = &map;
	}
	g_ParticleModifierFactories->AddRenderer(name, fc);
}
DLLCLIENT pragma::pts::ParticleModifierMap *pragma::pts::GetParticleModifierMap() { return g_ParticleModifierFactories; }

void pragma::pts::ParticleModifierMap::AddInitializer(std::string name, const pragma::pts::TParticleModifierFactory<pragma::pts::CParticleInitializer> &fc)
{
	ustring::to_lower(name);
	m_initializers.insert(std::make_pair(name, pragma::pts::TParticleModifierFactory<pragma::pts::CParticleInitializer> {[fc, name](pragma::ecs::CParticleSystemComponent &c, const std::unordered_map<std::string, std::string> &keyvalues) -> std::unique_ptr<pragma::pts::CParticleInitializer, void (*)(pragma::pts::CParticleInitializer *)> {
		auto initializer = fc(c, keyvalues);
		if(initializer)
			initializer->SetType(name);
		return initializer;
	}}));
}
void pragma::pts::ParticleModifierMap::AddOperator(std::string name, const pragma::pts::TParticleModifierFactory<pragma::pts::CParticleOperator> &fc)
{
	ustring::to_lower(name);
	m_operators.insert(std::make_pair(name, pragma::pts::TParticleModifierFactory<pragma::pts::CParticleOperator> {[fc, name](pragma::ecs::CParticleSystemComponent &c, const std::unordered_map<std::string, std::string> &keyvalues) -> std::unique_ptr<pragma::pts::CParticleOperator, void (*)(pragma::pts::CParticleOperator *)> {
		auto op = fc(c, keyvalues);
		if(op)
			op->SetType(name);
		return op;
	}}));
}
void pragma::pts::ParticleModifierMap::AddRenderer(std::string name, const pragma::pts::TParticleModifierFactory<pragma::pts::CParticleRenderer> &fc)
{
	ustring::to_lower(name);
	m_renderers.insert(std::make_pair(name, pragma::pts::TParticleModifierFactory<pragma::pts::CParticleRenderer> {[fc, name](pragma::ecs::CParticleSystemComponent &c, const std::unordered_map<std::string, std::string> &keyvalues) -> std::unique_ptr<pragma::pts::CParticleRenderer, void (*)(pragma::pts::CParticleRenderer *)> {
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
