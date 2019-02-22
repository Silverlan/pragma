#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <mathutil/glmutil.h>
#include "pragma/clientdefinitions.h"
#include <sharedutils/def_handle.h>
#include <vector>
#include <memory>
#include <sharedutils/property/util_property.hpp>
#include <sharedutils/property/util_property_vector.h>
#include <sharedutils/property/util_property_matrix.hpp>
#include <pragma/math/plane.h>

struct Camera;
DECLARE_BASE_HANDLE(DLLCLIENT,Camera,Camera);

enum class FRUSTUM_PLANE : int;
enum class FRUSTUM_POINT : int;
#pragma warning(push)
#pragma warning(disable : 4251)
struct DLLCLIENT Camera
	: public std::enable_shared_from_this<Camera>
{
public:
	friend CameraHandle;
private:
	Camera(float fov,float fovView,float aspectRatio,float nearZ,float farZ);
	Camera(const Camera &other);
	CameraHandle m_handle;
	mutable util::PMatrix4Property m_projectionMatrix = nullptr;
	mutable util::PMatrix4Property m_viewProjectionMatrix = nullptr;
	mutable util::PMatrix4Property m_viewMatrix = nullptr;
	mutable util::PVector3Property m_up = nullptr;
	mutable util::PVector3Property m_forward = nullptr;
	mutable util::PVector3Property m_pos = nullptr;
	mutable util::PFloatProperty m_fov = nullptr;
	mutable util::PFloatProperty m_fovView = nullptr;
	float m_fovRad;
	float m_fovViewRad;
	float m_aspectRatio;
	mutable util::PFloatProperty m_nearZ = nullptr;
	mutable util::PFloatProperty m_farZ = nullptr;
	std::vector<Plane> m_frustumPlanes;
public:
	static std::shared_ptr<Camera> Create(float fov,float fovView,float aspectRatio,float nearZ,float farZ);
	static std::shared_ptr<Camera> Create(const Camera &other);
	~Camera();
	CameraHandle GetHandle() const;
	void UpdateMatrices();
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();
	void UpdateViewProjectionMatrix();
	void UpdateProjectionMatrices();
	void SetViewMatrix(const Mat4 &mat);
	void SetViewProjectionMatrix(const Mat4 &mat);
	void SetProjectionMatrix(const Mat4 &mat);
	// Returns the six sides of the frustum in the order near,far,left,right,top,bottom
	void GetFrustumPlanes(std::vector<Plane> &outPlanes) const;
	void GetFrustumPlanes(const std::vector<Vector3> &points,std::vector<Plane> &outPlanes) const;
	void GetFrustumPoints(std::vector<Vector3> &outPoints) const;
	Vector3 GetFarPlaneCenter() const;
	Vector3 GetNearPlaneCenter() const;
	void GetNearPlaneBounds(float *wNear,float *hNear) const;
	void GetFarPlaneBounds(float *wFar,float *hFar) const;
	// Returns the boundary vectors for the far plane in the order Top left,Top right,Bottom left,Bottom right
	void GetFarPlaneBoundaries(std::vector<Vector3> *vec,float *wFar=NULL,float *hFar=NULL) const;
	// Returns the boundary vectors for the near plane in the order Top left,Top right,Bottom left,Bottom right
	void GetNearPlaneBoundaries(std::vector<Vector3> *vec,float *wNear=NULL,float *hNear=NULL) const;
	// Returns both the boundary vectors for the far and near planes in the order Top left,Top right,Bottom left,Bottom right
	void GetPlaneBoundaries(std::vector<Vector3> *vecNear,std::vector<Vector3> *vecFar,float *wNear=NULL,float *hNear=NULL,float *wFar=NULL,float *hFar=NULL) const;
	void SetFOV(float fov);
	void SetViewFOV(float fov);
	void SetAspectRatio(float aspectRatio);
	void SetZNear(float nearZ);
	void SetZFar(float farZ);
	void SetUp(const Vector3 &up);
	void SetForward(const Vector3 &forward);
	void SetPos(const Vector3 &pos);
	const Mat4 &GetProjectionMatrix() const;
	const Mat4 &GetViewProjectionMatrix() const;
	const Mat4 &GetViewMatrix() const;
	const Vector3 &GetUp() const;
	const Vector3 &GetPos() const;
	const Vector3 &GetForward() const;
	Quat GetRotation() const;
	Vector3 GetRight() const;
	float GetFOV() const;
	float GetFOVRad() const;
	float GetViewFOV() const;
	float GetViewFOVRad() const;

	const util::PMatrix4Property &GetProjectionMatrixProperty() const;
	const util::PMatrix4Property &GetViewProjectionMatrixProperty() const;
	const util::PMatrix4Property &GetViewMatrixProperty() const;

	const util::PVector3Property &GetUpProperty() const;
	const util::PVector3Property &GetForwardProperty() const;
	const util::PVector3Property &GetPosProperty() const;

	const util::PFloatProperty &GetNearZProperty() const;
	const util::PFloatProperty &GetFarZProperty() const;
	const util::PFloatProperty &GetFOVProperty() const;
	const util::PFloatProperty &GetViewFOVProperty() const;

	float GetAspectRatio() const;
	float GetZNear() const;
	float GetZFar() const;
	void UpdateFrustumPlanes();
	const std::vector<Plane> &GetFrustumPlanes() const;
	Vector3 GetNearPlanePoint(const Vector2 &uv) const;
	Vector3 GetFarPlanePoint(const Vector2 &uv) const;

	void CreateFrustumMesh(const Vector2 &uvStart,const Vector2 &uvEnd,std::vector<Vector3> &verts,std::vector<uint16_t> &indices) const;
	void CreateFrustumKDop(const Vector2 &uvStart,const Vector2 &uvEnd,std::vector<Plane> &kDop) const;

	static void GetFrustumPoints(std::vector<Vector3> &outPoints,float neard,float fard,float fov,float ratio,const Vector3 &center,const Vector3 &viewDir,const Vector3 &viewUp);
	static void GetFrustumNeighbors(FRUSTUM_PLANE planeID,FRUSTUM_PLANE *neighborIDs);
	static void GetFrustumPlaneCornerPoints(FRUSTUM_PLANE planeA,FRUSTUM_PLANE planeB,FRUSTUM_POINT *cornerPoints);
	static void CreateFrustumKDop(const std::vector<Plane> &planes,const std::vector<Vector3> &points,const Vector3 &dir,std::vector<Plane> *kDop);
};
#pragma warning(pop)
#endif
