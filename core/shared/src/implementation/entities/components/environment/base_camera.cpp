// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.environment.base_camera;

#define EPSILON 0.001f

using namespace pragma;

static float normalize_plane_z(float z) { return math::max(z, 0.1f); } // z value must never be 0; values close to zero can cause visual artifacts

void BaseEnvCameraComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseEnvCameraComponent;
	{
		auto memberInfo = create_component_member_info<T, float, static_cast<void (T::*)(float)>(&T::SetFOV), static_cast<float (T::*)() const>(&T::GetFOV)>("fov", baseEnvCameraComponent::DEFAULT_FOV, AttributeSpecializationType::Angle);
		memberInfo.SetMin(0.01f);
		memberInfo.SetMax(179.99f);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float, static_cast<void (T::*)(float)>(&T::SetNearZ), static_cast<float (T::*)() const>(&T::GetNearZ)>("nearz", baseEnvCameraComponent::DEFAULT_NEAR_Z, AttributeSpecializationType::Distance);
		memberInfo.SetMin(0.1f);
		memberInfo.updateDependenciesFunction = [](BaseEntityComponent &component, std::vector<std::string> &outAffectedProps) {
			auto &c = static_cast<T &>(component);
			auto farZ = c.GetFarZ();
			auto nearZ = c.GetNearZ();
			if(nearZ >= farZ) {
				c.SetFarZ(nearZ + 0.01f);
				outAffectedProps.push_back("farz");
			}
		};
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float, static_cast<void (T::*)(float)>(&T::SetFarZ), static_cast<float (T::*)() const>(&T::GetFarZ)>("farz", baseEnvCameraComponent::DEFAULT_FAR_Z, AttributeSpecializationType::Distance);
		memberInfo.SetMin(0.1f);
		memberInfo.updateDependenciesFunction = [](BaseEntityComponent &component, std::vector<std::string> &outAffectedProps) {
			auto &c = static_cast<T &>(component);
			auto farZ = c.GetFarZ();
			auto nearZ = c.GetNearZ();
			if(farZ < nearZ) {
				c.SetNearZ(farZ - 0.01f);
				outAffectedProps.push_back("nearz");
			}
		};
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float, static_cast<void (T::*)(float)>(&T::SetFocalDistance), static_cast<float (T::*)() const>(&T::GetFocalDistance)>("focalDistance", baseEnvCameraComponent::DEFAULT_FOCAL_DISTANCE, AttributeSpecializationType::Distance);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float, static_cast<void (T::*)(float)>(&T::SetAspectRatio), static_cast<float (T::*)() const>(&T::GetAspectRatio)>("aspectRatio", 1.f);
		memberInfo.SetFlag(ComponentMemberFlags::HideInInterface);
		registerMember(std::move(memberInfo));
	}
}

#ifdef WINDOWS_CLANG_COMPILER_FIX
const float &baseEnvCameraComponent::DEFAULT_NEAR_Z {
	static float val = 1.f;
	return val;
}
const float &baseEnvCameraComponent::DEFAULT_FAR_Z {
	static float val = 32'768.f;
	return val;
}
const float &baseEnvCameraComponent::DEFAULT_FOV {
	static float val = 90.f;
	return val;
}
const float &baseEnvCameraComponent::DEFAULT_VIEWMODEL_FOV {
	static float val = 70.f;
	return val;
}
const float &baseEnvCameraComponent::DEFAULT_FOCAL_DISTANCE {
	static float val = 72.f;
	return val;
}
#else
decltype(baseEnvCameraComponent::DEFAULT_NEAR_Z) baseEnvCameraComponent::DEFAULT_NEAR_Z = 1.f;
decltype(baseEnvCameraComponent::DEFAULT_FAR_Z) baseEnvCameraComponent::DEFAULT_FAR_Z = 32'768.f;
decltype(baseEnvCameraComponent::DEFAULT_FOV) baseEnvCameraComponent::DEFAULT_FOV = 90.f;
decltype(baseEnvCameraComponent::DEFAULT_VIEWMODEL_FOV) baseEnvCameraComponent::DEFAULT_VIEWMODEL_FOV = 70.f;
decltype(baseEnvCameraComponent::DEFAULT_FOCAL_DISTANCE) baseEnvCameraComponent::DEFAULT_FOCAL_DISTANCE = 72.f;
#endif
BaseEnvCameraComponent::BaseEnvCameraComponent(ecs::BaseEntity &ent)
    : BaseEntityComponent {ent}, m_nearZ(util::FloatProperty::Create(baseEnvCameraComponent::DEFAULT_NEAR_Z)), m_farZ(util::FloatProperty::Create(baseEnvCameraComponent::DEFAULT_FAR_Z)), m_focalDistance(util::FloatProperty::Create(baseEnvCameraComponent::DEFAULT_FOCAL_DISTANCE)),
      m_projectionMatrix(util::Matrix4Property::Create()), m_viewMatrix(util::Matrix4Property::Create()), m_aspectRatio(util::FloatProperty::Create(1.f))
{
}
void BaseEnvCameraComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("toggle");
	ent.AddComponent("transform");
	SetFieldAngleComponent(*ent.AddComponent("field_angle").get<BaseFieldAngleComponent>());

	SetFOV(75.f);
}

