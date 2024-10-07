/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_bsp.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/components/c_bsp_leaf_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/c_model.h"
#include <pragma/math/intersection.h>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

OcclusionCullingHandlerBSP::OcclusionCullingHandlerBSP(const std::shared_ptr<util::BSPTree> &bspTree) : m_bspTree {bspTree} {}
const util::BSPTree &OcclusionCullingHandlerBSP::GetBSPTree() const { return *m_bspTree; }
void OcclusionCullingHandlerBSP::Update(const Vector3 &camPos)
{
	if(m_bLockCurrentNode)
		return;
	m_pCurrentNode = FindLeafNode(camPos);
}
bool OcclusionCullingHandlerBSP::ShouldExamine(CModelMesh &mesh, const Vector3 &pos, bool bViewModel, std::size_t numMeshes, const std::vector<umath::Plane> *optPlanes) const
{
	return ShouldPass(mesh, pos) && OcclusionCullingHandlerOctTree::ShouldExamine(mesh, pos, bViewModel, numMeshes, optPlanes);
}
bool OcclusionCullingHandlerBSP::ShouldExamine(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, CBaseEntity &cent, bool &outViewModel, std::vector<umath::Plane> **outPlanes) const
{
	return ShouldPass(cent) && OcclusionCullingHandlerOctTree::ShouldExamine(scene, renderer, cent, outViewModel, outPlanes);
}
bool OcclusionCullingHandlerBSP::ShouldPass(CBaseEntity &ent) const
{
	auto pRenderComponent = ent.GetRenderComponent();
	if(m_pCurrentNode == nullptr || !pRenderComponent)
		return false;
	if(pRenderComponent->IsExemptFromOcclusionCulling())
		return true;
	auto pBspLeafComponent = ent.GetComponent<CBSPLeafComponent>();
	//if(pBspLeafComponent.valid())
	//return false;//pBspLeafComponent->GetLeafVisibility(m_pCurrentNode->cluster); // TODO
	auto &aabb = ent.GetAbsoluteRenderBounds();
	return umath::intersection::aabb_aabb(aabb.min, aabb.max, m_pCurrentNode->minVisible, m_pCurrentNode->maxVisible) != umath::intersection::Intersect::Outside;
}
bool OcclusionCullingHandlerBSP::ShouldPass(CModelMesh &modelMesh, const Vector3 &entityPos) const
{
	auto &bspNodes = m_bspTree->GetNodes();
	auto clusterIndex = modelMesh.GetReferenceId();
	if(clusterIndex == std::numeric_limits<uint32_t>::max()) {
		// Probably not a world mesh
		return true; // TODO: Do manual culling by AABB? (calculate AABB for each cluster around all visible clusters)
	}
	return m_bspTree->IsClusterVisible(m_pCurrentNode->cluster, clusterIndex);

	/*Vector3 min,max;
	modelMesh.GetBounds(min,max);
	min += entityPos;
	max += entityPos;
	return umath::intersection::aabb_aabb(min,max,m_pCurrentNode->min,m_pCurrentNode->max);*/
}
bool OcclusionCullingHandlerBSP::ShouldPass(CModelSubMesh &subMesh, const Vector3 &entityPos) const
{
	Vector3 min, max;
	subMesh.GetBounds(min, max);
	min += entityPos;
	max += entityPos;
	return umath::intersection::aabb_aabb(min, max, m_pCurrentNode->minVisible, m_pCurrentNode->maxVisible) != umath::intersection::Intersect::Outside;
}
const util::BSPTree::Node *OcclusionCullingHandlerBSP::FindLeafNode(const Vector3 &point) const { return m_bspTree->FindLeafNode(point); }
const util::BSPTree::Node *OcclusionCullingHandlerBSP::GetCurrentNode() const { return m_pCurrentNode; }
void OcclusionCullingHandlerBSP::PerformCulling(pragma::CSceneComponent &scene, const CRasterizationRendererComponent &renderer, const Vector3 &camPos, std::vector<OcclusionMeshInfo> &culledMeshesOut, bool cullByViewFrustum)
{
	Update(camPos);
	return OcclusionCullingHandlerOctTree::PerformCulling(scene, renderer, camPos, culledMeshesOut, cullByViewFrustum);
}
void OcclusionCullingHandlerBSP::SetCurrentNodeLocked(bool bLocked) { m_bLockCurrentNode = bLocked; }
bool OcclusionCullingHandlerBSP::IsCurrentNodeLocked() const { return m_bLockCurrentNode; }

