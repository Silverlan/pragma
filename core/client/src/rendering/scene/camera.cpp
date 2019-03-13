#include "stdafx_client.h"
#include "pragma/rendering/scene/camera.h"
#include <mathutil/umat.h>
#include <pragma/math/plane.h>
#include "pragma/rendering/scene/e_frustum.h"
#include <mathutil/umath.h>
#include <mathutil/umath_frustum.hpp>
#include <pragma/math/vector/wvvector3.h>

#define EPSILON 0.001f

DEFINE_BASE_HANDLE(DLLCLIENT,Camera,Camera);

std::shared_ptr<Camera> Camera::Create(float fov,float fovView,float aspectRatio,float nearZ,float farZ)
{
	return std::shared_ptr<Camera>(new Camera(fov,fovView,aspectRatio,nearZ,farZ));
}
std::shared_ptr<Camera> Camera::Create(const Camera &other)
{
	return std::shared_ptr<Camera>(new Camera(other));
}

Camera::Camera(float fov,float fovView,float aspectRatio,float nearZ,float farZ)
	: std::enable_shared_from_this<Camera>(),
	m_aspectRatio(aspectRatio),m_fovRad(CFloat(umath::deg_to_rad(fov))),m_fovViewRad(CFloat(umath::deg_to_rad(fovView))),
	m_nearZ(util::FloatProperty::Create(nearZ)),m_farZ(util::FloatProperty::Create(farZ)),
	m_handle(new PtrCamera(this)),
	m_projectionMatrix(util::Matrix4Property::Create()),m_viewProjectionMatrix(util::Matrix4Property::Create()),
	m_viewMatrix(util::Matrix4Property::Create()),m_up(util::Vector3Property::Create(Vector3(0,1,0))),
	m_forward(util::Vector3Property::Create(Vector3(1,0,0))),m_pos(util::Vector3Property::Create(Vector3(0,0,0))),
	m_fov(util::FloatProperty::Create(fov)),m_fovView(util::FloatProperty::Create(fovView))
{
	UpdateMatrices();
}

Camera::Camera(const Camera &other)
	: std::enable_shared_from_this<Camera>(),m_projectionMatrix(other.m_projectionMatrix),m_viewProjectionMatrix(other.m_viewProjectionMatrix),
	m_viewMatrix(other.m_viewMatrix),m_up(other.m_up),m_forward(other.m_forward),m_pos(other.m_pos),
	m_fov(other.m_fov),m_fovView(other.m_fovView),m_fovRad(other.m_fovRad),m_fovViewRad(other.m_fovViewRad),
	m_aspectRatio(other.m_aspectRatio),m_nearZ(other.m_nearZ),m_farZ(other.m_farZ),m_frustumPlanes(other.m_frustumPlanes),
	m_handle(new PtrCamera(this))
{}

Camera::~Camera()
{
	m_handle.Invalidate();
}

const util::PMatrix4Property &Camera::GetProjectionMatrixProperty() const {return m_projectionMatrix;}
const util::PMatrix4Property &Camera::GetViewProjectionMatrixProperty() const {return m_viewProjectionMatrix;}
const util::PMatrix4Property &Camera::GetViewMatrixProperty() const {return m_viewMatrix;}

const util::PVector3Property &Camera::GetUpProperty() const {return m_up;}
const util::PVector3Property &Camera::GetForwardProperty() const {return m_forward;}
const util::PVector3Property &Camera::GetPosProperty() const {return m_pos;}

const util::PFloatProperty &Camera::GetNearZProperty() const {return m_nearZ;}
const util::PFloatProperty &Camera::GetFarZProperty() const {return m_farZ;}
const util::PFloatProperty &Camera::GetFOVProperty() const {return m_fov;}
const util::PFloatProperty &Camera::GetViewFOVProperty() const {return m_fovView;}

void Camera::UpdateFrustumPlanes()
{
	m_frustumPlanes.clear();
	GetFrustumPlanes(m_frustumPlanes);
}

CameraHandle Camera::GetHandle() const {return m_handle;}

