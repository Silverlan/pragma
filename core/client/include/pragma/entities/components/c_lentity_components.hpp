#ifndef __C_LENTITY_COMPONENTS_HPP__
#define __C_LENTITY_COMPONENTS_HPP__

#include "pragma/clientdefinitions.h"

struct lua_State;
namespace luabind {class module_;};
namespace Lua
{
	DLLCLIENT void register_cl_ai_component(lua_State *l,luabind::module_ &module);
	DLLCLIENT void register_cl_character_component(lua_State *l,luabind::module_ &module);
	DLLCLIENT void register_cl_player_component(lua_State *l,luabind::module_ &module);
	DLLCLIENT void register_cl_vehicle_component(lua_State *l,luabind::module_ &module);
	DLLCLIENT void register_cl_weapon_component(lua_State *l,luabind::module_ &module);

	namespace ParticleSystem
	{
		DLLCLIENT void AddInitializer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o);
		DLLCLIENT void AddOperator(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o);
		DLLCLIENT void AddRenderer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o);
	}
};

#endif
