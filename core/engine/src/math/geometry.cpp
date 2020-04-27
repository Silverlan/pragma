/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_engine.h"
#include "pragma/math/intersection.h"

// Source: http://stackoverflow.com/a/1568551/2482983
double Geometry::calc_volume_of_triangle(const Vector3 &v0,const Vector3 &v1,const Vector3 &v2)
{
	std::array<double,3> v0d = {v0.x,v0.y,v0.z};
	std::array<double,3> v1d = {v1.x,v1.y,v1.z};
	std::array<double,3> v2d = {v2.x,v2.y,v2.z};
	auto v321 = v2d.at(0) *v1d.at(1) *v0d.at(2);
	auto v231 = v1d.at(0) *v2d.at(1) *v0d.at(2);
	auto v312 = v2d.at(0) *v0d.at(1) *v1d.at(2);
	auto v132 = v0d.at(0) *v2d.at(1) *v1d.at(2);
	auto v213 = v1d.at(0) *v0d.at(1) *v2d.at(2);
	auto v123 = v0d.at(0) *v1d.at(1) *v2d.at(2);
	return (1.0 /6.0) *(-v321 +v231 +v312 -v132 -v213 +v123);
}
double Geometry::calc_volume_of_polyhedron(const std::function<bool(const Vector3**,const Vector3**,const Vector3**)> &fGetNextTriangle,Vector3 *centerOfMass)
{
	Vector3 c {};

	const Vector3 *v0;
	const Vector3 *v1;
	const Vector3 *v2;
	auto totalVolume = 0.0;
	std::array<double,3> r = {0.0,0.0,0.0};
	while(fGetNextTriangle(&v0,&v1,&v2) == true)
	{
		auto vol = static_cast<double>((v0->x *v1->y *v2->z -v0->x *v2->y *v1->z -v1->x *v0->y *v2->z +v1->x *v2->y *v0->z +v2->x *v0->y *v1->z -v2->x *v1->y *v0->z) /6.0);
		totalVolume += vol;
		if(centerOfMass != nullptr)
		{
			r.at(0) += ((v0->x +v1->x +v2->x) /4.0) *vol;
			r.at(1) += ((v0->y +v1->y +v2->y) /4.0) *vol;
			r.at(2) += ((v0->z +v1->z +v2->z) /4.0) *vol;
		}
	}
	if(centerOfMass != nullptr)
		*centerOfMass = Vector3(r.at(0) /totalVolume,r.at(1) /totalVolume,r.at(2) /totalVolume);
	return totalVolume;
}
double Geometry::calc_volume_of_polyhedron(const std::vector<Vector3> &verts,const std::vector<uint16_t> &triangles,Vector3 *centerOfMass)
{
	auto i = decltype(triangles.size()){0};
	auto numIndices = (triangles.size() /3) *3;
	return calc_volume_of_polyhedron([&verts,&triangles,i,numIndices](const Vector3 **v0,const Vector3 **v1,const Vector3 **v2) mutable -> bool {
		if(i >= numIndices)
			return false;
		*v0 = &verts.at(triangles.at(i));
		*v1 = &verts.at(triangles.at(i +1));
		*v2 = &verts.at(triangles.at(i +2));
		i += 3;
		return true;
	},centerOfMass);
}
Vector3 Geometry::calc_center_of_mass(const std::vector<Vector3> &verts,const std::vector<uint16_t> &triangles,double *volume)
{
	Vector3 r {};
	auto vol = calc_volume_of_polyhedron(verts,triangles,&r);
	if(volume != nullptr)
		*volume = vol;
	return r;
}

Vector3 Geometry::CalcFaceNormal(const Vector3 &v0,const Vector3 &v1,const Vector3 &v2)
{
	if(uvec::distance_sqr(v1 -v0,v2 -v0) < 0.001)
		return uvec::FORWARD;
	auto n = uvec::cross(v1 -v0,v2 -v0);
	uvec::normalize(&n);
	return n;
}

