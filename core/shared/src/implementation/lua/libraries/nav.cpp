// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.nav;

void Lua::nav::register_library(Interface &lua)
{
	auto &modNav = lua.RegisterLibrary("nav");
	modNav[(luabind::def("generate", static_cast<void (*)(lua::State *)>([](lua::State *l) {
		auto &nw = *pragma::Engine::Get()->GetNetworkState(l);
		auto &game = *nw.GetGameState();
		std::string err;
		std::shared_ptr<RcNavMesh> mesh = nullptr;
		auto &navConfig = Lua::Check<pragma::nav::Config>(l, 1);
		if(IsSet(l, 2) == false)
			mesh = pragma::nav::generate(game, navConfig, &err);
		else {
			if(Lua::IsType<pragma::ecs::BaseEntity>(l, 2)) {
				auto &ent = Lua::Check<pragma::ecs::BaseEntity>(l, 2);
				mesh = pragma::nav::generate(game, navConfig, ent, &err);
			}
			else {
				auto tVerts = 2;
				auto tIndices = 3;
				auto tAreas = 4;
				CheckTable(l, tVerts);
				CheckTable(l, tIndices);
				std::vector<Vector3> verts;
				std::vector<int32_t> indices;
				std::vector<pragma::nav::ConvexArea> areas;
				auto numVerts = GetObjectLength(l, tVerts);
				auto numIndices = GetObjectLength(l, tIndices);
				verts.reserve(numVerts);
				indices.reserve(numIndices);
				for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
					PushInt(l, i + 1u);
					GetTableValue(l, tVerts);
					auto &v = Lua::Check<Vector3>(l, -1);
					verts.push_back(v);
					Pop(l, 1);
				}

				for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i) {
					PushInt(l, i + 1u);
					GetTableValue(l, tIndices);
					auto idx = CheckInt(l, -1);
					indices.push_back(idx);
					Pop(l, 1);
				}

				if(IsSet(l, tAreas)) {
					CheckTable(l, tAreas);
					auto numAreaValues = GetObjectLength(l, tAreas);
					for(auto i = decltype(numAreaValues) {0u}; i < numAreaValues; ++i) {
						PushInt(l, i + 1u);
						GetTableValue(l, tAreas);
						CheckTable(l, -1);

						areas.push_back({});
						auto &area = areas.back();
						auto tArea = GetStackTop(l);

						PushString(l, "area");
						GetTableValue(l, tArea);
						area.area = CheckInt(l, -1);
						Pop(l, 1);

						PushString(l, "vertices");
						GetTableValue(l, tArea);
						CheckTable(l, -1);
						auto tVerts = GetStackTop(l);
						auto numVerts = GetObjectLength(l, tVerts);
						area.verts.reserve(numVerts);
						for(auto j = decltype(numVerts) {0u}; j < numVerts; ++j) {
							PushInt(l, j + 1u);
							GetTableValue(l, tVerts);
							area.verts.push_back(Lua::Check<Vector3>(l, -1));
							Pop(l, 1);
						}
						Pop(l, 1);

						Pop(l, 1);
					}
				}

				mesh = pragma::nav::generate(game, navConfig, verts, indices, &areas, &err);
			}
		}
		if(mesh == nullptr) {
			PushBool(l, false);
			PushString(l, err);
			return;
		}
		auto navMesh = pragma::nav::Mesh::Create(mesh, navConfig);
		Push(l, navMesh);
	})),
	  luabind::def("load", static_cast<opt<std::shared_ptr<pragma::nav::Mesh>> (*)(lua::State *)>([](lua::State *l) -> opt<std::shared_ptr<pragma::nav::Mesh>> {
		  auto &nw = *pragma::Engine::Get()->GetNetworkState(l);
		  auto &game = *nw.GetGameState();
		  std::string fname = CheckString(l, 1);
		  pragma::nav::Config config;
		  auto mesh = pragma::nav::load(game, fname, config);
		  if(mesh == nullptr)
			  return nil;
		  return {l, pragma::nav::Mesh::Create(mesh, config)};
	  })))];
	RegisterLibraryEnums(lua.GetState(), "nav",
	  {{"POLY_TYPE_BIT_NONE", pragma::math::to_integral(pragma::nav::PolyFlags::None)}, {"POLY_TYPE_BIT_WALK", pragma::math::to_integral(pragma::nav::PolyFlags::Walk)}, {"POLY_TYPE_BIT_SWIM", pragma::math::to_integral(pragma::nav::PolyFlags::Swim)},
	    {"POLY_TYPE_BIT_DOOR", pragma::math::to_integral(pragma::nav::PolyFlags::Door)}, {"POLY_TYPE_BIT_JUMP", pragma::math::to_integral(pragma::nav::PolyFlags::Jump)}, {"POLY_TYPE_BIT_DISABLED", pragma::math::to_integral(pragma::nav::PolyFlags::Disabled)},
	    {"POLY_TYPE_ALL", pragma::math::to_integral(pragma::nav::PolyFlags::All)}});

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
	classDefConfig.add_static_constant("PARTITION_TYPE_WATERSHED", pragma::math::to_integral(pragma::nav::Config::PartitionType::Watershed));
	classDefConfig.add_static_constant("PARTITION_TYPE_MONOTONE", pragma::math::to_integral(pragma::nav::Config::PartitionType::Monotone));
	classDefConfig.add_static_constant("PARTITION_TYPE_LAYERS", pragma::math::to_integral(pragma::nav::Config::PartitionType::Layers));
	modNav[classDefConfig];

	auto classDefMesh = luabind::class_<pragma::nav::Mesh>("Mesh");
	classDefMesh.def("Save", static_cast<void (*)(lua::State *, pragma::nav::Mesh &, const std::string &)>([](lua::State *l, pragma::nav::Mesh &navMesh, const std::string &fname) {
		auto outName = fname;
		if(file::validate_write_operation(l, outName) == false) {
			PushBool(l, false);
			PushString(l, "This file operation is not allowed!");
			return;
		}
		auto &nw = *pragma::Engine::Get()->GetNetworkState(l);
		auto &game = *nw.GetGameState();
		std::string err;
		auto r = navMesh.Save(game, outName, err);
		PushBool(l, r);
		if(r == false)
			PushString(l, err);
	}));
	/*classDefMesh.def("FindPath",static_cast<void(*)(lua::State*,pragma::nav::Mesh&,const Vector3&,const Vector3&)>([](lua::State *l,pragma::nav::Mesh &navMesh,const Vector3 &start,const Vector3 &end) {
		auto r = navMesh->FindPath(start,end);
		Lua::PushBool(l,r != nullptr);
		// TODO
	}));*/
	classDefMesh.def("RayCast", static_cast<void (*)(lua::State *, pragma::nav::Mesh &, const Vector3 &, const Vector3 &)>([](lua::State *l, pragma::nav::Mesh &navMesh, const Vector3 &start, const Vector3 &end) {
		Vector3 hit;
		auto r = navMesh.RayCast(start, end, hit);
		if(r == false)
			PushBool(l, r);
		else
			Lua::Push<Vector3>(l, hit);
	}));
	classDefMesh.def("GetConfig", static_cast<const pragma::nav::Config *(*)(lua::State *, pragma::nav::Mesh &)>([](lua::State *l, pragma::nav::Mesh &navMesh) -> const pragma::nav::Config * {
		auto &config = navMesh.GetConfig();
		return &config;
	}));
	modNav[classDefMesh];
}
