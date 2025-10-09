// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/transform.hpp"

#include "fsys/filesystem.h"

#include "udm.hpp"

#include "mathutil/umath.h"

#include "stdafx_client.h"
#include "sharedutils/util_hash.hpp"

module pragma.client;


import :rendering.lightmap_data_cache;
using namespace pragma;

size_t LightmapDataCache::CalcPoseHash(const umath::Transform &pose)
{
	auto ang = pose.GetAngles();
	auto &pos = pose.GetOrigin();
	auto floatToHash = [](float f) -> size_t {
		f *= 10.f; // Objects have to be at least 0.1 units away from each other, otherwise they cannot be distinguished in the cache
		auto n = static_cast<int64_t>(umath::round(f));
		return std::hash<int64_t> {}(n);
	};
	util::Hash hash = 0;
	hash = util::hash_combine<size_t>(hash, floatToHash(pos.x));
	hash = util::hash_combine<size_t>(hash, floatToHash(pos.y));
	hash = util::hash_combine<size_t>(hash, floatToHash(pos.z));
	hash = util::hash_combine<size_t>(hash, floatToHash(ang.p));
	hash = util::hash_combine<size_t>(hash, floatToHash(ang.y));
	hash = util::hash_combine<size_t>(hash, floatToHash(ang.r));
	return hash;
}

std::string LightmapDataCache::GetCacheFileName(const std::string &path)
{
	auto fpath = path;
	std::string ext;
	if(!ufile::get_extension(fpath, &ext) || (ext != FORMAT_MODEL_BINARY && ext != FORMAT_MODEL_ASCII)) {
		if(filemanager::exists(fpath + "." + std::string {FORMAT_MODEL_BINARY}))
			fpath += "." + std::string {FORMAT_MODEL_BINARY};
		else
			fpath += "." + std::string {FORMAT_MODEL_ASCII};
	}
	return fpath;
}

bool LightmapDataCache::Load(const std::string &path, LightmapDataCache &outCache, std::string &outErr)
{
	auto fpath = GetCacheFileName(path);
	std::shared_ptr<udm::Data> udmData = nullptr;
	try {
		udmData = udm::Data::Load(fpath);
	}
	catch(const udm::Exception &e) {
		outErr = "Failed to open input file '" + path + "': " + e.what() + "!";
		return false;
	}
	return outCache.LoadFromAssetData(udmData->GetAssetData(), outErr);
}

bool LightmapDataCache::SaveAs(const std::string &path, std::string &outErr) const
{
	auto udmData = udm::Data::Create(PLMD_IDENTIFIER, PLMD_VERSION);
	auto res = Save(udmData->GetAssetData(), outErr);
	if(res == false)
		return false;
	res = udmData->Save(path);
	if(!res) {
		outErr = "Failed to open output file '" + path + "'!";
		return false;
	}
	return true;
}

bool LightmapDataCache::Save(udm::AssetDataArg outData, std::string &outErr) const
{
	outData.SetAssetType(PLMD_IDENTIFIER);
	outData.SetAssetVersion(PLMD_VERSION);
	auto udm = *outData;
	udm["lightmapEntityId"] = util::uuid_to_string(lightmapEntityId);

	auto udmCacheData = udm["cacheData"];
	for(auto &pair : cacheData) {
		auto udmEnt = udmCacheData[util::uuid_to_string(pair.first.uuid)];
		udmEnt["model"] = pair.second.model;
		udmEnt["pose"] = pair.second.pose;

		auto udmMeshData = udmEnt["meshData"];
		for(auto &pairMesh : pair.second.meshData)
			udmMeshData[util::uuid_to_string(pairMesh.first.uuid)].AddArray("lightmapUvs", pairMesh.second.uvs, udm::ArrayType::Compressed);
	}
	return true;
}
bool LightmapDataCache::LoadFromAssetData(const udm::AssetData &data, std::string &outErr)
{
	if(data.GetAssetType() != PLMD_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	const auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}

	std::string uuid;
	if(udm["lightmapEntityId"](uuid))
		lightmapEntityId = util::uuid_string_to_bytes(uuid);

	auto udmCacheData = udm["cacheData"];
	cacheData.reserve(udmCacheData.GetChildCount());
	for(auto &pair : udmCacheData.ElIt()) {
		InstanceCacheData instanceData {};
		pair.property["model"](instanceData.model);
		pair.property["pose"](instanceData.pose);

		auto udmMeshData = pair.property["meshData"];
		instanceData.meshData.reserve(udmMeshData.GetChildCount());
		for(auto &pair : udmMeshData.ElIt()) {
			auto udmMesh = pair.property;
			MeshCacheData meshData {};
			udmMesh["lightmapUvs"](meshData.uvs);
			instanceData.meshData[LmUuid {util::uuid_string_to_bytes(std::string {pair.key})}] = std::move(meshData);
		}
		cacheData[LmUuid {util::uuid_string_to_bytes(std::string {pair.key})}] = std::move(instanceData);
	}
	return true;
}
const std::vector<Vector2> *LightmapDataCache::FindLightmapUvs(const util::Uuid &entUuid, const util::Uuid &meshUuid) const
{
	auto it = cacheData.find(LmUuid {entUuid});
	if(it == cacheData.end())
		return nullptr;
	auto &instanceCache = it->second.meshData;
	auto itInst = instanceCache.find(LmUuid {meshUuid});
	if(itInst == instanceCache.end())
		return nullptr;
	return &itInst->second.uvs;
}
void LightmapDataCache::AddInstanceData(const util::Uuid &entUuid, const std::string &model, const umath::Transform &pose, const util::Uuid &meshUuid, std::vector<Vector2> &&uvs)
{
	LmUuid lmEntUuid {entUuid};
	auto it = cacheData.find(lmEntUuid);
	if(it == cacheData.end()) {
		InstanceCacheData instanceData {};
		instanceData.entityUuid = entUuid;
		instanceData.model = model;
		instanceData.pose = pose;
		it = cacheData.insert(std::make_pair(lmEntUuid, std::move(instanceData))).first;
	}

	auto &instanceData = it->second;
	instanceData.meshData[LmUuid {meshUuid}] = {std::move(uvs)};
}