void Camera::UpdateMatrices()
{
	UpdateProjectionMatrices();
	UpdateViewMatrix();
}
void Camera::UpdateViewMatrix()
{
	*m_viewMatrix = glm::lookAtRH(
		**m_pos,
		**m_pos +**m_forward,
		**m_up
	);
}
void Camera::UpdateProjectionMatrix()
{
	*m_projectionMatrix = glm::perspectiveRH(m_fovRad,m_aspectRatio,**m_nearZ,**m_farZ);
	*m_projectionMatrix = glm::scale(**m_projectionMatrix,Vector3(1.f,-1.f,1.f));
}
void Camera::UpdateViewProjectionMatrix()
{
	*m_viewProjectionMatrix = glm::perspective(m_fovViewRad,m_aspectRatio,1.f,**m_farZ);
	*m_viewProjectionMatrix = glm::scale(**m_viewProjectionMatrix,Vector3(1.f,-1.f,1.f));
}
void Camera::UpdateProjectionMatrices()
{
	UpdateProjectionMatrix();
	UpdateViewProjectionMatrix();
}

Vector3 Camera::GetFarPlaneCenter() const {return umath::frustum::get_far_plane_center(*m_pos,*m_forward,*m_farZ);}
Vector3 Camera::GetNearPlaneCenter() const {return umath::frustum::get_near_plane_center(*m_pos,*m_forward,*m_nearZ);}

void Camera::SetViewMatrix(const Mat4 &mat)
{
	*m_viewMatrix = mat;
	*m_up = Vector3(mat[0][1],mat[1][1],mat[2][2]);
	*m_forward = Vector3(-mat[0][2],-mat[1][2],-mat[2][2]);
}

void Camera::SetViewProjectionMatrix(const Mat4 &mat) {*m_viewProjectionMatrix = mat;}
void Camera::SetProjectionMatrix(const Mat4 &mat) {*m_projectionMatrix = mat;}

void Camera::GetNearPlaneBounds(float *wNear,float *hNear) const
{
	umath::frustum::get_near_plane_size(m_fovRad,*m_nearZ,m_aspectRatio,*wNear,*hNear);
}
void Camera::GetFarPlaneBounds(float *wFar,float *hFar) const
{
	umath::frustum::get_far_plane_size(m_fovRad,*m_farZ,m_aspectRatio,*wFar,*hFar);
}

void Camera::GetFarPlaneBoundaries(std::vector<Vector3> *vec,float *wFar,float *hFar) const
{
	auto &boundaries = umath::frustum::get_far_plane_boundaries(*m_pos,*m_forward,*m_up,m_fovRad,*m_farZ,m_aspectRatio,wFar,hFar);
	vec->reserve(boundaries.size());
	for(auto &v : boundaries)
		vec->push_back(v);
}

void Camera::GetNearPlaneBoundaries(std::vector<Vector3> *vec,float *wNear,float *hNear) const
{
	auto &boundaries = umath::frustum::get_near_plane_boundaries(*m_pos,*m_forward,*m_up,m_fovRad,*m_nearZ,m_aspectRatio,wNear,hNear);
	vec->reserve(boundaries.size());
	for(auto &v : boundaries)
		vec->push_back(v);
}

void Camera::GetPlaneBoundaries(std::vector<Vector3> *vecNear,std::vector<Vector3> *vecFar,float *wNear,float *hNear,float *wFar,float *hFar) const
{
	GetNearPlaneBoundaries(vecNear,wNear,hNear);
	GetFarPlaneBoundaries(vecFar,wFar,hFar);
}

const std::vector<Plane> &Camera::GetFrustumPlanes() const {return m_frustumPlanes;}

void Camera::GetFrustumPlanes(const std::vector<Vector3> &points,std::vector<Plane> &outPlanes) const
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
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_BOTTOM_LEFT)),
		points.at(umath::to_integral(FRUSTUM_POINT::FAR_BOTTOM_LEFT)),
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_TOP_LEFT)),
	});

	// Right Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_BOTTOM_RIGHT)),
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_TOP_RIGHT)),
		points.at(umath::to_integral(FRUSTUM_POINT::FAR_BOTTOM_RIGHT)),
	});

	// Top Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_BOTTOM_RIGHT)),
		points.at(umath::to_integral(FRUSTUM_POINT::FAR_BOTTOM_LEFT)),
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_BOTTOM_LEFT)),
	});

	// Bottom Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_TOP_RIGHT)),
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_TOP_LEFT)),
		points.at(umath::to_integral(FRUSTUM_POINT::FAR_TOP_LEFT)),
	});

	// Near Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_TOP_RIGHT)),
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_BOTTOM_LEFT)),
		points.at(umath::to_integral(FRUSTUM_POINT::NEAR_TOP_LEFT)),
	});

	// Far Plane
	outPlanes.push_back(Plane{
		points.at(umath::to_integral(FRUSTUM_POINT::FAR_TOP_RIGHT)),
		points.at(umath::to_integral(FRUSTUM_POINT::FAR_TOP_LEFT)),
		points.at(umath::to_integral(FRUSTUM_POINT::FAR_BOTTOM_LEFT)),
	});
}

