/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/physics/collisiontypes.h"
#include "pragma/entities/s_flashlight.h"
#include "pragma/entities/player.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/entities/components/s_health_component.hpp"
#include "pragma/entities/components/s_observer_component.hpp"
#include "pragma/entities/components/s_observable_component.hpp"
#include "pragma/networking/s_nwm_util.h"
#include "pragma/networking/iserver_client.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/networking/enums.hpp>
#include <pragma/networking/error.hpp>
#include <pragma/util/util_handled.hpp>
#include <pragma/model/model.h>
#include <pragma/entities/components/base_animated_component.hpp>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_observable_component.hpp>
#include <pragma/entities/components/base_score_component.hpp>
#include <pragma/entities/components/base_name_component.hpp>
#include <pragma/entities/components/base_model_component.hpp>
#include <pragma/entities/components/global_component.hpp>
#include <pragma/entities/components/damageable_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

namespace pragma {
	using ::operator<<;
};

extern ServerState *server;

std::vector<SPlayerComponent *> SPlayerComponent::s_players;
const std::vector<SPlayerComponent *> &SPlayerComponent::GetAll() { return s_players; }
unsigned int SPlayerComponent::GetPlayerCount() { return CUInt32(s_players.size()); }

Con::c_cout &SPlayerComponent::print(Con::c_cout &os)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto nameC = ent.GetNameComponent();
	os << "Player[" << (nameC.valid() ? nameC->GetName() : "") << "][" << ent.GetIndex() << "]"
	   << "[" << ent.GetClass() << "]"
	   << "[";
	auto mdlComponent = ent.GetModelComponent();
	if(!mdlComponent || mdlComponent->GetModel() == nullptr)
		os << "NULL";
	else
		os << mdlComponent->GetModel()->GetName();
	os << "]";
	return os;
}

std::ostream &SPlayerComponent::print(std::ostream &os)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto nameC = ent.GetNameComponent();
	os << "Player[" << (nameC.valid() ? nameC->GetName() : "") << "][" << ent.GetIndex() << "]"
	   << "[" << ent.GetClass() << "]"
	   << "[";
	auto mdlComponent = ent.GetModelComponent();
	if(!mdlComponent || mdlComponent->GetModel() == nullptr)
		os << "NULL";
	else
		os << mdlComponent->GetModel()->GetName();
	os << "]";
	return os;
}

SPlayerComponent::SPlayerComponent(BaseEntity &ent) : BasePlayerComponent(ent), SBaseNetComponent(), m_session(NULL), m_bAuthed(false), m_bGameReady(false) { s_players.push_back(this); }

SPlayerComponent::~SPlayerComponent()
{
	auto it = std::find(s_players.begin(), s_players.end(), this);
	if(it != s_players.end())
		s_players.erase(it);
}

bool SPlayerComponent::SendResource(const std::string &fileName) const
{
	if(m_session.expired())
		return false;
	auto r = m_session->AddResource(fileName);
	server->InitResourceTransfer(*m_session);
	return r;
}

void SPlayerComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SPlayerComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePlayerComponent::OnEntityComponentAdded(component);
	auto *pCharComponent = dynamic_cast<BaseCharacterComponent *>(&component);
	if(pCharComponent != nullptr) {
		auto &pFrozenProp = pCharComponent->GetFrozenProperty();
		FlagCallbackForRemoval(pCharComponent->BindEventUnhandled(SCharacterComponent::EVENT_HANDLE_VIEW_ROTATION, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnSetViewOrientation(static_cast<CEViewRotation &>(evData.get()).rotation); }), CallbackType::Component,
		  pCharComponent);
		auto &pSlopeLimitProp = pCharComponent->GetSlopeLimitProperty();
		FlagCallbackForRemoval(pSlopeLimitProp->AddCallback([this](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> newVal) { OnSetSlopeLimit(newVal); }), CallbackType::Component, pCharComponent);
		auto &pStepOffsetProp = pCharComponent->GetStepOffsetProperty();
		FlagCallbackForRemoval(pStepOffsetProp->AddCallback([this](std::reference_wrapper<const float> oldVal, std::reference_wrapper<const float> newVal) { OnSetStepOffset(newVal); }), CallbackType::Component, pCharComponent);
	}
	if(typeid(component) == typeid(SObservableComponent))
		m_observableComponent = &static_cast<SObservableComponent &>(component);
}

void SPlayerComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BasePlayerComponent::OnEntityComponentRemoved(component);
	if(typeid(component) == typeid(SObservableComponent))
		m_observableComponent = nullptr;
}

