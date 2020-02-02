#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include <prosper_command_buffer.hpp>

void Lua::Flex::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCFlex = luabind::class_<CFlexHandle,BaseEntityComponentHandle>("FlexComponent");
	Lua::register_base_flex_component_methods<luabind::class_<CFlexHandle,BaseEntityComponentHandle>,CFlexHandle>(l,defCFlex);
	defCFlex.def("SetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,uint32_t,float,float,bool)>(
		[](lua_State *l,CFlexHandle &hEnt,uint32_t flexId,float value,float duration,bool clampToLimits
			) {
				pragma::Lua::check_component(l,hEnt);
				hEnt->SetFlexController(flexId,value,duration,clampToLimits);
		}));
	defCFlex.def("SetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,uint32_t,float,float)>(
		[](lua_State *l,CFlexHandle &hEnt,uint32_t flexId,float value,float duration
			) {
				pragma::Lua::check_component(l,hEnt);
				hEnt->SetFlexController(flexId,value,duration);
		}));
	defCFlex.def("SetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,uint32_t,float)>(
		[](lua_State *l,CFlexHandle &hEnt,uint32_t flexId,float value
			) {
				pragma::Lua::check_component(l,hEnt);
				hEnt->SetFlexController(flexId,value);
		}));
	defCFlex.def("SetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,const std::string&,float,float,bool)>(
		[](lua_State *l,CFlexHandle &hEnt,const std::string &flexName,float value,float duration,bool clampToLimits
			) {
				pragma::Lua::check_component(l,hEnt);
				hEnt->SetFlexController(flexName,value,duration,clampToLimits);
		}));
	defCFlex.def("SetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,const std::string&,float,float)>(
		[](lua_State *l,CFlexHandle &hEnt,const std::string &flexName,float value,float duration
			) {
				pragma::Lua::check_component(l,hEnt);
				hEnt->SetFlexController(flexName,value,duration);
		}));
	defCFlex.def("SetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,const std::string&,float)>(
		[](lua_State *l,CFlexHandle &hEnt,const std::string &flexName,float value
			) {
				pragma::Lua::check_component(l,hEnt);
				hEnt->SetFlexController(flexName,value);
		}));
	defCFlex.def("GetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,uint32_t)>(&Lua::Flex::GetFlexController));
	defCFlex.def("GetFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,const std::string&)>(&Lua::Flex::GetFlexController));
	defCFlex.def("GetScaledFlexController",static_cast<void(*)(lua_State*,CFlexHandle&,uint32_t)>(
		[](lua_State *l,CFlexHandle &hEnt,uint32_t flexControllerId
			) {
				pragma::Lua::check_component(l,hEnt);
				float value;
				if(hEnt->GetScaledFlexController(flexControllerId,value) == false)
					return;
				Lua::PushNumber(l,value);
		}));
	defCFlex.def("SetFlexControllerScale",static_cast<void(*)(lua_State*,CFlexHandle&,float)>(
		[](lua_State *l,CFlexHandle &hEnt,float scale
			) {
				pragma::Lua::check_component(l,hEnt);
				hEnt->SetFlexControllerScale(scale);
		}));
	defCFlex.def("GetFlexControllerScale",static_cast<void(*)(lua_State*,CFlexHandle&)>(
		[](lua_State *l,CFlexHandle &hEnt
			) {
				pragma::Lua::check_component(l,hEnt);
				Lua::PushNumber(l,hEnt->GetFlexControllerScale());
		}));
	defCFlex.def("CalcFlexValue",&Lua::Flex::CalcFlexValue);
	entsMod[defCFlex];
}
