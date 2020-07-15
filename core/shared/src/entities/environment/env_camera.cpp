/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/env_camera.h"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/math/e_frustum.h"
#include <algorithm>
#include <mathutil/umath_frustum.hpp>
#include <mathutil/umat.h>
#include <glm/gtx/matrix_decompose.hpp>

#define EPSILON 0.001f

using namespace pragma;

decltype(BaseEnvCameraComponent::DEFAULT_NEAR_Z) BaseEnvCameraComponent::DEFAULT_NEAR_Z = 1.f;
decltype(BaseEnvCameraComponent::DEFAULT_FAR_Z) BaseEnvCameraComponent::DEFAULT_FAR_Z = 32'768.f;
decltype(BaseEnvCameraComponent::DEFAULT_FOV) BaseEnvCameraComponent::DEFAULT_FOV = 90.f;
decltype(BaseEnvCameraComponent::DEFAULT_VIEWMODEL_FOV) BaseEnvCameraComponent::DEFAULT_VIEWMODEL_FOV = 70.f;
BaseEnvCameraComponent::BaseEnvCameraComponent(BaseEntity &ent)
	: BaseEntityComponent{ent},m_nearZ(util::FloatProperty::Create(DEFAULT_NEAR_Z)),m_farZ(util::FloatProperty::Create(DEFAULT_FAR_Z)),
	m_projectionMatrix(util::Matrix4Property::Create()),m_viewMatrix(util::Matrix4Property::Create()),
	m_fov(util::FloatProperty::Create(75.f)),m_aspectRatio(util::FloatProperty::Create(1.f))
{}
void BaseEnvCameraComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("toggle");
	ent.AddComponent("transform");
}

void BaseEnvCameraComponent::SetOrientation(const Vector3 &forward,const Vector3 &up)
{
	auto trComponent = GetEntity().GetTransformComponent();
	if(trComponent.expired())
		return;
	auto right = uvec::cross(forward,up);
	uvec::normalize(&right);
	trComponent->SetOrientation(uquat::create(forward,right,up));
}

