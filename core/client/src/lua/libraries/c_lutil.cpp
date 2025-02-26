/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/libraries/c_lutil.h"
#include "pragma/lua/classes/c_lcamera.h"
#include "pragma/entities/point/c_point_target.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "luasystem.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_attachment_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/util/util_baking.hpp"
#include "pragma/asset/c_util_model.hpp"
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/ldef_quaternion.h>
#include <pragma/lua/classes/ldef_entity.h>
#include <pragma/lua/libraries/lfile.h>
#include <pragma/lua/classes/ldef_material.h>
#include <pragma/util/giblet_create_info.hpp>
#include <pragma/asset_types/world.hpp>
#include <pragma/model/model.h>
#include <util_image_buffer.hpp>
#include <prosper_window.hpp>
#include <fsys/ifile.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

int Lua::util::Client::calc_world_direction_from_2d_coordinates(lua_State *l, pragma::CCameraComponent &hCam, const Vector2 &uv)
{
	auto trComponent = hCam.GetEntity().GetTransformComponent();
	auto forward = trComponent ? trComponent->GetForward() : uvec::FORWARD;
	auto right = trComponent ? trComponent->GetRight() : uvec::RIGHT;
	auto up = trComponent ? trComponent->GetUp() : uvec::UP;
	auto dir = uvec::calc_world_direction_from_2d_coordinates(forward, right, up, hCam.GetFOVRad(), hCam.GetNearZ(), hCam.GetFarZ(), hCam.GetAspectRatio(), 0.f, 0.f, uv);
	Lua::Push<Vector3>(l, dir);
	return 1;
}

int Lua::util::Client::create_particle_tracer(lua_State *l)
{
	auto &start = *Lua::CheckVector(l, 1);
	auto &end = *Lua::CheckVector(l, 2);
	auto radius = Lua::IsSet(l, 3) ? Lua::CheckNumber(l, 3) : BulletInfo::DEFAULT_TRACER_RADIUS;
	const auto *col = Lua::IsSet(l, 4) ? Lua::CheckColor(l, 4) : &BulletInfo::DEFAULT_TRACER_COLOR;
	auto length = Lua::IsSet(l, 5) ? Lua::CheckNumber(l, 5) : BulletInfo::DEFAULT_TRACER_LENGTH;
	auto speed = Lua::IsSet(l, 6) ? Lua::CheckNumber(l, 6) : BulletInfo::DEFAULT_TRACER_SPEED;
	auto *mat = Lua::IsSet(l, 7) ? Lua::CheckString(l, 7) : BulletInfo::DEFAULT_TRACER_MATERIAL.c_str();
	auto bloomScale = Lua::IsSet(l, 8) ? Lua::CheckNumber(l, 8) : BulletInfo::DEFAULT_TRACER_BLOOM;

	auto *particle = c_game->CreateParticleTracer(start, end, static_cast<float>(radius), *col, static_cast<float>(length), static_cast<float>(speed), mat, static_cast<float>(bloomScale));
	if(particle == nullptr)
		return 0;
	particle->PushLuaObject(l);
	return 1;
}

int Lua::util::Client::create_muzzle_flash(lua_State *l)
{
	if(Lua::IsType<BaseEntity>(l, 1) == true) {
		auto &ent = Lua::Check<BaseEntity>(l, 1);
		int32_t attId = -1;
		std::string att {};
		if(Lua::IsNumber(l, 2))
			attId = Lua::CheckInt(l, 2);
		else
			att = Lua::CheckString(l, 2);
		Vector3 relOffset {};
		auto relRot = uquat::identity();
		if(Lua::IsSet(l, 3)) {
			relOffset = *Lua::CheckVector(l, 3);
			if(Lua::IsSet(l, 4))
				relRot = *Lua::CheckQuaternion(l, 4);
		}
		std::string particleName = "muzzleflash0" + std::to_string(umath::random(1, 6));
		auto *pt = pragma::CParticleSystemComponent::Create(particleName);
		if(pt == nullptr)
			return 0;
		auto pRenderComponent = static_cast<CBaseEntity *>(&ent)->GetRenderComponent();
		if(pRenderComponent)
			pt->SetSceneRenderPass(pRenderComponent->GetSceneRenderPass());
		pt->GetEntity().SetKeyValue("transform_with_emitter", "1");
		pt->SetRemoveOnComplete(true);
		auto &entPt = pt->GetEntity();
		pt->Start();
		auto pAttachableComponent = entPt.AddComponent<pragma::CAttachmentComponent>();
		if(pAttachableComponent.valid()) {
			AttachmentInfo attInfo {};
			attInfo.offset = relOffset;
			attInfo.rotation = relRot;
			attInfo.flags |= FAttachmentMode::UpdateEachFrame;
			if(att.empty() == false)
				pAttachableComponent->AttachToAttachment(&ent, att, attInfo);
			else
				pAttachableComponent->AttachToAttachment(&ent, attId, attInfo);
		}
		pt->PushLuaObject(l);
		return 1;
	}
	auto &pos = *Lua::CheckVector(l, 1);
	auto &rot = *Lua::CheckQuaternion(l, 2);
	std::string particleName = "muzzleflash0" + std::to_string(umath::random(1, 6));
	auto *pt = pragma::CParticleSystemComponent::Create(particleName);
	if(pt == nullptr)
		return 0;
	auto pTrComponent = pt->GetEntity().GetTransformComponent();
	if(pTrComponent != nullptr) {
		pTrComponent->SetPosition(pos);
		pTrComponent->SetRotation(rot);
	}
	pt->SetRemoveOnComplete(true);
	pt->Start();
	pt->PushLuaObject(l);
	return 1;
}

