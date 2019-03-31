#include "stdafx_server.h"
#include "pragma/entities/components/s_weapon_component.hpp"
#include "pragma/entities/player.h"
#include "pragma/networking/wvserverclient.h"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/velocity_component.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <sharedutils/datastream.h>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

std::vector<SWeaponComponent*> SWeaponComponent::s_weapons;
const std::vector<SWeaponComponent*> &SWeaponComponent::GetAll() {return s_weapons;}
unsigned int SWeaponComponent::GetWeaponCount() {return static_cast<uint32_t>(s_weapons.size());}

extern DLLSERVER ServerState *server;
SWeaponComponent::SWeaponComponent(BaseEntity &ent)
	: BaseWeaponComponent(ent)
{
	s_weapons.push_back(this);
}

SWeaponComponent::~SWeaponComponent()
{
	auto it = std::find(s_weapons.begin(),s_weapons.end(),this);
	if(it != s_weapons.end())
		s_weapons.erase(it);
}

luabind::object SWeaponComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SWeaponComponentHandleWrapper>(l);}

void SWeaponComponent::OnUse(BaseEntity *pl)
{
	auto &ent = GetEntity();
	auto hEnt = ent.GetHandle();
	if(pl != nullptr)
	{
		auto charComponent = pl->GetCharacterComponent();
		if(charComponent.valid())
			charComponent->GiveWeapon(GetEntity());
	}
	if(hEnt.IsValid() == false)
		return;
	OnPickedUp(pl);
}
void SWeaponComponent::OnPickedUp(BaseEntity *ent) {}

void SWeaponComponent::Drop()
{
	auto *pOwnerComponent = GetOwnerComponent();
	auto *owner = (pOwnerComponent != nullptr) ? pOwnerComponent->GetOwner() : nullptr;
	if(owner == nullptr)
		return;
	pOwnerComponent->ClearOwner();
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent.valid() ? pPhysComponent->InitializePhysics(PHYSICSTYPE::DYNAMIC) : nullptr;
	if(owner != nullptr && owner->IsPlayer())
	{
		auto &ent = GetEntity();
		auto hEnt = ent.GetHandle();
		auto charComponent = ent.GetCharacterComponent();
		auto sCharComponent = ent.GetComponent<SCharacterComponent>();
		if(sCharComponent.expired() == false)
			sCharComponent.get()->DropWeapon(&ent);
		if(hEnt.IsValid() == false)
			return;
		auto pTrComponent = ent.GetTransformComponent();
		if(pTrComponent.expired())
			return;
		auto pos = pTrComponent->GetEyePosition();
		auto ang = charComponent.valid() ? charComponent->GetViewAngles() : pTrComponent->GetAngles();
		ang.p = 0.f;
		ang.r = 0;
		auto dir = ang.Forward();
		pTrComponent->SetPosition(pos +dir *10.f);
		if(phys != nullptr)
		{
			pTrComponent->SetAngles(uvec::to_angle(dir) +EulerAngles(0.f,180.f,0.f));
			auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
			if(pVelComponent.valid())
			{
				pVelComponent->SetVelocity(dir *200.f);
				pVelComponent->SetLocalAngularVelocity(Vector3(0.f,0.f,0.f));
			}
			return;
		}
	}
	if(pPhysComponent.valid())
		pPhysComponent->DropToFloor();
}

void SWeaponComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<UInt32>(GetPrimaryAmmoType());
	packet->Write<UInt32>(GetSecondaryAmmoType());
	packet->Write<UInt16>(GetPrimaryClipSize());
	packet->Write<UInt16>(GetSecondaryClipSize());
	packet->Write<UInt16>(GetMaxPrimaryClipSize());
	packet->Write<UInt16>(GetMaxSecondaryClipSize());
}