void SPlayerComponent::SetViewRotation(const Quat &rot)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto charC = ent.GetCharacterComponent();
	if(charC.valid())
		charC->SetViewOrientation(rot);
	if(ent.IsShared() == false)
		return;
	auto *session = GetClientSession();
	if(session == nullptr)
		return;
	NetPacket p;
	p->Write<Quat>(rot);
	ent.SendNetEvent(m_netEvSetViewOrientation, p, pragma::networking::Protocol::SlowReliable, *session);
}

util::EventReply SPlayerComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BasePlayerComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseCharacterComponent::EVENT_ON_RESPAWN)
		OnRespawn();
	return util::EventReply::Unhandled;
}

void SPlayerComponent::OnRespawn()
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->InitializePhysics(PHYSICSTYPE::CAPSULECONTROLLER);
	auto observerC = GetEntity().GetComponent<SObserverComponent>();
	if(observerC.valid())
		observerC->SetObserverMode(ObserverMode::FirstPerson);

	ent.SendNetEvent(m_netEvRespawn, pragma::networking::Protocol::SlowReliable);
}

void SPlayerComponent::PrintMessage(std::string message, MESSAGE type)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	p->WriteString(message);
	p->Write<std::underlying_type_t<decltype(type)>>(umath::to_integral(type));

	auto *session = GetClientSession();
	if(session)
		ent.SendNetEvent(m_netEvPrintMessage, p, pragma::networking::Protocol::FastUnreliable, {*session});
}

void SPlayerComponent::Kick(const std::string &)
{
	auto *session = GetClientSession();
	if(session != nullptr)
		server->DropClient(*session, pragma::networking::DropReason::Kicked);
	else {
		auto &ent = static_cast<SBaseEntity &>(GetEntity());
		ent.RemoveSafely();
	}
}

bool SPlayerComponent::IsGameReady() const { return m_bGameReady; }
void SPlayerComponent::SetGameReady(bool b) { m_bGameReady = b; }

void SPlayerComponent::OnEntitySpawn()
{
	BasePlayerComponent::OnEntitySpawn();
	InitializeFlashlight();

	InitializeGlobalNameComponent(); // Initialize global name component for level transitions
}

void SPlayerComponent::InitializeFlashlight()
{
	SGame *game = server->GetGameState();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	auto *light = game->CreateEntity<Flashlight>();
	if(light == nullptr)
		return;
	auto pTrComponentLight = light->GetTransformComponent();
	if(pTrComponentLight) {
		pTrComponentLight->SetPosition(pTrComponent->GetPosition());
		pTrComponentLight->SetRotation(charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetRotation());
	}
	light->Spawn();
	m_entFlashlight = light->GetHandle();
}

std::vector<InputAction> &SPlayerComponent::GetKeyStack() { return m_keyStack; }
void SPlayerComponent::ClearKeyStack() { m_keyStack.clear(); }

void SPlayerComponent::ApplyViewRotationOffset(const EulerAngles &ang, float dur)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	auto *session = GetClientSession();
	if(session == nullptr)
		return;

	NetPacket p;
	nwm::write_angles(p, ang);
	p->Write<float>(dur);
	ent.SendNetEvent(m_netEvApplyViewRotationOffset, p, pragma::networking::Protocol::SlowReliable, *session);
}

void SPlayerComponent::InitializeGlobalNameComponent()
{
	auto globalNameComponent = GetEntity().AddComponent<pragma::GlobalNameComponent>();
	auto address = GetClientIPAddress();
	globalNameComponent->SetGlobalName(address.ToString());
}

networking::IPAddress SPlayerComponent::GetClientIPAddress() const
{
	if(m_session.expired())
		return {};
	auto ipAddress = m_session->GetIPAddress();
	return ipAddress.has_value() ? *ipAddress : networking::IPAddress {};
}

std::string SPlayerComponent::GetClientIP()
{
	if(m_session.expired())
		return BasePlayerComponent::GetClientIP();
	auto ip = m_session->GetIP();
	return ip.has_value() ? *ip : BasePlayerComponent::GetClientIP();
}

unsigned short SPlayerComponent::GetClientPort()
{
	if(m_session.expired())
		return BasePlayerComponent::GetClientPort();
	auto port = m_session->GetPort();
	return port.has_value() ? *port : BasePlayerComponent::GetClientPort();
}

void SPlayerComponent::Initialize()
{
	BasePlayerComponent::Initialize();

	BindEventUnhandled(DamageableComponent::EVENT_ON_TAKE_DAMAGE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnTakeDamage(static_cast<CEOnTakeDamage &>(evData.get()).damageInfo); });
	BindEventUnhandled(BaseScoreComponent::EVENT_ON_SCORE_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { server->UpdatePlayerScore(*this, static_cast<CEOnScoreChanged &>(evData.get()).score); });
	BindEventUnhandled(BaseNameComponent::EVENT_ON_NAME_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { server->UpdatePlayerName(*this, static_cast<CEOnNameChanged &>(evData.get()).name); });
}

