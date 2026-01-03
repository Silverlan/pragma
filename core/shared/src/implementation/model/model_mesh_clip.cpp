// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.model_mesh;

static Vector3 intersect_line_plane(const Vector3 &a, const Vector3 &b, float da, float db) { return a + da / (da - db) * (b - a); }

static Vector2 calc_barycentric_coordinates(const pragma::math::Vertex &a, const pragma::math::Vertex &b, const pragma::math::Vertex &c, const Vector3 &va, const Vector3 &vb, const Vector3 &vc, const Vector3 &hitPoint)
{
	Vector2 uv;
	pragma::math::geometry::calc_barycentric_coordinates(va, a.uv, vb, b.uv, vc, c.uv, hitPoint, uv.x, uv.y);
	return uv;
}

static void clip_triangle(const std::function<void(uint32_t)> &fAddTriangleIndex, const std::function<void(uint16_t, uint16_t, const Vector3 &, const Vector2 &)> &fAddNewVertex, const Vector3 &C, const std::vector<pragma::math::Vertex> &verts, const std::array<uint32_t, 3> &indices,
  const std::array<double, 3> &distances)
{
	const auto d0 = distances.at(0);
	const auto d1 = distances.at(1);
	const auto d2 = distances.at(2);

	auto idx0 = indices.at(0);
	auto idx1 = indices.at(1);
	auto idx2 = indices.at(2);

	const auto &v0 = verts.at(idx0);
	const auto &v0Pos = v0.position;

	const auto &v1 = verts.at(idx1);
	const auto &v1Pos = v1.position;

	const auto &v2 = verts.at(idx2);
	const auto &v2Pos = v2.position;

	auto ab = intersect_line_plane(v0Pos, v1Pos, d0, d1);
	auto uvAb = calc_barycentric_coordinates(v0, v1, v2, v0Pos, v1Pos, v2Pos, ab);

	if(d0 < 0.0) {
		if(d2 < 0.0) {
			auto bc = intersect_line_plane(v1Pos, v2Pos, d1, d2);
			auto uvBc = calc_barycentric_coordinates(v0, v1, v2, v0Pos, v1Pos, v2Pos, bc);

			fAddNewVertex(idx1, idx2, bc, uvBc);
			fAddTriangleIndex(indices.at(2));
			fAddTriangleIndex(indices.at(0));
			fAddNewVertex(idx0, idx1, ab, uvAb);
			fAddNewVertex(idx1, idx2, bc, uvBc);
			fAddTriangleIndex(indices.at(0));
		}
		else {
			auto ac = intersect_line_plane(v0Pos, v2Pos, d0, d2);
			auto uvAc = calc_barycentric_coordinates(v0, v1, v2, v0Pos, v1Pos, v2Pos, ac);

			fAddTriangleIndex(indices.at(0));
			fAddNewVertex(idx0, idx1, ab, uvAb);
			fAddNewVertex(idx0, idx2, ac, uvAc);
		}
	}
	else {
		if(d2 < 0.0) {
			auto ac = intersect_line_plane(v0Pos, v2Pos, d0, d2);
			auto uvAc = calc_barycentric_coordinates(v0, v1, v2, v0Pos, v1Pos, v2Pos, ac);

			fAddNewVertex(idx0, idx2, ac, uvAc);
			fAddNewVertex(idx0, idx1, ab, uvAb);
			fAddTriangleIndex(indices.at(1));
			fAddTriangleIndex(indices.at(1));
			fAddTriangleIndex(indices.at(2));
			fAddNewVertex(idx0, idx2, ac, uvAc);
		}
		else {
			auto bc = intersect_line_plane(v1Pos, v2Pos, d1, d2);
			auto uvBc = calc_barycentric_coordinates(v0, v1, v2, v0Pos, v1Pos, v2Pos, bc);

			fAddTriangleIndex(indices.at(1));
			fAddNewVertex(idx1, idx2, bc, uvBc);
			fAddNewVertex(idx0, idx1, ab, uvAb);
		}
	}
}

