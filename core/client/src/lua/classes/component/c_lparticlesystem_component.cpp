/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/lua/classes/c_lparticle_modifiers.hpp"
#include "pragma/particlesystem/initializers/c_particle_initializer_lua.hpp"
#include <pragma/entities/environment/effects/particlesystemdata.h>
#include <pragma/model/model.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
static void register_particle_modifier(lua_State *l,pragma::LuaParticleModifierManager::Type type,const std::string &name,luabind::object oClass)
{
	Lua::CheckUserData(l,2);
	auto &particleModMan = c_game->GetLuaParticleModifierManager();
	if(particleModMan.RegisterModifier(type,name,oClass) == false)
		return;

	auto *map = GetParticleModifierMap();
	if(map == nullptr)
		return;
	switch(type)
	{
	case pragma::LuaParticleModifierManager::Type::Initializer:
		map->AddInitializer(name,[name](pragma::CParticleSystemComponent &psc,const std::unordered_map<std::string,std::string> &keyValues) -> std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)> {
			auto &particleModMan = c_game->GetLuaParticleModifierManager();
			auto *modifier = dynamic_cast<CParticleInitializer*>(particleModMan.CreateModifier(name));
			if(modifier == nullptr)
				return std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)>(nullptr,[](CParticleInitializer *p) {});
			modifier->Initialize(psc,keyValues);
			modifier->SetName(name);
			return std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)>(modifier,[](CParticleInitializer *p) {}); // Externally owned (by Lua state), so no delete
		});
		break;
	case pragma::LuaParticleModifierManager::Type::Operator:
		map->AddOperator(name,[name](pragma::CParticleSystemComponent &psc,const std::unordered_map<std::string,std::string> &keyValues) -> std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)> {
			auto &particleModMan = c_game->GetLuaParticleModifierManager();
			auto *modifier = dynamic_cast<CParticleOperator*>(particleModMan.CreateModifier(name));
			if(modifier == nullptr)
				return std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)>(nullptr,[](CParticleOperator *p) {});
			modifier->Initialize(psc,keyValues);
			modifier->SetName(name);
			return std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)>(modifier,[](CParticleOperator *p) {}); // Externally owned (by Lua state), so no delete
		});
		break;
	case pragma::LuaParticleModifierManager::Type::Renderer:
		map->AddRenderer(name,[name](pragma::CParticleSystemComponent &psc,const std::unordered_map<std::string,std::string> &keyValues) -> std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)> {
			auto &particleModMan = c_game->GetLuaParticleModifierManager();
			auto *modifier = dynamic_cast<CParticleRenderer*>(particleModMan.CreateModifier(name));
			if(modifier == nullptr)
				return std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)>(nullptr,[](CParticleRenderer *p) {});
			modifier->Initialize(psc,keyValues);
			modifier->SetName(name);
			return std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)>(modifier,[](CParticleRenderer *p) {}); // Externally owned (by Lua state), so no delete
		});
		break;
	case pragma::LuaParticleModifierManager::Type::Emitter:
		// TODO
		break;
	}
}

static void push_particle_system_definition_data(lua_State *l,const CParticleSystemData &ptSysData)
{
	auto tPtSys = Lua::CreateTable(l);

	for(auto &pair : ptSysData.settings)
	{
		Lua::PushString(l,pair.first);
		Lua::PushString(l,pair.second);
		Lua::SetTableValue(l,tPtSys);
	}

	auto fPushOperators = [l,tPtSys](const std::string &type,const std::vector<CParticleModifierData> &data) {
		Lua::PushString(l,type);
		auto tInitializers = Lua::CreateTable(l);
		int32_t idx = 1;
		for(auto &initializer : data)
		{
			Lua::PushInt(l,idx++);
			auto tInitializer = Lua::CreateTable(l);

			Lua::PushString(l,"operatorType");
			Lua::PushString(l,initializer.name);
			Lua::SetTableValue(l,tInitializer);

			for(auto &pair : initializer.settings)
			{
				Lua::PushString(l,pair.first);
				Lua::PushString(l,pair.second);
				Lua::SetTableValue(l,tInitializer);
			}

			Lua::SetTableValue(l,tInitializers);
		}
		Lua::SetTableValue(l,tPtSys);
	};
	fPushOperators("initializers",ptSysData.initializers);
	fPushOperators("operators",ptSysData.operators);
	fPushOperators("renderers",ptSysData.renderers);

	Lua::PushString(l,"children");
	auto tChildren = Lua::CreateTable(l);
	int32_t idx = 1;
	for(auto &childData : ptSysData.children)
	{
		Lua::PushInt(l,idx++);
		auto tChild = Lua::CreateTable(l);

		Lua::PushString(l,"childName");
		Lua::PushString(l,childData.childName);
		Lua::SetTableValue(l,tChild);

		Lua::PushString(l,"delay");
		Lua::PushNumber(l,childData.delay);
		Lua::SetTableValue(l,tChild);

		Lua::SetTableValue(l,tChildren);
	}
	Lua::SetTableValue(l,tPtSys);
}

