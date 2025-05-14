/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_CAMERA_H__
#define __ENV_CAMERA_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>
#include <sharedutils/property/util_property_vector.h>
#include <sharedutils/property/util_property_matrix.hpp>
#include <mathutil/plane.hpp>

enum class FrustumPlane : uint32_t;
enum class FrustumPoint : uint32_t;
namespace pragma {
	namespace rendering {
		struct Tile;
	};
	class BaseFieldAngleComponent;
	class DLLNETWORK BaseEnvCameraComponent : public BaseEntityComponent {
	  public:
		static const float DEFAULT_NEAR_Z;
		static const float DEFAULT_FAR_Z;
		static const float DEFAULT_FOV;
		static const float DEFAULT_VIEWMODEL_FOV;
		static const float DEFAULT_FOCAL_DISTANCE;

		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		BaseEnvCameraComponent(BaseEntity &ent);
		virtual void Initialize() override;

		enum class StateFlags : uint32_t { None = 0u, ViewMatrixDirtyBit = 1u, ProjectionMatrixDirtyBit = ViewMatrixDirtyBit << 1u, CustomViewMatrix = ProjectionMatrixDirtyBit << 1u, CustomProjectionMatrix = CustomViewMatrix << 1u };

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

		void UpdateMatrices();
		void UpdateViewMatrix();
		void UpdateProjectionMatrix();
		void SetViewMatrix(const Mat4 &mat);
		void SetProjectionMatrix(const Mat4 &mat);
		// Returns the six sides of the frustum in the order near,far,left,right,top,bottom
		void GetFrustumPlanes(std::vector<umath::Plane> &outPlanes) const;
		void GetFrustumPoints(std::vector<Vector3> &outPoints) const;
		Vector3 GetFarPlaneCenter() const;
		Vector3 GetNearPlaneCenter() const;
		Vector3 GetPlaneCenter(float z) const;
		void GetNearPlaneBounds(float *wNear, float *hNear) const;
		void GetFarPlaneBounds(float *wFar, float *hFar) const;
		void GetPlaneBounds(float z, float &outW, float &outH) const;
		// Returns the boundary vectors for the far plane in the order Top left,Top right,Bottom left,Bottom right
		void GetFarPlaneBoundaries(std::array<Vector3, 4> &outPoints, float *wFar = nullptr, float *hFar = nullptr) const;
		// Returns the boundary vectors for the near plane in the order Top left,Top right,Bottom left,Bottom right
		void GetNearPlaneBoundaries(std::array<Vector3, 4> &outPoints, float *wNear = nullptr, float *hNear = nullptr) const;
		// Returns both the boundary vectors for the far and near planes in the order Top left,Top right,Bottom left,Bottom right
		void GetPlaneBoundaries(std::array<Vector3, 8> &outPoints, float *wNear = nullptr, float *hNear = nullptr, float *wFar = nullptr, float *hFar = nullptr) const;
		void GetPlaneBoundaries(float z, std::array<Vector3, 4> &outPoints, float *wNear = nullptr, float *hNear = nullptr, float *wFar = nullptr, float *hFar = nullptr) const;

		void SetOrientation(const Vector3 &forward, const Vector3 &up);

		void SetFOV(float fov);
		void SetAspectRatio(float aspectRatio);
		void SetNearZ(float nearZ);
		void SetFarZ(float farZ);
		void SetFocalDistance(float focalDistance);
		const Mat4 &GetProjectionMatrix() const;
		const Mat4 &GetViewMatrix() const;

		float GetFOV() const;
		float GetFOVRad() const;

		const util::PMatrix4Property &GetProjectionMatrixProperty() const;
		const util::PMatrix4Property &GetViewMatrixProperty() const;

		const util::PFloatProperty &GetAspectRatioProperty() const;
		const util::PFloatProperty &GetNearZProperty() const;
		const util::PFloatProperty &GetFarZProperty() const;
		const util::PFloatProperty &GetFOVProperty() const;
		const util::PFloatProperty &GetFocalDistanceProperty() const;

		float GetAspectRatio() const;
		float GetNearZ() const;
		float GetFarZ() const;
		float GetFocalDistance() const;
		void UpdateFrustumPlanes();
		const std::vector<umath::Plane> &GetFrustumPlanes() const;
		Vector3 GetNearPlanePoint(const Vector2 &uv) const;
		Vector3 GetFarPlanePoint(const Vector2 &uv) const;
		Vector3 GetPlanePoint(float z, const Vector2 &uv) const;

		void CreateFrustumMesh(const Vector2 &uvStart, const Vector2 &uvEnd, std::vector<Vector3> &verts, std::vector<uint16_t> &indices) const;
		void CreateFrustumKDop(const Vector2 &uvStart, const Vector2 &uvEnd, std::vector<umath::Plane> &kDop) const;

		static void GetFrustumPoints(std::vector<Vector3> &outPoints, float neard, float fard, float fov, float ratio, const Vector3 &center, const Vector3 &viewDir, const Vector3 &viewUp);
		static void GetFrustumNeighbors(FrustumPlane planeID, FrustumPlane *neighborIDs);
		static void GetFrustumPlaneCornerPoints(FrustumPlane planeA, FrustumPlane planeB, FrustumPoint *cornerPoints);
		static void CreateFrustumKDop(const std::vector<umath::Plane> &planes, const std::vector<Vector3> &points, const Vector3 &dir, std::vector<umath::Plane> *kDop);
		static Mat4 CalcProjectionMatrix(umath::Radian fovRad, float aspectRatio, float nearZ, float farZ, const rendering::Tile *optTile = nullptr);
		static void GetFrustumPlanes(const std::vector<Vector3> &points, std::vector<umath::Plane> &outPlanes);
		static void GetFrustumPlanes(std::vector<umath::Plane> &outPlanes, float neard, float fard, float fov, float ratio, const Vector3 &center, const Vector3 &viewDir, const Vector3 &viewUp);
	  protected:
		void FlagViewMatrixAsDirty();
		void FlagProjectionMatrixAsDirty();
		virtual void SetFieldAngleComponent(BaseFieldAngleComponent &c);
	  private:
		util::PMatrix4Property m_projectionMatrix = nullptr;
		util::PMatrix4Property m_viewMatrix = nullptr;
		util::PFloatProperty m_aspectRatio = nullptr;
		util::PFloatProperty m_nearZ = nullptr;
		util::PFloatProperty m_farZ = nullptr;
		util::PFloatProperty m_focalDistance = nullptr;
		std::vector<umath::Plane> m_frustumPlanes;
		StateFlags m_stateFlags = StateFlags::None;
		ComponentHandle<BaseFieldAngleComponent> m_fieldAngleComponent;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseEnvCameraComponent::StateFlags)

#endif
