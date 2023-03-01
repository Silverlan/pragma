/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_softbody_component.hpp"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/model/model.h"

using namespace pragma;

void BaseSoftBodyComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("physics");
	ent.AddComponent("model");
	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { m_softBodyData = nullptr; });
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
			if(matId.has_value() == false || *matId >= materials.size() || ustring::compare<std::string>(materials.at(*matId)->GetShaderIdentifier(), "nodraw")) {
				it = subMeshes.erase(it);
				continue;
			}
			auto itTest = std::find_if(colMeshes.begin(), colMeshes.end(), [&subMesh](const std::shared_ptr<CollisionMesh> &colMesh) { return (colMesh->GetSoftBodyMesh() == subMesh.get()); });
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
	if(eventId == pragma::BasePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED)
		ReleaseSoftBodyData();
	return util::EventReply::Unhandled;
}
