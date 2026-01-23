// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.vector;

pragma::math::Vertex Lua::Vertex::Copy(pragma::math::Vertex &v) { return v; }
pragma::math::VertexWeight Lua::VertexWeight::Copy(pragma::math::VertexWeight &vw) { return vw; }
Vector3i Lua::Vectori::Copy(Vector3i &v) { return v; }
Vector2i Lua::Vector2i::Copy(::Vector2i &v) { return v; }
Vector4i Lua::Vector4i::Copy(::Vector4i &v) { return v; }

void Lua::Vector::Normalize(Vector3 &vec) { uvec::normalize(&vec); }

static const Vector3 vPitch(1, 0, 0);
static const Vector3 vYaw(0, 1, 0);
static const Vector3 vRoll(0, 0, 1);
void Lua::Vector::Rotate(lua::State *, Vector3 &vec, const EulerAngles &ang) { uvec::rotate(&vec, ang); }
void Lua::Vector::Rotate(lua::State *, Vector3 &vec, const Vector3 &normal, float angle) { vec = glm::gtc::rotate(vec, angle, normal); }
void Lua::Vector::Rotate(lua::State *, Vector3 &vec, const Quat &orientation) { uvec::rotate(&vec, orientation); }
void Lua::Vector::RotateAround(lua::State *, Vector3 &vec, const Vector3 &origin, const EulerAngles &ang) { uvec::rotate_around(&vec, ang, origin); }
Vector3 Lua::Vector::Copy(lua::State *l, const Vector3 &vec) { return vec; }
Vector3 Lua::Vector::Lerp(lua::State *l, const Vector3 &vec, const Vector3 &vecB, float factor) { return uvec::lerp(vec, vecB, factor); }

void Lua::Vector::Set(lua::State *, Vector3 &vec, const Vector3 &vecB)
{
	vec.x = vecB.x;
	vec.y = vecB.y;
	vec.z = vecB.z;
}

void Lua::Vector::Set(lua::State *, Vector3 &vec, float x, float y, float z)
{
	vec.x = x;
	vec.y = y;
	vec.z = z;
}

Mat4 Lua::Vector::ToMatrix(lua::State *l, const Vector3 &vec) { return glm::gtc::translate(umat::identity(), vec); }

void Lua::Vector::SnapToGrid(lua::State *, Vector3 &vec, UInt32 gridSize) { uvec::snap_to_grid(vec, gridSize); }
void Lua::Vector::SnapToGrid(lua::State *l, Vector3 &vec) { SnapToGrid(l, vec, 1); }

////////////////////////

