// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :entities.components.base_surface;

using namespace pragma;

void BaseSurfaceComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseSurfaceComponent::EVENT_ON_SURFACE_PLANE_CHANGED = registerEvent("ON_SURFACE_PLANE_CHANGED", ComponentEventInfo::Type::Broadcast);
	baseSurfaceComponent::EVENT_ON_SURFACE_MESH_CHANGED = registerEvent("ON_SURFACE_MESH_CHANGED", ComponentEventInfo::Type::Broadcast);
}

void BaseSurfaceComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseSurfaceComponent;
	{
		auto memberInfo = create_component_member_info<T, Vector4, [](const ComponentMemberInfo &, T &component, const Vector4 &value) { component.SetPlane(value); }, [](const ComponentMemberInfo &, T &component, Vector4 &value) { value = component.GetPlane().ToVector4(); }>("plane",
		  Vector4 {uvec::PRM_FORWARD, 0.0}, AttributeSpecializationType::Plane);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, Vector3,
		  [](const ComponentMemberInfo &, T &component, const Vector3 &value) {
			  auto plane = component.GetPlane();
			  plane.SetNormal(value);
			  component.SetPlane(plane);
		  },
		  [](const ComponentMemberInfo &, T &component, Vector3 &value) { value = component.GetPlane().GetNormal(); }>("normal", uvec::PRM_FORWARD, AttributeSpecializationType::Normal);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float,
		  [](const ComponentMemberInfo &, T &component, const float &value) {
			  auto plane = component.GetPlane();
			  plane.SetDistance(value);
			  component.SetPlane(plane);
		  },
		  [](const ComponentMemberInfo &, T &component, float &value) { value = component.GetPlane().GetDistance(); }>("distance", 0.0, AttributeSpecializationType::Distance);
		registerMember(std::move(memberInfo));
	}
}
BaseSurfaceComponent::BaseSurfaceComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

void BaseSurfaceComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	auto &ent = GetEntity();
	m_netEvSetPlane = SetupNetEvent("set_plane");
}
void BaseSurfaceComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["plane"] = m_plane.ToVector4();
}
void BaseSurfaceComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	Vector4 plane;
	udm["plane"](plane);
	m_plane = {plane};
}
void BaseSurfaceComponent::SetPlane(const math::Plane &plane)
{
	m_plane = plane;
	BroadcastEvent(baseSurfaceComponent::EVENT_ON_SURFACE_PLANE_CHANGED);
}
const math::Plane &BaseSurfaceComponent::GetPlane() const { return m_plane; }
math::Plane BaseSurfaceComponent::GetPlaneWs() const
{
	auto &plane = GetPlane();
	auto n = plane.GetNormal();
	auto d = plane.GetDistance();
	math::geometry::local_plane_to_world_space(n, d, GetEntity().GetPosition(), GetEntity().GetRotation());
	return math::Plane {n, d};
}

void BaseSurfaceComponent::SetPlane(const Vector3 &n, float d) { SetPlane({n, d}); }
void BaseSurfaceComponent::GetPlane(Vector3 &n, float &d) const
{
	n = m_plane.GetNormal();
	d = m_plane.GetDistance();
}
void BaseSurfaceComponent::GetPlaneWs(Vector3 &n, float &d) const
{
	auto plane = GetPlaneWs();
	n = plane.GetNormal();
	d = plane.GetDistance();
}

const Vector3 &BaseSurfaceComponent::GetPlaneNormal() const { return m_plane.GetNormal(); }
float BaseSurfaceComponent::GetPlaneDistance() const { return m_plane.GetDistance(); }