void BaseEnvCameraComponent::SetFieldAngleComponent(BaseFieldAngleComponent &c)
{
	m_fieldAngleComponent = c.GetHandle<BaseFieldAngleComponent>();
	c.GetFieldAngleProperty()->AddCallback([this](std::reference_wrapper<const float> oldAng, std::reference_wrapper<const float> newAng) { m_stateFlags |= StateFlags::ProjectionMatrixDirtyBit; });
}

void BaseEnvCameraComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["projectionMatrix"] = **m_projectionMatrix;
	udm["viewMatrix"] = **m_viewMatrix;
	udm["aspectRatio"] = **m_aspectRatio;
	udm["nearZ"] = **m_nearZ;
	udm["farZ"] = **m_farZ;
	auto dof = udm["depthOfField"];
	dof["focalDistance"] = **m_focalDistance;
}
void BaseEnvCameraComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	udm["projectionMatrix"](**m_projectionMatrix);
	udm["viewMatrix"](**m_viewMatrix);
	udm["aspectRatio"](**m_aspectRatio);
	udm["nearZ"](**m_nearZ);
	udm["farZ"](**m_farZ);
	auto dof = udm["depthOfField"];
	dof["focalDistance"](**m_farZ);
}

void BaseEnvCameraComponent::SetOrientation(const Vector3 &forward, const Vector3 &up)
{
	auto trComponent = GetEntity().GetTransformComponent();
	if(!trComponent)
		return;
	auto right = uvec::cross(forward, up);
	uvec::normalize(&right);
	trComponent->SetRotation(uquat::create(forward, right, up));
}