/*
local function calc_cone_surface_normal(coneCenter,coneDir,coneHeight,pointOnSurface,radiusAtPoint)
	local rot = Quaternion(coneDir,coneDir:GetPerpendicular())
	local rotInv = rot:GetInverse()

	pointOnSurface = pointOnSurface:Copy()
	coneCenter = coneCenter:Copy()

	pointOnSurface:Rotate(rotInv)
	coneCenter:Rotate(rotInv)
	local v = Vector(
		pointOnSurface.x -coneCenter.x,
		pointOnSurface.y -coneCenter.y,
		0
	)
	v:Normalize()

	local r = Vector(
		v.x *coneHeight /radiusAtPoint,
		v.y *coneHeight /radiusAtPoint,
		-radiusAtPoint /coneHeight
	)
	r:Rotate(rot)
	r:Normalize()
	return r
end
*/

static Vector3 calc_cone_surface_normal(Vector3 coneCenter,const Vector3 &coneDir,float coneHeight,Vector3 pointOnSurface,float radiusAtPoint)
{
	auto rot = uquat::create_look_rotation(coneDir,uvec::get_perpendicular(coneDir));
	auto rotInv = uquat::get_inverse(rot);

	uvec::rotate(&pointOnSurface,rotInv);
	uvec::rotate(&coneCenter,rotInv);
	auto v = Vector3{
		pointOnSurface.x -coneCenter.x,
		pointOnSurface.y -coneCenter.y,
		0.f
	};
	auto l = uvec::length(v);
	if(l > 0.f)
		v /= l;

	auto r = Vector3{
		(radiusAtPoint != 0.f) ? (v.x *coneHeight /radiusAtPoint) : 0.f,
		(radiusAtPoint != 0.f) ? (v.y *coneHeight /radiusAtPoint) : 0.f,
		(coneHeight != 0.f) ? (-radiusAtPoint /coneHeight) : 0.f
	};
	uvec::rotate(&r,rot);
	l = uvec::length(r);
	if(l > 0.f)
		r /= l;
	else
		r = -coneDir;
	return r;
}

