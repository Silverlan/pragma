// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.server;
import :scripting.lua.classes.model_mesh;

import pragma.shared;

void Lua::ModelMesh::Server::Create(lua::State *l) { Lua::Push<std::shared_ptr<::ModelMesh>>(l, std::make_shared<::ModelMesh>()); }

void Lua::ModelSubMesh::Server::Create(lua::State *l) { Lua::Push<std::shared_ptr<pragma::ModelSubMesh>>(l, std::make_shared<pragma::ModelSubMesh>()); }
