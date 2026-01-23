// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.player;

import :entities;
import :entities.components;
import :game;
import :server_state;
import pragma.shared;

using namespace pragma;

namespace pragma {
	using ::operator<<;
};

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

SPlayerComponent::SPlayerComponent(ecs::BaseEntity &ent) : BasePlayerComponent(ent), SBaseNetComponent(), m_session(nullptr), m_bAuthed(false), m_bGameReady(false) { s_players.push_back(this); }

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
	ServerState::Get()->InitResourceTransfer(*m_session);
	return r;
}

void SPlayerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SPlayerComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePlayerComponent::OnEntityComponentAdded(component);
	auto *pCharComponent = dynamic_cast<BaseCharacterComponent *>(&component);
	if(pCharComponent != nullptr) {
		auto &pFrozenProp = pCharComponent->GetFrozenProperty();
		FlagCallbackForRemoval(pCharComponent->BindEventUnhandled(sCharacterComponent::EVENT_HANDLE_VIEW_ROTATION, [this](std::reference_wrapper<ComponentEvent> evData) { OnSetViewOrientation(static_cast<CEViewRotation &>(evData.get()).rotation); }), CallbackType::Component,
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
	ent.SendNetEvent(m_netEvSetViewOrientation, p, networking::Protocol::SlowReliable, *session);
}

util::EventReply SPlayerComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BasePlayerComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == sCharacterComponent::EVENT_ON_RESPAWN)
		OnRespawn();
	return util::EventReply::Unhandled;
}

void SPlayerComponent::OnRespawn()
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->InitializePhysics(physics::PhysicsType::CapsuleController);
	auto observerC = GetEntity().GetComponent<SObserverComponent>();
	if(observerC.valid())
		observerC->SetObserverMode(ObserverMode::FirstPerson);

	ent.SendNetEvent(m_netEvRespawn, networking::Protocol::SlowReliable);
}

void SPlayerComponent::PrintMessage(std::string message, console::MESSAGE type)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	p->WriteString(message);
	p->Write<std::underlying_type_t<decltype(type)>>(math::to_integral(type));

	auto *session = GetClientSession();
	if(session)
		ent.SendNetEvent(m_netEvPrintMessage, p, networking::Protocol::FastUnreliable, {*session});
}

void SPlayerComponent::Kick(const std::string &)
{
	auto *session = GetClientSession();
	if(session != nullptr)
		ServerState::Get()->DropClient(*session, networking::DropReason::Kicked);
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
	SGame *game = ServerState::Get()->GetGameState();
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
	networking::write_angles(p, ang);
	p->Write<float>(dur);
	ent.SendNetEvent(m_netEvApplyViewRotationOffset, p, networking::Protocol::SlowReliable, *session);
}

void SPlayerComponent::InitializeGlobalNameComponent()
{
	auto globalNameComponent = GetEntity().AddComponent<GlobalNameComponent>();
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

	BindEventUnhandled(damageableComponent::EVENT_ON_TAKE_DAMAGE, [this](std::reference_wrapper<ComponentEvent> evData) { OnTakeDamage(static_cast<CEOnTakeDamage &>(evData.get()).damageInfo); });
	BindEventUnhandled(baseScoreComponent::EVENT_ON_SCORE_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { ServerState::Get()->UpdatePlayerScore(*this, static_cast<CEOnScoreChanged &>(evData.get()).score); });
	BindEventUnhandled(baseNameComponent::EVENT_ON_NAME_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { ServerState::Get()->UpdatePlayerName(*this, static_cast<CEOnNameChanged &>(evData.get()).name); });
}

void SPlayerComponent::OnSetSlopeLimit(float limit)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<float>(limit);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_SLOPELIMIT, p, networking::Protocol::SlowReliable);
	}
}
void SPlayerComponent::OnSetStepOffset(float offset)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<float>(offset);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_STEPOFFSET, p, networking::Protocol::SlowReliable);
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
		networking::write_unique_entity(packet, nullptr);
	else
		networking::write_unique_entity(packet, m_entFlashlight.get());
}

bool SPlayerComponent::IsAuthed() { return m_bAuthed; }
void SPlayerComponent::SetAuthed(bool b) { m_bAuthed = b; }

