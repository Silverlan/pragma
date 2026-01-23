// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_physics;

using namespace pragma;

util::TSharedHandle<physics::PhysObj> BasePhysicsComponent::InitializeSoftBodyPhysics()
{
	auto &ent = GetEntity();
	auto *pSoftBodyComponent = static_cast<BaseSoftBodyComponent *>(ent.AddComponent("softbody").get());
	if(pSoftBodyComponent == nullptr)
		return {};
	util::ScopeGuard sgSoftBodyComponent([&ent]() { ent.RemoveComponent("softbody"); });
	if(pSoftBodyComponent->InitializeSoftBodyData() == false)
		return {};
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr)
		return {};
	auto &colMeshes = hMdl->GetCollisionMeshes();
	std::vector<uint32_t> softBodyMeshes;
	softBodyMeshes.reserve(colMeshes.size());
	auto colMeshIdx = 0u;
	for(auto &colMesh : colMeshes) {
		auto idx = colMeshIdx++;
		if(colMesh->IsSoftBody() == false || colMesh->GetSoftBodyInfo() == nullptr || colMesh->GetSoftBodyMesh() == nullptr || colMesh->GetSoftBodyTriangles() == nullptr || colMesh->GetSoftBodyAnchors() == nullptr)
			continue;
		softBodyMeshes.push_back(idx);
	}
	if(softBodyMeshes.empty()) {
		pSoftBodyComponent->ReleaseSoftBodyData();
		return {};
	}
	auto mass = GetPhysicsMass();
	auto *state = ent.GetNetworkState();
	auto *game = state->GetGameState();
	auto *physEnv = game->GetPhysicsEnvironment();
	util::TSharedHandle<physics::SoftBodyPhysObj> phys = nullptr;
	for(auto idx : softBodyMeshes) {
		auto &colMesh = colMeshes.at(idx);
		auto &sbInfo = *colMesh->GetSoftBodyInfo();
		auto &mesh = *colMesh->GetSoftBodyMesh();

		auto &meshVerts = mesh.GetVertices();
		auto numMeshVerts = meshVerts.size();
		auto &sbTriangles = *colMesh->GetSoftBodyTriangles();
		if(sbTriangles.empty())
			continue;

		std::vector<Vector3> verts {};
		std::vector<uint16_t> indices {};
		std::vector<uint16_t> newVertexIndices(meshVerts.size(), std::numeric_limits<uint16_t>::max());
		std::vector<uint32_t> newVertexList;
		newVertexList.reserve(newVertexIndices.size());
		auto curVertIdx = 0u;
		indices.reserve(sbTriangles.size() * 3);
		verts.reserve(meshVerts.size());
		mesh.VisitIndices([&](auto *indexData, uint32_t numIndices) {
			for(auto triIdx : sbTriangles) {
				std::array<uint32_t, 3> triIndices = {indexData[triIdx * 3], indexData[triIdx * 3 + 1], indexData[triIdx * 3 + 2]};
				for(auto idx : triIndices) {
					auto &newIdx = newVertexIndices.at(idx);
					if(newIdx == std::numeric_limits<uint16_t>::max()) {
						newVertexList.push_back(idx);

						verts.push_back(meshVerts.at(idx).position);
						indices.push_back(curVertIdx);
						newIdx = curVertIdx++;
						continue;
					}
					indices.push_back(newIdx);
				}
			}
		});
		std::vector<uint16_t> indexTranslations;
		auto softBody = physEnv->CreateSoftBody(sbInfo, mass, verts, indices, indexTranslations);
		if(softBody == nullptr)
			continue;
		std::fill(newVertexIndices.begin(), newVertexIndices.end(), std::numeric_limits<uint16_t>::max());
		for(auto i = decltype(newVertexList.size()) {0}; i < newVertexList.size(); ++i)
			newVertexIndices.at(newVertexList.at(i)) = indexTranslations.at(i);

		softBody->SetSubMesh(mesh, newVertexIndices);
		Vector3 origin {0.f, 0.f, 0.f};
		softBody->SetOrigin(origin);
		auto pTrComponent = ent.GetTransformComponent();
		auto originOffset = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
		math::Transform startTransform;
		startTransform.SetIdentity();
		startTransform.SetOrigin(-origin + originOffset);
		startTransform.SetRotation(pTrComponent != nullptr ? pTrComponent->GetRotation() : uquat::identity());
		auto contactProcessingThreshold = 1e30;

		auto group = GetCollisionFilter();
		if(group != physics::CollisionMask::Default)
			softBody->SetCollisionFilterGroup(group);
		auto mask = GetCollisionFilterMask();
		if(mask != physics::CollisionMask::Default)
			softBody->SetCollisionFilterMask(mask);
		softBody->SetWorldTransform(startTransform);
		softBody->SetContactProcessingThreshold(CFloat(contactProcessingThreshold));

		physics::IRigidBody *pRigid = nullptr; // TODO
		for(auto &anchor : *colMesh->GetSoftBodyAnchors()) {
			uint16_t nodeIdx = 0u;
			if(softBody->MeshVertexIndexToNodeIndex(anchor.vertexIndex, nodeIdx) == false) {
				Con::CWAR << "Invalid vertex index " << anchor.vertexIndex << " for soft-body anchor! Skipping..." << Con::endl;
				continue;
			}
			if(pRigid == nullptr) {
				auto shape = physEnv->CreateBoxShape({1.f, 1.f, 1.f}, physEnv->GetGenericMaterial());
				auto rigid = physEnv->CreateRigidBody(*shape);
				rigid->SetPos({});
				rigid->SetSimulationEnabled(false);
				rigid->Spawn();
				pRigid = rigid.Get();
			}
			softBody->AppendAnchor(nodeIdx, *pRigid, (anchor.flags & physics::CollisionMesh::SoftBodyAnchor::Flags::DisableCollisions) != physics::CollisionMesh::SoftBodyAnchor::Flags::None, anchor.influence);
		}
		if(phys == nullptr)
			phys = util::to_shared_handle<physics::SoftBodyPhysObj>(physics::PhysObj::Create<physics::SoftBodyPhysObj, physics::ISoftBody &>(*this, *softBody));
		else
			phys->AddCollisionObject(*softBody);
	}

	if(phys == nullptr)
		return {};
	m_physObject = pragma::util::shared_handle_cast<physics::SoftBodyPhysObj, physics::PhysObj>(phys);
	m_physObject->Spawn();

	m_physicsType = physics::PhysicsType::SoftBody;
	SetCollisionFilter(physics::CollisionMask::Dynamic | physics::CollisionMask::Generic, physics::CollisionMask::All);
	SetMoveType(physics::MoveType::Physics);

	InitializePhysObj();
	OnPhysicsInitialized();
	sgSoftBodyComponent.dismiss();
	return m_physObject;
}