void SWeaponComponent::Initialize()
{
	BaseWeaponComponent::Initialize();

	BindEvent(UsableComponent::EVENT_CAN_USE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &bCanUse = static_cast<CECanUseData&>(evData.get()).canUse;
		auto *pOwnerComponent = GetOwnerComponent();
		bCanUse = (pOwnerComponent == nullptr || pOwnerComponent->GetOwner() == nullptr);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(UsableComponent::EVENT_ON_USE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnUse(static_cast<CEOnUseData&>(evData.get()).entity);
	});

	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	ent.AddComponent<UsableComponent>();

	ent.SetShared(true);
}

void SWeaponComponent::OnEntitySpawn()
{
	BaseWeaponComponent::OnEntitySpawn();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->InitializePhysics(PHYSICSTYPE::DYNAMIC);
}

void SWeaponComponent::OnPhysicsInitialized() {BaseWeaponComponent::OnPhysicsInitialized();}

void SWeaponComponent::GetTargetRecipients(nwm::RecipientFilter &rf)
{
	rf = nwm::RecipientFilter(nwm::RecipientFilter::Type::Exclude);
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner != nullptr && owner->IsPlayer())
		rf.Add(owner->GetComponent<SPlayerComponent>().get()->GetClientSession());
}

void SWeaponComponent::Deploy()
{
	BaseWeaponComponent::Deploy();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		server->BroadcastUDP("wep_deploy",p);
		//server->SendPacketTCP("wep_deploy",p,GetTargetRecipients());
	}
}

void SWeaponComponent::Holster()
{
	BaseWeaponComponent::Holster();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		server->BroadcastUDP("wep_holster",p);
		//server->SendPacketTCP("wep_holster",p,GetTargetRecipients());
	}
}

void SWeaponComponent::Think(double tDelta)
{
	BaseWeaponComponent::Think(tDelta);
}
void SWeaponComponent::OnRemove()
{
	BaseWeaponComponent::OnRemove();
}

