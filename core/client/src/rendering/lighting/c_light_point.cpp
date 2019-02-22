#include "stdafx_client.h"

// TODO: Remove this file
#if 0
#include "pragma/rendering/lighting/c_light_point.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcube.h"
#include <mathutil/umath.h>
#include "c_cubemapsize.h"

extern DLLCENGINE CEngine *c_engine;

static const std::array<Vector3,6> directions = {
	Vector3(1.f,0.f,0.f),
	Vector3(-1.f,0.f,0.f),
	Vector3(0.f,1.f,0.f),
	Vector3(0.f,-1.f,0.f),
	Vector3(0.f,0.f,1.f),
	Vector3(0.f,0.f,-1.f)
};

CLightPoint::CLightPoint(const Vector3 &pos,const Color &col,float distance,ShadowType shadowType)
	: CLightBase(LightType::Point,pos,col,shadowType),CLightRanged()
{
	SetDistance(distance);
}

void CLightPoint::Initialize()
{
	CLightBase::Initialize();
	bSkipUpdate = true;
	auto scale = Vector3(-1.f,-1.f,1.f); // Vulkan TODO
	auto p = glm::perspectiveRH<float>(CFloat(umath::deg_to_rad(90.f)),1.f,2.f,GetDistance()); // Vulkan TODO
	p = glm::scale(p,scale);
	SetProjectionMatrix(p);
	UpdateViewMatrices();
	bSkipUpdate = false;
	//SetProjectionMatrix(glm::perspective<float>(35.f *2.f,1.f,2.f,m_distance));
	//SetViewMatrix(glm::lookAt(GetPosition(),GetPosition() +Vector3(1,0,0),Vector3(0,1,0)));
	for(auto i=0;i<6;i++)
		UpdateTransformationMatrix(i);
}

bool CLightPoint::ShouldPass(const CBaseEntity &ent,uint32_t &renderFlags)
{
	if(CLightBase::ShouldPass(ent,renderFlags) == false || CLightRanged::ShouldPass(ent) == false)
		return false;
	const auto ang = 0.7853982f; // 45 Degree
	for(auto i=decltype(directions.size()){0};i<directions.size();++i)
	{
		auto &dir = directions[i];
		if(IsInCone(ent,dir,ang) == true)
			renderFlags |= 1<<i;
	}
	return (renderFlags != 0) ? true : false;
}
bool CLightPoint::ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags) {return CLightRanged::ShouldPass(ent,mesh,renderFlags);}

Mat4 &CLightPoint::GetTransformationMatrix(unsigned int j) {return MVPBias<6>::GetTransformationMatrix(j);}

void CLightPoint::UpdateViewMatrices()
{
	auto b = bSkipUpdate;
	bSkipUpdate = true;
	auto &pos = GetPosition();
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Left)],Vector3(0,1,0)),umath::to_integral(CubeMapSide::Left));//umat::look_at(pos,pos +Vector3(1,0,0),Vector3(0,1,0)),1); // Vulkan TODO
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Right)],Vector3(0,1,0)),umath::to_integral(CubeMapSide::Right));
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Top)],Vector3(0,0,-1)),umath::to_integral(CubeMapSide::Top));
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Bottom)],Vector3(0,0,1)),umath::to_integral(CubeMapSide::Bottom));
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Front)],Vector3(0,1,0)),umath::to_integral(CubeMapSide::Front));
	SetViewMatrix(glm::lookAtRH(pos,pos +directions[umath::to_integral(CubeMapSide::Back)],Vector3(0,1,0)),umath::to_integral(CubeMapSide::Back));
	bSkipUpdate = b;
	if(bSkipUpdate == false)
	{
		for(auto i=0;i<6;i++)
			UpdateTransformationMatrix(i);
	}
}

void CLightPoint::SetPosition(const Vector3 &pos)
{
	if(uvec::cmp(pos,GetPosition()) == true)
		return;
	CLightBase::SetPosition(pos);
	UpdateViewMatrices();
}

void CLightPoint::InitializeShadowMap()
{
	if(m_shadow != nullptr || m_shadowTypes == ShadowType::None)
		return;
	m_shadow = std::make_unique<ShadowMapCube>();
	CLightBase::InitializeShadowMap(*m_shadow);
}
void CLightPoint::ScheduleRender() {CLightRanged::ScheduleRender();}
bool CLightPoint::ShouldRender() {return CLightRanged::ShouldRender();}

const Vector3 &CLightPoint::GetPosition() const {return CLightBase::GetPosition();}

LightType CLightPoint::GetType() const {return LightType::Point;}
float CLightPoint::GetDistance() const {return CLightBase::GetDistance();}
void CLightPoint::SetDistance(float distance) {CLightBase::SetDistance(distance);}

void CLightPoint::UpdateTransformationMatrix(unsigned int j) // TODO This is called every time one of the matrices is changed; Only needs to be called once, after they're ALL changed!
{
	if(bSkipUpdate == true)
		return;
	MVPBias<6>::UpdateTransformationMatrix(j);
	if(m_shadowBuffer == nullptr)
		return;
	std::array<Mat4,3> matrices = {GetBiasTransformationMatrix(),GetViewMatrix(4),GetProjectionMatrix()};
	c_engine->ScheduleRecordUpdateBuffer(m_shadowBuffer,0ull,matrices);
}
#endif
