// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :scripting.lua.libraries.util;
import :assets;
import :client_state;
import :engine;
import :entities.components.particle_system;
import :entities.components.render;
import :game;
import :util;

int Lua::util::Client::calc_world_direction_from_2d_coordinates(lua::State *l, pragma::CCameraComponent &hCam, const ::Vector2 &uv)
{
	auto trComponent = hCam.GetEntity().GetTransformComponent();
	auto forward = trComponent ? trComponent->GetForward() : uvec::PRM_FORWARD;
	auto right = trComponent ? trComponent->GetRight() : uvec::PRM_RIGHT;
	auto up = trComponent ? trComponent->GetUp() : uvec::PRM_UP;
	auto dir = uvec::calc_world_direction_from_2d_coordinates(forward, right, up, hCam.GetFOVRad(), hCam.GetNearZ(), hCam.GetFarZ(), hCam.GetAspectRatio(), 0.f, 0.f, uv);
	Lua::Push<Vector3>(l, dir);
	return 1;
}

int Lua::util::Client::create_particle_tracer(lua::State *l)
{
	auto &start = Lua::Check<Vector3>(l, 1);
	auto &end = Lua::Check<Vector3>(l, 2);
	auto radius = IsSet(l, 3) ? CheckNumber(l, 3) : pragma::game::bulletInfo::DEFAULT_TRACER_RADIUS;
	const auto &col = IsSet(l, 4) ? Lua::Check<::Color>(l, 4) : pragma::game::bulletInfo::DEFAULT_TRACER_COLOR;
	auto length = IsSet(l, 5) ? CheckNumber(l, 5) : pragma::game::bulletInfo::DEFAULT_TRACER_LENGTH;
	auto speed = IsSet(l, 6) ? CheckNumber(l, 6) : pragma::game::bulletInfo::DEFAULT_TRACER_SPEED;
	std::string mat = IsSet(l, 7) ? CheckString(l, 7) : std::string {pragma::game::bulletInfo::DEFAULT_TRACER_MATERIAL};
	auto bloomScale = IsSet(l, 8) ? CheckNumber(l, 8) : pragma::game::bulletInfo::DEFAULT_TRACER_BLOOM;

	auto *particle = pragma::get_cgame()->CreateParticleTracer<pragma::ecs::CParticleSystemComponent>(start, end, static_cast<float>(radius), col, static_cast<float>(length), static_cast<float>(speed), mat, static_cast<float>(bloomScale));
	if(particle == nullptr)
		return 0;
	particle->PushLuaObject(l);
	return 1;
}

int Lua::util::Client::create_muzzle_flash(lua::State *l)
{
	if(Lua::IsType<pragma::ecs::BaseEntity>(l, 1) == true) {
		auto &ent = Lua::Check<pragma::ecs::BaseEntity>(l, 1);
		int32_t attId = -1;
		std::string att {};
		if(IsNumber(l, 2))
			attId = CheckInt(l, 2);
		else
			att = CheckString(l, 2);
		Vector3 relOffset {};
		auto relRot = uquat::identity();
		if(IsSet(l, 3)) {
			relOffset = Lua::Check<Vector3>(l, 3);
			if(IsSet(l, 4))
				relRot = Lua::Check<Quat>(l, 4);
		}
		std::string particleName = "muzzleflash0" + std::to_string(pragma::math::random(1, 6));
		auto *pt = pragma::ecs::CParticleSystemComponent::Create(particleName);
		if(pt == nullptr)
			return 0;
		auto pRenderComponent = static_cast<pragma::ecs::CBaseEntity *>(&ent)->GetRenderComponent();
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
			attInfo.flags |= pragma::FAttachmentMode::UpdateEachFrame;
			if(att.empty() == false)
				pAttachableComponent->AttachToAttachment(&ent, att, attInfo);
			else
				pAttachableComponent->AttachToAttachment(&ent, attId, attInfo);
		}
		pt->PushLuaObject(l);
		return 1;
	}
	auto &pos = Lua::Check<Vector3>(l, 1);
	auto &rot = Lua::Check<Quat>(l, 2);
	std::string particleName = "muzzleflash0" + std::to_string(pragma::math::random(1, 6));
	auto *pt = pragma::ecs::CParticleSystemComponent::Create(particleName);
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
	pragma::ecs::CParticleSystemComponent *particle = nullptr;
	pragma::get_cgame()->CreateGiblet(createInfo, &particle);
	if(particle == nullptr)
		return {};
	return particle->GetLuaObject();
}

