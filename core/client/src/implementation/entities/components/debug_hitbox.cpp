// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.debug_hitbox;
import :debug;
import :engine;

using namespace pragma;

CDebugHitboxComponent::~CDebugHitboxComponent() {}
void CDebugHitboxComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CDebugHitboxComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	SetTickPolicy(TickPolicy::Always);
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
					case physics::HitGroup::Head:
						col = colors::Red;
						break;
					case physics::HitGroup::Chest:
						col = colors::Lime;
						break;
					case physics::HitGroup::Stomach:
						col = colors::Blue;
						break;
					case physics::HitGroup::LeftArm:
						col = colors::Yellow;
						break;
					case physics::HitGroup::RightArm:
						col = colors::Cyan;
						break;
					case physics::HitGroup::LeftLeg:
						col = colors::Magenta;
						break;
					case physics::HitGroup::RightLeg:
						col = colors::OrangeRed;
						break;
					case physics::HitGroup::Gear:
						col = colors::SpringGreen;
						break;
					case physics::HitGroup::Tail:
						col = colors::Violet;
						break;
					}
				}

				debug::DebugRenderInfo renderInfo {col};
				renderInfo.SetOrigin(origin);
				renderInfo.SetRotation(rot);
				m_debugObjects.push_back(debug::DebugRenderer::DrawBox(min, max, renderInfo));
			}
		}
	}
}
void CDebugHitboxComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	InitializeDebugObjects();
}
