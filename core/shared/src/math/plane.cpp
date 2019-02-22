#include "stdafx_shared.h"
#include "pragma/math/plane.h"

Plane::Plane(const Vector3 &a,const Vector3 &b,const Vector3 &c)
{
	Vector3 n = glm::cross(b -a,c -a);
	uvec::normalize(&n);
	double d = glm::dot(-n,a);
	Initialize(n,d);
}

Plane::Plane(Vector3 n,const Vector3 &pos)
{
	m_normal = n;
	MoveToPos(pos);
	m_posCenter = pos;
}

Plane::Plane(Vector3 n,double d)
{
	Initialize(n,d);
}

Plane::Plane()
	: Plane(Vector3{},0.0)
{}

void Plane::Initialize(Vector3 n,double d)
{
	m_normal = n;
	m_distance = d;
	m_pos.x = n.x *static_cast<float>(-d);
	m_pos.y = n.y *static_cast<float>(-d);
	m_pos.z = n.z *static_cast<float>(-d);
	m_posCenter = m_pos;
}

const Vector3 &Plane::GetNormal() const {return const_cast<Plane*>(this)->GetNormal();}
const Vector3 &Plane::GetPos() const {return const_cast<Plane*>(this)->GetPos();}
const Vector3 &Plane::GetCenterPos() const {return const_cast<Plane*>(this)->GetCenterPos();}
Vector3 &Plane::GetNormal() {return m_normal;}
Vector3 &Plane::GetPos() {return m_pos;}
Vector3 &Plane::GetCenterPos() {return m_posCenter;}
double Plane::GetDistance() const {return m_distance;}
float Plane::GetDistance(const Vector3 &pos) const
{
	Vector3 offset = pos -m_posCenter;
	return uvec::dot(m_normal,offset);
}
void Plane::SetDistance(double d)
{
	m_distance = d;
}
void Plane::SetNormal(const Vector3 &n)
{
	m_normal = n;
}

void Plane::MoveToPos(const Vector3 &pos)
{
	Vector3 &n = GetNormal();
	double dNew = glm::dot(n,pos);
	m_distance = -dNew;
	m_pos = n *float(dNew);
	m_posCenter = pos;
}

void Plane::Rotate(const EulerAngles &ang)
{
	Vector3 &n = GetNormal();
	uvec::rotate(&n,EulerAngles(ang));
	m_normal = n;
}

bool Plane::GetPlaneIntersection(Vector3 *intersect,const Vector3 &na,const Vector3 &nb,const Vector3 &nc,double da,double db,double dc)
{
	float denom = glm::dot(na,glm::cross(nb,nc));
	if(denom == 0) return false;
	Vector3 crossbc = glm::cross(nb,nc);
	Vector3 crossca = glm::cross(nc,na);
	Vector3 crossab = glm::cross(na,nb);
	uvec::mul(&crossbc,-da);
	uvec::mul(&crossca,-db);
	uvec::mul(&crossab,-dc);
	*intersect = crossbc +crossca +crossab;
	uvec::div(intersect,denom);
	return true;
}