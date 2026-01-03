// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :model.vertex_buffer_data;

pragma::rendering::VertexBufferData::VertexBufferData(const Vector3 &position, const Vector2 &uv, const Vector3 &normal, const Vector3 &tangent, const Vector3 &biTangent) : position {position}, uv {uv}, normal {normal}, tangent {tangent}, biTangent {biTangent} {}
pragma::rendering::VertexBufferData::VertexBufferData(const Vector3 &position, const Vector2 &uv, const Vector3 &normal) : VertexBufferData {position, uv, normal, {}, {}} {}
pragma::rendering::VertexBufferData::VertexBufferData(const Vector3 &position, const Vector3 &normal) : VertexBufferData {position, Vector2 {}, normal} {}
pragma::rendering::VertexBufferData::VertexBufferData() : VertexBufferData {Vector3 {}, Vector3 {}} {}
pragma::rendering::VertexBufferData::VertexBufferData(const VertexBufferData &other) : VertexBufferData {other.position, other.uv, other.normal, other.tangent, other.biTangent} {}
pragma::rendering::VertexBufferData::VertexBufferData(const math::Vertex &vertex) : VertexBufferData {vertex.position, vertex.uv, vertex.normal, Vector3 {vertex.tangent}, vertex.GetBiTangent()} {}
