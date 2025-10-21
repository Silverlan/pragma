// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


#include <string>

module pragma.shared;

import :entities.components.intersection_handler;

using namespace pragma;

IntersectionHandlerComponent::IntersectionHandlerComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void IntersectionHandlerComponent::Initialize() { BaseEntityComponent::Initialize(); }
void IntersectionHandlerComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
std::optional<HitInfo> IntersectionHandlerComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, umath::CoordinateSpace space, float minDist, float maxDist) const
{
	pragma::HitInfo hitInfo {};
	if(IntersectionTest(origin, dir, space, minDist, maxDist, hitInfo))
		return hitInfo;
	return {};
}
bool IntersectionHandlerComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, umath::CoordinateSpace space, float minDist, float maxDist, HitInfo &outHitInfo) const
{
	switch(space) {
	case umath::CoordinateSpace::View:
	case umath::CoordinateSpace::Screen:
		return false;
	case umath::CoordinateSpace::World:
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
bool IntersectionHandlerComponent::IntersectionTestKDop(const std::vector<umath::Plane> &planes) const { return m_intersectionHandler.intersectionTestKDop(m_intersectionHandler.userData, planes, nullptr); }
bool IntersectionHandlerComponent::IntersectionTestKDop(const std::vector<umath::Plane> &planes, IntersectionInfo &outIntersectionInfo) const { return m_intersectionHandler.intersectionTestKDop(m_intersectionHandler.userData, planes, &outIntersectionInfo); }
void IntersectionHandlerComponent::ClearIntersectionHandler() { m_intersectionHandler = {}; }
void IntersectionHandlerComponent::SetIntersectionHandler(const IntersectionHandler &intersectionHandler) { m_intersectionHandler = intersectionHandler; }