void BaseEnvCameraComponent::UpdateMatrices()
{
	UpdateProjectionMatrix();
	UpdateViewMatrix();
}
void BaseEnvCameraComponent::UpdateViewMatrix()
{
	auto whTrComponent = GetEntity().GetTransformComponent();
	if(whTrComponent.expired())
		return;
	auto &pos = whTrComponent->GetPosition();
	*m_viewMatrix = glm::lookAtRH(
		pos,
		pos +whTrComponent->GetForward(),
		whTrComponent->GetUp()
	);
}
void BaseEnvCameraComponent::UpdateProjectionMatrix()
{
	*m_projectionMatrix = CalcProjectionMatrix(GetFOVRad(),*m_aspectRatio,**m_nearZ,**m_farZ);
}
void BaseEnvCameraComponent::SetViewMatrix(const Mat4 &mat)
{
	*m_viewMatrix = mat;
	auto whTrComponent = GetEntity().GetTransformComponent();
	if(whTrComponent.valid())
	{
		// Vector3 forward,right,up;
		// umat::to_axes(mat,forward,right,up);
		// whTrComponent->SetOrientation(uquat::create(forward,right,up));
		Vector3 scale;
		Quat rotation;
		Vector3 translation;
		Vector3 skew;
		Vector4 perspective;
		glm::decompose(glm::inverse(mat),scale,rotation,translation,skew,perspective);

		// Invert forward vector
		auto forward = -uquat::forward(rotation);
		auto up = uquat::up(rotation);
		auto right = uvec::cross(forward,up);
		uvec::normalize(&right);
		rotation = uquat::create(forward,right,up);

		whTrComponent->SetPosition(translation);
		whTrComponent->SetOrientation(rotation);
	}
}
void BaseEnvCameraComponent::SetProjectionMatrix(const Mat4 &mat) {*m_projectionMatrix = mat;}
void BaseEnvCameraComponent::GetFrustumPlanes(std::vector<Plane> &outPlanes) const
{
	std::vector<Vector3> points {};
	GetFrustumPoints(points);
	GetFrustumPlanes(points,outPlanes);
}
void BaseEnvCameraComponent::GetFrustumPlanes(std::vector<Plane> &outPlanes,float neard,float fard,float fov,float ratio,const Vector3 &center,const Vector3 &viewDir,const Vector3 &viewUp)
{
	std::vector<Vector3> points;
	GetFrustumPoints(points,neard,fard,fov,ratio,center,viewDir,viewUp);
	return GetFrustumPlanes(points,outPlanes);
}
void BaseEnvCameraComponent::GetFrustumPlanes(const std::vector<Vector3> &points,std::vector<Plane> &outPlanes)
{
	/*float wNear,hNear;
	float wFar,hFar;
	GetNearPlaneBounds(&wNear,&hNear);
	GetFarPlaneBounds(&wFar,&hFar);

	Vector3 fc = GetFarPlaneCenter();
	Vector3 nc = GetNearPlaneCenter();
	Vector3 right = -uvec::cross(*m_up,*m_forward);

	Vector3 nLeft = (nc +right *wNear /2.f) -**m_pos;
	uvec::normalize(&nLeft);
	nLeft = uvec::cross(nLeft,*m_up);
	outPlanes.push_back(Plane(nLeft,*m_pos)); // Left Plane

	Vector3 nRight = (nc -right *wNear /2.f) -**m_pos;
	uvec::normalize(&nRight);
	nRight = uvec::cross(*m_up,nRight);
	outPlanes.push_back(Plane(nRight,*m_pos)); // Right Plane

	Vector3 nTop = (nc +**m_up *hNear /2.f) -**m_pos;
	uvec::normalize(&nTop);
	nTop = uvec::cross(right,nTop);
	outPlanes.push_back(Plane(nTop,*m_pos)); // Top Plane

	Vector3 nBottom = (nc -**m_up *hNear /2.f) -**m_pos;
	uvec::normalize(&nBottom);
	nBottom = uvec::cross(nBottom,right);
	outPlanes.push_back(Plane(nBottom,**m_pos)); // Bottom Plane

	outPlanes.push_back(Plane(**m_forward,nc)); // Near Plane
	outPlanes.push_back(Plane(-**m_forward,fc)); // Far Plane*/

	outPlanes.reserve(outPlanes.size() +6u);
	// Left Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FrustumPoint::NearBottomLeft)),
		points.at(umath::to_integral(FrustumPoint::FarBottomLeft)),
		points.at(umath::to_integral(FrustumPoint::NearTopLeft)),
		});

	// Right Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FrustumPoint::NearBottomRight)),
		points.at(umath::to_integral(FrustumPoint::NearTopRight)),
		points.at(umath::to_integral(FrustumPoint::FarBottomRight)),
		});

	// Top Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FrustumPoint::NearBottomRight)),
		points.at(umath::to_integral(FrustumPoint::FarBottomLeft)),
		points.at(umath::to_integral(FrustumPoint::NearBottomLeft)),
		});

	// Bottom Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FrustumPoint::NearTopRight)),
		points.at(umath::to_integral(FrustumPoint::NearTopLeft)),
		points.at(umath::to_integral(FrustumPoint::FarTopLeft)),
		});

	// Near Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FrustumPoint::NearTopRight)),
		points.at(umath::to_integral(FrustumPoint::NearBottomLeft)),
		points.at(umath::to_integral(FrustumPoint::NearTopLeft)),
		});

	// Far Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FrustumPoint::FarTopRight)),
		points.at(umath::to_integral(FrustumPoint::FarTopLeft)),
		points.at(umath::to_integral(FrustumPoint::FarBottomLeft)),
		});
}
void BaseEnvCameraComponent::GetFrustumPoints(std::vector<Vector3> &outPoints) const
{
	auto &trComponent = GetEntity().GetTransformComponent();
	auto pos = trComponent.valid() ? trComponent->GetPosition() : Vector3{};
	auto forward = trComponent.valid() ? trComponent->GetForward() : uvec::FORWARD;
	auto up = trComponent.valid() ? trComponent->GetUp() : uvec::UP;
	GetFrustumPoints(outPoints,GetNearZ(),GetFarZ(),GetFOVRad(),GetAspectRatio(),pos,forward,up);
}
Vector3 BaseEnvCameraComponent::GetFarPlaneCenter() const {return GetPlaneCenter(*m_farZ);}
Vector3 BaseEnvCameraComponent::GetNearPlaneCenter() const {return GetPlaneCenter(*m_nearZ);}
Vector3 BaseEnvCameraComponent::GetPlaneCenter(float z) const
{
	auto &trComponent = GetEntity().GetTransformComponent();
	auto pos = trComponent.valid() ? trComponent->GetPosition() : Vector3{};
	auto forward = trComponent.valid() ? trComponent->GetForward() : uvec::FORWARD;
	return umath::frustum::get_plane_center(pos,forward,z);
}
void BaseEnvCameraComponent::GetNearPlaneBounds(float *wNear,float *hNear) const
{
	GetPlaneBounds(*m_nearZ,*wNear,*hNear);
}
void BaseEnvCameraComponent::GetFarPlaneBounds(float *wFar,float *hFar) const
{
	GetPlaneBounds(*m_farZ,*wFar,*hFar);
}
void BaseEnvCameraComponent::GetPlaneBounds(float z,float &outW,float &outH) const
{
	umath::frustum::get_plane_size(GetFOVRad(),z,*m_aspectRatio,outW,outH);
}
void BaseEnvCameraComponent::GetFarPlaneBoundaries(std::array<Vector3,4> &outPoints,float *wFar,float *hFar) const
{
	GetPlaneBoundaries(*m_farZ,outPoints,wFar,hFar);
}
void BaseEnvCameraComponent::GetNearPlaneBoundaries(std::array<Vector3,4> &outPoints,float *wNear,float *hNear) const
{
	GetPlaneBoundaries(*m_nearZ,outPoints,wNear,hNear);
}
void BaseEnvCameraComponent::GetPlaneBoundaries(std::array<Vector3,8> &outPoints,float *wNear,float *hNear,float *wFar,float *hFar) const
{
	std::array<Vector3,4> tmpPoints;
	GetNearPlaneBoundaries(tmpPoints,wNear,hNear);
	for(auto i=decltype(tmpPoints.size()){0u};i<tmpPoints.size();++i)
		outPoints.at(i) = tmpPoints.at(i);

	GetFarPlaneBoundaries(tmpPoints,wFar,hFar);
	for(auto i=decltype(tmpPoints.size()){0u};i<tmpPoints.size();++i)
		outPoints.at(i +tmpPoints.size()) = tmpPoints.at(i);
}
void BaseEnvCameraComponent::GetPlaneBoundaries(float z,std::array<Vector3,4> &outPoints,float *wNear,float *hNear,float *wFar,float *hFar) const
{
	auto &trComponent = GetEntity().GetTransformComponent();
	auto pos = trComponent.valid() ? trComponent->GetPosition() : Vector3{};
	auto forward = trComponent.valid() ? trComponent->GetForward() : uvec::FORWARD;
	auto up = trComponent.valid() ? trComponent->GetUp() : uvec::UP;
	outPoints = umath::frustum::get_plane_boundaries(pos,forward,up,GetFOVRad(),z,*m_aspectRatio,wNear,hNear);
}

