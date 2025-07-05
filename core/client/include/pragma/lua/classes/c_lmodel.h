// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LMODEL_H__
#define __C_LMODEL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/c_ldefinitions.h"

class CModel;
namespace pragma::asset {
	struct ModelExportInfo;
};
namespace Lua {
	namespace Model {
		namespace Client {
			DLLCLIENT void AddMaterial(lua_State *l, ::Model &mdl, uint32_t textureGroup, const std::string &name);
			DLLCLIENT void SetMaterial(lua_State *l, ::Model &mdl, uint32_t texIdx, const std::string &name);
			DLLCLIENT void GetVertexAnimationBuffer(lua_State *l, ::Model &mdl);
			DLLCLIENT void Export(lua_State *l, ::Model &mdl, const pragma::asset::ModelExportInfo &exportInfo);
			DLLCLIENT void ExportAnimation(lua_State *l, ::Model &mdl, const std::string &animName, const pragma::asset::ModelExportInfo &exportInfo);
			DLLCLIENT std::shared_ptr<::Model> create_generic_model(Game &game,::ModelSubMesh &subMesh);
		};
	};
};

#endif
