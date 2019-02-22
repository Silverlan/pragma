#include "stdafx_shared.h"
#include "pragma/model/vertex.h"

std::ostream &operator<<(std::ostream &out,const Vertex &v)
{
	out<<"Vertex["<<v.position<<"]["<<v.normal<<"]["<<v.uv<<"]";
	return out;
}

std::ostream &operator<<(std::ostream &out,const VertexWeight &v)
{
	out<<"VertexWeight["<<v.boneIds<<"]["<<v.weights<<"]";
	return out;
}

///////////////////////////////

Vertex::Vertex(const Vector3 &_position,const Vector2 &_uv,const Vector3 &_normal,const Vector3 &_tangent,const Vector3 &_biTangent)
	: position(_position),uv(_uv),normal(_normal),tangent(_tangent),biTangent(_biTangent)
{}

Vertex::Vertex(const Vector3 &position,const Vector2 &uv,const Vector3 &normal)
	: Vertex(position,uv,normal,Vector3{},Vector3{})
{}

Vertex::Vertex(const Vector3 &position,const Vector3 &normal)
	: Vertex(position,Vector2(0.f,0.f),normal)
{}

Vertex::Vertex()
	: Vertex(Vector3{0.f,0.f,0.f},Vector3{0.f,0.f,0.f})
{}

bool Vertex::operator==(const Vertex &other) const
{
	return (
		umath::abs(position.x -other.position.x) <= VERTEX_EPSILON && umath::abs(position.y -other.position.y) <= VERTEX_EPSILON && umath::abs(position.z -other.position.z) <= VERTEX_EPSILON &&
		umath::abs(uv.x -other.uv.x) <= VERTEX_EPSILON && umath::abs(uv.y -other.uv.y) <= VERTEX_EPSILON &&
		umath::abs(normal.x -other.normal.x) <= VERTEX_EPSILON && umath::abs(normal.y -other.normal.y) <= VERTEX_EPSILON && umath::abs(normal.z -other.normal.z) <= VERTEX_EPSILON
	) ? true : false;
}
bool Vertex::operator!=(const Vertex &other) const
{
	return (*this == other) ? false : true;
}

///////////////////////////////

VertexWeight::VertexWeight(const Vector4i &_boneIds,const Vector4 &_weights)
	: boneIds(_boneIds),weights(_weights)
{}

VertexWeight::VertexWeight()
	: VertexWeight(Vector4i{-1,-1,-1,-1},Vector4{0.f,0.f,0.f,0.f})
{}

bool VertexWeight::operator==(const VertexWeight &other) const
{
	return (boneIds == other.boneIds &&
		umath::abs(weights.x -other.weights.x) <= VERTEX_EPSILON && umath::abs(weights.y -other.weights.y) <= VERTEX_EPSILON && umath::abs(weights.z -other.weights.z) <= VERTEX_EPSILON && umath::abs(weights.w -other.weights.w) <= VERTEX_EPSILON
	) ? true : false;
}
bool VertexWeight::operator!=(const VertexWeight &other) const
{
	return (*this == other) ? false : true;
}
