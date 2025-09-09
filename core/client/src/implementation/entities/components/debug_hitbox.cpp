// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/model/c_model.h"
#include "pragma/c_engine.h"
#include "pragma/debug/debug_render_info.hpp"
#include "pragma/entities/components/base_model_component.hpp"

module pragma.client.entities.components.debug_hitbox;

import pragma.client.debug;

extern CEngine *c_engine;

using namespace pragma;

CDebugHitboxComponent::~CDebugHitboxComponent() {}
void CDebugHitboxComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CDebugHitboxComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	SetTickPolicy(pragma::TickPolicy::Always);
}
void CDebugHitboxComponent::OnTick(double tDelta)
{
	BaseEntityComponent::OnTick(tDelta);
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	if(m_dirty) {
		m_dirty = false;
		InitializeDebugObjects();
	}
	auto boneIds = mdl->GetHitboxBones();
	uint32_t objId = 0;
	auto numObjs = m_debugObjects.size();
	for(auto boneId : boneIds) {
		if(objId >= numObjs)
			break;
		auto &dbgObj = m_debugObjects[objId];
		Vector3 origin, min, max;
		Quat rot;
		if(mdlComponent->GetHitboxBounds(boneId, min, max, origin, rot) == true) {
			dbgObj->SetPos(origin);
			dbgObj->SetRotation(rot);
		}
		++objId;
	}
}
void CDebugHitboxComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	ClearDebugObjects();
}
void CDebugHitboxComponent::SetHitboxColor(animation::BoneId boneId, const std::optional<Color> &color)
{
	m_dirty = true;
	if(color) {
		m_hitboxColors[boneId] = *color;
		return;
	}

	auto it = m_hitboxColors.find(boneId);
	if(it != m_hitboxColors.end())
		m_hitboxColors.erase(it);
}
void CDebugHitboxComponent::ClearDebugObjects()
{
	for(auto &obj : m_debugObjects) {
		if(!obj->IsValid())
			continue;
		obj->Remove();
	}
	m_debugObjects.clear();
}
void CDebugHitboxComponent::InitializeDebugObjects()
{
	ClearDebugObjects();
	auto mdlComponent = GetEntity().GetModelComponent();
	auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	auto boneIds = mdl->GetHitboxBones();
	if(!boneIds.empty()) {
		for(auto boneId : boneIds) {
			auto &hb = *mdl->GetHitbox(boneId);
			Vector3 origin, min, max;
			Quat rot;
			if(mdlComponent->GetHitboxBounds(boneId, min, max, origin, rot) == true) {
				Color col {255, 255, 255, 255};
				auto it = m_hitboxColors.find(boneId);
				if(it != m_hitboxColors.end())
					col = it->second;
				else {
					switch(hb.group) {
					case HitGroup::Head:
						col = Color::Red;
						break;
					case HitGroup::Chest:
						col = Color::Lime;
						break;
					case HitGroup::Stomach:
						col = Color::Blue;
						break;
					case HitGroup::LeftArm:
						col = Color::Yellow;
						break;
					case HitGroup::RightArm:
						col = Color::Cyan;
						break;
					case HitGroup::LeftLeg:
						col = Color::Magenta;
						break;
					case HitGroup::RightLeg:
						col = Color::OrangeRed;
						break;
					case HitGroup::Gear:
						col = Color::SpringGreen;
						break;
					case HitGroup::Tail:
						col = Color::Violet;
						break;
					}
				}

				DebugRenderInfo renderInfo {col};
				renderInfo.SetOrigin(origin);
				renderInfo.SetRotation(rot);
				m_debugObjects.push_back(::DebugRenderer::DrawBox(min, max, renderInfo));
			}
		}
	}
}
void CDebugHitboxComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	InitializeDebugObjects();
}