void BaseEnvCameraComponent::SetFOV(float fov) {*m_fov = fov;}
void BaseEnvCameraComponent::SetAspectRatio(float aspectRatio) {*m_aspectRatio = aspectRatio;}
void BaseEnvCameraComponent::SetNearZ(float nearZ) {*m_nearZ = nearZ;}
void BaseEnvCameraComponent::SetFarZ(float farZ) {*m_farZ = farZ;}
const Mat4 &BaseEnvCameraComponent::GetProjectionMatrix() const {return *m_projectionMatrix;}
const Mat4 &BaseEnvCameraComponent::GetViewMatrix() const {return *m_viewMatrix;}

float BaseEnvCameraComponent::GetFOV() const {return *m_fov;}
float BaseEnvCameraComponent::GetFOVRad() const {return umath::deg_to_rad(*m_fov);}

const util::PMatrix4Property &BaseEnvCameraComponent::GetProjectionMatrixProperty() const {return m_projectionMatrix;}
const util::PMatrix4Property &BaseEnvCameraComponent::GetViewMatrixProperty() const {return m_viewMatrix;}

const util::PFloatProperty &BaseEnvCameraComponent::GetAspectRatioProperty() const {return m_aspectRatio;}
const util::PFloatProperty &BaseEnvCameraComponent::GetNearZProperty() const {return m_nearZ;}
const util::PFloatProperty &BaseEnvCameraComponent::GetFarZProperty() const {return m_farZ;}
const util::PFloatProperty &BaseEnvCameraComponent::GetFOVProperty() const {return m_fov;}