void BaseEnvCameraComponent::UpdateMatrices()
{
	UpdateProjectionMatrix();
	UpdateViewMatrix();
}
void BaseEnvCameraComponent::UpdateViewMatrix()
{
	auto whTrComponent = GetEntity().GetTransformComponent();
	if(!whTrComponent)
		return;
	auto &pos = whTrComponent->GetPosition();
	*m_viewMatrix = glm::gtc::lookAtRH(pos, pos + whTrComponent->GetForward(), whTrComponent->GetUp());
	math::set_flag(m_stateFlags, StateFlags::ViewMatrixDirtyBit, false);
	math::set_flag(m_stateFlags, StateFlags::CustomViewMatrix, false);
}
void BaseEnvCameraComponent::UpdateProjectionMatrix()
{
	*m_projectionMatrix = CalcProjectionMatrix(GetFOVRad(), *m_aspectRatio, normalize_plane_z(**m_nearZ), normalize_plane_z(**m_farZ));
	math::set_flag(m_stateFlags, StateFlags::ProjectionMatrixDirtyBit, false);
	math::set_flag(m_stateFlags, StateFlags::CustomProjectionMatrix, false);
}
void BaseEnvCameraComponent::SetViewMatrix(const Mat4 &mat)
{
	*m_viewMatrix = mat;
	m_stateFlags |= StateFlags::CustomViewMatrix;
	auto whTrComponent = GetEntity().GetTransformComponent();
	if(whTrComponent) {
		// Vector3 forward,right,up;
		// umat::to_axes(mat,forward,right,up);
		// whTrComponent->SetOrientation(uquat::create(forward,right,up));
		Vector3 scale;
		Quat rotation;
		Vector3 translation;
		Vector3 skew;
		Vector4 perspective;
		glm::gtx::decompose(glm::inverse(mat), scale, rotation, translation, skew, perspective);

		// Invert forward vector
		auto forward = -uquat::forward(rotation);
		auto up = uquat::up(rotation);
		auto right = uvec::cross(forward, up);
		uvec::normalize(&right);
		rotation = uquat::create(forward, right, up);

		whTrComponent->SetPosition(translation);
		whTrComponent->SetRotation(rotation);
	}
}
void BaseEnvCameraComponent::SetProjectionMatrix(const Mat4 &mat)
{
	*m_projectionMatrix = mat;
	m_stateFlags |= StateFlags::CustomProjectionMatrix;
}
void BaseEnvCameraComponent::GetFrustumPlanes(std::vector<math::Plane> &outPlanes) const
{
	std::vector<Vector3> points {};
	GetFrustumPoints(points);
	GetFrustumPlanes(points, outPlanes);
}
void BaseEnvCameraComponent::GetFrustumPlanes(std::vector<math::Plane> &outPlanes, float neard, float fard, float fov, float ratio, const Vector3 &center, const Vector3 &viewDir, const Vector3 &viewUp)
{
	std::vector<Vector3> points;
	GetFrustumPoints(points, neard, fard, fov, ratio, center, viewDir, viewUp);
	return GetFrustumPlanes(points, outPlanes);
}
void BaseEnvCameraComponent::GetFrustumPlanes(const std::vector<Vector3> &points, std::vector<math::Plane> &outPlanes)
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

	outPlanes.reserve(outPlanes.size() + 6u);
	// Left Plane
	outPlanes.push_back(math::Plane {
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomLeft)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearTopLeft)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomLeft)),
	});

	// Right Plane
	outPlanes.push_back(math::Plane {
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomRight)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomRight)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearTopRight)),
	});

	// Top Plane
	outPlanes.push_back(math::Plane {
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomRight)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomLeft)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomLeft)),
	});

	// Bottom Plane
	outPlanes.push_back(math::Plane {
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearTopRight)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::FarTopLeft)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearTopLeft)),
	});

	// Near Plane
	outPlanes.push_back(math::Plane {
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearTopRight)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearTopLeft)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::NearBottomLeft)),
	});

	// Far Plane
	outPlanes.push_back(math::Plane {
	  points.at(pragma::math::to_integral(math::FrustumPoint::FarTopRight)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::FarBottomLeft)),
	  points.at(pragma::math::to_integral(math::FrustumPoint::FarTopLeft)),
	});
}
void BaseEnvCameraComponent::GetFrustumPoints(std::vector<Vector3> &outPoints) const
{
	auto *trComponent = GetEntity().GetTransformComponent();
	auto pos = trComponent ? trComponent->GetPosition() : Vector3 {};
	auto forward = trComponent ? trComponent->GetForward() : uvec::PRM_FORWARD;
	auto up = trComponent ? trComponent->GetUp() : uvec::PRM_UP;
	GetFrustumPoints(outPoints, GetNearZ(), GetFarZ(), GetFOVRad(), GetAspectRatio(), pos, forward, up);
}
Vector3 BaseEnvCameraComponent::GetFarPlaneCenter() const { return GetPlaneCenter(*m_farZ); }
Vector3 BaseEnvCameraComponent::GetNearPlaneCenter() const { return GetPlaneCenter(*m_nearZ); }
Vector3 BaseEnvCameraComponent::GetPlaneCenter(float z) const
{
	auto *trComponent = GetEntity().GetTransformComponent();
	auto pos = trComponent ? trComponent->GetPosition() : Vector3 {};
	auto forward = trComponent ? trComponent->GetForward() : uvec::PRM_FORWARD;
	return math::frustum::get_plane_center(pos, forward, z);
}
void BaseEnvCameraComponent::GetNearPlaneBounds(float *wNear, float *hNear) const { GetPlaneBounds(*m_nearZ, *wNear, *hNear); }
void BaseEnvCameraComponent::GetFarPlaneBounds(float *wFar, float *hFar) const { GetPlaneBounds(*m_farZ, *wFar, *hFar); }
void BaseEnvCameraComponent::GetPlaneBounds(float z, float &outW, float &outH) const { math::frustum::get_plane_size(GetFOVRad(), z, *m_aspectRatio, outW, outH); }
void BaseEnvCameraComponent::GetFarPlaneBoundaries(std::array<Vector3, 4> &outPoints, float *wFar, float *hFar) const { GetPlaneBoundaries(*m_farZ, outPoints, wFar, hFar); }
void BaseEnvCameraComponent::GetNearPlaneBoundaries(std::array<Vector3, 4> &outPoints, float *wNear, float *hNear) const { GetPlaneBoundaries(*m_nearZ, outPoints, wNear, hNear); }
void BaseEnvCameraComponent::GetPlaneBoundaries(std::array<Vector3, 8> &outPoints, float *wNear, float *hNear, float *wFar, float *hFar) const
{
	std::array<Vector3, 4> tmpPoints;
	GetNearPlaneBoundaries(tmpPoints, wNear, hNear);
	for(auto i = decltype(tmpPoints.size()) {0u}; i < tmpPoints.size(); ++i)
		outPoints.at(i) = tmpPoints.at(i);

	GetFarPlaneBoundaries(tmpPoints, wFar, hFar);
	for(auto i = decltype(tmpPoints.size()) {0u}; i < tmpPoints.size(); ++i)
		outPoints.at(i + tmpPoints.size()) = tmpPoints.at(i);
}
void BaseEnvCameraComponent::GetPlaneBoundaries(float z, std::array<Vector3, 4> &outPoints, float *wNear, float *hNear, float *wFar, float *hFar) const
{
	auto *trComponent = GetEntity().GetTransformComponent();
	auto pos = trComponent ? trComponent->GetPosition() : Vector3 {};
	auto forward = trComponent ? trComponent->GetForward() : uvec::PRM_FORWARD;
	auto up = trComponent ? trComponent->GetUp() : uvec::PRM_UP;
	outPoints = math::frustum::get_plane_boundaries(pos, forward, up, GetFOVRad(), z, *m_aspectRatio, wNear, hNear);
}

