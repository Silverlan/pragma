/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lutil.hpp"
#include "pragma/lua/util.hpp"
#include <pragma/engine.h>
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/lua/classes/ldef_damageinfo.h"
#include "pragma/lua/classes/ldef_angle.h"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/lua/libraries/lray.h"
#include "pragma/lua/custom_constructor.hpp"
#include "pragma/lua/class_manager.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/policies/core_policies.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/util/render_tile.hpp"
#include "pragma/lua/ostream_operator_alias.hpp"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/asset_types/world.hpp"
#include "pragma/util/functional_parallel_worker.hpp"
#include "pragma/util/rig_config.hpp"
#include <pragma/game/game.h>
#include "luasystem.h"
#include "pragma/util/util_python.hpp"
#include "pragma/game/damageinfo.h"
#include "pragma/model/model.h"
#include "pragma/physics/raytraces.h"
#include "pragma/entities/environment/env_quake.h"
#include "pragma/physics/collisionmasks.h"
#include "pragma/util/util_game.hpp"
#include "pragma/util/bulletinfo.h"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_flex_component.hpp"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/util/util_splash_damage_info.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/util/util_rgbcsv.hpp"
#include "pragma/util/util_variable_type.hpp"
#include "pragma/lua/classes/parallel_job.hpp"
#include <util_image_buffer.hpp>
#include <sharedutils/netpacket.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/scope_guard.h>
#include <luainterface.hpp>
#include <pragma/math/intersection.h>
#include <pragma/model/modelmesh.h>
#include "pragma/model/animation/skeleton.hpp"
#include <luabind/class_info.hpp>
#include <fsys/ifile.hpp>
#include <sharedutils/util_markup_file.hpp>

import se_script;
import util_zip;

extern DLLNETWORK Engine *engine;

static auto s_bIgnoreIncludeCache = false;
void Lua::set_ignore_include_cache(bool b) { s_bIgnoreIncludeCache = b; }

std::optional<std::string> Lua::find_script_file(const std::string &fileName)
{
	auto raw = fileName + Lua::DOT_FILE_EXTENSION;
	if(filemanager::exists(Lua::SCRIPT_DIRECTORY_SLASH + raw))
		return raw;
	auto precompiled = fileName + Lua::DOT_FILE_EXTENSION_PRECOMPILED;
	if(filemanager::exists(Lua::SCRIPT_DIRECTORY_SLASH + precompiled))
		return precompiled;
	return {};
}

luabind::detail::class_rep *Lua::get_crep(luabind::object o)
{
	auto *L = o.interpreter();
	luabind::detail::class_rep *crep = nullptr;

	o.push(L);
	if(luabind::detail::is_class_rep(L, -1)) {
		crep = static_cast<luabind::detail::class_rep *>(lua_touserdata(L, -1));
		lua_pop(L, 1);
	}
	else {
		auto *obj = luabind::detail::get_instance(L, -1);
		if(!obj)
			lua_pop(L, 1);
		else {
			lua_pop(L, 1);
			// OK, we were given an object - gotta get the crep.
			crep = obj->crep();
		}
	}
	return crep;
}

#include <sharedutils/util_hair.hpp>
static Vector3 calc_hair_normal(const Vector3 &flowNormal, const Vector3 &faceNormal)
{
	auto hairNormal = flowNormal - uvec::project(flowNormal, faceNormal);
	auto l = uvec::length(hairNormal);
	if(l > 0.001f)
		hairNormal /= l;
	else
		hairNormal = faceNormal;
	return hairNormal;
}

static Vector3 apply_curvature(const Vector3 &baseHairDir, const Vector3 &surfaceTargetNormal, float curvature, float factor)
{
	if(uvec::distance(baseHairDir, surfaceTargetNormal) < 0.001f)
		return baseHairDir;
	auto f = factor * curvature;
	auto n = glm::slerp(baseHairDir, surfaceTargetNormal, f);
	uvec::normalize(&n); // Probably not needed
	return n;
}

static bool save_hair_strand_data(const util::HairStrandData &strandData, udm::AssetDataArg outData, std::string &outErr)
{
	outData.SetAssetType("PHD");
	outData.SetAssetVersion(1);

	auto udm = *outData;
	udm["strandCount"] = strandData.hairSegments.size();
	udm.AddArray("segmentCounts", strandData.hairSegments, udm::ArrayType::Compressed);
	auto udmStrands = udm["strands"];
	udmStrands.AddArray("points", strandData.points, udm::ArrayType::Compressed);
	udmStrands.AddArray("uvs", strandData.uvs, udm::ArrayType::Compressed);
	udmStrands.AddArray("thickness", strandData.thicknessData, udm::ArrayType::Compressed);
	return true;
}
static bool load_hair_strand_data(util::HairStrandData &strandData, const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != "PHD" || data.GetAssetVersion() < 1)
		return false;
	auto udm = *data;
	uint32_t numStrands = 0;
	udm["strandCount"](numStrands);
	udm["segmentCounts"](strandData.hairSegments);
	auto udmStrands = udm["strands"];
	udmStrands["points"](strandData.points);
	udmStrands["uvs"](strandData.uvs);
	udmStrands["thickness"](strandData.thicknessData);
	return true;
}

static std::unique_ptr<util::HairStrandData> generate_hair_file(const util::HairConfig &hairConfig, const util::HairData &hairData)
{
	auto numHair = hairData.hairPoints.size();
	auto numSegments = hairConfig.numSegments;
	auto numPoints = numHair * (numSegments + 1);
	auto hairFile = std::make_unique<util::HairStrandData>();

	auto &lxUvs = hairFile->uvs;
	auto &thicknessData = hairFile->thicknessData;
	auto &points = hairFile->points;
	auto &hairSegments = hairFile->hairSegments;
	points.resize(numPoints);
	lxUvs.resize(numPoints);
	thicknessData.resize(numPoints);
	uint32_t hairIdx = 0;
	auto fAddHairPoint = [&hairIdx, &points, &lxUvs, &thicknessData](const Vector3 &p, const Vector2 &uv, float thickness) {
		points[hairIdx] = {p.x, p.y, p.z};
		lxUvs[hairIdx] = {uv.x, uv.y};
		thicknessData[hairIdx] = thickness;

		++hairIdx;
	};
	hairSegments.resize(hairData.hairPoints.size());
	for(auto i = decltype(hairData.hairPoints.size()) {0u}; i < hairData.hairPoints.size(); ++i) {
		auto &p = hairData.hairPoints[i];
		auto &uv = hairData.hairUvs[i];
		auto &faceNormal = hairData.hairNormals[i];

		auto hairStrength = hairConfig.defaultHairStrength;
		auto length = hairConfig.defaultLength;
		auto randomHairLengthFactor = hairConfig.randomHairLengthFactor;
		auto thickness = hairConfig.defaultThickness;

		Vector3 gravity {0.f, -1.f, 0.f};
		auto lxGravity = gravity;
		gravity = {lxGravity.x, lxGravity.y, lxGravity.z};
		const Vector3 flowNormal = gravity;
		auto baseHairNormal = calc_hair_normal(flowNormal, faceNormal);
		auto hairNormal = faceNormal * hairStrength + (1.f - hairStrength) * baseHairNormal;
		uvec::normalize(&hairNormal);

		//hairPoints.push_back(p);
		//hairPoints.push_back(p +n *length); // TODO: Take adjacent face normals into account for hair direction?
		auto hairLength = length * (1.f - randomHairLengthFactor) + length * randomHairLengthFactor * umath::random(0.f, 1.f);

		fAddHairPoint(p, uv, thickness);
		auto lenPerSegment = hairLength / static_cast<float>(numSegments);
		auto p0 = p;
		hairSegments[i] = numSegments;
		for(auto j = decltype(numSegments) {0u}; j < numSegments; ++j) {
			auto f = (j + 1) / static_cast<float>(numSegments);

			auto n = apply_curvature(hairNormal, baseHairNormal, hairConfig.curvature, f);
			//auto p0 = (j > 0) ? hairPoints.back() : p;
			auto p1 = p0 + n * lenPerSegment;
			fAddHairPoint(p1, uv, (1.f - f) * thickness);
			p0 = p1;
		}
	}
	return hairFile;
}

