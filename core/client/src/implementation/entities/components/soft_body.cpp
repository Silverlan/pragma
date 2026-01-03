// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.soft_body;
import :engine;

using namespace pragma;

void CSoftBodyComponent::Initialize()
{
	BaseSoftBodyComponent::Initialize();
	// TODO
	//BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
	//	UpdateSoftBodyGeometry(); // TODO: Is this update ALWAYS required?
	//});
}
void CSoftBodyComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
bool CSoftBodyComponent::InitializeSoftBodyData()
{
	return BaseSoftBodyComponent::InitializeSoftBodyData();
	/*if(m_softBodyData == nullptr)
		return;
	//m_softBodyBuffers = std::make_unique<std::vector<Vulkan::SwapBuffer>>(); // prosper TODO

	auto &meshes = m_softBodyData->meshes;
	auto &context = pragma::get_cengine()->GetRenderContext();
	for(auto &mesh : meshes)
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			subMesh->Update(pragma::asset::ModelUpdateFlags::UpdateBuffers);
			auto &vkMesh = static_cast<pragma::geometry::CModelSubMesh&>(*subMesh).GetSceneMesh();
			if(vkMesh == nullptr)
				continue;
			auto &verts = subMesh->GetVertices();
			auto swapBuffer = Vulkan::SwapBuffer::Create(context,prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::VertexBufferBit,verts.size() *sizeof(verts.front()));
			auto numBuffers = swapBuffer->GetBufferCount();
			for(auto i=decltype(numBuffers){0};i<numBuffers;++i)
			{
				auto &buf = *swapBuffer->GetBuffer(i);
				buf->MapMemory(verts.data(),true);
			}
			m_softBodyBuffers->push_back(swapBuffer);
		}
	}*/ // prosper TODO
}
void CSoftBodyComponent::ReleaseSoftBodyData()
{
	/*if(m_softBodyData != nullptr)
	{
		auto &context = pragma::get_cengine()->GetRenderContext();
		for(auto &buf : m_softBodyData->meshes)
			const_cast<Vulkan::Context&>(context).ReleaseResourceSafely(buf);
		if(m_softBodyBuffers != nullptr)
		{
			for(auto &buf : *m_softBodyBuffers)
				const_cast<Vulkan::Context&>(context).ReleaseResourceSafely(buf);
		}
	}
	m_softBodyBuffers = nullptr;*/ // prosper TODO
	BaseSoftBodyComponent::ReleaseSoftBodyData();
}

void CSoftBodyComponent::UpdateSoftBodyGeometry()
{
	// prosper TODO
	/*if(GetPhysicsType() != pragma::physics::PhysicsType::SoftBody || m_softBodyData == nullptr || m_softBodyBuffers == nullptr)
		return;
	auto *phys = GetPhysicsObject();
	if(phys == nullptr || !phys->IsSoftBody())
		return;
	auto *physSoftBody = static_cast<SoftBodyPhysObj*>(phys);
	std::unordered_map<pragma::geometry::CModelSubMesh*,std::vector<PhysSoftBody*>> softBodies;
	for(auto &hObj : physSoftBody->GetCollisionObjects())
	{
		if(hObj.IsValid() == false || hObj->IsSoftBody() == false)
			continue;
		auto &softBody = *static_cast<PhysSoftBody*>(hObj.get());
		auto *mesh = static_cast<pragma::geometry::CModelSubMesh*>(softBody.GetSubMesh());
		if(mesh == nullptr)
			continue;
		auto it = softBodies.find(mesh);
		if(it == softBodies.end())
			it = softBodies.insert(std::make_pair(mesh,std::vector<PhysSoftBody*>{})).first;
		it->second.push_back(&softBody);
	}

	for(auto &pair : softBodies)
	{
		auto &mesh = *pair.first;
		auto &softBodies = pair.second;

		auto &buf = m_softBodyBuffers->at(0u); // TODO
		auto &vkMesh = mesh.GetSceneMesh();
		if(vkMesh == nullptr)
			continue;
		auto numVerts = mesh.GetVertexCount();
		buf->Swap();
		vkMesh->SetVertexBuffer(buf->GetBuffer());
		if(numVerts == 0)
			continue;
		auto map = buf->MapMemory();
		if(map == nullptr)
			continue;
		auto *data = map->GetData();
		for(auto *softBody : softBodies)
		{
			auto *btSoftBody = softBody->GetSoftBody();
			auto &nodes = btSoftBody->m_nodes;
			auto nodeCount = nodes.size();

			auto &colMesh = GetModel()->GetCollisionMeshes().front(); // TODO
			//auto &sbTriangles = *colMesh->GetSoftBodyTriangles(); // TODO
			auto &nodeIndicesToLocalIndices = softBody->GetNodeIndicesToLocalVertexIndices();
			auto &localIndicesToMeshIndices = softBody->GetLocalVertexIndicesToMeshVertexIndices();
			for(auto physIdx=decltype(nodes.size()){0};physIdx<nodes.size();++physIdx)
			{
				assert(physIdx < nodeIndicesToLocalIndices.size());
				if(physIdx >= nodeIndicesToLocalIndices.size())
					continue;
				auto localIdx = nodeIndicesToLocalIndices.at(physIdx);
				assert(localIdx < localIndicesToMeshIndices.size());
				if(localIdx >= localIndicesToMeshIndices.size())
					continue;
				auto vertIdx = localIndicesToMeshIndices.at(localIdx);
				auto &node = nodes.at(physIdx);
				auto &pNode = node.m_x;
				auto &nNode = node.m_n;
				// Update Position
				auto vertexOffset = static_cast<uint8_t*>(data) +vertIdx *sizeof(Vertex);
				Vector3 p {pNode.x() /PhysEnv::WORLD_SCALE,pNode.y() /PhysEnv::WORLD_SCALE,pNode.z() /PhysEnv::WORLD_SCALE};
				memcpy(vertexOffset,&p,sizeof(Vector3));

				// Update Normal
				Vector3 n {nNode.x(),nNode.y(),nNode.z()};
				memcpy(vertexOffset +sizeof(Vector3) +sizeof(Vector2),&n,sizeof(Vector3));
			}
		}
		map->Flush();
		map = nullptr;
	}*/
}
