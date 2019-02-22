#include "stdafx_client.h"
// TODO: Remove this file
#if 0
#include "pragma/rendering/lighting/c_light_spot.h"
#include <mathutil/umath.h>

CLightSpot::CLightSpot(const Vector3 &pos,const Color &col,float distance,const Vector3 &dir,float outerCutoff,float innerCutoff,ShadowType shadowType)
	: CLight(LightType::Spot,pos,col,shadowType),CLightRanged()
{
	SetDistance(distance);
	SetDirection(dir);
	SetOuterCutoffAngle(outerCutoff);
	SetInnerCutoffAngle(innerCutoff);
}

void CLightSpot::Initialize()
{
	CLight::Initialize();

	UpdateProjectionMatrix();

	auto pos = GetPosition();
	SetPosition({}); // Required; Otherwise the position isn't updated properly.
	SetPosition(pos);
}
void CLightSpot::UpdateProjectionMatrix()
{
	auto scale = Vector3(1.f,-1.f,1.f); // Vulkan TODO
	//SetProjectionMatrix(glm::perspectiveLH<float>(CFloat(umath::deg_to_rad(m_angOuterCutoff *2.f)),1.f,2.f,m_distance));
	//auto p = glm::perspective<float>(CFloat(umath::deg_to_rad(m_angOuterCutoff *2.f)),1.f,1.f,m_distance);
	//p = glm::scale(p,scale); /* Shadow TODO */
	/*static Mat4 transform{
		1.f,0.f,0.f,0.f,
		0.f,-1.f,0.f,0.f,
		0.f,0.f,0.5f,0.5f,
		0.f,0.f,0.f,1.f
	};*/
	//auto p = glm::perspective<float>(CFloat(umath::deg_to_rad(m_angOuterCutoff *2.f)),1.f,1.f,m_distance);
	auto p = glm::perspectiveRH<float>(CFloat(umath::deg_to_rad(GetOuterCutoffAngle() *2.f)),1.f,1.f,GetDistance());
	//p = transform *p;
	p = glm::scale(p,scale); /* Shadow TODO */
	SetProjectionMatrix(p);
}
void CLightSpot::ScheduleRender() {CLightRanged::ScheduleRender();}
bool CLightSpot::ShouldRender() {return CLightRanged::ShouldRender();}

LightType CLightSpot::GetType() const {return LightType::Spot;}

float CLightSpot::GetDistance() const {return CLight::GetDistance();}
void CLightSpot::SetDistance(float distance)
{
	if(distance == GetDistance())
		return;
	CLight::SetDistance(distance);
	UpdateProjectionMatrix();
}

void CLightSpot::UpdateViewMatrices()
{
	//SetViewMatrix(glm::lookAtLH(GetPosition(),GetPosition() +m_dir,Vector3(0,1,0)));
	//SetViewMatrix(umat::look_at(GetPosition(),GetPosition() +m_dir,Vector3(0,1,0))); // Vulkan TODO
	//SetViewMatrix(umat::look_at(GetPosition(),GetPosition() +m_dir,Vector3(0,1,0)));
	auto &dir = GetDirection();
	SetViewMatrix(glm::lookAtRH(GetPosition(),GetPosition() +dir,uvec::get_perpendicular(dir)));
}

const Vector3 &CLightSpot::GetPosition() const {return CLightBase::GetPosition();}
void CLightSpot::SetPosition(const Vector3 &pos)
{
	if(uvec::cmp(pos,GetPosition()) == true)
		return;
	CLight::SetPosition(pos);
	UpdateViewMatrices();
}

void CLightSpot::SetOuterCutoffAngle(float ang)
{
	if(ang == GetOuterCutoffAngle())
		return;
	CLight::SetOuterCutoffAngle(ang);
	UpdateProjectionMatrix();
}
void CLightSpot::SetDirection(const Vector3 &dir)
{
	if(uvec::cmp(dir,GetDirection()) == true)
		return;
	CLight::SetDirection(dir);
	m_bFullUpdateRequired = true;
	SetStaticResolved(false);

	UpdateProjectionMatrix();
	UpdateViewMatrices();
}
bool CLightSpot::ShouldPass(const CBaseEntity &ent,uint32_t &renderFlags)
{
	if(CLightRanged::ShouldPass(ent) == false || CLight::ShouldPass(ent,renderFlags) == false)
		return false;
	return IsInCone(ent,GetDirection(),static_cast<float>(umath::deg_to_rad(GetOuterCutoffAngle())));
}
bool CLightSpot::ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags) {return CLightRanged::ShouldPass(ent,mesh,renderFlags);}
#endif
