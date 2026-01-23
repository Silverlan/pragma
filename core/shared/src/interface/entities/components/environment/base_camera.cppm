// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.environment.base_camera;

export import :entities.components.base;
import :entities.components.base_field_angle;
export import :math.frustum;
import :util.render_tile;

export {
	namespace pragma {
		namespace baseEnvCameraComponent {
#ifdef WINDOWS_CLANG_COMPILER_FIX
			DLLNETWORK const float &DEFAULT_NEAR_Z;
			DLLNETWORK const float &DEFAULT_FAR_Z;
			DLLNETWORK const float &DEFAULT_FOV;
			DLLNETWORK const float &DEFAULT_VIEWMODEL_FOV;
			DLLNETWORK const float &DEFAULT_FOCAL_DISTANCE;
#else
			CLASS_ENUM_COMPAT const float DEFAULT_NEAR_Z;
			CLASS_ENUM_COMPAT const float DEFAULT_FAR_Z;
			CLASS_ENUM_COMPAT const float DEFAULT_FOV;
			CLASS_ENUM_COMPAT const float DEFAULT_VIEWMODEL_FOV;
			CLASS_ENUM_COMPAT const float DEFAULT_FOCAL_DISTANCE;
#endif
		}
		class DLLNETWORK BaseEnvCameraComponent : public BaseEntityComponent {
		  public:
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
			BaseEnvCameraComponent(ecs::BaseEntity &ent);
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
			void GetFrustumPlanes(std::vector<math::Plane> &outPlanes) const;
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
			const std::vector<math::Plane> &GetFrustumPlanes() const;
			Vector3 GetNearPlanePoint(const Vector2 &uv) const;
			Vector3 GetFarPlanePoint(const Vector2 &uv) const;
			Vector3 GetPlanePoint(float z, const Vector2 &uv) const;

			void CreateFrustumMesh(const Vector2 &uvStart, const Vector2 &uvEnd, std::vector<Vector3> &verts, std::vector<uint16_t> &indices) const;
			void CreateFrustumKDop(const Vector2 &uvStart, const Vector2 &uvEnd, std::vector<math::Plane> &kDop) const;

			static void GetFrustumPoints(std::vector<Vector3> &outPoints, float neard, float fard, float fov, float ratio, const Vector3 &center, const Vector3 &viewDir, const Vector3 &viewUp);
			static void GetFrustumNeighbors(math::FrustumPlane planeID, math::FrustumPlane *neighborIDs);
			static void GetFrustumPlaneCornerPoints(math::FrustumPlane planeA, math::FrustumPlane planeB, math::FrustumPoint *cornerPoints);
			static void CreateFrustumKDop(const std::vector<math::Plane> &planes, const std::vector<Vector3> &points, const Vector3 &dir, std::vector<math::Plane> *kDop);
			static Mat4 CalcProjectionMatrix(math::Radian fovRad, float aspectRatio, float nearZ, float farZ, const rendering::Tile *optTile = nullptr);
			static void GetFrustumPlanes(const std::vector<Vector3> &points, std::vector<math::Plane> &outPlanes);
			static void GetFrustumPlanes(std::vector<math::Plane> &outPlanes, float neard, float fard, float fov, float ratio, const Vector3 &center, const Vector3 &viewDir, const Vector3 &viewUp);
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
			std::vector<math::Plane> m_frustumPlanes;
			StateFlags m_stateFlags = StateFlags::None;
			ComponentHandle<BaseFieldAngleComponent> m_fieldAngleComponent;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseEnvCameraComponent::StateFlags)
};