float BaseEnvCameraComponent::GetAspectRatio() const {return *m_aspectRatio;}
float BaseEnvCameraComponent::GetNearZ() const {return *m_nearZ;}
float BaseEnvCameraComponent::GetFarZ() const {return *m_farZ;}
void BaseEnvCameraComponent::UpdateFrustumPlanes()
{
	m_frustumPlanes.clear();
	GetFrustumPlanes(m_frustumPlanes);
}
const std::vector<Plane> &BaseEnvCameraComponent::GetFrustumPlanes() const {return m_frustumPlanes;}
Vector3 BaseEnvCameraComponent::GetNearPlanePoint(const Vector2 &uv) const
{
	return GetPlanePoint(*m_nearZ,uv);
}
Vector3 BaseEnvCameraComponent::GetFarPlanePoint(const Vector2 &uv) const
{
	return GetPlanePoint(*m_farZ,uv);
}
Vector3 BaseEnvCameraComponent::GetPlanePoint(float z,const Vector2 &uv) const
{
	auto &trComponent = GetEntity().GetTransformComponent();
	auto pos = trComponent.valid() ? trComponent->GetPosition() : Vector3{};
	auto forward = trComponent.valid() ? trComponent->GetForward() : uvec::FORWARD;
	auto right = trComponent.valid() ? trComponent->GetRight() : uvec::RIGHT;
	auto up = trComponent.valid() ? trComponent->GetUp() : uvec::UP;
	return umath::frustum::get_plane_point(pos,forward,right,up,GetFOVRad(),z,*m_aspectRatio,uv);
}

