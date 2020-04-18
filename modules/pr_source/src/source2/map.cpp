#include "source2/source2.hpp"
#include "source2/pr_impl.hpp"
#include "source_engine/source_engine.hpp"
#include <util_source2.hpp>
#include <source2/resource.hpp>
#include <source2/resource_data.hpp>
#include <source2/resource_edit_info.hpp>
#include <pragma/model/vertex.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/networkstate/networkstate.h>
#include <pragma/game/game_resources.hpp>
#include <pragma/game/game.h>
#include <pragma/asset_types/world.hpp>
#include <unordered_set>
#include <sharedutils/util_file.h>
#include <util_fgd.hpp>

#pragma optimize("",off)
std::shared_ptr<pragma::asset::WorldData> source2::convert::convert_map(Game &game,std::shared_ptr<VFilePtrInternal> &f,const std::string &mapName)
{
	auto &nw = *game.GetNetworkState();
	auto resource = impl::load_resource(nw,f);
	auto *s2World = resource ? dynamic_cast<source2::resource::World*>(resource->FindBlock(source2::BlockType::DATA)) : nullptr;
	if(s2World == nullptr)
		return nullptr;
	auto scene = s2World->Load();
	if(scene == nullptr)
		return nullptr;
	auto world = pragma::asset::WorldData::Create(nw);
	std::vector<std::string> models {};
	auto &nodes = scene->GetSceneNodes();
	models.reserve(nodes.size());
	for(auto &node : nodes)
	{
		auto &t = node->GetTransform();
		switch(node->GetType())
		{
		case source2::resource::SceneNode::Type::Mesh:
		{
			auto &mesh = static_cast<source2::resource::MeshSceneNode&>(*node).GetMesh();
			// std::cout<<"Mesh: "<<mesh.get()<<std::endl;
			break;
		}
		case source2::resource::SceneNode::Type::Model:
		{
			auto &modelNode = static_cast<source2::resource::ModelSceneNode&>(*node);
			auto &s2Mdl = modelNode.GetModel();
			if(s2Mdl)
			{
				auto mdl = convert_model(game,*s2Mdl);
				auto name = s2Mdl->GetName();
				ufile::remove_extension_from_filename(name);
				mdl->Save(nw.GetGameState(),name,util::IMPORT_PATH);
				models.push_back(name);
			}
			break;
		}
		}
	}

	// For now we'll just create the map geometry as static props
	for(auto &mdl : models)
	{
		// TODO: Transform (pos/rot)
		auto entData = pragma::asset::EntityData::Create();
		entData->SetClassName("prop_static");
		entData->SetKeyValue("model",mdl);
		world->AddEntity(*entData);
	}

	util::Path mapPath {mapName};
	mapPath.PopBack(); // Pop file name
	auto lightmapPath = mapPath.GetString() +"/irradiance.vtex_c";
	auto targetPath = mapPath.GetString() +"/lightmap_atlas.vtex_c";
	util::port_file(&nw,lightmapPath,targetPath);
	world->SetLightMapEnabled(true);

	// These are somewhat arbitrary but seem to create results that match the original colors closely (tested with the Russell's Lab VR demo)
	world->SetLightMapIntensity(1.f /static_cast<float>(std::numeric_limits<uint8_t>::max()));
	world->SetLightMapSqrtFactor(1.f);
	
	for(auto &ent : scene->GetEntities())
	{
		auto &keyValues = ent->GetKeyValues();
		auto itClassName = keyValues.find("classname");
		if(itClassName == keyValues.end())
			continue;
		auto className = itClassName->second;

		auto entData = pragma::asset::EntityData::Create();
		entData->SetClassName(className);
		for(auto &pair : keyValues)
			entData->SetKeyValue(pair.first,pair.second);

		auto isWorld = (className == "worldspawn");
		world->AddEntity(*entData,isWorld);
	}
	auto fgdData = source_engine::load_fgds(nw);
	source_engine::translate_entity_data(*world,fgdData,true);
	for(auto &ent : world->GetEntities())
	{
		if(ent->GetClassName() == "worldspawn") // TODO: Move this to source_engine::translate_entity_data
		{
			ent->SetKeyValue("classname","world");
			ent->SetClassName("world");
		}

		auto &keyValues = ent->GetKeyValues();
		auto it = keyValues.find("origin");
		if(it == keyValues.end())
			continue;
		auto origin = uvec::create(it->second);
		ent->SetOrigin(origin);
	}
	return world;
}
#pragma optimize("",on)
