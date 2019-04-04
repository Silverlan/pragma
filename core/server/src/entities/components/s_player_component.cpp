#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/networking/wvserverclient.h"
#include "pragma/physics/collisiontypes.h"
#include "pragma/entities/s_flashlight.h"
#include "pragma/entities/player.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/entities/components/s_health_component.hpp"
#include "pragma/networking/s_nwm_util.h"
#include <pragma/util/util_handled.hpp>
#include <pragma/model/model.h>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_observable_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

namespace pragma
{
	using ::operator<<;
};

extern ServerState *server;

std::vector<SPlayerComponent*> SPlayerComponent::s_players;
const std::vector<SPlayerComponent*> &SPlayerComponent::GetAll() {return s_players;}
unsigned int SPlayerComponent::GetPlayerCount() {return CUInt32(s_players.size());}

Con::c_cout& SPlayerComponent::print(Con::c_cout &os)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	os<<"Player["<<GetPlayerName()<<"]["<<ent.GetIndex()<<"]"<<"["<<ent.GetClass()<<"]"<<"[";
	auto mdlComponent = ent.GetModelComponent();
	if(mdlComponent.expired() || mdlComponent->GetModel() == nullptr)
		os<<"NULL";
	else
		os<<mdlComponent->GetModel()->GetName();
	os<<"]";
	return os;
}

std::ostream& SPlayerComponent::print(std::ostream &os)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	os<<"Player["<<GetPlayerName()<<"]["<<ent.GetIndex()<<"]"<<"["<<ent.GetClass()<<"]"<<"[";
	auto mdlComponent = ent.GetModelComponent();
	if(mdlComponent.expired() || mdlComponent->GetModel() == nullptr)
		os<<"NULL";
	else
		os<<mdlComponent->GetModel()->GetName();
	os<<"]";
	return os;
}

SPlayerComponent::SPlayerComponent(BaseEntity &ent)
	: BasePlayerComponent(ent),SBaseNetComponent(),
	m_session(NULL),m_bAuthed(false),
	m_bGameReady(false)
{
	s_players.push_back(this);
}

SPlayerComponent::~SPlayerComponent()
{
	auto it = std::find(s_players.begin(),s_players.end(),this);
	if(it != s_players.end())
		s_players.erase(it);
}

bool SPlayerComponent::SendResource(const std::string &fileName) const
{
	if(m_session == nullptr)
		return false;
	auto *info = m_session->GetSessionInfo();
	if(info == nullptr)
		return false;
	auto r = info->AddResource(fileName);
	server->InitResourceTransfer(m_session);
	return r;
}

luabind::object SPlayerComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SPlayerComponentHandleWrapper>(l);}

void SPlayerComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePlayerComponent::OnEntityComponentAdded(component);
	auto *pCharComponent = dynamic_cast<BaseCharacterComponent*>(&component);
	if(pCharComponent != nullptr)
	{
		auto &pFrozenProp = pCharComponent->GetFrozenProperty();
		FlagCallbackForRemoval(pCharComponent->BindEventUnhandled(SCharacterComponent::EVENT_HANDLE_VIEW_ROTATION,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
			OnSetViewOrientation(static_cast<CEViewRotation&>(evData.get()).rotation);
		}),CallbackType::Component,pCharComponent);
		auto &pSlopeLimitProp = pCharComponent->GetSlopeLimitProperty();
		FlagCallbackForRemoval(pSlopeLimitProp->AddCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
			OnSetSlopeLimit(newVal);
		}),CallbackType::Component,pCharComponent);
		auto &pStepOffsetProp = pCharComponent->GetStepOffsetProperty();
		FlagCallbackForRemoval(pStepOffsetProp->AddCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
			OnSetStepOffset(newVal);
		}),CallbackType::Component,pCharComponent);
	}
}

util::EventReply SPlayerComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BasePlayerComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseCharacterComponent::EVENT_ON_RESPAWN)
		OnRespawn();
	return util::EventReply::Unhandled;
}

void SPlayerComponent::OnRespawn()
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->InitializePhysics(PHYSICSTYPE::CAPSULECONTROLLER);
	SetObserverMode(OBSERVERMODE::FIRSTPERSON);

	ent.SendNetEventTCP(m_netEvRespawn);
}

