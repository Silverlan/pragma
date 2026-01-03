// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.weapon;

import :entities;
import :entities.components;
import :server_state;

using namespace pragma;

std::vector<SWeaponComponent *> SWeaponComponent::s_weapons;
const std::vector<SWeaponComponent *> &SWeaponComponent::GetAll() { return s_weapons; }
unsigned int SWeaponComponent::GetWeaponCount() { return static_cast<uint32_t>(s_weapons.size()); }

SWeaponComponent::SWeaponComponent(ecs::BaseEntity &ent) : BaseWeaponComponent(ent) { s_weapons.push_back(this); }

SWeaponComponent::~SWeaponComponent()
{
	auto it = std::find(s_weapons.begin(), s_weapons.end(), this);
	if(it != s_weapons.end())
		s_weapons.erase(it);
}

void SWeaponComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SWeaponComponent::OnUse(ecs::BaseEntity *pl)
{
	auto &ent = GetEntity();
	auto hEnt = ent.GetHandle();
	if(pl != nullptr) {
		auto charComponent = pl->GetCharacterComponent();
		if(charComponent.valid())
			charComponent->GiveWeapon(GetEntity());
	}
	if(hEnt.valid() == false)
		return;
	OnPickedUp(pl);
}
void SWeaponComponent::OnPickedUp(ecs::BaseEntity *ent) {}

void SWeaponComponent::Drop()
{
	auto *pOwnerComponent = GetOwnerComponent();
	auto *owner = (pOwnerComponent != nullptr) ? pOwnerComponent->GetOwner() : nullptr;
	if(owner == nullptr)
		return;
	pOwnerComponent->ClearOwner();
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto *phys = pPhysComponent != nullptr ? pPhysComponent->InitializePhysics(physics::PhysicsType::Dynamic) : nullptr;
	if(owner != nullptr && owner->IsPlayer()) {
		auto &ent = GetEntity();
		auto hEnt = ent.GetHandle();
		auto charComponent = ent.GetCharacterComponent();
		auto sCharComponent = ent.GetComponent<SCharacterComponent>();
		if(sCharComponent.expired() == false)
			sCharComponent.get()->DropWeapon(&ent);
		if(hEnt.valid() == false)
			return;
		auto pTrComponent = ent.GetTransformComponent();
		if(pTrComponent == nullptr)
			return;
		auto pos = pTrComponent->GetEyePosition();
		auto ang = charComponent.valid() ? charComponent->GetViewAngles() : pTrComponent->GetAngles();
		ang.p = 0.f;
		ang.r = 0;
		auto dir = ang.Forward();
		pTrComponent->SetPosition(pos + dir * 10.f);
		if(phys != nullptr) {
			pTrComponent->SetAngles(uvec::to_angle(dir) + EulerAngles(0.f, 180.f, 0.f));
			auto pVelComponent = ent.GetComponent<VelocityComponent>();
			if(pVelComponent.valid()) {
				pVelComponent->SetVelocity(dir * 200.f);
				pVelComponent->SetLocalAngularVelocity(Vector3(0.f, 0.f, 0.f));
			}
			return;
		}
	}
	if(pPhysComponent != nullptr)
		pPhysComponent->DropToFloor();
}

void SWeaponComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
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

	BindEvent(usableComponent::EVENT_CAN_USE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &bCanUse = static_cast<CECanUseData &>(evData.get()).canUse;
		auto *pOwnerComponent = GetOwnerComponent();
		bCanUse = (pOwnerComponent == nullptr || pOwnerComponent->GetOwner() == nullptr);
		return util::EventReply::Handled;
	});
	BindEventUnhandled(usableComponent::EVENT_ON_USE, [this](std::reference_wrapper<ComponentEvent> evData) { OnUse(static_cast<CEOnUseData &>(evData.get()).entity); });

	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	ent.AddComponent<UsableComponent>();

	ent.SetShared(true);
}

void SWeaponComponent::OnEntitySpawn()
{
	BaseWeaponComponent::OnEntitySpawn();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->InitializePhysics(physics::PhysicsType::Dynamic);
}

void SWeaponComponent::OnPhysicsInitialized() { BaseWeaponComponent::OnPhysicsInitialized(); }

void SWeaponComponent::GetTargetRecipients(networking::ClientRecipientFilter &rf)
{
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner != nullptr && owner->IsPlayer()) {
		auto *session = owner->GetComponent<SPlayerComponent>().get()->GetClientSession();
		if(session) {
			rf = {*session, networking::ClientRecipientFilter::FilterType::Exclude};
			return;
		}
	}
	rf = {};
}

