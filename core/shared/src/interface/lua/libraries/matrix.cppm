// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.matrix;

export import pragma.lua;

#define LUA_MATRIX_MEMBERS_DEC(type)                                                                                                                                                                                                                                                             \
	namespace Mat##type                                                                                                                                                                                                                                                                          \
	{                                                                                                                                                                                                                                                                                            \
		DLLNETWORK void Copy(lua::State * l, const ::Mat##type &mat);                                                                                                                                                                                                                            \
		DLLNETWORK void Get(lua::State * l, const ::Mat##type &mat, int k, int m);                                                                                                                                                                                                               \
		DLLNETWORK void Set(lua::State * l, ::Mat##type & mat, int k, int m, float f);                                                                                                                                                                                                           \
		DLLNETWORK void Transpose(lua::State * l, ::Mat##type & mat);                                                                                                                                                                                                                            \
		DLLNETWORK void GetTransposition(lua::State * l, const ::Mat##type &mat);                                                                                                                                                                                                                \
	}

#define LUA_MATRIX_MEMBERS_DEC_INVERSE(type)                                                                                                                                                                                                                                                     \
	LUA_MATRIX_MEMBERS_DEC(type);                                                                                                                                                                                                                                                                \
	namespace Mat##type                                                                                                                                                                                                                                                                          \
	{                                                                                                                                                                                                                                                                                            \
		DLLNETWORK void Inverse(lua::State * l, ::Mat##type & mat);                                                                                                                                                                                                                              \
		DLLNETWORK void GetInverse(lua::State * l, const ::Mat##type &mat);                                                                                                                                                                                                                      \
	}

export namespace Lua {
	LUA_MATRIX_MEMBERS_DEC_INVERSE(2);
	LUA_MATRIX_MEMBERS_DEC(2x3);
	LUA_MATRIX_MEMBERS_DEC(2x4);
	LUA_MATRIX_MEMBERS_DEC_INVERSE(3);
	LUA_MATRIX_MEMBERS_DEC(3x2);
	LUA_MATRIX_MEMBERS_DEC(3x4);
	LUA_MATRIX_MEMBERS_DEC_INVERSE(4);
	LUA_MATRIX_MEMBERS_DEC(4x2);
	LUA_MATRIX_MEMBERS_DEC(4x3);
	namespace matrix {
		DLLNETWORK ::Mat4 create_orthogonal_matrix(float left, float right, float bottom, float top, float zNear, float zFar);
		DLLNETWORK ::Mat4 create_perspective_matrix(float fov, float aspectRatio, float zNear, float zFar);
		DLLNETWORK ::Mat3 calc_covariance_matrix(lua::State *l, luabind::table<> points);
		DLLNETWORK ::Mat3 calc_covariance_matrix(lua::State *l, luabind::table<> points, const Vector3 &avg);
	};
	namespace Mat4 {
		DLLNETWORK void Translate(lua::State *l, ::Mat4 &mat, const Vector3 &pos);
		DLLNETWORK void Rotate(lua::State *l, ::Mat4 &mat, const EulerAngles &ang);
		DLLNETWORK void Rotate(lua::State *l, ::Mat4 &mat, const Vector3 &axis, float ang);
		DLLNETWORK void Scale(lua::State *l, ::Mat4 &mat, const Vector3 &scale);
		DLLNETWORK void ToEulerAngles(lua::State *l, const ::Mat4 &mat);
		DLLNETWORK void ToQuaternion(lua::State *l, const ::Mat4 &mat);
		DLLNETWORK void Decompose(lua::State *l, const ::Mat4 &mat);
	};
	namespace Mat2 {
		DLLNETWORK void Set(lua::State *l, ::Mat2 &mat, float x1, float y1, float x2, float y2);
		DLLNETWORK void Set(lua::State *l, ::Mat2 &mat, const ::Mat2 &mat2);
	};
	namespace Mat2x3 {
		DLLNETWORK void Set(lua::State *l, ::Mat2x3 &mat, float x1, float y1, float z1, float x2, float y2, float z2);
		DLLNETWORK void Set(lua::State *l, ::Mat2x3 &mat, const ::Mat2x3 &mat2);
	};
	namespace Mat2x4 {
		DLLNETWORK void Set(lua::State *l, ::Mat2x4 &mat, float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2);
		DLLNETWORK void Set(lua::State *l, ::Mat2x4 &mat, const ::Mat2x4 &mat2);
	};
	namespace Mat3x2 {
		DLLNETWORK void Set(lua::State *l, ::Mat3x2 &mat, float x1, float y1, float x2, float y2, float x3, float y3);
		DLLNETWORK void Set(lua::State *l, ::Mat3x2 &mat, const ::Mat3x2 &mat2);
	};
	namespace Mat3 {
		DLLNETWORK void Set(lua::State *l, ::Mat3 &mat, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);
		DLLNETWORK void Set(lua::State *l, ::Mat3 &mat, const ::Mat3 &mat2);
		DLLNETWORK void CalcEigenValues(lua::State *l, const ::Mat3 &mat);
	};
	namespace Mat3x4 {
		DLLNETWORK void Set(lua::State *l, ::Mat3x4 &mat, float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float x3, float y3, float z3, float w3);
		DLLNETWORK void Set(lua::State *l, ::Mat3x4 &mat, const ::Mat3x4 &mat2);
	};
	namespace Mat4x2 {
		DLLNETWORK void Set(lua::State *l, ::Mat4x2 &mat, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
		DLLNETWORK void Set(lua::State *l, ::Mat4x2 &mat, const ::Mat4x2 &mat2);
	};
	namespace Mat4x3 {
		DLLNETWORK void Set(lua::State *l, ::Mat4x3 &mat, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4);
		DLLNETWORK void Set(lua::State *l, ::Mat4x3 &mat, const ::Mat4x3 &mat2);
	};
	namespace Mat4 {
		DLLNETWORK void Set(lua::State *l, ::Mat4 &mat, float x1, float y1, float z1, float w1, float x2, float y2, float z2, float w2, float x3, float y3, float z3, float w3, float x4, float y4, float z4, float w4);
		DLLNETWORK void Set(lua::State *l, ::Mat4 &mat, const ::Mat4 &mat2);
	};
};
