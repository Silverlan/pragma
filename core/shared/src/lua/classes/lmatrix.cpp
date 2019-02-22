#include "stdafx_shared.h"
#include "pragma/lua/libraries/lmatrix.h"
#include <pragma/engine.h>
#include "luasystem.h"
#include <mathutil/umat.h>
#include "pragma/lua/classes/ldef_vector.h"
#include <pragma/math/vector/wvvector3.h>
#include <mathutil/glmutil.h>
#include <pragma/math/angle/wvquaternion.h>
#include "pragma/lua/classes/ldef_mat4.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <Eigen/Eigenvalues>

extern DLLENGINE Engine *engine;
#define LUA_MATRIX_MEMBERS_DEF(type,maxk,maxm) \
	void Lua::Mat##type::Copy(lua_State *l,const ::Mat##type &mat) \
	{ \
		::Mat##type matCopy(mat); \
		luabind::object(l,matCopy).push(l); \
	} \
	void Lua::Mat##type::Get(lua_State *l,const ::Mat##type &mat,int k,int m) \
	{ \
		if(k < 0 || m < 0 || k >= maxk || m >= maxm) \
			return; \
		Lua::PushNumber(l,mat[k][m]); \
	} \
	void Lua::Mat##type::Set(lua_State*,::Mat##type &mat,int k,int m,float f) \
	{ \
		if(k < 0 || m < 0 || k >= maxk || m >= maxm) \
			return; \
		mat[k][m] = f; \
	} \
	void Lua::Mat##type::Transpose(lua_State*,::Mat##type &mat) \
	{ \
		::Mat##maxm##x##maxk transpose = glm::transpose(mat); \
		for(unsigned int i=0;i<maxk;i++) \
		{ \
			for(unsigned int j=0;j<maxm;j++) \
				mat[j][i] = transpose[i][j]; \
		} \
	} \
	void Lua::Mat##type::GetTransposition(lua_State *l,const ::Mat##type &mat) \
	{ \
		::Mat##maxm##x##maxk transpose = glm::transpose(mat); \
		luabind::object(l,transpose).push(l); \
	}

#define LUA_MATRIX_MEMBERS_DEF_INVERSE(type,maxk,maxm) \
	LUA_MATRIX_MEMBERS_DEF(type,maxk,maxm); \
	void Lua::Mat##type::Inverse(lua_State*,::Mat##type &mat) \
	{ \
		mat = glm::inverse(mat); \
	} \
	void Lua::Mat##type::GetInverse(lua_State *l,const ::Mat##type &mat) \
	{ \
		::Mat##type inv = glm::inverse(mat); \
		luabind::object(l,inv).push(l); \
	}

LUA_MATRIX_MEMBERS_DEF_INVERSE(2,2,2);
LUA_MATRIX_MEMBERS_DEF(2x3,2,3);
LUA_MATRIX_MEMBERS_DEF(2x4,2,4);
LUA_MATRIX_MEMBERS_DEF_INVERSE(3,3,3);
LUA_MATRIX_MEMBERS_DEF(3x2,3,2);
LUA_MATRIX_MEMBERS_DEF(3x4,3,4);
LUA_MATRIX_MEMBERS_DEF_INVERSE(4,4,4);
LUA_MATRIX_MEMBERS_DEF(4x2,4,2);
LUA_MATRIX_MEMBERS_DEF(4x3,4,3);

void Lua::Mat4::Translate(lua_State*,::Mat4 &mat,const Vector3 &pos)
{
	mat = glm::translate(mat,pos);
}

void Lua::Mat4::Rotate(lua_State*,::Mat4 &mat,const EulerAngles &ang)
{
	mat = glm::rotate(mat,CFloat(umath::deg_to_rad(ang.p)),uvec::FORWARD);
	mat = glm::rotate(mat,CFloat(umath::deg_to_rad(ang.y)),uvec::UP);
	mat = glm::rotate(mat,CFloat(umath::deg_to_rad(ang.r)),uvec::RIGHT);
}
void Lua::Mat4::Rotate(lua_State*,::Mat4 &mat,const Vector3 &axis,float ang) {mat = glm::rotate(mat,ang,axis);}

