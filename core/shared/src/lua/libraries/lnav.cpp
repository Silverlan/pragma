/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/lua/libraries/lnav.hpp"
#include "pragma/ai/navsystem.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/libraries/lfile.h"
#include <luainterface.hpp>

extern DLLNETWORK Engine *engine;

lua_registercheck(NavConfig, pragma::nav::Config);

void Lua::nav::register_library(Lua::Interface &lua)
{
	auto &modNav = lua.RegisterLibrary("nav");
	modNav[luabind::def("generate", static_cast<void (*)(lua_State *)>([](lua_State *l) {
		auto &nw = *engine->GetNetworkState(l);
		auto &game = *nw.GetGameState();
		std::string err;
		std::shared_ptr<RcNavMesh> mesh = nullptr;
		auto &navConfig = *Lua::CheckNavConfig(l, 1);
		if(Lua::IsSet(l, 2) == false)
			mesh = pragma::nav::generate(game, navConfig, &err);
		else {
			if(Lua::IsType<BaseEntity>(l, 2)) {
				auto &ent = Lua::Check<BaseEntity>(l, 2);
				mesh = pragma::nav::generate(game, navConfig, ent, &err);
			}
			else {
				auto tVerts = 2;
				auto tIndices = 3;
				auto tAreas = 4;
				Lua::CheckTable(l, tVerts);
				Lua::CheckTable(l, tIndices);
				std::vector<Vector3> verts;
				std::vector<int32_t> indices;
				std::vector<pragma::nav::ConvexArea> areas;
				auto numVerts = Lua::GetObjectLength(l, tVerts);
				auto numIndices = Lua::GetObjectLength(l, tIndices);
				verts.reserve(numVerts);
				indices.reserve(numIndices);
				for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
					Lua::PushInt(l, i + 1u);
					Lua::GetTableValue(l, tVerts);
					auto &v = *Lua::CheckVector(l, -1);
					verts.push_back(v);
					Lua::Pop(l, 1);
				}

				for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i) {
					Lua::PushInt(l, i + 1u);
					Lua::GetTableValue(l, tIndices);
					auto idx = Lua::CheckInt(l, -1);
					indices.push_back(idx);
					Lua::Pop(l, 1);
				}

				if(Lua::IsSet(l, tAreas)) {
					Lua::CheckTable(l, tAreas);
					auto numAreaValues = Lua::GetObjectLength(l, tAreas);
					for(auto i = decltype(numAreaValues) {0u}; i < numAreaValues; ++i) {
						Lua::PushInt(l, i + 1u);
						Lua::GetTableValue(l, tAreas);
						Lua::CheckTable(l, -1);

						areas.push_back({});
						auto &area = areas.back();
						auto tArea = Lua::GetStackTop(l);

						Lua::PushString(l, "area");
						Lua::GetTableValue(l, tArea);
						area.area = Lua::CheckInt(l, -1);
						Lua::Pop(l, 1);

						Lua::PushString(l, "vertices");
						Lua::GetTableValue(l, tArea);
						Lua::CheckTable(l, -1);
						auto tVerts = Lua::GetStackTop(l);
						auto numVerts = Lua::GetObjectLength(l, tVerts);
						area.verts.reserve(numVerts);
						for(auto j = decltype(numVerts) {0u}; j < numVerts; ++j) {
							Lua::PushInt(l, j + 1u);
							Lua::GetTableValue(l, tVerts);
							area.verts.push_back(*Lua::CheckVector(l, -1));
							Lua::Pop(l, 1);
						}
						Lua::Pop(l, 1);

						Lua::Pop(l, 1);
					}
				}

				mesh = pragma::nav::generate(game, navConfig, verts, indices, &areas, &err);
			}
		}
		if(mesh == nullptr) {
			Lua::PushBool(l, false);
			Lua::PushString(l, err);
			return;
		}
		auto navMesh = pragma::nav::Mesh::Create(mesh, navConfig);
		Lua::Push(l, navMesh);
	})),
	  luabind::def("load", static_cast<opt<std::shared_ptr<pragma::nav::Mesh>> (*)(lua_State *)>([](lua_State *l) -> opt<std::shared_ptr<pragma::nav::Mesh>> {
		  auto &nw = *engine->GetNetworkState(l);
		  auto &game = *nw.GetGameState();
		  std::string fname = Lua::CheckString(l, 1);
		  pragma::nav::Config config;
		  auto mesh = pragma::nav::load(game, fname, config);
		  if(mesh == nullptr)
			  return nil;
		  return {l, pragma::nav::Mesh::Create(mesh, config)};
	  }))];
	Lua::RegisterLibraryEnums(lua.GetState(), "nav",
	  {{"POLY_TYPE_BIT_NONE", umath::to_integral(pragma::nav::PolyFlags::None)}, {"POLY_TYPE_BIT_WALK", umath::to_integral(pragma::nav::PolyFlags::Walk)}, {"POLY_TYPE_BIT_SWIM", umath::to_integral(pragma::nav::PolyFlags::Swim)},
	    {"POLY_TYPE_BIT_DOOR", umath::to_integral(pragma::nav::PolyFlags::Door)}, {"POLY_TYPE_BIT_JUMP", umath::to_integral(pragma::nav::PolyFlags::Jump)}, {"POLY_TYPE_BIT_DISABLED", umath::to_integral(pragma::nav::PolyFlags::Disabled)},
	    {"POLY_TYPE_ALL", umath::to_integral(pragma::nav::PolyFlags::All)}});

	auto classDefConfig = luabind::class_<pragma::nav::Config>("Config");
	classDefConfig.def(luabind::constructor<float, float, float, float>());
	classDefConfig.def(luabind::constructor<>());
	classDefConfig.def_readwrite("walkableRadius", &pragma::nav::Config::walkableRadius);
	classDefConfig.def_readwrite("characterHeight", &pragma::nav::Config::characterHeight);
	classDefConfig.def_readwrite("maxClimbHeight", &pragma::nav::Config::maxClimbHeight);
	classDefConfig.def_readwrite("walkableSlopeAngle", &pragma::nav::Config::walkableSlopeAngle);
	classDefConfig.def_readwrite("maxEdgeLength", &pragma::nav::Config::maxEdgeLength);
	classDefConfig.def_readwrite("maxSimplificationError", &pragma::nav::Config::maxSimplificationError);
	classDefConfig.def_readwrite("minRegionSize", &pragma::nav::Config::minRegionSize);
	classDefConfig.def_readwrite("mergeRegionSize", &pragma::nav::Config::mergeRegionSize);
	classDefConfig.def_readwrite("cellSize", &pragma::nav::Config::cellSize);
	classDefConfig.def_readwrite("cellHeight", &pragma::nav::Config::cellHeight);
	classDefConfig.def_readwrite("vertsPerPoly", &pragma::nav::Config::vertsPerPoly);
	classDefConfig.def_readwrite("sampleDetailDist", &pragma::nav::Config::sampleDetailDist);
	classDefConfig.def_readwrite("sampleDetailMaxError", &pragma::nav::Config::sampleDetailMaxError);
	classDefConfig.def_readwrite("samplePartitionType", reinterpret_cast<std::underlying_type_t<decltype(pragma::nav::Config::partitionType)> pragma::nav::Config::*>(&pragma::nav::Config::partitionType));
	classDefConfig.add_static_constant("PARTITION_TYPE_WATERSHED", umath::to_integral(pragma::nav::Config::PartitionType::Watershed));
	classDefConfig.add_static_constant("PARTITION_TYPE_MONOTONE", umath::to_integral(pragma::nav::Config::PartitionType::Monotone));
	classDefConfig.add_static_constant("PARTITION_TYPE_LAYERS", umath::to_integral(pragma::nav::Config::PartitionType::Layers));
	modNav[classDefConfig];

	auto classDefMesh = luabind::class_<pragma::nav::Mesh>("Mesh");
	classDefMesh.def("Save", static_cast<void (*)(lua_State *, pragma::nav::Mesh &, const std::string &)>([](lua_State *l, pragma::nav::Mesh &navMesh, const std::string &fname) {
		auto outName = fname;
		if(Lua::file::validate_write_operation(l, outName) == false) {
			Lua::PushBool(l, false);
			Lua::PushString(l, "This file operation is not allowed!");
			return;
		}
		auto &nw = *engine->GetNetworkState(l);
		auto &game = *nw.GetGameState();
		std::string err;
		auto r = navMesh.Save(game, outName, err);
		Lua::PushBool(l, r);
		if(r == false)
			Lua::PushString(l, err);
	}));
	/*classDefMesh.def("FindPath",static_cast<void(*)(lua_State*,pragma::nav::Mesh&,const Vector3&,const Vector3&)>([](lua_State *l,pragma::nav::Mesh &navMesh,const Vector3 &start,const Vector3 &end) {
		auto r = navMesh->FindPath(start,end);
		Lua::PushBool(l,r != nullptr);
		// TODO
	}));*/
	classDefMesh.def("RayCast", static_cast<void (*)(lua_State *, pragma::nav::Mesh &, const Vector3 &, const Vector3 &)>([](lua_State *l, pragma::nav::Mesh &navMesh, const Vector3 &start, const Vector3 &end) {
		Vector3 hit;
		auto r = navMesh.RayCast(start, end, hit);
		if(r == false)
			Lua::PushBool(l, r);
		else
			Lua::Push<Vector3>(l, hit);
	}));
	classDefMesh.def("GetConfig", static_cast<const pragma::nav::Config *(*)(lua_State *, pragma::nav::Mesh &)>([](lua_State *l, pragma::nav::Mesh &navMesh) -> const pragma::nav::Config * {
		auto &config = navMesh.GetConfig();
		return &config;
	}));
	modNav[classDefMesh];
}
