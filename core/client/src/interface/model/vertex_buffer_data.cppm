// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:model.vertex_buffer_data;

export import pragma.math;
import pragma.prosper;

export namespace pragma::rendering {
	constexpr auto VERTEX_BUFER_DATA_POSITION_FORMAT = prosper::Format::R32G32B32_SFloat;
	constexpr auto VERTEX_BUFER_DATA_UV_FORMAT = prosper::Format::R32G32_SFloat;
	constexpr auto VERTEX_BUFER_DATA_NORMAL_FORMAT = prosper::Format::R32G32B32_SFloat;
	constexpr auto VERTEX_BUFER_DATA_TANGENT_FORMAT = prosper::Format::R32G32B32A32_SFloat;
	// constexpr auto VERTEX_BUFER_DATA_BI_TANGENT_FORMAT = prosper::Format::R32G32B32_SFloat;
}

#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
#pragma pack(push, 1)
export namespace pragma::rendering {
	struct DLLCLIENT VertexBufferData {
		VertexBufferData(const Vector3 &position, const Vector2 &uv, const Vector3 &normal, const Vector4 &tangent);
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
			Vector4 tangent; // w-component is handedness
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
	using VertexBufferData = math::Vertex;
};
#endif