void Lua::util::register_world_data(lua_State *l, luabind::module_ &mod)
{
	auto defWorldData = luabind::class_<pragma::asset::WorldData>("WorldData");
	defWorldData.def(luabind::tostring(luabind::self));
	defWorldData.scope[luabind::def(
	  "load", +[](NetworkState &nw, const std::string &fileName) -> std::pair<std::shared_ptr<pragma::asset::WorldData>, std::optional<std::string>> {
		  std::string err;
		  auto worldData = pragma::asset::WorldData::load(nw, fileName, err);
		  if(!worldData)
			  return {worldData, err};
		  return {worldData, {}};
	  })];
	defWorldData.scope[luabind::def(
	  "load_from_udm_data", +[](NetworkState &nw, udm::LinkedPropertyWrapper &prop) -> std::pair<std::shared_ptr<pragma::asset::WorldData>, std::optional<std::string>> {
		  std::string err;
		  auto worldData = pragma::asset::WorldData::load_from_udm_data(nw, prop, err);
		  if(!worldData)
			  return {worldData, err};
		  return {worldData, {}};
	  })];

	auto defOutput = luabind::class_<pragma::asset::Output>("Output");
	defOutput.def(luabind::constructor<>());
	defOutput.def(luabind::tostring(luabind::self));
	defOutput.def_readwrite("name", &pragma::asset::Output::name);
	defOutput.def_readwrite("target", &pragma::asset::Output::target);
	defOutput.def_readwrite("input", &pragma::asset::Output::input);
	defOutput.def_readwrite("param", &pragma::asset::Output::param);
	defOutput.def_readwrite("delay", &pragma::asset::Output::delay);
	defOutput.def_readwrite("name", &pragma::asset::Output::times);
	defWorldData.scope[defOutput];

	auto defComponentData = luabind::class_<pragma::asset::ComponentData>("ComponentData");
	defComponentData.def(luabind::tostring(luabind::self));
	defComponentData.add_static_constant("FLAG_NONE", umath::to_integral(pragma::asset::ComponentData::Flags::None));
	defComponentData.add_static_constant("FLAG_CLIENTSIDE_ONLY_BIT", umath::to_integral(pragma::asset::ComponentData::Flags::ClientsideOnly));
	defComponentData.def("GetFlags", &pragma::asset::ComponentData::GetFlags);
	defComponentData.def("SetFlags", &pragma::asset::ComponentData::SetFlags);
	defComponentData.def(
	  "GetData", +[](pragma::asset::ComponentData &componentData) -> udm::LinkedPropertyWrapper { return udm::LinkedPropertyWrapper {*componentData.GetData()}; });
	defWorldData.scope[defComponentData];

	auto defEntityData = luabind::class_<pragma::asset::EntityData>("EntityData");
	defEntityData.def(luabind::tostring(luabind::self));
	defEntityData.add_static_constant("FLAG_NONE", umath::to_integral(pragma::asset::EntityData::Flags::None));
	defEntityData.add_static_constant("FLAG_CLIENTSIDE_ONLY_BIT", umath::to_integral(pragma::asset::EntityData::Flags::ClientsideOnly));
	defEntityData.def("IsWorld", &pragma::asset::EntityData::IsWorld);
	defEntityData.def("IsSkybox", &pragma::asset::EntityData::IsSkybox);
	defEntityData.def("IsClientSideOnly", &pragma::asset::EntityData::IsClientSideOnly);
	defEntityData.def("SetClassName", &pragma::asset::EntityData::SetClassName);
	defEntityData.def("SetPose", &pragma::asset::EntityData::SetPose);
	defEntityData.def("GetPose", &pragma::asset::EntityData::GetPose);
	defEntityData.def("GetEffectivePose", &pragma::asset::EntityData::GetEffectivePose);
	defEntityData.def("SetLeafData", &pragma::asset::EntityData::SetLeafData);
	defEntityData.def("SetKeyValue", &pragma::asset::EntityData::SetKeyValue);
	defEntityData.def("AddOutput", &pragma::asset::EntityData::AddOutput);
	defEntityData.def("GetMapIndex", &pragma::asset::EntityData::GetMapIndex);
	defEntityData.def("GetClassName", &pragma::asset::EntityData::GetClassName);
	defEntityData.def("GetFlags", &pragma::asset::EntityData::GetFlags);
	defEntityData.def("SetFlags", &pragma::asset::EntityData::SetFlags);
	defEntityData.def("AddComponent", &pragma::asset::EntityData::AddComponent);
	defEntityData.def("GetComponents", static_cast<const std::unordered_map<std::string, std::shared_ptr<pragma::asset::ComponentData>> &(pragma::asset::EntityData ::*)() const>(&pragma::asset::EntityData::GetComponents));
	defEntityData.def("GetKeyValues", static_cast<const std::unordered_map<std::string, std::string> &(pragma::asset::EntityData ::*)() const>(&pragma::asset::EntityData::GetKeyValues));
	defEntityData.def("GetKeyValue", static_cast<std::optional<std::string> (pragma::asset::EntityData ::*)(const std::string &) const>(&pragma::asset::EntityData::GetKeyValue));
	defEntityData.def("GetKeyValue", static_cast<std::string (pragma::asset::EntityData ::*)(const std::string &, const std::string &) const>(&pragma::asset::EntityData::GetKeyValue));
	defEntityData.def("GetOutputs", static_cast<const std::vector<pragma::asset::Output> &(pragma::asset::EntityData ::*)() const>(&pragma::asset::EntityData::GetOutputs));
	defEntityData.def("GetLeaves", static_cast<const std::vector<uint16_t> &(pragma::asset::EntityData ::*)() const>(&pragma::asset::EntityData::GetLeaves));
	defEntityData.def(
	  "GetLeafData", +[](const pragma::asset::EntityData &entData) -> std::pair<uint32_t, uint32_t> {
		  uint32_t firstLeaf;
		  uint32_t numLeaves;
		  entData.GetLeafData(firstLeaf, numLeaves);
		  return {firstLeaf, numLeaves};
	  });
	defWorldData.scope[defEntityData];

	defWorldData.add_static_constant("DATA_FLAG_NONE", umath::to_integral(pragma::asset::WorldData::DataFlags::None));
	defWorldData.add_static_constant("DATA_FLAG_HAS_LIGHTMAP_ATLAS_BIT", umath::to_integral(pragma::asset::WorldData::DataFlags::HasLightmapAtlas));
	defWorldData.add_static_constant("DATA_FLAG_HAS_BSP_TREE_BIT", umath::to_integral(pragma::asset::WorldData::DataFlags::HasBSPTree));
	defWorldData.def("AddEntity", &pragma::asset::WorldData::AddEntity);
	defWorldData.def("AddEntity", &pragma::asset::WorldData::AddEntity, luabind::default_parameter_policy<3, false> {});
	defWorldData.def("SetLightMapAtlas", &pragma::asset::WorldData::SetLightMapAtlas);
	defWorldData.def("SetLightMapEnabled", &pragma::asset::WorldData::SetLightMapEnabled);
	defWorldData.def("SetLightMapIntensity", &pragma::asset::WorldData::SetLightMapIntensity);
	defWorldData.def("SetLightMapExposure", &pragma::asset::WorldData::SetLightMapExposure);
	defWorldData.def("GetLightMapIntensity", &pragma::asset::WorldData::GetLightMapIntensity);
	defWorldData.def("GetLightMapExposure", &pragma::asset::WorldData::GetLightMapExposure);
	defWorldData.def("GetEntities", &pragma::asset::WorldData::GetEntities);
	defWorldData.def("GetMaterialTable", static_cast<const std::vector<std::string> &(pragma::asset::WorldData ::*)() const>(&pragma::asset::WorldData::GetMaterialTable));
	defWorldData.def("FindWorld", &pragma::asset::WorldData::FindWorld);
	// defWorldData.def("SetBSPTree", &pragma::asset::WorldData::SetBSPTree);
	// defWorldData.def("GetBSPTree", &pragma::asset::WorldData::GetBSPTree);
	defWorldData.def(
	  "Save", +[](lua_State *l, pragma::asset::WorldData &worldData, udm::AssetDataArg assetData, const std::string &mapName) -> Lua::mult<bool, Lua::opt<std::string>> {
		  std::string err;
		  auto result = worldData.Save(assetData, mapName, err);
		  if(result)
			  return luabind::object {l, true};
		  return luabind::object {l, std::pair<bool, std::string> {false, err}};
	  });
	defWorldData.def(
	  "Save", +[](lua_State *l, pragma::asset::WorldData &worldData, const std::string &fileName, const std::string &mapName) -> Lua::mult<bool, Lua::opt<std::string>> {
		  std::string err;
		  auto result = worldData.Save(fileName, mapName, err);
		  if(result)
			  return luabind::object {l, true};
		  return luabind::object {l, std::pair<bool, std::string> {false, err}};
	  });
	mod[defWorldData];
	pragma::lua::define_custom_constructor<pragma::asset::ComponentData, []() -> std::shared_ptr<pragma::asset::ComponentData> { return pragma::asset::ComponentData::Create(); }>(l);
	pragma::lua::define_custom_constructor<pragma::asset::EntityData, []() -> std::shared_ptr<pragma::asset::EntityData> { return pragma::asset::EntityData::Create(); }>(l);
	pragma::lua::define_custom_constructor<pragma::asset::WorldData, [](NetworkState &nw) -> std::shared_ptr<pragma::asset::WorldData> { return pragma::asset::WorldData::Create(nw); }, NetworkState &>(l);
}

void Lua::util::register_os(lua_State *l, luabind::module_ &mod) { mod[luabind::def("set_prevent_os_sleep_mode", &::util::set_prevent_os_sleep_mode)]; }

static Lua::var<bool, Lua::opt<std::string>, ::util::FunctionalParallelWorker> extract_files(lua_State *l, Game &game, const Lua::type<uzip::ZIPFile> &ozip, const std::string &outputPath, bool runInBackground = false)
{
	auto path = ::util::Path::CreateFile(outputPath);
	path.Canonicalize();
	path = ::util::Path::CreatePath(::util::get_program_path()) + path;

	auto &zip = *luabind::object_cast<uzip::ZIPFile *>(ozip);
	if(runInBackground) {
		auto job = ::util::create_parallel_job<::util::FunctionalParallelWorker>(false);
		auto cpyOzip = ozip;
		auto &worker = static_cast<::util::FunctionalParallelWorker &>(job.GetWorker());
		worker.CallOnRemove([cpyOzip]() mutable {
			cpyOzip = luabind::object {}; // We need to keep a reference to the zip file alive until the job is complete
		});
		worker.ResetTask([&zip, path = std::move(path)](::util::FunctionalParallelWorker &worker) mutable {
			std::condition_variable waitCond;
			std::mutex mutex;
			std::atomic<bool> complete = false;

			std::string err;
			auto res = zip.ExtractFiles(path.GetString(), err, [&worker, &mutex, &waitCond, &complete](float progress, bool pcomplete) mutable {
				worker.UpdateProgress(progress);
				if(pcomplete) {
					worker.SetStatus(::util::JobStatus::Successful);

					mutex.lock();
					complete = true;
					waitCond.notify_one();
					mutex.unlock();
					return false;
				}
				auto cancelled = worker.IsCancelled();
				if(cancelled) {
					mutex.lock();
					complete = true;
					waitCond.notify_one();
					mutex.unlock();
				}
				return cancelled;
			});
			if(!res) {
				worker.SetStatus(::util::JobStatus::Failed, err);
				return;
			}

			auto ul = std::unique_lock<std::mutex> {mutex};
			waitCond.wait(ul, [&complete]() -> bool { return complete; });

			if(!worker.IsCancelled())
				worker.SetStatus(util::JobStatus::Successful);
		});
		return luabind::object {l, job};
	}

	std::string err;
	auto res = zip.ExtractFiles(path.GetString(), err);
	if(!res)
		return luabind::object {l, std::pair<bool, std::string> {res, err}};
	return luabind::object {l, res};
}

