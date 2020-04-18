#include "stdafx_client.h"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

void CLightMapReceiverComponent::SetupLightMapUvData(CBaseEntity &ent)
{
	auto mdl = ent.GetModel();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0u) : nullptr;
	if(meshGroup == nullptr)
		return;
	uint32_t subMeshIdx = 0u;
	auto wasInitialized = false;
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto *uvSet = subMesh->GetUVSet("lightmap");
			if(uvSet == nullptr)
			{
				++subMeshIdx;
				continue;
			}
			auto lightMapReceiverC = ent.AddComponent<pragma::CLightMapReceiverComponent>();
			if(wasInitialized == false)
			{
				wasInitialized = true;
				lightMapReceiverC->m_modelName = ent.GetModelName();
				lightMapReceiverC->m_isModelBakedWithLightMaps = true;
			}
			lightMapReceiverC->m_uvDataPerMesh.insert(std::make_pair(subMeshIdx,*uvSet));
			lightMapReceiverC->m_meshes.insert(std::make_pair(subMeshIdx,subMesh));
			lightMapReceiverC->m_meshToMeshIdx.insert(std::make_pair(static_cast<CModelSubMesh*>(subMesh.get()),subMeshIdx));
			++subMeshIdx;
		}
	}
}
luabind::object CLightMapReceiverComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CLightMapReceiverComponentHandleWrapper>(l);}
void CLightMapReceiverComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<ComponentEvent> evData) {
		m_isModelBakedWithLightMaps = (GetEntity().GetModelName() == m_modelName);
		if(m_isModelBakedWithLightMaps)
			UpdateModelMeshes();
	});
}
void CLightMapReceiverComponent::UpdateModelMeshes()
{
	auto mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	m_meshes.clear();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0u) : nullptr;
	if(meshGroup == nullptr)
		return;
	std::unordered_map<MeshIdx,BufferIdx> meshIdxToBufIdx {};
	for(auto &pair : m_meshToMeshIdx)
	{
		auto *mesh = pair.first;
		auto it = m_meshToBufIdx.find(mesh);
		if(it == m_meshToBufIdx.end())
			continue;
		meshIdxToBufIdx.insert(std::make_pair(pair.second,it->second));
	}
	m_meshToBufIdx.clear();
	m_meshToMeshIdx.clear();
	uint32_t subMeshIdx = 0u;
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto *uvSet = subMesh->GetUVSet("lightmap");
			if(uvSet == nullptr)
			{
				++subMeshIdx;
				continue;
			}
			m_meshes.insert(std::make_pair(subMeshIdx,subMesh));
			auto itBufIdx = meshIdxToBufIdx.find(subMeshIdx);
			if(itBufIdx != meshIdxToBufIdx.end())
			{
				m_meshToBufIdx.insert(std::make_pair(static_cast<CModelSubMesh*>(subMesh.get()),itBufIdx->second));
				m_meshToMeshIdx.insert(std::make_pair(static_cast<CModelSubMesh*>(subMesh.get()),subMeshIdx));
			}
			++subMeshIdx;
		}
	}
}
const std::unordered_map<CLightMapReceiverComponent::MeshIdx,std::vector<Vector2>> &CLightMapReceiverComponent::GetMeshLightMapUvData() const {return m_uvDataPerMesh;}
void CLightMapReceiverComponent::AssignBufferIndex(MeshIdx meshIdx,BufferIdx bufIdx)
{
	auto itMesh = m_meshes.find(meshIdx);
	if(itMesh == m_meshes.end())
		return;
	m_meshToBufIdx.insert(std::make_pair(static_cast<CModelSubMesh*>(itMesh->second.get()),bufIdx));
}
std::optional<CLightMapReceiverComponent::BufferIdx> CLightMapReceiverComponent::FindBufferIndex(CModelSubMesh &mesh) const
{
	if(m_isModelBakedWithLightMaps == false)
		return {};
	auto it = m_meshToBufIdx.find(&mesh);
	if(it == m_meshToBufIdx.end())
		return {};
	return it->second;
}