void SPlayerComponent::SetWalkSpeed(float speed)
{
	BasePlayerComponent::SetWalkSpeed(speed);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<float>(speed);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_SPEED_WALK, p, networking::Protocol::SlowReliable);
	}
}

void SPlayerComponent::SetRunSpeed(float speed)
{
	BasePlayerComponent::SetRunSpeed(speed);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<float>(speed);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_SPEED_RUN, p, networking::Protocol::SlowReliable);
	}
}

void SPlayerComponent::SetCrouchedWalkSpeed(float speed)
{
	BasePlayerComponent::SetCrouchedWalkSpeed(speed);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<float>(speed);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_SPEED_CROUCH_WALK, p, networking::Protocol::SlowReliable);
	}
}

void SPlayerComponent::SetStandHeight(float height)
{
	BasePlayerComponent::SetStandHeight(height);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<float>(height);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_HEIGHT_STAND, p, networking::Protocol::SlowReliable);
	}
}
void SPlayerComponent::SetCrouchHeight(float height)
{
	BasePlayerComponent::SetCrouchHeight(height);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<float>(height);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_HEIGHT_CROUCH, p, networking::Protocol::SlowReliable);
	}
}
void SPlayerComponent::SetStandEyeLevel(float eyelevel)
{
	BasePlayerComponent::SetStandEyeLevel(eyelevel);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<float>(eyelevel);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_EYELEVEL_STAND, p, networking::Protocol::SlowReliable);
	}
}
void SPlayerComponent::SetCrouchEyeLevel(float eyelevel)
{
	BasePlayerComponent::SetCrouchEyeLevel(eyelevel);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<float>(eyelevel);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_EYELEVEL_CROUCH, p, networking::Protocol::SlowReliable);
	}
}

bool SPlayerComponent::PlaySharedActivity(Activity activity)
{
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	return animComponent.valid() ? animComponent->PlayActivity(activity, FPlayAnim::None) : false;
}

void SPlayerComponent::SetSprintSpeed(float speed)
{
	BasePlayerComponent::SetSprintSpeed(speed);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<float>(speed);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_SPEED_SPRINT, p, networking::Protocol::SlowReliable);
	}
}
void SPlayerComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BasePlayerComponent)); }
void SPlayerComponent::OnTakeDamage(game::DamageInfo &info)
{
	auto &ent = GetEntity();
	auto *charComponent = static_cast<SCharacterComponent *>(ent.GetCharacterComponent().get());
	if(charComponent != nullptr && charComponent->GetGodMode() == true)
		info.SetDamage(0);
}

namespace Lua {
	namespace Player {
		namespace Server {
			static void Respawn(lua::State *l, SPlayerComponent &hEnt);
			static void SetActionInput(lua::State *l, SPlayerComponent &hPl, UInt32 input, Bool pressed);
			static bool SendResource(lua::State *l, SPlayerComponent &hPl, const std::string &name);
		};
	};
};

void SPlayerComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BasePlayerComponent::RegisterLuaBindings(l, modEnts);

	auto def = pragma::LuaCore::create_entity_component_class<SPlayerComponent, BasePlayerComponent>("PlayerComponent");
	def.def("Respawn", &Lua::Player::Server::Respawn);
	def.def("SetActionInput", &Lua::Player::Server::SetActionInput);
	def.def("Kick", &SPlayerComponent::Kick);
	def.def("SendResource", &Lua::Player::Server::SendResource);
	modEnts[def];
}

void Lua::Player::Server::Respawn(lua::State *l, SPlayerComponent &hEnt)
{
	auto charComponent = hEnt.GetEntity().GetCharacterComponent();
	if(charComponent.valid())
		charComponent->Respawn();
}

void Lua::Player::Server::SetActionInput(lua::State *l, SPlayerComponent &hPl, UInt32 input, Bool pressed)
{
	auto *actionInputC = hPl.GetActionInputController();
	if(actionInputC)
		actionInputC->SetActionInput(static_cast<Action>(input), pressed);
}

bool Lua::Player::Server::SendResource(lua::State *l, SPlayerComponent &hPl, const std::string &name) { return hPl.SendResource(name); }
