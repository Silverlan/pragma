// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BRUSHMESH_H_
#define __BRUSHMESH_H_
#include "pragma/networkdefinitions.h"
#include <vector>

#include "pragma/model/side.h"
#include "pragma/physics/shape.hpp"
#include <mathutil/uvec.h>
#include <pragma/math/intersection.h>
#include <pragma/console/conout.h>

class DLLNETWORK BrushMesh {
	friend Con::c_cout &operator<<(Con::c_cout &, const BrushMesh &);
	friend std::ostream &operator<<(std::ostream &, const BrushMesh &);
  public:
	BrushMesh();
  private:
	std::vector<std::shared_ptr<Side>> m_sides;
	Vector3 m_min;
	Vector3 m_max;
	bool m_bConvex;
	std::shared_ptr<pragma::physics::IShape> m_shape;
	void UpdateHullShape(pragma::physics::IEnvironment &env, const std::vector<SurfaceMaterial> *surfaceMaterials = nullptr);
  public:
	bool IsConvex();
	void SetConvex(bool b);
	std::shared_ptr<pragma::physics::IShape> GetShape();
	void Optimize();
	virtual void AddSide(std::shared_ptr<Side> side);
	const std::vector<std::shared_ptr<Side>> &GetSides() const;
	std::vector<std::shared_ptr<Side>> &GetSides();
	bool IntersectAABB(Vector3 *min, Vector3 *max) const;
	bool IntersectAABB(const Vector3 &pos, const Vector3 &posNew, const Vector3 &extents, const Vector3 &posObj, float *entryTime, float *exitTime, Vector3 *hitnormal = nullptr) const;
	void Calculate(pragma::physics::IEnvironment &env, const std::vector<SurfaceMaterial> *surfaceMaterials = nullptr);
	void GetBounds(Vector3 *min, Vector3 *max) const;
	bool PointInside(Vector3 &p, double epsilon = 0);
};

#endif // __BRUSHMESH_H_
