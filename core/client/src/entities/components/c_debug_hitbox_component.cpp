/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_debug_hitbox_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/debug/c_debugoverlay.h"

extern DLLCLIENT CEngine *c_engine;

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
	for(auto &obj : m_debugObjects) {
		if(!obj->IsValid())
			continue;
		obj->Remove();
	}
}
void CDebugHitboxComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();

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
				m_debugObjects.push_back(::DebugRenderer::DrawBox(origin, min, max, rot, col));
			}
		}
	}
}
