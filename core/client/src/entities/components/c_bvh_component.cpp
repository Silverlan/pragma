/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/entities/components/c_bvh_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_animated_bvh_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/model/c_modelmesh.h"

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

bool CBvhComponent::ShouldConsiderMesh(const ModelSubMesh &mesh, const rendering::RenderBufferData &bufferData)
{
	return BaseBvhComponent::ShouldConsiderMesh(mesh) && !umath::is_flag_set(bufferData.stateFlags, pragma::rendering::RenderBufferData::StateFlags::ExcludeFromAccelerationStructures);
}

void CBvhComponent::InitializeLuaObject(lua_State *l) { return BaseBvhComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CBvhComponent::Initialize()
{
	BaseBvhComponent::Initialize();

	BindEventUnhandled(CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(static_cast<CEOnRenderMeshesUpdated &>(evData.get()).requireBoundingVolumeUpdate)
			RebuildBvh();
	});
	if(GetEntity().IsSpawned())
		RebuildBvh();
}

void CBvhComponent::OnEntitySpawn()
{
	BaseBvhComponent::OnEntitySpawn();
	UpdateBvhStatus();
}

bool CBvhComponent::IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo) const
{
	// TODO: If dirty?
	return BaseBvhComponent::IntersectionTest(origin, dir, minDist, maxDist, outHitInfo);
}

void CBvhComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseBvhComponent::OnEntityComponentAdded(component);
	if(GetEntity().IsSpawned() && typeid(component) == typeid(CAnimatedComponent))
		UpdateBvhStatus();
}
void CBvhComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseBvhComponent::OnEntityComponentAdded(component);
	if(GetEntity().IsSpawned() && typeid(component) == typeid(CAnimatedComponent))
		UpdateBvhStatus();
}
void CBvhComponent::OnRemove()
{
	BaseBvhComponent::OnRemove();
	GetEntity().RemoveComponent<CAnimatedBvhComponent>();
}

void CBvhComponent::UpdateBvhStatus()
{
	auto useAnimatedBvh = GetEntity().HasComponent<CAnimatedComponent>();
	if(useAnimatedBvh) {
		auto animBvh = GetEntity().AddComponent<CAnimatedBvhComponent>();
		if(animBvh.valid())
			animBvh->SetUpdateLazily(true);
	}
	else
		GetEntity().RemoveComponent<CAnimatedBvhComponent>();
}

void CBvhComponent::DoRebuildBvh()
{
	ClearBvh();
	auto *mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(!mdlC)
		return;
	auto &renderMeshes = mdlC->GetRenderMeshes();
	BvhBuildInfo buildInfo {};
	buildInfo.shouldConsiderMesh = [mdlC](const ModelSubMesh &mesh, uint32_t meshIdx) -> bool { return ShouldConsiderMesh(mesh, *mdlC->GetRenderBufferData(meshIdx)); };
	m_bvhData = BaseBvhComponent::RebuildBvh(renderMeshes, &buildInfo, nullptr, &GetEntity());
}
