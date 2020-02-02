#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include <prosper_command_buffer.hpp>

void Lua::SoundEmitter::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCSoundEmitter = luabind::class_<CSoundEmitterHandle,BaseEntityComponentHandle>("SoundEmitterComponent");
	Lua::register_base_sound_emitter_component_methods<luabind::class_<CSoundEmitterHandle,BaseEntityComponentHandle>,CSoundEmitterHandle>(l,defCSoundEmitter);
	defCSoundEmitter.def("CreateSound",static_cast<void(*)(lua_State*,CSoundEmitterHandle&,std::string,uint32_t,bool)>(&Lua::SoundEmitter::CreateSound));
	defCSoundEmitter.def("EmitSound",static_cast<void(*)(lua_State*,CSoundEmitterHandle&,std::string,uint32_t,float,float,bool)>(&Lua::SoundEmitter::EmitSound));
	entsMod[defCSoundEmitter];
}
