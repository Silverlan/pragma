// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_soft_body;

using namespace pragma;

void BaseSoftBodyComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("physics");
	ent.AddComponent("model");
	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { m_softBodyData = nullptr; });
}
bool BaseSoftBodyComponent::InitializeSoftBodyData()
{
	ReleaseSoftBodyData();
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr)
		return false;
	m_softBodyData = std::unique_ptr<SoftBodyData>(new SoftBodyData);
	auto &meshes = m_softBodyData->meshes;
	hMdl->GetBodyGroupMeshes(mdlComponent->GetBodyGroups(), 0u, meshes);
	if(meshes.empty() == true) {
		m_softBodyData = nullptr;
		return false;
	}
	auto &materials = hMdl->GetMaterials();
	auto &colMeshes = hMdl->GetCollisionMeshes();
	for(auto it = meshes.begin(); it != meshes.end();) {
		auto &mesh = *it;
		mesh = mesh->Copy();
		auto &subMeshes = mesh->GetSubMeshes();
		for(auto it = subMeshes.begin(); it != subMeshes.end();) {
			auto &subMesh = *it;
			auto matId = hMdl->GetMaterialIndex(*subMesh);
			if(matId.has_value() == false || *matId >= materials.size() || string::compare<std::string>(materials.at(*matId)->GetShaderIdentifier(), "nodraw")) {
				it = subMeshes.erase(it);
				continue;
			}
			auto itTest = std::find_if(colMeshes.begin(), colMeshes.end(), [&subMesh](const std::shared_ptr<physics::CollisionMesh> &colMesh) { return (colMesh->GetSoftBodyMesh() == subMesh.get()); });
			if(itTest == colMeshes.end()) {
				it = subMeshes.erase(it);
				continue;
			}
			++it;
		}
		if(subMeshes.empty() == true)
			it = meshes.erase(it);
		else
			++it;
	}
	if(meshes.empty() == true) {
		m_softBodyData = nullptr;
		return false;
	}
	return true;
}
const BaseSoftBodyComponent::SoftBodyData *BaseSoftBodyComponent::GetSoftBodyData() const { return const_cast<BaseSoftBodyComponent *>(this)->GetSoftBodyData(); }
BaseSoftBodyComponent::SoftBodyData *BaseSoftBodyComponent::GetSoftBodyData() { return m_softBodyData.get(); }
void BaseSoftBodyComponent::ReleaseSoftBodyData() { m_softBodyData = nullptr; }
util::EventReply BaseSoftBodyComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == basePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED)
		ReleaseSoftBodyData();
	return util::EventReply::Unhandled;
}
