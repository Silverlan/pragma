/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/lua/libraries/c_lengine.h"
#include "pragma/input/inputhelper.h"
#include "cmaterialmanager.h"
#include "luasystem.h"
#include "pragma/c_engine.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/file_formats/wmd_load.h"
#include "pragma/lua/libraries/c_lengine.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <wgui/fontmanager.h>
#include "pragma/lua/classes/c_ldef_fontinfo.h"
#include <texturemanager/texturemanager.h>
#include "textureinfo.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include <pragma/lua/libraries/lengine.h>
#include <pragma/lua/libraries/lfile.h>
#include <pragma/lua/lua_entity_component.hpp>
#include <image/prosper_render_target.hpp>
#include <pragma/entities/environment/effects/particlesystemdata.h>
#include <prosper_window.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

std::shared_ptr<const FontInfo> Lua::engine::create_font(lua_State *l,const std::string &identifier,const std::string &font,uint32_t size,bool reload) {return FontManager::LoadFont(identifier.c_str(),font.c_str(),size,reload);}
std::shared_ptr<const FontInfo> Lua::engine::create_font(lua_State *l,const std::string &identifier,const std::string &font,uint32_t size) {return create_font(l,identifier,font,size,false);}
std::shared_ptr<const FontInfo> Lua::engine::get_font(lua_State *l,const std::string &identifier) {return FontManager::GetFont(identifier);}

void Lua::engine::register_library(lua_State *l)
{
	auto modEngine = luabind::module_(l,"engine");
	modEngine[
		luabind::def("create_font",static_cast<std::shared_ptr<const FontInfo>(*)(lua_State*,const std::string&,const std::string&,uint32_t,bool)>(Lua::engine::create_font)),
		luabind::def("create_font",static_cast<std::shared_ptr<const FontInfo>(*)(lua_State*,const std::string&,const std::string&,uint32_t)>(Lua::engine::create_font)),
		luabind::def("get_font",Lua::engine::get_font),
		luabind::def("set_fixed_frame_delta_time_interpretation",Lua::engine::set_fixed_frame_delta_time_interpretation),
		luabind::def("clear_fixed_frame_delta_time_interpretation",Lua::engine::clear_fixed_frame_delta_time_interpretation),
		luabind::def("set_tick_delta_time_tied_to_frame_rate",Lua::engine::set_tick_delta_time_tied_to_frame_rate),
		luabind::def("get_window_resolution",Lua::engine::get_window_resolution),
		luabind::def("get_render_resolution",Lua::engine::get_render_resolution),
		luabind::def("get_staging_render_target",Lua::engine::get_staging_render_target),
		luabind::def("set_record_console_output",Lua::engine::set_record_console_output),
		luabind::def("get_tick_count",&Lua::engine::GetTickCount),
		luabind::def("shutdown",&Lua::engine::exit),
		luabind::def("get_working_directory",Lua::engine::get_working_directory),
		luabind::def("get_current_frame_index",&Lua::engine::get_current_frame_index)
	];
}

Vector2i Lua::engine::get_text_size(lua_State *l,const std::string &text,const std::string &font)
{
	auto info = FontManager::GetFont(font);
	if(info == nullptr)
		return {0,0};
	int w = 0;
	int h = 0;
	FontManager::GetTextSize(text.c_str(),0u,info.get(),&w,&h);
	return Vector2i{w,h};
}

Vector2i Lua::engine::get_text_size(lua_State *l,const std::string &text,const FontInfo &font)
{
	int w = 0;
	int h = 0;
	FontManager::GetTextSize(text.c_str(),0u,&font,&w,&h);
	return Vector2i{w,h};
}

void Lua::engine::bind_key(lua_State *l,const std::string &key,const std::string &cmd)
{
	short c;
	if(!StringToKey(key,&c))
		return;
	c_engine->MapKey(c,cmd);
	return;
}

void Lua::engine::bind_key(lua_State *l,const std::string &key,luabind::object function)
{
	Lua::CheckType(function,Lua::Type::Function);
	short c;
	if(!StringToKey(key,&c))
		return;
	c_engine->MapKey(c,function);
}

void Lua::engine::unbind_key(const std::string &key)
{
	short c;
	if(!StringToKey(key,&c))
		return;
	c_engine->UnmapKey(c);
}
void Lua::engine::precache_material(lua_State *l,const std::string &mat) {client->LoadMaterial(mat.c_str());}