void Lua::util::register_shared_generic(lua_State *l, luabind::module_ &mod)
{
	mod[luabind::def("is_valid", static_cast<bool (*)(lua_State *)>(Lua::util::is_valid)), luabind::def("is_valid", static_cast<bool (*)(lua_State *, const luabind::object &)>(Lua::util::is_valid)),
	  luabind::def("remove", static_cast<void (*)(lua_State *, const luabind::object &)>(remove)), luabind::def("remove", static_cast<void (*)(lua_State *, const luabind::object &, bool)>(remove)),
	  luabind::def(
	    "remove", +[]() {}),
	  luabind::def(
	    "remove", +[](const luabind::detail::nil_type &, bool) {}),
	  luabind::def(
	    "remove", +[](const luabind::detail::nil_type &) {}),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const std::string &, const luabind::object &, const luabind::object &, const luabind::object &, const luabind::object &, const luabind::object &)>(Lua::util::register_class)),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const std::string &, const luabind::object &, const luabind::object &, const luabind::object &, const luabind::object &)>(Lua::util::register_class)),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const std::string &, const luabind::object &, const luabind::object &, const luabind::object &)>(Lua::util::register_class)),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const std::string &, const luabind::object &, const luabind::object &)>(Lua::util::register_class)),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const std::string &, const luabind::object &)>(Lua::util::register_class)), luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const std::string &)>(Lua::util::register_class)),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const luabind::table<> &, const std::string &, const luabind::object &, const luabind::object &, const luabind::object &, const luabind::object &, const luabind::object &)>(Lua::util::register_class)),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const luabind::table<> &, const std::string &, const luabind::object &, const luabind::object &, const luabind::object &, const luabind::object &)>(Lua::util::register_class)),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const luabind::table<> &, const std::string &, const luabind::object &, const luabind::object &, const luabind::object &)>(Lua::util::register_class)),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const luabind::table<> &, const std::string &, const luabind::object &, const luabind::object &)>(Lua::util::register_class)),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const luabind::table<> &, const std::string &, const luabind::object &)>(Lua::util::register_class)),
	  luabind::def("register_class", static_cast<luabind::object (*)(lua_State *, const luabind::table<> &, const std::string &)>(Lua::util::register_class)),

	  luabind::def("local_to_world", static_cast<Quat (*)(lua_State *, const Quat &, const Quat &)>(local_to_world)), luabind::def("local_to_world", static_cast<Vector3 (*)(lua_State *, const Vector3 &, const Quat &, const Vector3 &)>(local_to_world)),
	  luabind::def("local_to_world", static_cast<void (*)(lua_State *, const Vector3 &, const Quat &, const Vector3 &, const Quat &)>(local_to_world)),

	  luabind::def("world_to_local", static_cast<Quat (*)(lua_State *, const Quat &, const Quat &)>(world_to_local)), luabind::def("world_to_local", static_cast<Vector3 (*)(lua_State *, const Vector3 &, const Quat &, const Vector3 &)>(world_to_local)),
	  luabind::def("world_to_local", static_cast<void (*)(lua_State *, const Vector3 &, const Quat &, const Vector3 &, const Quat &)>(world_to_local)),

	  luabind::def("get_pretty_duration", static_cast<std::string (*)(lua_State *, uint32_t, uint32_t, bool)>(Lua::util::get_pretty_duration)), luabind::def("get_pretty_duration", static_cast<std::string (*)(lua_State *, uint32_t, uint32_t)>(Lua::util::get_pretty_duration)),
	  luabind::def("get_pretty_duration", static_cast<std::string (*)(lua_State *, uint32_t)>(Lua::util::get_pretty_duration)), luabind::def("get_pretty_time", Lua::util::get_pretty_time),
	  luabind::def(
	    "get_object_hash", +[](Lua::userData o) { return std::hash<void *> {}(lua_touserdata(o.interpreter(), 1)); }),

	  luabind::def("fade_property", static_cast<luabind::object (*)(lua_State *, LColorProperty &, const Color &, float)>(Lua::util::fade_property)),
	  luabind::def("fade_property", static_cast<luabind::object (*)(lua_State *, LVector2iProperty &, const Vector2i &, float)>(Lua::util::fade_property)),
	  luabind::def("fade_property", static_cast<luabind::object (*)(lua_State *, LVector3Property &, const Vector3 &, float)>(Lua::util::fade_property)),
	  luabind::def("fade_property", static_cast<luabind::object (*)(lua_State *, LVector3iProperty &, const Vector3i &, float)>(Lua::util::fade_property)),
	  luabind::def("fade_property", static_cast<luabind::object (*)(lua_State *, LVector4Property &, const Vector4 &, float)>(Lua::util::fade_property)),
	  luabind::def("fade_property", static_cast<luabind::object (*)(lua_State *, LVector4iProperty &, const Vector4i &, float)>(Lua::util::fade_property)),
	  luabind::def("fade_property", static_cast<luabind::object (*)(lua_State *, LQuatProperty &, const Quat &, float)>(Lua::util::fade_property)),
	  luabind::def("fade_property", static_cast<luabind::object (*)(lua_State *, LEulerAnglesProperty &, const EulerAngles &, float)>(Lua::util::fade_property)),
	  luabind::def("fade_property", static_cast<luabind::object (*)(lua_State *, LGenericIntPropertyWrapper &, const int64_t &, float)>(Lua::util::fade_property)),
	  luabind::def("fade_property", static_cast<luabind::object (*)(lua_State *, LGenericFloatPropertyWrapper &, const double &, float)>(Lua::util::fade_property)),

	  luabind::def("round_string", static_cast<std::string (*)(lua_State *, float, uint32_t)>(Lua::util::round_string)), luabind::def("round_string", static_cast<std::string (*)(lua_State *, float)>(Lua::util::round_string)),

	  luabind::def("get_type_name", Lua::util::get_type_name), luabind::def("is_same_object", Lua::util::is_same_object), luabind::def("clamp_resolution_to_aspect_ratio", Lua::util::clamp_resolution_to_aspect_ratio), luabind::def("get_class_value", Lua::util::get_class_value),
	  luabind::def("pack_zip_archive", Lua::util::pack_zip_archive), luabind::def("world_space_point_to_screen_space_uv", static_cast<void (*)(lua_State *, const Vector3 &, const Mat4 &, float, float)>(Lua::util::world_space_point_to_screen_space_uv)),
	  luabind::def("world_space_direction_to_screen_space", Lua::util::world_space_direction_to_screen_space), luabind::def("calc_screen_space_distance_to_world_space_position", Lua::util::calc_screenspace_distance_to_worldspace_position),
	  luabind::def("depth_to_distance", Lua::util::depth_to_distance),

	  luabind::def("generate_hair_file", &generate_hair_file),
	  luabind::def(
	    "generate_hair_data", +[](float hairPerArea, const ModelSubMesh &mesh) {
		    struct MeshInterface : public ::util::HairGenerator::MeshInterface {
			    virtual uint32_t GetTriangleCount() const override { return getTriangleCount(); }
			    virtual uint32_t GetVertexCount() const override { return getVertexCount(); }
			    virtual std::array<uint32_t, 3> GetTriangle(uint32_t triIdx) const override { return getTriangle(triIdx); }
			    virtual const Vector3 GetVertexPosition(uint32_t vertIdx) const override { return getVertexPosition(vertIdx); }
			    virtual const Vector3 GetVertexNormal(uint32_t vertIdx) const override { return getVertexNormal(vertIdx); }
			    virtual const Vector2 GetVertexUv(uint32_t vertIdx) const override { return getVertexUv(vertIdx); }

			    std::function<uint32_t()> getTriangleCount = nullptr;
			    std::function<uint32_t()> getVertexCount = nullptr;
			    std::function<std::array<uint32_t, 3>(uint32_t)> getTriangle = nullptr;
			    std::function<Vector3(uint32_t)> getVertexPosition = nullptr;
			    std::function<Vector3(uint32_t)> getVertexNormal = nullptr;
			    std::function<Vector2(uint32_t)> getVertexUv = nullptr;
		    };

		    auto meshInterface = std::make_unique<MeshInterface>();
		    meshInterface->getTriangleCount = [&mesh]() -> uint32_t { return mesh.GetTriangleCount(); };
		    meshInterface->getVertexCount = [&mesh]() -> uint32_t { return mesh.GetVertexCount(); };
		    meshInterface->getTriangle = [&mesh](uint32_t triIdx) -> std::array<uint32_t, 3> { return std::array<uint32_t, 3> {*mesh.GetIndex(triIdx * 3), *mesh.GetIndex(triIdx * 3 + 1), *mesh.GetIndex(triIdx * 3 + 2)}; };
		    meshInterface->getVertexPosition = [&mesh](uint32_t vertIdx) -> Vector3 { return mesh.GetVertexPosition(vertIdx) * static_cast<float>(util::units_to_metres(1.f)); };
		    meshInterface->getVertexNormal = [&mesh](uint32_t vertIdx) -> Vector3 { return mesh.GetVertexNormal(vertIdx); };
		    meshInterface->getVertexUv = [&mesh](uint32_t vertIdx) -> Vector2 { return mesh.GetVertexUV(vertIdx); };

		    ::util::HairGenerator gen {};
		    gen.SetMeshDataInterface(std::move(meshInterface));
		    return gen.Generate(hairPerArea);
	    })];

	Lua::util::register_world_data(l, mod);

	auto defHairStrandData = luabind::class_<::util::HairStrandData>("HairStrandData");
	defHairStrandData.def(
	  "Save", +[](const ::util::HairStrandData &strandData, udm::AssetDataArg outData) {
		  std::string err;
		  return save_hair_strand_data(strandData, outData, err);
	  });
	defHairStrandData.def(
	  "Load", +[](::util::HairStrandData &strandData, const udm::AssetData &data) {
		  std::string err;
		  return load_hair_strand_data(strandData, data, err);
	  });
	defHairStrandData.def(
	  "GetStrandCount", +[](const ::util::HairStrandData &strandData) { return strandData.hairSegments.size(); });
	defHairStrandData.def(
	  "GetSegmentCount", +[](const ::util::HairStrandData &strandData, uint32_t idx) { return strandData.hairSegments[idx]; });
	defHairStrandData.def(
	  "GetStrandPoint", +[](const ::util::HairStrandData &strandData, uint32_t idx) { return strandData.points[idx]; });
	defHairStrandData.def(
	  "GetStrandUv", +[](const ::util::HairStrandData &strandData, uint32_t idx) { return strandData.uvs[idx]; });
	defHairStrandData.def(
	  "GetStrandThickness", +[](const ::util::HairStrandData &strandData, uint32_t idx) { return strandData.thicknessData[idx]; });
	mod[defHairStrandData];

	auto defHairConfig = luabind::class_<::util::HairConfig>("HairConfig");
	defHairConfig.def(luabind::constructor<>());
	defHairConfig.def_readwrite("numSegments", &::util::HairConfig::numSegments);
	defHairConfig.def_readwrite("hairPerSquareMeter", &::util::HairConfig::hairPerSquareMeter);
	defHairConfig.def_readwrite("defaultThickness", &::util::HairConfig::defaultThickness);
	defHairConfig.def_readwrite("defaultLength", &::util::HairConfig::defaultLength);
	defHairConfig.def_readwrite("defaultHairStrength", &::util::HairConfig::defaultHairStrength);
	defHairConfig.def_readwrite("randomHairLengthFactor", &::util::HairConfig::randomHairLengthFactor);
	defHairConfig.def_readwrite("curvature", &::util::HairConfig::curvature);
	mod[defHairConfig];

	auto defHairData = luabind::class_<::util::HairData>("HairData");
	mod[defHairData];

	auto defZip = luabind::class_<uzip::ZIPFile>("ZipFile");
	defZip.add_static_constant("OPEN_MODE_READ", umath::to_integral(uzip::OpenMode::Read));
	defZip.add_static_constant("OPEN_MODE_WRITE", umath::to_integral(uzip::OpenMode::Write));

	defZip.scope[luabind::def(
	  "open", +[](const std::string &filePath, uzip::OpenMode openMode) -> std::shared_ptr<uzip::ZIPFile> {
		  auto path = ::util::Path::CreateFile(filePath);
		  path.Canonicalize();
		  path = ::util::Path::CreatePath(::util::get_program_path()) + path;
		  auto zipFile = uzip::ZIPFile::Open(path.GetString(), openMode);
		  if(!zipFile)
			  return nullptr;
		  return zipFile;
	  })];
	defZip.scope[luabind::def(
	  "open", +[](LFile &f, uzip::OpenMode openMode) -> std::shared_ptr<uzip::ZIPFile> {
		  auto ptr = f.GetHandle();
		  if(!ptr)
			  return nullptr;
		  auto filePath = ptr->GetFileName();
		  if(!filePath.has_value())
			  return nullptr;
		  auto zipFile = uzip::ZIPFile::Open(*filePath, openMode);
		  if(!zipFile)
			  return nullptr;
		  return zipFile;
	  })];
	defZip.scope[luabind::def(
	  "get_supported_format_extensions", +[]() -> std::vector<std::string> {
		  return std::vector<std::string> {"r", "arj", "cab", "chm", "cpio", "cramfs", "dmg", "ext", "fat", "gpt", "hfs", "ihex", "iso", "lzh", "lzma", "mbr", "msi", "nsis", "ntfs", "qcow2", "rar", "rpm", "squashfs", "udf", "uefi", "vdi", "vhd", "vhdx", "vmdk", "wim", "xar", "z", "zip",
		    "7z"};
	  })];
	defZip.def(
	  "GetFileList", +[](uzip::ZIPFile &zip) -> std::optional<std::vector<std::string>> {
		  std::vector<std::string> files;
		  if(!zip.GetFileList(files))
			  return {};
		  return files;
	  });
	defZip.def("ExtractFiles", &extract_files);
	defZip.def("ExtractFiles", &extract_files, luabind::default_parameter_policy<5, false> {});
	defZip.def(
	  "ExtractFile", +[](uzip::ZIPFile &zip, const std::string &zipFileName, const std::string &outputZipFileName) -> std::pair<bool, std::optional<std::string>> {
		  std::vector<uint8_t> data;
		  std::string err;
		  if(!zip.ReadFile(zipFileName, data, err))
			  return {false, err};
		  filemanager::create_path(ufile::get_path_from_filename(outputZipFileName));
		  auto f = filemanager::open_file(outputZipFileName, filemanager::FileMode::Write | filemanager::FileMode::Binary);
		  if(!f)
			  return {false, "Unable to open output file!"};
		  fsys::File fp {f};
		  fp.Write(data.data(), data.size());
		  return {true, {}};
	  });
	mod[defZip];
}

void Lua::util::register_shared(lua_State *l, luabind::module_ &mod)
{
	register_shared_generic(l, mod);
	mod[luabind::def("is_valid_entity", static_cast<bool (*)(lua_State *)>(Lua::util::is_valid_entity)), luabind::def("is_valid_entity", static_cast<bool (*)(lua_State *, const luabind::object &)>(Lua::util::is_valid_entity)),
	  luabind::def("shake_screen", static_cast<void (*)(lua_State *, const Vector3 &, float, float, float, float, float, float)>(Lua::util::shake_screen)), luabind::def("shake_screen", static_cast<void (*)(lua_State *, float, float, float, float, float)>(Lua::util::shake_screen)),
	  luabind::def("read_scene_file", Lua::util::read_scene_file),
	  luabind::def(
	    "get_program_path", +[]() { return ::util::Path::CreatePath(::util::get_program_path()).GetString(); })];
}
static Lua::mult<bool, Lua::opt<std::string>> exec_python(lua_State *l, const std::string &fileName, const std::vector<std::string> &args)
{
	std::vector<const char *> cargs;
	cargs.reserve(args.size());
	for(auto &arg : args)
		cargs.push_back(arg.c_str());
	auto fullPath = filemanager::get_canonicalized_path(fileName);
	auto res = pragma::python::exec(fullPath.c_str(), cargs.size(), cargs.data());
	if(res == false) {
		auto err = pragma::python::get_last_error();
		if(!err.has_value())
			err = "Unknown Error";
		return luabind::object {l, std::pair<bool, std::string> {res, *err}};
	}
	return luabind::object {l, res};
}
static Lua::mult<bool, Lua::opt<std::string>> exec_python(lua_State *l, const std::string &fileName) { return exec_python(l, fileName, {}); }
#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::rendering, pragma::rendering::Tile);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, pragma::ik::RigConfig);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, pragma::ik::RigConfigBone);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, pragma::ik::RigConfigControl);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, pragma::ik::RigConfigConstraint);

DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::asset, pragma::asset::Output);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::asset, pragma::asset::ComponentData);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::asset, pragma::asset::EntityData);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::asset, pragma::asset::WorldData);
#endif
void Lua::util::register_library(lua_State *l)
{
	auto pythonMod = luabind::module(l, "python");
	pythonMod[luabind::def(
	            "run",
	            +[](lua_State *l, const std::string &code) -> Lua::mult<bool, Lua::opt<std::string>> {
		            auto res = pragma::python::run(code.c_str());
		            if(res == false) {
			            auto err = pragma::python::get_last_error();
			            if(!err.has_value())
				            err = "Unknown Error";
			            return luabind::object {l, std::pair<bool, std::string> {res, *err}};
		            }
		            return luabind::object {l, res};
	            }),
	  luabind::def("exec", static_cast<Lua::mult<bool, Lua::opt<std::string>> (*)(lua_State *, const std::string &)>(&exec_python)),
	  luabind::def("exec", static_cast<Lua::mult<bool, Lua::opt<std::string>> (*)(lua_State *, const std::string &, const std::vector<std::string> &)>(&exec_python)), luabind::def("init_blender", &pragma::python::init_blender), luabind::def("reload", &pragma::python::reload)];

	auto utilMod = luabind::module(l, "util");

	auto defRigConfig = luabind::class_<pragma::ik::RigConfig>("IkRigConfig");
	defRigConfig.def(luabind::constructor<>());
	defRigConfig.def(luabind::tostring(luabind::self));
	defRigConfig.scope[luabind::def("load", &pragma::ik::RigConfig::load)];
	defRigConfig.scope[luabind::def("load_from_udm_data", &pragma::ik::RigConfig::load_from_udm_data)];
	defRigConfig.scope[luabind::def("get_supported_extensions", &pragma::ik::RigConfig::get_supported_extensions)];
	defRigConfig.def("DebugPrint", &pragma::ik::RigConfig::DebugPrint);
	defRigConfig.def("ToUdmData", &pragma::ik::RigConfig::ToUdmData);
	defRigConfig.def("AddBone", &pragma::ik::RigConfig::AddBone);
	defRigConfig.def("GetBones", &pragma::ik::RigConfig::GetBones);
	defRigConfig.def("FindBone", &pragma::ik::RigConfig::FindBone);
	defRigConfig.def("GetConstraints", &pragma::ik::RigConfig::GetConstraints);
	defRigConfig.def("GetControls", &pragma::ik::RigConfig::GetControls);
	defRigConfig.def("RemoveBone", static_cast<void (pragma::ik::RigConfig::*)(const pragma::GString &)>(&pragma::ik::RigConfig::RemoveBone));
	defRigConfig.def("RemoveControl", static_cast<void (pragma::ik::RigConfig::*)(const pragma::ik::RigConfigControl &)>(&pragma::ik::RigConfig::RemoveControl));
	defRigConfig.def("RemoveControl", static_cast<void (pragma::ik::RigConfig::*)(const pragma::GString &)>(&pragma::ik::RigConfig::RemoveControl));
	defRigConfig.def("RemoveConstraint", static_cast<void (pragma::ik::RigConfig::*)(const pragma::ik::RigConfigConstraint &)>(&pragma::ik::RigConfig::RemoveConstraint));
	defRigConfig.def("RemoveBone", static_cast<void (pragma::ik::RigConfig::*)(const pragma::ik::RigConfigBone &)>(&pragma::ik::RigConfig::RemoveBone));
	defRigConfig.def("HasBone", &pragma::ik::RigConfig::HasBone);
	defRigConfig.def("IsBoneLocked", &pragma::ik::RigConfig::IsBoneLocked);
	defRigConfig.def("SetBoneLocked", &pragma::ik::RigConfig::SetBoneLocked);
	defRigConfig.def("HasControl", &pragma::ik::RigConfig::HasControl);
	defRigConfig.def("AddControl", &pragma::ik::RigConfig::AddControl);
	defRigConfig.def("RemoveConstraints", static_cast<void (pragma::ik::RigConfig::*)(const pragma::GString &, const pragma::GString &)>(&pragma::ik::RigConfig::RemoveConstraints));
	defRigConfig.def("RemoveConstraints", static_cast<void (pragma::ik::RigConfig::*)(const pragma::GString &)>(&pragma::ik::RigConfig::RemoveConstraints));
	defRigConfig.def("AddFixedConstraint", &pragma::ik::RigConfig::AddFixedConstraint);
	defRigConfig.def("AddHingeConstraint", &pragma::ik::RigConfig::AddHingeConstraint);
	defRigConfig.def("AddBallSocketConstraint", &pragma::ik::RigConfig::AddBallSocketConstraint);
	defRigConfig.def("AddBallSocketConstraint", &pragma::ik::RigConfig::AddBallSocketConstraint, luabind::default_parameter_policy<6, pragma::SignedAxis::Z> {});
	defRigConfig.def("SetRootBone", &pragma::ik::RigConfig::SetRootBone);
	defRigConfig.def("ClearRootBone", &pragma::ik::RigConfig::ClearRootBone);
	defRigConfig.def("GetRootBone", &pragma::ik::RigConfig::GetRootBone);
	defRigConfig.def("CalcScaleFactor", &pragma::ik::RigConfig::CalcScaleFactor);
	defRigConfig.def(
	  "Save", +[](lua_State *l, pragma::ik::RigConfig &rigConfig, const std::string &fileName) -> std::pair<bool, std::optional<std::string>> {
		  auto fname = fileName;
		  if(Lua::file::validate_write_operation(l, fname) == false)
			  return std::pair<bool, std::optional<std::string>> {false, "Invalid write location!"};
		  auto res = rigConfig.Save(fname);
		  if(!res)
			  return std::pair<bool, std::optional<std::string>> {false, "Unknown error"};
		  return std::pair<bool, std::optional<std::string>> {true, {}};
	  });

	auto defRigBone = luabind::class_<pragma::ik::RigConfigBone>("Bone");
	defRigBone.def(luabind::tostring(luabind::self));
	defRigBone.def_readwrite("locked", &pragma::ik::RigConfigBone::locked);
	defRigBone.def_readwrite("name", &pragma::ik::RigConfigBone::name);
	defRigConfig.scope[defRigBone];

	auto defRigControl = luabind::class_<pragma::ik::RigConfigControl>("Control");
	defRigControl.def(luabind::tostring(luabind::self));
	defRigControl.add_static_constant("TYPE_DRAG", umath::to_integral(pragma::ik::RigConfigControl::Type::Drag));
	defRigControl.add_static_constant("TYPE_ANGULAR_PLANE", umath::to_integral(pragma::ik::RigConfigControl::Type::AngularPlane));
	defRigControl.add_static_constant("TYPE_STATE", umath::to_integral(pragma::ik::RigConfigControl::Type::State));
	defRigControl.add_static_constant("TYPE_ORIENTED_DRAG", umath::to_integral(pragma::ik::RigConfigControl::Type::OrientedDrag));
	defRigControl.add_static_constant("TYPE_REVOLUTE", umath::to_integral(pragma::ik::RigConfigControl::Type::Revolute));
	defRigControl.add_static_constant("TYPE_AXIS", umath::to_integral(pragma::ik::RigConfigControl::Type::Axis));
	defRigControl.add_static_constant("TYPE_POLE_TARGET", umath::to_integral(pragma::ik::RigConfigControl::Type::PoleTarget));
	static_assert(umath::to_integral(pragma::ik::RigConfigControl::Type::Count) == 7u, "Update this list when new types are added!");
	defRigControl.def_readwrite("bone", &pragma::ik::RigConfigControl::bone);
	defRigControl.def_readwrite("type", &pragma::ik::RigConfigControl::type);
	defRigControl.def_readwrite("maxForce", &pragma::ik::RigConfigControl::maxForce);
	defRigControl.def_readwrite("rigidity", &pragma::ik::RigConfigControl::rigidity);
	defRigConfig.scope[defRigControl];
	defRigConfig.scope[defRigBone];

	auto defRigConstraint = luabind::class_<pragma::ik::RigConfigConstraint>("Constraint");
	defRigConstraint.def(luabind::tostring(luabind::self));
	defRigConstraint.add_static_constant("TYPE_FIXED", umath::to_integral(pragma::ik::RigConfigConstraint::Type::Fixed));
	defRigConstraint.add_static_constant("TYPE_HINGE", umath::to_integral(pragma::ik::RigConfigConstraint::Type::Hinge));
	defRigConstraint.add_static_constant("TYPE_BALL_SOCKET", umath::to_integral(pragma::ik::RigConfigConstraint::Type::BallSocket));
	static_assert(umath::to_integral(pragma::ik::RigConfigConstraint::Type::Count) == 3u, "Update this list when new types are added!");
	defRigConstraint.def_readwrite("bone0", &pragma::ik::RigConfigConstraint::bone0);
	defRigConstraint.def_readwrite("bone1", &pragma::ik::RigConfigConstraint::bone1);
	defRigConstraint.def_readwrite("type", &pragma::ik::RigConfigConstraint::type);
	defRigConstraint.def_readwrite("minLimits", &pragma::ik::RigConfigConstraint::minLimits);
	defRigConstraint.def_readwrite("maxLimits", &pragma::ik::RigConfigConstraint::maxLimits);
	defRigConstraint.def_readwrite("axis", &pragma::ik::RigConfigConstraint::axis);
	defRigConfig.scope[defRigConstraint];
	utilMod[defRigConfig];

	auto defRenderTile = luabind::class_<pragma::rendering::Tile>("RenderTile");
	defRenderTile.def(luabind::constructor<>());
	defRenderTile.def(luabind::constructor<float, float, float, float>());
	defRenderTile.def(luabind::tostring(luabind::self));
	defRenderTile.def_readwrite("x", &pragma::rendering::Tile::x);
	defRenderTile.def_readwrite("y", &pragma::rendering::Tile::y);
	defRenderTile.def_readwrite("w", &pragma::rendering::Tile::w);
	defRenderTile.def_readwrite("h", &pragma::rendering::Tile::h);
	utilMod[defRenderTile];

	utilMod[luabind::def("splash_damage", splash_damage), luabind::def("get_date_time", static_cast<std::string (*)(const std::string &)>(Lua::util::date_time)), luabind::def("get_date_time", static_cast<std::string (*)()>(Lua::util::date_time)),
	  luabind::def("is_table", static_cast<bool (*)(luabind::argument)>(Lua::util::is_table)), luabind::def("is_table", static_cast<bool (*)()>(Lua::util::is_table)),
	  luabind::def("get_faded_time_factor", static_cast<float (*)(float, float, float, float)>(Lua::util::get_faded_time_factor)), luabind::def("get_faded_time_factor", static_cast<float (*)(float, float, float)>(Lua::util::get_faded_time_factor)),
	  luabind::def("get_scale_factor", static_cast<float (*)(float, float, float)>(Lua::util::get_scale_factor)), luabind::def("get_scale_factor", static_cast<float (*)(float, float)>(Lua::util::get_scale_factor)),
	  luabind::def("open_path_in_explorer", static_cast<void (*)(const std::string &, const std::string &)>(Lua::util::open_path_in_explorer)), luabind::def("open_path_in_explorer", static_cast<void (*)(const std::string &)>(Lua::util::open_path_in_explorer)),
	  luabind::def("get_pretty_bytes", Lua::util::get_pretty_bytes), luabind::def("units_to_metres", Lua::util::units_to_metres), luabind::def("metres_to_units", Lua::util::metres_to_units), luabind::def("variable_type_to_string", Lua::util::variable_type_to_string),
	  luabind::def("open_url_in_browser", Lua::util::open_url_in_browser), luabind::def("get_addon_path", static_cast<std::string (*)(lua_State *)>(Lua::util::get_addon_path)), luabind::def("get_string_hash", Lua::util::get_string_hash),
	  luabind::def(
	    "generate_uuid_v4", +[]() -> util::Uuid { return util::Uuid {::util::generate_uuid_v4()}; }),
	  luabind::def(
	    "generate_uuid_v4", +[](size_t seed) -> util::Uuid { return util::Uuid {::util::generate_uuid_v4(seed)}; }),
	  luabind::def(
	    "generate_uuid_v4",
	    +[](const std::string &str) -> util::Uuid {
		    auto seed = std::hash<std::string> {}(str);
		    return util::Uuid {::util::generate_uuid_v4(seed)};
	    }),
	  luabind::def(
	    "run_updater", +[](Engine &engine) { engine.SetRunUpdaterOnClose(true); })];

	auto defUuid = luabind::class_<util::Uuid>("Uuid");
	defUuid.def(
	  "__tostring", +[](const util::Uuid &uuid) { return ::util::uuid_to_string(uuid.value); });
	defUuid.def(
	  "IsValid", +[](const util::Uuid &uuid) {
		  for(auto v : uuid.value) {
			  if(v != 0)
				  return true;
		  }
		  return false;
	  });
	utilMod[defUuid];
	pragma::lua::define_custom_constructor<util::Uuid, [](const std::string &uuid) -> util::Uuid { return util::Uuid {::util::uuid_string_to_bytes(uuid)}; }, const std::string &>(l);
}

