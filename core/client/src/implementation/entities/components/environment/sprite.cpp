// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.effects.sprite;
import :entities.components.attachment;
import :entities.components.color;
import :client_state;
import :engine;

using namespace pragma;

void CSpriteComponent::Initialize() { BaseEnvSpriteComponent::Initialize(); }

CSpriteComponent::~CSpriteComponent()
{
	StopParticle();

	if(m_hCbRenderCallback.IsValid())
		m_hCbRenderCallback.Remove();
}

void CSpriteComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	StartParticle();
}

void CSpriteComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEnvSpriteComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CColorComponent)) {
		FlagCallbackForRemoval(static_cast<CColorComponent *>(&component)->GetColorProperty()->AddCallback([this](std::reference_wrapper<const Vector4> oldColor, std::reference_wrapper<const Vector4> color) { UpdateColor(); }), CallbackType::Component, &component);
	}
}

void CSpriteComponent::UpdateColor()
{
	auto colorC = GetEntity().GetComponent<CColorComponent>();
	auto colorFactor = colorC.valid() ? colorC->GetColor() : colors::White.ToVector4();
	if(m_hParticle.IsValid())
		m_hParticle->SetColorFactor(colorFactor);
}

void CSpriteComponent::SetOrientationType(pts::ParticleOrientationType orientationType) { m_orientationType = orientationType; }

util::EventReply CSpriteComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEnvSpriteComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseToggleComponent::EVENT_ON_TURN_ON)
		StartParticle();
	else if(eventId == baseToggleComponent::EVENT_ON_TURN_OFF)
		StopParticle();
	return util::EventReply::Unhandled;
}

void CSpriteComponent::StopParticle()
{
	if(m_hParticle.expired())
		return;
	// m_hParticle->Stop(); // Do we need this?
	m_hParticle->Die(m_tFadeOut > 0.f ? m_tFadeOut : 0.f);
	m_hParticle->SetRemoveOnComplete(true);
}

void CSpriteComponent::OnTick(double dt)
{
	if(m_hParticle.expired())
		GetEntity().RemoveSafely();
}

void CSpriteComponent::StopAndRemoveEntity()
{
	auto &ent = GetEntity();
	StopParticle();
	if(m_hParticle.expired()) {
		ent.RemoveSafely();
		return;
	}
	SetTickPolicy(TickPolicy::Always);
}

void CSpriteComponent::StartParticle()
{
	auto &ent = GetEntity();
	StopParticle();
	auto startAlpha = (m_tFadeIn == 0.f) ? m_color.a : 0;
	std::unordered_map<std::string, std::string> values = {{"maxparticles", "1"}, {"emission_rate", std::to_string(std::numeric_limits<uint16_t>::max())}, {"material", m_spritePath}, {"sort_particles", "0"}, {"orientation_type", std::to_string(math::to_integral(m_orientationType))},
	  {"cast_shadows", "0"}, {"static_scale", std::to_string(m_size)}, {"color", std::to_string(m_color.r) + " " + std::to_string(m_color.g) + " " + std::to_string(m_color.b) + " " + std::to_string(startAlpha)}, {"bloom_scale", std::to_string(m_bloomScale)}, {"move_with_emitter", "1"}};
	auto spawnFlags = ent.GetSpawnFlags();
	if(spawnFlags & math::to_integral(SpawnFlags::BlackToAlpha))
		values.insert(std::make_pair("black_to_alpha", "1"));
	if(spawnFlags & math::to_integral(SpawnFlags::NoSoftParticles))
		values.insert(std::make_pair("soft_particles", "0"));
	auto *pt = ecs::CParticleSystemComponent::Create(values);
	if(pt == nullptr)
		return;
	pt->AddInitializer("radius_random", std::unordered_map<std::string, std::string> {{"radius_min", std::to_string(m_size)}, {"radius_max", std::to_string(m_size)}});
	pt->AddInitializer("lifetime_random", std::unordered_map<std::string, std::string> {{"lifetime_min", std::to_string(std::numeric_limits<float>::max())}, {"lifetime_max", std::to_string(std::numeric_limits<float>::max())}});
	if(m_tFadeIn > 0.f) {
		pt->AddOperator("color_fade", std::unordered_map<std::string, std::string> {{"alpha", std::to_string(m_color.a)}, {"fade_start", "0"}, {"fade_end", std::to_string(m_tFadeIn)}});
	}
	auto tFadeOut = (m_tFadeOut > 0.f) ? m_tFadeOut : 1.f;
	if(tFadeOut > 0.f) {
		pt->AddOperator("color_fade",
		  std::unordered_map<std::string, std::string> {
		    {"alpha", "0"},
		    {"fade_start", std::to_string(-tFadeOut)},
		    {"fade_end", "-0.0001"},
		  });
	}
	pt->AddRenderer("sprite", std::unordered_map<std::string, std::string> {});

	if(m_hCbRenderCallback.IsValid())
		m_hCbRenderCallback.Remove();

	auto whAttComponent = pt->GetEntity().AddComponent<CAttachmentComponent>();
	if(whAttComponent.valid()) {
		AttachmentInfo attInfo {};
		attInfo.flags = FAttachmentMode::SnapToOrigin | FAttachmentMode::UpdateEachFrame;
		whAttComponent->AttachToEntity(&ent, attInfo);
	}
	// We need to update the particle system position every frame
	m_hCbRenderCallback = pt->AddRenderCallback([this]() {
		auto &ent = GetEntity();
		auto pAttComponent = ent.GetComponent<CAttachmentComponent>();
		if(pAttComponent.valid())
			pAttComponent->UpdateAttachmentOffset();
	});

	if(m_particleRenderMode != std::numeric_limits<uint32_t>::max())
		pt->SetSceneRenderPass(static_cast<rendering::SceneRenderPass>(m_particleRenderMode));
	pt->SetContinuous(true);
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentPt = pt->GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr && pTrComponentPt) {
		pTrComponentPt->SetPosition(pTrComponent->GetPosition());
		pTrComponentPt->SetRotation(pTrComponent->GetRotation());
	}
	pt->Start();
	m_hParticle = pt->GetHandle<ecs::CParticleSystemComponent>();

	UpdateColor();
}

void CSpriteComponent::ReceiveData(NetPacket &packet)
{
	m_spritePath = packet->ReadString();
	m_size = packet->Read<float>();
	m_bloomScale = packet->Read<float>();
	m_color = packet->Read<Color>();
	m_particleRenderMode = packet->Read<uint32_t>();
	m_tFadeIn = packet->Read<float>();
	m_tFadeOut = packet->Read<float>();
}
void CSpriteComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////

void CEnvSprite::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSpriteComponent>();
}
