// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :engine;
import :entities.components.animated;
import :entities.components.animated_bvh;
import :entities.components.bvh;
import :entities.components.model;

using namespace pragma;

bool CBvhComponent::ShouldConsiderMesh(const geometry::ModelSubMesh &mesh, const rendering::RenderBufferData &bufferData)
{
	return BaseBvhComponent::ShouldConsiderMesh(mesh) && !math::is_flag_set(bufferData.stateFlags, rendering::RenderBufferData::StateFlags::ExcludeFromAccelerationStructures);
}

void CBvhComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CBvhComponent::Initialize()
{
	BaseBvhComponent::Initialize();

	BindEventUnhandled(cModelComponent::EVENT_ON_RENDER_MESHES_UPDATED, [this](std::reference_wrapper<ComponentEvent> evData) {
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
	buildInfo.shouldConsiderMesh = [mdlC](const geometry::ModelSubMesh &mesh, uint32_t meshIdx) -> bool { return ShouldConsiderMesh(mesh, *mdlC->GetRenderBufferData(meshIdx)); };
	m_bvhData = RebuildBvh(renderMeshes, &buildInfo, nullptr, &GetEntity());
}
