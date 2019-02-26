#include "mdl.h"

EulerAngles import::mdl::util::convert_rotation_matrix_to_degrees(float m0,float m1,float m2,float m3,float m4,float m5,float m8)
{
	EulerAngles r {};
	r.y = -umath::asin(umath::round(m2,6.0));
	auto c = umath::cos(r.y);
	r.y = umath::rad_to_deg(r.y);
	if(umath::abs(c) > 0.005)
	{
		Vector2 translate(
			umath::round(m8,6.0) /c,
			umath::round(-m5,6.0) /c
		);
		r.p = umath::rad_to_deg(umath::atan2(translate.y,translate.x));
		translate.x = umath::round(m0,6.0) /c;
		translate.y = umath::round(-m1,6.0) /c;
		r.r = umath::rad_to_deg(umath::atan2(translate.y,translate.x));
	}
	else
	{
		r.p = 0.f;
		Vector2 translate(
			umath::round(m4,6.0),
			umath::round(m3,6.0)
		);
		r.r = umath::rad_to_deg(umath::atan2(translate.y,translate.x));
	}
	return r;
}

Mat4 import::mdl::util::euler_angles_to_matrix(const EulerAngles &ang)
{
	auto r = Mat4(1.f);
	r = glm::rotate(r,-ang.r,uvec::RIGHT);
	r = glm::rotate(r,-ang.y,uvec::UP);
	r = glm::rotate(r,ang.p,uvec::FORWARD);
	return r;
}

void import::mdl::util::translate_matrix(Mat4 &m,const Vector3 &offset)
{
	m[3][0] = offset.x;
	m[3][1] = offset.y;
	m[3][2] = offset.z;
}

Mat4 import::mdl::util::mul_matrix(const Mat4 &m1,const Mat4 &m2)
{
	auto r = Mat4(1.f);
	for(auto i=0;i<=3;i++)
	{
		for(auto j=0;j<=3;j++)
		{
			auto sum = 0.f;
			for(auto k=0;k<=3;k++)
				sum += m1[i][k] *m2[k][j];
			r[i][j] = sum;
		}
	}
	return r;
}

Vector3 import::mdl::util::get_translation(const Mat4 &m)
{
	return Vector3(m[3][0],m[3][1],m[3][2]);
}

Quat import::mdl::util::get_rotation(const Mat4 &m)
{
	auto q = Quat(1.f,0.f,0.f,0.f);
	auto trace = m[0][0] +m[1][1] +m[2][2];
	if(trace > 0.f)
	{
		auto s = 0.5f /sqrtf(trace +1.f);
		q.w = 0.25f /s;
		q.x = (m[2][1] -m[1][2]) *-s;
		q.y = (m[0][2] -m[2][0]) *s;
		q.z = (m[1][0] -m[0][1]) *s;
	}
	else
	{
		if(m[0][0] > m[1][1] && m[0][0] > m[2][2])
		{
			auto s = 2.f *sqrtf(1.f +m[0][0] -m[1][1] -m[2][2]);
			q.w = (m[2][1] -m[1][2]) /s;
			q.x = 0.25f *-s;
			q.y = (m[0][1] +m[1][0]) /s;
			q.z = (m[0][2] +m[2][0]) /s;
		}
		else if(m[1][1] > m[2][2])
		{
			auto s = 2.f *sqrtf(1.0 +m[1][1] -m[0][0] -m[2][2]);
			q.w = (m[0][2] -m[2][0]) /-s;
			q.x = (m[0][1] +m[1][0]) /s;
			q.y = 0.25f *-s;
			q.z = (m[1][2] +m[2][1]) /-s;
		}
		else
		{
			auto s = 2.f *sqrtf(1.0 +m[2][2] -m[0][0] -m[1][1]);
			q.w = (m[1][0] -m[0][1]) /-s;
			q.x = (m[0][2] +m[2][0]) /s;
			q.y = (m[1][2] +m[2][1]) /-s;
			q.z = 0.25f *-s;
		}
	}
	return q;
}

void import::mdl::util::rotation_to_axis_angle(Quat &rot,Vector3 &axis,float &angle)
{
	angle = 2.f *umath::acos(rot.w);
	auto sqr = sqrtf(1.f -rot.w *rot.w);
	if(sqr == 0.f)
	{
		axis = Vector3(0.f,0.f,0.f);
		return;
	}
	axis = Vector3(
		rot.x /sqr,
		rot.y /sqr,
		rot.z /sqr
	);
}

void import::mdl::util::axis_angle_to_rotation(Vector3 &axis,float angle,Quat &rot)
{
	auto s = umath::sin(angle /2.f);
	if(s == 0.f)
	{
		rot = Quat(1.f,0.f,0.f,0.f);
		return;
	}
	rot = Quat(
		umath::cos(angle /2.f),
		axis.x *s,
		axis.y *s,
		axis.z *s
	);
}

void import::mdl::util::convert_rotation(Quat &rot)
{
	Vector3 axis;
	float angle;
	rotation_to_axis_angle(rot,axis,angle);
	// Equivalent to QuatXYZToXZY in MaxScript
	auto xAxis = axis.x;
	axis.x = -axis.y;
	axis.y = -axis.z;
	axis.z = -xAxis;
	//
	axis_angle_to_rotation(axis,angle,rot);

	//if(umath::abs(rot.w -1.f) < 0.001f && umath::abs(rot.x) < 0.001f && umath::abs(rot.y) < 0.001f && umath::abs(rot.z) < 0.001f)
	//	return; // ??

	// This is to comply to an accidental mistake in axis shift when loading the wrmi-file
	auto w = rot.w;
	auto x = rot.x;
	auto y = rot.y;
	auto z = rot.z;
	rot.x = w;
	rot.y = x;
	rot.z = y;
	rot.w = z;
}

Vector3 import::mdl::util::vectori_transform(const Vector3 &v,const Vector3 &matCol0,const Vector3 &matCol1,const Vector3 &matCol2,const Vector3 &matCol3)
{
	Vector3 r {};
	Vector3 tmp {};

	tmp.x = v.x -matCol3.x;
	tmp.y = v.y -matCol3.y;
	tmp.z = v.z -matCol3.z;

	r.x = tmp.x *matCol0.x +tmp.y *matCol0.y +tmp.z *matCol0.z;
	r.y = tmp.x *matCol1.x +tmp.y *matCol1.y +tmp.z *matCol1.z;
	r.z = tmp.x *matCol2.x +tmp.y *matCol2.y +tmp.z *matCol2.z;
	return r;
}
Vector3 import::mdl::util::transform_physics_vertex(const std::shared_ptr<mdl::Bone> &bone,const Vector3 &v,bool sourcePhyIsCol)
{
	const auto scale = 1.0 /0.0254;
	Vector3 r {};
	if(sourcePhyIsCol == true)
	{
		//r.x = scale *v.z;
		//r.y = scale *-v.x;
		//r.z = scale *-v.y;
		r.x = scale *v.z;
		r.y = scale *-v.x;
		r.z = scale *v.y;
	}
	else
	{
		r.x = scale *v.x;
		r.y = scale *v.z;
		r.z = scale *-v.y;
		auto &pose = bone->GetPoseToBone();
		r = vectori_transform(r,{pose[0][0],pose[1][0],pose[2][0]},{pose[0][1],pose[1][1],pose[2][1]},{pose[0][2],pose[1][2],pose[2][2]},{pose[0][3],pose[1][3],pose[2][3]});
	}
	return r;
}
