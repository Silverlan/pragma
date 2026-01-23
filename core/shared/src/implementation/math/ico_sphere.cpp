/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright (c) 2009 Andreas Kahler
*/
module;

module pragma.shared;

import :math.ico_sphere;

// Source: http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html

static uint32_t addVertex(std::vector<Vector3> &verts, const Vector3 &p)
{
	auto n = uvec::get_normal(p);
	verts.push_back(n);
	return static_cast<uint32_t>(verts.size()) - 1;
}
static uint32_t get_middle_point(uint32_t p1, uint32_t p2, std::vector<Vector3> &verts, std::unordered_map<int64_t, uint32_t> &middlePointIndexCache)
{
	auto firstIsSmaller = (p1 < p2) ? true : false;
	int64_t smallerIndex = (firstIsSmaller == true) ? p1 : p2;
	int64_t greaterIndex = (firstIsSmaller == true) ? p2 : p1;
	int64_t key = (smallerIndex << 32) + greaterIndex;
	auto it = middlePointIndexCache.find(key);
	if(it != middlePointIndexCache.end())
		return it->second;
	auto &point1 = verts[p1];
	auto &point2 = verts[p2];
	auto middle = Vector3((point1.x + point2.x) / 2.f, (point1.y + point2.y) / 2.f, (point1.z + point2.z) / 2.f);
	auto i = addVertex(verts, middle);
	middlePointIndexCache.insert(std::remove_reference_t<decltype(middlePointIndexCache)>::value_type(key, i));
	return i;
}
void pragma::math::IcoSphere::Create(const Vector3 &origin, float radius, std::vector<Vector3> &verts, std::vector<uint16_t> &indices, uint32_t recursionLevel)
{
	auto t = (1.0 + sqrt(5.0)) / 2.0;
	verts = {Vector3(-1, t, 0), Vector3(1, t, 0), Vector3(-1, -t, 0), Vector3(1, -t, 0),

	  Vector3(0, -1, t), Vector3(0, 1, t), Vector3(0, -1, -t), Vector3(0, 1, -t),

	  Vector3(t, 0, -1), Vector3(t, 0, 1), Vector3(-t, 0, -1), Vector3(-t, 0, 1)};
	for(auto &v : verts)
		uvec::normalize(&v);

	auto numFacesTotal = static_cast<uint32_t>(20 * (pow(static_cast<uint32_t>(4), recursionLevel)) * 3);
	auto &faces = indices;
	std::vector<uint32_t> faces2;
	faces.reserve(numFacesTotal);
	faces2.reserve(numFacesTotal);
	faces.push_back(0);
	faces.push_back(11);
	faces.push_back(5);
	faces.push_back(0);
	faces.push_back(5);
	faces.push_back(1);
	faces.push_back(0);
	faces.push_back(1);
	faces.push_back(7);
	faces.push_back(0);
	faces.push_back(7);
	faces.push_back(10);
	faces.push_back(0);
	faces.push_back(10);
	faces.push_back(11);

	faces.push_back(1);
	faces.push_back(5);
	faces.push_back(9);
	faces.push_back(5);
	faces.push_back(11);
	faces.push_back(4);
	faces.push_back(11);
	faces.push_back(10);
	faces.push_back(2);
	faces.push_back(10);
	faces.push_back(7);
	faces.push_back(6);
	faces.push_back(7);
	faces.push_back(1);
	faces.push_back(8);

	faces.push_back(3);
	faces.push_back(9);
	faces.push_back(4);
	faces.push_back(3);
	faces.push_back(4);
	faces.push_back(2);
	faces.push_back(3);
	faces.push_back(2);
	faces.push_back(6);
	faces.push_back(3);
	faces.push_back(6);
	faces.push_back(8);
	faces.push_back(3);
	faces.push_back(8);
	faces.push_back(9);

	faces.push_back(4);
	faces.push_back(9);
	faces.push_back(5);
	faces.push_back(2);
	faces.push_back(4);
	faces.push_back(11);
	faces.push_back(6);
	faces.push_back(2);
	faces.push_back(10);
	faces.push_back(8);
	faces.push_back(6);
	faces.push_back(7);
	faces.push_back(9);
	faces.push_back(8);
	faces.push_back(1);

	std::unordered_map<int64_t, uint32_t> middlePointCacheIndex;
	for(auto i = decltype(recursionLevel) {0}; i < recursionLevel; ++i) {
		faces2.clear();
		for(auto i = decltype(faces.size()) {0}; i < faces.size(); i += 3) {
			auto va = get_middle_point(faces[i], faces[i + 1], verts, middlePointCacheIndex);
			auto vb = get_middle_point(faces[i + 1], faces[i + 2], verts, middlePointCacheIndex);
			auto vc = get_middle_point(faces[i + 2], faces[i], verts, middlePointCacheIndex);

			faces2.push_back(faces[i]);
			faces2.push_back(va);
			faces2.push_back(vc);
			faces2.push_back(faces[i + 1]);
			faces2.push_back(vb);
			faces2.push_back(va);
			faces2.push_back(faces[i + 2]);
			faces2.push_back(vc);
			faces2.push_back(vb);
			faces2.push_back(va);
			faces2.push_back(vb);
			faces2.push_back(vc);
		}
		faces.clear();
		for(auto &ti : faces2)
			faces.push_back(ti);
	}
	for(auto &v : verts)
		v = origin + v * radius;
}

void pragma::math::IcoSphere::Create(const Vector3 &origin, float radius, std::vector<Vector3> &verts, uint32_t recursionLevel)
{
	std::vector<Vector3> localVerts;
	std::vector<uint16_t> indices;
	Create(origin, radius, localVerts, indices, recursionLevel);
	verts.reserve(indices.size());
	for(auto idx : indices)
		verts.push_back(localVerts[idx]);
}
