// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.classes.model;
export import :assets;

export namespace Lua {
	namespace Model {
		namespace Client {
			DLLCLIENT void AddMaterial(lua::State *l, pragma::asset::Model &mdl, uint32_t textureGroup, const std::string &name);
			DLLCLIENT void SetMaterial(lua::State *l, pragma::asset::Model &mdl, uint32_t texIdx, const std::string &name);
			DLLCLIENT void GetVertexAnimationBuffer(lua::State *l, pragma::asset::Model &mdl);
			DLLCLIENT void Export(lua::State *l, pragma::asset::Model &mdl, const pragma::asset::ModelExportInfo &exportInfo);
			DLLCLIENT void ExportAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &animName, const pragma::asset::ModelExportInfo &exportInfo);
			DLLCLIENT std::shared_ptr<pragma::asset::Model> create_generic_model(pragma::Game &game, pragma::geometry::ModelSubMesh &subMesh);
		};
	};
};