void SPlayerComponent::PrintMessage(std::string message,MESSAGE type)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	p->WriteString(message);
	p->Write<std::underlying_type_t<decltype(type)>>(umath::to_integral(type));

	nwm::RecipientFilter rp {};
	rp.Add(GetClientSession());
	ent.SendNetEventUDP(m_netEvPrintMessage,p,rp);
}

void SPlayerComponent::Kick(const std::string&)
{
	auto *session = GetClientSession();
	if(session != nullptr)
		session->Drop(nwm::ClientDropped::Kicked); // Player will be removed automatically
	else
	{
		auto &ent = static_cast<SBaseEntity&>(GetEntity());
		ent.RemoveSafely();
	}
}

bool SPlayerComponent::IsGameReady() const {return m_bGameReady;}
void SPlayerComponent::SetGameReady(bool b) {m_bGameReady = b;}

void SPlayerComponent::SetObserverMode(OBSERVERMODE mode)
{
	BasePlayerComponent::SetObserverMode(mode);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<UChar>(CUChar(mode));

		server->SendPacketTCP("pl_observermode",p,GetClientSession());
	}
}

void SPlayerComponent::SetObserverTarget(BaseObservableComponent *ent)
{
	BasePlayerComponent::SetObserverTarget(ent);
	auto &entThis = static_cast<SBaseEntity&>(GetEntity());
	if(entThis.IsShared() == false)
		return;
	NetPacket p {};
	nwm::write_entity(p,&ent->GetEntity());

	nwm::RecipientFilter rp {};
	rp.Add(GetClientSession());
	entThis.SendNetEventTCP(m_netEvSetObserverTarget,p,rp);
}
void SPlayerComponent::SetObserverCameraOffset(const Vector3 &offset)
{
	BasePlayerComponent::SetObserverCameraOffset(offset);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	p->Write<Vector3>(offset);

	nwm::RecipientFilter rp {};
	rp.Add(GetClientSession());
	ent.SendNetEventTCP(m_netEvSetObserverCameraOffset,p,rp);
}
void SPlayerComponent::SetObserverCameraLocked(bool b)
{
	BasePlayerComponent::SetObserverCameraLocked(b);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p {};
	p->Write<bool>(b);

	nwm::RecipientFilter rp {};
	rp.Add(GetClientSession());
	ent.SendNetEventTCP(m_netEvSetObserverCameraLocked,p,rp);
}

void SPlayerComponent::OnEntitySpawn()
{
	BasePlayerComponent::OnEntitySpawn();
	InitializeFlashlight();
	
	InitializeGlobalNameComponent(); // Initialize global name component for level transitions
}

void SPlayerComponent::InitializeFlashlight()
{
	SGame *game = server->GetGameState();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.expired())
		return;
	auto *light = game->CreateEntity<Flashlight>();
	if(light == nullptr)
		return;
	auto pTrComponentLight = light->GetTransformComponent();
	if(pTrComponentLight.valid())
	{
		pTrComponentLight->SetPosition(pTrComponent->GetPosition());
		pTrComponentLight->SetOrientation(charComponent.valid() ? charComponent->GetViewOrientation() : pTrComponent->GetOrientation());
	}
	light->Spawn();
	m_entFlashlight = light->GetHandle();
}

std::vector<InputAction> &SPlayerComponent::GetKeyStack() {return m_keyStack;}
void SPlayerComponent::ClearKeyStack() {m_keyStack.clear();}

void SPlayerComponent::ApplyViewRotationOffset(const EulerAngles &ang,float dur)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	nwm::RecipientFilter rp {};
	rp.Add(GetClientSession());

	NetPacket p;
	nwm::write_angles(p,ang);
	p->Write<float>(dur);
	ent.SendNetEventTCP(m_netEvApplyViewRotationOffset,p,rp);
}

void SPlayerComponent::InitializeGlobalNameComponent()
{
	auto globalNameComponent = GetEntity().AddComponent<pragma::GlobalNameComponent>();
	auto address = GetClientIPAddress();
	globalNameComponent->SetGlobalName(address.ToString());
}

nwm::IPAddress SPlayerComponent::GetClientIPAddress() const
{
	if(m_session == nullptr)
		return {};
	return m_session->GetAddress();
}

std::string SPlayerComponent::GetClientIP()
{
	if(m_session == nullptr)
		return BasePlayerComponent::GetClientIP();
	return m_session->GetIP();
}

unsigned short SPlayerComponent::GetClientPort()
{
	if(m_session == NULL)
		return BasePlayerComponent::GetClientPort();
	return m_session->GetPort();
}

