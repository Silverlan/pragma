#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>

static const auto VERTEX_EPSILON = 0.001f;

#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLNETWORK Vertex
{
	Vertex(const Vector3 &position,const Vector2 &uv,const Vector3 &normal,const Vector3 &tangent,const Vector3 &biTangent);
	Vertex(const Vector3 &position,const Vector2 &uv,const Vector3 &normal);
	Vertex(const Vector3 &position,const Vector3 &normal);
	Vertex();
	bool operator==(const Vertex &other) const;
	bool operator!=(const Vertex &other) const;
	Vector3 position = {};
	Vector2 uv = {};
	Vector3 normal = {};
	Vector3 tangent = {};
	Vector3 biTangent = {};
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
