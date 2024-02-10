/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_hitbox_bvh_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include <pragma/entities/components/bvh_data.hpp>
#include <pragma/entities/components/util_bvh.hpp>
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <panima/skeleton.hpp>
#include <panima/bone.hpp>
#include <panima/skeleton.hpp>
#include <pragma/entities/components/base_bvh_component.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

struct HitboxBvh {
	std::shared_ptr<pragma::bvh::BvhData> bvh;
};

using namespace pragma;
#pragma optimize("", off)
CHitboxBvhComponent::CHitboxBvhComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void CHitboxBvhComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CHitboxBvhComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnModelChanged(); });
}

void CHitboxBvhComponent::InitializeBvh()
{
	if(!GetEntity().IsSpawned())
		return;
	InitializeModel();
	InitializeHitboxMeshes();
}

void CHitboxBvhComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	InitializeBvh();
}

void CHitboxBvhComponent::OnModelChanged() { InitializeBvh(); }

bool CHitboxBvhComponent::InitializeModel()
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	mdl->GenerateLowLevelLODs(GetGame());
	auto extData = mdl->GetExtensionData();
	auto udmHbMeshes = extData["hitboxMeshes"];
	if(!udmHbMeshes)
		pragma::CHitboxBvhComponent::generate_hitbox_meshes(*mdl);
	InitializeHitboxMeshes();
	// TODO: Save model
	// std::string err;
	// mdl->Save(GetGame(), err);

	// TODO: Only initialize bvh meshes ONCE per model! -> Re-use between entities
	return true;
}

void CHitboxBvhComponent::InitializeHitboxMeshes()
{
	auto &mdl = GetEntity().GetModel();
	auto extData = mdl->GetExtensionData();
	auto udmHbMeshes = extData["hitboxMeshes"];
	if(!udmHbMeshes)
		return;
	std::unordered_map<std::string, std::shared_ptr<ModelSubMesh>> mdlMeshes;
	for(auto &mg : mdl->GetMeshGroups()) {
		for(auto &m : mg->GetMeshes()) {
			for(auto &sm : m->GetSubMeshes()) {
				mdlMeshes[util::uuid_to_string(sm->GetUuid())] = sm;
			}
		}
	}
	auto &skeleton = mdl->GetSkeleton();
	auto &ref = mdl->GetReference();
	for(auto udmHbMesh : udmHbMeshes.ElIt()) {
		auto &boneName = udmHbMesh.key;
		auto boneId = skeleton.LookupBone(std::string {boneName});
		if(boneId < 0)
			continue;

		umath::ScaledTransform pose;
		if(!ref.GetBonePose(boneId, pose))
			continue;
		auto invPose = pose.GetInverse();

		std::string meshUuid;
		udmHbMesh.property["meshUuid"](meshUuid);
		auto itMesh = mdlMeshes.find(meshUuid);
		if(itMesh == mdlMeshes.end())
			continue;
		auto &mesh = itMesh->second;
		std::vector<uint32_t> triIndices;
		udmHbMesh.property["triangleIndices"](triIndices);
		if(triIndices.empty())
			continue;
		auto &verts = mesh->GetVertices();

		std::vector<bvh::Primitive> bvhTris;
		bvhTris.reserve(triIndices.size());
		mesh->VisitIndices([&triIndices, &verts, &invPose, & bvhTris](auto *indexDataSrc, uint32_t numIndicesSrc) {
			for(auto triIdx : triIndices) {
				auto idx0 = indexDataSrc[triIdx * 3];
				auto idx1 = indexDataSrc[triIdx * 3 + 1];
				auto idx2 = indexDataSrc[triIdx * 3 + 2];
				auto &v0 = verts[idx0];
				auto &v1 = verts[idx1];
				auto &v2 = verts[idx2];
				auto pos0 = invPose * v0.position;
				auto pos1 = invPose * v1.position;
				auto pos2 = invPose * v2.position;
				bvhTris.push_back(bvh::create_triangle(pos0, pos1, pos2));
			}
		});

		auto bvhData = bvh::create_bvh_data(std::move(bvhTris));
		m_hitboxBvhs[boneId] = std::move(bvhData);
	}

	DebugDraw();
}

void CHitboxBvhComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, pragma::bvh::HitInfo &outHitInfo)
{
	// TODO: CHeck hitbox intersections
	// If intersection:
	// TODO: Hitbox BVH
	auto ray = pragma::bvh::get_ray(origin, dir, minDist, maxDist);
	for(auto &pair : m_hitboxBvhs) {
		Vector3 min, max;
		float t;
		auto res = umath::intersection::line_aabb(origin, dir, min, max, &t);
		if(res != umath::intersection::Result::Intersect)
			continue;
		auto &bvhData = pair.second;
		auto &traverser = bvhData->intersectorData->traverser;
		auto &primitiveIntersector = bvhData->intersectorData->primitiveIntersector;
		auto hit = traverser.traverse(ray, primitiveIntersector);
		// TODO: Closest hit
		if(hit) {
			/*pragma::bvh::MeshRange search {};
			search.start = hit->primitive_index * 3;
			auto it = std::upper_bound(bvhData->meshRanges.begin(), bvhData->meshRanges.end(), search);
			assert(it != bvhData->meshRanges.begin());
			--it;

			auto &hitInfo = outHitInfo;
			hitInfo.primitiveIndex = hit->primitive_index - it->start / 3;
			hitInfo.distance = hit->distance();
			hitInfo.u = hit->intersection.u;
			hitInfo.v = hit->intersection.v;
			hitInfo.t = hit->intersection.t;
			hitInfo.mesh = it->mesh;
			hitInfo.entity = it->entity ? it->entity->GetHandle() : GetEntity().GetHandle();*/
			return;
		}
		//umath::intersection::line_obb(origin, dir, min, max, &t, pos, rot);
		//std::shared_ptr<pragma::BvhData> BaseBvhComponent::RebuildBvh(const std::vector<std::shared_ptr<ModelSubMesh>> &meshes, const BvhBuildInfo *optBvhBuildInfo, std::vector<size_t> *optOutMeshIndices)
		/*struct DLLNETWORK BvhHitInfo {
			std::shared_ptr<ModelSubMesh> mesh;
			EntityHandle entity;
			size_t primitiveIndex;
			float distance;
			float t;
			float u;
			float v;
		};*/

		//pair.second->
	}
}

