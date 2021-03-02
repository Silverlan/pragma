/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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