void SPlayerComponent::Initialize()
{
	BasePlayerComponent::Initialize();

	BindEventUnhandled(DamageableComponent::EVENT_ON_TAKE_DAMAGE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnTakeDamage(static_cast<CEOnTakeDamage&>(evData.get()).damageInfo);
	});
}

void SPlayerComponent::OnSetSlopeLimit(float limit)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<float>(limit);
		server->BroadcastTCP("pl_slopelimit",p);
	}
}
void SPlayerComponent::OnSetStepOffset(float offset)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<float>(offset);
		server->BroadcastTCP("pl_stepoffset",p);
	}
}

void SPlayerComponent::SetClientSession(WVServerClient *session) {m_session = session;}

void SPlayerComponent::UpdateViewOrientation(const Quat &rot)
{
	auto charComponent = GetEntity().GetCharacterComponent();
	if(charComponent.valid())
		charComponent->SetViewOrientation(rot);
}

void SPlayerComponent::OnSetViewOrientation(const Quat &orientation)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared() == false)
		return;
	nwm::RecipientFilter rp {};
	rp.Add(GetClientSession());

	NetPacket p;
	p->Write<Quat>(orientation);
	ent.SendNetEventUDP(m_netEvSetViewOrientation,p,rp);
}

WVServerClient *SPlayerComponent::GetClientSession() {return m_session;}

void SPlayerComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->WriteString(GetPlayerName());
	packet->Write<double>(ConnectionTime());
	if(m_entFlashlight == nullptr)
		nwm::write_unique_entity(packet,nullptr);
	else
		nwm::write_unique_entity(packet,m_entFlashlight.get());
}

bool SPlayerComponent::IsAuthed() {return m_bAuthed;}
void SPlayerComponent::SetAuthed(bool b) {m_bAuthed = b;}

void SPlayerComponent::SetWalkSpeed(float speed)
{
	BasePlayerComponent::SetWalkSpeed(speed);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<float>(speed);
		server->BroadcastTCP("pl_speed_walk",p);
	}
}

void SPlayerComponent::SetRunSpeed(float speed)
{
	BasePlayerComponent::SetRunSpeed(speed);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<float>(speed);
		server->BroadcastTCP("pl_speed_run",p);
	}
}

void SPlayerComponent::SetCrouchedWalkSpeed(float speed)
{
	BasePlayerComponent::SetCrouchedWalkSpeed(speed);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<float>(speed);
		server->BroadcastTCP("pl_speed_crouch_walk",p);
	}
}

void SPlayerComponent::SetStandHeight(float height)
{
	BasePlayerComponent::SetStandHeight(height);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<float>(height);
		server->BroadcastTCP("pl_height_stand",p);
	}
}
void SPlayerComponent::SetCrouchHeight(float height)
{
	BasePlayerComponent::SetCrouchHeight(height);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<float>(height);
		server->BroadcastTCP("pl_height_crouch",p);
	}
}
void SPlayerComponent::SetStandEyeLevel(float eyelevel)
{
	BasePlayerComponent::SetStandEyeLevel(eyelevel);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<float>(eyelevel);
		server->BroadcastTCP("pl_eyelevel_stand",p);
	}
}
void SPlayerComponent::SetCrouchEyeLevel(float eyelevel)
{
	BasePlayerComponent::SetCrouchEyeLevel(eyelevel);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<float>(eyelevel);
		server->BroadcastTCP("pl_eyelevel_crouch",p);
	}
}

bool SPlayerComponent::PlaySharedActivity(Activity activity)
{
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	return animComponent.valid() ? animComponent->PlayActivity(activity,pragma::FPlayAnim::None) : false;
}

void SPlayerComponent::SetSprintSpeed(float speed)
{
	BasePlayerComponent::SetSprintSpeed(speed);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<float>(speed);
		server->BroadcastTCP("pl_speed_sprint",p);
	}
}
void SPlayerComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const {outTypeIndex = std::type_index(typeid(BasePlayerComponent));}
void SPlayerComponent::OnTakeDamage(DamageInfo &info)
{
	auto &ent = GetEntity();
	auto *charComponent = static_cast<pragma::SCharacterComponent*>(ent.GetCharacterComponent().get());
	if(charComponent != nullptr && charComponent->GetGodMode() == true)
		info.SetDamage(0);
}
