/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_VERTEX_BUFFER_DATA_HPP__
#define __C_VERTEX_BUFFER_DATA_HPP__

#include "pragma/clientdefinitions.h"
#include <mathutil/uvec.h>

#define ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER

struct Vertex;
#ifdef ENABLE_VERTEX_BUFFER_AS_STORAGE_BUFFER
	#pragma pack(push,1)
	struct DLLCLIENT VertexBufferData
	{
		VertexBufferData(const Vector3 &position,const Vector2 &uv,const Vector3 &normal,const Vector3 &tangent,const Vector3 &biTangent);
		VertexBufferData(const Vector3 &position,const Vector2 &uv,const Vector3 &normal);
		VertexBufferData(const Vector3 &position,const Vector3 &normal);
		VertexBufferData();
		VertexBufferData(const VertexBufferData &other);
		VertexBufferData(const Vertex &vertex);
		union
		{
			Vector3 position;
			Vector4 paddedPosition;
		};
		union
		{
			Vector3 normal;
			Vector4 paddedNormal;
		};
		union
		{
			Vector3 tangent;
			Vector4 paddedTangent;
		};
		union
		{
			Vector3 biTangent;
			Vector4 paddedBiTangent;
		};
		union
		{
			Vector2 uv;
			Vector4 paddedUv;
		};
	};
	#pragma pack(pop)
#else
	#include <vertex.h>
	// No alignment required
	using VertexBufferData = Vertex;
#endif

#endif