void Lua::ParticleSystem::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCParticleSystem = luabind::class_<CParticleSystemHandle,BaseEntityComponentHandle>("ParticleSystemComponent");
	defCParticleSystem.add_static_constant("RENDER_FLAG_NONE",umath::to_integral(pragma::ParticleRenderFlags::None));
	defCParticleSystem.add_static_constant("RENDER_FLAG_BIT_BLOOM",umath::to_integral(pragma::ParticleRenderFlags::Bloom));
	defCParticleSystem.add_static_constant("RENDER_FLAG_BIT_DEPTH_ONLY",umath::to_integral(pragma::ParticleRenderFlags::DepthOnly));
	Lua::register_base_env_particle_system_component_methods<luabind::class_<CParticleSystemHandle,BaseEntityComponentHandle>,CParticleSystemHandle>(l,defCParticleSystem);

	defCParticleSystem.def("Start",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Start();
		}));
	defCParticleSystem.def("Stop",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		Lua::ParticleSystem::Stop(l,hComponent,false);
		}));
	defCParticleSystem.def("Stop",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool bStopImmediately) {
		Lua::ParticleSystem::Stop(l,hComponent,bStopImmediately);
		}));
	defCParticleSystem.def("Die",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Die();
	}));
	defCParticleSystem.def("Die",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float t) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Die(t);
	}));
	defCParticleSystem.def("AddInitializer",static_cast<void(*)(lua_State*,CParticleSystemHandle&,std::string,luabind::object)>([](lua_State *l,CParticleSystemHandle &hComponent,std::string name,luabind::object o) {
		pragma::Lua::check_component(l,hComponent);
		Lua::ParticleSystem::AddInitializer(l,*hComponent,name,o);
		}));
	defCParticleSystem.def("AddOperator",static_cast<void(*)(lua_State*,CParticleSystemHandle&,std::string,luabind::object)>([](lua_State *l,CParticleSystemHandle &hComponent,std::string name,luabind::object o) {
		pragma::Lua::check_component(l,hComponent);
		Lua::ParticleSystem::AddOperator(l,*hComponent,name,o);
		}));
	defCParticleSystem.def("AddRenderer",static_cast<void(*)(lua_State*,CParticleSystemHandle&,std::string,luabind::object)>([](lua_State *l,CParticleSystemHandle &hComponent,std::string name,luabind::object o) {
		pragma::Lua::check_component(l,hComponent);
		Lua::ParticleSystem::AddRenderer(l,*hComponent,name,o);
		}));
	defCParticleSystem.def("RemoveInitializer",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->RemoveInitializer(name);
		}));
	defCParticleSystem.def("RemoveOperator",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->RemoveOperator(name);
		}));
	defCParticleSystem.def("RemoveRenderer",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->RemoveRenderer(name);
		}));
	defCParticleSystem.def("RemoveInitializerByType",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->RemoveInitializersByType(name);
		}));
	defCParticleSystem.def("RemoveOperatorByType",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->RemoveOperatorsByType(name);
		}));
	defCParticleSystem.def("RemoveRendererByType",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->RemoveRenderersByType(name);
		}));
	defCParticleSystem.def("GetInitializers",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto t = Lua::CreateTable(l);
		auto &initializers = hComponent->GetInitializers();
		uint32_t idx = 1;
		for(auto &initializer : initializers)
		{
			Lua::PushInt(l,idx++);
			auto *luaInit = dynamic_cast<CParticleInitializerLua*>(initializer.get());
			if(luaInit)
				luaInit->GetLuaObject().push(l);
			else
				Lua::Push(l,initializer.get());
			Lua::SetTableValue(l,t);
		}
	}));
	defCParticleSystem.def("GetOperators",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto t = Lua::CreateTable(l);
		auto &operators = hComponent->GetOperators();
		uint32_t idx = 1;
		for(auto &op : operators)
		{
			Lua::PushInt(l,idx++);
			auto *luaOp = dynamic_cast<CParticleOperatorLua*>(op.get());
			if(luaOp)
				luaOp->GetLuaObject().push(l);
			else
				Lua::Push(l,op.get());
			Lua::SetTableValue(l,t);
		}
	}));
	defCParticleSystem.def("GetRenderers",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto t = Lua::CreateTable(l);
		auto &renderers = hComponent->GetRenderers();
		uint32_t idx = 1;
		for(auto &renderer : renderers)
		{
			Lua::PushInt(l,idx++);
			auto *luaRenderer = dynamic_cast<CParticleRendererLua*>(renderer.get());
			if(luaRenderer)
				luaRenderer->GetLuaObject().push(l);
			else
				Lua::Push(l,renderer.get());
			Lua::SetTableValue(l,t);
		}
	}));
	defCParticleSystem.def("FindInitializer",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		auto &initializers = hComponent->GetInitializers();
		auto it = std::find_if(initializers.begin(),initializers.end(),[&name](const std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)> &initializer) {
			return ustring::compare(name,initializer->GetName(),false);
		});
		if(it == initializers.end())
			return;
		Lua::Push(l,it->get());
	}));
	defCParticleSystem.def("FindOperator",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		auto &operators = hComponent->GetOperators();
		auto it = std::find_if(operators.begin(),operators.end(),[&name](const std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)> &op) {
			return ustring::compare(name,op->GetName(),false);
			});
		if(it == operators.end())
			return;
		Lua::Push(l,it->get());
		}));
	defCParticleSystem.def("FindRenderer",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		auto &renderers = hComponent->GetRenderers();
		auto it = std::find_if(renderers.begin(),renderers.end(),[&name](const std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)> &renderer) {
			return ustring::compare(name,renderer->GetName(),false);
			});
		if(it == renderers.end())
			return;
		Lua::Push(l,it->get());
		}));
	defCParticleSystem.def("FindInitializerByType",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		auto &initializers = hComponent->GetInitializers();
		auto it = std::find_if(initializers.begin(),initializers.end(),[&name](const std::unique_ptr<CParticleInitializer,void(*)(CParticleInitializer*)> &initializer) {
			return ustring::compare(name,initializer->GetType(),false);
			});
		if(it == initializers.end())
			return;
		Lua::Push(l,it->get());
		}));
	defCParticleSystem.def("FindOperatorByType",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		auto &operators = hComponent->GetOperators();
		auto it = std::find_if(operators.begin(),operators.end(),[&name](const std::unique_ptr<CParticleOperator,void(*)(CParticleOperator*)> &op) {
			return ustring::compare(name,op->GetType(),false);
			});
		if(it == operators.end())
			return;
		Lua::Push(l,it->get());
		}));
	defCParticleSystem.def("FindRendererByType",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		auto &renderers = hComponent->GetRenderers();
		auto it = std::find_if(renderers.begin(),renderers.end(),[&name](const std::unique_ptr<CParticleRenderer,void(*)(CParticleRenderer*)> &renderer) {
			return ustring::compare(name,renderer->GetType(),false);
			});
		if(it == renderers.end())
			return;
		Lua::Push(l,it->get());
		}));
	defCParticleSystem.def("AddChild",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		auto *pt = hComponent.get()->AddChild(name);
		if(pt == nullptr)
			return;
		Lua::Push(l,pt->GetHandle());
		}));
	defCParticleSystem.def("AddChild",static_cast<void(*)(lua_State*,CParticleSystemHandle&,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,CParticleSystemHandle &hChild) {
		pragma::Lua::check_component(l,hComponent);
		pragma::Lua::check_component(l,hChild);
		hComponent.get()->AddChild(*hChild);
		}));
	defCParticleSystem.def("SetNodeTarget",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t,EntityHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t nodeId,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetNodeTarget(nodeId,static_cast<CBaseEntity*>(hEnt.get()));
		}));
	defCParticleSystem.def("SetNodeTarget",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t,const Vector3&)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t nodeId,const Vector3 &pos) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetNodeTarget(nodeId,pos);
		}));
	defCParticleSystem.def("GetNodeCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetNodeCount());
		}));
	defCParticleSystem.def("GetNodePosition",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t nodeId) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Push<Vector3>(l,hComponent->GetNodePosition(nodeId));
		}));
	defCParticleSystem.def("GetNodeTarget",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t nodeId) {
		pragma::Lua::check_component(l,hComponent);
		auto *ent = hComponent->GetNodeTarget(nodeId);
		if(ent == nullptr)
			return;
		ent->GetLuaObject()->push(l);
		}));
	defCParticleSystem.def("SetRemoveOnComplete",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool b) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetRemoveOnComplete(b);
		}));
	defCParticleSystem.def("GetRenderBounds",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &bounds = hComponent->GetRenderBounds();
		Lua::Push<Vector3>(l,bounds.first);
		Lua::Push<Vector3>(l,bounds.second);
		}));
	defCParticleSystem.def("CalcRenderBounds",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto bounds = hComponent->CalcRenderBounds();
		Lua::Push<Vector3>(l,bounds.first);
		Lua::Push<Vector3>(l,bounds.second);
	}));
	defCParticleSystem.def("SetRadius",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float radius) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetRadius(radius);
		}));
	defCParticleSystem.def("GetRadius",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushNumber(l,hComponent->GetRadius());
		}));
	defCParticleSystem.def("GetSimulationTime",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushNumber(l,hComponent->GetSimulationTime());
	}));
	defCParticleSystem.def("SetExtent",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float extent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetExtent(extent);
		}));
	defCParticleSystem.def("GetExtent",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushNumber(l,hComponent->GetExtent());
		}));
	defCParticleSystem.def("SetMaterial",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetMaterial(name.c_str());
		}));
	defCParticleSystem.def("SetMaterial",static_cast<void(*)(lua_State*,CParticleSystemHandle&,Material*)>([](lua_State *l,CParticleSystemHandle &hComponent,Material *mat) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetMaterial(mat);
		}));
	defCParticleSystem.def("GetMaterial",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *mat = hComponent->GetMaterial();
		if(mat == nullptr)
			return;
		Lua::Push<Material*>(l,mat);
		}));
	defCParticleSystem.def("SetOrientationType",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t orientationType) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetOrientationType(static_cast<pragma::CParticleSystemComponent::OrientationType>(orientationType));
		}));
	defCParticleSystem.def("GetOrientationType",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,umath::to_integral(hComponent->GetOrientationType()));
		}));
	defCParticleSystem.def("IsContinuous",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsContinuous());
		}));
	defCParticleSystem.def("SetContinuous",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool b) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetContinuous(b);
		}));
	defCParticleSystem.def("GetRemoveOnComplete",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetRemoveOnComplete());
		}));
	defCParticleSystem.def("GetCastShadows",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetCastShadows());
		}));
	defCParticleSystem.def("SetCastShadows",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool b) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetCastShadows(b);
		}));
	defCParticleSystem.def("SetBloomColorFactor",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const Vector4&)>([](lua_State *l,CParticleSystemHandle &hComponent,const Vector4 &factor) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetBloomColorFactor(factor);
		}));
	defCParticleSystem.def("GetBloomColorFactor",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Push<Vector4>(l,hComponent->GetBloomColorFactor());
	}));
	defCParticleSystem.def("GetEffectiveBloomColorFactor",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto bloomCol = hComponent->GetEffectiveBloomColorFactor();
		if(bloomCol.has_value() == false)
			return;
		Lua::Push<Vector4>(l,*bloomCol);
	}));
	defCParticleSystem.def("IsBloomEnabled",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsBloomEnabled());
	}));
	defCParticleSystem.def("SetColorFactor",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const Vector4&)>([](lua_State *l,CParticleSystemHandle &hComponent,const Vector4 &factor) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetColorFactor(factor);
		}));
	defCParticleSystem.def("GetColorFactor",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Push<Vector4>(l,hComponent->GetColorFactor());
		}));
	defCParticleSystem.def("GetParticleCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetParticleCount());
		}));
	defCParticleSystem.def("GetParticle",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t idx) {
		pragma::Lua::check_component(l,hComponent);
		auto *pt = hComponent->GetParticle(idx);
		if(pt == nullptr)
			return;
		Lua::Push(l,pt);
	}));
	defCParticleSystem.def("GetParticles",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &particles = hComponent->GetParticles();
		auto t = Lua::CreateTable(l);
		uint32_t ptIdx = 1u;
		for(auto &pt : particles)
		{
			Lua::PushInt(l,ptIdx++);
			Lua::Push<CParticle*>(l,const_cast<CParticle*>(&pt));
			Lua::SetTableValue(l,t);
		}
	}));
	defCParticleSystem.def("GetMaxParticleCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetMaxParticleCount());
		}));
	defCParticleSystem.def("IsActive",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsActive());
		}));
	defCParticleSystem.def("GetRenderMode",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetRenderMode());
		}));
	defCParticleSystem.def("SetRenderMode",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t renderMode) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetRenderMode(static_cast<RenderMode>(renderMode));
		}));
	defCParticleSystem.def("SetName",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetParticleSystemName(name);
		}));
	defCParticleSystem.def("GetName",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushString(l,hComponent->GetParticleSystemName());
		}));
	defCParticleSystem.def("IsStatic",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsStatic());
		}));
	defCParticleSystem.def("IsDying",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsDying());
		}));
	defCParticleSystem.def("GetChildren",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &children = hComponent->GetChildren();
		auto t = Lua::CreateTable(l);
		auto idx = 1u;
		for(auto &hChild : children)
		{
			if(hChild.child.expired())
				continue;
			Lua::PushInt(l,idx++);
			Lua::Push(l,hChild);
			Lua::SetTableValue(l,t);
		}
		}));
	defCParticleSystem.def("GetAlphaMode",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,umath::to_integral(hComponent->GetAlphaMode()));
		}));
	defCParticleSystem.def("GetEffectiveAlphaMode",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,umath::to_integral(hComponent->GetEffectiveAlphaMode()));
		}));
	defCParticleSystem.def("SetAlphaMode",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t alphaMode) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetAlphaMode(static_cast<pragma::ParticleAlphaMode>(alphaMode));
		}));
	defCParticleSystem.def("GetEmissionRate",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetEmissionRate());
		}));
	defCParticleSystem.def("SetEmissionRate",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t emissionRate) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetEmissionRate(emissionRate);
		}));
	defCParticleSystem.def("SetNextParticleEmissionCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t count) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetNextParticleEmissionCount(count);
		}));
	defCParticleSystem.def("PauseEmission",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->PauseEmission();
		}));
	defCParticleSystem.def("ResumeEmission",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->ResumeEmission();
		}));
	defCParticleSystem.def("GetLifeTime",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushNumber(l,hComponent->GetLifeTime());
		}));
	defCParticleSystem.def("GetStartTime",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushNumber(l,hComponent->GetStartTime());
	}));
	defCParticleSystem.def("GetSoftParticles",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetSoftParticles());
		}));
	defCParticleSystem.def("SetSoftParticles",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool bSoft) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetSoftParticles(bSoft);
		}));
	defCParticleSystem.def("GetSortParticles",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetSortParticles());
		}));
	defCParticleSystem.def("SetSortParticles",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool bSort) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetSortParticles(bSort);
		}));
	defCParticleSystem.def("GetInitialColor",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Push<Color>(l,hComponent->GetInitialColor());
		}));
	defCParticleSystem.def("SetInitialColor",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const Color&)>([](lua_State *l,CParticleSystemHandle &hComponent,const Color &col) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetInitialColor(col);
		}));
	defCParticleSystem.def("Simulate",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float tDelta) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Simulate(tDelta);
		}));
	defCParticleSystem.def("Render",static_cast<void(*)(lua_State*,CParticleSystemHandle&,std::shared_ptr<prosper::ICommandBuffer>&,pragma::rendering::RasterizationRenderer&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,std::shared_ptr<prosper::ICommandBuffer> &drawCmd,pragma::rendering::RasterizationRenderer &renderer,uint32_t renderFlags) {
		pragma::Lua::check_component(l,hComponent);
		if(drawCmd->IsPrimary() == false)
			return;
		hComponent->Render(std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(drawCmd),renderer,static_cast<pragma::ParticleRenderFlags>(renderFlags));
		}));
	defCParticleSystem.def("GetRenderParticleCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetRenderParticleCount());
		}));
	defCParticleSystem.def("IsActiveOrPaused",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsActiveOrPaused());
		}));
	defCParticleSystem.def("GetParticleBufferIndexFromParticleIndex",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t ptIdx) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->TranslateParticleIndex(ptIdx));
	}));
	defCParticleSystem.def("GetParticleIndexFromParticleBufferIndex",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t ptBufIdx) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->TranslateBufferIndex(ptBufIdx));
	}));
	defCParticleSystem.def("IsEmissionPaused",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsEmissionPaused());
		}));
	defCParticleSystem.def("SetParent",static_cast<void(*)(lua_State*,CParticleSystemHandle&,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,CParticleSystemHandle &hParent) {
		pragma::Lua::check_component(l,hComponent);
		pragma::Lua::check_component(l,hParent);
		hComponent->SetParent(hParent.get());
		}));
	defCParticleSystem.def("RemoveChild",static_cast<void(*)(lua_State*,CParticleSystemHandle&,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,CParticleSystemHandle &hChild) {
		pragma::Lua::check_component(l,hComponent);
		pragma::Lua::check_component(l,hChild);
		hComponent->RemoveChild(hChild.get());
		}));
	defCParticleSystem.def("HasChild",static_cast<void(*)(lua_State*,CParticleSystemHandle&,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,CParticleSystemHandle &hChild) {
		pragma::Lua::check_component(l,hComponent);
		pragma::Lua::check_component(l,hChild);
		Lua::PushBool(l,hComponent->HasChild(*hChild));
		}));
	defCParticleSystem.def("GetParent",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *hParent = hComponent->GetParent();
		if(hParent == nullptr)
			return;
		Lua::Push(l,hParent->GetHandle());
		}));
	defCParticleSystem.def("GetParticleBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &particleBuffer = hComponent->GetParticleBuffer();
		if(particleBuffer == nullptr)
			return;
		Lua::Push(l,particleBuffer);
		}));
	defCParticleSystem.def("GetParticleAnimationBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &animBuffer = hComponent->GetParticleAnimationBuffer();
		if(animBuffer == nullptr)
			return;
		Lua::Push(l,animBuffer);
		}));
	defCParticleSystem.def("GetAnimationSpriteSheetBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &spriteSheetBuffer = hComponent->GetSpriteSheetBuffer();
		if(spriteSheetBuffer == nullptr)
			return;
		Lua::Push(l,spriteSheetBuffer);
		}));
	defCParticleSystem.def("GetSpriteSheetAnimation",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *spriteSheetBuffer = hComponent->GetSpriteSheetAnimation();
		if(spriteSheetBuffer == nullptr)
			return;
		Lua::Push(l,spriteSheetBuffer);
	}));
	defCParticleSystem.def("GetAnimationDescriptorSet",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &animDescSetGroup = hComponent->GetAnimationDescriptorSetGroup();
		if(animDescSetGroup == nullptr)
			return;
		Lua::Push(l,animDescSetGroup);
		}));
	defCParticleSystem.def("IsAnimated",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsAnimated());
		}));
	defCParticleSystem.def("ShouldAutoSimulate",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->ShouldAutoSimulate());
		}));
	defCParticleSystem.def("SetAutoSimulate",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool autoSimulate) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetAutoSimulate(autoSimulate);
	}));
	defCParticleSystem.def("SetAutoSimulate",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool autoSimulate) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetAutoSimulate(autoSimulate);
	}));
	defCParticleSystem.def("InitializeFromParticleSystemDefinition",static_cast<void(*)(lua_State*,CParticleSystemHandle&,const std::string&)>([](lua_State *l,CParticleSystemHandle &hComponent,const std::string &name) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->SetupParticleSystem(name));
	}));
	defCParticleSystem.def("SetControlPointEntity",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t,EntityHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t cpIdx,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetControlPointEntity(cpIdx,static_cast<CBaseEntity&>(*hEnt.get()));
	}));
	defCParticleSystem.def("SetControlPointPosition",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t,const Vector3&)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t cpIdx,const Vector3 &pos) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetControlPointPosition(cpIdx,pos);
	}));
	defCParticleSystem.def("SetControlPointRotation",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t,const Quat&)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t cpIdx,const Quat &rot) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetControlPointRotation(cpIdx,rot);
	}));
	defCParticleSystem.def("SetControlPointPose",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t,const umath::Transform&)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t cpIdx,const umath::Transform &pose) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetControlPointPose(cpIdx,pose);
	}));
	defCParticleSystem.def("SetControlPointPose",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t,const umath::Transform&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t cpIdx,const umath::Transform &pose,float timeStamp) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetControlPointPose(cpIdx,pose,&timeStamp);
	}));
	defCParticleSystem.def("GetControlPointEntity",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t cpIdx) {
		pragma::Lua::check_component(l,hComponent);
		auto *ent = hComponent->GetControlPointEntity(cpIdx);
		if(ent == nullptr)
			return;
		ent->GetLuaObject()->push(l);
	}));
	defCParticleSystem.def("GetPrevControlPointPose",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t cpIdx) {
		pragma::Lua::check_component(l,hComponent);
		float timeStamp;
		auto pose = hComponent->GetPrevControlPointPose(cpIdx,&timeStamp);
		if(pose.has_value() == false)
			return;
		Lua::Push(l,*pose);
		Lua::PushNumber(l,timeStamp);
	}));
	defCParticleSystem.def("GetControlPointPose",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t cpIdx) {
		pragma::Lua::check_component(l,hComponent);
		float timeStamp;
		auto pose = hComponent->GetControlPointPose(cpIdx,&timeStamp);
		if(pose.has_value() == false)
			return;
		Lua::Push(l,*pose);
		Lua::PushNumber(l,timeStamp);
	}));
	defCParticleSystem.def("GetControlPointPose",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t,float)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t cpIdx,float timeStamp) {
		pragma::Lua::check_component(l,hComponent);
		auto pose = hComponent->GetControlPointPose(cpIdx,timeStamp);
		if(pose.has_value() == false)
			return;
		Lua::Push(l,*pose);
	}));
	defCParticleSystem.def("GenerateModel",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto mdl = hComponent->GenerateModel();
		if(mdl == nullptr)
			return;
		Lua::Push(l,mdl);
	}));
