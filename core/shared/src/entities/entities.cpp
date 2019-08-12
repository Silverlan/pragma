#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/level/mapgeometry.h"
#include "pragma/entities/components/map_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/baseworld.h"
#include "pragma/entities/output.h"
#include "pragma/entities/entity_component_system_t.hpp"

pragma::BaseWorldComponent *Game::GetWorld() {return m_worldComponent.get();}
unsigned int Game::GetEntityCount() {return 0;}
void Game::RemoveEntity(BaseEntity*) {}
BaseEntity *Game::CreateEntity() {return NULL;}
BaseEntity *Game::CreateEntity(std::string classname) {return NULL;}
void Game::SpawnEntity(BaseEntity *ent) {ent->OnSpawn();}
BaseEntity *Game::GetEntity(unsigned int) {return NULL;}
BaseEntity *Game::GetEntityByLocalIndex(uint32_t idx) {return GetEntity(idx);}

BaseEntity *Game::CreateMapEntity(uint32_t version,const std::string &classname,VFilePtr f,const pragma::level::BSPInputData &bspInputData,std::vector<Material*> &materials,const Vector3 &origin,uint64_t offsetToEndOfEntity,std::vector<EntityHandle> &ents,std::vector<EntityHandle> *entities)
{
	auto *ent = CreateEntity(classname);
	if(ent == nullptr && offsetToEndOfEntity != std::numeric_limits<uint64_t>::max())
		f->Seek(offsetToEndOfEntity);
	else
	{
		unsigned int numKeyValues = f->Read<unsigned int>();
		std::unordered_map<std::string,std::string> keyvalues;
		for(unsigned int j=0;j<numKeyValues;j++)
		{
			std::string key = f->ReadString();
			std::string val = f->ReadString();
			keyvalues.insert(std::unordered_map<std::string,std::string>::value_type(key,val));
		}
		unsigned int numOutputs = f->Read<unsigned int>();
		std::unordered_map<std::string,std::vector<Output>> outputs;
		for(unsigned int i=0;i<numOutputs;i++)
		{
			std::string name = f->ReadString();
			std::string target = f->ReadString();
			std::string input = f->ReadString();
			std::string param = f->ReadString();
			float delay = f->Read<float>();
			int times = f->Read<int>();
			StringToLower(name);
			std::unordered_map<std::string,std::vector<Output>>::iterator it = outputs.find(name);
			if(it == outputs.end())
				it = outputs.insert(std::unordered_map<std::string,std::vector<Output>>::value_type(name,std::vector<Output>())).first;
			it->second.push_back(Output(target,input,param,delay,times));
		}
		std::vector<std::string> componentNames {};
		if(version >= 5)
		{
			auto numComponents = f->Read<uint32_t>();
			for(auto i=decltype(numComponents){0u};i<numComponents;++i)
				componentNames.push_back(f->ReadString());
		}
		Vector3 position {};
		if(version >= 7)
			position = f->Read<Vector3>();
		if(version >= 9)
		{
			auto numLeaves = f->Read<uint32_t>();
			std::vector<uint16_t> leaves {};
			leaves.resize(numLeaves);
			f->Read(leaves.data(),leaves.size() *sizeof(leaves.front()));
		}
		pragma::level::load_map_brushes(*this,version,f,ent,materials,GetSurfaceMaterials(),origin);
		if(version >= 8)
			pragma::level::load_map_faces(*this,f,*ent,bspInputData,materials);
		if(ent != NULL)
		{
			if(version >= 7)
			{
				auto pTrComponent = ent->GetTransformComponent();
				if(pTrComponent.valid())
					pTrComponent->SetPosition(position);
			}

			for(auto &componentName : componentNames)
				ent->AddComponent(componentName);

			auto pMapComponent = ent->AddComponent<pragma::MapComponent>();
			if(pMapComponent.valid())
				pMapComponent->SetMapIndex(m_mapEntityIdx);
			for(auto &pair : keyvalues)
				ent->SetKeyValue(pair.first,pair.second);

			auto *pIoComponent = static_cast<pragma::BaseIOComponent*>(ent->FindComponent("io").get());
			if(pIoComponent != nullptr)
			{
				for(auto &typeOutputs : outputs)
				{
					for(auto &output : typeOutputs.second)
						pIoComponent->StoreOutput(typeOutputs.first,output.entities,output.input,output.param,output.delay,output.times);
				}
			}
			auto pTrComponent = ent->GetTransformComponent();
			if(pTrComponent.valid())
				pTrComponent->SetPosition(pTrComponent->GetPosition() +origin);
			ents.push_back(ent->GetHandle());
			if(entities != nullptr)
				entities->push_back(ent->GetHandle());
		}
	}
	if(offsetToEndOfEntity != std::numeric_limits<uint64_t>::max())
		f->Seek(offsetToEndOfEntity);
	return ent;
}