void Lua::Mat4::Scale(lua_State*,::Mat4 &mat,const Vector3 &scale) {mat = glm::scale(mat,scale);}

void Lua::Mat4::ToEulerAngles(lua_State *l,const ::Mat4 &mat)
{
	Quat q(mat);
	EulerAngles ang(q);
	luabind::object(l,ang).push(l);
}

void Lua::Mat4::ToQuaternion(lua_State *l,const ::Mat4 &mat)
{
	Lua::Push<Quat>(l,glm::toQuat(mat));
	//luabind::object(l,uquat::create(*mat)).push(l);
}

void Lua::Mat4::Decompose(lua_State *l,const ::Mat4 &mat)
{
	Vector3 scale;
	Quat rotation;
	Vector3 translation;
	Vector3 skew;
	Vector4 perspective;
	glm::decompose(mat,scale,rotation,translation,skew,perspective);
	Lua::Push<Vector3>(l,scale);
	Lua::Push<Quat>(l,glm::conjugate(rotation));
	Lua::Push<Vector3>(l,translation);
	Lua::Push<Vector3>(l,skew);
	Lua::Push<Vector4>(l,perspective);
}

int Lua::matrix::create_from_axis_angle(lua_State *l)
{
	const Vector3 &v = *_lua_Vector_check(l,1);
	float ang = Lua::CheckNumber<float>(l,2);
	::Mat4 m = umat::create_from_axis_angle(v,ang);
	Lua::Push<::Mat4>(l,m);
	return 1;
}

int Lua::matrix::create_from_axes(lua_State *l)
{
	auto *v1 = Lua::CheckVector(l,1);
	auto *v2 = Lua::CheckVector(l,2);
	auto *v3 = Lua::CheckVector(l,3);
	Lua::Push<::Mat4>(l,umat::create_from_axes(*v1,*v2,*v3));
	return 1;
}

int Lua::matrix::create_orthogonal_matrix(lua_State *l)
{
	float left = Lua::CheckNumber<float>(l,1);
	float right = Lua::CheckNumber<float>(l,2);
	float bottom = Lua::CheckNumber<float>(l,3);
	float top = Lua::CheckNumber<float>(l,4);
	float zNear = Lua::CheckNumber<float>(l,5);
	float zFar = Lua::CheckNumber<float>(l,6);
	auto p = glm::ortho(left,right,bottom,top,zNear,zFar);
	p = glm::scale(p,Vector3(1.f,-1.f,1.f));
	Lua::Push<::Mat4>(l,p);
	return 1;
}

int Lua::matrix::create_perspective_matrix(lua_State *l)
{
	float fov = Lua::CheckNumber<float>(l,1);
	float aspectRatio = Lua::CheckNumber<float>(l,2);
	float zNear = Lua::CheckNumber<float>(l,3);
	float zFar = Lua::CheckNumber<float>(l,4);
	auto p = glm::perspective(fov,aspectRatio,zNear,zFar);
	p = glm::scale(p,Vector3(1.f,-1.f,1.f));
	Lua::Push<::Mat4>(l,p);
	return 1;
}

int Lua::matrix::create_look_at_matrix(lua_State *l)
{
	const Vector3 &pos = *_lua_Vector_check(l,1);
	const Vector3 &target = *_lua_Vector_check(l,2);
	const Vector3 &up = *_lua_Vector_check(l,3);
	Lua::Push<::Mat4>(l,glm::lookAtRH(pos,target,up));
	return 1;
}

int Lua::matrix::calc_covariance_matrix(lua_State *l)
{
	int32_t tPoints = 1;
	Lua::CheckTable(l,tPoints);

	Vector3 avg {};
	if(Lua::IsSet(l,2))
		avg = *Lua::CheckVector(l,2);
	else
	{
		auto numPoints = Lua::GetObjectLength(l,tPoints);
		for(auto i=decltype(numPoints){0};i<numPoints;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,tPoints);
			avg += *Lua::CheckVector(l,-1);
		}
		avg /= static_cast<float>(numPoints);
	}

	auto C = ::Mat3(0.f);
	auto numPoints = Lua::GetObjectLength(l,tPoints);
	for(auto i=decltype(numPoints){0};i<numPoints;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,tPoints);
		auto p = *Lua::CheckVector(l,-1);
		p -= avg;
		C += uvec::calc_outer_product(p,p);
	}
	Lua::Push<::Mat3>(l,C);
	return 1;
}

