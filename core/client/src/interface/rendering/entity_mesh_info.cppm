// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.entity_mesh_info;

export import :model.mesh;

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace pragma::ecs {
	class CBaseEntity;
}
export namespace pragma::rendering {
	class DLLCLIENT EntityMeshInfo {
	  public:
		EntityMeshInfo(ecs::CBaseEntity *ent) : entity(ent) {};
		EntityMeshInfo(const EntityMeshInfo &) = delete;
		EntityMeshInfo(EntityMeshInfo &&) = default;
		EntityMeshInfo &operator=(const EntityMeshInfo &other) = delete;
		ecs::CBaseEntity *entity;
		std::vector<geometry::CModelSubMesh *> meshes;
	};
}
#pragma warning(pop)