luabind::object Lua::util::Client::create_giblet(GibletCreateInfo &createInfo)
{
	pragma::CParticleSystemComponent *particle = nullptr;
	c_game->CreateGiblet(createInfo, &particle);
	if(particle == nullptr)
		return {};
	return particle->GetLuaObject();
}

int Lua::util::Client::import_gltf(lua_State *l)
{
	std::shared_ptr<ufile::IFile> f = nullptr;
	std::string fileName;
	if(Lua::IsString(l, 1))
		fileName = Lua::CheckString(l, 1);
	else {
		auto *lf = Lua::CheckFile(l, 1);
		if(lf == nullptr)
			return 0;
		f = lf->GetHandle();
	}
	::util::Path outputPath {};
	if(Lua::IsSet(l, 2))
		outputPath = ::util::Path::CreatePath(Lua::CheckString(l, 2));
	auto importAsSingleModel = false;
	if(Lua::IsSet(l, 3))
		importAsSingleModel = Lua::CheckBool(l, 3);
	std::string errMsg;
	std::optional<pragma::asset::AssetImportResult> importInfo {};
	if(f) {
		importInfo = pragma::asset::import_gltf(*f, errMsg, outputPath, importAsSingleModel);
	}
	else
		importInfo = pragma::asset::import_gltf(fileName, errMsg, outputPath, importAsSingleModel);
	if(!importInfo) {
		Lua::PushBool(l, false);
		Lua::PushString(l, errMsg);
		return 2;
	}
	auto t = luabind::newtable(l);
	t["mapName"] = importInfo->mapName;
	t["models"] = importInfo->models;
	t.push(l);
	return 1;
}

int Lua::util::Client::import_model(lua_State *l)
{
	std::shared_ptr<ufile::IFile> f = nullptr;
	std::string fileName;
	if(Lua::IsString(l, 1))
		fileName = Lua::CheckString(l, 1);
	else {
		auto *lf = Lua::CheckFile(l, 1);
		if(lf == nullptr)
			return 0;
		f = lf->GetHandle();
	}
	::util::Path outputPath {};
	if(Lua::IsSet(l, 2))
		outputPath = ::util::Path::CreatePath(Lua::CheckString(l, 2));
	auto importAsSingleModel = true;
	if(Lua::IsSet(l, 3))
		importAsSingleModel = Lua::CheckBool(l, 3);
	std::string errMsg;
	std::shared_ptr<Model> mdl = nullptr;
	if(f) {
		mdl = pragma::asset::import_model(*f, errMsg, outputPath, importAsSingleModel);
	}
	else
		mdl = pragma::asset::import_model(fileName, errMsg, outputPath, importAsSingleModel);
	if(mdl == nullptr) {
		Lua::PushBool(l, false);
		Lua::PushString(l, errMsg);
		return 2;
	}
	Lua::Push(l, mdl);
	return 1;
}

int Lua::util::Client::export_map(lua_State *l)
{
	std::string mapName = Lua::CheckString(l, 1);
	auto &exportInfo = Lua::Check<pragma::asset::ModelExportInfo>(l, 2);

	pragma::asset::MapExportInfo mapExportInfo {};
	if(Lua::IsSet(l, 3))
		mapExportInfo = Lua::Check<pragma::asset::MapExportInfo>(l, 3);

	std::string errMsg;
	auto result = pragma::asset::export_map(mapName, exportInfo, errMsg, mapExportInfo);
	Lua::PushBool(l, result);
	if(result)
		return 1;
	Lua::PushString(l, errMsg);
	return 2;
}