void SPlayerComponent::OnSetSlopeLimit(float limit)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<float>(limit);
		server->SendPacket("pl_slopelimit", p, pragma::networking::Protocol::SlowReliable);
	}
}
void SPlayerComponent::OnSetStepOffset(float offset)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<float>(offset);
		server->SendPacket("pl_stepoffset", p, pragma::networking::Protocol::SlowReliable);
	}
}

void SPlayerComponent::SetClientSession(networking::IServerClient &session) { m_session = session.shared_from_this(); }

void SPlayerComponent::UpdateViewOrientation(const Quat &rot)
{
	auto charComponent = GetEntity().GetCharacterComponent();
	if(charComponent.valid())
		charComponent->SetViewOrientation(rot);
}

void SPlayerComponent::OnSetViewOrientation(const Quat &orientation)
{
	/*// Obsolete: View rotation is sent via snapshot	
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	auto *session = GetClientSession();
	if(session == nullptr)
		return;
	NetPacket p;
	p->Write<Quat>(orientation);
	ent.SendNetEvent(m_netEvSetViewOrientation,p,pragma::networking::Protocol::FastUnreliable,*session);*/
}

networking::IServerClient *SPlayerComponent::GetClientSession() { return m_session.get(); }

void SPlayerComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<double>(ConnectionTime());
	if(m_entFlashlight.expired())
		nwm::write_unique_entity(packet, nullptr);
	else
		nwm::write_unique_entity(packet, m_entFlashlight.get());
}

bool SPlayerComponent::IsAuthed() { return m_bAuthed; }
void SPlayerComponent::SetAuthed(bool b) { m_bAuthed = b; }

void SPlayerComponent::SetWalkSpeed(float speed)
{
	BasePlayerComponent::SetWalkSpeed(speed);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<float>(speed);
		server->SendPacket("pl_speed_walk", p, pragma::networking::Protocol::SlowReliable);
	}
}

void SPlayerComponent::SetRunSpeed(float speed)
{
	BasePlayerComponent::SetRunSpeed(speed);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<float>(speed);
		server->SendPacket("pl_speed_run", p, pragma::networking::Protocol::SlowReliable);
	}
}

void SPlayerComponent::SetCrouchedWalkSpeed(float speed)
{
	BasePlayerComponent::SetCrouchedWalkSpeed(speed);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<float>(speed);
		server->SendPacket("pl_speed_crouch_walk", p, pragma::networking::Protocol::SlowReliable);
	}
}

void SPlayerComponent::SetStandHeight(float height)
{
	BasePlayerComponent::SetStandHeight(height);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<float>(height);
		server->SendPacket("pl_height_stand", p, pragma::networking::Protocol::SlowReliable);
	}
}
void SPlayerComponent::SetCrouchHeight(float height)
{
	BasePlayerComponent::SetCrouchHeight(height);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<float>(height);
		server->SendPacket("pl_height_crouch", p, pragma::networking::Protocol::SlowReliable);
	}
}
void SPlayerComponent::SetStandEyeLevel(float eyelevel)
{
	BasePlayerComponent::SetStandEyeLevel(eyelevel);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<float>(eyelevel);
		server->SendPacket("pl_eyelevel_stand", p, pragma::networking::Protocol::SlowReliable);
	}
}
void SPlayerComponent::SetCrouchEyeLevel(float eyelevel)
{
	BasePlayerComponent::SetCrouchEyeLevel(eyelevel);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<float>(eyelevel);
		server->SendPacket("pl_eyelevel_crouch", p, pragma::networking::Protocol::SlowReliable);
	}
}

bool SPlayerComponent::PlaySharedActivity(Activity activity)
{
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	return animComponent.valid() ? animComponent->PlayActivity(activity, pragma::FPlayAnim::None) : false;
}

void SPlayerComponent::SetSprintSpeed(float speed)
{
	BasePlayerComponent::SetSprintSpeed(speed);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<float>(speed);
		server->SendPacket("pl_speed_sprint", p, pragma::networking::Protocol::SlowReliable);
	}
}
void SPlayerComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BasePlayerComponent)); }
void SPlayerComponent::OnTakeDamage(DamageInfo &info)
{
	auto &ent = GetEntity();
	auto *charComponent = static_cast<pragma::SCharacterComponent *>(ent.GetCharacterComponent().get());
	if(charComponent != nullptr && charComponent->GetGodMode() == true)
		info.SetDamage(0);
}