int Lua::util::Client::import_gltf(lua::State *l)
{
	std::shared_ptr<ufile::IFile> f = nullptr;
	std::string fileName;
	if(IsString(l, 1))
		fileName = CheckString(l, 1);
	else {
		auto &lf = Lua::Check<LFile>(l, 1);
		f = lf.GetHandle();
	}
	pragma::util::Path outputPath {};
	if(IsSet(l, 2))
		outputPath = pragma::util::Path::CreatePath(CheckString(l, 2));
	auto importAsSingleModel = false;
	if(IsSet(l, 3))
		importAsSingleModel = CheckBool(l, 3);
	std::string errMsg;
	std::optional<pragma::asset::AssetImportResult> importInfo {};
	if(f) {
		importInfo = pragma::asset::import_gltf(*f, errMsg, outputPath, importAsSingleModel);
	}
	else
		importInfo = pragma::asset::import_gltf(fileName, errMsg, outputPath, importAsSingleModel);
	if(!importInfo) {
		PushBool(l, false);
		PushString(l, errMsg);
		return 2;
	}
	auto t = luabind::newtable(l);
	t["mapName"] = importInfo->mapName;
	t["models"] = importInfo->models;
	t.push(l);
	return 1;
}

int Lua::util::Client::import_model(lua::State *l)
{
	std::shared_ptr<ufile::IFile> f = nullptr;
	std::string fileName;
	if(IsString(l, 1))
		fileName = CheckString(l, 1);
	else {
		auto &lf = Lua::Check<LFile>(l, 1);
		f = lf.GetHandle();
	}
	pragma::util::Path outputPath {};
	if(IsSet(l, 2))
		outputPath = pragma::util::Path::CreatePath(CheckString(l, 2));
	auto importAsSingleModel = true;
	if(IsSet(l, 3))
		importAsSingleModel = CheckBool(l, 3);
	std::string errMsg;
	std::shared_ptr<pragma::asset::Model> mdl = nullptr;
	if(f) {
		mdl = pragma::asset::import_model(*f, errMsg, outputPath, importAsSingleModel);
	}
	else
		mdl = pragma::asset::import_model(fileName, errMsg, outputPath, importAsSingleModel);
	if(mdl == nullptr) {
		PushBool(l, false);
		PushString(l, errMsg);
		return 2;
	}
	Push(l, mdl);
	return 1;
}

int Lua::util::Client::export_map(lua::State *l)
{
	std::string mapName = CheckString(l, 1);
	auto &exportInfo = Lua::Check<pragma::asset::ModelExportInfo>(l, 2);

	pragma::asset::MapExportInfo mapExportInfo {};
	if(IsSet(l, 3))
		mapExportInfo = Lua::Check<pragma::asset::MapExportInfo>(l, 3);

	std::string errMsg;
	auto result = pragma::asset::export_map(mapName, exportInfo, errMsg, mapExportInfo);
	PushBool(l, result);
	if(result)
		return 1;
	PushString(l, errMsg);
	return 2;
}

