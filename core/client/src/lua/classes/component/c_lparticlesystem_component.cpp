#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/lua/classes/c_lparticle_modifiers.hpp"
#include "pragma/particlesystem/initializers/c_particle_initializer_lua.hpp"
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

void Lua::ParticleSystem::register_class(lua_State *l,luabind::module_ &entsMod)
{
	auto defCParticleSystem = luabind::class_<CParticleSystemHandle,BaseEntityComponentHandle>("ParticleSystemComponent");
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
	defCParticleSystem.def("SetRadius",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float radius) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetRadius(radius);
		}));
	defCParticleSystem.def("GetRadius",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushNumber(l,hComponent->GetRadius());
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
	defCParticleSystem.def("GetBloomScale",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetBloomScale());
		}));
	defCParticleSystem.def("SetBloomScale",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float scale) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetBloomScale(scale);
		}));
	defCParticleSystem.def("GetIntensity",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetIntensity());
		}));
	defCParticleSystem.def("SetIntensity",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float intensity) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetIntensity(intensity);
		}));
	defCParticleSystem.def("GetParticleCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetParticleCount());
		}));
	defCParticleSystem.def("GetMaxParticleCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetMaxParticleCount());
		}));
	defCParticleSystem.def("Die",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Die();
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
	defCParticleSystem.def("GetChildren",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &children = hComponent->GetChildren();
		auto t = Lua::CreateTable(l);
		auto idx = 1u;
		for(auto &hChild : children)
		{
			if(hChild.expired())
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
	defCParticleSystem.def("SetAlphaMode",static_cast<void(*)(lua_State*,CParticleSystemHandle&,uint32_t)>([](lua_State *l,CParticleSystemHandle &hComponent,uint32_t alphaMode) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetAlphaMode(static_cast<pragma::AlphaMode>(alphaMode));
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
	defCParticleSystem.def("GetSoftParticles",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->GetSoftParticles());
		}));
	defCParticleSystem.def("SetSoftParticles",static_cast<void(*)(lua_State*,CParticleSystemHandle&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,bool bSoft) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetSoftParticles(bSoft);
		}));
	defCParticleSystem.def("Simulate",static_cast<void(*)(lua_State*,CParticleSystemHandle&,float)>([](lua_State *l,CParticleSystemHandle &hComponent,float tDelta) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->Simulate(tDelta);
		}));
	defCParticleSystem.def("Render",static_cast<void(*)(lua_State*,CParticleSystemHandle&,std::shared_ptr<prosper::CommandBuffer>&,pragma::rendering::RasterizationRenderer&,bool)>([](lua_State *l,CParticleSystemHandle &hComponent,std::shared_ptr<prosper::CommandBuffer> &drawCmd,pragma::rendering::RasterizationRenderer &renderer,bool bBloom) {
		pragma::Lua::check_component(l,hComponent);
		if(drawCmd->IsPrimary() == false)
			return;
		hComponent->Render(std::static_pointer_cast<prosper::PrimaryCommandBuffer>(drawCmd),renderer,bBloom);
		}));
	defCParticleSystem.def("GetRenderParticleCount",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetRenderParticleCount());
		}));
	defCParticleSystem.def("IsActiveOrPaused",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushBool(l,hComponent->IsActiveOrPaused());
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
	defCParticleSystem.def("GetVertexBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &vertexBuffer = hComponent->GetVertexBuffer();
		if(vertexBuffer == nullptr)
			return;
		Lua::Push(l,vertexBuffer);
		}));
	defCParticleSystem.def("GetParticleBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &particleBuffer = hComponent->GetParticleBuffer();
		if(particleBuffer == nullptr)
			return;
		Lua::Push(l,particleBuffer);
		}));
	defCParticleSystem.def("GetAnimationStartBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &animStartBuffer = hComponent->GetAnimationStartBuffer();
		if(animStartBuffer == nullptr)
			return;
		Lua::Push(l,animStartBuffer);
		}));
	defCParticleSystem.def("GetAnimationBuffer",static_cast<void(*)(lua_State*,CParticleSystemHandle&)>([](lua_State *l,CParticleSystemHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto &animBuffer = hComponent->GetAnimationBuffer();
		if(animBuffer == nullptr)
			return;
		Lua::Push(l,animBuffer);
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
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_ALIGNED",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Aligned));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_UPRIGHT",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Upright));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_STATIC",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Static));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_WORLD",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::World));
	defCParticleSystem.add_static_constant("ORIENTATION_TYPE_BILLBOARD",umath::to_integral(pragma::CParticleSystemComponent::OrientationType::Billboard));

	defCParticleSystem.add_static_constant("ALPHA_MODE_ADDITIVE",umath::to_integral(pragma::AlphaMode::Additive));
	defCParticleSystem.add_static_constant("ALPHA_MODE_ADDITIVE_FULL",umath::to_integral(pragma::AlphaMode::AdditiveFull));
	defCParticleSystem.add_static_constant("ALPHA_MODE_OPAQUE",umath::to_integral(pragma::AlphaMode::Opaque));
	defCParticleSystem.add_static_constant("ALPHA_MODE_MASKED",umath::to_integral(pragma::AlphaMode::Masked));
	defCParticleSystem.add_static_constant("ALPHA_MODE_TRANSLUCENT",umath::to_integral(pragma::AlphaMode::Translucent));
	defCParticleSystem.add_static_constant("ALPHA_MODE_PREMULTIPLIED",umath::to_integral(pragma::AlphaMode::Premultiplied));
	defCParticleSystem.add_static_constant("ALPHA_MODE_COUNT",umath::to_integral(pragma::AlphaMode::Count));
	ParticleSystemModifier::register_particle_class(defCParticleSystem);
	ParticleSystemModifier::register_modifier_class(defCParticleSystem);
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
	entsMod[defCParticleSystem];
}
#pragma optimize("",on)
