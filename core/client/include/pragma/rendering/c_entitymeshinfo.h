#ifndef __C_ENTITYMESHINFO_H__
#define __C_ENTITYMESHINFO_H__

#include "pragma/clientdefinitions.h"
#include <vector>

class CModelSubMesh;
class CBaseEntity;
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT EntityMeshInfo
{
public:
	EntityMeshInfo(CBaseEntity *ent) : entity(ent) {};
	EntityMeshInfo(const EntityMeshInfo&)=delete;
	EntityMeshInfo(EntityMeshInfo&&)=default;
	EntityMeshInfo &operator=(const EntityMeshInfo &other)=delete;
	CBaseEntity *entity;
	std::vector<CModelSubMesh*> meshes;
};
#pragma warning(pop)

#endif