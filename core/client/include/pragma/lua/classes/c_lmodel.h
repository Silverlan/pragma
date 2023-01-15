/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LMODEL_H__
#define __C_LMODEL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/c_ldefinitions.h"

class CModel;
namespace pragma::asset {struct ModelExportInfo;};
namespace Lua
{
	namespace Model
	{
		namespace Client
		{
			DLLCLIENT void AddMaterial(lua_State *l,::Model &mdl,uint32_t textureGroup,const std::string &name);
			DLLCLIENT void SetMaterial(lua_State *l,::Model &mdl,uint32_t texIdx,const std::string &name);
			DLLCLIENT void GetVertexAnimationBuffer(lua_State *l,::Model &mdl);
			DLLCLIENT void Export(lua_State *l,::Model &mdl,const pragma::asset::ModelExportInfo &exportInfo);
			DLLCLIENT void ExportAnimation(lua_State *l,::Model &mdl,const std::string &animName,const pragma::asset::ModelExportInfo &exportInfo);

			DLLCLIENT std::shared_ptr<::Model> create_quad(lua_State *l,Game &game,float size);
			DLLCLIENT std::shared_ptr<::Model> create_box(lua_State *l,Game &game,const Vector3 &min,const Vector3 &max);
			DLLCLIENT std::shared_ptr<::Model> create_sphere(lua_State *l,Game &game,const Vector3 &origin,float radius,uint32_t recursionLevel);
			DLLCLIENT std::shared_ptr<::Model> create_sphere(lua_State *l,Game &game,const Vector3 &origin,float radius);
			DLLCLIENT std::shared_ptr<::Model> create_cylinder(lua_State *l,Game &game,float startRadius,float length,uint32_t segmentCount);
			DLLCLIENT std::shared_ptr<::Model> create_cone(lua_State *l,Game &game,float startRadius,float length,float endRadius,uint32_t segmentCount);
			DLLCLIENT std::shared_ptr<::Model> create_circle(lua_State *l,Game &game,float radius,bool doubleSided,uint32_t segmentCount);
			DLLCLIENT std::shared_ptr<::Model> create_ring(lua_State *l,Game &game,float innerRadius,float outerRadius,bool doubleSided,uint32_t segmentCount);
		};
	};
};

#endif