void BaseEnvCameraComponent::CreateFrustumMesh(const Vector2 &uvStart,const Vector2 &uvEnd,std::vector<Vector3> &verts,std::vector<uint16_t> &indices) const
{
	auto uvTopLeft = Vector2(umath::min(uvStart.x,uvEnd.x),umath::min(uvStart.y,uvEnd.y));
	auto uvBottomRight = Vector2(umath::max(uvStart.x,uvEnd.x),umath::max(uvStart.y,uvEnd.y));
	auto uvTopRight = Vector2(uvBottomRight.x,uvTopLeft.y);
	auto uvBottomLeft = Vector2(uvTopLeft.x,uvBottomRight.y);
	verts = {
		GetNearPlanePoint(uvTopLeft),
		GetNearPlanePoint(uvTopRight),
		GetNearPlanePoint(uvBottomRight),
		GetNearPlanePoint(uvBottomLeft),

		GetFarPlanePoint(uvTopLeft),
		GetFarPlanePoint(uvTopRight),
		GetFarPlanePoint(uvBottomRight),
		GetFarPlanePoint(uvBottomLeft)
	};
	indices = {
		2,1,0,
		3,2,0,

		0,1,4,
		1,5,4,

		1,2,5,
		2,6,5,

		7,2,3,
		7,6,2,

		4,3,0,
		4,7,3,

		4,5,6,
		4,6,7
	};
}
void BaseEnvCameraComponent::CreateFrustumKDop(const Vector2 &uvStart,const Vector2 &uvEnd,std::vector<Plane> &kDop) const
{
	auto uvTopLeft = Vector2(umath::min(uvStart.x,uvEnd.x),umath::min(uvStart.y,uvEnd.y));
	auto uvBottomRight = Vector2(umath::max(uvStart.x,uvEnd.x),umath::max(uvStart.y,uvEnd.y));
	auto uvTopRight = Vector2(uvBottomRight.x,uvTopLeft.y);
	auto uvBottomLeft = Vector2(uvTopLeft.x,uvBottomRight.y);
	std::array<Vector3,8> verts = {
		GetNearPlanePoint(uvTopLeft),
		GetNearPlanePoint(uvTopRight),
		GetNearPlanePoint(uvBottomRight),
		GetNearPlanePoint(uvBottomLeft),

		GetFarPlanePoint(uvTopLeft),
		GetFarPlanePoint(uvTopRight),
		GetFarPlanePoint(uvBottomRight),
		GetFarPlanePoint(uvBottomLeft)
	};

	// Move all far plane points closer to near plane to avoid precision errors when
	// creating the planes.
	for(auto i=decltype(verts.size()){0};i<verts.size() /2;++i)
	{
		auto &vNear = verts.at(i);
		auto &vFar = verts.at(i +4);
		vFar -= vNear;
		uvec::normalize(&vFar);
		vFar += vNear;
	}

	kDop = {
		Plane(verts.at(2),verts.at(0),verts.at(1)),
		Plane(verts.at(0),verts.at(4),verts.at(1)),
		Plane(verts.at(1),verts.at(5),verts.at(2)),
		Plane(verts.at(7),verts.at(3),verts.at(2)),
		Plane(verts.at(4),verts.at(0),verts.at(3)),
		Plane(verts.at(4),verts.at(6),verts.at(5))
	};
	kDop.back().MoveToPos(GetFarPlaneCenter()); // Move back plane back to its actual position. TODO: Can this also cause precision errors?
}

