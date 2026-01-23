// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.effects.explosion;

import :server_state;

using namespace pragma;

void SExplosionComponent::Explode()
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		ServerState::Get()->SendPacket(networking::net_messages::client::ENVEXPLOSION_EXPLODE, p, networking::Protocol::SlowReliable);
	}
	BaseEnvExplosionComponent::Explode();
}

void SExplosionComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvExplosion::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SExplosionComponent>();
}
