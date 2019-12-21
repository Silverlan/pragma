#include <pragma/pragma_module.hpp>
#include <datasystem.h>
#include <pragma/debug/debug_performance_profiler.hpp>
#include <pragma/level/mapinfo.h>
#include <util_dmx.hpp>
#include <fsys/filesystem.h>
#include <pragma/networkstate/networkstate.h>
#include <pragma/model/model.h>
#include <pragma/lua/libraries/lfile.h>
#include <pragma/engine.h>
#include <pragma/entities/environment/effects/particlesystemdata.h>
#include <luasystem.h>
#include <luainterface.hpp>
#include <sharedutils/util.h>
#include <sharedutils/util_file.h>

extern DLLENGINE Engine *engine;

namespace import
{
	bool load_dmx(NetworkState *nw,const VFilePtr &f,const std::function<std::shared_ptr<Model>()> &fCreateModel);
	bool load_source_particle(NetworkState *nw,const VFilePtr &f);
};

extern "C" {
	bool PRAGMA_EXPORT load_source_particle(NetworkState &nw,const std::string &fileName)
	{
		auto f = FileManager::OpenSystemFile(fileName.c_str(),"rb");
		if(f == nullptr)
			return false;
		return ::import::load_source_particle(&nw,f);
		/*auto &libSteamWorks = lua.RegisterLibrary("import",{
			{"import_particle",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
				auto &f = *Lua::CheckFile(l,1);
				auto &mdl = Lua::Check<std::shared_ptr<Model>>(l,2);

				Lua::PushBool(l,::import::load_source_particle(engine->GetNetworkState(l),f.GetHandle()));
				return 1;
			})}
		});*/
	}
};

#pragma optimize("",off)
bool import::load_dmx(NetworkState *nw,const VFilePtr &f,const std::function<std::shared_ptr<Model>()> &fCreateModel)
{
	auto vf = f;
	auto dmxData = dmx::FileData::Load(vf);
	if(dmxData == nullptr)
		return false;
	std::function<void(const dmx::FileData&)> fPrintData = nullptr;
	fPrintData = [](const dmx::FileData &data) {
		for(auto &el : data.GetElements())
		{
			el->name;
		//	el->GUID
		}
		//dmxData->GetElements
	};
	fPrintData(*dmxData);
	std::cout<<"T"<<std::endl;
	return true;
}

static std::string attribute_to_string(dmx::Element &el,const std::string &attrName,const std::string &defaultStr="")
{
	auto attr = el.GetAttr(attrName);
	if(attr == nullptr)
		return defaultStr;
	switch(attr->type)
	{
	case dmx::AttrType::Int:
		return std::to_string(*attr->GetInt());
	case dmx::AttrType::Float:
		return std::to_string(*attr->GetFloat());
	case dmx::AttrType::Bool:
		return *attr->GetBoolean() ? "1" : "0";
	case dmx::AttrType::String:
		return *attr->GetString();
	case dmx::AttrType::Time:
		return std::to_string(*attr->GetTime());
	case dmx::AttrType::Color:
	{
		auto &color = *attr->GetColor();
		return std::to_string(color.at(0)) +' ' +std::to_string(color.at(1)) +' ' +std::to_string(color.at(2)) +' ' +std::to_string(color.at(3));
	}
	case dmx::AttrType::Vector2:
	{
		auto &v = *attr->GetVector2();
		return std::to_string(v.x) +' ' +std::to_string(v.y);
	}
	case dmx::AttrType::Vector3:
	{
		auto &v = *attr->GetVector3();
		return std::to_string(v.x) +' ' +std::to_string(v.y) +' ' +std::to_string(v.z);
	}
	case dmx::AttrType::Vector4:
	{
		auto &v = *attr->GetVector4();
		return std::to_string(v[0]) +' ' +std::to_string(v[1]) +' ' +std::to_string(v[2]) +' ' +std::to_string(v[3]);
	}
	case dmx::AttrType::Angle:
	{
		auto &v = *attr->GetAngle();
		return std::to_string(v.p) +' ' +std::to_string(v.y) +' ' +std::to_string(v.r);
	}
	case dmx::AttrType::UInt64:
		return std::to_string(*attr->GetUInt64());
	case dmx::AttrType::UInt8:
		return std::to_string(*attr->GetUInt8());
	}
	throw std::invalid_argument{"Unsupported attribute type!"};
}

static void convert_source_engine_position_to_pragma(Vector3 &pos)
{
	pos = Vector3{pos.x,pos.z,-pos.y};
	uvec::rotate(&pos,uquat::create(EulerAngles{0.f,-90.f,0.f}));
}

