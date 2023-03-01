/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/sortedrendermeshcontainer.h"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/model/model.h>

EntityMeshContainer::EntityMeshContainer(CBaseEntity *ent) : entity(ent), m_end(m_meshes.begin()) {}
void EntityMeshContainer::Insert(CModelSubMesh *mesh, std::vector<SortedEntityMeshIteratorInfo> &locations)
{
	if(m_meshes.size() == m_meshes.capacity())
		m_meshes.reserve(m_meshes.capacity() + 4);
	if(!m_freeIndices.empty()) {
		auto idx = m_freeIndices.front();
		m_freeIndices.pop();
		m_meshes[idx] = mesh;
		locations.push_back(SortedEntityMeshIteratorInfo(m_meshes, idx));
		if(m_freeIndices.empty()) {
			auto sz = m_meshes.size();
			for(auto it = m_meshes.rbegin(); it != m_meshes.rend(); ++it) {
				if(*it == nullptr)
					sz--;
			}
			m_meshes.resize(sz);
		}
		UpdateLastIterator();
		return;
	}
	m_meshes.push_back(mesh);
	locations.push_back(SortedEntityMeshIteratorInfo(m_meshes, CUInt32(m_meshes.size() - 1)));
	UpdateLastIterator();
}

void EntityMeshContainer::UpdateLastIterator()
{
	for(auto i = m_meshes.size(); i > 0;) {
		i--;
		if(m_meshes[i] != nullptr) {
			m_end = m_meshes.begin();
			++m_end;
			return;
		}
	}
	m_end = m_meshes.begin();
}

void EntityMeshContainer::Erase(UInt32 idx)
{
	m_freeIndices.push(idx);
	m_meshes[idx] = nullptr;
	UpdateLastIterator();
}

CSubMeshIterator EntityMeshContainer::begin() { return m_meshes.begin(); }
CSubMeshIterator EntityMeshContainer::end() { return m_meshes.end(); } //m_end;}

////////////////////////////////////

SortedMaterialMeshContainer::SortedMaterialMeshContainer(Material *mat) : material(mat) {}
void SortedMaterialMeshContainer::Insert(CBaseEntity *ent, std::vector<SortedEntityMeshIteratorInfo> &locations, CModelSubMesh *mesh)
{
	auto itEnt = std::find_if(m_ents.begin(), m_ents.end(), [ent](const std::unique_ptr<EntityMeshContainer> &c) { return (c->entity == ent) ? true : false; });
	if(itEnt == m_ents.end()) {
		if(m_ents.size() == m_ents.capacity())
			m_ents.reserve(m_ents.capacity() + 10);
		m_ents.push_back(std::make_unique<EntityMeshContainer>(ent));
		itEnt = m_ents.end();
		--itEnt;
	}
	auto &entContainer = *itEnt;
	entContainer->Insert(mesh, locations);
}

SortedEntityMeshIterator SortedMaterialMeshContainer::begin() { return m_ents.begin(); }
SortedEntityMeshIterator SortedMaterialMeshContainer::end() { return m_ents.end(); }

////////////////////////////////////

SortedShaderMeshContainer::SortedShaderMeshContainer(Shader::TexturedBase3D *_shader) : shader(_shader) {}

void SortedShaderMeshContainer::Insert(CBaseEntity *ent, std::vector<SortedEntityMeshIteratorInfo> &locations, CModelSubMesh *mesh, Material *mat)
{
	auto itMat = std::find_if(m_materials.begin(), m_materials.end(), [mat](const std::unique_ptr<SortedMaterialMeshContainer> &c) { return (c->material == mat) ? true : false; });
	if(itMat == m_materials.end()) {
		if(m_materials.size() == m_materials.capacity())
			m_materials.reserve(m_materials.capacity() + 20);
		m_materials.push_back(std::make_unique<SortedMaterialMeshContainer>(mat));
		itMat = m_materials.end();
		--itMat;
	}
	auto &materialContainer = *itMat;
	materialContainer->Insert(ent, locations, mesh);
}

