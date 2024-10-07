/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/physics/phys_water_buoyancy_simulator.hpp"
#include "pragma/physics/phys_water_surface_simulator.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/phys_liquid.hpp"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/collisionmesh.h"
#include "pragma/util/util_best_fitting_plane.hpp"
#include "pragma/util/util_game.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include <pragma/physics/movetypes.h>

// See http://www.randygaul.net/wp-content/uploads/2014/02/RigidBodies_WaterSurface.pdf for algorithms

#define ENABLE_DEBUG_DRAW 0

pragma::physics::WaterBuoyancySimulator::WaterBuoyancySimulator() {}

template<class InputItVert, class InputItIndex>
double pragma::physics::WaterBuoyancySimulator::CalcBuoyancy(const Quat &rorigin, const PhysLiquid &liquid, const Vector3 &waterPlane, double waterPlaneDist, const Vector3 &waterPlaneRelObj, double waterPlaneDistRelObj, const Vector3 &waterVelocity, double gravity, InputItVert vertsBegin,
  InputItIndex indicesBegin, InputItIndex indicesEnd, double mass, double volBody, const Vector3 &bodyVelocity, const Vector3 &bodyAngularVelocity, Vector3 *force, Vector3 *torque, Vector3 *optOutSubmergedCenter) const
{
	auto C = waterPlaneRelObj * static_cast<float>(waterPlaneDistRelObj);

	auto submergedVolume = 0.0;
	Vector3 submergedCenter = {};
	uint32_t submergedVertCount = 0;

	Vector3 bodyCenter = {};
	uint32_t bodyVertCount = 0;

	for(auto it = indicesBegin; it != indicesEnd; it += 3) {
		auto &v0 = *(vertsBegin + *it);
		auto &v1 = *(vertsBegin + *(it + 1));
		auto &v2 = *(vertsBegin + *(it + 2));

		bodyCenter += v0 + v1 + v2;
		bodyVertCount += 3;

		auto d0 = CalcDistanceFromVertexToPlane(v0, waterPlaneRelObj, waterPlaneDistRelObj);
		auto d1 = CalcDistanceFromVertexToPlane(v1, waterPlaneRelObj, waterPlaneDistRelObj);
		auto d2 = CalcDistanceFromVertexToPlane(v2, waterPlaneRelObj, waterPlaneDistRelObj);
		Vector3 center;
		uint32_t vertCount;
#if ENABLE_DEBUG_DRAW == 1
		std::vector<Vector3> clippedVerts;
		std::vector<Vector3> *pclippedVerts = &clippedVerts;
#else
		std::vector<Vector3> *pclippedVerts = nullptr;
#endif
		auto clipVol = CalcClipVolume(C, v0, v1, v2, d0, d1, d2, &center, &vertCount, pclippedVerts);
#if ENABLE_DEBUG_DRAW == 1
		for(auto i = decltype(clippedVerts.size()) {0u}; i < clippedVerts.size(); i += 3) {
			auto &v0 = clippedVerts[i];
			auto &v1 = clippedVerts[i + 1];
			auto &v2 = clippedVerts[i + 2];
			std::stringstream ss;
			ss << "local drawInfo = debug.DrawInfo(); drawInfo:SetColor(Color(0,255,255,64));";
			ss << "drawInfo:SetDuration(0.1);";
			ss << "debug.draw_mesh({";
			ss << "Vector(" << v0.x << "," << v0.y << "," << v0.z << "),";
			ss << "Vector(" << v1.x << "," << v1.y << "," << v1.z << "),";
			ss << "Vector(" << v2.x << "," << v2.y << "," << v2.z << ")},drawInfo)";
			pragma::get_engine()->GetClientState()->GetGameState()->RunLua(ss.str());
		}
#endif
		submergedCenter += center;
		submergedVertCount += vertCount;

		submergedVolume += clipVol;
	}
	if(bodyVertCount > 0)
		bodyCenter /= static_cast<float>(bodyVertCount);
	if(submergedVertCount > 0)
		submergedCenter /= static_cast<float>(submergedVertCount);

	if(optOutSubmergedCenter)
		*optOutSubmergedCenter = submergedCenter;

	auto bSubmerged = (submergedVolume > 0.0) ? true : false;
	if(bSubmerged == false)
		return 0.0;

	// These factors shouldn't be necessary, but without them
	// the forces are excessive. Reason is currently unknown. FIXME?
	constexpr auto forceFactor = 0.0700000301;
	constexpr auto torqueFactor = 0.0700000301;

	if(force || torque) {
		// Liquid density is specified in kg/m^3, we'll have to convert it to Pragma's scale
		constexpr auto scale = umath::pow3(pragma::units_to_metres(1.0));
		auto density = liquid.density * scale;
		if(force != nullptr) {
			*force = CalcBuoyancy(density, submergedVolume, gravity, waterPlane);
			*force *= forceFactor;
			*force += CalcCattoDragLinearForceApproximation(liquid.linearDragCoefficient, mass, submergedVolume, volBody, waterVelocity, bodyVelocity); // TODO
		}

		if(torque != nullptr) {
			*torque = CalcTorque(rorigin, bodyCenter, submergedCenter, density, submergedVolume, gravity, waterPlane);
			*torque *= torqueFactor;
			auto lenPolyhedron = uvec::distance(bodyCenter, submergedCenter); // TODO: Optimize
			*torque += CalcCattoDragTorqueForceApproximation(liquid.torqueDragCoefficient, mass, submergedVolume, volBody, lenPolyhedron, bodyAngularVelocity);
		}
	}
	return submergedVolume;
}