luabind::object Lua::global::include(lua_State *l, const std::string &f) { return include(l, f, s_bIgnoreIncludeCache); }
luabind::object Lua::global::include(lua_State *l, const std::string &f, std::vector<std::string> *optCache) { return include(l, f, optCache, false); }
static std::vector<std::string> g_globalTmpCache;
static uint32_t g_globalTmpCacheRecursiveCount = 0;
luabind::object Lua::global::include(lua_State *l, const std::string &f, bool ignoreGlobalCache)
{
	if(ignoreGlobalCache) {
		++g_globalTmpCacheRecursiveCount;
		::util::ScopeGuard sg {[]() {
			if(--g_globalTmpCacheRecursiveCount == 0)
				g_globalTmpCache.clear();
		}};
		return include(l, f, &g_globalTmpCache);
	}
	return include(l, f, nullptr);
}

luabind::object Lua::global::include(lua_State *l, const std::string &f, std::vector<std::string> *optCache, bool reload, bool throwErr)
{
	auto *lInterface = engine->GetLuaInterface(l);
	std::vector<std::string> *includeCache = optCache;
	if(!includeCache)
		includeCache = (lInterface != nullptr) ? &lInterface->GetIncludeCache() : nullptr;
	auto fShouldInclude = [includeCache, reload](std::string fpath) -> bool {
		if(includeCache == nullptr)
			return true;
		if(fpath.empty() == false) {
			if(fpath.front() == '/' || fpath.front() == '\\')
				fpath.erase(fpath.begin());
			else
				fpath = Lua::GetIncludePath(fpath);
		}
		fpath = FileManager::GetCanonicalizedPath(fpath);
		auto it = std::find_if(includeCache->begin(), includeCache->end(), [fpath](const std::string &other) { return ustring::compare(fpath, other, false); });
		if(!reload && it != includeCache->end())
			return false;
		if(it == includeCache->end())
			includeCache->push_back(fpath);
		return true;
	};
	auto *nw = engine->GetNetworkState(l);
	auto *game = (nw != nullptr) ? nw->GetGameState() : nullptr;
	std::string ext;
	if(ufile::get_extension(f, &ext) == false) // Assume it's a directory
	{
		std::string relPath = f;
		if(relPath.back() != '\\' && relPath.back() != '/')
			relPath += "/";

		auto incPath = relPath;
		if(incPath.empty() == false) {
			if(incPath.front() == '/' || incPath.front() == '\\')
				incPath.erase(incPath.begin());
			else
				incPath = Lua::GetIncludePath(incPath);
		}
		incPath = Lua::SCRIPT_DIRECTORY_SLASH + incPath;
		auto incPathLua = incPath + "*." + Lua::FILE_EXTENSION;
		std::vector<std::string> files;
		FileManager::FindFiles(incPathLua.c_str(), &files, nullptr);

		auto incPathCLua = incPath + "*." + Lua::FILE_EXTENSION_PRECOMPILED;
		std::vector<std::string> cfiles;
		FileManager::FindFiles(incPathCLua.c_str(), &cfiles, nullptr);
		files.reserve(files.size() + cfiles.size());

		// Add pre-compiled Lua-files to list, but make sure there are no duplicates!
		for(auto &cf : cfiles) {
			auto it = std::find_if(files.begin(), files.end(), [&cf](const std::string &fother) { return ustring::compare(cf.c_str(), fother.c_str(), false, cf.length() - 5); });
			if(it != files.end()) {
				*it = cf; // Prefer pre-compiled files over regular files
				continue;
			}
			files.push_back(cf);
		}

		for(auto &fName : files) {
			auto fpath = relPath + fName;
			if(fShouldInclude(fpath) == false)
				continue;
			auto r = Lua::IncludeFile(l, fpath, Lua::HandleTracebackError);
			switch(r) {
			case Lua::StatusCode::ErrorFile:
				lua_error(l);
				/* unreachable */
				break;
			case Lua::StatusCode::ErrorSyntax:
				Lua::HandleSyntaxError(l, r, fpath);
				break;
			}
		}
		return {};
	}
	if(fShouldInclude(f) == true) {
		//auto r = Lua::Execute(l,[&f,l](int(*traceback)(lua_State*)) {
		//	return Lua::IncludeFile(l,f,traceback);
		//});
		auto n = Lua::GetStackTop(l);
		auto fileName = f;
		auto r = Lua::IncludeFile(l, fileName, Lua::HandleTracebackError, LUA_MULTRET);
		switch(r) {
		case Lua::StatusCode::ErrorFile:
			if(throwErr)
				lua_error(l);
			else {
				Con::cwar << "File not found: '" << fileName << "'!" << Con::endl;
				return {};
			}
			/* unreachable */
			break;
		case Lua::StatusCode::ErrorSyntax:
			Lua::HandleSyntaxError(l, r, fileName);
			break;
		case Lua::StatusCode::Ok:
			{
				auto t = Lua::GetStackTop(l);
				if(t <= n)
					return Lua::nil;
				return luabind::object {luabind::from_stack {l, t - n}};
			}
		}
	}
	return {};
}

luabind::object Lua::global::include(lua_State *l, const std::string &f, std::vector<std::string> *optCache, bool reload) { return include(l, f, optCache, reload, true); }

luabind::object Lua::global::exec(lua_State *l, const std::string &f)
{
	auto n = Lua::GetStackTop(l);
	std::string fileName = f;
	auto r = Lua::ExecuteFile(l, fileName, Lua::HandleTracebackError, LUA_MULTRET);
	switch(r) {
	case Lua::StatusCode::ErrorFile:
		lua_error(l);
		/* unreachable */
		break;
	case Lua::StatusCode::ErrorSyntax:
		Lua::HandleSyntaxError(l, r, fileName);
		break;
	case Lua::StatusCode::Ok:
		return luabind::object {luabind::from_stack {l, Lua::GetStackTop(l) - n}};
	}
	return {};
}

std::string Lua::global::get_script_path() { return Lua::GetIncludePath(); }
EulerAngles Lua::global::angle_rand() { return EulerAngles(umath::random(-180.f, 180.f), umath::random(-180.f, 180.f), umath::random(-180.f, 180.f)); }
EulerAngles Lua::global::create_from_string(const std::string &str) { return EulerAngles {str}; }

static bool check_valid_lua_object(lua_State *l, const luabind::object &o)
{
	auto *pEnt = luabind::object_cast_nothrow<EntityHandle *>(o, static_cast<EntityHandle *>(nullptr));
	if(pEnt != nullptr)
		return pEnt->valid(); // Used frequently, and is faster than looking up "IsValid"
	auto bValid = true;
	try {
		auto oIsValid = o["IsValid"];
		if(!oIsValid)
			return bValid;
		bValid = luabind::call_member<bool>(o, "IsValid");
	}
	catch(std::exception &) // No "IsValid" method exists
	{
	}
	return bValid;
}

bool Lua::util::is_valid(lua_State *l) { return false; }
bool Lua::util::is_valid(lua_State *l, const luabind::object &o)
{
	if(!o)
		return false;
	auto type = luabind::type(o);
	switch(type) {
	case LUA_TUSERDATA:
		return check_valid_lua_object(l, o);
	case LUA_TBOOLEAN:
		return luabind::object_cast<bool>(o);
	case LUA_TTABLE:
		{
			for(luabind::iterator i {o}, e; i != e; ++i) {
				auto child = luabind::object {*i};
				if(check_valid_lua_object(l, child) == false)
					return false;
			}
			return true;
		}
	}
	return true;
}

bool Lua::util::is_valid_entity(lua_State *l)
{
	if(!Lua::IsSet(l, 1) || !Lua::IsType<BaseEntity>(l, 1))
		return false;
	return is_valid(l);
}

bool Lua::util::is_valid_entity(lua_State *l, const luabind::object &o)
{
	if(!o || !is_entity(o))
		return false;
	return is_valid(l, o);
}

static void safely_remove(const luabind::object &o, const char *removeFunction, bool useSafeMethod)
{
	auto *pEnt = luabind::object_cast_nothrow<EntityHandle *>(o, static_cast<EntityHandle *>(nullptr));
	if(pEnt != nullptr) // Used frequently, and is faster than looking up "IsValid"
	{
		if(pEnt->valid()) {
			if(useSafeMethod)
				(*pEnt)->RemoveSafely();
			else {
				try {
					(*pEnt)->Remove();
				}
				catch(const std::exception &e) {
					Lua::Error(o.interpreter(), e.what());
				}
			}
		}
		return;
	}
	try {
		auto oRemove = o[removeFunction];
		if(!oRemove)
			return;
		luabind::call_member<void>(o, removeFunction);
	}
	catch(std::exception &) // No "IsValid" method exists
	{
	}
}

void Lua::util::remove(lua_State *l, const luabind::object &o, bool removeSafely)
{
	auto type = luabind::type(o);
	if(type != LUA_TTABLE && is_valid(l, o) == false)
		return;
	auto *removeFunction = removeSafely ? "RemoveSafely" : "Remove";
	if(type == LUA_TTABLE) {
		for(luabind::iterator i(o), e; i != e; ++i) {
			auto o = luabind::object {*i};
			if(is_valid(l, o) == false)
				continue;
			safely_remove(o, removeFunction, removeSafely);
		}
		return;
	}
	safely_remove(o, removeFunction, removeSafely);
}
void Lua::util::remove(lua_State *l, const luabind::object &o) { return remove(l, o, false); }

bool Lua::util::is_table(luabind::argument arg) { return luabind::type(arg) == LUA_TTABLE; }
bool Lua::util::is_table() { return false; }

std::string Lua::util::date_time(const std::string &format) { return engine->GetDate(format); }
std::string Lua::util::date_time() { return engine->GetDate(); }

