/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <iostream>

static const auto VERTEX_EPSILON = 0.001f;

#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLNETWORK Vertex
{
	Vertex(const Vector3 &position,const Vector2 &uv,const Vector3 &normal,const Vector4 &tangent);
	Vertex(const Vector3 &position,const Vector2 &uv,const Vector3 &normal);
	Vertex(const Vector3 &position,const Vector3 &normal);
	Vertex();
	bool Equal(const Vertex &other,float epsilon) const;
	bool operator==(const Vertex &other) const;
	bool operator!=(const Vertex &other) const;
	Vector3 position = {};
	Vector2 uv = {};
	Vector3 normal = {};
	Vector4 tangent = {}; // w-component is handedness
	Vector3 GetBiTangent() const;
};

struct DLLNETWORK VertexWeight
{
	VertexWeight(const Vector4i &boneIds,const Vector4 &weights);
	VertexWeight();
	bool operator==(const VertexWeight &other) const;
	bool operator!=(const VertexWeight &other) const;
	Vector4i boneIds = {};
	Vector4 weights = {};
};
#pragma warning(pop)

DLLNETWORK std::ostream &operator<<(std::ostream &out,const Vertex &v);
DLLNETWORK std::ostream &operator<<(std::ostream &out,const VertexWeight &v);

#endif
