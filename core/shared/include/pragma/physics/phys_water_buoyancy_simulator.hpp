/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PHYS_WATER_BUOYANCY_SIMULATOR_HPP__
#define __PHYS_WATER_BUOYANCY_SIMULATOR_HPP__

#include "pragma/networkdefinitions.h"

class BaseEntity;
class PhysWaterSurfaceSimulator;
struct PhysLiquid;
namespace pragma::physics {
	class DLLNETWORK WaterBuoyancySimulator {
	  public:
		WaterBuoyancySimulator();
		template<class InputItVert, class InputItIndex>
		double CalcBuoyancy(const Quat &rorigin, const PhysLiquid &liquid, const Vector3 &waterPlane, double waterPlaneDist, const Vector3 &waterPlaneRelObj, double waterPlaneDistRelObj, const Vector3 &waterVelocity, double gravity, InputItVert vertsBegin, InputItIndex indicesBegin,
		  InputItIndex indicesEnd, double mass, double volBody, const Vector3 &bodyVelocity, const Vector3 &bodyAngularVelocity, Vector3 *force = nullptr, Vector3 *torque = nullptr, Vector3 *optOutSubmergedCenter = nullptr) const;
		// Not safe to call, unless particle heights of surface simulator were locked!
		void Simulate(BaseEntity &entWater, const PhysLiquid &liquid, BaseEntity &ent, Vector3 waterPlane, double waterPlaneDist, const Vector3 &waterVelocity, const PhysWaterSurfaceSimulator *surfaceSim = nullptr) const;
	  protected:
		Vector3 CalcBuoyancy(double liquidDensity, double submergedVolume, double gravity, const Vector3 &liquidUpVec) const;
		Vector3 CalcTorque(const Quat &rorigin, const Vector3 &centerBody, const Vector3 &centerSubmergedVolume, double liquidDensity, double submergedVolume, double gravity, const Vector3 &liquidUpVec) const;

		Vector3 IntersectLinePlane(const Vector3 &a, const Vector3 &b, double da, double db) const;
		double CalcTetrahedronVolume(Vector3 &c, const Vector3 &u, const Vector3 &v, const Vector3 &w) const;
		double CalcDistanceFromVertexToPlane(const Vector3 &v, const Vector3 &p, double d) const;
		double ClipTriangle(Vector3 &C, const Vector3 &a, const Vector3 &b, const Vector3 &c, double d1, double d2, double d3, Vector3 *vertSum = nullptr, uint32_t *vertCount = nullptr, std::vector<Vector3> *clippedVerts = nullptr) const;
		double CalcClipVolume(Vector3 &C, const Vector3 &a, const Vector3 &b, const Vector3 &c, double d1, double d2, double d3, Vector3 *vertSum = nullptr, uint32_t *vertCount = nullptr, std::vector<Vector3> *clippedVerts = nullptr) const;

		Vector3 CalcCattoDragLinearForceApproximation(double dragCoefficientHz, double mass, double submergedLiquidVolume, double volume, const Vector3 &liquidVel, const Vector3 &velSubmergedVolume) const;
		Vector3 CalcCattoDragTorqueForceApproximation(double dragCoefficientHz, double mass, double submergedLiquidVolume, double volume, double lenPolyhedron, const Vector3 &bodyAngularVelocity) const;
	};
};

#endif