void BaseEnvCameraComponent::SetFOV(float fov)
{
	if(m_fieldAngleComponent.expired())
		return;
	m_fieldAngleComponent->SetFieldAngle(fov);
}
void BaseEnvCameraComponent::SetAspectRatio(float aspectRatio)
{
	*m_aspectRatio = aspectRatio;
	m_stateFlags |= StateFlags::ProjectionMatrixDirtyBit;
}
void BaseEnvCameraComponent::SetNearZ(float nearZ)
{
	*m_nearZ = nearZ;
	m_stateFlags |= StateFlags::ProjectionMatrixDirtyBit;
}
void BaseEnvCameraComponent::SetFarZ(float farZ)
{
	*m_farZ = farZ;
	m_stateFlags |= StateFlags::ProjectionMatrixDirtyBit;
}
void BaseEnvCameraComponent::SetFocalDistance(float focalDistance) { *m_focalDistance = focalDistance; }
const Mat4 &BaseEnvCameraComponent::GetProjectionMatrix() const
{
	if(math::is_flag_set(m_stateFlags, StateFlags::ProjectionMatrixDirtyBit)) {
		if(!math::is_flag_set(m_stateFlags, StateFlags::CustomProjectionMatrix))
			const_cast<BaseEnvCameraComponent *>(this)->UpdateProjectionMatrix();
		math::set_flag(const_cast<BaseEnvCameraComponent *>(this)->m_stateFlags, StateFlags::ProjectionMatrixDirtyBit, false);
	}
	return *m_projectionMatrix;
}
void BaseEnvCameraComponent::FlagViewMatrixAsDirty() { math::set_flag(const_cast<BaseEnvCameraComponent *>(this)->m_stateFlags, StateFlags::ViewMatrixDirtyBit, true); }
void BaseEnvCameraComponent::FlagProjectionMatrixAsDirty() { math::set_flag(const_cast<BaseEnvCameraComponent *>(this)->m_stateFlags, StateFlags::ProjectionMatrixDirtyBit, true); }
const Mat4 &BaseEnvCameraComponent::GetViewMatrix() const
{
	if(math::is_flag_set(m_stateFlags, StateFlags::ViewMatrixDirtyBit)) {
		if(!math::is_flag_set(m_stateFlags, StateFlags::CustomViewMatrix))
			const_cast<BaseEnvCameraComponent *>(this)->UpdateViewMatrix();
		math::set_flag(const_cast<BaseEnvCameraComponent *>(this)->m_stateFlags, StateFlags::ViewMatrixDirtyBit, false);
	}
	return *m_viewMatrix;
}

