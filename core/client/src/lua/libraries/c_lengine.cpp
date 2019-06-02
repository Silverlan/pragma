#include "stdafx_client.h"
#include "pragma/lua/libraries/c_lengine.h"
#include "pragma/input/inputhelper.h"
#include "cmaterialmanager.h"
#include "luasystem.h"
#include "pragma/c_engine.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/particlesystem/c_particlesystemdata.h"
#include "pragma/file_formats/wmd_load.h"
#include <wgui/fontmanager.h>
#include "pragma/lua/classes/c_ldef_fontinfo.h"
#include "pragma/rendering/scene/scene.h"
#include <texturemanager/texturemanager.h>
#include "textureinfo.h"
#include "pragma/entities/components/c_lentity_components.hpp"
#include <pragma/lua/libraries/lfile.h>
#include <pragma/lua/lua_entity_component.hpp>

#define WPT_VERSION 0x0001

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

int Lua::engine::create_font(lua_State *l)
{
	std::string identifier = Lua::CheckString(l,1);
	std::string font = Lua::CheckString(l,2);
	unsigned int size = Lua::CheckInt<unsigned int>(l,3);
	bool bReload = false;
	if(Lua::IsSet(l,4))
		bReload = Lua::CheckBool(l,4);
	auto info = FontManager::LoadFont(identifier.c_str(),font.c_str(),size,bReload);
	if(info == nullptr)
		return 0;
	Lua::Push<std::shared_ptr<const FontInfo>>(l,info);
	return 1;
}

int Lua::engine::get_font(lua_State *l)
{
	std::string identifier = Lua::CheckString(l,1);
	auto info = FontManager::GetFont(identifier.c_str());
	if(info == nullptr)
		return 0;
	Lua::Push<std::shared_ptr<const FontInfo>>(l,info);
	return 1;
}

int Lua::engine::get_text_size(lua_State *l)
{
	std::string text = Lua::CheckString(l,1);
	if(Lua::IsString(l,2))
	{
		std::string identifier = Lua::CheckString(l,2);
		auto info = FontManager::GetFont(identifier.c_str());
		if(info == nullptr)
		{
			Lua::PushNumber(l,0.f);
			Lua::PushNumber(l,0.f);
			return 2;
		}
		int w = 0;
		int h = 0;
		FontManager::GetTextSize(text.c_str(),0u,info.get(),&w,&h);
		Lua::Push<Vector2i>(l,Vector2i{w,h});
		return 1;
	}
	auto *info = _lua_Font_check(l,2);
	int w = 0;
	int h = 0;
	FontManager::GetTextSize(text.c_str(),0u,info,&w,&h);
	Lua::Push<Vector2i>(l,Vector2i{w,h});
	return 1;
}

int Lua::engine::bind_key(lua_State *l)
{
	std::string key = luaL_checkstring(l,1);
	if(lua_isstring(l,2))
	{
		std::string cmd = luaL_checkstring(l,2);
		short c;
		if(!StringToKey(key,&c))
			return 0;
		c_engine->MapKey(c,cmd);
		return 0;
	}
	luaL_checkfunction(l,2);
	int fc = lua_createreference(l,2);
	short c;
	if(!StringToKey(key,&c))
		return 0;
	c_engine->MapKey(c,fc);
	return 0;
}

int Lua::engine::unbind_key(lua_State *l)
{
	std::string key = luaL_checkstring(l,1);
	short c;
	if(!StringToKey(key,&c))
		return 0;
	c_engine->UnmapKey(c);
	return 0;
}

int Lua::engine::precache_material(lua_State *l)
{
	std::string mat = luaL_checkstring(l,1);
	client->LoadMaterial(mat.c_str());
	return 0;
}