void SWeaponComponent::Deploy()
{
	BaseWeaponComponent::Deploy();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_DEPLOY, p, networking::Protocol::FastUnreliable);
	}
}

void SWeaponComponent::Holster()
{
	BaseWeaponComponent::Holster();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_HOLSTER, p, networking::Protocol::FastUnreliable);
	}
}

void SWeaponComponent::OnTick(double tDelta) { BaseWeaponComponent::OnTick(tDelta); }
void SWeaponComponent::OnRemove() { BaseWeaponComponent::OnRemove(); }

void SWeaponComponent::PrimaryAttack()
{
	if(!CanPrimaryAttack())
		return;
	BaseWeaponComponent::PrimaryAttack();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_PRIMARYATTACK, p, networking::Protocol::FastUnreliable, rpFilter);
	}
}
void SWeaponComponent::SecondaryAttack()
{
	if(!CanSecondaryAttack())
		return;
	BaseWeaponComponent::SecondaryAttack();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_SECONDARYATTACK, p, networking::Protocol::FastUnreliable, rpFilter);
	}
}
void SWeaponComponent::TertiaryAttack()
{
	BaseWeaponComponent::TertiaryAttack();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_ATTACK3, p, networking::Protocol::FastUnreliable, rpFilter);
	}
}
void SWeaponComponent::Attack4()
{
	BaseWeaponComponent::Attack4();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_ATTACK4, p, networking::Protocol::FastUnreliable, rpFilter);
	}
}
void SWeaponComponent::Reload()
{
	BaseWeaponComponent::Reload();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_RELOAD, p, networking::Protocol::FastUnreliable, rpFilter);
	}
}
void SWeaponComponent::SetPrimaryClipSize(UInt16 size)
{
	BaseWeaponComponent::SetPrimaryClipSize(size);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<UInt16>(*m_clipPrimary);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_PRIM_CLIP_SIZE, p, networking::Protocol::FastUnreliable);
	}
}
void SWeaponComponent::SetSecondaryClipSize(UInt16 size)
{
	BaseWeaponComponent::SetSecondaryClipSize(size);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<UInt16>(*m_clipSecondary);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_SEC_CLIP_SIZE, p, networking::Protocol::FastUnreliable);
	}
}
void SWeaponComponent::SetMaxPrimaryClipSize(UInt16 size)
{
	BaseWeaponComponent::SetMaxPrimaryClipSize(size);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<UInt16>(*m_maxPrimaryClipSize);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_PRIM_MAX_CLIP_SIZE, p, networking::Protocol::FastUnreliable);
	}
}
void SWeaponComponent::SetMaxSecondaryClipSize(UInt16 size)
{
	BaseWeaponComponent::SetMaxSecondaryClipSize(size);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<UInt16>(*m_maxSecondaryClipSize);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_SEC_MAX_CLIP_SIZE, p, networking::Protocol::FastUnreliable);
	}
}
void SWeaponComponent::SetPrimaryAmmoType(UInt32 type)
{
	BaseWeaponComponent::SetPrimaryAmmoType(type);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<UInt32>(type);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_PRIM_AMMO_TYPE, p, networking::Protocol::FastUnreliable);
	}
}
void SWeaponComponent::SetSecondaryAmmoType(UInt32 type)
{
	BaseWeaponComponent::SetSecondaryAmmoType(type);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<UInt32>(type);
		networking::ClientRecipientFilter rpFilter;
		GetTargetRecipients(rpFilter);
		ServerState::Get()->SendPacket(networking::net_messages::client::WEP_SEC_AMMO_TYPE, p, networking::Protocol::FastUnreliable);
	}
}
void SWeaponComponent::AddPrimaryClip(UInt16 num) { SetPrimaryClipSize(pragma::math::limit<UInt16>(CUInt32(GetPrimaryClipSize()) + CUInt32(num))); }
void SWeaponComponent::AddSecondaryClip(UInt16 num) { SetSecondaryClipSize(pragma::math::limit<UInt16>(CUInt32(GetSecondaryClipSize()) + CUInt32(num))); }
void SWeaponComponent::RefillPrimaryClip(UInt16 num)
{
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner == nullptr)
		return;
	if(!owner->IsPlayer()) {
		AddPrimaryClip(num);
		return;
	}
	auto charComponent = owner->GetCharacterComponent();
	auto ammo = charComponent.valid() ? charComponent->GetAmmoCount(*m_ammoPrimary) : 0u;
	auto clip = GetMaxPrimaryClipSize();
	auto clipCurrent = GetPrimaryClipSize();
	if(clipCurrent >= clip)
		return;
	auto clipAdd = clip - clipCurrent;
	if(ammo < clipAdd)
		clipAdd = ammo;
	if(charComponent.valid())
		charComponent->SetAmmoCount(*m_ammoPrimary, CUInt16(ammo - clipAdd));
	AddPrimaryClip(CUInt16(clipAdd));
}
void SWeaponComponent::RefillSecondaryClip(UInt16 num)
{
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner == nullptr)
		return;
	if(!owner->IsPlayer()) {
		AddSecondaryClip(num);
		return;
	}
	auto charComponent = owner->GetCharacterComponent();
	auto ammo = charComponent.valid() ? charComponent->GetAmmoCount(*m_ammoSecondary) : 0u;
	auto clip = GetMaxSecondaryClipSize();
	auto clipCurrent = GetSecondaryClipSize();
	if(clipCurrent >= clip)
		return;
	auto clipAdd = clip - clipCurrent;
	if(ammo < clipAdd)
		clipAdd = ammo;
	if(charComponent.valid())
		charComponent->SetAmmoCount(*m_ammoSecondary, CUInt16(ammo - clipAdd));
	AddSecondaryClip(CUInt16(clipAdd));
}
void SWeaponComponent::RefillPrimaryClip() { RefillPrimaryClip(GetMaxPrimaryClipSize()); }
void SWeaponComponent::RefillSecondaryClip() { RefillSecondaryClip(GetMaxSecondaryClipSize()); }
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
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	UInt32 ammoTypeId = 0;
	if(game->GetAmmoType(type, &ammoTypeId) == nullptr)
		return;
	SetPrimaryAmmoType(ammoTypeId);
}
void SWeaponComponent::SetSecondaryAmmoType(const std::string &type)
{
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	UInt32 ammoTypeId = 0;
	if(game->GetAmmoType(type, &ammoTypeId) == nullptr)
		return;
	SetSecondaryAmmoType(ammoTypeId);
}

void SWeaponComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BaseWeaponComponent::RegisterLuaBindings(l, modEnts);

	auto def = pragma::LuaCore::create_entity_component_class<SWeaponComponent, BaseWeaponComponent>("WeaponComponent");
	def.def("SetPrimaryClipSize", &SWeaponComponent::SetPrimaryClipSize);
	def.def("SetSecondaryClipSize", &SWeaponComponent::SetSecondaryClipSize);
	def.def("SetMaxPrimaryClipSize", &SWeaponComponent::SetMaxPrimaryClipSize);
	def.def("SetMaxSecondaryClipSize", &SWeaponComponent::SetMaxSecondaryClipSize);
	def.def("AddPrimaryClip", &SWeaponComponent::AddPrimaryClip);
	def.def("AddSecondaryClip", &SWeaponComponent::AddSecondaryClip);
	def.def("RefillPrimaryClip", static_cast<void (SWeaponComponent::*)(UInt16)>(&SWeaponComponent::RefillPrimaryClip));
	def.def("RefillSecondaryClip", static_cast<void (SWeaponComponent::*)(UInt16)>(&SWeaponComponent::RefillSecondaryClip));
	def.def("RefillPrimaryClip", static_cast<void (SWeaponComponent::*)()>(&SWeaponComponent::RefillPrimaryClip));
	def.def("RefillSecondaryClip", static_cast<void (SWeaponComponent::*)()>(&SWeaponComponent::RefillSecondaryClip));
	def.def("RemovePrimaryClip", static_cast<void (SWeaponComponent::*)(UInt16)>(&SWeaponComponent::RemovePrimaryClip));
	def.def("RemoveSecondaryClip", static_cast<void (SWeaponComponent::*)(UInt16)>(&SWeaponComponent::RemoveSecondaryClip));
	def.def("RemovePrimaryClip", static_cast<void (*)(SWeaponComponent &)>([](SWeaponComponent &wepComponent) { wepComponent.RemovePrimaryClip(); }));
	def.def("RemoveSecondaryClip", static_cast<void (*)(SWeaponComponent &)>([](SWeaponComponent &wepComponent) { wepComponent.RemoveSecondaryClip(); }));
	def.def("SetPrimaryAmmoType", static_cast<void (SWeaponComponent::*)(UInt32)>(&SWeaponComponent::SetPrimaryAmmoType));
	def.def("SetPrimaryAmmoType", static_cast<void (SWeaponComponent::*)(const std::string &)>(&SWeaponComponent::SetPrimaryAmmoType));
	def.def("SetSecondaryAmmoType", static_cast<void (SWeaponComponent::*)(UInt32)>(&SWeaponComponent::SetSecondaryAmmoType));
	def.def("SetSecondaryAmmoType", static_cast<void (SWeaponComponent::*)(const std::string &)>(&SWeaponComponent::SetSecondaryAmmoType));
	modEnts[def];
}