void BaseSurfaceComponent::SetPlaneNormal(const Vector3 &n)
{
	auto plane = GetPlane();
	plane.SetNormal(n);
	SetPlane(plane);
}
void BaseSurfaceComponent::SetPlaneDistance(float d)
{
	auto plane = GetPlane();
	plane.SetDistance(d);
	SetPlane(plane);
}
void BaseSurfaceComponent::Clear()
{
	m_mesh = {};
	SetPlane({{0.f, 1.f, 0.f}, 0.f});
}
geometry::ModelSubMesh *BaseSurfaceComponent::GetMesh() { return m_mesh.lock().get(); }
Vector3 BaseSurfaceComponent::ProjectToSurface(const Vector3 &pos) const
{
	Vector3 n;
	float d;
	GetPlaneWs(n, d);
	return uvec::project_to_plane(pos, n, d);
}
std::optional<BaseSurfaceComponent::MeshInfo> BaseSurfaceComponent::FindAndAssignMesh(const std::function<int32_t(geometry::ModelMesh &, geometry::ModelSubMesh &, material::Material &, const std::string &)> &filter)
{
	auto &ent = GetEntity();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr)
		return {};
	material::Material *mat = nullptr;
	auto dir = Vector3(0.f, 1.f, 0.f); // TODO
	auto &mats = hMdl->GetMaterials();
	std::vector<std::shared_ptr<geometry::ModelMesh>> meshes;
	hMdl->GetBodyGroupMeshes({}, 0, meshes);
	auto minDot = std::numeric_limits<float>::max();
	uint32_t highestPriority = 0;

	// Find best plane candidate
	math::Plane plane;
	std::optional<MeshInfo> meshInfo {};
	for(auto &mesh : meshes) {
		for(auto &subMesh : mesh->GetSubMeshes()) {
			auto matId = hMdl->GetMaterialIndex(*subMesh);
			if(matId.has_value() == false || *matId >= mats.size())
				continue;
			auto &hMat = mats.at(*matId);
			if(!hMat)
				continue;
			auto shaderName = hMat->GetShaderIdentifier();
			string::to_lower(shaderName);
			string::substr(shaderName, 0, 5);

			auto prio = filter ? filter(*mesh, *subMesh, *hMat.get(), shaderName) : 0;
			if(prio < 0)
				continue;
			if(prio > highestPriority)
				highestPriority = prio;
			else if(prio < highestPriority)
				continue;
			auto &verts = subMesh->GetVertices();
			if(verts.size() < 3)
				continue;
			auto &v0 = verts.at(0);
			auto &v1 = verts.at(1);
			auto &v2 = verts.at(2);

			math::Plane p {v0.position, v1.position, v2.position};
			auto dot = math::abs(uvec::dot(dir, p.GetNormal()) - 1.f);
			if(dot >= minDot)
				continue;
			minDot = dot;
			plane = p;
			mat = hMat.get();

			meshInfo = {mesh.get(), subMesh.get(), mat};
		}
	}
	if(minDot == std::numeric_limits<float>::max()) {
		Clear();
		spdlog::warn("No water plane found for func_water entity '{}'!", ent.ToString());
		return {};
	}
	else {
		assert(meshInfo.has_value());
		SetPlane(plane);
		m_mesh = meshInfo->subMesh->shared_from_this();
		BroadcastEvent(baseSurfaceComponent::EVENT_ON_SURFACE_MESH_CHANGED, CEOnSurfaceMeshChanged {*meshInfo});
		return meshInfo;
	}
	return {};
}

bool BaseSurfaceComponent::IsPointBelowSurface(const Vector3 &p) const { return math::geometry::get_side_of_point_to_plane(m_plane.GetNormal(), m_plane.GetDistance(), p) == math::geometry::PlaneSide::Back; }

Quat BaseSurfaceComponent::GetPlaneRotation() const
{
	auto plane = GetPlaneWs();
	auto up = uvec::PRM_UP;
	auto &n = plane.GetNormal();
	if(math::abs(uvec::dot(n, up)) > 0.99f)
		up = uvec::PRM_FORWARD;
	up = up - uvec::project(up, n);
	uvec::normalize(&up);
	return uquat::create_look_rotation(n, up);
}

bool BaseSurfaceComponent::CalcLineSurfaceIntersection(const Vector3 &lineOrigin, const Vector3 &lineDir, double *outT) const
{
	Vector3 n;
	float d;
	GetPlaneWs(n, d);
	auto t = 0.f;
	auto r = math::intersection::line_plane(lineOrigin, lineDir, n, d, &t);
	if(outT != nullptr)
		*outT = t;
	return r == math::intersection::Result::Intersect;
}

/////////////////////////////////

CEOnSurfaceMeshChanged::CEOnSurfaceMeshChanged(const BaseSurfaceComponent::MeshInfo &meshInfo) : meshInfo {meshInfo} {}
void CEOnSurfaceMeshChanged::PushArguments(lua::State *l)
{
	Lua::Push(l, meshInfo.mesh->shared_from_this());
	Lua::Push(l, meshInfo.subMesh->shared_from_this());
	Lua::Push(l, meshInfo.material);
}