void pragma::geometry::ModelSubMesh::ClipAgainstPlane(const Vector3 &n, double d, ModelSubMesh &clippedMesh, const std::vector<Mat4> *boneMatrices, ModelSubMesh *clippedCoverMesh)
{
	const auto &verts = GetVertices();
	const auto &alphas = GetAlphas();
	const auto &vertexWeights = GetVertexWeights();
	auto bUseWeights = vertexWeights.size() >= verts.size();
	auto fGetVertexPosition = std::function<const Vector3 &(uint32_t)>([&verts](uint32_t vertexIndex) -> const Vector3 & { return verts.at(vertexIndex).position; });
	auto fApplyVertexBoneTransformations = [boneMatrices](const Vector3 &v, const math::VertexWeight &vw) -> Vector3 {
		auto m = Mat4 {0.f};
		for(auto i = 0u; i < 4u; ++i) {
			auto boneId = vw.boneIds[i];
			if(boneId == -1 || boneId >= boneMatrices->size())
				continue;
			auto weight = vw.weights[i];
			m += weight * (*boneMatrices)[boneId];
		}
		auto v4 = m * Vector4 {v.x, v.y, v.z, 1.f};
		return Vector3 {v4.x, v4.y, v4.z};
	};

	std::vector<Vector3> weightedVerts {};
	if(boneMatrices != nullptr && bUseWeights == true) {
		weightedVerts.reserve(verts.size());
		for(auto i = decltype(verts.size()) {0u}; i < verts.size(); ++i)
			weightedVerts.push_back(fApplyVertexBoneTransformations(verts.at(i).position, vertexWeights.at(i)));
		fGetVertexPosition = [&weightedVerts](uint32_t vertexIndex) -> const Vector3 & { return weightedVerts.at(vertexIndex); };
	}

	auto &clipMeshVerts = clippedMesh.GetVertices();
	auto &clippedAlphas = clippedMesh.GetAlphas();
	auto &clippedVertexWeights = clippedMesh.GetVertexWeights();
	clipMeshVerts.reserve(verts.size());
	clippedAlphas.reserve(alphas.size());
	clippedMesh.ReserveIndices(GetIndexCount());
	clippedVertexWeights.reserve(clipMeshVerts.size());

	std::vector<uint32_t> originalVertexIndicesToClippedVertexIndices(verts.size(), std::numeric_limits<uint32_t>::max());
	const auto fAddTriangleIndex = [bUseWeights, &originalVertexIndicesToClippedVertexIndices, &verts, &alphas, &clipMeshVerts, &clippedAlphas, &clippedMesh, &clippedVertexWeights, &vertexWeights](uint16_t idx) {
		if(originalVertexIndicesToClippedVertexIndices.at(idx) == std::numeric_limits<uint32_t>::max()) {
			originalVertexIndicesToClippedVertexIndices.at(idx) = clipMeshVerts.size();
			clipMeshVerts.push_back(verts.at(idx));
			if(bUseWeights)
				clippedVertexWeights.push_back(vertexWeights.at(idx));
			if(idx < alphas.size())
				clippedAlphas.push_back(alphas.at(idx));
		}
		clippedMesh.AddIndex(originalVertexIndicesToClippedVertexIndices.at(idx));
	};
	struct PointData {
		math::Vertex vertex;
		math::VertexWeight weight;
	};
	using BoneId = int32_t;
	std::vector<PointData> newPoints {};
	const auto fAddNewVertex = [bUseWeights, &fApplyVertexBoneTransformations, boneMatrices, &vertexWeights, &clippedVertexWeights, &verts, &alphas, &clippedMesh, &clipMeshVerts, &clippedAlphas, &newPoints](uint16_t idx0, uint16_t idx1, const Vector3 &v, const Vector2 &uv) {
		clippedMesh.AddIndex(clipMeshVerts.size());
		clipMeshVerts.push_back({v, uv, (verts.at(idx0).normal + verts.at(idx1).normal) / 2.f});

		if(bUseWeights) {
			const auto &vw0 = vertexWeights.at(idx0);
			const auto &vw1 = vertexWeights.at(idx1);

			std::unordered_map<animation::BoneId, float> weightMap {};
			for(auto i = 0u; i < 4u; ++i) {
				for(auto &vw : {vw0, vw1}) {
					if(vw.boneIds[i] != -1) {
						auto it = weightMap.find(vw.boneIds[i]);
						if(it == weightMap.end())
							weightMap.insert(std::make_pair(vw.boneIds[i], vw.weights[i]));
						else
							it->second = (it->second + vw.weights[i]) / 2.f;
					}
				}
			}
			std::array<std::pair<animation::BoneId, float>, 4> weights {std::pair<animation::BoneId, float> {-1, 0.f}};
			auto weightSum = 0.f;
			auto weightIdx = 0u;
			for(auto &pair : weightMap) {
				weights.at(weightIdx) = pair;
				weightSum += pair.second;
				if(weights.size() == 4u)
					break;
			}
			// Make sure weights add up to 1
			if(weightSum != 0.f) {
				for(auto &w : weights)
					w.second /= weightSum;
			}
			clippedVertexWeights.push_back({});
			auto &vwClipped = clippedVertexWeights.back();
			for(auto i = 0; i < weights.size(); ++i) {
				auto &w = weights.at(i);
				vwClipped.boneIds[i] = w.first;
				vwClipped.weights[i] = w.second;
			}
			vwClipped = vertexWeights.at(idx0); // TODO

			if(boneMatrices != nullptr)
				fApplyVertexBoneTransformations(clipMeshVerts.back().position, vwClipped);
		}

		newPoints.push_back({clipMeshVerts.back(), bUseWeights ? clippedVertexWeights.back() : math::VertexWeight {}});

		if(idx0 < alphas.size() && idx1 < alphas.size()) {
			auto &a0 = alphas.at(idx0);
			auto &a1 = alphas.at(idx1);
			clippedAlphas.push_back((a0 + a1) / 2.f);
		}
	};

	auto C = n * static_cast<float>(d);
	VisitIndices([&](auto *indexData, uint32_t numIndices) {
		for(auto i = decltype(numIndices) {0}; i < numIndices; i += 3) {
			auto idx0 = indexData[i];
			auto idx1 = indexData[i + 1];
			auto idx2 = indexData[i + 2];

			auto &v0 = fGetVertexPosition(idx0);
			auto &v1 = fGetVertexPosition(idx1);
			auto &v2 = fGetVertexPosition(idx2);

			auto d0 = uvec::dot(v0, n) - d;
			auto d1 = uvec::dot(v1, n) - d;
			auto d2 = uvec::dot(v2, n) - d;

			// a to b crosses the clipping plane
			if(d0 * d1 < 0.0)
				clip_triangle(fAddTriangleIndex, fAddNewVertex, C, verts, {idx0, idx1, idx2}, {d0, d1, d2});
			// a to c crosses the clipping plane
			else if(d0 * d2 < 0.0)
				clip_triangle(fAddTriangleIndex, fAddNewVertex, C, verts, {idx2, idx0, idx1}, {d2, d0, d1});
			// b to c crosses the clipping plane
			else if(d1 * d2 < 0.0)
				clip_triangle(fAddTriangleIndex, fAddNewVertex, C, verts, {idx1, idx2, idx0}, {d1, d2, d0});
			// Full clipping plane intersection; keep the whole triangle
			else if(d0 < 0.0 || d1 < 0.0 || d2 < 0.0) {
				fAddTriangleIndex(idx0);
				fAddTriangleIndex(idx1);
				fAddTriangleIndex(idx2);
			}
		}
	});

	clippedMesh.m_skinTextureIndex = m_skinTextureIndex;
	clippedMesh.m_center = m_center;
	clippedMesh.m_numAlphas = m_numAlphas;
	clippedMesh.m_min = m_min;
	clippedMesh.m_max = m_max;

	// Calculate new mesh to fill clipping hole
	if(newPoints.empty() == false) {
		// Remove all duplicate positions
		Vector3 center {};
		for(auto it = newPoints.begin(); it != newPoints.end();) {
			auto &p = *it;
			auto itOther = std::find_if(it + 1, newPoints.end(), [&p](const PointData &pOther) { return uvec::cmp(p.vertex.position, pOther.vertex.position); });
			if(itOther == newPoints.end()) {
				center += it->vertex.position;
				++it;
				continue;
			}
			it = newPoints.erase(it);
		}
		center /= static_cast<float>(newPoints.size());
		newPoints.push_back({});
		newPoints.back().vertex.position = center;                           // TODO: UV, etc.
		newPoints.back().weight = newPoints.at(newPoints.size() - 2).weight; // TODO
		auto centerIdx = newPoints.size() - 1;

		std::vector<Vector2> points2d;
		points2d.reserve(newPoints.size());
		auto rot = math::geometry::calc_rotation_between_planes(n, uvec::PRM_UP);
		for(auto &v : newPoints) {
			auto p = v.vertex.position;
			p = p * rot;
			points2d.push_back({p.x, p.z}); // All points lie on plane pointing upwards, so y has to be 0
		}

		std::vector<uint16_t> newTriangles {};
		auto outlineIndices = math::geometry::get_outline_vertices(points2d);
		if(outlineIndices.has_value()) {
			newTriangles.reserve((outlineIndices->size() - 1) * 3);
			for(auto i = decltype(outlineIndices->size()) {0u}; i < outlineIndices->size(); ++i) {
				newTriangles.push_back(outlineIndices->at(i));
				newTriangles.push_back(outlineIndices->at((i + 1) % outlineIndices->size()));
				newTriangles.push_back(centerIdx);
			}
		}

		std::stringstream ss {};
		ss << "local points = {\n";
		for(auto &p : points2d)
			ss << "\tmath.Vector2(" << p.x << "," << p.y << "),\n";
		ss << "}\n";

		ss << "local outlineIndices = {\n";
		for(auto idx : *outlineIndices)
			ss << "\t" << idx << "\n";
		ss << "}\n";

		ss << "local tris = {\n";
		for(auto idx : newTriangles)
			ss << "\t" << idx << "\n";
		ss << "}\n";
		Con::COUT << ss.str() << Con::endl;

		if(/*b == true &&*/ newTriangles.empty() == false) {
			auto fCalcVertexWeights = [&newPoints](const Vector3 &pos) -> math::VertexWeight {
				std::vector<float> pointWeights {};
				pointWeights.reserve(newPoints.size());
				auto weightSum = 0.f;
				for(auto &p : newPoints) {
					auto d = uvec::distance(pos, p.vertex.position);
					pointWeights.push_back(d);
					weightSum += d;
				}
				for(auto &w : pointWeights)
					w = 1.f - (w / weightSum);

				using WeightCount = uint32_t;
				std::unordered_map<animation::BoneId, std::pair<float, WeightCount>> weights {};
				weights.reserve(newPoints.size() * 4);
				for(auto i = decltype(newPoints.size()) {0u}; i < newPoints.size(); ++i) {
					auto &p = newPoints.at(i);
					auto w = p.weight;
					w.weights *= pointWeights.at(i);
					for(auto j = 0; j < 4; ++j) {
						auto boneIdx = w.boneIds[j];
						if(boneIdx == -1)
							continue;
						auto it = weights.find(boneIdx);
						if(it == weights.end())
							it = weights.insert(std::make_pair(boneIdx, std::make_pair(0.f, 0u))).first;
						it->second.first += w.weights[j];
						++it->second.second;
					}
				}
				using Weight = float;
				std::vector<std::pair<animation::BoneId, Weight>> sortedWeights {};
				sortedWeights.reserve(weights.size());
				for(auto &pair : weights)
					sortedWeights.push_back({pair.first, pair.second.first / static_cast<float>(pair.second.second)});
				std::sort(sortedWeights.begin(), sortedWeights.end(), [](const std::pair<animation::BoneId, Weight> &a, const std::pair<animation::BoneId, Weight> &b) { return a.second > b.second; });

				math::VertexWeight r {};
				auto num = math::min(sortedWeights.size(), static_cast<size_t>(4));
				weightSum = 0.f;
				for(auto i = decltype(num) {0u}; i < num; ++i) {
					auto &w = sortedWeights.at(i);
					r.boneIds[i] = w.first;
					r.weights[i] = w.second;
					weightSum += w.second;
				}
				if(weightSum != 0.f) {
					for(auto i = decltype(num) {0u}; i < num; ++i)
						r.weights[i] /= weightSum;
				}
				return r;
			};

			auto &p0 = newPoints.at(newTriangles.at(0));
			auto &p1 = newPoints.at(newTriangles.at(1));
			auto &p2 = newPoints.at(newTriangles.at(2));
			auto nTriangulated = uvec::calc_face_normal(p0.vertex.position, p1.vertex.position, p2.vertex.position);
			auto d = uvec::dot(n, nTriangulated);
			if(d < 0.0) // Wrong orientation; Triangles have to be flipped
			{
				for(auto i = decltype(newTriangles.size()) {0}; i < newTriangles.size(); i += 3)
					math::swap(newTriangles.at(i), newTriangles.at(i + 2));
			}

			auto *pCoverVerts = &clipMeshVerts;
			auto *coverMesh = &clippedMesh;
			auto *pCoverVertexWeights = &clippedVertexWeights;
			if(clippedCoverMesh != nullptr) {
				pCoverVerts = &clippedCoverMesh->GetVertices();
				coverMesh = clippedCoverMesh;
				pCoverVertexWeights = &clippedCoverMesh->GetVertexWeights();
			}

			pCoverVerts->reserve(pCoverVerts->size() + newTriangles.size());
			coverMesh->ReserveIndices(coverMesh->GetIndexCount() + newTriangles.size());
			pCoverVertexWeights->reserve(pCoverVertexWeights->size() + newTriangles.size());
			auto triangleIdx = coverMesh->GetIndexCount();
			for(auto i = decltype(newTriangles.size()) {0u}; i < newTriangles.size(); i += 3) {
				std::array<uint16_t, 3> indices = {newTriangles.at(i), newTriangles.at(i + 1), newTriangles.at(i + 2)};
				auto &p0 = newPoints.at(indices.at(0));
				auto &p1 = newPoints.at(indices.at(1));
				auto &p2 = newPoints.at(indices.at(2));
				for(auto idx : indices) {
					auto v = newPoints.at(idx);
					//v.weight = fCalcVertexWeights(v.vertex.position);
					//v.weight.boneIds = {-1,-1,-1,-1};
					//v.weight.weights = {0.f,0.f,0.f,0.f};

					v.vertex.normal = nTriangulated;
					v.vertex.uv = {};
					math::geometry::calc_barycentric_coordinates(p0.vertex.position, p1.vertex.position, p2.vertex.position, v.vertex.position, v.vertex.uv.x, v.vertex.uv.y);

					pCoverVerts->push_back(v.vertex);
					coverMesh->AddIndex(pCoverVerts->size() - 1u);
					pCoverVertexWeights->push_back(v.weight);

					/*Vector2 uv0 {};
					Vector2 uv1 {};
					Vector2 uv2 {};
					Geometry::calc_barycentric_coordinates(p0.position,p0.uv,p1.position,p1.uv,p2.position,p2.uv,p0.position,uv0.x,uv0.y);
					Geometry::calc_barycentric_coordinates(p0.position,p0.uv,p1.position,p1.uv,p2.position,p2.uv,p1.position,uv1.x,uv1.y);
					Geometry::calc_barycentric_coordinates(p0.position,p0.uv,p1.position,p1.uv,p2.position,p2.uv,p2.position,uv2.x,uv2.y);
					*/
				}
			}
		}
	}
	//
}

void pragma::geometry::ModelSubMesh::ClipAgainstPlane(const Vector3 &n, double d, ModelSubMesh &clippedMeshA, ModelSubMesh &clippedMeshB, const std::vector<Mat4> *boneMatrices, ModelSubMesh *clippedCoverMeshA, ModelSubMesh *clippedCoverMeshB)
{
	ClipAgainstPlane(n, d, clippedMeshA, boneMatrices, clippedCoverMeshA);
	ClipAgainstPlane(-n, -d, clippedMeshB, boneMatrices, clippedCoverMeshB);
}