Vector2 Lua::Vector2::GetNormal(const ::Vector2 &vec)
{
	Vector3 n(vec.x, vec.y, 0);
	uvec::normalize(&n);
	return ::Vector2(n.x, n.y);
}
void Lua::Vector2::Normalize(::Vector2 &vec)
{
	Vector3 n(vec.x, vec.y, 0);
	uvec::normalize(&n);
	vec.x = n.x;
	vec.y = n.y;
}
float Lua::Vector2::Length(const ::Vector2 &vec) { return uvec::length(Vector3(vec.x, vec.y, 0)); }
float Lua::Vector2::LengthSqr(const ::Vector2 &vec) { return uvec::length_sqr(Vector3(vec.x, vec.y, 0)); }
float Lua::Vector2::Distance(const ::Vector2 &a, const ::Vector2 &b)
{
	Vector3 v = Vector3(b.x, b.y, 0) - Vector3(a.x, a.y, 0);
	return uvec::length(v);
}
float Lua::Vector2::DistanceSqr(const ::Vector2 &a, const ::Vector2 &b)
{
	Vector3 v = Vector3(b.x, b.y, 0) - Vector3(a.x, a.y, 0);
	return uvec::length_sqr(v);
}
Vector2 Lua::Vector2::Cross(lua::State *l, const ::Vector2 &a, const ::Vector2 &b)
{
	Vector3 cross = glm::cross(Vector3(a.x, a.y, 0), Vector3(b.x, b.y, 0));
	return ::Vector2(cross.x, cross.y);
}
float Lua::Vector2::DotProduct(lua::State *l, const ::Vector2 &a, const ::Vector2 &b) { return glm::dot(Vector3(a.x, a.y, 0), Vector3(b.x, b.y, 0)); }
void Lua::Vector2::Rotate(lua::State *, ::Vector2 &vec, const EulerAngles &ang)
{
	Vector3 v = Vector3(vec.x, vec.y, 0);
	uvec::rotate(&v, ang);
	vec.x = v.x;
	vec.y = v.y;
}
void Lua::Vector2::RotateAround(lua::State *, ::Vector2 &vec, const ::Vector2 &origin, const EulerAngles &ang)
{
	Vector3 v(vec.x, vec.y, 0);
	Vector3 vB = Vector3(origin.x, origin.y, 0);
	uvec::rotate_around(&v, ang, vB);
	vec.x = v.x;
	vec.y = v.y;
}
Vector2 Lua::Vector2::Copy(lua::State *l, const ::Vector2 &vec) { return ::Vector2(vec.x, vec.y); }
Vector2 Lua::Vector2::Lerp(lua::State *l, const ::Vector2 &vec, const ::Vector2 &vecB, float factor)
{
	Vector3 v = uvec::lerp(Vector3(vec.x, vec.y, 0), Vector3(vecB.x, vecB.y, 0), factor);
	return ::Vector2 {v.x, v.y};
}
void Lua::Vector2::Set(lua::State *, ::Vector2 &vec, const ::Vector2 &vecB)
{
	vec.x = vecB.x;
	vec.y = vecB.y;
}
void Lua::Vector2::Set(lua::State *, ::Vector2 &vec, float x, float y)
{
	vec.x = x;
	vec.y = y;
}
Vector2 Lua::Vector2::Project(lua::State *l, const ::Vector2 &vec, const ::Vector2 &n) { return glm::gtx::proj(vec, n); }

////////////////////////

Vector4 Lua::Vector4::GetNormal(const ::Vector4 &vec) { return glm::normalize(vec); }
void Lua::Vector4::Normalize(::Vector4 &vec) { vec = glm::normalize(vec); }
float Lua::Vector4::Length(const ::Vector4 &vec) { return glm::length(vec); }
float Lua::Vector4::LengthSqr(const ::Vector4 &vec) { return glm::gtx::length2(vec); }
float Lua::Vector4::Distance(const ::Vector4 &a, const ::Vector4 &b) { return glm::distance(a, b); }
float Lua::Vector4::DistanceSqr(const ::Vector4 &a, const ::Vector4 &b) { return glm::gtx::distance2(a, b); }
float Lua::Vector4::DotProduct(lua::State *l, const ::Vector4 &a, const ::Vector4 &b) { return glm::dot(a, b); }
Vector4 Lua::Vector4::Copy(lua::State *l, const ::Vector4 &vec) { return ::Vector4(vec.x, vec.y, vec.z, vec.w); }
Vector4 Lua::Vector4::Lerp(lua::State *l, const ::Vector4 &vec, const ::Vector4 &vecB, float factor) { return vec + (vecB - vec) * factor; }
void Lua::Vector4::Set(lua::State *, ::Vector4 &vec, const ::Vector4 &vecB)
{
	vec.w = vecB.w;
	vec.x = vecB.x;
	vec.y = vecB.y;
	vec.z = vecB.z;
}
void Lua::Vector4::Set(lua::State *, ::Vector4 &vec, float x, float y, float z, float w)
{
	vec.w = w;
	vec.x = x;
	vec.y = y;
	vec.z = z;
}
Vector4 Lua::Vector4::Project(lua::State *l, const ::Vector4 &vec, const ::Vector4 &n) { return glm::gtx::proj(vec, n); }

////////////////////////

template<class TVector>
static void to_min_max(TVector &inOutA, TVector &inOutB)
{
	auto numComponents = inOutA.length();
	for(auto i = decltype(numComponents) {0}; i < numComponents; ++i) {
		if(inOutB[i] < inOutA[i]) {
			auto t = inOutA[i];
			inOutA[i] = inOutB[i];
			inOutB[i] = t;
		}
	}
}

