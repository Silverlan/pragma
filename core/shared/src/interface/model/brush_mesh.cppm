// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <iostream>
#include <vector>

export module pragma.shared:model.brush_mesh;

export import :model.side;

export import :physics.shape;
export import :physics.surface_material;

export class DLLNETWORK BrushMesh {
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