void Geometry::GenerateTruncatedConeMesh(const Vector3 &origin,float startRadius,const Vector3 &dir,float dist,float endRadius,std::vector<Vector3> &verts,std::vector<uint16_t> *triangles,std::vector<Vector3> *normals,uint32_t segmentCount,bool bAddCaps)
{
	auto rot = uquat::create_look_rotation(dir,uvec::get_perpendicular(dir));

	verts.reserve(verts.size() +4 +segmentCount *2);
	if(triangles != nullptr)
	{
		uint32_t numTriangles = 0;
		if(startRadius != 0.f)
			numTriangles += 3;
		if(endRadius != 0.f)
			numTriangles += 3;
		if(startRadius != 0.f && endRadius != 0.f)
			numTriangles += 6;
		else
			numTriangles += 3;
		triangles->reserve(triangles->size() +segmentCount *numTriangles);
	}

	if(normals != nullptr)
		normals->reserve(verts.capacity());

	auto rad = umath::deg_to_rad(0.f);
	auto stepSize = 360.f /static_cast<float>(segmentCount);

	auto origin0 = origin;
	auto origin1 = origin0 +dir *dist;

	Vector3 vStart = {umath::sin(rad),umath::cos(rad),0.0};
	auto vPrev0 = vStart;
	//uvec::rotate(&vPrev0,rot);
	vPrev0 = origin0 +vPrev0 *static_cast<float>(startRadius);

	auto vPrev1 = vStart;
	//uvec::rotate(&vPrev1,rot);
	vPrev1 = origin1 +vPrev1 *static_cast<float>(endRadius);

	auto idxOrigin0 = verts.size();
	if(bAddCaps == true || startRadius == 0.f)
	{
		verts.push_back(origin0);
		if(normals != nullptr)
			normals->push_back(-dir);
	}
	auto idxOrigin1 = verts.size();
	if(bAddCaps == true || endRadius == 0.f)
	{
		verts.push_back(origin1);
		if(normals != nullptr)
			normals->push_back(dir);
	}

	auto totalDist = dist *(endRadius /(endRadius -startRadius));
	auto sc0 = startRadius /endRadius;
	auto sc1 = 1.f;

	auto idxVPrev0 = verts.size();
	auto idxStart0 = idxVPrev0;
	verts.push_back(vPrev0);
	if(normals != nullptr)
		normals->push_back(calc_cone_surface_normal(origin0,dir,totalDist *sc0,verts.back(),startRadius));

	auto idxVPrev1 = verts.size();
	auto idxStart1 = idxVPrev1;
	verts.push_back(vPrev1);
	if(normals != nullptr)
		normals->push_back(calc_cone_surface_normal(origin1,dir,totalDist *sc1,verts.back(),endRadius));

	auto i = stepSize;
	while(segmentCount > 0)
	{
		rad = umath::deg_to_rad(i);
		Vector3 vBase {umath::sin(rad),umath::cos(rad),0.f};
		uvec::rotate(&vBase,rot);

		decltype(verts.size()) idxV0 = 0;
		if(segmentCount > 1)
		{
			auto v0 = origin0 +vBase *static_cast<float>(startRadius);
			idxV0 = verts.size();
			verts.push_back(v0);
			if(normals != nullptr)
				normals->push_back(calc_cone_surface_normal(origin0,dir,totalDist *sc0,verts.back(),startRadius));
		}
		else
			idxV0 = idxStart0;
		// Cap 0
		if(bAddCaps == true && startRadius != 0.f && triangles != nullptr)
		{
			triangles->push_back(static_cast<uint16_t>(idxVPrev0));
			triangles->push_back(static_cast<uint16_t>(idxV0));
			triangles->push_back(static_cast<uint16_t>(idxOrigin0));
		}
		
		decltype(verts.size()) idxV1 = 0;
		if(segmentCount > 1)
		{
			auto v1 = origin1 +vBase *static_cast<float>(endRadius);
			idxV1 = verts.size();
			verts.push_back(v1);
			if(normals != nullptr)
				normals->push_back(calc_cone_surface_normal(origin1,dir,totalDist *sc1,verts.back(),endRadius));
		}
		else
			idxV1 = idxStart1;
		// Cap 1
		if(bAddCaps == true && endRadius != 0.f && triangles != nullptr)
		{
			triangles->push_back(static_cast<uint16_t>(idxVPrev1));
			triangles->push_back(static_cast<uint16_t>(idxOrigin1));
			triangles->push_back(static_cast<uint16_t>(idxV1));
		}
		
		if(triangles != nullptr)
		{
			if(startRadius == 0.f)
			{
				triangles->push_back(static_cast<uint16_t>(idxVPrev1));
				triangles->push_back(static_cast<uint16_t>(idxV1));
				triangles->push_back(static_cast<uint16_t>(idxOrigin0));
			}
			else if(endRadius == 0.f)
			{
				triangles->push_back(static_cast<uint16_t>(idxVPrev0));
				triangles->push_back(static_cast<uint16_t>(idxOrigin1));
				triangles->push_back(static_cast<uint16_t>(idxV0));
			}
			else
			{
				triangles->push_back(static_cast<uint16_t>(idxV0));
				triangles->push_back(static_cast<uint16_t>(idxVPrev0));
				triangles->push_back(static_cast<uint16_t>(idxV1));

				triangles->push_back(static_cast<uint16_t>(idxV1));
				triangles->push_back(static_cast<uint16_t>(idxVPrev0));
				triangles->push_back(static_cast<uint16_t>(idxVPrev1));
			}
		}

		idxVPrev0 = idxV0;
		idxVPrev1 = idxV1;

		i += stepSize;
		--segmentCount;
	}
}

bool Geometry::calc_barycentric_coordinates(const Vector3 &p0,const Vector3 &p1,const Vector3 &p2,const Vector3 &hitPoint,float &b1,float &b2)
{
	//const auto EPSILON = 0.001f;
	const auto u = p1 - p0;
	const auto v = p2 - p0;
	const auto w = hitPoint - p0;

	const auto vCrossW = uvec::cross(v,w);
	const auto vCrossU = uvec::cross(v,u);

	//if(uvec::dot(vCrossW,vCrossU) < -EPSILON)
	//	return false;

	const auto uCrossW = uvec::cross(u, w);
	const auto uCrossV = uvec::cross(u, v);

	//if(uvec::dot(uCrossW, uCrossV) < -EPSILON)
	//	return false;

	const auto denom = uvec::length(uCrossV);
	const auto r = uvec::length(vCrossW) /denom;
	const auto t = uvec::length(uCrossW) /denom;

	b1 = r;
	b2 = t;
	return true;// ((r <= 1.f) && (t <= 1.f) && (r + t <= 1.f));
}