template<class TVector>
static void to_min_max(TVector &inOutA, TVector &inOutB, const TVector &c)
{
	auto numComponents = inOutA.length();
	for(auto i = decltype(numComponents) {0}; i < numComponents; ++i) {
		if(c[i] < inOutA[i])
			inOutA[i] = c[i];
		if(c[i] > inOutB[i])
			inOutB[i] = c[i];
	}
}

void Lua::vector::to_min_max(::Vector2 &inOutA, ::Vector2 &inOutB) { ::to_min_max(inOutA, inOutB); }
void Lua::vector::to_min_max(Vector3 &inOutA, Vector3 &inOutB) { ::to_min_max(inOutA, inOutB); }
void Lua::vector::to_min_max(::Vector4 &inOutA, ::Vector4 &inOutB) { ::to_min_max(inOutA, inOutB); }
void Lua::vector::to_min_max(::Vector2 &inOutA, ::Vector2 &inOutB, const ::Vector2 &c) { ::to_min_max(inOutA, inOutB, c); }
void Lua::vector::to_min_max(Vector3 &inOutA, Vector3 &inOutB, const Vector3 &c) { ::to_min_max(inOutA, inOutB, c); }
void Lua::vector::to_min_max(::Vector4 &inOutA, ::Vector4 &inOutB, const ::Vector4 &c) { ::to_min_max(inOutA, inOutB, c); }

template<class TVector>
static void get_min_max(lua::State *l, luabind::table<> t, TVector &outMin, TVector &outMax)
{
	auto numEls = Lua::GetObjectLength(l, 1);
	if(numEls == 0) {
		outMin = {};
		outMax = {};
		return;
	}
	for(auto i = decltype(outMin.length()) {0u}; i < outMin.length(); ++i) {
		outMin[i] = std::numeric_limits<float>::max();
		outMax[i] = std::numeric_limits<float>::lowest();
	}
	for(auto it = luabind::iterator {t}; it != luabind::iterator {}; ++it) {
		auto val = luabind::object_cast_nothrow<TVector>(*it, TVector {});
		for(auto i = decltype(outMin.length()) {0u}; i < outMin.length(); ++i) {
			outMin[i] = pragma::math::min(outMin[i], val[i]);
			outMax[i] = pragma::math::min(outMax[i], val[i]);
		}
	}
}

void Lua::vector::get_min_max(lua::State *l, luabind::table<> t, ::Vector2 &outMin, ::Vector2 &outMax) { return ::get_min_max<::Vector2>(l, t, outMin, outMax); }
void Lua::vector::get_min_max(lua::State *l, luabind::table<> t, Vector3 &outMin, Vector3 &outMax) { return ::get_min_max<Vector3>(l, t, outMin, outMax); }
void Lua::vector::get_min_max(lua::State *l, luabind::table<> t, ::Vector4 &outMin, ::Vector4 &outMax) { return ::get_min_max<::Vector4>(l, t, outMin, outMax); }

Vector2 Lua::vector::random_2d()
{
	float azimuth = pragma::math::random(0.f, 2.f) * static_cast<float>(pragma::math::pi);
	return ::Vector2(std::cos(azimuth), std::sin(azimuth));
}

Vector3 Lua::vector::calc_average(luabind::table<> points)
{
	Vector3 avg {};
	uint32_t numPoints = 0;
	for(auto it = luabind::iterator {points}; it != luabind::iterator {}; ++it) {
		auto val = luabind::object_cast_nothrow<Vector3>(*it, Vector3 {});
		avg += val;
		++numPoints;
	}
	avg /= static_cast<float>(numPoints);
	return avg;
}

void Lua::vector::calc_best_fitting_plane(luabind::table<> points, float ang, Vector3 &outNormal, double &outDistance)
{
	auto avg = calc_average(points);
	auto mat = umat::create_from_axis_angle(avg, ang);

	pragma::math::calc_best_fitting_plane(mat, avg, outNormal, outDistance);
}
