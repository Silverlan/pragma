/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_hitbox_bvh_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
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
	InitializeHitboxBvh();
	DebugDraw();
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

void CHitboxBvhComponent::UpdateTest() { UpdateHitboxBvh(); }

void CHitboxBvhComponent::UpdateHitboxBvh()
{
	if(!m_hitboxBvh)
		return;
	auto *animC = static_cast<CAnimatedComponent *>(GetEntity().GetAnimatedComponent().get());
	if(!animC)
		return;
	auto &bonePoses = animC->GetProcessedBones();
	for(auto &hbObb : m_hitboxBvh->primitives) {
		if(hbObb.boneId >= bonePoses.size())
			continue;
		auto &pose = bonePoses[hbObb.boneId];
		hbObb.pose = pose;
		//m_hitboxBvh->bvh.nodes[0];
		// TODO: Re-scale min,max bounds?
	}
	m_hitboxBvh->Refit();
}

void CHitboxBvhComponent::InitializeHitboxBvh()
{
	auto &mdl = GetEntity().GetModel();
	auto &hitboxes = mdl->GetHitboxes();
	auto bvhTree = std::make_unique<ObbBvhTree>();
	auto &hitboxObbs = bvhTree->primitives;
	hitboxObbs.reserve(hitboxes.size());
	auto &ref = mdl->GetReference();
	for(auto &pair : hitboxes) {
		umath::ScaledTransform pose;
		if(!ref.GetBonePose(pair.first, pose))
			continue;
		auto &hb = pair.second;
		hitboxObbs.push_back({hb.min, hb.max});
		auto &hbObb = hitboxObbs.back();
		hbObb.pose = pose;
		hbObb.boneId = pair.first;
	}

	// Test
	/*Vector3 pos {0.f, 50.f, 0.f};
	hitboxObbs.push_back({pos + Vector3 {-5.f, -5.f, -5.f}, pos + Vector3 {5.f, 5.f, 5.f}});
	pos = {20.f, 30.f, 0.f};
	hitboxObbs.push_back({pos + Vector3 {-5.f, -5.f, -5.f}, pos + Vector3 {5.f, 5.f, 5.f}});
	pos = {0.f, 40.f, 20.f};
	hitboxObbs.push_back({pos + Vector3 {-5.f, -5.f, -5.f}, pos + Vector3 {5.f, 5.f, 5.f}});*/
	//pos = {-20.f, 30.f, 0.f};
	//hitboxObbs.push_back({pos + Vector3 {-5.f, -5.f, -5.f}, pos + Vector3 {5.f, 5.f, 5.f}});
	//pos = {0.f, 30.f, -20.f};
	//hitboxObbs.push_back({pos + Vector3 {-5.f, -5.f, -5.f}, pos + Vector3 {5.f, 5.f, 5.f}});
	//

	bvhTree->InitializeBvh();
	m_hitboxBvh = std::move(bvhTree);
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
		auto valid = true;
		mesh->VisitIndices([&triIndices, &verts, &invPose, &bvhTris, &valid](auto *indexDataSrc, uint32_t numIndicesSrc) {
			auto numVerts = verts.size();
			for(auto triIdx : triIndices) {
				if(triIdx + 2 >= numIndicesSrc) {
					valid = false;
					return;
				}
				auto idx0 = indexDataSrc[triIdx * 3];
				auto idx1 = indexDataSrc[triIdx * 3 + 1];
				auto idx2 = indexDataSrc[triIdx * 3 + 2];
				if(idx0 >= numVerts || idx1 >= numVerts || idx2 >= numVerts) {
					valid = false;
					return;
				}
				auto &v0 = verts[idx0];
				auto &v1 = verts[idx1];
				auto &v2 = verts[idx2];
				auto pos0 = invPose * v0.position;
				auto pos1 = invPose * v1.position;
				auto pos2 = invPose * v2.position;
				bvhTris.push_back(bvh::create_triangle(pos0, pos1, pos2));
			}
		});

		if(!valid)
			continue;

		auto bvhData = bvh::create_bvh_data(std::move(bvhTris));
		m_hitboxBvhs[boneId] = std::move(bvhData);
	}
}

void CHitboxBvhComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, pragma::bvh::HitInfo &outHitInfo)
{
	if(!m_hitboxBvh)
		return;

	auto localOrigin = origin;
	auto localDir = dir;
	auto &entPose = GetEntity().GetPose();
	localOrigin -= entPose.GetOrigin();
	uvec::rotate(&localDir, uquat::get_inverse(entPose.GetRotation()));

	// Put raycast into relative space
	std::vector<ObbBvhTree::HitData> hits;
	auto res = m_hitboxBvh->Raycast(localOrigin, localDir, minDist, maxDist, hits);
	Con::cout << (res ? "1" : "0") << Con::endl;
	if(!res)
		return;
	for(auto &hitData : hits) {
		auto &hObb = m_hitboxBvh->primitives[hitData.primitiveIndex];
		// TODO: We need ALL hits!
		// TODO: Get hitbox mesh

		// TODO: CHeck hitbox intersections
		// If intersection:
		// TODO: Hitbox BVH
		auto ray = pragma::bvh::get_ray(origin, dir, minDist, maxDist);
		for(auto &pair : m_hitboxBvhs) {
#if 0
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
#endif
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
	if(!m_hitboxBvh)
		return;
	auto color = Color::Red;
	auto outlineColor = Color::Lime;
	auto duration = 20.f;
	for(auto &hObb : m_hitboxBvh->primitives) {
		auto pose = hObb.pose;
		pose.TranslateLocal(hObb.position);
		auto &pos = pose.GetOrigin();
		::DebugRenderer::DrawBox(pos, -hObb.halfExtents, hObb.halfExtents, EulerAngles {pose.GetRotation()}, color, outlineColor, duration);
	}

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
			auto &p0 = prim.p0;
			auto &p1 = prim.p1;
			auto &p2 = prim.p2;
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

#include <mathutil/boundingvolume.h>
#include <bvh/v2/stack.h>

pragma::CHitboxBvhComponent::HitboxObb::HitboxObb(const Vector3 &min, const Vector3 &max) : min {min}, max {max}
{
	position = (max + min) / 2.f;
	halfExtents = (max - min) / 2.f;
}

pragma::bvh::BBox pragma::CHitboxBvhComponent::HitboxObb::ToBvhBBox(Vector3 &outOrigin) const
{
	// Calculate world space axes
	glm::mat3 rotationMatrix = glm::mat3_cast(pose.GetRotation());
	glm::vec3 worldX = rotationMatrix * glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 worldY = rotationMatrix * glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 worldZ = rotationMatrix * glm::vec3(0.0f, 0.0f, 1.0f);

	// Calculate AABB extents
	glm::vec3 xAxisExtents = glm::abs(worldX * halfExtents.x);
	glm::vec3 yAxisExtents = glm::abs(worldY * halfExtents.y);
	glm::vec3 zAxisExtents = glm::abs(worldZ * halfExtents.z);

	auto posRot = position;
	uvec::rotate(&posRot, pose.GetRotation());
	auto &origin = outOrigin;
	origin = pose.GetOrigin() + posRot;

	// Calculate AABB min and max
	auto aabbMin = origin - xAxisExtents - yAxisExtents - zAxisExtents;
	auto aabbMax = origin + xAxisExtents + yAxisExtents + zAxisExtents;
	return pragma::bvh::BBox {pragma::bvh::to_bvh_vector(aabbMin), pragma::bvh::to_bvh_vector(aabbMax)};
	/*auto &basePose = pose;

	auto posRot = position;
	uvec::rotate(&posRot, basePose.GetRotation());
	auto origin = basePose.GetOrigin() + posRot;

	Mat3 mRot {basePose.GetRotation()};
	for(uint8_t x = 0; x < 3; ++x) {
		for(uint8_t y = 0; y < 3; ++y)
			mRot[x][y] = glm::abs(mRot[x][y]);
	}

	auto hfAabb = mRot * Vector3 {1.f, 1.f, 1.f};
	return pragma::bvh::BBox {pragma::bvh::to_bvh_vector(origin - hfAabb), pragma::bvh::to_bvh_vector(origin + hfAabb)};*/
}

bool ObbBvhTree::DoInitializeBvh(::bvh::v2::ParallelExecutor &executor, ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::Config &config)
{
	auto numObbs = primitives.size();
	if(numObbs == 0)
		return false;
	std::vector<pragma::bvh::BBox> bboxes {numObbs};
	std::vector<pragma::bvh::Vec> centers {numObbs};
	executor.for_each(0, numObbs, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			Vector3 center;
			bboxes[i] = primitives[i].ToBvhBBox(center);
			centers[i] = bvh::to_bvh_vector(center);
		}
	});

	bvh = ::bvh::v2::DefaultBuilder<pragma::bvh::Node>::build(GetThreadPool(), bboxes, centers, config);
	return true;
}

static Vector3 to_vec(pragma::bvh::Vec vec) { return Vector3 {vec.values[0], vec.values[1], vec.values[2]}; }
static void draw_node(const pragma::bvh::Node &node, const Vector3 &offset = {}, float duration = 20.f)
{
	auto bbox = node.get_bbox();
	auto vstart = to_vec(bbox.min);
	auto vend = to_vec(bbox.max);
	DebugRenderer::DrawBox(vstart + offset, vend + offset, EulerAngles {}, Color {0, 255, 0, 64}, Color::Aqua, duration);
}
static void print_bvh_tree(pragma::bvh::Bvh &bvh)
{
	std::stringstream ss;
	std::function<void(const pragma::bvh::Node &, std::string)> printStack = nullptr;
	printStack = [&printStack, &ss, &bvh](const pragma::bvh::Node &node, std::string t) {
		auto bbox = node.get_bbox();
		auto min = to_vec(bbox.min);
		auto max = to_vec(bbox.max);
		auto isLeaf = node.index.prim_count > 0;
		ss << t;
		if(isLeaf)
			ss << "Leaf";
		else
			ss << "Node";
		ss << "[" << min.x << "," << min.y << "," << min.z << "][" << max.x << "," << max.y << "," << max.z << "]\n";
		if(isLeaf)
			return;
		printStack(bvh.nodes[node.index.first_id], t + "\t");
		printStack(bvh.nodes[node.index.first_id + 1], t + "\t");
	};
	printStack(bvh.get_root(), "");
	Con::cout << "Tree:" << ss.str() << Con::endl;
}
static void draw_bvh_tree(pragma::bvh::Bvh &bvh, float duration = 20.f)
{
	constexpr size_t stack_size = 64;
	::bvh::v2::SmallStack<pragma::bvh::Bvh::Index, stack_size> stack;
	draw_node(bvh.get_root(), {}, duration);
	auto start = bvh.get_root().index;
	stack.push(start);
restart:
	while(!stack.is_empty()) {
		auto top = stack.pop();
		while(top.prim_count == 0) {
			auto &left = bvh.nodes[top.first_id];
			auto &right = bvh.nodes[top.first_id + 1];

			draw_node(left, {}, duration);
			draw_node(right, {}, duration);

			if(true) {
				auto near_index = left.index;
				if(true)
					stack.push(right.index);
				top = near_index;
			}
		}
	}
}

#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
bool ObbBvhTree::Raycast(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, std::vector<HitData> &outHits)
{
	constexpr size_t invalid_id = std::numeric_limits<size_t>::max();
	constexpr size_t stack_size = 64;
	constexpr bool use_robust_traversal = false;

	print_bvh_tree(bvh);

	auto distDiff = maxDist - minDist;
	::bvh::v2::SmallStack<pragma::bvh::Bvh::Index, stack_size> stack;
	auto ray = pragma::bvh::get_ray(origin, dir, minDist, maxDist);
	bvh.intersect<false, use_robust_traversal>(ray, bvh.get_root().index, stack, [&](size_t begin, size_t end) {
		for(size_t i = begin; i < end; ++i) {
			size_t j = bvh.prim_ids[i];

			auto &obb = primitives[j];
			float dist;

			auto tmpDir = dir * maxDist;
			auto hit = umath::intersection::line_obb(origin, tmpDir, obb.min, obb.max, &dist, obb.pose.GetOrigin(), obb.pose.GetRotation());
			auto hitColor = hit ? Color {255, 255, 0, 64} : Color {0, 255, 255, 64};
			DebugRenderer::DrawBox(obb.pose.GetOrigin(), obb.min, obb.max, EulerAngles {obb.pose.GetRotation()}, hitColor, Color::White, 12.f);
			if(hit) {
				dist *= maxDist;
				if(outHits.size() == outHits.capacity())
					outHits.reserve(outHits.size() * 2 + 5);
				HitData hitData {};
				hitData.primitiveIndex = i;

				ray.tmax = dist;
				if(distDiff > 0.0001f)
					hitData.t = (ray.tmax - minDist) / distDiff;
				else
					hitData.t = 0.f;

				util::insert_sorted(outHits, hitData, [](const HitData &a, const HitData &b) { return a.t < b.t; });
			}
		}
		return false;
	});
	return !outHits.empty();
}