void Camera::GetFrustumPlanes(std::vector<Plane> &outPlanes) const
{
	std::vector<Vector3> points {};
	GetFrustumPoints(points);
	GetFrustumPlanes(points,outPlanes);
}

Vector3 Camera::GetNearPlanePoint(const Vector2 &uv) const
{
	return umath::frustum::get_near_plane_point(*m_pos,*m_forward,GetRight(),*m_up,m_fovRad,*m_nearZ,m_aspectRatio,uv);
}
Vector3 Camera::GetFarPlanePoint(const Vector2 &uv) const
{
	return umath::frustum::get_far_plane_point(*m_pos,*m_forward,GetRight(),*m_up,m_fovRad,*m_farZ,m_aspectRatio,uv);
}

void Camera::GetFrustumPoints(std::vector<Vector3> &outPoints,float neard,float fard,float fov,float ratio,const Vector3 &center,const Vector3 &viewDir,const Vector3 &viewUp)
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

void Camera::GetFrustumPoints(std::vector<Vector3> &points) const
{
	GetFrustumPoints(points,GetZNear(),GetZFar(),GetFOVRad(),GetAspectRatio(),GetPos(),GetForward(),GetUp());
}

void Camera::SetFOV(float fov)
{
	*m_fov = fov;
	m_fovRad = CFloat(umath::deg_to_rad(fov));
}
void Camera::SetViewFOV(float fov)
{
	*m_fovView = fov;
	m_fovViewRad = CFloat(umath::deg_to_rad(fov));
}
void Camera::SetAspectRatio(float aspectRatio)
{
	m_aspectRatio = aspectRatio;
}
void Camera::SetZNear(float nearZ)
{
	*m_nearZ = nearZ;
}
void Camera::SetZFar(float farZ)
{
	*m_farZ = farZ;
}
void Camera::SetUp(const Vector3 &up)
{
	*m_up = up;
}
void Camera::SetForward(const Vector3 &forward)
{
	*m_forward = forward;
}
void Camera::SetPos(const Vector3 &pos)
{
	*m_pos = pos;
}
const Mat4 &Camera::GetProjectionMatrix() const {return *m_projectionMatrix;}
const Mat4 &Camera::GetViewProjectionMatrix() const {return *m_viewProjectionMatrix;}
const Mat4 &Camera::GetViewMatrix() const {return *m_viewMatrix;}
const Vector3 &Camera::GetUp() const {return *m_up;}
const Vector3 &Camera::GetPos() const {return *m_pos;}
const Vector3 &Camera::GetForward() const {return *m_forward;}
Quat Camera::GetRotation() const {return uquat::create(const_cast<Camera*>(this)->GetForward(),const_cast<Camera*>(this)->GetRight(),const_cast<Camera*>(this)->GetUp());}
Vector3 Camera::GetRight() const
{
	auto r = uvec::cross(*m_forward,*m_up);
	uvec::normalize(&r);
	return r;
}
float Camera::GetFOV() const {return *m_fov;}
float Camera::GetFOVRad() const {return m_fovRad;}
float Camera::GetViewFOV() const {return *m_fovView;}
float Camera::GetViewFOVRad() const {return m_fovViewRad;}
float Camera::GetAspectRatio() const {return m_aspectRatio;}
float Camera::GetZNear() const {return *m_nearZ;}
float Camera::GetZFar() const {return *m_farZ;}

void Camera::CreateFrustumMesh(const Vector2 &uvStart,const Vector2 &uvEnd,std::vector<Vector3> &verts,std::vector<uint16_t> &indices) const
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

void Camera::CreateFrustumKDop(const Vector2 &uvStart,const Vector2 &uvEnd,std::vector<Plane> &kDop) const
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

///////////////////////////////////////////

