// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.util_pbr_converter;
import :client_state;
import :engine;
import :game;

using namespace pragma;

void CPBRConverterComponent::UpdateMetalness(asset::Model &mdl, material::CMaterial &mat)
{
	// Material has no surface material. To find out whether it is a metal material,
	// we'll try to find a collision mesh near the visual mesh that uses this material
	// and check its surface material instead. If its metal, we'll assume the visual material is metal as well.
	auto &mats = mdl.GetMaterials();
	auto itMat = std::find_if(mats.begin(), mats.end(), [&mat](const material::MaterialHandle &hMat) { return hMat.get() == &mat; });
	assert(itMat != mats.end());
	if(itMat == mats.end())
		return; // Material doesn't exist in model, this shouldn't happen
	auto matIdx = itMat - mats.begin();

	// Calculate center positions of model collision meshes.
	// We'll need these for the next step.
	auto colMeshes = mdl.GetCollisionMeshes();
	std::vector<Vector3> colMeshCenterPositions {};
	colMeshCenterPositions.reserve(colMeshes.size());
	for(auto &colMesh : colMeshes) {
		auto boneId = colMesh->GetBoneParent();
		auto tBone = mdl.CalcReferenceBonePose(boneId);
		math::Transform t {};
		if(tBone.has_value())
			t = *tBone;

		Vector3 colMeshCenter {};
		auto &verts = colMesh->GetVertices();
		if(verts.empty() == false) {
			for(auto &v : verts)
				colMeshCenter += v;
			colMeshCenter /= static_cast<float>(verts.size());
		}

		colMeshCenter = t * colMeshCenter;
		colMeshCenterPositions.push_back(colMeshCenter);
	}

	// For each sub-mesh that uses the material:
	// Find the closest collision mesh
	// Then calculate the average metalness of all found collision meshes
	// via the collision mesh surface material
	std::vector<physics::CollisionMesh *> matColMeshes {};
	std::vector<std::shared_ptr<geometry::ModelMesh>> lodMeshes {};
	std::vector<uint32_t> bodyGroups {};
	bodyGroups.resize(mdl.GetBodyGroupCount());
	mdl.GetBodyGroupMeshes(bodyGroups, 0, lodMeshes);
	Vector3 meshCenter {};
	uint32_t vertexCount = 0;
	auto numSkins = mdl.GetTextureGroups().size();
	for(auto &mesh : lodMeshes) {
		for(auto &subMesh : mesh->GetSubMeshes()) {
			auto isMeshUsingTexture = false;
			for(auto i = decltype(numSkins) {0u}; i < numSkins; ++i) {
				auto texIdx = mdl.GetMaterialIndex(*subMesh, i);
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
			if(verts.empty() == false) {
				for(auto &v : verts)
					center += v.position;
				center /= static_cast<float>(verts.size());
			}

			// Find closest collision mesh
			auto dClosest = std::numeric_limits<float>::max();
			int32_t iClosest = -1;
			for(auto i = decltype(colMeshes.size()) {0u}; i < colMeshes.size(); ++i) {
				auto &colMesh = colMeshes.at(i);
				auto &colMeshCenter = colMeshCenterPositions.at(i);
				auto d = uvec::distance_sqr(center, colMeshCenter);
				if(d >= dClosest)
					continue;
				d = dClosest;
				iClosest = i;
			}
			if(iClosest == -1)
				continue;
			matColMeshes.push_back(colMeshes.at(iClosest).get());
			colMeshes.erase(colMeshes.begin() + iClosest);
			colMeshCenterPositions.erase(colMeshCenterPositions.begin() + iClosest);
		}
	}

	// matColMeshes now contains all collision meshes associated with the material.
	// Next, we need to determine the average metalness.
	uint32_t numSurfMats = 0u;
	auto accMetalness = 0.f;
	auto accRoughness = 0.f;
	auto accFlesh = 0.f;
	auto accGlass = 0.f;
	physics::SurfaceMaterial *surfMatGlass = nullptr;
	physics::SurfaceMaterial *sufMatSSS = nullptr;
	for(auto *colMesh : matColMeshes) {
		auto surfMatIdx = colMesh->GetSurfaceMaterial();
		auto &surfMats = colMesh->GetSurfaceMaterials();
		if(surfMatIdx == -1 && surfMats.empty() == false)
			surfMatIdx = surfMats.front();
		auto *surfMat = get_cgame()->GetSurfaceMaterial(surfMatIdx);
		if(surfMat == nullptr)
			continue;
		++numSurfMats;
		auto &pbrInfo = surfMat->GetPBRInfo();
		accMetalness += pbrInfo.metalness;
		accRoughness += pbrInfo.roughness;
		if(surfMat->GetIdentifier().find("glass") != std::string::npos) {
			if(surfMatGlass == nullptr)
				surfMatGlass = surfMat;
			accGlass += 1.f;
		}
		if(pbrInfo.subsurface.factor != 0.f && sufMatSSS == nullptr)
			sufMatSSS = surfMat; // We'll just take the first surface material that has SSS values instead of interpolating.
	}
	std::optional<float> metalness = (numSurfMats > 0) ? (accMetalness / static_cast<float>(numSurfMats)) : std::optional<float> {};
	std::optional<float> roughness = (numSurfMats > 0) ? (accRoughness / static_cast<float>(numSurfMats)) : std::optional<float> {};
	accGlass = (numSurfMats > 0) ? (accGlass / static_cast<float>(numSurfMats)) : 0.f;

	if(!mat.HasPropertyBlock("rma_info"))
		return;
	if(mat.GetProperty("rma_info/requires_roughness_update", false)) {
		if(roughness.has_value()) {
			Con::COUT << "Assigning roughness value of " << *roughness << " to material '" << mat.GetName() << "', based on surface material properties of model '" << mdl.GetName() << "'!" << Con::endl;
			mat.SetProperty("roughness_factor", *roughness);
		}
		mat.ClearProperty("rma_info/requires_roughness_update");
	}
	if(mat.GetProperty("rma_info/requires_metalness_update", false)) {
		if(metalness.has_value()) {
			Con::COUT << "Assigning metalness value of " << *metalness << " to material '" << mat.GetName() << "', based on surface material properties of model '" << mdl.GetName() << "'!" << Con::endl;
			mat.SetProperty("metalness_factor", *metalness);
		}
		mat.ClearProperty("rma_info/requires_metalness_update");
	}

	if(mat.GetProperty("rma_info/requires_sss_update", false)) {
		if(sufMatSSS) {
			auto &pbrInfo = sufMatSSS->GetPBRInfo();
			mat.SetProperty("subsurface_scattering/factor", pbrInfo.subsurface.factor);
			mat.SetProperty("subsurface_scattering/method", 0);
			mat.SetProperty("subsurface_scattering/scatter_color", pbrInfo.subsurface.scatterColor);
		}
		mat.ClearProperty("rma_info/requires_sss_update");
	}

	if(accGlass > 0.5f)
		ApplyMiscMaterialProperties(mat, *surfMatGlass, "glass");

	auto resWatcherLock = get_cengine()->ScopeLockResourceWatchers();

	mat.UpdateTextures();
	std::string err;
	if(mat.Save(err))
		get_client_state()->LoadMaterial(mat.GetName(), nullptr, true, true); // Reload material immediately
}

void CPBRConverterComponent::UpdateMetalness(asset::Model &mdl)
{
	ConvertMaterialsToPBR(mdl);
	for(auto hMat : mdl.GetMaterials()) {
		if(!hMat)
			continue;
		auto &mat = static_cast<material::CMaterial &>(*hMat.get());
		// Make sure it's a PBR material
		if(IsPBR(mat) == false)
			continue;
		if(mat.GetProperty("rma_info/requires_metalness_update", false) == false)
			continue;
		UpdateMetalness(mdl, mat);
	}
}