void Lua::engine::precache_model(lua_State *l,const std::string &mdl)
{
	auto *nw = c_engine->GetNetworkState(l);
	FWMD wmd(nw->GetGameState());
	wmd.Load<CModel,CModelMesh,CModelSubMesh>(nw->GetGameState(),mdl.c_str(),[](const std::string &path,bool bReload) -> Material* {
		return client->LoadMaterial(path,bReload);	
	},[](const std::string &path) -> std::shared_ptr<Model> {
		return c_game->LoadModel(path);
	});
}

std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua_State *l,const std::string &name,TextureLoadFlags loadFlags)
{
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = loadFlags;
	std::shared_ptr<void> tex = nullptr;
	auto result = static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().Load(c_engine->GetRenderContext(),name,loadInfo,&tex);
	if(result == false || tex == nullptr || std::static_pointer_cast<Texture>(tex)->HasValidVkTexture() == false)
		return nullptr;
	return std::static_pointer_cast<Texture>(tex)->GetVkTexture();
}
std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua_State *l,const std::string &name) {return load_texture(l,name,TextureLoadFlags::None);}

std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua_State *l,const LFile &file,const std::string &cacheName,TextureLoadFlags loadFlags)
{
	auto *lf = Lua::CheckFile(l,1);
	if(lf == nullptr)
		return nullptr;
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = loadFlags;
	loadInfo.flags |= TextureLoadFlags::DontCache;
	std::shared_ptr<void> tex = nullptr;
	auto result = static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().Load(c_engine->GetRenderContext(),cacheName,lf->GetHandle(),loadInfo,&tex);
	if(result == false || tex == nullptr || std::static_pointer_cast<Texture>(tex)->HasValidVkTexture() == false)
		return nullptr;
	return std::static_pointer_cast<Texture>(tex)->GetVkTexture();
}
std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua_State *l,const LFile &file,const std::string &cacheName) {return load_texture(l,file,cacheName,TextureLoadFlags::None);}
std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua_State *l,const LFile &file,TextureLoadFlags loadFlags)
{
	auto *lf = Lua::CheckFile(l,1);
	if(lf == nullptr)
		return nullptr;
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = loadFlags;
	std::shared_ptr<void> tex = nullptr;
	auto result = static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().Load(c_engine->GetRenderContext(),"",lf->GetHandle(),loadInfo,&tex);
	if(result == false || tex == nullptr || std::static_pointer_cast<Texture>(tex)->HasValidVkTexture() == false)
		return nullptr;
	return std::static_pointer_cast<Texture>(tex)->GetVkTexture();
}
std::shared_ptr<prosper::Texture> Lua::engine::load_texture(lua_State *l,const LFile &file) {return load_texture(l,file,TextureLoadFlags::None);}

Material *Lua::engine::load_material(lua_State *l,const std::string &mat,bool reload,bool loadInstantly) {return client->LoadMaterial(mat,loadInstantly,reload);}
Material *Lua::engine::load_material(lua_State *l,const std::string &mat,bool reload) {return load_material(l,mat,reload,false);}
Material *Lua::engine::load_material(lua_State *l,const std::string &mat) {return load_material(l,mat,false,false);}

Material *Lua::engine::get_error_material() {return client->GetMaterialManager().GetErrorMaterial();}
void Lua::engine::clear_unused_materials() {client->GetMaterialManager().ClearUnused();}

Material *Lua::engine::create_material(const std::string &identifier,const std::string &shader) {return client->CreateMaterial(identifier,shader);;}
Material *Lua::engine::create_material(const std::string &shader) {return client->CreateMaterial(shader);}
Material *Lua::engine::get_material(const std::string &identifier) {return client->GetMaterialManager().FindMaterial(identifier);}