template<class TIterator>
static void calc_surface_plane(const PhysWaterSurfaceSimulator *surfaceSim, const Vector3 &pos, const Quat &rot, TIterator itVertBegin, TIterator itVertEnd, Vector3 &waterPlane, double &waterPlaneDist, Vector3 &waterPlaneRelObj, double &waterPlaneDistRelObj)
{
	if(surfaceSim == nullptr || itVertBegin == itVertEnd)
		return;
	// Move water plane to coordinate system of surface
	//auto *game = ent.GetNetworkState()->GetGameState(); // TODO
	/*static Vector3 offset {-700.f,195.5f,-493.5f};
	waterPlaneDistRelObj = uvec::dot(waterPlaneRelObj,waterPlaneRelObj *static_cast<float>(waterPlaneDistRelObj) +offset);
	auto *game = ent.GetNetworkState()->GetGameState(); // TODO
	if(game->IsClient())
	{
		game->DrawLine(Vector3(),waterPlaneRelObj *static_cast<float>(waterPlaneDistRelObj),Color::Red,0.1f);
		Con::cout<<"Distance: "<<waterPlaneDistRelObj<<Con::endl;
	}*/

	Vector3 min {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
	Vector3 max {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
	for(auto it = itVertBegin; it != itVertEnd; ++it) {
		auto vProj = uvec::project_to_plane(*it, waterPlaneRelObj, waterPlaneDistRelObj);
		/*if(game->IsClient())
		{
			auto x = vProj;
			uvec::local_to_world(pos,rot,x);
			game->DrawLine(x,x +Vector3(0,100,0),Color::White,5.f);
		}*/
		uvec::min(&min, vProj); // TODO: What if plane is slanted?
		uvec::max(&max, vProj);
	}

	// Transform bounds from entity space to world space
	uvec::local_to_world(pos, rot, min);
	uvec::local_to_world(pos, rot, max);

	/*auto *entWater = ent.GetNetworkState()->GetGameState()->FindEntityByClass("func_water");
	// Transform bounds from world space to water space
	{
		auto &pos = entWater->GetPosition();
		auto &rot = entWater->GetOrientation();
		uvec::world_to_local(pos,rot,min);
		uvec::world_to_local(pos,rot,max);
	}*/

	const std::array<Vector3, 4> aabb2d = {Vector3(min.x, 0.f, min.z), // TODO: Plane orientation
	  Vector3(min.x, 0.f, max.z), Vector3(max.x, 0.f, min.z), Vector3(max.x, 0.f, max.z)};
	static std::vector<Vector3> surfaceIntersectionPoints = {};
	surfaceIntersectionPoints.clear();
	surfaceIntersectionPoints.reserve(aabb2d.size());
	for(auto &v : aabb2d) {
		Vector3 intersection;
		if(surfaceSim->CalcPointSurfaceIntersection(v, intersection) == true)
			surfaceIntersectionPoints.push_back(intersection);
	}
	if(surfaceIntersectionPoints.size() > 2) // Need at least three points to calculate a plane
	{
		// Water surface space to world space
		/*for(auto &p : surfaceIntersectionPoints)
		{
			auto &pos = entWater->GetPosition();
			auto &rot = entWater->GetOrientation();
			uvec::local_to_world(pos,rot,min);
			uvec::local_to_world(pos,rot,max);
		}*/

		// World space to entity space
		// TODO: Could skip this conversion and transform the new plane from world space to entity space instead
		for(auto &p : surfaceIntersectionPoints)
			uvec::world_to_local(pos, rot, p);

		auto avg = uvec::calc_average(surfaceIntersectionPoints);
		auto mat = umat::calc_covariance_matrix(surfaceIntersectionPoints, avg);
		auto oldPlane = waterPlaneRelObj;

		umath::calc_best_fitting_plane(mat, avg, waterPlaneRelObj, waterPlaneDistRelObj);

		// Plane might be pointing in the wrong direction; Flip it if it is
		if(umath::abs(uvec::dot(oldPlane, waterPlaneRelObj) - 1.0) > 1.0) {
			waterPlaneRelObj = -waterPlaneRelObj;
			waterPlaneDistRelObj = -waterPlaneDistRelObj;
		}

		// Move water plane to world space
		waterPlane = waterPlaneRelObj;
		waterPlaneDist = waterPlaneDistRelObj;
		uvec::rotate(&waterPlane, rot);
		waterPlaneDist = uvec::dot(waterPlane, waterPlane * static_cast<float>(waterPlaneDist) + pos);

		/*auto *game = ent.GetNetworkState()->GetGameState(); // TODO
		if(game->IsClient())
		{
			for(auto &p : surfaceIntersectionPoints)
				game->DrawLine(p,p +Vector3(0,100,0),Color::White,1.f);
			game->DrawPlane(waterPlaneRelObj,-waterPlaneDistRelObj,Color::Red,1.f);
		}*/
	}
}

void pragma::physics::WaterBuoyancySimulator::Simulate(BaseEntity &entWater, const PhysLiquid &liquid, BaseEntity &ent, Vector3 waterPlane, double waterPlaneDist, const Vector3 &waterVelocity, const PhysWaterSurfaceSimulator *surfaceSim) const
{
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto physType = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsType() : PHYSICSTYPE::NONE;
	auto *physObj = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(physObj == nullptr)
		return;
	auto pGravityComponent = ent.GetComponent<pragma::GravityComponent>();
	auto gravity = pGravityComponent.valid() ? pGravityComponent->GetGravityForce() : Vector3 {};

	auto totalVolume = 0.0;
	auto totalSubmerged = 0.0;
	if(physType == PHYSICSTYPE::BOXCONTROLLER || physType == PHYSICSTYPE::CAPSULECONTROLLER) {
		if(pPhysComponent->GetMoveType() == MOVETYPE::NOCLIP)
			return;
		const auto mass = 10.0;
		const auto dragCoefficient = 2.0;

		Vector3 min, max;
		pPhysComponent->GetCollisionBounds(&min, &max);
		auto bounds = max - min;
		auto volume = bounds.x * bounds.y * bounds.z;

		auto pTrComponent = ent.GetTransformComponent();
		auto rot = pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity();
		std::array<Vector3, 8> verts = {min, {min.x, min.y, max.z}, {min.x, max.y, max.z}, {max.x, max.y, min.z}, {min.x, max.y, min.z}, {max.x, min.y, max.z}, {max.x, min.y, min.z}, max};
		for(auto &v : verts)
			uvec::rotate(&v, rot);
		constexpr std::array<uint16_t, 36> triangles = {0, 1, 2, 3, 0, 4, 5, 0, 6, 3, 6, 0, 0, 2, 4, 5, 1, 0, 2, 1, 5, 7, 6, 3, 6, 7, 5, 7, 3, 4, 7, 4, 2, 7, 2, 5};
		totalVolume += volume;

		// Move water plane to collision object coordinate system
		auto pose = pTrComponent->GetPose();
		pose.SetRotation(uquat::identity()); // No rotation; TODO: Why?
		auto relPlane = pose.GetInverse() * umath::Plane {waterPlane, waterPlaneDist};
		auto &waterPlaneRelObj = relPlane.GetNormal();
		auto waterPlaneDistRelObj = relPlane.GetDistance();
		calc_surface_plane(surfaceSim, pose.GetOrigin(), rot, verts.begin(), verts.end(), waterPlane, waterPlaneDist, waterPlaneRelObj, waterPlaneDistRelObj);

		auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
		auto velEnt = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {};
		Vector3 bouyany;
		totalSubmerged += CalcBuoyancy({}, liquid, waterPlane, waterPlaneDist, waterPlaneRelObj, waterPlaneDistRelObj, waterVelocity, -gravity.y, verts.begin(), triangles.begin(), triangles.end(), mass, volume, velEnt, {}, nullptr, nullptr);
		constexpr auto gravityScale = -0.1f;
		bouyany = gravity * gravityScale * static_cast<float>(totalSubmerged / totalVolume);
		bouyany += CalcCattoDragLinearForceApproximation(dragCoefficient, mass, totalSubmerged, totalVolume, waterVelocity, velEnt);
		if(pVelComponent.valid())
			pVelComponent->AddVelocity(bouyany * static_cast<float>(ent.GetNetworkState()->GetGameState()->DeltaTime()));
	}
	else {
		auto &colObjs = physObj->GetCollisionObjects();
		for(auto &hColObj : colObjs) {
			if(hColObj.IsValid() == false)
				continue;
			if(hColObj->IsRigid() == false)
				continue;
			auto *colObj = hColObj->GetRigidBody();
			auto mass = colObj->GetMass();
			auto shape = colObj->GetCollisionShape();
			if(mass == 0.f || shape == nullptr)
				continue;

			Vector3 buoyancy;
			Vector3 torque;

			auto pose = colObj->GetBaseTransform(); // Pose without local pose of physics object
			if(shape->IsConvex()) {
				auto *convexShape = shape->GetConvexShape();
				auto *colMesh = convexShape->GetCollisionMesh();
				if(colMesh != nullptr) {
					auto volume = colMesh->GetVolume();
					auto com = colMesh->GetCenterOfMass();
					auto *verts = &colMesh->GetVertices();
					const auto *triangles = &colMesh->GetTriangles();

					std::vector<Vector3> aabbVerts;
					const std::vector<uint16_t> aabbTris = {
					  0, 6, 7, // 1
					  0, 7, 5, // 1
					  3, 0, 5, // 2
					  3, 1, 0, // 2
					  2, 0, 1, // 3
					  2, 6, 0, // 3
					  7, 6, 2, // 4
					  4, 7, 2, // 4
					  4, 1, 3, // 5
					  1, 4, 2, // 5
					  4, 3, 5, // 6
					  4, 5, 7  // 6
					};
					if(triangles->empty()) {
						// If physics mesh has no triangles, build generic mesh from AABB
						Vector3 min, max;
						colMesh->GetAABB(&min, &max);
#if ENABLE_DEBUG_DRAW == 1
						pragma::get_engine()->GetClientState()->GetGameState()->DrawBox(min, max, EulerAngles {}, Color::Aqua, 0.1f);
#endif
						aabbVerts = {
						  min,                          // 0
						  Vector3(max.x, min.y, min.z), // 1
						  Vector3(max.x, min.y, max.z), // 2
						  Vector3(max.x, max.y, min.z), // 3
						  max,                          // 4
						  Vector3(min.x, max.y, min.z), // 5
						  Vector3(min.x, min.y, max.z), // 6
						  Vector3(min.x, max.y, max.z)  // 7
						};

						verts = &aabbVerts;
						triangles = &aabbTris;
						volume = (max.x - min.x) * (max.y - min.y) * (max.z - min.z);
					}

					totalVolume += volume;

					// Move water plane to collision object coordinate system
					auto relPlane = pose.GetInverse() * umath::Plane {waterPlane, waterPlaneDist};
					auto &waterPlaneRelObj = relPlane.GetNormal();
					auto waterPlaneDistRelObj = relPlane.GetDistance();
#if ENABLE_DEBUG_DRAW == 1
					//pragma::get_engine()->GetClientState()->GetGameState()->DrawPlane(
					//	waterPlaneRelObj,waterPlaneDistRelObj,Color{255,255,0,64},0.1f
					//);
#endif

					calc_surface_plane(surfaceSim, pose.GetOrigin(), pose.GetRotation(), verts->begin(), verts->end(), waterPlane, waterPlaneDist, waterPlaneRelObj, waterPlaneDistRelObj);

					Vector3 submergedCenter;
					totalSubmerged += CalcBuoyancy(pose.GetRotation(), liquid, waterPlane, waterPlaneDist, waterPlaneRelObj, waterPlaneDistRelObj, waterVelocity, -gravity.y, verts->begin(), triangles->begin(), triangles->end(), colObj->GetMass(), volume, colObj->GetLinearVelocity(),
					  colObj->GetAngularVelocity(), &buoyancy, &torque, &submergedCenter);

#if ENABLE_DEBUG_DRAW == 1
					ent.GetNetworkState()->GetGameState()->DrawLine(submergedCenter, submergedCenter + uvec::get_normal(buoyancy) * 20.f, Color::White, 0.1f);
#endif
					static auto applyAtCenter = false;
					if(applyAtCenter)
						colObj->ApplyForce(buoyancy, submergedCenter);
					else
						colObj->ApplyForce(buoyancy);
					colObj->ApplyTorque(torque);
				}
			}
			/*static std::vector<Vector3> verts;
			verts.clear();
			static std::vector<uint16_t> triangles;
			triangles.clear();
			totalVolume = 0.0;
			for(auto &mesh : ent.GetModel()->GetMeshGroup(0)->GetMeshes())
			{
				for(auto &subMesh : mesh->GetSubMeshes())
				{
					auto triangleOffset = verts.size();
					for(auto &v : subMesh->GetVertices())
						verts.push_back(v.position);
					for(auto idx : subMesh->GetTriangles())
						triangles.push_back(idx +triangleOffset);
				}
			}
			totalVolume = Geometry::calc_volume_of_polyhedron(verts,triangles);*/
			/*const std::array<Vector3,8> verts = {
				min,
				{min.x,min.y,max.z},
				{min.x,max.y,max.z},
				{max.x,max.y,min.z},
				{min.x,max.y,min.z},
				{max.x,min.y,max.z},
				{max.x,min.y,min.z},
				max
			};
			//for(auto &v : verts)
			//	uvec::rotate(&v,rot);
			constexpr std::array<uint16_t,36> triangles = {
				0,1,2,
				3,0,4,
				5,0,6,
				3,6,0,
				0,2,4,
				5,1,0,
				2,1,5,
				7,6,3,
				6,7,5,
				7,3,4,
				7,4,2,
				7,2,5
			};*/
			/*{
				auto *nw = ent.GetNetworkState();
				if(nw->IsClient())
				{
					auto *game = nw->GetGameState();
					game->DrawPlane(n,-d,Color::Red,0.1f);
					game->DrawLine(n *static_cast<float>(d),n *static_cast<float>(d) +n *100.f,Color::Lime,0.1f);
					//virtual void DrawPlane(const Vector3 &n,float dist,const Color &color,float duration=0.f)=0;
				}
			}*/
		}
	}
	auto pSubmergedComponent = ent.GetComponent<pragma::SubmergibleComponent>();
	if(pSubmergedComponent.expired())
		return;
	auto submergedPercent = (totalVolume > 0.0) ? (totalSubmerged / totalVolume) : 0.0;

	static auto prev = submergedPercent;
	if(submergedPercent != prev) {
		prev = submergedPercent;
		//Con::cout<<"Submerged: "<<submergedPercent<<Con::endl;
	}

	pSubmergedComponent->SetSubmergedFraction(entWater, submergedPercent);
}

////////////////////////

Vector3 pragma::physics::WaterBuoyancySimulator::CalcBuoyancy(double liquidDensity, double submergedVolume, double gravity, const Vector3 &liquidUpVec) const { return static_cast<float>(liquidDensity * submergedVolume * gravity) * liquidUpVec; }

Vector3 pragma::physics::WaterBuoyancySimulator::CalcTorque(const Quat &rorigin, const Vector3 &centerBody, const Vector3 &centerSubmergedVolume, double liquidDensity, double submergedVolume, double gravity, const Vector3 &liquidUpVec) const
{
	auto r = centerSubmergedVolume - centerBody;
	uvec::rotate(&r, rorigin);
	return uvec::cross(r, static_cast<float>(liquidDensity * submergedVolume * gravity) * liquidUpVec);
}

Vector3 pragma::physics::WaterBuoyancySimulator::IntersectLinePlane(const Vector3 &a, const Vector3 &b, double da, double db) const { return a + static_cast<float>(da / (da - db)) * (b - a); }

double pragma::physics::WaterBuoyancySimulator::CalcTetrahedronVolume(Vector3 &c, const Vector3 &u, const Vector3 &v, const Vector3 &w) const
{
	auto volume = (1.0 / 6.0) * uvec::dot(uvec::cross(u, v), w);
	c += static_cast<float>((1.0 / 4.0) * volume) * (u + v + w);
	return volume;
}

double pragma::physics::WaterBuoyancySimulator::CalcDistanceFromVertexToPlane(const Vector3 &v, const Vector3 &p, double d) const { return uvec::dot(v, p) - d; }

double pragma::physics::WaterBuoyancySimulator::ClipTriangle(Vector3 &C, const Vector3 &a, const Vector3 &b, const Vector3 &c, double d1, double d2, double d3, Vector3 *vertSum, uint32_t *vertCount, std::vector<Vector3> *clippedVerts) const
{
	auto ab = IntersectLinePlane(a, b, d1, d2);
	auto volume = 0.0;

	if(vertSum != nullptr)
		*vertSum = {};
	if(vertCount != nullptr)
		*vertCount = 0;
	if(d1 < 0.0) {
		if(d3 < 0.0) {
			auto bc = IntersectLinePlane(b, c, d2, d3);
			volume += CalcTetrahedronVolume(C, bc, c, a);
			volume += CalcTetrahedronVolume(C, ab, bc, a);

			if(vertSum != nullptr)
				*vertSum = bc + c + a + ab + bc + a;
			if(vertCount != nullptr)
				*vertCount = 6;
			if(clippedVerts != nullptr) {
				clippedVerts->reserve(clippedVerts->size() + 6);
				clippedVerts->push_back(bc);
				clippedVerts->push_back(c);
				clippedVerts->push_back(a);
				clippedVerts->push_back(ab);
				clippedVerts->push_back(bc);
				clippedVerts->push_back(a);
			}
		}
		else {
			auto ac = IntersectLinePlane(a, c, d1, d3);
			volume += CalcTetrahedronVolume(C, a, ab, ac);

			if(vertSum != nullptr)
				*vertSum = a + ab + ac;
			if(vertCount != nullptr)
				*vertCount = 3;
			if(clippedVerts != nullptr) {
				clippedVerts->reserve(clippedVerts->size() + 3);
				clippedVerts->push_back(a);
				clippedVerts->push_back(ab);
				clippedVerts->push_back(ac);
			}
		}
	}
	else {
		if(d3 < 0.0) {
			auto ac = IntersectLinePlane(a, c, d1, d3);
			volume += CalcTetrahedronVolume(C, ac, ab, b);
			volume += CalcTetrahedronVolume(C, b, c, ac);

			if(vertSum != nullptr)
				*vertSum = ac + ab + b + b + c + ac;
			if(vertCount != nullptr)
				*vertCount = 6;
			if(clippedVerts != nullptr) {
				clippedVerts->reserve(clippedVerts->size() + 6);
				clippedVerts->push_back(ac);
				clippedVerts->push_back(ab);
				clippedVerts->push_back(b);
				clippedVerts->push_back(b);
				clippedVerts->push_back(c);
				clippedVerts->push_back(ac);
			}
		}
		else {
			auto bc = IntersectLinePlane(b, c, d2, d3);
			volume += CalcTetrahedronVolume(C, b, bc, ab);

			if(vertSum != nullptr)
				*vertSum = b + bc + ab;
			if(vertCount != nullptr)
				*vertCount = 3;
			if(clippedVerts != nullptr) {
				clippedVerts->reserve(clippedVerts->size() + 3);
				clippedVerts->push_back(b);
				clippedVerts->push_back(bc);
				clippedVerts->push_back(ab);
			}
		}
	}
	return volume;
}

double pragma::physics::WaterBuoyancySimulator::CalcClipVolume(Vector3 &C, const Vector3 &a, const Vector3 &b, const Vector3 &c, double d1, double d2, double d3, Vector3 *vertSum, uint32_t *vertCount, std::vector<Vector3> *clippedVerts) const
{
	if(vertSum != nullptr)
		*vertSum = {};
	if(vertCount != nullptr)
		*vertCount = {};

	auto clipVolume = 0.0;
	// a to b crosses the clipping plane
	if(d1 * d2 < 0.0)
		clipVolume = ClipTriangle(C, a, b, c, d1, d2, d3, vertSum, vertCount, clippedVerts);

	// a to c crosses the clipping plane
	else if(d1 * d3 < 0.0)
		clipVolume = ClipTriangle(C, c, a, b, d3, d1, d2, vertSum, vertCount, clippedVerts);

	// b to c crosses the clipping plane
	else if(d2 * d3 < 0.0)
		clipVolume = ClipTriangle(C, b, c, a, d2, d3, d1, vertSum, vertCount, clippedVerts);

	// Full clipping plane intersection; keep the whole triangle
	else if(d1 < 0.0 || d2 < 0.0 || d3 < 0.0) {
		clipVolume = CalcTetrahedronVolume(C, a, b, c);

		if(vertSum != nullptr)
			*vertSum = a + b + c;
		if(vertCount != nullptr)
			*vertCount = 3;
		if(clippedVerts != nullptr) {
			clippedVerts->reserve(clippedVerts->size() + 3);
			clippedVerts->push_back(a);
			clippedVerts->push_back(b);
			clippedVerts->push_back(c);
		}
	}
	return clipVolume;
}

Vector3 pragma::physics::WaterBuoyancySimulator::CalcCattoDragLinearForceApproximation(double dragCoefficientHz, double mass, double submergedLiquidVolume, double volume, const Vector3 &liquidVel, const Vector3 &velSubmergedVolume) const
{
	return static_cast<float>(dragCoefficientHz * mass * (submergedLiquidVolume / volume)) * (liquidVel - velSubmergedVolume);
}

Vector3 pragma::physics::WaterBuoyancySimulator::CalcCattoDragTorqueForceApproximation(double dragCoefficientHz, double mass, double submergedLiquidVolume, double volume, double lenPolyhedron, const Vector3 &bodyAngularVelocity) const
{
	return static_cast<float>(dragCoefficientHz * mass * (submergedLiquidVolume / volume) * umath::pow2(lenPolyhedron)) * -bodyAngularVelocity;
}
