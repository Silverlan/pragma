/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/game/scene_snapshot.hpp"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"

using namespace pragma;

std::shared_ptr<SceneSnapshot> SceneSnapshot::Create() { return std::shared_ptr<SceneSnapshot> {new SceneSnapshot {}}; }

void SceneSnapshot::AddMaterial(Material &mat)
{
	auto it = std::find_if(m_materials.begin(), m_materials.end(), [&mat](const msys::MaterialHandle &hMat) { return hMat.get() == &mat; });
	if(it != m_materials.end())
		return;
	m_materials.push_back(mat.GetHandle());
}

void SceneSnapshot::AddModel(Model &mdl, uint32_t skin)
{
	for(auto &meshGroup : mdl.GetMeshGroups()) {
		for(auto &mesh : meshGroup->GetMeshes()) {
			for(auto &subMesh : mesh->GetSubMeshes()) {
				auto snapshotMesh = std::make_shared<Mesh>();

				auto &verts = subMesh->GetVertices();
				snapshotMesh->verts.reserve(verts.size());
				for(auto &v : subMesh->GetVertices()) {
					snapshotMesh->verts.push_back({});
					auto &vSnapshotMesh = snapshotMesh->verts.back();
					vSnapshotMesh.position = v.position;
					vSnapshotMesh.normal = v.normal;
					vSnapshotMesh.uv = v.uv;
				}

				auto *lightmapUvSet = subMesh->GetUVSet("lightmap");
				if(lightmapUvSet) {
					snapshotMesh->lightmapUvs.reserve(lightmapUvSet->size());
					for(auto &uv : *lightmapUvSet)
						snapshotMesh->lightmapUvs.push_back(uv);
				}

				subMesh->GetIndices(snapshotMesh->tris);
				auto matIdx = mdl.GetMaterialIndex(*subMesh, skin);
				auto *mat = matIdx.has_value() ? mdl.GetMaterial(*matIdx) : nullptr;
				if(mat == nullptr)
					continue;
				snapshotMesh->material = mat->GetHandle();
				if(snapshotMesh->material)
					AddMaterial(*snapshotMesh->material.get());
				m_meshes.push_back(snapshotMesh);
			}
		}
	}
}

void SceneSnapshot::MergeMeshesByMaterial()
{
	// Group meshes by material
	std::unordered_map<Material *, std::vector<Mesh *>> matMeshes {};
	for(auto &mesh : m_meshes) {
		auto *mat = mesh->material.get();
		auto it = matMeshes.find(mat);
		if(it == matMeshes.end())
			it = matMeshes.insert(std::make_pair(mat, std::vector<Mesh *> {})).first;
		it->second.push_back(mesh.get());
	}

	// Merge into first mesh per material and discard the others
	for(auto &pair : matMeshes) {
		auto &meshes = pair.second;
		if(meshes.size() < 2)
			continue;
		auto *meshFirst = meshes.front();
		for(auto i = decltype(meshes.size()) {1u}; i < meshes.size(); ++i) {
			auto *meshOther = meshes.at(i);
			auto vertOffset = meshFirst->verts.size();
			meshFirst->verts.reserve(meshFirst->verts.size() + meshOther->verts.size());
			for(auto &v : meshOther->verts)
				meshFirst->verts.push_back(v);

			if(meshOther->lightmapUvs.empty() == false) {
				meshFirst->lightmapUvs.resize(vertOffset + meshOther->lightmapUvs.size());
				for(auto i = decltype(meshOther->lightmapUvs.size()) {0u}; i < meshOther->lightmapUvs.size(); ++i)
					meshFirst->lightmapUvs.at(vertOffset + i) = meshOther->lightmapUvs.at(i);
			}

			meshFirst->tris.reserve(meshFirst->tris.size() + meshOther->tris.size());
			for(auto idx : meshOther->tris)
				meshFirst->tris.push_back(vertOffset + idx);

			auto itMeshOther = std::find_if(m_meshes.begin(), m_meshes.end(), [meshOther](const std::shared_ptr<Mesh> &mesh) { return mesh.get() == meshOther; });
			if(itMeshOther != m_meshes.end())
				m_meshes.erase(itMeshOther);
		}
	}
}

const std::vector<std::shared_ptr<SceneSnapshot::Mesh>> &SceneSnapshot::GetMeshes() const { return const_cast<SceneSnapshot *>(this)->GetMeshes(); }
std::vector<std::shared_ptr<SceneSnapshot::Mesh>> &SceneSnapshot::GetMeshes() { return m_meshes; }
const std::vector<std::shared_ptr<SceneSnapshot::Object>> &SceneSnapshot::GetObjects() const { return const_cast<SceneSnapshot *>(this)->GetObjects(); }
std::vector<std::shared_ptr<SceneSnapshot::Object>> &SceneSnapshot::GetObjects() { return m_objects; }
const std::vector<msys::MaterialHandle> &SceneSnapshot::GetMaterials() const { return const_cast<SceneSnapshot *>(this)->GetMaterials(); }
std::vector<msys::MaterialHandle> &SceneSnapshot::GetMaterials() { return m_materials; }
