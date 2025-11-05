// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:scripting.lua.classes.model;
export import :assets;

export namespace Lua {
	namespace Model {
		namespace Client {
			DLLCLIENT void AddMaterial(lua_State *l, pragma::Model &mdl, uint32_t textureGroup, const std::string &name);
			DLLCLIENT void SetMaterial(lua_State *l, pragma::Model &mdl, uint32_t texIdx, const std::string &name);
			DLLCLIENT void GetVertexAnimationBuffer(lua_State *l, pragma::Model &mdl);
			DLLCLIENT void Export(lua_State *l, pragma::Model &mdl, const pragma::asset::ModelExportInfo &exportInfo);
			DLLCLIENT void ExportAnimation(lua_State *l, pragma::Model &mdl, const std::string &animName, const pragma::asset::ModelExportInfo &exportInfo);
			DLLCLIENT std::shared_ptr<pragma::Model> create_generic_model(pragma::Game &game, pragma::ModelSubMesh &subMesh);
		};
	};
};
