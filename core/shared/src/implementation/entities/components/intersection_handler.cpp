// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.intersection_handler;

using namespace pragma;

IntersectionHandlerComponent::IntersectionHandlerComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void IntersectionHandlerComponent::Initialize() { BaseEntityComponent::Initialize(); }
void IntersectionHandlerComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
std::optional<HitInfo> IntersectionHandlerComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, math::CoordinateSpace space, float minDist, float maxDist) const
{
	HitInfo hitInfo {};
	if(IntersectionTest(origin, dir, space, minDist, maxDist, hitInfo))
		return hitInfo;
	return {};
}
bool IntersectionHandlerComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, math::CoordinateSpace space, float minDist, float maxDist, HitInfo &outHitInfo) const
{
	switch(space) {
	case math::CoordinateSpace::View:
	case math::CoordinateSpace::Screen:
		return false;
	case math::CoordinateSpace::World:
		{
			// Move ray to entity space
			auto entPoseInv = GetEntity().GetPose().GetInverse();
			auto originEs = entPoseInv * origin;
			auto dirEs = dir;
			uvec::rotate(&dirEs, entPoseInv.GetRotation());
			return IntersectionTest(originEs, dirEs, minDist, maxDist, outHitInfo);
		}
	}
	return IntersectionTest(origin, dir, minDist, maxDist, outHitInfo);
}
bool IntersectionHandlerComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo) const { return m_intersectionHandler.intersectionTest(m_intersectionHandler.userData, origin, dir, minDist, maxDist, outHitInfo); }
bool IntersectionHandlerComponent::IntersectionTestAabb(const Vector3 &min, const Vector3 &max) const { return m_intersectionHandler.intersectionTestAabb(m_intersectionHandler.userData, min, max, nullptr); }
bool IntersectionHandlerComponent::IntersectionTestAabb(const Vector3 &min, const Vector3 &max, IntersectionInfo &outIntersectionInfo) const { return m_intersectionHandler.intersectionTestAabb(m_intersectionHandler.userData, min, max, &outIntersectionInfo); }
bool IntersectionHandlerComponent::IntersectionTestKDop(const std::vector<math::Plane> &planes) const { return m_intersectionHandler.intersectionTestKDop(m_intersectionHandler.userData, planes, nullptr); }
bool IntersectionHandlerComponent::IntersectionTestKDop(const std::vector<math::Plane> &planes, IntersectionInfo &outIntersectionInfo) const { return m_intersectionHandler.intersectionTestKDop(m_intersectionHandler.userData, planes, &outIntersectionInfo); }
void IntersectionHandlerComponent::ClearIntersectionHandler() { m_intersectionHandler = {}; }
void IntersectionHandlerComponent::SetIntersectionHandler(const IntersectionHandler &intersectionHandler) { m_intersectionHandler = intersectionHandler; }
