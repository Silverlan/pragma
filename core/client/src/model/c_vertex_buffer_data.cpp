// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <mathutil/vertex.hpp>
#include <pragma/types.hpp>

VertexBufferData::VertexBufferData(const Vector3 &position, const Vector2 &uv, const Vector3 &normal, const Vector3 &tangent, const Vector3 &biTangent) : position {position}, uv {uv}, normal {normal}, tangent {tangent}, biTangent {biTangent} {}
VertexBufferData::VertexBufferData(const Vector3 &position, const Vector2 &uv, const Vector3 &normal) : VertexBufferData {position, uv, normal, {}, {}} {}
VertexBufferData::VertexBufferData(const Vector3 &position, const Vector3 &normal) : VertexBufferData {position, Vector2 {}, normal} {}
VertexBufferData::VertexBufferData() : VertexBufferData {Vector3 {}, Vector3 {}} {}
VertexBufferData::VertexBufferData(const VertexBufferData &other) : VertexBufferData {other.position, other.uv, other.normal, other.tangent, other.biTangent} {}
VertexBufferData::VertexBufferData(const umath::Vertex &vertex) : VertexBufferData {vertex.position, vertex.uv, vertex.normal, Vector3 {vertex.tangent}, vertex.GetBiTangent()} {}
