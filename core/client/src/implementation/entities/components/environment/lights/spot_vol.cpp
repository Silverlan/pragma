// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.lights.spot_vol;
import :client_state;
import :engine;
import :game;
import :networking.util;
import :scripting.lua;

using namespace pragma;

void CLightSpotVolComponent::Initialize()
{
	auto &ent = GetEntity();
	ent.AddComponent<CModelComponent>();
	auto pRenderComponent = ent.AddComponent<CRenderComponent>();
	if(pRenderComponent.valid()) {
		// TODO
		/*FlagCallbackForRemoval(pRenderComponent->BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
			auto *scene = pragma::get_cgame()->GetScene();
			auto *renderer = scene ? scene->GetRenderer<pragma::CRendererComponent>() : nullptr;
			if(renderer != nullptr && renderer->IsRasterizationRenderer())
				static_cast<pragma::CRasterizationRendererComponent*>(renderer)->SetFrameDepthBufferSamplingRequired();
		}),CallbackType::Entity);*/
	}
	BindEventUnhandled(cRadiusComponent::EVENT_ON_RADIUS_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		if(!m_model)
			return;
		UpdateMeshData();
	});
	BindEventUnhandled(cFieldAngleComponent::EVENT_ON_FIELD_ANGLE_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		if(!m_model)
			return;
		UpdateMeshData();
	});
	BaseEnvLightSpotVolComponent::Initialize();
}

constexpr uint32_t segmentCount = 20;
float CLightSpotVolComponent::CalcEndRadius() const
{
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto pFieldAngleComponent = GetEntity().GetComponent<CFieldAngleComponent>();
	auto maxDist = pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 100.f;
	auto fieldAngle = pFieldAngleComponent.valid() ? pFieldAngleComponent->GetFieldAngle() : 0.f;
	return maxDist * math::tan(math::deg_to_rad(fieldAngle / 2.f));
}
uint32_t CLightSpotVolComponent::CalcSegmentCount() const
{
	auto endRadius = CalcEndRadius();
	uint32_t segCount = 0;
	for(auto i = decltype(segmentCount) {0}; i < segmentCount; ++i) {
		auto startSc = i / static_cast<float>(segmentCount);
		auto endSc = (i + 1) / static_cast<float>(segmentCount);

		auto segEndRadius = endRadius * endSc;
		if(segEndRadius < m_coneStartOffset)
			continue; // Skip this segment
		++segCount;
	}
	return segCount;
}

bool CLightSpotVolComponent::UpdateMeshData()
{
	auto pRadiusComponent = GetEntity().GetComponent<CRadiusComponent>();
	auto maxDist = pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 100.f;
	auto endRadius = CalcEndRadius();

	if(m_material)
		m_material->SetProperty("cone_height", maxDist);

	const uint32_t coneDetail = 64;
	const Vector3 dir {0.f, 0.f, 1.f};
	struct ConeSegment {
		std::vector<Vector3> verts;
		std::vector<Vector3> normals;
		std::vector<uint16_t> indices;
	};
	std::vector<ConeSegment> coneSegments;
	coneSegments.reserve(segmentCount);
	for(auto i = decltype(segmentCount) {0}; i < segmentCount; ++i) {
		auto startSc = i / static_cast<float>(segmentCount);
		auto endSc = (i + 1) / static_cast<float>(segmentCount);

		auto segEndRadius = endRadius * endSc;
		if(segEndRadius < m_coneStartOffset)
			continue; // Skip this segment
		auto segStartRadius = endRadius * startSc;
		if(segStartRadius < m_coneStartOffset) {
			// Clamp this segment
			segStartRadius = m_coneStartOffset;
		}

		auto startPos = dir * maxDist * static_cast<float>(segStartRadius / endRadius);
		auto endPos = dir * maxDist * static_cast<float>(segEndRadius / endRadius);
		ConeSegment segment;
		math::geometry::generate_truncated_cone_mesh(startPos, static_cast<float>(segStartRadius), dir, uvec::distance(startPos, endPos), static_cast<float>(segEndRadius), segment.verts, &segment.indices, &segment.normals, coneDetail, false);
		coneSegments.push_back(std::move(segment));
	}

	auto newMeshes = false;
	if(m_subMeshes.size() != coneSegments.size()) {
		m_subMeshes.clear();
		m_subMeshes.reserve(coneSegments.size());
		for(auto &segData : coneSegments) {
			auto subMesh = pragma::util::make_shared<geometry::CModelSubMesh>();
			subMesh->SetIndexCount(segData.indices.size());
			subMesh->GetVertices().resize(segData.verts.size());
			subMesh->SetSkinTextureIndex(0);
			m_subMeshes.push_back(subMesh);
		}
		newMeshes = true;
	}

	for(uint32_t idx = 0; auto &segData : coneSegments) {
		auto &subMesh = m_subMeshes[idx];
		auto &verts = subMesh->GetVertices();
		if(verts.size() != segData.verts.size() || subMesh->GetIndexCount() != segData.indices.size()) {
			// TODO: This can happen if the radius is 0, how to deal with this case?
			// TODO: Hide the model?
			return false;
		}
		for(auto i = decltype(segData.verts.size()) {0u}; i < segData.verts.size(); ++i)
			verts[i] = math::Vertex {segData.verts[i], segData.normals[i]};
		subMesh->VisitIndices([&segData](auto *indexData, uint32_t numIndices) {
			assert(pragma::util::size_of_container(segData.indices) == sizeof(indexData[0]) * numIndices);
			if(util::size_of_container(segData.indices) != sizeof(indexData[0]) * numIndices)
				throw std::runtime_error {"Volumetric mesh index data size mismatch!"};
			memcpy(indexData, segData.indices.data(), util::size_of_container(segData.indices));
		});
		subMesh->Update(asset::ModelUpdateFlags::UpdateVertexBuffer | asset::ModelUpdateFlags::UpdateIndexBuffer);
		++idx;
	}
	return newMeshes;
}

