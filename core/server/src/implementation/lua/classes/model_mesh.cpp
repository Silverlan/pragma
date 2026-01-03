// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.classes.model_mesh;

import pragma.shared;

void Lua::ModelMesh::Server::Create(lua::State *l) { Lua::Push<std::shared_ptr<pragma::geometry::ModelMesh>>(l, pragma::util::make_shared<pragma::geometry::ModelMesh>()); }

void Lua::ModelSubMesh::Server::Create(lua::State *l) { Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, pragma::util::make_shared<pragma::geometry::ModelSubMesh>()); }
