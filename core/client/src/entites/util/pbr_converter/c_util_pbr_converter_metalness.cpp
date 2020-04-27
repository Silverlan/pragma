/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/game/c_game.h"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "cmaterial.h"
#include <pragma/physics/collisionmesh.h>
#include <pragma/physics/transform.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void CPBRConverterComponent::UpdateMetalness(Model &mdl,CMaterial &mat)
{
	// Material has no surface material. To find out whether it is a metal material,
	// we'll try to find a collision mesh near the visual mesh that uses this material
	// and check its surface material instead. If its metal, we'll assume the visual material is metal as well.
	auto &mats = mdl.GetMaterials();
	auto itMat = std::find_if(mats.begin(),mats.end(),[&mat](const MaterialHandle &hMat) {
		return hMat.get() == &mat;
	});
	assert(itMat != mats.end());
	if(itMat == mats.end())
		return; // Material doesn't exist in model, this shouldn't happen
	auto matIdx = itMat -mats.begin();

	// Calculate center positions of model collision meshes.
	// We'll need these for the next step.
	auto colMeshes = mdl.GetCollisionMeshes();
	std::vector<Vector3> colMeshCenterPositions {};
	colMeshCenterPositions.reserve(colMeshes.size());
	for(auto &colMesh : colMeshes)
	{
		auto boneId = colMesh->GetBoneParent();
		auto tBone = mdl.CalcReferenceBonePose(boneId);
		pragma::physics::Transform t {};
		if(tBone.has_value())
			t = *tBone;

		Vector3 colMeshCenter {};
		auto &verts = colMesh->GetVertices();
		if(verts.empty() == false)
		{
			for(auto &v : verts)
				colMeshCenter += v;
			colMeshCenter /= static_cast<float>(verts.size());
		}

		colMeshCenter = t *colMeshCenter;
		colMeshCenterPositions.push_back(colMeshCenter);
	}

	// For each sub-mesh that uses the material:
	// Find the closest collision mesh
	// Then calculate the average metalness of all found collision meshes
	// via the collision mesh surface material
	std::vector<CollisionMesh*> matColMeshes {};
	std::vector<std::shared_ptr<ModelMesh>> lodMeshes {};
	std::vector<uint32_t> bodyGroups {};
	bodyGroups.resize(mdl.GetBodyGroupCount());
	mdl.GetBodyGroupMeshes(bodyGroups,0,lodMeshes);
	Vector3 meshCenter {};
	uint32_t vertexCount = 0;
	auto numSkins = mdl.GetTextureGroups().size();
	for(auto &mesh : lodMeshes)
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto isMeshUsingTexture = false;
			for(auto i=decltype(numSkins){0u};i<numSkins;++i)
			{
				auto texIdx = mdl.GetMaterialIndex(*subMesh,i);
				if(texIdx.has_value() == false || *texIdx != matIdx)
					continue;
				isMeshUsingTexture = true;
				break;
			}
			if(isMeshUsingTexture == false)
				continue;
			// Calculate center pos of the mesh
			auto &verts = subMesh->GetVertices();
			Vector3 center {};
			if(verts.empty() == false)
			{
				for(auto &v : verts)
					center += v.position;
				center /= static_cast<float>(verts.size());
			}

			// Find closest collision mesh
			auto dClosest = std::numeric_limits<float>::max();
			int32_t iClosest = -1;
			for(auto i=decltype(colMeshes.size()){0u};i<colMeshes.size();++i)
			{
				auto &colMesh = colMeshes.at(i);
				auto &colMeshCenter = colMeshCenterPositions.at(i);
				auto d = uvec::distance_sqr(center,colMeshCenter);
				if(d >= dClosest)
					continue;
				d = dClosest;
				iClosest = i;
			}
			if(iClosest == -1)
				continue;
			matColMeshes.push_back(colMeshes.at(iClosest).get());
			colMeshes.erase(colMeshes.begin() +iClosest);
			colMeshCenterPositions.erase(colMeshCenterPositions.begin() +iClosest);
		}
	}

	// matColMeshes now contains all collision meshes associated with the material.
	// Next, we need to determine the average metalness.
	uint32_t numSurfMats = 0u;
	auto accMetalness = 0.f;
	auto accRoughness = 0.f;
	auto accFlesh = 0.f;
	SurfaceMaterial *sufMatSSS = nullptr;
	for(auto *colMesh : matColMeshes)
	{
		auto surfMatIdx = colMesh->GetSurfaceMaterial();
		auto &surfMats = colMesh->GetSurfaceMaterials();
		if(surfMatIdx == -1 && surfMats.empty() == false)
			surfMatIdx = surfMats.front();
		auto *surfMat = c_game->GetSurfaceMaterial(surfMatIdx);
		if(surfMat == nullptr)
			continue;
		++numSurfMats;
		auto &pbrInfo = surfMat->GetPBRInfo();
		accMetalness += pbrInfo.metalness;
		accRoughness += pbrInfo.roughness;
		if(pbrInfo.subsurfaceMultiplier != 0.f && sufMatSSS == nullptr)
			sufMatSSS = surfMat; // We'll just take the first surface material that has SSS values instead of interpolating.
	}
	std::optional<float> metalness = (numSurfMats > 0) ? (accMetalness /static_cast<float>(numSurfMats)) : std::optional<float>{};
	std::optional<float> roughness = (numSurfMats > 0) ? (accRoughness /static_cast<float>(numSurfMats)) : std::optional<float>{};

	auto rmaInfo = mat.GetDataBlock()->GetBlock("rma_info");
	if(rmaInfo == nullptr)
		return;
	if(rmaInfo->GetBool("requires_roughness_update"))
	{
		if(roughness.has_value())
		{
			Con::cout<<"Assigning roughness value of "<<*roughness<<" to material '"<<mat.GetName()<<"', based on surface material properties of model '"<<mdl.GetName()<<"'!"<<Con::endl;
			mat.GetDataBlock()->AddValue("float","roughness_factor",std::to_string(*roughness));
		}
		rmaInfo->RemoveValue("requires_roughness_update");
	}
	if(rmaInfo->GetBool("requires_metalness_update"))
	{
		if(metalness.has_value())
		{
			Con::cout<<"Assigning metalness value of "<<*metalness<<" to material '"<<mat.GetName()<<"', based on surface material properties of model '"<<mdl.GetName()<<"'!"<<Con::endl;
			mat.GetDataBlock()->AddValue("float","metalness_factor",std::to_string(*metalness));
		}
		rmaInfo->RemoveValue("requires_metalness_update");
	}
	if(rmaInfo->GetBool("requires_sss_update"))
	{
		if(sufMatSSS)
		{
			auto &data = mat.GetDataBlock();
			auto &pbrInfo = sufMatSSS->GetPBRInfo();
			data->AddValue("float","subsurface_multiplier",std::to_string(pbrInfo.subsurfaceMultiplier));
			data->AddValue("color","subsurface_color",pbrInfo.subsurfaceColor.ToString());
			data->AddValue("int","subsurface_method",std::to_string(umath::to_integral(pbrInfo.subsurfaceMethod)));
			data->AddValue("vector","subsurface_radius",std::to_string(pbrInfo.subsurfaceRadius.x) +" " +std::to_string(pbrInfo.subsurfaceRadius.y) +" " +std::to_string(pbrInfo.subsurfaceRadius.z));
		}
		rmaInfo->RemoveValue("requires_sss_update");
	}
	auto resWatcherLock = c_engine->ScopeLockResourceWatchers();

	if(rmaInfo->IsEmpty())
		mat.GetDataBlock()->RemoveValue("rma_info");

	mat.UpdateTextures();
	if(mat.Save())
		client->LoadMaterial(mat.GetName(),true,true); // Reload material immediately
}

void CPBRConverterComponent::UpdateMetalness(Model &mdl)
{
	ConvertMaterialsToPBR(mdl);
	for(auto hMat : mdl.GetMaterials())
	{
		if(hMat.IsValid() == false)
			continue;
		auto &mat = static_cast<CMaterial&>(*hMat.get());
		// Make sure it's a PBR material
		if(IsPBR(mat) == false)
			continue;
		auto rmaInfo = mat.GetDataBlock()->GetBlock("rma_info");
		if(rmaInfo == nullptr || rmaInfo->GetBool("requires_metalness_update") == false)
			continue;
		UpdateMetalness(mdl,mat);
	}
}
