// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:rendering.entity_mesh_info;

export import :model.mesh;

#pragma warning(push)
#pragma warning(disable : 4251)
export class CBaseEntity;
export class DLLCLIENT EntityMeshInfo {
  public:
	EntityMeshInfo(CBaseEntity *ent) : entity(ent) {};
	EntityMeshInfo(const EntityMeshInfo &) = delete;
	EntityMeshInfo(EntityMeshInfo &&) = default;
	EntityMeshInfo &operator=(const EntityMeshInfo &other) = delete;
	CBaseEntity *entity;
	std::vector<CModelSubMesh *> meshes;
};
#pragma warning(pop)