int Lua::engine::precache_model(lua_State *l)
{
	std::string mdl = luaL_checkstring(l,1);
	auto *nw = c_engine->GetNetworkState(l);
	FWMD wmd(nw->GetGameState());
	wmd.Load<CModel,CModelMesh,CModelSubMesh>(nw->GetGameState(),mdl.c_str(),[](const std::string &path,bool bReload) -> Material* {
		return client->LoadMaterial(path,bReload);	
	},[](const std::string &path) -> std::shared_ptr<Model> {
		return c_game->LoadModel(path);
	});
	return 0;
}

int Lua::engine::load_material(lua_State *l)
{
	std::string mat = Lua::CheckString(l,1);
	bool bReload = false;
	if(Lua::IsSet(l,2))
		bReload = Lua::CheckBool(l,2);
	auto bLoadInstantly = false;
	if(Lua::IsSet(l,3))
		bLoadInstantly = Lua::CheckBool(l,3);
	Material *material = client->LoadMaterial(mat.c_str(),bLoadInstantly,bReload);
	if(material == NULL)
		return 0;
	luabind::object(l,material).push(l);
	return 1;
}

int Lua::engine::create_material(lua_State *l)
{
	if(Lua::IsString(l,2) == true)
	{
		auto *identifier = Lua::CheckString(l,1);
		auto *shader = Lua::CheckString(l,2);
		auto *mat = client->CreateMaterial(identifier,shader);
		Lua::Push<Material*>(l,mat);
		return 1;
	}
	auto *shader = Lua::CheckString(l,1);
	auto *mat = client->CreateMaterial(shader);
	Lua::Push<Material*>(l,mat);
	return 1;
}

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
			auto hParent = Lua::Check<CParticleSystemHandle>(l,2);
			pragma::Lua::check_component(l,hParent);
			parent = hParent.get();
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
			auto hParent = Lua::Check<CParticleSystemHandle>(l,2);
			pragma::Lua::check_component(l,hParent);
			parent = hParent.get();
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

int Lua::engine::precache_particle_system(lua_State *l)
{
	std::string particle = luaL_checkstring(l,1);
	auto bReload = false;
	if(Lua::IsSet(l,2))
		bReload = Lua::CheckBool(l,2);
	Lua::PushBool(l,pragma::CParticleSystemComponent::Precache(particle,bReload));
	return 1;
}

