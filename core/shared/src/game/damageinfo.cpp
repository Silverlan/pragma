/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/game/damageinfo.h"
#include "pragma/lua/classes/ldef_entity.h"

HitGroup DamageInfo::GetHitGroup() const {return m_hitGroup;}
void DamageInfo::SetHitGroup(HitGroup hitGroup) {m_hitGroup = hitGroup;}
void DamageInfo::SetDamage(unsigned short dmg)
{
	m_damage = dmg;
}
void DamageInfo::AddDamage(unsigned short dmg)
{
	if(static_cast<unsigned int>(m_damage) +static_cast<unsigned int>(dmg) > std::numeric_limits<unsigned short>::max())
		m_damage = std::numeric_limits<unsigned short>::max();
	else m_damage += dmg;
}
void DamageInfo::ScaleDamage(float scale)
{
	m_damage = umath::limit<UInt16>(UInt32(Float(m_damage) *scale));
}
unsigned short DamageInfo::GetDamage() {return m_damage;}
BaseEntity *DamageInfo::GetAttacker()
{
	if(!m_attacker.IsValid())
		return nullptr;
	return m_attacker.get();
}
BaseEntity *DamageInfo::GetInflictor()
{
	if(!m_inflictor.IsValid())
		return nullptr;
	return m_inflictor.get();
}
void DamageInfo::SetAttacker(const BaseEntity *ent)
{
	if(m_attacker.IsValid())
		m_attacker = EntityHandle();
	if(ent == nullptr)
		return;
	m_attacker = ent->GetHandle();
}
void DamageInfo::SetAttacker(const EntityHandle &hnd) {SetAttacker(hnd.get());}
void DamageInfo::SetInflictor(const BaseEntity *ent)
{
	if(m_inflictor.IsValid())
		m_inflictor = EntityHandle();
	if(ent == nullptr)
		return;
	m_inflictor = ent->GetHandle();
}
void DamageInfo::SetInflictor(const EntityHandle &hnd) {SetInflictor(hnd.get());}
unsigned int DamageInfo::GetDamageTypes()
{
	return m_types;
}
void DamageInfo::SetDamageType(DAMAGETYPE type)
{
	m_types = type;
}
void DamageInfo::AddDamageType(DAMAGETYPE type)
{
	m_types |= type;
}
void DamageInfo::RemoveDamageType(DAMAGETYPE type)
{
	m_types &= ~type;
}
bool DamageInfo::IsDamageType(DAMAGETYPE type)
{
	return (m_types &UInt32(type)) == UInt32(type) ? true : false;
}
void DamageInfo::SetSource(const Vector3 &origin)
{
	m_source = origin;
}
Vector3 &DamageInfo::GetSource()
{
	return m_source;
}
void DamageInfo::SetHitPosition(const Vector3 &pos)
{
	m_hitPosition = pos;
}
Vector3 &DamageInfo::GetHitPosition()
{
	return m_hitPosition;
}
void DamageInfo::SetForce(const Vector3 &force)
{
	m_force = force;
}
Vector3 &DamageInfo::GetForce()
{
	return m_force;
}