luabind::object Lua::util::fire_bullets(lua_State *l, BulletInfo &bulletInfo, bool hitReport, const std::function<void(DamageInfo &, ::TraceData &, TraceResult &, uint32_t &)> &f)
{
	DamageInfo dmg;
	dmg.SetDamage(umath::min(CUInt16(bulletInfo.damage), CUInt16(std::numeric_limits<UInt16>::max())));
	dmg.SetDamageType(bulletInfo.damageType);
	dmg.SetSource(bulletInfo.effectOrigin);
	dmg.SetAttacker(bulletInfo.hAttacker.get());
	dmg.SetInflictor(bulletInfo.hInflictor.get());

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();

	auto &src = dmg.GetSource();
	luabind::object t {};
	if(hitReport == true)
		t = luabind::newtable(l);
	int32_t tIdx = 1;
	for(Int32 i = 0; i < bulletInfo.bulletCount; i++) {
		auto randSpread = EulerAngles(umath::random(-bulletInfo.spread.p, bulletInfo.spread.p), umath::random(-bulletInfo.spread.y, bulletInfo.spread.y), 0);
		auto bulletDir = bulletInfo.direction;
		uvec::rotate(&bulletDir, randSpread);
		::TraceData data;
		data.SetSource(src);
		data.SetTarget(src + bulletDir * bulletInfo.distance);
		data.SetCollisionFilterMask(CollisionMask::AllHitbox & ~CollisionMask::Trigger); // Let everything pass (Except specific filters below)
		auto *attacker = dmg.GetAttacker();
		data.SetFilter([attacker](pragma::physics::IShape &shape, pragma::physics::IRigidBody &body) -> RayCastHitType {
			auto *phys = body.GetPhysObj();
			auto *ent = phys ? phys->GetOwner() : nullptr;
			if(ent == nullptr || &ent->GetEntity() == attacker) // Attacker can't shoot themselves
				return RayCastHitType::None;
			auto filterGroup = phys->GetCollisionFilter();
			auto mdlComponent = ent->GetEntity().GetModelComponent();
			if(mdlComponent && mdlComponent->GetHitboxCount() > 0 && (filterGroup & CollisionMask::NPC) != CollisionMask::None || (filterGroup & CollisionMask::Player) != CollisionMask::None) // Filter out player and NPC collision objects, since we only want to check their hitboxes
				return RayCastHitType::None;
			return RayCastHitType::Block;
		});
		auto filterGroup = CollisionMask::None;
		if(attacker != nullptr) {
			auto pPhysComponent = attacker->GetPhysicsComponent();
			if(pPhysComponent != nullptr)
				filterGroup = pPhysComponent->GetCollisionFilter();
			filterGroup |= CollisionMask::Water | CollisionMask::WaterSurface | CollisionMask::PlayerHitbox | CollisionMask::NPCHitbox;
		}
		else
			filterGroup = CollisionMask::AllHitbox;
		data.SetCollisionFilterGroup(filterGroup);
		std::vector<TraceResult> results {};
		if(game->RayCast(data, &results) && results.front().entity.valid()) {
			auto &result = results.front();
			auto pDamageableComponent = result.entity->GetComponent<pragma::DamageableComponent>();
			if(pDamageableComponent.valid()) {
				auto hitGroup = HitGroup::Generic;
				if(result.collisionObj.IsValid()) {
					auto charComponent = result.entity->GetCharacterComponent();
					if(charComponent.valid())
						charComponent->FindHitgroup(*result.collisionObj, hitGroup);
				}
				dmg.SetHitGroup(hitGroup);
				dmg.SetForce(bulletDir * bulletInfo.force);
				dmg.SetHitPosition(result.position);
				pDamageableComponent->TakeDamage(dmg);
			}
		}
		if(hitReport == true) {
			if(results.empty())
				t[tIdx++] = TraceResult {data};
			else {
				for(auto &result : results)
					t[tIdx++] = result;
			}
		}
		if(f != nullptr) {
			if(results.empty()) {
				TraceResult result {data};
				f(dmg, data, result, bulletInfo.tracerCount);
			}
			else {
				for(auto &result : results)
					f(dmg, data, result, bulletInfo.tracerCount);
			}
		}
	}
	return t;
}
luabind::object Lua::util::fire_bullets(lua_State *l, BulletInfo &bulletInfo, bool hitReport) { return fire_bullets(l, bulletInfo, hitReport, nullptr); }
luabind::object Lua::util::fire_bullets(lua_State *l, BulletInfo &bulletInfo) { return fire_bullets(l, bulletInfo, false, nullptr); }

static luabind::object register_class(lua_State *l, const std::string &pclassName, uint32_t idxBaseClassStart, const luabind::table<> *baseTable = nullptr)
{
	auto className = pclassName;
	auto fullClassName = className;

	auto nParentClasses = Lua::GetStackTop(l) - 1;
	auto fRegisterBaseClasses = [l, nParentClasses, idxBaseClassStart]() {
		for(auto i = idxBaseClassStart; i <= (nParentClasses + 1); ++i) {
			Lua::PushValue(l, -1);                                 /* 2 */
			Lua::PushValue(l, i);                                  /* 3 */
			if(Lua::ProtectedCall(l, 1, 0) != Lua::StatusCode::Ok) /* 1 */
				Lua::HandleLuaError(l);
		}
	};

	auto d = className.rfind('.');
	std::string slibs;
	if(d != std::string::npos) {
		slibs = className;
		className = ustring::substr(className, d + 1);

		std::vector<std::string> libs;
		ustring::explode(slibs, ".", libs);
		if(libs.empty() == false) {
			Lua::GetGlobal(l, libs.front()); /* 1 */
			if(Lua::IsSet(l, -1) == false) {
				Lua::Pop(l, 1); /* 0 */
				return {};
			}

			auto numPop = 0u;
			for(auto it = libs.begin() + 1; it != libs.end(); ++it) {
				auto bLast = (it == libs.end() - 1);
				auto &lib = *it;
				auto t = Lua::GetStackTop(l);
				auto status = Lua::GetProtectedTableValue(l, t, lib);
				if(status == Lua::StatusCode::Ok)
					++numPop; /* 1 */
				if(status != Lua::StatusCode::Ok || Lua::IsSet(l, -1) == false) {
					if(bLast == true)
						break;
					Lua::Pop(l, numPop + 1); /* 0 */
					return {};
				}
				if(bLast == true) {
					Lua::Pop(l, numPop + 1); /* 0 */

					auto *nw = engine->GetNetworkState(l);
					auto *game = nw->GetGameState();
					auto *classInfo = game->GetLuaClassManager().FindClassInfo(fullClassName);
					if(classInfo) {
						// Re-register base classes for this class, in case they have been changed
						classInfo->regFunc.push(l); /* 1 */
						fRegisterBaseClasses();
						Lua::Pop(l, 1); /* 0 */

						return classInfo->classObject;
					}

					return {};
				}
			}
			Lua::Pop(l, numPop + 1); /* 0 */
		}
	}

	if(slibs.empty()) {
		// Check if class already exists
		auto oBase = baseTable ? luabind::object {*baseTable} : luabind::globals(l);
		auto o = oBase[className];
		if(luabind::type(o) == LUA_TUSERDATA) {
			auto *nw = engine->GetNetworkState(l);
			auto *game = nw->GetGameState();
			auto *classInfo = game->GetLuaClassManager().FindClassInfo(fullClassName);
			if(classInfo) {
				// Re-register base classes for this class, in case they have been changed
				classInfo->regFunc.push(l); /* 1 */
				fRegisterBaseClasses();
				Lua::Pop(l, 1); /* 0 */

				return classInfo->classObject;
			}

			return {};
		}
	}

	auto restorePreviousGlobalValue = (!slibs.empty() || baseTable != nullptr);
	if(restorePreviousGlobalValue)
		Lua::GetGlobal(l, className); /* +1 */
	std::stringstream ss;
	ss << "return class '" << className << "'";
	auto r = Lua::RunString(l, ss.str(), 1, "internal"); /* 1 */
	luabind::object oClass {};
	if(r == Lua::StatusCode::Ok) {
		auto *nw = engine->GetNetworkState(l);
		auto *game = nw->GetGameState();
		oClass = luabind::globals(l)[className];
		luabind::object regFc {luabind::from_stack(l, -1)};
		game->GetLuaClassManager().RegisterClass(fullClassName, oClass, regFc);

		// Init default constructor and print methods; They can still be overwritten by the Lua script
		oClass["__init"] = luabind::make_function(
		  l, +[](lua_State *l, const luabind::object &o) {
			  static luabind::detail::class_rep *crep = nullptr;
			  if(crep == nullptr)
				  crep = Lua::get_crep(o);
			  if(!crep)
				  return;
			  std::vector<luabind::detail::class_rep *> initialized;
			  for(auto &base : crep->bases()) {
				  if(std::find(initialized.begin(), initialized.end(), base.base) != initialized.end())
					  continue;
				  initialized.push_back(base.base);

				  base.base->get_table(l);
				  auto oBase = luabind::object {luabind::from_stack(l, -1)};
				  crep = base.base;
				  if(crep)
					  oBase["__init"](o);
				  Lua::Pop(l, 1);
			  }
			  crep = nullptr;
		  });
		oClass["__tostring"] = luabind::make_function(
		  l, +[](lua_State *l, const luabind::object &o) -> std::string { return luabind::get_class_info(luabind::from_stack(l, 1)).name; });

		fRegisterBaseClasses();
		Lua::Pop(l, 1); /* 0 */

		if(slibs.empty() == false) {
			ss = std::stringstream {};
			ss << slibs << "=" << className;
			r = Lua::RunString(l, ss.str(), 1, "internal"); /* 1 */
			if(r != Lua::StatusCode::Ok)
				Lua::HandleLuaError(l);
			Lua::Pop(l, 1); /* 0 */

			Lua::PushNil(l);              /* 1 */
			Lua::SetGlobal(l, className); /* 0 */
		}
		else if(baseTable) {
			(*baseTable)[className] = oClass;
			Lua::PushNil(l);              /* 1 */
			Lua::SetGlobal(l, className); /* 0 */
		}
	}

	if(restorePreviousGlobalValue)
		Lua::SetGlobal(l, className); /* -1 */
	return oClass;
}
static luabind::object register_class(lua_State *l, const std::string &className, const luabind::object &oBase0, uint32_t idxBaseClassStart, const luabind::table<> *baseTable = nullptr)
{
	if(luabind::type(oBase0) == LUA_TTABLE) {
		uint32_t n = 0;
		for(luabind::iterator i(oBase0), e; i != e; ++i) {
			(*i).push(l);
			++n;
		}

		auto r = ::register_class(l, className, idxBaseClassStart + 1, baseTable);
		for(auto i = decltype(n) {0u}; i < n; ++i)
			Lua::RemoveValue(l, -2);
		return r;
	}
	return ::register_class(l, className, idxBaseClassStart, baseTable);
}
luabind::object Lua::util::register_class(lua_State *l, const std::string &className, const luabind::object &oBase0) { return ::register_class(l, className, oBase0, 2); }
luabind::object Lua::util::register_class(lua_State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1) { return ::register_class(l, className, 2); }
luabind::object Lua::util::register_class(lua_State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2) { return ::register_class(l, className, 2); }
luabind::object Lua::util::register_class(lua_State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3) { return ::register_class(l, className, 2); }
luabind::object Lua::util::register_class(lua_State *l, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3, const luabind::object &oBase4) { return ::register_class(l, className, 2); }

luabind::object Lua::util::register_class(lua_State *l, const std::string &pclassName) { return ::register_class(l, pclassName, 2); }

void Lua::util::splash_damage(lua_State *l, const ::util::SplashDamageInfo &splashDamageInfo)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto callback = splashDamageInfo.callback;
	if(splashDamageInfo.cone.has_value()) {
		auto coneAngle = 1.f - (splashDamageInfo.cone->second / 180.f) * 2.f;
		auto &forward = splashDamageInfo.cone->first;
		auto posEnd = splashDamageInfo.origin + forward * static_cast<float>(splashDamageInfo.radius);
		callback = [&splashDamageInfo, posEnd, forward, coneAngle](BaseEntity *ent, DamageInfo &dmgInfo) -> bool {
			Vector3 min {};
			Vector3 max {};
			auto pPhysComponent = ent->GetPhysicsComponent();
			if(pPhysComponent != nullptr)
				pPhysComponent->GetCollisionBounds(&min, &max);
			auto pTrComponent = ent->GetTransformComponent();
			auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
			min += pos;
			max += pos;
			Vector3 posCone {};
			umath::geometry::closest_point_on_aabb_to_point(min, max, posEnd, &posCone);
			auto dirEnt = posCone - splashDamageInfo.origin;
			uvec::normalize(&dirEnt);
			if(uvec::dot(forward, dirEnt) < coneAngle)
				return false;
			if(splashDamageInfo.callback != nullptr)
				return splashDamageInfo.callback(ent, dmgInfo);
			return true;
		};
	}
	game->SplashDamage(splashDamageInfo.origin, splashDamageInfo.radius, const_cast<DamageInfo &>(splashDamageInfo.damageInfo), splashDamageInfo.callback);
}

luabind::object Lua::util::register_class(lua_State *l, const luabind::table<> &t, const std::string &className) { return ::register_class(l, className, 3, &t); }
luabind::object Lua::util::register_class(lua_State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0) { return ::register_class(l, className, oBase0, 3, &t); }
luabind::object Lua::util::register_class(lua_State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1) { return ::register_class(l, className, 3); }
luabind::object Lua::util::register_class(lua_State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2) { return ::register_class(l, className, 3); }
luabind::object Lua::util::register_class(lua_State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3) { return ::register_class(l, className, 3); }
luabind::object Lua::util::register_class(lua_State *l, const luabind::table<> &t, const std::string &className, const luabind::object &oBase0, const luabind::object &oBase1, const luabind::object &oBase2, const luabind::object &oBase3, const luabind::object &oBase4)
{
	return ::register_class(l, className, 3);
}