void CHitboxBvhComponent::OnRemove() {}

void pragma::CHitboxBvhComponent::generate_hitbox_meshes(Model &mdl)
{
	auto &lods = mdl.GetLODs();
	if(lods.empty())
		return;
	auto extData = mdl.GetExtensionData();
	auto udmHbMeshes = extData["hitboxMeshes"];

	auto lodLast = lods.back();

	auto &ref = mdl.GetReference();
	auto &hitboxes = mdl.GetHitboxes();
	auto &skeleton = mdl.GetSkeleton();
	for(auto &pair : hitboxes) {
		auto bone = skeleton.GetBone(pair.first).lock();
		if(!bone)
			continue;
		auto &hb = pair.second;
		umath::ScaledTransform pose;
		if(!ref.GetBonePose(pair.first, pose))
			continue;
		auto &pos = pose.GetOrigin();
		auto &rot = pose.GetRotation();
		std::vector<umath::Plane> planes {
		  umath::Plane {uquat::up(rot), pos + uquat::up(rot) * hb.max.y},
		  umath::Plane {-uquat::up(rot), pos + uquat::up(rot) * hb.min.y},
		  umath::Plane {uquat::forward(rot), pos + uquat::forward(rot) * hb.max.z},
		  umath::Plane {-uquat::forward(rot), pos + uquat::forward(rot) * hb.min.z},
		  umath::Plane {-uquat::right(rot), pos - uquat::right(rot) * hb.max.x},
		  umath::Plane {uquat::right(rot), pos - uquat::right(rot) * hb.min.x},
		};
		auto hbMin = pos + hb.min;
		auto hbMax = pos + hb.max;
		for(auto &pair : lodLast.meshReplacements) {
			auto mg = mdl.GetMeshGroup(pair.second);
			if(!mg)
				continue;
			for(auto &mesh : mg->GetMeshes()) {
				for(auto &subMesh : mesh->GetSubMeshes()) {
					Vector3 smMin, smMax;
					subMesh->GetBounds(smMin, smMax);
					if(umath::intersection::aabb_in_plane_mesh(smMin, smMax, planes) != umath::intersection::Intersect::Outside) {
						auto &verts = subMesh->GetVertices();
						auto numVerts = verts.size();

						std::vector<uint32_t> usedTris;
						usedTris.reserve(subMesh->GetTriangleCount());
						subMesh->VisitIndices([&verts, &usedTris, &hbMin, &hbMax, &rot](auto *indexDataSrc, uint32_t numIndicesSrc) {
							for(auto i = decltype(numIndicesSrc) {0u}; i < numIndicesSrc; i += 3) {
								auto idx0 = indexDataSrc[i];
								auto idx1 = indexDataSrc[i + 1];
								auto idx2 = indexDataSrc[i + 2];
								auto &v0 = verts[idx0];
								auto &v1 = verts[idx1];
								auto &v2 = verts[idx2];

								if(umath::intersection::obb_triangle(hbMin, hbMax, rot, v0.position, v1.position, v2.position))
									usedTris.push_back(i / 3);
							}
						});

						if(usedTris.empty())
							continue;
						auto udmHb = udmHbMeshes[bone->name];
						udmHb["meshUuid"] = util::uuid_to_string(subMesh->GetUuid());
						udmHb.AddArray("triangleIndices", usedTris, udm::ArrayType::Compressed);
					}
				}
			}
		}
	}
	Con::cout << "extData: " << Con::endl;
	std::stringstream ss;
	extData->ToAscii(udm::AsciiSaveFlags::DontCompressLz4Arrays, ss, "");
	Con::cout << ss.str() << Con::endl;
}

#include "pragma/debug/c_debugoverlay.h"
void CHitboxBvhComponent::DebugDraw()
{
	auto color = Color::Red;
	auto outlineColor = Color::Lime;
	auto duration = 200.f;
	auto &mdl = GetEntity().GetModel();
	auto &ref = mdl->GetReference();
	Con::cout << "Count: " << m_hitboxBvhs.size() << Con::endl;
	for(auto &pair : m_hitboxBvhs) {
		umath::ScaledTransform pose;
		if(!ref.GetBonePose(pair.first, pose))
			continue;
		std::vector<Vector3> dbgMeshVerts;
		dbgMeshVerts.reserve(pair.second->primitives.size() * 3);
		for(auto &prim : pair.second->primitives) {
			auto p0 = prim.p0;
			auto p1 = prim.p1();
			auto p2 = prim.p2();
			auto v0 = Vector3 {p0[0], p0[1], p0[2]};
			auto v1 = Vector3 {p1[0], p1[1], p1[2]};
			auto v2 = Vector3 {p2[0], p2[1], p2[2]};
			v0 = pose * v0;
			v1 = pose * v1;
			v2 = pose * v2;
			dbgMeshVerts.push_back(v0);
			dbgMeshVerts.push_back(v1);
			dbgMeshVerts.push_back(v2);
		}
		::DebugRenderer::DrawMesh(dbgMeshVerts, color, outlineColor, duration);
	}
}