float BaseEnvCameraComponent::GetFOV() const
{
	if(m_fieldAngleComponent.expired())
		return 0.f;
	return m_fieldAngleComponent->GetFieldAngle();
}
float BaseEnvCameraComponent::GetFOVRad() const { return math::deg_to_rad(GetFOV()); }

const util::PMatrix4Property &BaseEnvCameraComponent::GetProjectionMatrixProperty() const { return m_projectionMatrix; }
const util::PMatrix4Property &BaseEnvCameraComponent::GetViewMatrixProperty() const { return m_viewMatrix; }

const util::PFloatProperty &BaseEnvCameraComponent::GetAspectRatioProperty() const { return m_aspectRatio; }
const util::PFloatProperty &BaseEnvCameraComponent::GetNearZProperty() const { return m_nearZ; }
const util::PFloatProperty &BaseEnvCameraComponent::GetFarZProperty() const { return m_farZ; }
const util::PFloatProperty &BaseEnvCameraComponent::GetFOVProperty() const
{
	if(m_fieldAngleComponent.expired()) {
		static util::PFloatProperty nptr = nullptr;
		return nptr;
	}
	return m_fieldAngleComponent->GetFieldAngleProperty();
}
const util::PFloatProperty &BaseEnvCameraComponent::GetFocalDistanceProperty() const { return m_focalDistance; }

float BaseEnvCameraComponent::GetAspectRatio() const { return *m_aspectRatio; }
float BaseEnvCameraComponent::GetNearZ() const { return *m_nearZ; }
float BaseEnvCameraComponent::GetFarZ() const { return *m_farZ; }
float BaseEnvCameraComponent::GetFocalDistance() const { return *m_focalDistance; }
void BaseEnvCameraComponent::UpdateFrustumPlanes()
{
	m_frustumPlanes.clear();
	GetFrustumPlanes(m_frustumPlanes);
}
const std::vector<math::Plane> &BaseEnvCameraComponent::GetFrustumPlanes() const { return m_frustumPlanes; }
Vector3 BaseEnvCameraComponent::GetNearPlanePoint(const Vector2 &uv) const { return GetPlanePoint(*m_nearZ, uv); }
Vector3 BaseEnvCameraComponent::GetFarPlanePoint(const Vector2 &uv) const { return GetPlanePoint(*m_farZ, uv); }
Vector3 BaseEnvCameraComponent::GetPlanePoint(float z, const Vector2 &uv) const
{
	auto *trComponent = GetEntity().GetTransformComponent();
	auto pos = trComponent ? trComponent->GetPosition() : Vector3 {};
	auto forward = trComponent ? trComponent->GetForward() : uvec::PRM_FORWARD;
	auto right = trComponent ? trComponent->GetRight() : uvec::PRM_RIGHT;
	auto up = trComponent ? trComponent->GetUp() : uvec::PRM_UP;
	return math::frustum::get_plane_point(pos, forward, right, up, GetFOVRad(), z, *m_aspectRatio, uv);
}

