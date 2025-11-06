// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.server;
import :entities.components.triggers.gravity;

import :entities;
import :server_state;

using namespace pragma;

void STriggerGravityComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "gravity_dir", false)) {
			EulerAngles ang(kvData.value);
			m_kvGravityDir = ang.Forward();
		}
		else if(ustring::compare<std::string>(kvData.key, "gravity_force", false))
			m_kvGravityForce = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "use_force", false))
			m_kvUseForce = util::to_boolean(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
}

void STriggerGravityComponent::OnResetGravity(pragma::ecs::BaseEntity *ent, GravitySettings &settings)
{
	BaseEntityTriggerGravityComponent::OnResetGravity(ent, settings);
	auto &entThis = GetEntity();
	NetPacket p {};
	nwm::write_entity(p, ent);
	p->Write<uint8_t>(umath::to_integral(pragma::Entity::TriggerGravity::NetFlags::None));
	p->Write<uint32_t>(entThis.GetSpawnFlags());
	p->Write<Vector3>((settings.dir != nullptr) ? *settings.dir : Vector3 {});
	p->Write<float>((settings.force != nullptr) ? *settings.force : 0.f);
	p->Write<Vector3>((settings.dirMove != nullptr) ? *settings.dirMove : Vector3 {});
	ServerState::Get()->SendPacket("ent_trigger_gravity_onstarttouch", p, pragma::networking::Protocol::SlowReliable);
}

void STriggerGravityComponent::OnStartTouch(pragma::ecs::BaseEntity *ent)
{
	BaseEntityTriggerGravityComponent::OnStartTouch(ent);
	auto netFlags = pragma::Entity::TriggerGravity::NetFlags::StartTouch;
	if(m_kvUseForce == true)
		netFlags |= pragma::Entity::TriggerGravity::NetFlags::UseForce;
	auto &entThis = GetEntity();
	NetPacket p {};
	nwm::write_entity(p, ent);
	p->Write<uint8_t>(umath::to_integral(netFlags));
	p->Write<uint32_t>(entThis.GetSpawnFlags());
	p->Write<Vector3>(m_kvGravityDir);
	p->Write<float>(m_kvGravityForce);
	ServerState::Get()->SendPacket("ent_trigger_gravity_onstarttouch", p, pragma::networking::Protocol::SlowReliable);
}

////////////

void STriggerGravityComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerGravity::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STriggerGravityComponent>();
}