void SWeaponComponent::PrimaryAttack()
{
	if(!CanPrimaryAttack())
		return;
	BaseWeaponComponent::PrimaryAttack();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->SendPacketUDP("wep_primaryattack",p,rpFilter);
	}
}
void SWeaponComponent::SecondaryAttack()
{
	if(!CanSecondaryAttack())
		return;
	BaseWeaponComponent::SecondaryAttack();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->SendPacketUDP("wep_secondaryattack",p,rpFilter);
	}
}
void SWeaponComponent::TertiaryAttack()
{
	BaseWeaponComponent::TertiaryAttack();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->SendPacketUDP("wep_attack3",p,rpFilter);
	}
}
void SWeaponComponent::Attack4()
{
	BaseWeaponComponent::Attack4();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->SendPacketUDP("wep_attack4",p,rpFilter);
	}
}
void SWeaponComponent::Reload()
{
	BaseWeaponComponent::Reload();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->SendPacketUDP("wep_reload",p,rpFilter);
	}
}
void SWeaponComponent::SetPrimaryClipSize(UInt16 size)
{
	BaseWeaponComponent::SetPrimaryClipSize(size);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<UInt16>(*m_clipPrimary);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->BroadcastUDP("wep_prim_clip_size",p);
	}
}
void SWeaponComponent::SetSecondaryClipSize(UInt16 size)
{
	BaseWeaponComponent::SetSecondaryClipSize(size);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<UInt16>(*m_clipSecondary);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->BroadcastUDP("wep_sec_clip_size",p);
	}
}
void SWeaponComponent::SetMaxPrimaryClipSize(UInt16 size)
{
	BaseWeaponComponent::SetMaxPrimaryClipSize(size);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<UInt16>(*m_maxPrimaryClipSize);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->BroadcastUDP("wep_prim_max_clip_size",p);
	}
}
void SWeaponComponent::SetMaxSecondaryClipSize(UInt16 size)
{
	BaseWeaponComponent::SetMaxSecondaryClipSize(size);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<UInt16>(*m_maxSecondaryClipSize);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->BroadcastUDP("wep_sec_max_clip_size",p);
	}
}
void SWeaponComponent::SetPrimaryAmmoType(UInt32 type)
{
	BaseWeaponComponent::SetPrimaryAmmoType(type);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<UInt32>(type);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->BroadcastUDP("wep_prim_ammo_type",p);
	}
}
void SWeaponComponent::SetSecondaryAmmoType(UInt32 type)
{
	BaseWeaponComponent::SetSecondaryAmmoType(type);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<UInt32>(type);
		nwm::RecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		server->BroadcastUDP("wep_sec_ammo_type",p);
	}
}
void SWeaponComponent::AddPrimaryClip(UInt16 num) {SetPrimaryClipSize(umath::limit<UInt16>(CUInt32(GetPrimaryClipSize()) +CUInt32(num)));}
void SWeaponComponent::AddSecondaryClip(UInt16 num) {SetSecondaryClipSize(umath::limit<UInt16>(CUInt32(GetSecondaryClipSize()) +CUInt32(num)));}
void SWeaponComponent::RefillPrimaryClip(UInt16 num)
{
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner == nullptr)
		return;
	if(!owner->IsPlayer())
	{
		AddPrimaryClip(num);
		return;
	}
	auto charComponent = owner->GetCharacterComponent();
	auto ammo = charComponent.valid() ? charComponent->GetAmmoCount(*m_ammoPrimary) : 0u;
	auto clip = GetMaxPrimaryClipSize();
	auto clipCurrent = GetPrimaryClipSize();
	if(clipCurrent >= clip)
		return;
	auto clipAdd = clip -clipCurrent;
	if(ammo < clipAdd)
		clipAdd = ammo;
	if(charComponent.valid())
		charComponent->SetAmmoCount(*m_ammoPrimary,CUInt16(ammo -clipAdd));
	AddPrimaryClip(CUInt16(clipAdd));
}
void SWeaponComponent::RefillSecondaryClip(UInt16 num)
{
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner == nullptr)
		return;
	if(!owner->IsPlayer())
	{
		AddSecondaryClip(num);
		return;
	}
	auto charComponent = owner->GetCharacterComponent();
	auto ammo = charComponent.valid() ? charComponent->GetAmmoCount(*m_ammoSecondary) : 0u;
	auto clip = GetMaxSecondaryClipSize();
	auto clipCurrent = GetSecondaryClipSize();
	if(clipCurrent >= clip)
		return;
	auto clipAdd = clip -clipCurrent;
	if(ammo < clipAdd)
		clipAdd = ammo;
	if(charComponent.valid())
		charComponent->SetAmmoCount(*m_ammoSecondary,CUInt16(ammo -clipAdd));
	AddSecondaryClip(CUInt16(clipAdd));
}
void SWeaponComponent::RefillPrimaryClip() {RefillPrimaryClip(GetMaxPrimaryClipSize());}
void SWeaponComponent::RefillSecondaryClip() {RefillSecondaryClip(GetMaxSecondaryClipSize());}
void SWeaponComponent::RemovePrimaryClip(UInt16 num)
{
	auto clip = GetPrimaryClipSize();
	if(num >= clip)
		clip = 0;
	else
		clip -= num;
	SetPrimaryClipSize(clip);
}
void SWeaponComponent::RemoveSecondaryClip(UInt16 num)
{
	auto clip = GetSecondaryClipSize();
	if(num >= clip)
		clip = 0;
	else
		clip -= num;
	SetSecondaryClipSize(clip);
}
void SWeaponComponent::SetPrimaryAmmoType(const std::string &type)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	UInt32 ammoTypeId = 0;
	if(game->GetAmmoType(type,&ammoTypeId) == nullptr)
		return;
	SetPrimaryAmmoType(ammoTypeId);
}
void SWeaponComponent::SetSecondaryAmmoType(const std::string &type)
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	UInt32 ammoTypeId = 0;
	if(game->GetAmmoType(type,&ammoTypeId) == nullptr)
		return;
	SetSecondaryAmmoType(ammoTypeId);
}