void Camera::GetFrustumNeighbors(FRUSTUM_PLANE planeID,FRUSTUM_PLANE *neighborIDs)
{
	static FRUSTUM_PLANE neighbors[6][4] = {
		{ // Left
			FRUSTUM_PLANE::TOP,
			FRUSTUM_PLANE::BOTTOM,
			FRUSTUM_PLANE::NEAR,
			FRUSTUM_PLANE::FAR
		},
		{ // Right
			FRUSTUM_PLANE::TOP,
			FRUSTUM_PLANE::BOTTOM,
			FRUSTUM_PLANE::NEAR,
			FRUSTUM_PLANE::FAR
		},
		{ // Top
			FRUSTUM_PLANE::LEFT,
			FRUSTUM_PLANE::RIGHT,
			FRUSTUM_PLANE::NEAR,
			FRUSTUM_PLANE::FAR
		},
		{ // Bottom
			FRUSTUM_PLANE::LEFT,
			FRUSTUM_PLANE::RIGHT,
			FRUSTUM_PLANE::NEAR,
			FRUSTUM_PLANE::FAR
		},
		{ // Near
			FRUSTUM_PLANE::LEFT,
			FRUSTUM_PLANE::RIGHT,
			FRUSTUM_PLANE::TOP,
			FRUSTUM_PLANE::BOTTOM
		},
		{ // Far
			FRUSTUM_PLANE::LEFT,
			FRUSTUM_PLANE::RIGHT,
			FRUSTUM_PLANE::TOP,
			FRUSTUM_PLANE::BOTTOM
		}
	};
	for(unsigned int i=0;i<4;i++)
		neighborIDs[i] = neighbors[static_cast<int>(planeID)][i];
}

