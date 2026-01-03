// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.effects.explosion;
import :client_state;
import :engine;
import :entities.components.attachment;
import :entities.components.env_quake;
import :entities.components.io;
import :entities.components.particle_system;
import :entities.components.sound_emitter;
import :game;

using namespace pragma;

void CExplosionComponent::Initialize()
{
	BaseEnvExplosionComponent::Initialize();
	ecs::CParticleSystemComponent::Precache("explosion");
	get_client_state()->LoadSoundScripts("fx.udm");
}

void CExplosionComponent::Explode()
{
#pragma message("TODO: Underwater effects +sounds")
#pragma message("TODO: Apply damage to all ents within range -> Serverside")
#pragma message("TODO: Leave a scorch mark!")
	auto &ent = GetEntity();
	auto *particle = ecs::CParticleSystemComponent::Create("explosion");
	if(particle != nullptr) {
		auto pTrComponent = ent.GetTransformComponent();
		auto pTrComponentPt = particle->GetEntity().GetTransformComponent();
		if(pTrComponent != nullptr && pTrComponentPt)
			pTrComponentPt->SetPosition(pTrComponent->GetPosition());
		particle->SetRemoveOnComplete(true);
		if(particle != nullptr)
			particle->Start();
	}
	auto pSoundEmitterComponent = ent.GetComponent<CSoundEmitterComponent>();
	if(pSoundEmitterComponent.valid())
		pSoundEmitterComponent->EmitSound("fx.explosion", audio::ALSoundType::Effect, 1.f);
	auto radius = 500.f;
	auto *entQuake = get_cgame()->CreateEntity<CEnvQuake>();
	if(entQuake != nullptr) {
		auto *pQuakeComponent = static_cast<CQuakeComponent *>(entQuake->FindComponent("quake").get());
		if(pQuakeComponent != nullptr) {
			pQuakeComponent->SetFrequency(50.f);
			pQuakeComponent->SetAmplitude(50.f);
			pQuakeComponent->SetRadius(radius);
		}
		auto pAttComponent = entQuake->AddComponent<CAttachmentComponent>();
		if(pAttComponent.valid()) {
			AttachmentInfo attInfo {};
			attInfo.flags |= FAttachmentMode::SnapToOrigin | FAttachmentMode::PositionOnly;
			pAttComponent->AttachToEntity(&ent, attInfo);
		}
		entQuake->SetKeyValue("spawnflags", std::to_string(SF_QUAKE_IN_AIR | SF_QUAKE_REMOVE_ON_COMPLETE));
		entQuake->Spawn();
		auto pIoComponent = entQuake->GetComponent<CIOComponent>();
		if(pIoComponent.valid())
			pIoComponent->Input("StartShake");
	}
}
void CExplosionComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////////

void CEnvExplosion::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CExplosionComponent>();
}