////////////////////////////////////////

void Lua::Mat3::CalcEigenValues(lua_State *l,const ::Mat3 &mat)
{
	auto &A = reinterpret_cast<const Eigen::Matrix3f&>(mat);

	Eigen::EigenSolver<Eigen::Matrix3f> es(A);

	auto &eigenValues = es.eigenvalues();
	auto eigenVectors = es.eigenvectors();

	auto tEigenValues = Lua::CreateTable(l);
	for(auto i=decltype(eigenValues.size()){0};i<eigenValues.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::PushNumber(l,eigenValues[i].real());
		Lua::SetTableValue(l,tEigenValues);
	}
	auto tEigenVectors = Lua::CreateTable(l);
	for(auto i=decltype(eigenVectors.size()){0};i<eigenVectors.size();++i)
	{
		Lua::PushInt(l,i +1);
		auto v = eigenVectors.col(i);
		Lua::Push<Vector3>(l,Vector3(v[0].real(),v[1].real(),v[2].real()));
		Lua::SetTableValue(l,tEigenVectors);
	}
}

////////////////////////////////////////

void Lua::Mat2::Set(lua_State*,::Mat2 &mat,float x1,float y1,float x2,float y2)
{
	mat[0][0] = x1;
	mat[0][1] = y1;
	mat[1][0] = x2;
	mat[1][1] = y2;
}
void Lua::Mat2x3::Set(lua_State*,::Mat2x3 &mat,float x1,float y1,float z1,float x2,float y2,float z2)
{
	mat[0][0] = x1;
	mat[0][1] = y1;
	mat[0][2] = z1;
	mat[1][0] = x2;
	mat[1][1] = y2;
	mat[1][2] = z2;
}
void Lua::Mat2x4::Set(lua_State*,::Mat2x4 &mat,float x1,float y1,float z1,float w1,float x2,float y2,float z2,float w2)
{
	mat[0][0] = x1;
	mat[0][1] = y1;
	mat[0][2] = z1;
	mat[0][3] = w1;
	mat[1][0] = x2;
	mat[1][1] = y2;
	mat[1][2] = z2;
	mat[1][3] = w2;
}
void Lua::Mat3x2::Set(lua_State*,::Mat3x2 &mat,float x1,float y1,float x2,float y2,float x3,float y3)
{
	mat[0][0] = x1;
	mat[0][1] = y1;
	mat[1][0] = x2;
	mat[1][1] = y2;
	mat[2][0] = x3;
	mat[2][1] = y3;
}
void Lua::Mat3::Set(lua_State*,::Mat3 &mat,float x1,float y1,float z1,float x2,float y2,float z2,float x3,float y3,float z3)
{
	mat[0][0] = x1;
	mat[0][1] = y1;
	mat[0][2] = z1;
	mat[1][0] = x2;
	mat[1][1] = y2;
	mat[1][2] = z2;
	mat[2][0] = x3;
	mat[2][1] = y3;
	mat[2][2] = z3;
}
void Lua::Mat3x4::Set(lua_State*,::Mat3x4 &mat,float x1,float y1,float z1,float w1,float x2,float y2,float z2,float w2,float x3,float y3,float z3,float w3)
{
	mat[0][0] = x1;
	mat[0][1] = y1;
	mat[0][2] = z1;
	mat[0][3] = w1;
	mat[1][0] = x2;
	mat[1][1] = y2;
	mat[1][2] = z2;
	mat[1][3] = w2;
	mat[2][0] = x3;
	mat[2][1] = y3;
	mat[2][2] = z3;
	mat[2][3] = w3;
}
void Lua::Mat4x2::Set(lua_State*,::Mat4x2 &mat,float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4)
{
	mat[0][0] = x1;
	mat[0][1] = y1;
	mat[1][0] = x2;
	mat[1][1] = y2;
	mat[2][0] = x3;
	mat[2][1] = y3;
	mat[3][0] = x4;
	mat[3][1] = y4;
}
void Lua::Mat4x3::Set(lua_State*,::Mat4x3 &mat,float x1,float y1,float z1,float x2,float y2,float z2,float x3,float y3,float z3,float x4,float y4,float z4)
{
	mat[0][0] = x1;
	mat[0][1] = y1;
	mat[0][2] = z1;
	mat[1][0] = x2;
	mat[1][1] = y2;
	mat[1][2] = z2;
	mat[2][0] = x3;
	mat[2][1] = y3;
	mat[2][2] = z3;
	mat[3][0] = x4;
	mat[3][1] = y4;
	mat[3][2] = z4;
}
void Lua::Mat4::Set(lua_State*,::Mat4 &mat,float x1,float y1,float z1,float w1,float x2,float y2,float z2,float w2,float x3,float y3,float z3,float w3,float x4,float y4,float z4,float w4)
{
	mat[0][0] = x1;
	mat[0][1] = y1;
	mat[0][2] = z1;
	mat[0][3] = w1;
	mat[1][0] = x2;
	mat[1][1] = y2;
	mat[1][2] = z2;
	mat[1][3] = w2;
	mat[2][0] = x3;
	mat[2][1] = y3;
	mat[2][2] = z3;
	mat[2][3] = w3;
	mat[3][0] = x4;
	mat[3][1] = y4;
	mat[3][2] = z4;
	mat[3][3] = w4;
}