SortedMaterialMeshIterator SortedShaderMeshContainer::begin() { return m_materials.begin(); }
SortedMaterialMeshIterator SortedShaderMeshContainer::end() { return m_materials.end(); }

////////////////////////////////////

std::vector<std::unique_ptr<SortedShaderMeshContainer>> SortedRenderMeshContainer::m_shaders;
std::vector<SortedRenderInfo> SortedRenderMeshContainer::m_sortedData;

void SortedRenderMeshContainer::Insert(CBaseEntity *ent, CModelSubMesh *mesh, Material *mat, Shader::TexturedBase3D *shader)
{
	auto itShader = std::find_if(m_shaders.begin(), m_shaders.end(), [shader](const std::unique_ptr<SortedShaderMeshContainer> &c) { return (c->shader == shader) ? true : false; });
	if(itShader == m_shaders.end()) {
		if(m_shaders.size() == m_shaders.capacity())
			m_shaders.reserve(m_shaders.capacity() + 50);
		m_shaders.push_back(std::make_unique<SortedShaderMeshContainer>(shader));
		itShader = m_shaders.end();
		--itShader;
	}
	auto &shaderContainer = *itShader;
	shaderContainer->Insert(ent, m_meshLocations, mesh, mat);

	auto idx = FindDataIndex(shader, mat);
	m_sortedData.insert(m_sortedData.begin() + idx, SortedRenderInfo(shader, mat, ent, mesh)); // TODO Erase this somehow when the entity / mesh is removed / updated
}

UInt SortedRenderMeshContainer::FindDataIndex(Shader::TexturedBase3D *shader, Material *mat)
{
	auto bFoundShader = false;
	for(UInt i = 0; i < m_sortedData.size(); i++) {
		auto &info = m_sortedData[i];
		if(bFoundShader == true || info.shader == shader) {
			bFoundShader = true;
			if(info.material == mat)
				return i;
		}
		else if(bFoundShader == true)
			return i - 1;
	}
	return 0;
}

SortedRenderMeshContainer::SortedRenderMeshContainer(CBaseEntity *ent, std::vector<std::shared_ptr<ModelMesh>> &meshes)
{
	if(true)
		return;
	auto mdlComponent = ent->GetModelComponent();
	auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
	assert(mdl != nullptr);
	auto &materials = mdl->GetMaterials();
	if(materials.empty())
		return;
	for(auto it = meshes.begin(); it != meshes.end(); ++it) {
		auto *mesh = static_cast<CModelMesh *>(it->get());
		auto &subMeshes = mesh->GetSubMeshes();
		for(auto it = subMeshes.begin(); it != subMeshes.end(); ++it) {
			auto *subMesh = static_cast<CModelSubMesh *>(it->get());
			auto idxTexture = mdl->GetMaterialIndex(*subMesh, ent->GetSkin());
			auto hMat = idxTexture.has_value() ? materials[*idxTexture] : msys::MaterialHandle {};
			if(hMat) {
				auto *mat = hMat.get();
				auto *info = mat->GetShaderInfo();
				if(info != nullptr) {
					/*auto *base = info->GetShader<ShaderBase>();
					auto *shader = (base != nullptr) ? base->Get3DTexturedShader() : nullptr;
					if(shader == nullptr)
						;//Con::cerr<<"NULL shader for material "<<mat->GetDiffuseMap()->name<<Con::endl; // TODO This should never happen!
					else
						Insert(ent,subMesh,mat,shader);*/ // Vulkan TODO
				}
			}
		}
	}
}

SortedRenderMeshContainer::~SortedRenderMeshContainer()
{
	for(auto it = m_meshLocations.begin(); it != m_meshLocations.end(); ++it)
		it->Erase();
}

SortedRenderInfoIterator SortedRenderMeshContainer::begin() { return m_sortedData.begin(); }
SortedRenderInfoIterator SortedRenderMeshContainer::end() { return m_sortedData.end(); }

////////////////////////////////////

SortedRenderInfo::SortedRenderInfo(Shader::TexturedBase3D *_shader, Material *_material, CBaseEntity *_entity, CModelSubMesh *_mesh) : shader(_shader), material(_material), entity(_entity), mesh(_mesh) {}