void BaseEnvCameraComponent::CreateFrustumMesh(const Vector2 &uvStart, const Vector2 &uvEnd, std::vector<Vector3> &verts, std::vector<uint16_t> &indices) const
{
	auto uvTopLeft = Vector2(math::min(uvStart.x, uvEnd.x), math::min(uvStart.y, uvEnd.y));
	auto uvBottomRight = Vector2(math::max(uvStart.x, uvEnd.x), math::max(uvStart.y, uvEnd.y));
	auto uvTopRight = Vector2(uvBottomRight.x, uvTopLeft.y);
	auto uvBottomLeft = Vector2(uvTopLeft.x, uvBottomRight.y);
	verts = {GetNearPlanePoint(uvTopLeft), GetNearPlanePoint(uvTopRight), GetNearPlanePoint(uvBottomRight), GetNearPlanePoint(uvBottomLeft),

	  GetFarPlanePoint(uvTopLeft), GetFarPlanePoint(uvTopRight), GetFarPlanePoint(uvBottomRight), GetFarPlanePoint(uvBottomLeft)};
	indices = {2, 1, 0, 3, 2, 0,

	  0, 1, 4, 1, 5, 4,

	  1, 2, 5, 2, 6, 5,

	  7, 2, 3, 7, 6, 2,

	  4, 3, 0, 4, 7, 3,

	  4, 5, 6, 4, 6, 7};
}
void BaseEnvCameraComponent::CreateFrustumKDop(const Vector2 &uvStart, const Vector2 &uvEnd, std::vector<math::Plane> &kDop) const
{
	auto uvTopLeft = Vector2(math::min(uvStart.x, uvEnd.x), math::min(uvStart.y, uvEnd.y));
	auto uvBottomRight = Vector2(math::max(uvStart.x, uvEnd.x), math::max(uvStart.y, uvEnd.y));
	auto uvTopRight = Vector2(uvBottomRight.x, uvTopLeft.y);
	auto uvBottomLeft = Vector2(uvTopLeft.x, uvBottomRight.y);
	std::array<Vector3, 8> verts = {GetNearPlanePoint(uvTopLeft), GetNearPlanePoint(uvTopRight), GetNearPlanePoint(uvBottomRight), GetNearPlanePoint(uvBottomLeft),

	  GetFarPlanePoint(uvTopLeft), GetFarPlanePoint(uvTopRight), GetFarPlanePoint(uvBottomRight), GetFarPlanePoint(uvBottomLeft)};

	// Move all far plane points closer to near plane to avoid precision errors when
	// creating the planes.
	for(auto i = decltype(verts.size()) {0}; i < verts.size() / 2; ++i) {
		auto &vNear = verts.at(i);
		auto &vFar = verts.at(i + 4);
		vFar -= vNear;
		uvec::normalize(&vFar);
		vFar += vNear;
	}

	kDop = {math::Plane(verts.at(2), verts.at(0), verts.at(1)), math::Plane(verts.at(0), verts.at(4), verts.at(1)), math::Plane(verts.at(1), verts.at(5), verts.at(2)), math::Plane(verts.at(7), verts.at(3), verts.at(2)), math::Plane(verts.at(4), verts.at(0), verts.at(3)),
	  math::Plane(verts.at(4), verts.at(6), verts.at(5))};
	kDop.back().MoveToPos(GetFarPlaneCenter()); // Move back plane back to its actual position. TODO: Can this also cause precision errors?
}

