#include "stdafx_client.h"
// TODO: Remove this file
#if 0
#include "pragma/rendering/lighting/c_light_ranged.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/math/intersection.h>

extern DLLCLIENT CGame *c_game;
CLightRanged::CLightRanged()
	: m_tLastRender(0.0),m_bScheduleRender(true)
{}

void CLightRanged::ScheduleRender() {m_bScheduleRender = true;}

bool CLightRanged::ShouldPass(const CBaseEntity &ent)
{
	auto pRenderComponent = ent.GetRenderComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(pRenderComponent.expired() || pTrComponent.expired())
		return false;
	auto &origin = GetPosition();
	auto sphere = pRenderComponent->GetRenderSphereBounds();
	auto &pos = pTrComponent->GetPosition();
	auto radius = GetDistance();
	return (uvec::distance(pos +sphere.pos,origin) <= (radius +sphere.radius)) ? true : false;
}
bool CLightRanged::ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t&)
{
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.expired())
		return false;
	auto &origin = GetPosition();
	auto radius = GetDistance();
	auto &pos = pTrComponent->GetPosition();
	Vector3 min;
	Vector3 max;
	mesh.GetBounds(min,max);
	min += pos;
	max += pos;
	return Intersection::AABBSphere(min,max,origin,radius);
}
bool CLightRanged::IsRenderScheduled() const {return m_bScheduleRender;}
void CLightRanged::SetRenderScheduled(bool b) {m_bScheduleRender = b;}
bool CLightRanged::ShouldRender()
{
	/*if(!IsRenderScheduled())
		return false;
	auto &scene = c_game->GetRenderScene();
	auto &cam = scene->camera;
	auto &posCam = cam->GetPos();
	auto &pos = GetPosition();
	auto dist = uvec::distance(pos,posCam) -GetDistance();
	const auto farDist = 500.f; // Lights up to this distance will be refreshed every frame
	if(dist <= farDist)
	{
		SetRenderScheduled(false);
		return true;
	}
	dist -= farDist;
	auto t = c_game->RealTime();
	if(t -m_tLastRender > umath::min((dist /farDist) *0.25f,0.25f))
	{
		SetRenderScheduled(false);
		m_tLastRender = t;
		return true;
	}*/ // prosper TODO
	return false;
}
#endif
