// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:model.vertex_buffer_data;

export import pragma.math;

#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
#pragma pack(push, 1)
export namespace pragma::rendering {
	struct DLLCLIENT VertexBufferData {
		VertexBufferData(const Vector3 &position, const Vector2 &uv, const Vector3 &normal, const Vector3 &tangent, const Vector3 &biTangent);
		VertexBufferData(const Vector3 &position, const Vector2 &uv, const Vector3 &normal);
		VertexBufferData(const Vector3 &position, const Vector3 &normal);
		VertexBufferData();
		VertexBufferData(const VertexBufferData &other);
		VertexBufferData(const math::Vertex &vertex);
		union {
			Vector3 position;
			Vector4 paddedPosition;
		};
		union {
			Vector3 normal;
			Vector4 paddedNormal;
		};
		union {
			Vector3 tangent;
			Vector4 paddedTangent;
		};
		union {
			Vector3 biTangent;
			Vector4 paddedBiTangent;
		};
		union {
			Vector2 uv;
			Vector4 paddedUv;
		};
	};
}
#pragma pack(pop)
#else
// No alignment required
export namespace pragma::rendering {
	using VertexBufferData = Vertex;
};
#endif