// See c_particlesystem_save.cpp as well
int Lua::engine::save_particle_system(lua_State *l)
{
	std::string name = luaL_checkstring(l,1);
	Lua::CheckTable(l,2);

	name = FileManager::GetCanonicalizedPath(name);
	ustring::to_lower(name);
	name = "particles\\" +name;
	ufile::remove_extension_from_filename(name);
	name += ".wpt";
	if(Lua::file::validate_write_operation(l,name) == false)
	{
		Lua::PushBool(l,false);
		return 1;
	}
	FileManager::CreatePath(ufile::get_path_from_filename(name).c_str());
	auto f = FileManager::OpenFile<VFilePtrReal>(name.c_str(),"wb");
	if(f == NULL)
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
					if(hChild.expired())
						continue;
					auto *ps = hChild.get();
					if(ps->IsRecordingKeyValues() == false)
					{
						Con::cwar<<"WARNING: Cannot save particle system '"<<ps->GetParticleSystemName()<<"', which wasn't created with the \"record key-values\" flag set! Skipping..."<<Con::endl;
						return;
					}
					particleSystems.push_back(hChild.get());
					fIncludeChildren(*hChild.get());
				}
			};
			for(auto i=decltype(numParticleSystems){0u};i<numParticleSystems;++i)
			{
				Lua::PushInt(l,i +1u);
				Lua::GetTableValue(l,t);
				auto &ps = Lua::Check<CParticleSystemHandle>(l,-1);
				pragma::Lua::check_component(l,ps);
				if(ps->IsRecordingKeyValues() == false)
					Con::cwar<<"WARNING: Cannot save particle system '"<<ps->GetParticleSystemName()<<"', which wasn't created with the \"record key-values\" flag set! Skipping..."<<Con::endl;
				else
				{
					particleSystems.push_back(ps.get());
					fIncludeChildren(*ps);
				}
				Lua::Pop(l,1);
			}
			if(particleSystems.empty())
			{
				Con::cwar<<"WARNING: No particles to save. Particle file will not be generated!"<<Con::endl;
				Lua::PushBool(l,false);
				return 1;
			}
			Lua::PushBool(l,pragma::CParticleSystemComponent::Save(f,particleSystems));
			return 1;
		}
	}

	std::unordered_map<std::string,CParticleSystemData*> particles;
	Lua::PushValue(l,2);
	int tparticles = 2;
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,tparticles) != 0)
	{
		if(Lua::IsTable(l,-1))
		{
			Lua::PushValue(l,-2);
			CParticleSystemData *data = new CParticleSystemData;
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
						Lua::PushValue(l,-2);
						int tmod = Lua::GetStackTop(l);
						Lua::PushNil(l);
						while(Lua::GetNextPair(l,tmod) != 0)
						{
							if(Lua::IsTable(l,-1))
							{
								Lua::PushValue(l,-2);
								std::string mod = Lua::ToString(l,-3);
								Lua::RemoveValue(l,-3);
								int tmodSettings = Lua::GetStackTop(l) -1;
								Lua::PushNil(l);
								std::vector<std::unique_ptr<CParticleModifierData>> modData;
								modData.push_back(std::make_unique<CParticleModifierData>(mod));
								int dataIdx = 0;
								char dataType = -1;
								while(Lua::GetNextPair(l,tmodSettings) != 0)
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
											modData[dataIdx]->settings.insert(std::unordered_map<std::string,std::string>::value_type(modKey,modVal));
										}
									}
									else if(dataType != 1)
									{
										dataType = 0;
										if(dataIdx > 0)
											modData.push_back(std::make_unique<CParticleModifierData>(mod));
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
											modData[dataIdx -1]->settings.insert(std::unordered_map<std::string,std::string>::value_type(modKey,modVal));
										}
										Lua::Pop(l,1);
									}
									else
										Lua::Pop(l,1);
								}
								Lua::RemoveValue(l,-2);
								StringToLower(mod);
								for(unsigned int i=0;i<modData.size();i++)
								{
									if(modData[i]->settings.empty())
										modData[i] = nullptr;
									else
									{
										if(key == "initializers")
											data->initializers.push_back(std::move(modData[i]));
										else if(key == "operators")
											data->operators.push_back(std::move(modData[i]));
										else if(key == "renderers")
											data->renderers.push_back(std::move(modData[i]));
									}
								}
							}
							else
								Lua::Pop(l,1);
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
							if(!Lua::IsTable(l,-1))
							{
								std::string child = Lua::ToString(l,-1);
								StringToLower(child);
								Lua::RemoveValue(l,-1);
								data->children.push_back(child);
							}
							else
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
					data->settings.insert(std::unordered_map<std::string,std::string>::value_type(key,val));
				}
			}
			Lua::Pop(l,1);
			particles.insert(std::unordered_map<std::string,CParticleSystemData*>::value_type(particle,data));
		}
		else
			Lua::Pop(l,1);
	}
	Lua::Pop(l,2);
	f->Write<char>('W');
	f->Write<char>('P');
	f->Write<char>('T');
	f->Write<unsigned int>(WPT_VERSION);
	f->Write<unsigned int>(CUInt32(particles.size()));
	std::unordered_map<std::string,CParticleSystemData*>::iterator it;
	std::vector<unsigned long long> offsets;
	for(it=particles.begin();it!=particles.end();it++)
	{
		f->WriteString(it->first);
		offsets.push_back(f->Tell());
		f->Write<unsigned long long>((unsigned long long)(0));
	}
	unsigned int i = 0;
	for(it=particles.begin();it!=particles.end();it++)
	{
		unsigned long long offset = f->Tell();
		f->Seek(offsets[i]);
		f->Write<unsigned long long>(offset);
		f->Seek(offset);

		CParticleSystemData *data = it->second;
		f->Write<unsigned int>(CUInt32(data->settings.size()));
		std::unordered_map<std::string,std::string>::iterator j;
		for(j=data->settings.begin();j!=data->settings.end();j++)
		{
			f->WriteString(j->first);
			f->WriteString(j->second);
		}
		f->Write<unsigned int>(CUInt32(data->initializers.size()));
		for(unsigned int k=0;k<data->initializers.size();k++)
		{
			auto &modData = data->initializers[k];
			f->WriteString(modData->name);
			f->Write<unsigned int>(CUInt32(modData->settings.size()));
			std::unordered_map<std::string,std::string>::iterator l;
			for(l=modData->settings.begin();l!=modData->settings.end();l++)
			{
				f->WriteString(l->first);
				f->WriteString(l->second);
			}
		}
		f->Write<unsigned int>(CUInt32(data->operators.size()));
		for(unsigned int k=0;k<data->operators.size();k++)
		{
			auto &modData = data->operators[k];
			f->WriteString(modData->name);
			f->Write<unsigned int>(CUInt32(modData->settings.size()));
			std::unordered_map<std::string,std::string>::iterator l;
			for(l=modData->settings.begin();l!=modData->settings.end();l++)
			{
				f->WriteString(l->first);
				f->WriteString(l->second);
			}
		}
		f->Write<unsigned int>(CUInt32(data->renderers.size()));
		for(unsigned int k=0;k<data->renderers.size();k++)
		{
			auto &modData = data->renderers[k];
			f->WriteString(modData->name);
			f->Write<unsigned int>(CUInt32(modData->settings.size()));
			std::unordered_map<std::string,std::string>::iterator l;
			for(l=modData->settings.begin();l!=modData->settings.end();l++)
			{
				f->WriteString(l->first);
				f->WriteString(l->second);
			}
		}
		std::vector<std::string> &children = it->second->children;
		f->Write<unsigned char>(CUInt8(children.size()));
		for(unsigned char j=0;j<children.size();j++)
			f->WriteString(children[j]);
		delete it->second;
		i++;
	}
	Lua::PushBool(l,true);
	return 1;
}
int Lua::engine::save_frame_buffer_as_tga(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	int x = Lua::CheckInt<int>(l,2);
	int y = Lua::CheckInt<int>(l,3);
	int w = Lua::CheckInt<int>(l,4);
	int h = Lua::CheckInt<int>(l,5);
	bool r;
	if(!Lua::IsSet(l,6))
		r = client->SaveFrameBufferAsTGA(name.c_str(),x,y,w,h);
	else
	{
		unsigned int format = Lua::CheckInt<UInt32>(l,6);
		r = client->SaveFrameBufferAsTGA(name.c_str(),x,y,w,h,format);
	}
	Lua::PushBool(l,r);
	return 1;
}
int Lua::engine::save_texture_as_tga(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	bool r;
	if(!Lua::IsSet(l,2))
		r = client->SaveTextureAsTGA(name.c_str());
	else
	{
		unsigned int target = Lua::CheckInt<UInt32>(l,2);
		unsigned int level = 0;
		if(Lua::IsSet(l,3))
			level = Lua::CheckInt<UInt32>(l,3);
		r = client->SaveTextureAsTGA(name.c_str(),target,level);
	}
	Lua::PushBool(l,r);
	return 1;
}
int Lua::engine::create_texture(lua_State *l)
{
	/*auto name = Lua::CheckString(l,1);
	if(Lua::IsGLTexture(l,2))
	{
		auto *glTex = Lua::CheckGLTexture(l,2);
		auto *tex = TextureManager::CreateTexture(name,*glTex);
		if(tex == nullptr)
			return 0;
		Lua::Push<Texture*>(l,tex);
		return 1;
	}
	auto w = Lua::CheckInt(l,2);
	auto h = Lua::CheckInt(l,3);
	auto *tex = TextureManager::CreateTexture(name,CUInt32(w),CUInt32(h));
	if(tex == nullptr)
		return 0;
	Lua::Push<Texture*>(l,tex);*/ // Vulkan TODO
	return 1;
}