void Camera::GetFrustumPlaneCornerPoints(FRUSTUM_PLANE planeA,FRUSTUM_PLANE planeB,FRUSTUM_POINT *cornerPoints)
{
	static FRUSTUM_POINT points[6][6][2] = {
		{ // Left
			{ // Left
				FRUSTUM_POINT::FAR_BOTTOM_LEFT,FRUSTUM_POINT::FAR_BOTTOM_LEFT // Invalid
			},
			{ // Right
				FRUSTUM_POINT::FAR_BOTTOM_LEFT,FRUSTUM_POINT::FAR_BOTTOM_LEFT // Invalid
			},
			{ // Top
				FRUSTUM_POINT::NEAR_TOP_LEFT,FRUSTUM_POINT::FAR_TOP_LEFT
			},
			{ // Bottom
				FRUSTUM_POINT::FAR_BOTTOM_LEFT,FRUSTUM_POINT::NEAR_BOTTOM_LEFT
			},
			{ // Near
				FRUSTUM_POINT::NEAR_BOTTOM_LEFT,FRUSTUM_POINT::NEAR_TOP_LEFT
			},
			{ // Far
				FRUSTUM_POINT::FAR_TOP_LEFT,FRUSTUM_POINT::FAR_BOTTOM_LEFT
			}
		},
		{ // Right
			{ // Left
				FRUSTUM_POINT::FAR_BOTTOM_RIGHT,FRUSTUM_POINT::FAR_BOTTOM_RIGHT // Invalid
			},
			{ // Right
				FRUSTUM_POINT::FAR_BOTTOM_RIGHT,FRUSTUM_POINT::FAR_BOTTOM_RIGHT // Invalid
			},
			{ // Top
				FRUSTUM_POINT::FAR_TOP_RIGHT,FRUSTUM_POINT::NEAR_TOP_RIGHT
			},
			{ // Bottom
				FRUSTUM_POINT::NEAR_BOTTOM_RIGHT,FRUSTUM_POINT::FAR_BOTTOM_RIGHT
			},
			{ // Near
				FRUSTUM_POINT::NEAR_TOP_RIGHT,FRUSTUM_POINT::NEAR_BOTTOM_RIGHT
			},
			{ // Far
				FRUSTUM_POINT::FAR_BOTTOM_RIGHT,FRUSTUM_POINT::FAR_TOP_RIGHT
			}
		},
		{ // Top
			{ // Left
				FRUSTUM_POINT::FAR_TOP_LEFT,FRUSTUM_POINT::NEAR_TOP_LEFT
			},
			{ // Right
				FRUSTUM_POINT::NEAR_TOP_RIGHT,FRUSTUM_POINT::FAR_TOP_RIGHT
			},
			{ // Top
				FRUSTUM_POINT::NEAR_TOP_LEFT,FRUSTUM_POINT::FAR_TOP_LEFT // Invalid
			},
			{ // Bottom
				FRUSTUM_POINT::FAR_BOTTOM_LEFT,FRUSTUM_POINT::NEAR_BOTTOM_LEFT // Invalid
			},
			{ // Near
				FRUSTUM_POINT::NEAR_TOP_LEFT,FRUSTUM_POINT::NEAR_TOP_RIGHT
			},
			{ // Far
				FRUSTUM_POINT::FAR_TOP_RIGHT,FRUSTUM_POINT::FAR_TOP_LEFT
			}
		},
		{ // Bottom
			{ // Left
				FRUSTUM_POINT::NEAR_BOTTOM_LEFT,FRUSTUM_POINT::FAR_BOTTOM_LEFT
			},
			{ // Right
				FRUSTUM_POINT::FAR_BOTTOM_RIGHT,FRUSTUM_POINT::NEAR_BOTTOM_RIGHT
			},
			{ // Top
				FRUSTUM_POINT::NEAR_BOTTOM_LEFT,FRUSTUM_POINT::FAR_BOTTOM_LEFT // Invalid
			},
			{ // Bottom
				FRUSTUM_POINT::FAR_BOTTOM_LEFT,FRUSTUM_POINT::NEAR_BOTTOM_LEFT // Invalid
			},
			{ // Near
				FRUSTUM_POINT::NEAR_BOTTOM_RIGHT,FRUSTUM_POINT::NEAR_BOTTOM_LEFT
			},
			{ // Far
				FRUSTUM_POINT::FAR_BOTTOM_LEFT,FRUSTUM_POINT::FAR_BOTTOM_RIGHT
			}
		},
		{ // Near
			{ // Left
				FRUSTUM_POINT::NEAR_TOP_LEFT,FRUSTUM_POINT::NEAR_BOTTOM_LEFT
			},
			{ // Right
				FRUSTUM_POINT::NEAR_BOTTOM_RIGHT,FRUSTUM_POINT::NEAR_TOP_RIGHT
			},
			{ // Top
				FRUSTUM_POINT::NEAR_TOP_RIGHT,FRUSTUM_POINT::NEAR_TOP_LEFT
			},
			{ // Bottom
				FRUSTUM_POINT::NEAR_BOTTOM_LEFT,FRUSTUM_POINT::NEAR_BOTTOM_RIGHT
			},
			{ // Near
				FRUSTUM_POINT::NEAR_TOP_LEFT,FRUSTUM_POINT::NEAR_TOP_RIGHT
			},
			{ // Far
				FRUSTUM_POINT::FAR_TOP_RIGHT,FRUSTUM_POINT::FAR_TOP_LEFT
			},
		},
		{ // Far
			{ // Left
				FRUSTUM_POINT::FAR_BOTTOM_LEFT,FRUSTUM_POINT::FAR_TOP_LEFT
			},
			{ // Right
				FRUSTUM_POINT::FAR_TOP_RIGHT,FRUSTUM_POINT::FAR_BOTTOM_RIGHT
			},
			{ // Top
				FRUSTUM_POINT::FAR_TOP_LEFT,FRUSTUM_POINT::FAR_TOP_RIGHT
			},
			{ // Bottom
				FRUSTUM_POINT::FAR_BOTTOM_RIGHT,FRUSTUM_POINT::FAR_BOTTOM_LEFT
			},
			{ // Near
				FRUSTUM_POINT::FAR_TOP_LEFT,FRUSTUM_POINT::FAR_TOP_RIGHT
			},
			{ // Far
				FRUSTUM_POINT::FAR_TOP_RIGHT,FRUSTUM_POINT::FAR_TOP_LEFT
			}
		}
	};
	cornerPoints[0] = points[static_cast<int>(planeA)][static_cast<int>(planeB)][0];
	cornerPoints[1] = points[static_cast<int>(planeA)][static_cast<int>(planeB)][1];
}

void Camera::CreateFrustumKDop(const std::vector<Plane> &planes,const std::vector<Vector3> &points,const Vector3 &dir,std::vector<Plane> *kDop)
{
	float fDir[6];
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
			FRUSTUM_PLANE neighbors[4];
			GetFrustumNeighbors(FRUSTUM_PLANE(i),&neighbors[0]);
			for(unsigned j=0;j<4;j++)
			{
				float fNDir = fDir[static_cast<int>(neighbors[j])];
				if(fNDir > EPSILON)
				{
					FRUSTUM_POINT corners[2];
					GetFrustumPlaneCornerPoints(FRUSTUM_PLANE(i),neighbors[j],&corners[0]);
					Plane p(points[static_cast<int>(corners[0])],points[static_cast<int>(corners[1])],points[static_cast<int>(corners[0])] -dir);
					kDop->push_back(p);
				}
			}
		}
	}
}