bool Geometry::calc_barycentric_coordinates(const Vector3 &p0,const Vector2 &uv0,const Vector3 &p1,const Vector2 &uv1,const Vector3 &p2,const Vector2 &uv2,const Vector3 &hitPoint,float &u,float &v)
{
	float b1,b2;
	if(calc_barycentric_coordinates(p0,p1,p2,hitPoint,b1,b2) == false)
		return false;
	const auto b0 = 1.f -b1 -b2;
	u = b0 *uv0.x + b1 *uv1.x + b2 *uv2.x;
	v = b0 *uv0.y + b1 *uv1.y + b2 *uv2.y;
	return true;
}

bool Geometry::calc_barycentric_coordinates(const Vector2 uv0,const Vector2 &uv1,const Vector2 &uv2,const Vector2 &uv,float &a1,float &a2,float &a3)
{
	// See http://answers.unity.com/answers/372156/view.html

	auto a = Geometry::calc_triangle_area(uv0,uv1,uv2,true);
	if(a == 0)
		return false;
	a1 = Geometry::calc_triangle_area(uv1,uv2,uv,true) /a;
	if(a1 < 0)
		return false;
	a2 = Geometry::calc_triangle_area(uv2,uv0,uv,true) /a;
	if(a2 < 0)
		return false;
	a3 = Geometry::calc_triangle_area(uv0,uv1,uv,true) /a;
	if(a3 < 0)
		return false;
	return true;
}

Quat Geometry::calc_rotation_between_planes(const Vector3 &n0,const Vector3 &n1)
{
	auto m = n0 +n1;
	uvec::normalize(&m);
	auto axis = uvec::cross(m,n0);
	auto angle = uvec::dot(m,n0);
	auto q = Quat{angle,axis.x,axis.y,axis.z};
	uquat::normalize(q);
	return q;
}

Geometry::PlaneSide Geometry::get_side_of_point_to_plane(const Vector3 &n,double d,const Vector3 &p)
{
	auto planePos = n *static_cast<float>(d);
	auto dot = uvec::dot(p -planePos,n);
	if(umath::abs(dot) < 0.01f)
		return PlaneSide::OnPlane;
	return dot > 0.f ? PlaneSide::Front : PlaneSide::Back;
}

WindingOrder Geometry::get_triangle_winding_order(const Vector3 &v0,const Vector3 &v1,const Vector3 &v2,const Vector3 &n)
{
	auto w = uvec::dot(n,uvec::cross(v1 -v0,v2 -v0));
	return (w > 0.f) ? WindingOrder::Clockwise : WindingOrder::CounterClockwise;
}
WindingOrder Geometry::get_triangle_winding_order(const Vector2 &v0,const Vector2 &v1,const Vector2 &v2)
{
	return get_triangle_winding_order(Vector3{v0.x,0.f,v0.y},Vector3{v1.x,0.f,v1.y},Vector3{v2.x,0.f,v2.y},uvec::UP);
}
float Geometry::calc_triangle_area(const Vector3 &p0,const Vector3 &p1,const Vector3 &p2)
{
	return uvec::length(uvec::cross(p0-p1, p0-p2));
}
float Geometry::calc_triangle_area(const Vector2 &p0,const Vector2 &p1,const Vector2 &p2,bool keepSign)
{
	auto dArea = ((p1.x -p0.x) *(p2.y -p0.y) -(p2.x -p0.x) *(p1.y -p0.y)) /2.0;
	if(keepSign == false)
		dArea = umath::abs(dArea);
	return dArea;
}

const auto EPSILON = 0.1f;
static inline bool nearly_equal(float v0,float v1)
{
	return umath::abs(v0 -v1) < EPSILON;
}
static inline bool lt(float v0,float v1) {return v0 +EPSILON < v1;}
static inline bool gt(float v0,float v1) {return v0 -EPSILON > v1;}