static void shake_screen(lua_State *l, const Vector3 &pos, float radius, float amplitude, float frequency, float duration, float fadeIn, float fadeOut, bool useRadius)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto *ent = game->CreateEntity("env_quake");
	auto *pQuakeComponent = (ent != nullptr) ? static_cast<pragma::BaseEnvQuakeComponent *>(ent->FindComponent("quake").get()) : nullptr;
	if(pQuakeComponent != nullptr) {
		auto spawnFlags = SF_QUAKE_IN_AIR | SF_QUAKE_REMOVE_ON_COMPLETE;
		if(useRadius == true) {
			auto pTrComponent = ent->GetTransformComponent();
			if(pTrComponent != nullptr)
				pTrComponent->SetPosition(pos);
			pQuakeComponent->SetRadius(radius);
		}
		else
			spawnFlags |= SF_QUAKE_GLOBAL_SHAKE;
		ent->SetKeyValue("spawnflags", ustring::int_to_string(spawnFlags));
		pQuakeComponent->SetAmplitude(CFloat(amplitude));
		pQuakeComponent->SetFrequency(CFloat(frequency));
		pQuakeComponent->SetDuration(CFloat(duration));
		pQuakeComponent->SetFadeInDuration(CFloat(fadeIn));
		pQuakeComponent->SetFadeOutDuration(CFloat(fadeOut));
		ent->Spawn();
		auto *pInputComponent = static_cast<pragma::BaseIOComponent *>(ent->FindComponent("io").get());
		if(pInputComponent != nullptr)
			pInputComponent->Input("StartShake", nullptr, nullptr, "");
	}
}
void Lua::util::shake_screen(lua_State *l, const Vector3 &pos, float radius, float amplitude, float frequency, float duration, float fadeIn, float fadeOut) { ::shake_screen(l, pos, radius, amplitude, frequency, duration, fadeIn, fadeOut, true); }
void Lua::util::shake_screen(lua_State *l, float amplitude, float frequency, float duration, float fadeIn, float fadeOut) { ::shake_screen(l, {}, {}, amplitude, frequency, duration, fadeIn, fadeOut, false); }

float Lua::util::get_faded_time_factor(float cur, float dur, float fadeIn, float fadeOut) { return ::util::get_faded_time_factor(CFloat(cur), CFloat(dur), fadeIn, fadeOut); }
float Lua::util::get_faded_time_factor(float cur, float dur, float fadeIn) { return get_faded_time_factor(cur, dur, fadeIn, 0.f); }
float Lua::util::get_faded_time_factor(float cur, float dur) { return get_faded_time_factor(cur, dur, 0.f); }

float Lua::util::get_scale_factor(float val, float min, float max) { return ::util::get_scale_factor(CFloat(val), CFloat(min), CFloat(max)); }
float Lua::util::get_scale_factor(float val, float min) { return ::util::get_scale_factor(CFloat(val), CFloat(min)); }

Quat Lua::util::local_to_world(lua_State *l, const Quat &r0, const Quat &r1)
{
	auto res = r1;
	uvec::local_to_world(r0, res);
	return res;
}

Vector3 Lua::util::local_to_world(lua_State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v)
{
	auto vOut = v;
	uvec::local_to_world(vLocal, rLocal, vOut);
	return vOut;
}

void Lua::util::local_to_world(lua_State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v, const Quat &r)
{
	auto vOut = v;
	auto rOut = r;
	uvec::local_to_world(vLocal, rLocal, vOut, rOut);
	Lua::Push<Vector3>(l, vOut);
	Lua::Push<Quat>(l, rOut);
}

Quat Lua::util::world_to_local(lua_State *l, const Quat &rLocal, const Quat &r)
{
	auto rOut = r;
	uvec::world_to_local(rLocal, rOut);
	return rOut;
}

Vector3 Lua::util::world_to_local(lua_State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v)
{
	auto vOut = v;
	uvec::world_to_local(vLocal, rLocal, vOut);
	return vOut;
}

void Lua::util::world_to_local(lua_State *l, const Vector3 &vLocal, const Quat &rLocal, const Vector3 &v, const Quat &r)
{
	auto vOut = v;
	auto rOut = r;
	uvec::world_to_local(vLocal, rLocal, vOut, rOut);
	Lua::Push<Vector3>(l, vOut);
	Lua::Push<Quat>(l, rOut);
}

Vector3 Lua::util::calc_world_direction_from_2d_coordinates(lua_State *l, const Vector3 &forward, const Vector3 &right, const Vector3 &up, float fov, float nearZ, float farZ, float aspectRatio, const Vector2 &uv)
{
	return uvec::calc_world_direction_from_2d_coordinates(forward, right, up, static_cast<float>(umath::deg_to_rad(fov)), nearZ, farZ, aspectRatio, 0.f, 0.f, uv);
}
void Lua::util::world_space_point_to_screen_space_uv(lua_State *l, const Vector3 &point, const Mat4 &vp, float nearZ, float farZ)
{
	float dist;
	auto uv = uvec::calc_screenspace_uv_from_worldspace_position(point, vp, nearZ, farZ, dist);
	Lua::Push(l, uv);
	Lua::PushNumber(l, dist);
}
::Vector2 Lua::util::world_space_point_to_screen_space_uv(lua_State *l, const Vector3 &point, const Mat4 &vp) { return uvec::calc_screenspace_uv_from_worldspace_position(point, vp); }
Vector2 Lua::util::world_space_direction_to_screen_space(lua_State *l, const Vector3 &dir, const Mat4 &vp) { return uvec::calc_screenspace_direction_from_worldspace_direction(dir, vp); }
float Lua::util::calc_screenspace_distance_to_worldspace_position(lua_State *l, const Vector3 &point, const Mat4 &vp, float nearZ, float farZ) { return uvec::calc_screenspace_distance_to_worldspace_position(point, vp, nearZ, farZ); }
float Lua::util::depth_to_distance(lua_State *l, float depth, float nearZ, float farZ) { return uvec::depth_to_distance(depth, nearZ, farZ); }
void Lua::util::open_url_in_browser(const std::string &url) { return ::util::open_url_in_browser(url); }
void Lua::util::open_path_in_explorer(const std::string &spath, const std::string &selectFile)
{
	auto path = ::util::Path::CreatePath(spath) + ::util::Path::CreateFile(selectFile);
	std::string strAbsPath;
	if(FileManager::FindAbsolutePath(path.GetString(), strAbsPath) == false)
		return;
	auto absPath = ::util::Path::CreateFile(strAbsPath);
	::util::open_path_in_explorer(std::string {absPath.GetPath()}, std::string {absPath.GetFileName()});
}
void Lua::util::open_path_in_explorer(const std::string &spath)
{
	auto path = ::util::Path::CreatePath(spath);
	std::string strAbsPath;
	if(FileManager::FindAbsolutePath(path.GetString(), strAbsPath) == false)
		return;
	auto absPath = ::util::Path::CreatePath(strAbsPath);
	::util::open_path_in_explorer(std::string {absPath.GetPath()});
}
void Lua::util::clamp_resolution_to_aspect_ratio(lua_State *l, uint32_t w, uint32_t h, float aspectRatio)
{
	Vector2i size {w, h};
	w = size.y * aspectRatio;
	h = size.y;
	if(w > size.x) {
		w = size.x;
		h = size.x / aspectRatio;
	}
	Lua::PushNumber(l, w);
	Lua::PushNumber(l, h);
}
std::string Lua::util::get_pretty_bytes(uint32_t bytes) { return ::util::get_pretty_bytes(bytes); }
std::string Lua::util::get_pretty_duration(lua_State *l, uint32_t ms) { return get_pretty_duration(l, ms, 0, true); }
std::string Lua::util::get_pretty_duration(lua_State *l, uint32_t ms, uint32_t segments) { return get_pretty_duration(l, ms, segments, true); }
std::string Lua::util::get_pretty_duration(lua_State *l, uint32_t ms, uint32_t segments, bool noMs) { return ::util::get_pretty_duration(ms, segments, noMs); }
bool Lua::util::is_same_object(lua_State *l, const luabind::object &o0, const luabind::object &o1) { return lua_rawequal(l, 1, 2) == 1; }
std::string Lua::util::get_pretty_time(lua_State *l, float t)
{
	auto sign = umath::sign(static_cast<float>(t));
	t *= sign;
	auto seconds = umath::floor(t);
	auto milliseconds = umath::floor((t - seconds) * 1'000.f);
	auto minutes = umath::floor(seconds / 60.f);
	seconds -= minutes * 60.0;
	auto hours = umath::floor(minutes / 60.f);
	minutes -= hours * 60.f;

	auto prettyTime = ustring::fill_zeroes(::util::round_string(seconds), 2) + '.' + ustring::fill_zeroes(::util::round_string(milliseconds), 3);

	prettyTime = ustring::fill_zeroes(::util::round_string(minutes), 2) + ':' + prettyTime;

	if(hours > 0.f)
		prettyTime = ustring::fill_zeroes(::util::round_string(hours), 2) + ':' + prettyTime;

	if(sign < 0)
		prettyTime = '-' + prettyTime;
	return prettyTime;
}

double Lua::util::units_to_metres(double units) { return ::pragma::units_to_metres(units); }
double Lua::util::metres_to_units(double metres) { return ::pragma::metres_to_units(metres); }
luabind::object Lua::util::read_scene_file(lua_State *l, const std::string &fileName)
{
	auto fname = "scenes\\" + FileManager::GetCanonicalizedPath(fileName);
	auto f = FileManager::OpenFile(fname.c_str(), "r");
	if(f == nullptr)
		return {};
	source_engine::script::SceneScriptValue root {};
	if(source_engine::script::read_scene(f, root) != ::util::MarkupFile::ResultCode::Ok)
		return {};
	std::function<void(const source_engine::script::SceneScriptValue &)> fPushValue = nullptr;
	fPushValue = [l, &fPushValue](const source_engine::script::SceneScriptValue &val) {
		auto t = Lua::CreateTable(l);

		Lua::PushString(l, "identifier");
		Lua::PushString(l, val.identifier);
		Lua::SetTableValue(l, t);

		Lua::PushString(l, "parameters");
		auto tParams = Lua::CreateTable(l);
		auto paramIdx = 1u;
		for(auto &param : val.parameters) {
			Lua::PushInt(l, paramIdx++);
			Lua::PushString(l, param);
			Lua::SetTableValue(l, tParams);
		}
		Lua::SetTableValue(l, t);

		Lua::PushString(l, "children");
		auto tChildren = Lua::CreateTable(l);
		auto childIdx = 1u;
		for(auto &child : val.subValues) {
			Lua::PushInt(l, childIdx++);
			fPushValue(*child);
			Lua::SetTableValue(l, tChildren);
		}
		Lua::SetTableValue(l, t);
	};
	fPushValue(root);
	auto o = luabind::object {luabind::from_stack {l, -1}};
	Lua::Pop(l, 1);
	return o;
}

template<class TProperty, typename TUnderlyingType>
static luabind::object fade_property_generic(Game &game, lua_State *l, TProperty &vProp, const TUnderlyingType &vDst, float duration, const std::function<TUnderlyingType(const TUnderlyingType &, const TUnderlyingType &, float)> &fLerp)
{
	auto vSrc = vProp.GetValue();
	if(duration == 0.f) {
		vProp.SetValue(vDst);
		return luabind::object {};
	}
	auto tStart = game.RealTime();
	auto cb = FunctionCallback<void>::Create(nullptr);
	cb.get<Callback<void>>()->SetFunction([&game, tStart, duration, vProp, vSrc, vDst, cb, fLerp]() mutable {
		auto tDelta = game.RealTime() - tStart;
		auto sc = umath::min(umath::smooth_step(0.f, 1.f, static_cast<float>(tDelta / duration)), 1.f);
		auto vNew = fLerp(vSrc, vDst, sc);
		vProp.SetValue(vNew);
		if(sc == 1.f) {
			if(cb.IsValid())
				cb.Remove();
		}
	});
	game.AddCallback("Think", cb);
	return luabind::object {l, cb};
}

template<class TProperty, typename TUnderlyingType>
luabind::object fade_vector_property_generic(Game &game, lua_State *l, TProperty &vProp, const TUnderlyingType &vDst, float duration, const std::function<TUnderlyingType(const TUnderlyingType &, const TUnderlyingType &, float)> &fLerp)
{
	return fade_property_generic<TProperty, TUnderlyingType>(game, l, vProp, vDst, duration, fLerp);
}

luabind::object Lua::util::fade_property(lua_State *l, LColorProperty &colProp, const Color &colDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	auto hsvSrc = ::util::rgb_to_hsv(*colProp);
	auto hsvDst = ::util::rgb_to_hsv(colDst);
	auto aSrc = (*colProp)->a;
	auto aDst = colDst.a;

	auto tStart = game.RealTime();
	auto cb = FunctionCallback<void>::Create(nullptr);
	cb.get<Callback<void>>()->SetFunction([&game, tStart, duration, colProp, hsvSrc, hsvDst, aSrc, aDst, cb]() mutable {
		auto tDelta = game.RealTime() - tStart;
		auto sc = umath::min(umath::smooth_step(0.f, 1.f, static_cast<float>(tDelta / duration)), 1.f);
		auto hsv = ::util::lerp_hsv(hsvSrc, hsvDst, sc);
		auto newColor = ::util::hsv_to_rgb(hsv);
		newColor.a = aSrc + (aDst - aSrc) * sc;
		*colProp = newColor;
		if(sc == 1.f) {
			if(cb.IsValid())
				cb.Remove();
		}
	});
	game.AddCallback("Think", cb);
	return luabind::object {l, cb};
}

luabind::object Lua::util::fade_property(lua_State *l, LVector2Property &vProp, const ::Vector2 &vDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector2Property, Vector2>(game, l, vProp, vDst, duration, [](const Vector2 &a, const Vector2 &b, float factor) -> Vector2 { return Vector2 {umath::lerp(a.x, b.x, factor), umath::lerp(a.y, b.y, factor)}; });
}

luabind::object Lua::util::fade_property(lua_State *l, LVector2iProperty &vProp, const ::Vector2i &vDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector2iProperty, Vector2i>(game, l, vProp, vDst, duration, [](const Vector2i &a, const Vector2i &b, float factor) -> Vector2i {
		return Vector2i {static_cast<int32_t>(umath::lerp(a.x, b.x, factor)), static_cast<int32_t>(umath::lerp(a.y, b.y, factor))};
	});
}

luabind::object Lua::util::fade_property(lua_State *l, LVector3Property &vProp, const ::Vector3 &vDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector3Property, Vector3>(game, l, vProp, vDst, duration, uvec::lerp);
}

luabind::object Lua::util::fade_property(lua_State *l, LVector3iProperty &vProp, const ::Vector3i &vDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector3iProperty, Vector3i>(game, l, vProp, vDst, duration, uvec::lerp);
}

luabind::object Lua::util::fade_property(lua_State *l, LVector4Property &vProp, const ::Vector4 &vDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector4Property, Vector4>(game, l, vProp, vDst, duration, [](const Vector4 &a, const Vector4 &b, float factor) -> Vector4 {
		return Vector4 {umath::lerp(a.x, b.x, factor), umath::lerp(a.y, b.y, factor), umath::lerp(a.z, b.z, factor), umath::lerp(a.w, b.w, factor)};
	});
}

luabind::object Lua::util::fade_property(lua_State *l, LVector4iProperty &vProp, const ::Vector4i &vDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LVector4iProperty, Vector4i>(game, l, vProp, vDst, duration, [](const Vector4i &a, const Vector4i &b, float factor) -> Vector4i {
		return Vector4i {static_cast<int32_t>(umath::lerp(a.x, b.x, factor)), static_cast<int32_t>(umath::lerp(a.y, b.y, factor)), static_cast<int32_t>(umath::lerp(a.z, b.z, factor)), static_cast<int32_t>(umath::lerp(a.w, b.w, factor))};
	});
}

luabind::object Lua::util::fade_property(lua_State *l, LQuatProperty &vProp, const ::Quat &vDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LQuatProperty, Quat>(game, l, vProp, vDst, duration, uquat::slerp);
}

luabind::object Lua::util::fade_property(lua_State *l, LEulerAnglesProperty &vProp, const ::EulerAngles &vDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LEulerAnglesProperty, EulerAngles>(game, l, vProp, vDst, duration, [](const EulerAngles &a, const EulerAngles &b, float factor) -> EulerAngles {
		return EulerAngles {static_cast<float>(umath::lerp_angle(a.p, b.p, factor)), static_cast<float>(umath::lerp_angle(a.y, b.y, factor)), static_cast<float>(umath::lerp_angle(a.r, b.r, factor))};
	});
}

luabind::object Lua::util::fade_property(lua_State *l, LGenericIntPropertyWrapper &vProp, const int64_t &vDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LGenericIntPropertyWrapper, int64_t>(game, l, vProp, vDst, duration, [](const int64_t &a, const int64_t &b, float factor) -> int64_t { return umath::lerp(a, b, factor); });
}

luabind::object Lua::util::fade_property(lua_State *l, LGenericFloatPropertyWrapper &vProp, const double &vDst, float duration)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	return fade_vector_property_generic<LGenericFloatPropertyWrapper, double>(game, l, vProp, vDst, duration, [](const double &a, const double &b, float factor) -> double { return umath::lerp(a, b, factor); });
}