#if 0
	defCParticleSystem.def("GetAnimationFrameCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *animData = hComponent->GetRenderParticleAnimationStartData();
		if(animData == nullptr)
			return;
		Lua::PushInt(l,animData->frames);
	}));
	defCParticleSystem.def("GetAnimationFPS",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *animData = hComponent->GetAnimationData();
		if(animData == nullptr)
			return;
		Lua::PushInt(l,animData->fps);
	}));
#endif
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_ALIGNED",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Aligned));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_UPRIGHT",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Upright));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_STATIC",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Static));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_WORLD",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::World));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_BILLBOARD",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Billboard));

	defCParticleSystem.add_static_constant("ALPHA_MODE_ADDITIVE",umath::to_integral(pragma::ParticleAlphaMode::Additive));
	defCParticleSystem.add_static_constant("ALPHA_MODE_ADDITIVE_BY_COLOR",umath::to_integral(pragma::ParticleAlphaMode::AdditiveByColor));
	defCParticleSystem.add_static_constant("ALPHA_MODE_OPAQUE",umath::to_integral(pragma::ParticleAlphaMode::Opaque));
	defCParticleSystem.add_static_constant("ALPHA_MODE_MASKED",umath::to_integral(pragma::ParticleAlphaMode::Masked));
	defCParticleSystem.add_static_constant("ALPHA_MODE_TRANSLUCENT",umath::to_integral(pragma::ParticleAlphaMode::Translucent));
	defCParticleSystem.add_static_constant("ALPHA_MODE_PREMULTIPLIED",umath::to_integral(pragma::ParticleAlphaMode::Premultiplied));
	defCParticleSystem.add_static_constant("ALPHA_MODE_COUNT",umath::to_integral(pragma::ParticleAlphaMode::Count));
	ParticleSystemModifier::register_particle_class(defCParticleSystem);
	ParticleSystemModifier::register_modifier_class(defCParticleSystem);
	defCParticleSystem.scope[luabind::def("find_particle_system_file",static_cast<void(*)(lua_State*,const std::string&)>([](lua_State *l,const std::string &ptSystemName) {
		std::string ptName = Lua::CheckString(l,1);
		auto ptFileName = pragma::CParticleSystemComponent::FindParticleSystemFile(ptName);
		if(ptFileName.has_value() == false)
			return;
		Lua::PushString(l,"particles/" +*ptFileName +".wpt");
	}))];
	defCParticleSystem.scope[luabind::def("get_particle_system_definitions",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		auto &ptSystemCache = pragma::CParticleSystemComponent::GetCachedParticleSystemData();
		auto t = Lua::CreateTable(l);
		for(auto &pair : ptSystemCache)
		{
			Lua::PushString(l,pair.first);
			push_particle_system_definition_data(l,*pair.second);
			Lua::SetTableValue(l,t);
		}
	}))];
	defCParticleSystem.scope[luabind::def("get_particle_system_definition",static_cast<void(*)(lua_State*,const std::string&)>([](lua_State *l,const std::string &ptSystemName) {
		auto lPtSystemName = ptSystemName;
		ustring::to_lower(lPtSystemName);
		auto &ptSystemCache = pragma::CParticleSystemComponent::GetCachedParticleSystemData();
		auto it = ptSystemCache.find(lPtSystemName);
		if(it == ptSystemCache.end())
			return;
		push_particle_system_definition_data(l,*it->second);
	}))];
	defCParticleSystem.scope[luabind::def("generate_model",static_cast<void(*)(lua_State*,luabind::object)>([](lua_State *l,luabind::object o) {
		int32_t t = 1;
		Lua::CheckTable(l,t);
		auto n = Lua::GetObjectLength(l,t);
		std::vector<const pragma::CParticleSystemComponent*> particleSystems {};
		particleSystems.reserve(n);
		for(auto i=decltype(n){0u};i<n;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,t);
			auto &hPtC = Lua::Check<CParticleSystemHandle>(l,-1);
			pragma::Lua::check_component(l,hPtC);
			particleSystems.push_back(hPtC.get());

			Lua::Pop(l,1);
		}
		auto mdl = pragma::CParticleSystemComponent::GenerateModel(static_cast<CGame&>(*engine->GetNetworkState(l)->GetGameState()),particleSystems);
		if(mdl == nullptr)
			return;
		Lua::Push(l,mdl);
	}))];
	defCParticleSystem.scope[luabind::def("read_header_data",static_cast<void(*)(lua_State*,const std::string&)>([](lua_State *l,const std::string &name) {
		auto fileHeader = pragma::CParticleSystemComponent::ReadHeader(*engine->GetNetworkState(l),name);
		if(fileHeader.has_value() == false)
			return;
		auto t = Lua::CreateTable(l);

		Lua::PushString(l,"version");
		Lua::PushInt(l,fileHeader->version);
		Lua::SetTableValue(l,t);

		Lua::PushString(l,"numParticles");
		Lua::PushInt(l,fileHeader->numParticles);
		Lua::SetTableValue(l,t);

		Lua::PushString(l,"particleSystemNames");
		auto tNames = Lua::CreateTable(l);
		for(auto i=decltype(fileHeader->particleSystemNames.size()){0};i<fileHeader->particleSystemNames.size();++i)
		{
			auto &name = fileHeader->particleSystemNames.at(i);
			Lua::PushInt(l,i +1);
			Lua::PushString(l,name);
			Lua::SetTableValue(l,tNames);
		}
		Lua::SetTableValue(l,t);

		Lua::PushString(l,"particleSystemOffsets");
		auto tOffsets = Lua::CreateTable(l);
		for(auto i=decltype(fileHeader->particleSystemOffsets.size()){0};i<fileHeader->particleSystemOffsets.size();++i)
		{
			auto offset = fileHeader->particleSystemOffsets.at(i);
			Lua::PushInt(l,i +1);
			Lua::PushInt(l,offset);
			Lua::SetTableValue(l,tOffsets);
		}
		Lua::SetTableValue(l,t);
	}))];
	defCParticleSystem.scope[luabind::def("register_initializer",static_cast<void(*)(lua_State*,const std::string&,luabind::object)>([](lua_State *l,const std::string &name,luabind::object oClass) {
		register_particle_modifier(l,pragma::LuaParticleModifierManager::Type::Initializer,name,oClass);
	}))];
	defCParticleSystem.scope[luabind::def("register_operator",static_cast<void(*)(lua_State*,const std::string&,luabind::object)>([](lua_State *l,const std::string &name,luabind::object oClass) {
		register_particle_modifier(l,pragma::LuaParticleModifierManager::Type::Operator,name,oClass);
	}))];
	defCParticleSystem.scope[luabind::def("register_renderer",static_cast<void(*)(lua_State*,const std::string&,luabind::object)>([](lua_State *l,const std::string &name,luabind::object oClass) {
		register_particle_modifier(l,pragma::LuaParticleModifierManager::Type::Renderer,name,oClass);
	}))];
	defCParticleSystem.scope[luabind::def("register_emitter",static_cast<void(*)(lua_State*,const std::string&,luabind::object)>([](lua_State *l,const std::string &name,luabind::object oClass) {
		register_particle_modifier(l,pragma::LuaParticleModifierManager::Type::Emitter,name,oClass);
	}))];

	defCParticleSystem.scope[luabind::def("get_registered_initializers",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		auto t = Lua::CreateTable(l);
		auto *map = GetParticleModifierMap();
		if(map == nullptr)
			return;
		uint32_t idx = 1;
		for(auto &pair : map->GetInitializers())
		{
			Lua::PushInt(l,idx++);
			Lua::PushString(l,pair.first);
			Lua::SetTableValue(l,t);
		}
	}))];
	defCParticleSystem.scope[luabind::def("get_registered_operators",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		auto t = Lua::CreateTable(l);
		auto *map = GetParticleModifierMap();
		if(map == nullptr)
			return;
		uint32_t idx = 1;
		for(auto &pair : map->GetOperators())
		{
			Lua::PushInt(l,idx++);
			Lua::PushString(l,pair.first);
			Lua::SetTableValue(l,t);
		}
	}))];
	defCParticleSystem.scope[luabind::def("get_registered_renderers",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		auto t = Lua::CreateTable(l);
		auto *map = GetParticleModifierMap();
		if(map == nullptr)
			return;
		uint32_t idx = 1;
		for(auto &pair : map->GetRenderers())
		{
			Lua::PushInt(l,idx++);
			Lua::PushString(l,pair.first);
			Lua::SetTableValue(l,t);
		}
	}))];
#if 0
	// TODO
	defCParticleSystem.scope[luabind::def("get_registered_emitters",static_cast<void(*)(lua_State*,const std::string&,luabind::object)>([](lua_State *l,const std::string &name,luabind::object oClass) {
		auto t = Lua::CreateTable(l);
		auto *map = GetParticleModifierMap();
		if(map == nullptr)
			return;
		uint32_t idx = 1;
		for(auto &pair : map->GetEmitters())
		{
			Lua::PushInt(l,idx++);
			Lua::PushString(l,pair.first);
			Lua::SetTableValue(l,t);
		}
	}))];
#endif
	entsMod[defCParticleSystem];
}
#pragma optimize("",on)