// Source: https://stackoverflow.com/a/8524921/2482983
Geometry::LineSide Geometry::get_side_of_point_to_line(const Vector2 &a,const Vector2 &b,const Vector2 &c)
{
	if(nearly_equal(b.x -a.x,0)) // vertical line
	{
		if(lt(c.x,b.x))
			return gt(b.y,a.y) ? LineSide::Left : LineSide::Right;
		if(gt(c.x,b.x))
			return gt(b.y,a.y) ? LineSide::Right : LineSide::Left;
		return LineSide::OnLine;
	}
	if(nearly_equal(b.y -a.y,0)) // horizontal line
	{
		if(lt(c.y,b.y))
			return gt(b.x,a.x) ? LineSide::Right : LineSide::Left;
		if(gt(c.y,b.y))
			return gt(b.x,a.x) ? LineSide::Left : LineSide::Right;
		return LineSide::OnLine;
	}
	auto slope = (b.y -a.y) /(b.x -a.x);
	auto yIntercept = a.y -a.x *slope;
	auto cSolution = (slope *c.x) +yIntercept;
	if(slope != 0)
	{
		if(gt(c.y,cSolution))
			return gt(b.x,a.x) ? LineSide::Left : LineSide::Right;
		if(lt(c.y,cSolution))
			return gt(b.x,a.x) ? LineSide::Right : LineSide::Left;
		return LineSide::OnLine;
	}
    return LineSide::OnLine;
}

static bool is_line_on_outline(const std::vector<Vector2> &verts,const Vector2 &v0,const Vector2 &v1)
{
	auto side = Geometry::LineSide::OnLine;
	for(auto &v : verts)
	{
		auto vertSide = Geometry::get_side_of_point_to_line(v0,v1,v);
		if(vertSide == Geometry::LineSide::OnLine)
			continue;
		if(side == Geometry::LineSide::OnLine)
			side = vertSide;
		if(vertSide != side)
			return false;
	}
	return true;
}

std::optional<std::vector<uint32_t>> Geometry::get_outline_vertices(const std::vector<Vector2> &verts)
{
	std::vector<bool> vertexStates(verts.size(),true);
	std::vector<size_t> vertexIndices {};
	vertexIndices.reserve(verts.size());
	for(auto i=decltype(verts.size()){0u};i<verts.size();++i)
	{
		if(vertexStates.at(i) == false)
			continue;
		auto &v0 = verts.at(i);
		for(auto j=i+1;j<verts.size();++j)
		{
			auto &v1 = verts.at(j);
			auto dSqr = glm::distance2(v0,v1);
			if(dSqr < 0.01)
			{
				// Duplicate vertex
				vertexStates.at(j) = false;
			}
		}
		vertexIndices.push_back(i);
	}

	Vector2 center {0.f,0.f};
	for(auto &v : verts)
		center += v;
	center /= static_cast<float>(verts.size());

	std::unordered_map<size_t,size_t> outlineMap {};
	for(auto i=decltype(vertexIndices.size()){0u};i<vertexIndices.size();++i)
	{
		auto idx0 = vertexIndices.at(i);
		auto &v0 = verts.at(idx0);
		for(auto j=i+1;j<vertexIndices.size();++j)
		{
			auto idx1 = vertexIndices.at(j);
			auto &v1 = verts.at(idx1);
			if(is_line_on_outline(verts,v0,v1))
			{
				auto windingOrder = Geometry::get_triangle_winding_order(v0,v1,center);
				uint32_t iSrc,iDst;
				switch(windingOrder)
				{
					case WindingOrder::Clockwise:
						iSrc = idx0;
						iDst = idx1;
						break;
					default:
						iSrc = idx1;
						iDst = idx0;
						break;
				}
				auto it = outlineMap.find(iSrc);
				if(it != outlineMap.end())
				{
					// At least three vertices lie on the same line; Choose the longest path
					auto &vSrc = verts.at(iSrc);
					auto &vDst = verts.at(iDst);
					auto &vDstOld = verts.at(it->second);
					auto l = glm::distance2(vSrc,vDst);
					auto lOld = glm::distance2(vSrc,vDstOld);
					if(l > lOld)
						it->second = iDst;
				}
				else
					outlineMap.insert(std::make_pair(iSrc,iDst));
			}
		}
	}
	std::vector<uint32_t> outlineList {};
	if(outlineMap.empty() == false)
	{
		outlineList.reserve(outlineMap.size() +1);
		auto it = outlineMap.begin();
		outlineList.push_back(it->first);
		outlineList.push_back(it->second);
		while((it=outlineMap.find(it->second)) != outlineMap.end())
		{
			outlineList.push_back(it->second);
			if(it->second == outlineList.front())
				break;
		}
	}
	if(outlineList.size() < 4 || outlineList.back() != outlineList.front()) // If last item does not match first, the outline is not a full loop and something went wrong
		return std::nullopt;
	outlineList.erase(outlineList.end() -1);
	return outlineList;
}