bool import::load_source_particle(NetworkState *nw,const VFilePtr &f)
{
	auto vf = f;
	auto dmxData = dmx::FileData::Load(vf);
	if(dmxData == nullptr)
		return false;
	auto &rootAttr = dmxData->GetRootAttribute();
	auto *elRoot = rootAttr->GetElement();
#if 0
	std::unordered_map<std::string,std::string> values = {
		{"maxparticles","1"},
	{"emission_rate",std::to_string(std::numeric_limits<uint16_t>::max())},
	{"sort_particles","0"},
	{"orientation_type",std::to_string(umath::to_integral(m_orientationType))},
	{"cast_shadows","0"},
	{"static_scale",std::to_string(m_size)},
	{"color",std::to_string(m_color.r) +" " +std::to_string(m_color.g) +" " +std::to_string(m_color.b) +" " +std::to_string(startAlpha)},
	{"bloom_scale",std::to_string(m_bloomScale)},
	{"move_with_emitter","1"}
	};
	auto spawnFlags = ent.GetSpawnFlags();
	if(spawnFlags &umath::to_integral(SpawnFlags::BlackToAlpha))
		values.insert(std::make_pair("black_to_alpha","1"));
	if(spawnFlags &umath::to_integral(SpawnFlags::NoSoftParticles))
		values.insert(std::make_pair("soft_particles","0"));
	auto *pt = pragma::CParticleSystemComponent::Create(values);
	if(pt == nullptr)
		return;
	pt->AddInitializer("radius_random",std::unordered_map<std::string,std::string>{
		{"radius_min",std::to_string(m_size)},
		{"radius_max",std::to_string(m_size)}
	});
	pt->AddInitializer("lifetime_random",std::unordered_map<std::string,std::string>{
		{"lifetime_min",std::to_string(std::numeric_limits<float>::max())},
		{"lifetime_max",std::to_string(std::numeric_limits<float>::max())}
	});
	if(m_tFadeIn > 0.f)
	{
		pt->AddOperator("color_fade",std::unordered_map<std::string,std::string>{
			{"alpha",std::to_string(m_color.a)},
			{"fade_start","0"},
			{"fade_end",std::to_string(m_tFadeIn)}
		});
	}
	auto tFadeOut = (m_tFadeOut > 0.f) ? m_tFadeOut : 1.f;
	if(tFadeOut > 0.f)
	{
		pt->AddOperator("color_fade",std::unordered_map<std::string,std::string>{
			{"alpha","0"},
			{"fade_start",std::to_string(-tFadeOut)},
			{"fade_end","-0.0001"},
		});
	}
	pt->AddRenderer("sprite",std::unordered_map<std::string,std::string>{});

	if(m_hCbRenderCallback.IsValid())
		m_hCbRenderCallback.Remove();

	auto whAttComponent = pt->GetEntity().AddComponent<CAttachableComponent>();
	if(whAttComponent.valid())
	{
		AttachmentInfo attInfo {};
		attInfo.flags = FAttachmentMode::SnapToOrigin | FAttachmentMode::UpdateEachFrame;
		whAttComponent->AttachToEntity(&ent,attInfo);
	}
	// We need to update the particle system position every frame
	m_hCbRenderCallback = pt->AddRenderCallback([this]() {
		auto &ent = GetEntity();
		auto pAttComponent = ent.GetComponent<pragma::CAttachableComponent>();
		if(pAttComponent.valid())
			pAttComponent->UpdateAttachmentOffset();
		});

	if(m_particleRenderMode != std::numeric_limits<uint32_t>::max())
		pt->SetRenderMode(static_cast<RenderMode>(m_particleRenderMode));
	pt->SetContinuous(true);
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentPt = pt->GetEntity().GetTransformComponent();
	if(pTrComponent.valid() && pTrComponentPt.valid())
	{
		pTrComponentPt->SetPosition(pTrComponent->GetPosition());
		pTrComponentPt->SetOrientation(pTrComponent->GetOrientation());
	}
	pt->Start();
	m_hParticle = pt->GetHandle<pragma::CParticleSystemComponent>();
#endif
	if(elRoot && elRoot->expired() == false)
	{
		auto attrDefinitions = elRoot->lock()->GetAttr("particleSystemDefinitions");
		if(attrDefinitions)
		{
			auto *elements = attrDefinitions->GetArray(dmx::AttrType::ElementArray);
			if(elements)
			{
				std::unordered_map<std::string,CParticleSystemData> particles {};
				for(auto &attr : *elements)
				{
					auto &elRef = *attr->GetElement();
					if(elRef.expired())
						continue;
					auto &el = elRef.lock();
					if(el->type != "DmeParticleSystemDefinition")
						continue;
					auto it = particles.insert(std::make_pair(el->name,CParticleSystemData{}));
					auto &particleSystemData = it.first->second;

					std::cout<<"Particle System: "<<el->name<<std::endl;

					for(auto &pair : el->attributes)
					{
						if(ustring::compare(pair.first,"initializers",false))
						{
							auto *value = pair.second->GetArray(dmx::AttrType::ElementArray);
							for(auto &attr : *value)
							{
								auto &elRef = *attr->GetElement();
								if(elRef.expired())
									continue;
								auto &el = elRef.lock();
								if(el->type != "DmeParticleOperator")
									continue;
								auto &opName = el->name;
								if(ustring::compare(opName,"lifetime random",false))
								{
									particleSystemData.initializers.push_back({"lifetime_random"});
									auto &initializer = particleSystemData.initializers.back();
									initializer.settings["lifetime_min"] = attribute_to_string(*el,"lifetime_min","0");
									initializer.settings["lifetime_max"] = attribute_to_string(*el,"lifetime_max","0");
								}
								else if(ustring::compare(opName,"Position Within Box Random",false))
								{
									auto xMin = attribute_to_string(*el,"x_min","0");
									auto xMax = attribute_to_string(*el,"x_max","0");
									auto yMin = attribute_to_string(*el,"y_min","0");
									auto yMax = attribute_to_string(*el,"y_max","0");
									auto zMin = attribute_to_string(*el,"z_min","0");
									auto zMax = attribute_to_string(*el,"z_max","0");
									Vector3 min {util::to_float(xMin),util::to_float(yMin),util::to_float(zMin)};
									convert_source_engine_position_to_pragma(min);
									Vector3 max{util::to_float(xMax),util::to_float(yMax),util::to_float(zMax)};
									convert_source_engine_position_to_pragma(max);

									particleSystemData.initializers.push_back({"position_random_box"});
									auto &initializer = particleSystemData.initializers.back();
									initializer.settings["min"] = std::to_string(min.x) +' ' +std::to_string(min.y) +' ' +std::to_string(min.z);
									initializer.settings["max"] = std::to_string(max.x) +' ' +std::to_string(max.y) +' ' +std::to_string(max.z);
								}
								else if(ustring::compare(opName,"Position Within Sphere Random",false))
								{
									particleSystemData.initializers.push_back({"position_random_sphere"});
									auto &initializer = particleSystemData.initializers.back();
									initializer.settings["distance_min"] = attribute_to_string(*el,"distance_min","0");
									initializer.settings["distance_max"] = attribute_to_string(*el,"distance_min","0");
									initializer.settings["distance_bias"] = attribute_to_string(*el,"distance_bias","1 1 1");
								}
								else if(ustring::compare(opName,"Radius Random",false))
								{
									particleSystemData.initializers.push_back({"radius_random"});
									auto &initializer = particleSystemData.initializers.back();
									initializer.settings["radius_min"] = attribute_to_string(*el,"radius_min","0");
									initializer.settings["radius_max"] = attribute_to_string(*el,"radius_max","0");
								}
							}
						}
						else if(ustring::compare(pair.first,"renderers",false))
						{
							auto *value = pair.second->GetArray(dmx::AttrType::ElementArray);
							for(auto &attr : *value)
							{
								auto &elRef = *attr->GetElement();
								if(elRef.expired())
									continue;
								auto &el = elRef.lock();
								if(el->type != "DmeParticleOperator")
									continue;
								auto &opName = el->name;
								if(ustring::compare(opName,"render_animated_sprites",false))
									particleSystemData.settings["emission_rate"] = attribute_to_string(*el,"animation rate","0");
							}
						}
						else if(ustring::compare(pair.first,"operators",false))
						{
							auto *value = pair.second->GetArray(dmx::AttrType::ElementArray);
							for(auto &attr : *value)
							{
								auto &elRef = *attr->GetElement();
								if(elRef.expired())
									continue;
								auto &el = elRef.lock();
								if(el->type != "DmeParticleOperator")
									continue;

							}
						}
						else if(ustring::compare(pair.first,"children",false))
						{
							auto *value = pair.second->GetArray(dmx::AttrType::ElementArray);
							for(auto &attr : *value)
							{
								auto &elRef = *attr->GetElement();
								if(elRef.expired())
									continue;
								auto &el = elRef.lock();
								if(el->type != "DmeParticleChild")
									continue;

							}
						}
						else if(ustring::compare(pair.first,"emitters",false))
						{
							auto *value = pair.second->GetArray(dmx::AttrType::ElementArray);

						}
						else if(ustring::compare(pair.first,"view model effect",false))
						{
							auto *value = pair.second->GetBoolean();

						}
						else if(ustring::compare(pair.first,"cull_control_point",false))
						{
							auto *value = pair.second->GetInt();

						}
						else if(ustring::compare(pair.first,"color",false))
						{
							auto *value = pair.second->GetColor();
							if(value)
							{
								particleSystemData.settings["color"] = std::to_string(value->at(0)) +' ' +
									std::to_string(value->at(1)) +' ' +
									std::to_string(value->at(2)) +' ' +
									std::to_string(value->at(3));
							}
						}
						else if(ustring::compare(pair.first,"forces",false))
						{
							auto *value = pair.second->GetArray(dmx::AttrType::ElementArray);

						}
						else if(ustring::compare(pair.first,"preventNameBasedLookup",false))
						{
							auto *value = pair.second->GetBoolean();

						}
						else if(ustring::compare(pair.first,"maximum sim tick rate",false))
						{
							auto *value = pair.second->GetFloat();

						}
						else if(ustring::compare(pair.first,"constraints",false))
						{
							auto *value = pair.second->GetArray(dmx::AttrType::ElementArray);

						}
						else if(ustring::compare(pair.first,"max_particles",false))
						{
							auto *value = pair.second->GetInt();
							if(value)
								particleSystemData.settings["maxparticles"] = std::to_string(*value);
						}
						else if(ustring::compare(pair.first,"time to sleep when not drawn",false))
						{
							auto *value = pair.second->GetFloat();

						}
						else if(ustring::compare(pair.first,"initial_particles",false))
						{
							auto *value = pair.second->GetInt();

						}
						else if(ustring::compare(pair.first,"material",false))
						{
							auto *value = pair.second->GetString();
							if(value)
							{
								auto matPath = *value;
								ufile::remove_extension_from_filename(matPath);
								particleSystemData.settings["material"] = matPath;
							}
						}
						else if(ustring::compare(pair.first,"bounding_box_min",false))
						{
							auto *value = pair.second->GetVector3();

						}
						else if(ustring::compare(pair.first,"cull_replacement_definition",false))
						{
							auto *value = pair.second->GetString();

						}
						else if(ustring::compare(pair.first,"bounding_box_max",false))
						{
							auto *value = pair.second->GetVector3();

						}
						else if(ustring::compare(pair.first,"cull_radius",false))
						{
							auto *value = pair.second->GetFloat();

						}
						else if(ustring::compare(pair.first,"cull_cost",false))
						{
							auto *value = pair.second->GetFloat();

						}
						else if(ustring::compare(pair.first,"sequence_number 1",false))
						{
							auto *value = pair.second->GetInt();

						}
						else if(ustring::compare(pair.first,"radius",false))
						{
							auto *value = pair.second->GetFloat();
							if(value)
								particleSystemData.settings["radius"] = std::to_string(*value);
						}
						else if(ustring::compare(pair.first,"control point to disable rendering if it is the camera",false))
						{
							auto *value = pair.second->GetInt();

						}
						else if(ustring::compare(pair.first,"rotation",false))
						{
							auto *value = pair.second->GetFloat();
							
						}
						else if(ustring::compare(pair.first,"rotation_speed",false))
						{
							auto *value = pair.second->GetFloat();

						}
						else if(ustring::compare(pair.first,"sequence_number",false))
						{
							auto *value = pair.second->GetInt();

						}
						else if(ustring::compare(pair.first,"group id",false))
						{
							auto *value = pair.second->GetInt();

						}
						else if(ustring::compare(pair.first,"maximum draw distance",false))
						{
							auto *value = pair.second->GetFloat();

						}
						else if(ustring::compare(pair.first,"maximum time step",false))
						{
							auto *value = pair.second->GetFloat();

						}
						else if(ustring::compare(pair.first,"minimum sim tick rate",false))
						{
							auto *value = pair.second->GetFloat();

						}
						else if(ustring::compare(pair.first,"minimum rendered frames",false))
						{
							auto *value = pair.second->GetInt();

						}
						else if(ustring::compare(pair.first,"Sort particles",false))
						{
							auto *value = pair.second->GetBoolean();
							if(value)
								particleSystemData.settings["sort_particles"] = *value ? "1" : "0";
						}
						else if(ustring::compare(pair.first,"batch particle systems",false))
						{
							auto *value = pair.second->GetBoolean();
						
						}
						else
							std::cout<<"Unknown particle attribute '"<<pair.first<<"' of type '"<<dmx::type_to_string(pair.second->type)<<"'!"<<std::endl;
					}
				}
				pragma::asset::save_particle_system("source_ported_particles",particles);
			}
			/*attrDefinitions->
			for(auto &pair : attrDefinitions->attributes)
			{
				auto el = pair.second->Get("DmeParticleSystemDefinition");
				//pair.second->
				//type = ""
				//dmx::AttrType::
				//pair.second->type
			}*/
		}
	}
	return true;
}
#pragma optimize("",on)
