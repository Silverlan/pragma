#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"

const std::vector<Plane> &Scene::GetFrustumPlanes() const {return m_frustumPlanes;}
const std::vector<Plane> &Scene::GetClippedFrustumPlanes() const {return m_clippedFrustumPlanes;}

void Scene::UpdateFrustumPlanes()
{
	m_frustumPlanes.clear();
	m_clippedFrustumPlanes.clear();
	if(camera == nullptr)
		return;
	camera->GetFrustumPlanes(m_frustumPlanes);
	m_clippedFrustumPlanes = m_frustumPlanes;
/*	auto forward = camera->GetForward();
	auto up = camera->GetUp();
	auto rot = camera->GetRotation();
	auto pos = camera->GetPos();
	camera->SetForward(uvec::FORWARD);
	camera->SetUp(uvec::UP);
	camera->SetPos(Vector3{});

	std::vector<Vector3> frustumPoints {};
	camera->GetFrustumPoints(frustumPoints);
	for(auto &p : frustumPoints)
	{
		uvec::rotate(&p,rot);
		p += pos;
	}
	camera->GetFrustumPlanes(frustumPoints,m_frustumPlanes);
	m_clippedFrustumPlanes = m_frustumPlanes;

	camera->SetForward(forward);
	camera->SetUp(up);
	camera->SetPos(pos);*/


	/*if(FogController::IsFogEnabled() == true)
	{
		float fogDist = FogController::GetFarDistance();
		float farZ = cam->GetZFar();
		if(fogDist < farZ)
			farZ = fogDist;
		Plane &farPlane = planesClipped[static_cast<int>(FRUSTUM_PLANE::FAR)];
		Vector3 &start = planesClipped[static_cast<int>(FRUSTUM_PLANE::NEAR)].GetCenterPos();
		Vector3 dir = farPlane.GetCenterPos() -start;
		uvec::normalize(&dir);
		farPlane.MoveToPos(start +dir *farZ); // TODO Checkme
	}*/
}