void BaseEnvCameraComponent::GetFrustumPoints(std::vector<Vector3> &outPoints,float neard,float fard,float fov,float ratio,const Vector3 &center,const Vector3 &viewDir,const Vector3 &viewUp)
{
	/*Vector3 fc = center +viewDir *fard;
	Vector3 nc = center +viewDir *neard;
	auto &up = viewUp;
	auto right = uvec::cross(viewDir,viewUp);
	uvec::normalize(&right);*/
	Vector3 up = viewUp;//(0.f,1.f,0.f);
	Vector3 right = -glm::cross(viewDir,up);
	//auto right = uvec::get_perpendicular(viewDir);
	Vector3 fc = center +viewDir *fard;
	Vector3 nc = center +viewDir *neard;
	uvec::normalize(&right);
	up = glm::cross(right,viewDir);
	uvec::normalize(&up);

	float near_height = tanf(fov /2.f) *neard;
	float near_width = near_height *ratio;
	float far_height = tanf(fov /2.f) *fard;
	float far_width = far_height *ratio;

	outPoints.reserve(outPoints.size() +8u);
	outPoints.push_back(fc -up *far_height -right *far_width); // Bottom Left
	outPoints.push_back(fc +up *far_height -right *far_width); // Top Left
	outPoints.push_back(fc +up *far_height +right *far_width); // Top Right
	outPoints.push_back(fc -up *far_height +right *far_width); // Bottom Right

	outPoints.push_back(nc -up *near_height -right *near_width); // Bottom Left
	outPoints.push_back(nc +up *near_height -right *near_width); // Top Left
	outPoints.push_back(nc +up *near_height +right *near_width); // Top Right
	outPoints.push_back(nc -up *near_height +right *near_width); // Bottom Right
}
void BaseEnvCameraComponent::GetFrustumNeighbors(FrustumPlane planeID,FrustumPlane *neighborIDs)
{
	static FrustumPlane neighbors[6][4] = {
		{ // Left
			FrustumPlane::Top,
			FrustumPlane::Bottom,
			FrustumPlane::Near,
			FrustumPlane::Far
		},
		{ // Right
			FrustumPlane::Top,
			FrustumPlane::Bottom,
			FrustumPlane::Near,
			FrustumPlane::Far
		},
		{ // Top
			FrustumPlane::Left,
			FrustumPlane::Right,
			FrustumPlane::Near,
			FrustumPlane::Far
		},
		{ // Bottom
			FrustumPlane::Left,
			FrustumPlane::Right,
			FrustumPlane::Near,
			FrustumPlane::Far
		},
		{ // Near
			FrustumPlane::Left,
			FrustumPlane::Right,
			FrustumPlane::Top,
			FrustumPlane::Bottom
		},
		{ // Far
			FrustumPlane::Left,
			FrustumPlane::Right,
			FrustumPlane::Top,
			FrustumPlane::Bottom
		}
	};
	for(unsigned int i=0;i<4;i++)
		neighborIDs[i] = neighbors[static_cast<int>(planeID)][i];
}
void BaseEnvCameraComponent::GetFrustumPlaneCornerPoints(FrustumPlane planeA,FrustumPlane planeB,FrustumPoint *cornerPoints)
{
	static FrustumPoint points[6][6][2] = {
		{ // Left
			{ // Left
				FrustumPoint::FarBottomLeft,FrustumPoint::FarBottomLeft // Invalid
			},
			{ // Right
				FrustumPoint::FarBottomLeft,FrustumPoint::FarBottomLeft // Invalid
			},
			{ // Top
				FrustumPoint::NearTopLeft,FrustumPoint::FarTopLeft
			},
			{ // Bottom
				FrustumPoint::FarBottomLeft,FrustumPoint::NearBottomLeft
			},
			{ // Near
				FrustumPoint::NearBottomLeft,FrustumPoint::NearTopLeft
			},
			{ // Far
				FrustumPoint::FarTopLeft,FrustumPoint::FarBottomLeft
			}
		},
		{ // Right
			{ // Left
				FrustumPoint::FarBottomRight,FrustumPoint::FarBottomRight // Invalid
			},
			{ // Right
				FrustumPoint::FarBottomRight,FrustumPoint::FarBottomRight // Invalid
			},
			{ // Top
				FrustumPoint::FarTopRight,FrustumPoint::NearTopRight
			},
			{ // Bottom
				FrustumPoint::NearBottomRight,FrustumPoint::FarBottomRight
			},
			{ // Near
				FrustumPoint::NearTopRight,FrustumPoint::NearBottomRight
			},
			{ // Far
				FrustumPoint::FarBottomRight,FrustumPoint::FarTopRight
			}
		},
		{ // Top
			{ // Left
				FrustumPoint::FarTopLeft,FrustumPoint::NearTopLeft
			},
			{ // Right
				FrustumPoint::NearTopRight,FrustumPoint::FarTopRight
			},
			{ // Top
				FrustumPoint::NearTopLeft,FrustumPoint::FarTopLeft // Invalid
			},
			{ // Bottom
				FrustumPoint::FarBottomLeft,FrustumPoint::NearBottomLeft // Invalid
			},
			{ // Near
				FrustumPoint::NearTopLeft,FrustumPoint::NearTopRight
			},
			{ // Far
				FrustumPoint::FarTopRight,FrustumPoint::FarTopLeft
			}
		},
		{ // Bottom
			{ // Left
				FrustumPoint::NearBottomLeft,FrustumPoint::FarBottomLeft
			},
			{ // Right
				FrustumPoint::FarBottomRight,FrustumPoint::NearBottomRight
			},
			{ // Top
				FrustumPoint::NearBottomLeft,FrustumPoint::FarBottomLeft // Invalid
			},
			{ // Bottom
				FrustumPoint::FarBottomLeft,FrustumPoint::NearBottomLeft // Invalid
			},
			{ // Near
				FrustumPoint::NearBottomRight,FrustumPoint::NearBottomLeft
			},
			{ // Far
				FrustumPoint::FarBottomLeft,FrustumPoint::FarBottomRight
			}
		},
		{ // Near
			{ // Left
				FrustumPoint::NearTopLeft,FrustumPoint::NearBottomLeft
			},
			{ // Right
				FrustumPoint::NearBottomRight,FrustumPoint::NearTopRight
			},
			{ // Top
				FrustumPoint::NearTopRight,FrustumPoint::NearTopLeft
			},
			{ // Bottom
				FrustumPoint::NearBottomLeft,FrustumPoint::NearBottomRight
			},
			{ // Near
				FrustumPoint::NearTopLeft,FrustumPoint::NearTopRight
			},
			{ // Far
				FrustumPoint::FarTopRight,FrustumPoint::FarTopLeft
			},
		},
		{ // Far
			{ // Left
				FrustumPoint::FarBottomLeft,FrustumPoint::FarTopLeft
			},
			{ // Right
				FrustumPoint::FarTopRight,FrustumPoint::FarBottomRight
			},
			{ // Top
				FrustumPoint::FarTopLeft,FrustumPoint::FarTopRight
			},
			{ // Bottom
				FrustumPoint::FarBottomRight,FrustumPoint::FarBottomLeft
			},
			{ // Near
				FrustumPoint::FarTopLeft,FrustumPoint::FarTopRight
			},
			{ // Far
				FrustumPoint::FarTopRight,FrustumPoint::FarTopLeft
			}
		}
	};
	cornerPoints[0] = points[static_cast<int>(planeA)][static_cast<int>(planeB)][0];
	cornerPoints[1] = points[static_cast<int>(planeA)][static_cast<int>(planeB)][1];
}
void BaseEnvCameraComponent::CreateFrustumKDop(const std::vector<Plane> &planes,const std::vector<Vector3> &points,const Vector3 &dir,std::vector<Plane> *kDop)
{
	std::array<float,6> fDir;
	for(unsigned int i=0;i<6;i++)
	{
		Plane &plane = const_cast<Plane&>(planes[i]);
		fDir[i] = uvec::dot(plane.GetNormal(),dir);
		if(fDir[i] < EPSILON)
			kDop->push_back(plane);
	}
	for(unsigned int i=0;i<6;i++)
	{
		//const Plane &plane = planes[i];
		if(fDir[i] <= EPSILON)
		{
			FrustumPlane neighbors[4];
			GetFrustumNeighbors(FrustumPlane(i),&neighbors[0]);
			for(unsigned j=0;j<4;j++)
			{
				float fNDir = fDir[static_cast<int>(neighbors[j])];
				if(fNDir > EPSILON)
				{
					FrustumPoint corners[2];
					GetFrustumPlaneCornerPoints(FrustumPlane(i),neighbors[j],&corners[0]);
					Plane p(points[static_cast<int>(corners[0])],points[static_cast<int>(corners[1])],points[static_cast<int>(corners[0])] -dir);
					kDop->push_back(p);
				}
			}
		}
	}
}
Mat4 BaseEnvCameraComponent::CalcProjectionMatrix(float fovRad,float aspectRatio,float nearZ,float farZ)
{
	auto mat = glm::perspectiveRH(fovRad,aspectRatio,nearZ,farZ);
	mat = glm::scale(mat,Vector3(1.f,-1.f,1.f));
	return mat;
}