int Lua::engine::create_particle_system(lua_State *l)
{
	std::unordered_map<std::string,std::string> values;
	std::vector<std::string> children;
	pragma::CParticleSystemComponent *particle;
	auto bRecordKeyvalues = false;
	if(Lua::IsSet(l,3))
		bRecordKeyvalues = Lua::CheckBool(l,3);
	if(Lua::IsString(l,1))
	{
		std::string name = Lua::CheckString(l,1);
		pragma::CParticleSystemComponent *parent = NULL;
		if(Lua::IsSet(l,2))
		{
			auto &hParent = Lua::Check<pragma::CParticleSystemComponent>(l,2);
			parent = &hParent;
		}
		particle = pragma::CParticleSystemComponent::Create(name,parent,bRecordKeyvalues);
	}
	else
	{
		struct ParticleData
		{
			ParticleData(const std::string &n,const luabind::object &t)
				: name(n),table(t)
			{}
			std::string name;
			luabind::object table;
		};
		std::vector<std::unique_ptr<ParticleData>> initializers;
		std::vector<std::unique_ptr<ParticleData>> operators;
		std::vector<std::unique_ptr<ParticleData>> renderers;

		Lua::CheckTable(l,1);
		Lua::PushNil(l);
		while(Lua::GetNextPair(l,1) != 0)
		{
			Lua::PushValue(l,-2);
			std::string key = Lua::ToString(l,-3);
			if(!Lua::IsTable(l,-2))
			{
				std::string val = Lua::ToString(l,-2);
				StringToLower(key);
				values[key] = val;
				Lua::RemoveValue(l,-3);
				Lua::RemoveValue(l,-2);
			}
			else if(key == "initializers" || key == "operators" || key == "renderers")
			{
				uint8_t type = (key == "initializers") ? 0 : ((key == "operators") ? 1 : 2);
				Lua::CheckTable(l,-2);
				auto t = Lua::GetStackTop(l) -1;
				Lua::PushNil(l); /* 1 */
				while(Lua::GetNextPair(l,t) != 0) /* 2 */
				{
					Lua::PushValue(l,-2); /* 3 */
					const int8_t k = -3;
					const int8_t v = -2;
					auto *name = Lua::ToString(l,k);
					Lua::CheckTable(l,v);
					auto tObj = luabind::object(luabind::from_stack(l,v));
					if(type == 0)
						initializers.push_back(std::make_unique<ParticleData>(name,tObj));
					else if(type == 1)
						operators.push_back(std::make_unique<ParticleData>(name,tObj));
					else
						renderers.push_back(std::make_unique<ParticleData>(name,tObj));
					Lua::RemoveValue(l,k); /* 2 */
					Lua::RemoveValue(l,v); /* 1 */
				} /* 0 */
				Lua::RemoveValue(l,-3);
				Lua::RemoveValue(l,-2);
			}
			else if(key == "children")
			{
				int tchildren = Lua::GetStackTop(l) -1;
				Lua::PushNil(l);
				while(Lua::GetNextPair(l,tchildren) != 0)
				{
					if(!Lua::IsTable(l,-1))
					{
						std::string child = Lua::ToString(l,-1);
						StringToLower(child);
						children.push_back(child);
						Lua::RemoveValue(l,-1);
					}
					else
						Lua::Pop(l,1);
				}
				Lua::RemoveValue(l,-3);
				Lua::RemoveValue(l,-2);
			}
			else
				Lua::Pop(l,2);
		}
		pragma::CParticleSystemComponent *parent = NULL;
		if(Lua::IsSet(l,2))
		{
			auto &hParent = Lua::Check<pragma::CParticleSystemComponent>(l,2);
			parent = &hParent;
		}
		particle = pragma::CParticleSystemComponent::Create(values,parent,bRecordKeyvalues);
		if(particle != nullptr)
		{
			particle->PushLuaObject(l); /* 1 */
			
			for(auto &initializer : initializers)
			{
				Lua::PushString(l,initializer->name); /* 2 */
				initializer->table.push(l); /* 3 */
				Lua::ParticleSystem::AddInitializer(l,*particle,initializer->name,initializer->table);
				Lua::Pop(l,2); /* 1 */
			}
			for(auto &op : operators)
			{
				Lua::PushString(l,op->name); /* 2 */
				op->table.push(l); /* 3 */
				Lua::ParticleSystem::AddOperator(l,*particle,op->name,op->table);
				Lua::Pop(l,2); /* 1 */
			}
			for(auto &renderer : renderers)
			{
				Lua::PushString(l,renderer->name); /* 2 */
				renderer->table.push(l); /* 3 */
				Lua::ParticleSystem::AddRenderer(l,*particle,renderer->name,renderer->table);
				Lua::Pop(l,2); /* 1 */
			}

			Lua::Pop(l,1); /* 0 */
		}
	}
	if(particle == NULL)
		return 0;
	for(unsigned int i=0;i<children.size();i++)
		pragma::CParticleSystemComponent::Create(children[i],particle,bRecordKeyvalues);
	particle->PushLuaObject(l);
	return 1;
}

bool Lua::engine::precache_particle_system(lua_State *l,const std::string &particle,bool reload) {return pragma::CParticleSystemComponent::Precache(particle,reload);}
bool Lua::engine::precache_particle_system(lua_State *l,const std::string &particle) {return precache_particle_system(l,particle,false);}

