/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/lightmap_data_cache.hpp"

using namespace pragma;
#pragma optimize("",off)
size_t LightmapDataCache::CalcPoseHash(const umath::Transform &pose)
{
	auto ang = pose.GetAngles();
	auto &pos = pose.GetOrigin();
	auto floatToHash = [](float f) -> size_t {
		f *= 10.f; // Objects have to be at least 0.1 units away from each other, otherwise they cannot be distinguished in the cache
		auto n = static_cast<int64_t>(umath::round(f));
		return std::hash<int64_t>{}(n);
	};
	util::Hash hash = 0;
	hash = util::hash_combine<size_t>(hash,floatToHash(pos.x));
	hash = util::hash_combine<size_t>(hash,floatToHash(pos.y));
	hash = util::hash_combine<size_t>(hash,floatToHash(pos.z));
	hash = util::hash_combine<size_t>(hash,floatToHash(ang.p));
	hash = util::hash_combine<size_t>(hash,floatToHash(ang.y));
	hash = util::hash_combine<size_t>(hash,floatToHash(ang.r));
	return hash;
}

bool LightmapDataCache::Load(const std::string &path,LightmapDataCache &outCache,std::string &outErr)
{
	auto fpath = path;
	std::string ext;
	if(!ufile::get_extension(fpath,&ext) || (ext != "lmd_b" && ext != "lmd"))
	{
		if(filemanager::exists(fpath +".lmd_b"))
			fpath += ".lmd_b";
		else
			fpath += ".lmd";
	}
	auto udmData = udm::Data::Load(fpath);
	if(!udmData)
	{
		outErr = "Failed to open input file '" +path +"'!";
		return false;
	}
	return outCache.LoadFromAssetData(udmData->GetAssetData(),outErr);
}

bool LightmapDataCache::SaveAs(const std::string &path,std::string &outErr) const
{
	auto udmData = udm::Data::Create(PLMD_IDENTIFIER,PLMD_VERSION);
	auto res = Save(udmData->GetAssetData(),outErr);
	if(res == false)
		return false;
	res = udmData->Save(path);
	if(!res)
	{
		outErr = "Failed to open output file '" +path +"'!";
		return false;
	}
	return true;
}

bool LightmapDataCache::Save(udm::AssetDataArg outData,std::string &outErr) const
{
	outData.SetAssetType(PLMD_IDENTIFIER);
	outData.SetAssetVersion(PLMD_VERSION);
	auto udm = *outData;

	auto udmCacheData = udm["cacheData"];
	for(auto &pair : cacheData)
	{
		for(auto &meshPair : pair.second.meshCacheData)
		{
			auto udmMeshCache = udmCacheData[util::uuid_to_string(meshPair.first.uuid)];
			udmMeshCache["model"] = meshPair.second.model;
			auto udmInstances = udmMeshCache["instances"];
			for(auto &instancePair : meshPair.second.instanceCacheData)
			{
				auto udmInstance = udmInstances[std::to_string(instancePair.first)];
				udmInstance["pose"] = instancePair.second.pose;
				udmInstance.AddArray("lightmapUvs",instancePair.second.uvs,udm::ArrayType::Compressed);
			}
		}
	}
	return true;
}
bool LightmapDataCache::LoadFromAssetData(const udm::AssetData &data,std::string &outErr)
{
	if(data.GetAssetType() != PLMD_IDENTIFIER)
	{
		outErr = "Incorrect format!";
		return false;
	}

	const auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1)
	{
		outErr = "Invalid version!";
		return false;
	}
	
	auto udmCacheData = udm["cacheData"];
	for(auto &pair : udmCacheData.ElIt())
	{
		auto uuid = util::uuid_string_to_bytes(std::string{pair.key});
		std::string model;
		pair.property["model"](model);
		auto it = cacheData.find(model);
		if(it == cacheData.end())
			it = cacheData.insert(std::make_pair(model,ModelCacheData{})).first;
		
		auto &mdlCacheData = it->second;
		auto itMesh = mdlCacheData.meshCacheData.find(LmUuid{uuid});
		if(itMesh == mdlCacheData.meshCacheData.end())
		{
			itMesh = mdlCacheData.meshCacheData.insert(std::make_pair(uuid,MeshCacheData{})).first;
			itMesh->second.model = model;
		}

		auto &instanceCacheDataMap = itMesh->second.instanceCacheData;
		auto udmInstances = pair.property["instances"];
		for(auto &udmInstPair : udmInstances.ElIt())
		{
			auto poseHash = util::to_int<int64_t>(udmInstPair.key);
			auto itInstance = instanceCacheDataMap.find(poseHash);
			if(itInstance == instanceCacheDataMap.end())
				itInstance = instanceCacheDataMap.insert(std::make_pair(poseHash,InstanceCacheData{})).first;
			auto &instanceCacheData = itInstance->second;
			auto &udmInst = udmInstPair.property;
			udmInst["pose"](instanceCacheData.pose);
			udmInst["lightmapUvs"](instanceCacheData.uvs);
		}
	}
}
const std::vector<Vector2> *LightmapDataCache::FindLightmapUvs(const std::string &model,const umath::Transform &pose,const util::Uuid &meshUuid) const
{
	auto it = cacheData.find(model);
	if(it == cacheData.end())
		return nullptr;
	auto &meshCacheData = it->second.meshCacheData;
	auto itMeshCache = meshCacheData.find(LmUuid{meshUuid});
	if(itMeshCache == meshCacheData.end())
		return nullptr;
	auto &meshCache = itMeshCache->second.instanceCacheData;
	auto itInst = meshCache.find(CalcPoseHash(pose));
	if(itInst == meshCache.end())
		return nullptr;
	return &itInst->second.uvs;
}
void LightmapDataCache::AddInstanceData(const std::string &model,const umath::Transform &pose,const util::Uuid &meshUuid,std::vector<Vector2> &&uvs)
{
	auto it = cacheData.find(model);
	if(it == cacheData.end())
		it = cacheData.insert(std::make_pair(model,ModelCacheData{})).first;
	auto itMesh = it->second.meshCacheData.find(LmUuid{meshUuid});
	if(itMesh == it->second.meshCacheData.end())
	{
		itMesh = it->second.meshCacheData.insert(std::make_pair(meshUuid,MeshCacheData{})).first;
		itMesh->second.model = model;
	}
	auto poseHash = CalcPoseHash(pose);
	auto itInstance = itMesh->second.instanceCacheData.find(poseHash);
	InstanceCacheData instanceCacheData {};
	instanceCacheData.pose = pose;
	instanceCacheData.uvs = std::move(uvs);
	itMesh->second.instanceCacheData[poseHash] = std::move(instanceCacheData);
}
#pragma optimize("",on)