std::string Lua::util::round_string(lua_State *l, float value) { return round_string(l, value, 0); }
std::string Lua::util::round_string(lua_State *l, float value, uint32_t places) { return ::util::round_string(value, places); }

std::string Lua::util::get_type_name(lua_State *l, const luabind::object &o)
{
	if(Lua::IsSet(l, 1) == false)
		return Lua::IsNone(l, 1) ? "none" : "nil";

	auto *crep = Lua::get_crep(o);
	if(crep)
		return crep->name();
	return lua_typename(l, lua_type(l, -1));
}

std::string Lua::util::variable_type_to_string(::util::VarType varType) { return ::util::variable_type_to_string(varType); }
std::string Lua::util::get_string_hash(const std::string &str) { return std::to_string(std::hash<std::string> {}(str)); }

luabind::object Lua::util::get_class_value(lua_State *l, const luabind::object &oClass, const std::string &key)
{
	int32_t t = 1;
	Lua::CheckUserData(l, t);

	auto n = Lua::GetStackTop(l);
	Lua::PushString(l, key);                    /* 1 */
	auto r = Lua::GetProtectedTableValue(l, t); /* 2 */
	if(r != Lua::StatusCode::Ok) {
		Lua::Pop(l, Lua::GetStackTop(l) - n);
		return {};
	}
	// Pop key from stack
	Lua::RemoveValue(l, -2); /* 1 */
	luabind::object ro {luabind::from_stack {l, -1}};
	Lua::Pop(l);
	return ro;
}

Lua::var<bool, ::util::ParallelJob<luabind::object>> Lua::util::pack_zip_archive(lua_State *l, Game &game, const std::string &pzipFileName, const luabind::table<> &t)
{
	auto zipFileName = pzipFileName;
	ufile::remove_extension_from_filename(zipFileName);
	zipFileName += ".zip";
	if(Lua::file::validate_write_operation(l, zipFileName) == false)
		return luabind::object {l, false};

	std::unordered_map<std::string, std::string> files {};
	std::unordered_map<std::string, std::string> customTextFiles {};
	std::unordered_map<std::string, DataStream> customBinaryFiles {};
	auto numFiles = Lua::GetObjectLength(l, t);
	if(numFiles > 0) {
		// Table format: t{[1] = diskFileName/zipFileName,...}
		files.reserve(numFiles);
		for(luabind::iterator i(t), e; i != e; ++i) {
			auto fileName = luabind::object_cast<std::string>(*i);
			files[fileName] = fileName;
		}
	}
	else {
		// Table format: t{[zipFileName] = diskFileName,...}
		for(luabind::iterator i(t), e; i != e; ++i) {
			auto zipFileName = luabind::object_cast<std::string>(i.key());
			auto value = *i;
			if(luabind::type(value) == LUA_TTABLE) {
				auto *ds = luabind::object_cast_nothrow<DataStream *>(value["contents"], static_cast<DataStream *>(nullptr));
				if(ds)
					customBinaryFiles[zipFileName] = *ds;
				else
					customTextFiles[zipFileName] = luabind::object_cast<std::string>(value["contents"]);
			}
			else {
				auto diskFileName = luabind::object_cast<std::string>(*i);
				files[zipFileName] = diskFileName;
			}
		}
	}

	auto zip = uzip::ZIPFile::Open(zipFileName, uzip::OpenMode::Write);
	if(zip == nullptr)
		return luabind::object {l, false};
	auto pzip = std::shared_ptr<uzip::ZIPFile> {std::move(zip)};

	struct ResultData {
		std::vector<std::string> notFound;
	};
	auto job = ::util::create_parallel_job<::util::TFunctionalParallelWorkerWithResult<ResultData>, bool>(false);
	auto &worker = static_cast<::util::TFunctionalParallelWorkerWithResult<ResultData> &>(job.GetWorker());
	worker.ResetTask([files = std::move(files), customTextFiles = std::move(customTextFiles), customBinaryFiles = std::move(customBinaryFiles), pzip = std::move(pzip)](::util::TFunctionalParallelWorker<ResultData> &worker) mutable {
		std::vector<std::string> notFound;
		size_t numFilesTotal = files.size() + customTextFiles.size() + customBinaryFiles.size();
		size_t curFileIdx = 0;
		auto incrementFileIndex = [&worker, &curFileIdx, numFilesTotal]() {
			auto progress = (numFilesTotal > 0) ? (curFileIdx / static_cast<float>(numFilesTotal)) : 1.f;
			worker.UpdateProgress(progress * 0.05f);
			++curFileIdx;
		};
		for(auto &pair : files) {
			incrementFileIndex();
			if(worker.IsCancelled())
				return;
			auto f = filemanager::open_file(pair.second, filemanager::FileMode::Read | filemanager::FileMode::Binary);
			if(f == nullptr) {
				if(notFound.size() == notFound.capacity())
					notFound.reserve(notFound.size() * 1.5 + 100);
				notFound.push_back(pair.second);
				continue;
			}
			auto sz = f->GetSize();
			std::vector<uint8_t> data {};
			data.resize(sz);
			f->Read(data.data(), sz);
			if(worker.IsCancelled())
				return;
			pzip->AddFile(pair.first, data.data(), sz);
		}
		for(auto &pair : customTextFiles) {
			incrementFileIndex();
			if(worker.IsCancelled())
				return;
			pzip->AddFile(pair.first, pair.second);
		}
		for(auto &pair : customBinaryFiles) {
			incrementFileIndex();
			if(worker.IsCancelled())
				return;
			auto &ds = pair.second;
			pzip->AddFile(pair.first, ds->GetData(), ds->GetInternalSize());
		}
		incrementFileIndex();
		pzip->SetPackProgressCallback([&worker](double progress) { worker.UpdateProgress(0.05f + progress * 0.95f); });
		pzip = nullptr;

		if(!worker.IsCancelled()) {
			ResultData resultData {};
			resultData.notFound = std::move(notFound);
			static_cast<::util::TFunctionalParallelWorkerWithResult<ResultData> &>(worker).SetResult(std::move(resultData));
			worker.SetStatus(::util::JobStatus::Successful);
		}
	});
	auto pjob = std::make_shared<::util::ParallelJob<ResultData>>(job);
	auto jobWrapper = std::make_shared<::util::ParallelJob<luabind::object>>(::util::create_parallel_job<pragma::lua::LuaWorker>(game, "pack_zip_archive"));
	auto &workerWrapper = static_cast<pragma::lua::LuaWorker &>(jobWrapper->GetWorker());
	workerWrapper.AddCppTask(
	  pjob,
	  [&worker, &workerWrapper, l]() {
		  auto &resultData = worker.GetResultRef();
		  luabind::object o {l, resultData.notFound};
		  workerWrapper.SetResult(o);
	  },
	  1.f);
	return luabind::object {l, jobWrapper};
}

std::string Lua::util::get_addon_path(lua_State *l, const std::string &relPath)
{
	std::string rpath;
	if(FileManager::FindAbsolutePath(relPath, rpath) == false)
		return relPath;
	::util::Path path {relPath};
	path.MakeRelative(::util::get_program_path());
	return path.GetString();
}

std::string Lua::util::get_addon_path(lua_State *l)
{
	auto path = Lua::get_current_file(l);
	ustring::replace(path, "\\", "/");
	if(ustring::compare(path.c_str(), "addons/", false, 7) == false)
		return "";

	// Get path up to addon directory
	auto br = path.find('/', 7);
	if(br != std::string::npos)
		path = path.substr(0, br);
	path += '/';
	return path;
}