int Lua::engine::save_particle_system(lua_State *l)
{
	std::string name = luaL_checkstring(l,1);
	Lua::CheckTable(l,2);
	name = pragma::asset::get_normalized_path(name,pragma::asset::Type::ParticleSystem);
	std::string rootPath;
	if(Lua::file::validate_write_operation(l,name,rootPath) == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}

	{
		auto t = 2;

		Lua::PushInt(l,1);
		Lua::GetTableValue(l,t);
		auto bParticleSystem = Lua::IsType<::util::WeakHandle<pragma::CParticleSystemComponent>>(l,-1);
		Lua::Pop(l,1);
		if(bParticleSystem)
		{
			auto numParticleSystems = Lua::GetObjectLength(l,t);
			std::vector<pragma::CParticleSystemComponent*> particleSystems;
			particleSystems.reserve(numParticleSystems);
			std::function<void(const pragma::CParticleSystemComponent&)> fIncludeChildren = nullptr;
			fIncludeChildren = [&particleSystems,&fIncludeChildren](const pragma::CParticleSystemComponent &ps) {
				auto &children = ps.GetChildren();
				particleSystems.reserve(particleSystems.size() +children.size());
				for(auto &hChild : children)
				{
					if(hChild.child.expired())
						continue;
					auto *ps = hChild.child.get();
					if(ps->IsRecordingKeyValues() == false)
					{
						Con::cwar<<"WARNING: Cannot save particle system '"<<ps->GetParticleSystemName()<<"', which wasn't created with the \"record key-values\" flag set! Skipping..."<<Con::endl;
						return;
					}
					particleSystems.push_back(hChild.child.get());
					fIncludeChildren(*hChild.child.get());
				}
			};
			for(auto i=decltype(numParticleSystems){0u};i<numParticleSystems;++i)
			{
				Lua::PushInt(l,i +1u);
				Lua::GetTableValue(l,t);
				auto &ps = Lua::Check<pragma::CParticleSystemComponent>(l,-1);
				if(ps.IsRecordingKeyValues() == false)
					Con::cwar<<"WARNING: Cannot save particle system '"<<ps.GetParticleSystemName()<<"', which wasn't created with the \"record key-values\" flag set! Skipping..."<<Con::endl;
				else
				{
					particleSystems.push_back(&ps);
					fIncludeChildren(ps);
				}
				Lua::Pop(l,1);
			}
			if(particleSystems.empty())
			{
				Con::cwar<<"WARNING: No particles to save. Particle file will not be generated!"<<Con::endl;
				Lua::PushBool(l,false);
				return 1;
			}

			FileManager::CreatePath(ufile::get_path_from_filename(name).c_str());
			auto f = FileManager::OpenFile<VFilePtrReal>(name.c_str(),"wb");
			if(f != NULL)
				Lua::PushBool(l,pragma::CParticleSystemComponent::Save(f,particleSystems));
			else
				Lua::PushBool(l,false);
			return 1;
		}
	}

	std::unordered_map<std::string,CParticleSystemData> particles;
	Lua::PushValue(l,2);
	int tparticles = 2;
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,tparticles) != 0)
	{
		if(Lua::IsTable(l,-1))
		{
			Lua::PushValue(l,-2);
			CParticleSystemData data {};
			std::string particle = Lua::ToString(l,-3);
			Lua::RemoveValue(l,-3);
			Lua::PushValue(l,-2);
			int table = Lua::GetStackTop(l);
			Lua::PushNil(l);
			while(Lua::GetNextPair(l,table) != 0)
			{
				Lua::PushValue(l,-2);
				std::string key = Lua::ToString(l,-3);
				Lua::RemoveValue(l,-3);
				StringToLower(key);
				if(key == "initializers" || key == "operators" || key == "renderers")
				{
					if(Lua::IsTable(l,-2))
					{
						auto numOperators = Lua::GetObjectLength(l,-2);
						Lua::PushValue(l,-2);
						int tmod = Lua::GetStackTop(l);
						for(auto i=decltype(numOperators){0u};i<numOperators;++i)
						{
							Lua::PushInt(l,i +1);
							Lua::GetTableValue(l,tmod);
							auto tOp = Lua::GetStackTop(l);
							Lua::CheckTable(l,tOp);

							Lua::PushString(l,"operatorType");
							Lua::GetTableValue(l,tOp);
							std::string opType = Lua::CheckString(l,-1);
							Lua::Pop(l,1);

							std::vector<CParticleModifierData> modData;
							modData.push_back(CParticleModifierData{opType});
							int dataIdx = 0;
							char dataType = -1;
							Lua::PushNil(l);
							while(Lua::GetNextPair(l,tOp) != 0)
							{
								if(!Lua::IsTable(l,-1))
								{
									if(dataType == 0)
										Lua::Pop(l,1);
									else
									{
										dataType = 1;
										Lua::PushValue(l,-2);
										std::string modKey = Lua::ToString(l,-3);
										std::string modVal = Lua::ToString(l,-2);
										Lua::RemoveValue(l,-3);
										Lua::RemoveValue(l,-2);
										modData[dataIdx].settings.insert(std::unordered_map<std::string,std::string>::value_type(modKey,modVal));
									}
								}
								else if(dataType != 1)
								{
									dataType = 0;
									if(dataIdx > 0)
										modData.push_back(CParticleModifierData{opType});
									dataIdx++;
									int tModSubSettings = Lua::GetStackTop(l);
									Lua::PushNil(l);
									while(Lua::GetNextPair(l,tModSubSettings) != 0)
									{
										Lua::PushValue(l,-2);
										std::string modKey = Lua::ToString(l,-3);
										std::string modVal = Lua::ToString(l,-2);
										Lua::RemoveValue(l,-3);
										Lua::RemoveValue(l,-2);
										modData[dataIdx -1].settings.insert(std::unordered_map<std::string,std::string>::value_type(modKey,modVal));
									}
									Lua::Pop(l,1);
								}
								else
									Lua::Pop(l,1);
							}
							Lua::Pop(l,1);
							for(unsigned int i=0;i<modData.size();i++)
							{
								if(modData[i].settings.empty() == false)
								{
									if(key == "initializers")
										data.initializers.push_back(modData[i]);
									else if(key == "operators")
										data.operators.push_back(modData[i]);
									else if(key == "renderers")
										data.renderers.push_back(modData[i]);
								}
							}
						}
						Lua::Pop(l,1);
						Lua::RemoveValue(l,-2);
					}
					else
						Lua::RemoveValue(l,-2);
				}
				else if(key == "children")
				{
					if(Lua::IsTable(l,-2))
					{
						Lua::PushValue(l,-2);
						int tchildren = Lua::GetStackTop(l);
						Lua::PushNil(l);
						while(Lua::GetNextPair(l,tchildren) != 0)
						{
							Lua::CheckTable(l,-1);
							auto tChild = Lua::GetStackTop(l);

							CParticleChildData childData {};

							Lua::PushString(l,"childName");
							Lua::GetTableValue(l,tChild);
							childData.childName = Lua::CheckString(l,-1);
							Lua::Pop(l,1);

							Lua::PushString(l,"delay");
							Lua::GetTableValue(l,tChild);
							if(Lua::IsSet(l,-1))
								childData.delay = Lua::CheckNumber(l,-1);
							Lua::Pop(l,1);

							data.children.push_back(childData);

							Lua::Pop(l,1);
						}
						Lua::Pop(l,1);
						Lua::RemoveValue(l,-2);
					}
					else
						Lua::RemoveValue(l,-2);
				}
				else if(Lua::IsTable(l,-2))
					Lua::RemoveValue(l,-2);
				else
				{
					std::string val = Lua::ToString(l,-2);
					Lua::RemoveValue(l,-2);
					data.settings.insert(std::unordered_map<std::string,std::string>::value_type(key,val));
				}
			}
			Lua::Pop(l,1);
			particles.insert(std::make_pair(particle,data));
		}
		else
			Lua::Pop(l,1);
	}
	Lua::Pop(l,2);
	Lua::PushBool(l,pragma::asset::save_particle_system(name,particles,rootPath));
	return 1;
}
std::shared_ptr<prosper::RenderTarget> Lua::engine::get_staging_render_target() {return c_engine->GetRenderContext().GetWindow().GetStagingRenderTarget();}
void Lua::engine::set_fixed_frame_delta_time_interpretation(uint16_t fps) {c_engine->SetFixedFrameDeltaTimeInterpretationByFPS(fps);}
void Lua::engine::clear_fixed_frame_delta_time_interpretation() {c_engine->SetFixedFrameDeltaTimeInterpretation({});}
void Lua::engine::set_tick_delta_time_tied_to_frame_rate(bool tieToFrameRate) {c_engine->SetTickDeltaTimeTiedToFrameRate(tieToFrameRate);}
Vector2i Lua::engine::get_window_resolution()
{
	auto &window = c_engine->GetRenderContext().GetWindow();
	return window.IsValid() ? window->GetSize() : Vector2i{};
}
Vector2i Lua::engine::get_render_resolution() {return c_engine->GetRenderResolution();}
uint32_t Lua::engine::get_current_frame_index() {return c_engine->GetRenderContext().GetLastFrameId();}