void BaseEnvCameraComponent::GetFrustumPoints(std::vector<Vector3> &outPoints, float neard, float fard, float fov, float ratio, const Vector3 &center, const Vector3 &viewDir, const Vector3 &viewUp)
{
	/*Vector3 fc = center +viewDir *fard;
	Vector3 nc = center +viewDir *neard;
	auto &up = viewUp;
	auto right = uvec::cross(viewDir,viewUp);
	uvec::normalize(&right);*/
	Vector3 up = viewUp; //(0.f,1.f,0.f);
	Vector3 right = -glm::cross(viewDir, up);
	//auto right = uvec::get_perpendicular(viewDir);
	Vector3 fc = center + viewDir * fard;
	Vector3 nc = center + viewDir * neard;
	uvec::normalize(&right);
	up = glm::cross(right, viewDir);
	uvec::normalize(&up);

	float near_height = tanf(fov / 2.f) * neard;
	float near_width = near_height * ratio;
	float far_height = tanf(fov / 2.f) * fard;
	float far_width = far_height * ratio;

	outPoints.reserve(outPoints.size() + 8u);
	outPoints.push_back(fc - up * far_height - right * far_width); // Bottom Left
	outPoints.push_back(fc + up * far_height - right * far_width); // Top Left
	outPoints.push_back(fc + up * far_height + right * far_width); // Top Right
	outPoints.push_back(fc - up * far_height + right * far_width); // Bottom Right

	outPoints.push_back(nc - up * near_height - right * near_width); // Bottom Left
	outPoints.push_back(nc + up * near_height - right * near_width); // Top Left
	outPoints.push_back(nc + up * near_height + right * near_width); // Top Right
	outPoints.push_back(nc - up * near_height + right * near_width); // Bottom Right
}
void BaseEnvCameraComponent::GetFrustumNeighbors(math::FrustumPlane planeID, math::FrustumPlane *neighborIDs)
{
	static math::FrustumPlane neighbors[6][4] = {{// Left
	                                         math::FrustumPlane::Top, math::FrustumPlane::Bottom, math::FrustumPlane::Near, math::FrustumPlane::Far},
	  {// Right
	    math::FrustumPlane::Top, math::FrustumPlane::Bottom, math::FrustumPlane::Near, math::FrustumPlane::Far},
	  {// Top
	    math::FrustumPlane::Left, math::FrustumPlane::Right, math::FrustumPlane::Near, math::FrustumPlane::Far},
	  {// Bottom
	    math::FrustumPlane::Left, math::FrustumPlane::Right, math::FrustumPlane::Near, math::FrustumPlane::Far},
	  {// Near
	    math::FrustumPlane::Left, math::FrustumPlane::Right, math::FrustumPlane::Top, math::FrustumPlane::Bottom},
	  {// Far
	    math::FrustumPlane::Left, math::FrustumPlane::Right, math::FrustumPlane::Top, math::FrustumPlane::Bottom}};
	for(unsigned int i = 0; i < 4; i++)
		neighborIDs[i] = neighbors[static_cast<int>(planeID)][i];
}
void BaseEnvCameraComponent::GetFrustumPlaneCornerPoints(math::FrustumPlane planeA, math::FrustumPlane planeB, math::FrustumPoint *cornerPoints)
{
	static math::FrustumPoint points[6][6][2] = {{// Left
	                                         {
	                                           // Left
	                                           math::FrustumPoint::FarBottomLeft, math::FrustumPoint::FarBottomLeft // Invalid
	                                         },
	                                         {
	                                           // Right
	                                           math::FrustumPoint::FarBottomLeft, math::FrustumPoint::FarBottomLeft // Invalid
	                                         },
	                                         {// Top
	                                           math::FrustumPoint::NearTopLeft, math::FrustumPoint::FarTopLeft},
	                                         {// Bottom
	                                           math::FrustumPoint::FarBottomLeft, math::FrustumPoint::NearBottomLeft},
	                                         {// Near
	                                           math::FrustumPoint::NearBottomLeft, math::FrustumPoint::NearTopLeft},
	                                         {// Far
	                                           math::FrustumPoint::FarTopLeft, math::FrustumPoint::FarBottomLeft}},
	  {// Right
	    {
	      // Left
	      math::FrustumPoint::FarBottomRight, math::FrustumPoint::FarBottomRight // Invalid
	    },
	    {
	      // Right
	      math::FrustumPoint::FarBottomRight, math::FrustumPoint::FarBottomRight // Invalid
	    },
	    {// Top
	      math::FrustumPoint::FarTopRight, math::FrustumPoint::NearTopRight},
	    {// Bottom
	      math::FrustumPoint::NearBottomRight, math::FrustumPoint::FarBottomRight},
	    {// Near
	      math::FrustumPoint::NearTopRight, math::FrustumPoint::NearBottomRight},
	    {// Far
	      math::FrustumPoint::FarBottomRight, math::FrustumPoint::FarTopRight}},
	  {  // Top
	    {// Left
	      math::FrustumPoint::FarTopLeft, math::FrustumPoint::NearTopLeft},
	    {// Right
	      math::FrustumPoint::NearTopRight, math::FrustumPoint::FarTopRight},
	    {
	      // Top
	      math::FrustumPoint::NearTopLeft, math::FrustumPoint::FarTopLeft // Invalid
	    },
	    {
	      // Bottom
	      math::FrustumPoint::FarBottomLeft, math::FrustumPoint::NearBottomLeft // Invalid
	    },
	    {// Near
	      math::FrustumPoint::NearTopLeft, math::FrustumPoint::NearTopRight},
	    {// Far
	      math::FrustumPoint::FarTopRight, math::FrustumPoint::FarTopLeft}},
	  {  // Bottom
	    {// Left
	      math::FrustumPoint::NearBottomLeft, math::FrustumPoint::FarBottomLeft},
	    {// Right
	      math::FrustumPoint::FarBottomRight, math::FrustumPoint::NearBottomRight},
	    {
	      // Top
	      math::FrustumPoint::NearBottomLeft, math::FrustumPoint::FarBottomLeft // Invalid
	    },
	    {
	      // Bottom
	      math::FrustumPoint::FarBottomLeft, math::FrustumPoint::NearBottomLeft // Invalid
	    },
	    {// Near
	      math::FrustumPoint::NearBottomRight, math::FrustumPoint::NearBottomLeft},
	    {// Far
	      math::FrustumPoint::FarBottomLeft, math::FrustumPoint::FarBottomRight}},
	  {
	    // Near
	    {// Left
	      math::FrustumPoint::NearTopLeft, math::FrustumPoint::NearBottomLeft},
	    {// Right
	      math::FrustumPoint::NearBottomRight, math::FrustumPoint::NearTopRight},
	    {// Top
	      math::FrustumPoint::NearTopRight, math::FrustumPoint::NearTopLeft},
	    {// Bottom
	      math::FrustumPoint::NearBottomLeft, math::FrustumPoint::NearBottomRight},
	    {// Near
	      math::FrustumPoint::NearTopLeft, math::FrustumPoint::NearTopRight},
	    {// Far
	      math::FrustumPoint::FarTopRight, math::FrustumPoint::FarTopLeft},
	  },
	  {  // Far
	    {// Left
	      math::FrustumPoint::FarBottomLeft, math::FrustumPoint::FarTopLeft},
	    {// Right
	      math::FrustumPoint::FarTopRight, math::FrustumPoint::FarBottomRight},
	    {// Top
	      math::FrustumPoint::FarTopLeft, math::FrustumPoint::FarTopRight},
	    {// Bottom
	      math::FrustumPoint::FarBottomRight, math::FrustumPoint::FarBottomLeft},
	    {// Near
	      math::FrustumPoint::FarTopLeft, math::FrustumPoint::FarTopRight},
	    {// Far
	      math::FrustumPoint::FarTopRight, math::FrustumPoint::FarTopLeft}}};
	cornerPoints[0] = points[static_cast<int>(planeA)][static_cast<int>(planeB)][0];
	cornerPoints[1] = points[static_cast<int>(planeA)][static_cast<int>(planeB)][1];
}
void BaseEnvCameraComponent::CreateFrustumKDop(const std::vector<math::Plane> &planes, const std::vector<Vector3> &points, const Vector3 &dir, std::vector<math::Plane> *kDop)
{
	std::array<float, 6> fDir;
	for(unsigned int i = 0; i < 6; i++) {
		math::Plane &plane = const_cast<math::Plane &>(planes[i]);
		fDir[i] = uvec::dot(plane.GetNormal(), dir);
		if(fDir[i] < EPSILON)
			kDop->push_back(plane);
	}
	for(unsigned int i = 0; i < 6; i++) {
		//const Plane &plane = planes[i];
		if(fDir[i] <= EPSILON) {
			math::FrustumPlane neighbors[4];
			GetFrustumNeighbors(math::FrustumPlane(i), &neighbors[0]);
			for(unsigned j = 0; j < 4; j++) {
				float fNDir = fDir[static_cast<int>(neighbors[j])];
				if(fNDir > EPSILON) {
					math::FrustumPoint corners[2];
					GetFrustumPlaneCornerPoints(math::FrustumPlane(i), neighbors[j], &corners[0]);
					math::Plane p(points[static_cast<int>(corners[0])], points[static_cast<int>(corners[1])], points[static_cast<int>(corners[0])] - dir);
					kDop->push_back(p);
				}
			}
		}
	}
}

Mat4 BaseEnvCameraComponent::CalcProjectionMatrix(math::Radian fovRad, float aspectRatio, float nearZ, float farZ, const rendering::Tile *optTile)
{
	auto mat = glm::gtc::perspectiveRH(fovRad, aspectRatio, normalize_plane_z(nearZ), normalize_plane_z(farZ));

	if(optTile)
		mat = pragma::rendering::calc_tile_offset_matrix(*optTile) * mat;

	mat = glm::gtc::scale(mat, Vector3(1.f, -1.f, 1.f));
	return mat;
}