int Lua::util::Client::export_texture(lua::State *l)
{
	if(IsString(l, 1)) {
		std::string texturePath = CheckString(l, 1);
		auto imgFormat = static_cast<pragma::asset::ModelExportInfo::ImageFormat>(CheckInt(l, 2));
		auto alphaMode = pragma::image::TextureInfo::AlphaMode::Auto;
		if(IsSet(l, 3))
			alphaMode = static_cast<pragma::image::TextureInfo::AlphaMode>(CheckInt(l, 3));
		auto enabledExtendedDDs = false;
		if(IsSet(l, 4))
			enabledExtendedDDs = CheckBool(l, 4);

		std::string errMsg;
		std::string outputPath;
		auto result = pragma::asset::export_texture(texturePath, imgFormat, errMsg, alphaMode, enabledExtendedDDs, nullptr, &outputPath);
		PushBool(l, result);
		if(result == false)
			PushString(l, errMsg);
		else
			PushString(l, outputPath);
		return 2;
	}
	auto &imgBuf = Lua::Check<pragma::image::ImageBuffer>(l, 1);
	auto imgFormat = static_cast<pragma::asset::ModelExportInfo::ImageFormat>(CheckInt(l, 2));
	std::string outputPath = CheckString(l, 3);
	if(file::validate_write_operation(l, outputPath) == false) {
		PushBool(l, false);
		return 1;
	}
	auto normalMap = false;
	if(IsSet(l, 4))
		normalMap = CheckBool(l, 4);
	auto srgb = false;
	if(IsSet(l, 5))
		srgb = CheckBool(l, 5);
	auto alphaMode = pragma::image::TextureInfo::AlphaMode::Auto;
	if(IsSet(l, 6))
		alphaMode = static_cast<pragma::image::TextureInfo::AlphaMode>(CheckInt(l, 6));

	std::string errMsg;
	std::string finalOutputPath;
	auto result = pragma::asset::export_texture(imgBuf, imgFormat, outputPath, errMsg, normalMap, srgb, alphaMode, &finalOutputPath);
	PushBool(l, result);
	if(result == false)
		PushString(l, errMsg);
	else
		PushString(l, finalOutputPath);
	return 2;
}

int Lua::util::Client::export_material(lua::State *l)
{
	pragma::material::Material *mat = nullptr;
	if(IsString(l, 1)) {
		std::string matPath = CheckString(l, 1);
		mat = pragma::get_client_state()->LoadMaterial(matPath, nullptr, true, false);
	}
	else
		mat = &Lua::Check<pragma::material::Material>(l, 1);

	if(mat == nullptr) {
		PushBool(l, false);
		PushString(l, "Invalid material");
		return 2;
	}
	auto imgFormat = static_cast<pragma::asset::ModelExportInfo::ImageFormat>(CheckInt(l, 2));
	auto normalizeTextureNames = false;
	if(IsSet(l, 3))
		normalizeTextureNames = CheckBool(l, 3);

	std::string errMsg;
	std::string finalOutputPath;
	auto textures = pragma::asset::export_material(*mat, imgFormat, errMsg, nullptr, normalizeTextureNames);
	PushBool(l, textures.has_value());
	if(textures.has_value() == false)
		PushString(l, errMsg);
	else {
		auto t = CreateTable(l);
		for(auto &pair : *textures) {
			PushString(l, pair.first);
			PushString(l, pair.second);
			SetTableValue(l, t);
		}
	}
	return 2;
}

std::string Lua::util::Client::get_clipboard_string() { return pragma::get_cengine()->GetWindow()->GetClipboardString(); }
void Lua::util::Client::set_clipboard_string(const std::string &str) { pragma::get_cengine()->GetWindow()->SetClipboardString(str); }

pragma::util::ParallelJob<std::shared_ptr<pragma::image::ImageBuffer>> Lua::util::Client::bake_directional_lightmap_atlas(const std::vector<pragma::CLightComponent *> &lights, const std::vector<pragma::geometry::ModelSubMesh *> &meshes,
  const std::vector<pragma::ecs::BaseEntity *> &entities, uint32_t width, uint32_t height, pragma::rendering::LightmapDataCache *optLightmapDataCache)
{
	return pragma::util::baking::bake_directional_lightmap_atlas(lights, meshes, entities, width, height, optLightmapDataCache);
}
