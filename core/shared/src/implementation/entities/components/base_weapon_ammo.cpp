// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_weapon;

using namespace pragma;

UInt32 BaseWeaponComponent::GetPrimaryAmmoType() const { return *m_ammoPrimary; }
const util::PUInt32Property &BaseWeaponComponent::GetPrimaryAmmoTypeProperty() const { return m_ammoPrimary; }
UInt32 BaseWeaponComponent::GetSecondaryAmmoType() const { return *m_ammoSecondary; }
const util::PUInt32Property &BaseWeaponComponent::GetSecondaryAmmoTypeProperty() const { return m_ammoSecondary; }
Bool BaseWeaponComponent::IsPrimaryClipEmpty() const { return (**m_clipPrimary == 0) ? true : false; }
Bool BaseWeaponComponent::IsSecondaryClipEmpty() const { return (**m_clipSecondary == 0) ? true : false; }
Bool BaseWeaponComponent::HasPrimaryAmmo() const
{
	if(GetPrimaryClipSize() > 0)
		return true;
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner == nullptr || !owner->IsCharacter())
		return false;
	auto charComponent = owner->GetCharacterComponent();
	return (charComponent->GetAmmoCount(*m_ammoPrimary) > 0) ? true : false;
}
Bool BaseWeaponComponent::HasSecondaryAmmo() const
{
	if(GetSecondaryClipSize() > 0)
		return true;
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner == nullptr || !owner->IsCharacter())
		return false;
	auto charComponent = owner->GetCharacterComponent();
	return (charComponent->GetAmmoCount(*m_ammoSecondary) > 0) ? true : false;
}
Bool BaseWeaponComponent::HasAmmo() const { return (HasPrimaryAmmo() == true || HasSecondaryAmmo() == true) ? true : false; }
void BaseWeaponComponent::SetPrimaryClipSize(UInt16 size)
{
	auto oldSize = *m_clipPrimary;
	auto max = GetMaxPrimaryClipSize();
	if(size > max)
		size = max;
	*m_clipPrimary = size;

	CEOnClipSizeChanged evData {oldSize, size};
	BroadcastEvent(baseWeaponComponent::EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED, evData);
}
void BaseWeaponComponent::SetSecondaryClipSize(UInt16 size)
{
	auto oldSize = *m_clipSecondary;
	auto max = GetMaxSecondaryClipSize();
	if(size > max)
		size = max;
	*m_clipSecondary = size;

	CEOnClipSizeChanged evData {oldSize, size};
	BroadcastEvent(baseWeaponComponent::EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED, evData);
}
UInt16 BaseWeaponComponent::GetPrimaryClipSize() const { return *m_clipPrimary; }
const util::PUInt16Property &BaseWeaponComponent::GetPrimaryClipSizeProperty() const { return m_clipPrimary; }
UInt16 BaseWeaponComponent::GetSecondaryClipSize() const { return *m_clipSecondary; }
const util::PUInt16Property &BaseWeaponComponent::GetSecondaryClipSizeProperty() const { return m_clipSecondary; }
UInt16 BaseWeaponComponent::GetMaxPrimaryClipSize() const { return *m_maxPrimaryClipSize; }
const util::PUInt16Property &BaseWeaponComponent::GetMaxPrimaryClipSizeProperty() const { return m_maxPrimaryClipSize; }
UInt16 BaseWeaponComponent::GetMaxSecondaryClipSize() const { return *m_maxSecondaryClipSize; }
const util::PUInt16Property &BaseWeaponComponent::GetMaxSecondaryClipSizeProperty() const { return m_maxSecondaryClipSize; }
void BaseWeaponComponent::SetMaxPrimaryClipSize(UInt16 size) { *m_maxPrimaryClipSize = size; }
void BaseWeaponComponent::SetMaxSecondaryClipSize(UInt16 size) { *m_maxSecondaryClipSize = size; }
void BaseWeaponComponent::SetPrimaryAmmoType(UInt32 type)
{
	*m_ammoPrimary = type;
	SetPrimaryClipSize(GetPrimaryClipSize()); // Update
}
void BaseWeaponComponent::SetSecondaryAmmoType(UInt32 type)
{
	*m_ammoSecondary = type;
	SetSecondaryClipSize(GetSecondaryClipSize()); // Update
}