Bool CLightSpotVolComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetSpotlightTarget) {
		auto *ent = networking::read_entity(packet);
		if(ent != nullptr)
			SetSpotlightTarget(*ent);
		else
			m_hSpotlightTarget = EntityHandle {};
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

void CLightSpotVolComponent::ReceiveData(NetPacket &packet)
{
	m_coneStartOffset = packet->Read<float>();
	auto hEnt = GetHandle();
	networking::read_unique_entity(packet, [this, hEnt](ecs::BaseEntity *ent) {
		if(hEnt.expired())
			return;
		if(ent != nullptr)
			SetSpotlightTarget(*ent);
		else
			m_hSpotlightTarget = EntityHandle {};
	});
}

util::EventReply CLightSpotVolComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEnvLightSpotVolComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseToggleComponent::EVENT_ON_TURN_ON) {
		auto pRenderComponent = static_cast<ecs::CBaseEntity &>(GetEntity()).GetRenderComponent();
		if(pRenderComponent)
			pRenderComponent->SetSceneRenderPass(rendering::SceneRenderPass::World);
	}
	else if(eventId == baseToggleComponent::EVENT_ON_TURN_OFF) {
		auto pRenderComponent = static_cast<ecs::CBaseEntity &>(GetEntity()).GetRenderComponent();
		if(pRenderComponent)
			pRenderComponent->SetSceneRenderPass(rendering::SceneRenderPass::None);
	}
	return util::EventReply::Unhandled;
}

void CLightSpotVolComponent::InitializeVolumetricLight()
{
	auto mdlComponent = GetEntity().GetComponent<CModelComponent>();
	if(mdlComponent.expired())
		return;
	auto newMeshes = UpdateMeshData();
	if(!newMeshes)
		return;
	m_model = nullptr;
	auto mdl = get_cgame()->CreateModel();
	auto group = mdl->AddMeshGroup("reference");

	auto mat = get_client_state()->CreateMaterial("lightcone", "noop");
	auto *cmat = static_cast<material::CMaterial *>(mat.get());
	cmat->SetProperty("alpha_mode", AlphaMode::Blend);
	cmat->SetProperty("cone_height", CalcEndRadius());
	cmat->SetTexture("albedo_map", "error");
	cmat->UpdateTextures();
	cmat->SetLoaded(true);
	Lua::Material::Client::InitializeShaderData(nullptr, cmat, false);
	m_material = mat->GetHandle();

	auto mesh = pragma::util::make_shared<geometry::CModelMesh>();
	for(auto &subMesh : m_subMeshes)
		mesh->AddSubMesh(subMesh);
	group->AddMesh(mesh);
	mdl->AddMaterial(0, mat.get());

	mdl->Update(asset::ModelUpdateFlags::All);
	mdlComponent->SetModel(mdl);
	mdlComponent->SetDepthPrepassEnabled(false);
	m_model = mdl;
}

void CLightSpotVolComponent::OnEntitySpawn()
{
	BaseEnvLightSpotVolComponent::OnEntitySpawn();
	InitializeVolumetricLight();
}

void CLightSpotVolComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////

void CEnvLightSpotVol::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLightSpotVolComponent>();
}
