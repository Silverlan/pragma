#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/entities/components/s_weapon_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lua_entity_component.hpp>

namespace Lua
{
	namespace Weapon
	{
		namespace Server
		{
			static void SetPrimaryClipSize(lua_State *l,SWeaponHandle &hEnt,UInt32 size);
			static void SetSecondaryClipSize(lua_State *l,SWeaponHandle &hEnt,UInt32 size);
			static void SetMaxPrimaryClipSize(lua_State *l,SWeaponHandle &hEnt,UInt32 size);
			static void SetMaxSecondaryClipSize(lua_State *l,SWeaponHandle &hEnt,UInt32 size);
			static void AddPrimaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num);
			static void AddSecondaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num);
			static void RefillPrimaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num);
			static void RefillSecondaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num);
			static void RefillPrimaryClip(lua_State *l,SWeaponHandle &hEnt);
			static void RefillSecondaryClip(lua_State *l,SWeaponHandle &hEnt);
			static void RemovePrimaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num);
			static void RemoveSecondaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num);
			static void RemovePrimaryClip(lua_State *l,SWeaponHandle &hEnt);
			static void RemoveSecondaryClip(lua_State *l,SWeaponHandle &hEnt);
			static void SetPrimaryAmmoType(lua_State *l,SWeaponHandle &hEnt,UInt32 type);
			static void SetPrimaryAmmoType(lua_State *l,SWeaponHandle &hEnt,const std::string &type);
			static void SetSecondaryAmmoType(lua_State *l,SWeaponHandle &hEnt,UInt32 type);
			static void SetSecondaryAmmoType(lua_State *l,SWeaponHandle &hEnt,const std::string &type);
		};
	};
};
void Lua::register_sv_weapon_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<SWeaponHandle,BaseEntityComponentHandle>("WeaponComponent");
	Lua::register_base_weapon_component_methods<luabind::class_<SWeaponHandle,BaseEntityComponentHandle>,SWeaponHandle>(l,def);
	def.def("SetPrimaryClipSize",&Lua::Weapon::Server::SetPrimaryClipSize);
	def.def("SetSecondaryClipSize",&Lua::Weapon::Server::SetSecondaryClipSize);
	def.def("SetMaxPrimaryClipSize",&Lua::Weapon::Server::SetMaxPrimaryClipSize);
	def.def("SetMaxSecondaryClipSize",&Lua::Weapon::Server::SetMaxSecondaryClipSize);
	def.def("AddPrimaryClip",&Lua::Weapon::Server::AddPrimaryClip);
	def.def("AddSecondaryClip",&Lua::Weapon::Server::AddSecondaryClip);
	def.def("RefillPrimaryClip",static_cast<void(*)(lua_State*,SWeaponHandle&,UInt32)>(&Lua::Weapon::Server::RefillPrimaryClip));
	def.def("RefillSecondaryClip",static_cast<void(*)(lua_State*,SWeaponHandle&,UInt32)>(&Lua::Weapon::Server::RefillSecondaryClip));
	def.def("RefillPrimaryClip",static_cast<void(*)(lua_State*,SWeaponHandle&)>(&Lua::Weapon::Server::RefillPrimaryClip));
	def.def("RefillSecondaryClip",static_cast<void(*)(lua_State*,SWeaponHandle&)>(&Lua::Weapon::Server::RefillSecondaryClip));
	def.def("RemovePrimaryClip",static_cast<void(*)(lua_State*,SWeaponHandle&,UInt32)>(&Lua::Weapon::Server::RemovePrimaryClip));
	def.def("RemoveSecondaryClip",static_cast<void(*)(lua_State*,SWeaponHandle&,UInt32)>(&Lua::Weapon::Server::RemoveSecondaryClip));
	def.def("RemovePrimaryClip",static_cast<void(*)(lua_State*,SWeaponHandle&)>(&Lua::Weapon::Server::RemovePrimaryClip));
	def.def("RemoveSecondaryClip",static_cast<void(*)(lua_State*,SWeaponHandle&)>(&Lua::Weapon::Server::RemoveSecondaryClip));
	def.def("SetPrimaryAmmoType",static_cast<void(*)(lua_State*,SWeaponHandle&,UInt32)>(&Lua::Weapon::Server::SetPrimaryAmmoType));
	def.def("SetPrimaryAmmoType",static_cast<void(*)(lua_State*,SWeaponHandle&,const std::string&)>(&Lua::Weapon::Server::SetPrimaryAmmoType));
	def.def("SetSecondaryAmmoType",static_cast<void(*)(lua_State*,SWeaponHandle&,UInt32)>(&Lua::Weapon::Server::SetSecondaryAmmoType));
	def.def("SetSecondaryAmmoType",static_cast<void(*)(lua_State*,SWeaponHandle&,const std::string&)>(&Lua::Weapon::Server::SetSecondaryAmmoType));
	module[def];
}
void Lua::Weapon::Server::SetPrimaryClipSize(lua_State *l,SWeaponHandle &hEnt,UInt32 size)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt.get()->SetPrimaryClipSize(umath::limit<UInt16>(size));
}
void Lua::Weapon::Server::SetSecondaryClipSize(lua_State *l,SWeaponHandle &hEnt,UInt32 size)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt.get()->SetSecondaryClipSize(umath::limit<UInt16>(size));
}
void Lua::Weapon::Server::SetMaxPrimaryClipSize(lua_State *l,SWeaponHandle &hEnt,UInt32 size)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt.get()->SetMaxPrimaryClipSize(umath::limit<UInt16>(size));
}
void Lua::Weapon::Server::SetMaxSecondaryClipSize(lua_State *l,SWeaponHandle &hEnt,UInt32 size)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt.get()->SetMaxSecondaryClipSize(umath::limit<UInt16>(size));
}
void Lua::Weapon::Server::AddPrimaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->AddPrimaryClip(umath::limit<UInt16>(num));
}
void Lua::Weapon::Server::AddSecondaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->AddSecondaryClip(umath::limit<UInt16>(num));
}
void Lua::Weapon::Server::RefillPrimaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->RefillPrimaryClip(umath::limit<UInt16>(num));
}
void Lua::Weapon::Server::RefillSecondaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->RefillSecondaryClip(umath::limit<UInt16>(num));
}
void Lua::Weapon::Server::RefillPrimaryClip(lua_State *l,SWeaponHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->RefillPrimaryClip();
}
void Lua::Weapon::Server::RefillSecondaryClip(lua_State *l,SWeaponHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->RefillSecondaryClip();
}
void Lua::Weapon::Server::RemovePrimaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->RemovePrimaryClip(umath::limit<UInt16>(num));
}
void Lua::Weapon::Server::RemoveSecondaryClip(lua_State *l,SWeaponHandle &hEnt,UInt32 num)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->RemoveSecondaryClip(umath::limit<UInt16>(num));
}
void Lua::Weapon::Server::RemovePrimaryClip(lua_State *l,SWeaponHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->RemovePrimaryClip();
}
void Lua::Weapon::Server::RemoveSecondaryClip(lua_State *l,SWeaponHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->RemoveSecondaryClip();
}
void Lua::Weapon::Server::SetPrimaryAmmoType(lua_State *l,SWeaponHandle &hEnt,UInt32 type)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->SetPrimaryAmmoType(type);
}
void Lua::Weapon::Server::SetPrimaryAmmoType(lua_State *l,SWeaponHandle &hEnt,const std::string &type)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->SetPrimaryAmmoType(type);
}
void Lua::Weapon::Server::SetSecondaryAmmoType(lua_State *l,SWeaponHandle &hEnt,UInt32 type)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt.get()->SetSecondaryAmmoType(type);
}
void Lua::Weapon::Server::SetSecondaryAmmoType(lua_State *l,SWeaponHandle &hEnt,const std::string &type)
{
	pragma::Lua::check_component(l,hEnt);
	static_cast<pragma::SWeaponComponent*>(hEnt.get())->SetSecondaryAmmoType(type);
}