////////////////////////////////////////

void Lua::Mat2::Set(lua_State*,::Mat2 &mat,const ::Mat2 &mat2)
{
	for(unsigned char i=0;i<2;i++)
	{
		for(unsigned char j=0;j<2;j++)
		{
			mat[i][j] = mat2[i][j];
		}
	}
}
void Lua::Mat2x3::Set(lua_State*,::Mat2x3 &mat,const ::Mat2x3 &mat2)
{
	for(unsigned char i=0;i<2;i++)
	{
		for(unsigned char j=0;j<3;j++)
		{
			mat[i][j] = mat2[i][j];
		}
	}
}
void Lua::Mat2x4::Set(lua_State*,::Mat2x4 &mat,const ::Mat2x4 &mat2)
{
	for(unsigned char i=0;i<2;i++)
	{
		for(unsigned char j=0;j<4;j++)
		{
			mat[i][j] = mat2[i][j];
		}
	}
}
void Lua::Mat3x2::Set(lua_State*,::Mat3x2 &mat,const ::Mat3x2 &mat2)
{
	for(unsigned char i=0;i<3;i++)
	{
		for(unsigned char j=0;j<2;j++)
		{
			mat[i][j] = mat2[i][j];
		}
	}
}
void Lua::Mat3::Set(lua_State*,::Mat3 &mat,const ::Mat3 &mat2)
{
	for(unsigned char i=0;i<3;i++)
	{
		for(unsigned char j=0;j<3;j++)
		{
			mat[i][j] = mat2[i][j];
		}
	}
}
void Lua::Mat3x4::Set(lua_State*,::Mat3x4 &mat,const ::Mat3x4 &mat2)
{
	for(unsigned char i=0;i<3;i++)
	{
		for(unsigned char j=0;j<4;j++)
		{
			mat[i][j] = mat2[i][j];
		}
	}
}
void Lua::Mat4x2::Set(lua_State*,::Mat4x2 &mat,const ::Mat4x2 &mat2)
{
	for(unsigned char i=0;i<4;i++)
	{
		for(unsigned char j=0;j<2;j++)
		{
			mat[i][j] = mat2[i][j];
		}
	}
}
void Lua::Mat4x3::Set(lua_State*,::Mat4x3 &mat,const ::Mat4x3 &mat2)
{
	for(unsigned char i=0;i<4;i++)
	{
		for(unsigned char j=0;j<3;j++)
		{
			mat[i][j] = mat2[i][j];
		}
	}
}
void Lua::Mat4::Set(lua_State*,::Mat4 &mat,const ::Mat4 &mat2)
{
	for(unsigned char i=0;i<4;i++)
	{
		for(unsigned char j=0;j<4;j++)
		{
			mat[i][j] = mat2[i][j];
		}
	}
}