static void debug_bsp_nodes(NetworkState *, const ConVar &, int32_t, int32_t val)
{
	if(c_game == nullptr)
		return;
	auto *scene = c_game->GetScene();
	auto *world = c_game->GetWorld();
	auto bspTree = world ? static_cast<CWorldComponent *>(world)->GetBSPTree() : nullptr;
	if(bspTree == nullptr) {
		Con::cwar << "Scene does not have BSP tree!" << Con::endl;
		return;
	}
	auto *cam = c_game->GetRenderCamera();
	auto &camPos = cam ? cam->GetEntity().GetPosition() : uvec::ORIGIN;
	auto *pCurrentNode = bspTree->FindLeafNode(camPos);
	if(pCurrentNode == nullptr) {
		Con::cwar << "Camera not located in any leaf node!" << Con::endl;
		return;
	}
	auto &clusterVisibility = bspTree->GetClusterVisibility();
	Con::cout << "Camera position: (" << camPos.x << " " << camPos.y << " " << camPos.z << ")" << Con::endl;
	Con::cout << "Leaf cluster id: " << pCurrentNode->cluster << Con::endl;
	Con::cout << "Leaf bounds: (" << pCurrentNode->min.x << "," << pCurrentNode->min.y << "," << pCurrentNode->min.z << ") (" << pCurrentNode->max.x << "," << pCurrentNode->max.y << "," << pCurrentNode->max.z << ")" << Con::endl;
	Con::cout << "Absolute leaf bounds: (" << pCurrentNode->minVisible.x << "," << pCurrentNode->minVisible.y << "," << pCurrentNode->minVisible.z << ") (" << pCurrentNode->maxVisible.x << "," << pCurrentNode->maxVisible.y << "," << pCurrentNode->maxVisible.z << ")" << Con::endl;
	if(pCurrentNode->cluster >= clusterVisibility.size() || pCurrentNode->cluster == std::numeric_limits<uint16_t>::max())
		Con::cwar << "Invalid cluster id " << pCurrentNode->cluster << "!" << Con::endl;
	else {
		auto numClusters = bspTree->GetClusterCount();
		std::vector<std::vector<uint8_t>> decompressedClusters(numClusters, std::vector<uint8_t>(numClusters, 0u));
		auto cluster0 = 0ull;
		auto cluster1 = 0ull;
		for(auto i = decltype(clusterVisibility.size()) {0u}; i < clusterVisibility.size(); ++i) {
			auto vis = clusterVisibility.at(i);
			for(auto j = 0u; j < 8u; ++j) {
				if(vis & (1 << j))
					decompressedClusters.at(cluster0).at(cluster1) = 1u;
				if(++cluster1 == numClusters) {
					++cluster0;
					cluster1 = 0ull;
				}
			}
		}

		auto &visArray = decompressedClusters.at(pCurrentNode->cluster);
		std::unordered_set<size_t> visClusters {};
		std::vector<size_t> clusterIdToVisClusterId(visArray.size(), std::numeric_limits<size_t>::max());
		for(auto i = decltype(visArray.size()) {0u}; i < visArray.size(); ++i) {
			auto visibility = visArray.at(i);
			if(visibility == 0u)
				continue;
			clusterIdToVisClusterId.at(i) = visClusters.size();
			visClusters.insert(i);
		}
		Con::cout << "Cluster bounds: (" << pCurrentNode->min.x << " " << pCurrentNode->min.y << " " << pCurrentNode->min.z << ") (" << pCurrentNode->max.x << " " << pCurrentNode->max.y << " " << pCurrentNode->max.z << ")" << Con::endl;
		Con::cout << "Number of clusters visible from this cluster: " << visClusters.size() << Con::endl;

		static std::vector<std::shared_ptr<DebugRenderer::BaseObject>> dbgObjects {};
		dbgObjects.clear();
		auto *pWorld = c_game->GetWorld();
		if(pWorld == nullptr)
			Con::cwar << "No world entity found!" << Con::endl;
		else {
			auto &entWorld = pWorld->GetEntity();
			auto mdlComponent = entWorld.GetModelComponent();
			auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
			auto meshGroup = (mdl != nullptr) ? mdl->GetMeshGroup(0u) : nullptr;
			if(meshGroup != nullptr) {
				auto &meshes = meshGroup->GetMeshes();
				auto numTotalMeshes = meshes.size();
				auto numTotalIndices = 0ull;
				auto numTotalTris = 0ull;
				for(auto &mesh : meshes) {
					numTotalIndices += mesh->GetIndexCount();
					numTotalTris += mesh->GetTriangleCount();
				}

				const auto fFindVisInfo = [&meshes, pCurrentNode](const std::unordered_set<size_t> &visClusters) {
					auto numVerts = 0ull;
					auto numTris = 0ull;
					auto numMeshes = std::count_if(meshes.begin(), meshes.end(), [pCurrentNode, &numVerts, &numTris, &visClusters](const std::shared_ptr<ModelMesh> &mesh) {
						auto it = visClusters.find(mesh->GetReferenceId());
						if(it == visClusters.end())
							return false;
						numVerts += mesh->GetIndexCount();
						numTris += mesh->GetTriangleCount();
						return true;
					});
					return std::array<uint64_t, 3> {static_cast<uint64_t>(numMeshes), numVerts, numTris};
				};
				const auto fGetPercentage = [](uint32_t val, uint32_t total) -> std::string {
					if(total == 0u)
						return "0%";
					return std::to_string(umath::round(val / static_cast<float>(total) * 100.f, 2)) + "%";
				};
				auto visInfo = fFindVisInfo({pCurrentNode->cluster});
				Con::cout << "Number of meshes inside this cluster: " << visInfo.at(0) << " / " << numTotalMeshes << " (" << fGetPercentage(visInfo.at(0), numTotalMeshes) << ")" << Con::endl;
				//Con::cout<<"Number of vertices inside this cluster: "<<visInfo.at(1)<<" / "<<numTotalVerts<<" ("<<fGetPercentage(visInfo.at(1),numTotalVerts)<<")"<<Con::endl;
				Con::cout << "Number of triangles inside this cluster: " << visInfo.at(2) << " / " << numTotalTris << " (" << fGetPercentage(visInfo.at(2), numTotalTris) << ")" << Con::endl;
				Con::cout << Con::endl;

				visInfo = fFindVisInfo(visClusters);
				Con::cout << "Number of meshes visible from this cluster: " << visInfo.at(0) << " / " << numTotalMeshes << " (" << fGetPercentage(visInfo.at(0), numTotalMeshes) << ")" << Con::endl;
				//Con::cout<<"Number of vertices visible from this cluster: "<<visInfo.at(1)<<" / "<<numTotalVerts<<" ("<<fGetPercentage(visInfo.at(1),numTotalVerts)<<")"<<Con::endl;
				Con::cout << "Number of triangles visible from this cluster: " << visInfo.at(2) << " / " << numTotalTris << " (" << fGetPercentage(visInfo.at(2), numTotalTris) << ")" << Con::endl;

				if(val == 0) {
					auto &posWorld = entWorld.GetPosition();
					auto pTrComponent = entWorld.GetTransformComponent();
					auto rotWorld = pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
					auto clusterCenter = (pCurrentNode->min + pCurrentNode->max) * 0.5f;
					for(auto &mesh : meshes) {
						auto it = visClusters.find(mesh->GetReferenceId());
						if(it == visClusters.end())
							continue;
						auto pos = mesh->GetCenter();
						uvec::local_to_world(posWorld, rotWorld, pos);
						dbgObjects.push_back(DebugRenderer::DrawLine(clusterCenter, pos, (*it == pCurrentNode->cluster) ? Color::Red : Color::Lime));
					}
				}
			}
		}

		if(val == 1) {
			// Draw vis leafs
			dbgObjects.push_back(DebugRenderer::DrawBox(pCurrentNode->min, pCurrentNode->max, {Color {255, 0, 0, 255}, Color::Aqua}));
			auto &nodes = bspTree->GetNodes();
			std::vector<util::BSPTree::Node *> clusterNodes {};
			clusterNodes.reserve(visClusters.size());
			for(auto clusterId : visClusters) {
				auto itNode = std::find_if(nodes.begin(), nodes.end(), [clusterId](const util::BSPTree::Node &node) { return node.cluster == clusterId; });
				clusterNodes.push_back((itNode != nodes.end()) ? &*itNode : nullptr);
			}
			for(auto clusterId : visClusters) {
				auto visClusterId = clusterIdToVisClusterId.at(clusterId);
				auto *pNode = (visClusterId < clusterNodes.size()) ? clusterNodes.at(visClusterId) : nullptr;
				if(pNode == nullptr) {
					Con::cwar << "Reference to invalid cluster id " << clusterId << Con::endl;
					continue;
				}
				dbgObjects.push_back(DebugRenderer::DrawBox(pNode->min, pNode->max, {Color {0, 255, 0, 64}, Color::ForestGreen}));
			}
		}
	}
}
REGISTER_CONVAR_CALLBACK_CL(debug_bsp_nodes, debug_bsp_nodes);

static void debug_bsp_lock_callback(NetworkState *, const ConVar &, int32_t, int32_t val)
{
	if(c_game == nullptr)
		return;
	auto *scene = c_game->GetScene();
	auto *world = c_game->GetWorld();
	auto bspTree = world ? static_cast<CWorldComponent *>(world)->GetBSPTree() : nullptr;
	if(true) //bspTree == nullptr)
	{
		Con::cwar << "Scene does not have BSP tree!" << Con::endl;
		return;
	}
	//bspTree->SetCurrentNodeLocked(val != 0);
}
REGISTER_CONVAR_CALLBACK_CL(debug_bsp_lock, debug_bsp_lock_callback);