int Lua::util::Client::export_texture(lua_State *l)
{
	if(Lua::IsString(l, 1)) {
		std::string texturePath = Lua::CheckString(l, 1);
		auto imgFormat = static_cast<pragma::asset::ModelExportInfo::ImageFormat>(Lua::CheckInt(l, 2));
		auto alphaMode = uimg::TextureInfo::AlphaMode::Auto;
		if(Lua::IsSet(l, 3))
			alphaMode = static_cast<uimg::TextureInfo::AlphaMode>(Lua::CheckInt(l, 3));
		auto enabledExtendedDDs = false;
		if(Lua::IsSet(l, 4))
			enabledExtendedDDs = Lua::CheckBool(l, 4);

		std::string errMsg;
		std::string outputPath;
		auto result = pragma::asset::export_texture(texturePath, imgFormat, errMsg, alphaMode, enabledExtendedDDs, nullptr, &outputPath);
		Lua::PushBool(l, result);
		if(result == false)
			Lua::PushString(l, errMsg);
		else
			Lua::PushString(l, outputPath);
		return 2;
	}
	auto &imgBuf = Lua::Check<uimg::ImageBuffer>(l, 1);
	auto imgFormat = static_cast<pragma::asset::ModelExportInfo::ImageFormat>(Lua::CheckInt(l, 2));
	std::string outputPath = Lua::CheckString(l, 3);
	if(Lua::file::validate_write_operation(l, outputPath) == false) {
		Lua::PushBool(l, false);
		return 1;
	}
	auto normalMap = false;
	if(Lua::IsSet(l, 4))
		normalMap = Lua::CheckBool(l, 4);
	auto srgb = false;
	if(Lua::IsSet(l, 5))
		srgb = Lua::CheckBool(l, 5);
	auto alphaMode = uimg::TextureInfo::AlphaMode::Auto;
	if(Lua::IsSet(l, 6))
		alphaMode = static_cast<uimg::TextureInfo::AlphaMode>(Lua::CheckInt(l, 6));

	std::string errMsg;
	std::string finalOutputPath;
	auto result = pragma::asset::export_texture(imgBuf, imgFormat, outputPath, errMsg, normalMap, srgb, alphaMode, &finalOutputPath);
	Lua::PushBool(l, result);
	if(result == false)
		Lua::PushString(l, errMsg);
	else
		Lua::PushString(l, finalOutputPath);
	return 2;
}

int Lua::util::Client::export_material(lua_State *l)
{
	Material *mat = nullptr;
	if(Lua::IsString(l, 1)) {
		std::string matPath = Lua::CheckString(l, 1);
		mat = client->LoadMaterial(matPath, nullptr, true, false);
	}
	else
		mat = Lua::CheckMaterial(l, 1);

	if(mat == nullptr) {
		Lua::PushBool(l, false);
		Lua::PushString(l, "Invalid material");
		return 2;
	}
	auto imgFormat = static_cast<pragma::asset::ModelExportInfo::ImageFormat>(Lua::CheckInt(l, 2));
	auto normalizeTextureNames = false;
	if(Lua::IsSet(l, 3))
		normalizeTextureNames = Lua::CheckBool(l, 3);

	std::string errMsg;
	std::string finalOutputPath;
	auto textures = pragma::asset::export_material(*mat, imgFormat, errMsg, nullptr, normalizeTextureNames);
	Lua::PushBool(l, textures.has_value());
	if(textures.has_value() == false)
		Lua::PushString(l, errMsg);
	else {
		auto t = Lua::CreateTable(l);
		for(auto &pair : *textures) {
			Lua::PushString(l, pair.first);
			Lua::PushString(l, pair.second);
			Lua::SetTableValue(l, t);
		}
	}
	return 2;
}

std::string Lua::util::Client::get_clipboard_string() { return c_engine->GetWindow()->GetClipboardString(); }
void Lua::util::Client::set_clipboard_string(const std::string &str) { c_engine->GetWindow()->SetClipboardString(str); }

::util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> Lua::util::Client::bake_directional_lightmap_atlas(const std::vector<pragma::CLightComponent *> &lights, const std::vector<::ModelSubMesh *> &meshes, const std::vector<BaseEntity *> &entities, uint32_t width, uint32_t height,
  ::pragma::LightmapDataCache *optLightmapDataCache)
{
	return ::util::baking::bake_directional_lightmap_atlas(lights, meshes, entities, width, height, optLightmapDataCache);
}
