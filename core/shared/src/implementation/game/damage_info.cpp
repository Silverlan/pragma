// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.damage_info;

pragma::physics::HitGroup pragma::game::DamageInfo::GetHitGroup() const { return m_hitGroup; }
void pragma::game::DamageInfo::SetHitGroup(physics::HitGroup hitGroup) { m_hitGroup = hitGroup; }
void pragma::game::DamageInfo::SetDamage(unsigned short dmg) { m_damage = dmg; }
void pragma::game::DamageInfo::AddDamage(unsigned short dmg)
{
	if(static_cast<unsigned int>(m_damage) + static_cast<unsigned int>(dmg) > std::numeric_limits<unsigned short>::max())
		m_damage = std::numeric_limits<unsigned short>::max();
	else
		m_damage += dmg;
}
void pragma::game::DamageInfo::ScaleDamage(float scale) { m_damage = pragma::math::limit<UInt16>(UInt32(Float(m_damage) * scale)); }
unsigned short pragma::game::DamageInfo::GetDamage() { return m_damage; }
pragma::ecs::BaseEntity *pragma::game::DamageInfo::GetAttacker()
{
	if(!m_attacker.valid())
		return nullptr;
	return m_attacker.get();
}
pragma::ecs::BaseEntity *pragma::game::DamageInfo::GetInflictor()
{
	if(!m_inflictor.valid())
		return nullptr;
	return m_inflictor.get();
}
void pragma::game::DamageInfo::SetAttacker(const ecs::BaseEntity *ent)
{
	if(m_attacker.valid())
		m_attacker = EntityHandle();
	if(ent == nullptr)
		return;
	m_attacker = ent->GetHandle();
}
void pragma::game::DamageInfo::SetAttacker(const EntityHandle &hnd) { SetAttacker(hnd.get()); }
void pragma::game::DamageInfo::SetInflictor(const ecs::BaseEntity *ent)
{
	if(m_inflictor.valid())
		m_inflictor = EntityHandle();
	if(ent == nullptr)
		return;
	m_inflictor = ent->GetHandle();
}
void pragma::game::DamageInfo::SetInflictor(const EntityHandle &hnd) { SetInflictor(hnd.get()); }
unsigned int pragma::game::DamageInfo::GetDamageTypes() { return m_types; }
void pragma::game::DamageInfo::SetDamageType(DamageType type) { m_types = type; }
void pragma::game::DamageInfo::AddDamageType(DamageType type) { m_types |= type; }
void pragma::game::DamageInfo::RemoveDamageType(DamageType type) { m_types &= ~type; }
bool pragma::game::DamageInfo::IsDamageType(DamageType type) { return (m_types & UInt32(type)) == UInt32(type) ? true : false; }
void pragma::game::DamageInfo::SetSource(const Vector3 &origin) { m_source = origin; }
Vector3 &pragma::game::DamageInfo::GetSource() { return m_source; }
void pragma::game::DamageInfo::SetHitPosition(const Vector3 &pos) { m_hitPosition = pos; }
Vector3 &pragma::game::DamageInfo::GetHitPosition() { return m_hitPosition; }
void pragma::game::DamageInfo::SetForce(const Vector3 &force) { m_force = force; }
Vector3 &pragma::game::DamageInfo::GetForce() { return m_force; }

std::ostream &pragma::game::operator<<(std::ostream &out, const DamageInfo &po)
{
	auto &o = const_cast<DamageInfo &>(po);
	out << "DamageInfo";
	out << "[Dmg:" << o.GetDamage() << "]";

	auto *attacker = o.GetAttacker();
	out << "[Attacker:";
	if(attacker)
		attacker->print(out);
	else
		out << "NULL";
	out << "]";

	auto *inflictor = o.GetInflictor();
	out << "[Inflictor:";
	if(inflictor)
		inflictor->print(out);
	else
		out << "NULL";
	out << "]";

	out << "[DmgTypes:" << o.GetDamageTypes() << "]";
	out << "[Src:" << o.GetSource() << "]";
	out << "[HitPos:" << o.GetHitPosition() << "]";
	out << "[Force:" << o.GetForce() << "]";
	out << "[HitGroup:" << magic_enum::enum_name(o.GetHitGroup()) << "]";
	return out;
}
