// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.bullet_info;

std::ostream &pragma::game::operator<<(std::ostream &out, const BulletInfo &o)
{
	out << "BulletInfo";
	out << "[Spread:" << o.spread << "]";
	out << "[Force:" << o.force << "]";
	out << "[Dist:" << o.distance << "]";
	out << "[Dir:" << o.direction << "]";
	out << "[Origin:" << o.effectOrigin << "]";
	out << "[Dmg:" << o.damage << "]";
	out << "[DmgType:" << magic_enum::enum_name(o.damageType) << "]";
	out << "[Bullets:" << o.bulletCount << "]";

	out << "[Attacker:";
	if(o.hAttacker.valid())
		const_cast<ecs::BaseEntity *>(o.hAttacker.get())->print(out);
	else
		out << "NULL";
	out << "]";

	out << "[Inflictor:";
	if(o.hInflictor.valid())
		const_cast<ecs::BaseEntity *>(o.hInflictor.get())->print(out);
	else
		out << "NULL";
	out << "]";

	out << "[Tracers:" << o.tracerCount << "]";
	out << "[TracerRadius:" << o.tracerRadius << "]";
	out << "[TracerCol:" << o.tracerColor << "]";
	out << "[TracerLen:" << o.tracerLength << "]";
	out << "[TracerSpeed:" << o.tracerSpeed << "]";
	out << "[TracerMat:" << o.tracerMaterial << "]";
	out << "[TracerBloom:" << o.tracerBloom << "]";
	out << "[AmmoType:" << o.ammoType << "]";
	return out;
}
