/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/c_water_surface.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/physics/c_phys_water_surface_simulator.hpp"
#include "pragma/entities/func/c_func_water.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/model/model.h>
#include <prosper_util.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

LINK_ENTITY_TO_CLASS(c_water_surface,CWaterSurface);

void CWaterSurfaceComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		auto scale = pTrComponent != nullptr ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f};
		auto &matData = static_cast<CEOnUpdateRenderMatrices&>(evData.get());
		matData.pose = {};
		matData.transformation = glm::scale(umat::identity(),scale);
		//matData.transformation = matData.translation *matData.rotation *glm::scale(umat::identity(),scale);
	});
	// TODO
	//BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
	//	if(static_cast<CEOnUpdateRenderData&>(evData.get()).bufferUpdateRequired)
	//		UpdateSurfaceMesh();
	//});

	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent)
	{
		pRenderComponent->SetRenderMode(RenderMode::Water);
		pRenderComponent->SetCastShadows(false);
	}
}

CWaterSurfaceComponent::~CWaterSurfaceComponent()
{
	DestroySurface();
}

void CWaterSurfaceComponent::SetWaterObject(CWaterComponent *ent)
{
	m_hFuncWater = (ent != nullptr) ? ent->GetHandle<CWaterComponent>() : util::WeakHandle<CWaterComponent>{};
	if(ent != nullptr && ent->IsWaterSceneValid())
		InitializeWaterScene(ent->GetWaterScene());
}

CMaterial *CWaterSurfaceComponent::GetWaterMaterial() const
{
	auto *pFuncWaterComponent = m_hFuncWater.get();
	return (pFuncWaterComponent != nullptr) ? pFuncWaterComponent->GetWaterMaterial() : nullptr;
}

CModelSubMesh *CWaterSurfaceComponent::GetWaterSurfaceMesh() const
{
	if(m_waterSurfaceMesh.expired() == true)
		return nullptr;
	return m_waterSurfaceMesh.lock().get();
}

void CWaterSurfaceComponent::UpdateSurfaceMesh()
{
	if(m_hFuncWater.expired() || m_waterSurfaceMesh.expired() == true)
		return;
	auto *entWater = static_cast<CBaseEntity*>(&m_hFuncWater->GetEntity());
	auto *svEntWater = entWater->GetServersideEntity();
	CPhysWaterSurfaceSimulator *sim = nullptr;
	if(svEntWater != nullptr)
	{
		auto *pWaterComponent = static_cast<pragma::BaseFuncWaterComponent*>(svEntWater->FindComponent("water").get());
		if(pWaterComponent != nullptr)
			sim = const_cast<CPhysWaterSurfaceSimulator*>(static_cast<const CPhysWaterSurfaceSimulator*>(pWaterComponent->GetSurfaceSimulator()));
	}
	auto *pWaterComponent = static_cast<pragma::BaseFuncWaterComponent*>(entWater->FindComponent("water").get());
	if(sim == nullptr && pWaterComponent != nullptr)
		sim = const_cast<CPhysWaterSurfaceSimulator*>(static_cast<const CPhysWaterSurfaceSimulator*>(pWaterComponent->GetSurfaceSimulator()));

	if(sim == nullptr)
		return;
	auto drawCmd = c_game->GetCurrentDrawCommandBuffer();
	if(drawCmd == nullptr)
		return;
	auto *cmesh = static_cast<CModelSubMesh*>(m_waterSurfaceMesh.lock().get());
	sim->Draw(drawCmd,*cmesh);
}

void CWaterSurfaceComponent::SetSurfaceSimulator(const std::shared_ptr<PhysWaterSurfaceSimulator> &simulator)
{
	m_surfaceSimulator = simulator;
	if(GetEntity().IsSpawned() == true)
		InitializeSurface();
}

void CWaterSurfaceComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	InitializeSurface();
}

void CWaterSurfaceComponent::InitializeSurface()
{
	if(m_surfaceSimulator == nullptr)
		return;
	auto &sim = *static_cast<CPhysWaterSurfaceSimulator*>(m_surfaceSimulator.get());
	auto &simTriangles = sim.GetTriangleIndices();
	if(simTriangles.empty() == true)
		return;
	auto mdl = c_game->CreateModel();
	auto meshGroup = mdl->GetMeshGroup(0);

	auto subMesh = std::make_shared<CModelSubMesh>();
	// Initialize surface
	auto &verts = subMesh->GetVertices();
	auto &triangles = subMesh->GetTriangles();
	triangles = simTriangles;

	sim.LockParticleHeights();
	auto numParticles = sim.GetParticleCount();
	verts.reserve(numParticles);
	for(auto i=decltype(numParticles){0};i<numParticles;++i)
	{
		auto pos = sim.CalcParticlePosition(i);
		verts.push_back(Vertex(pos,{pos.x /(10.f *sim.GetWidth()),pos.z /(10.f *sim.GetLength())},{0.f,1.f,0.f})); // TODO
	}
	sim.UnlockParticleHeights();

	auto *matWater = GetWaterMaterial();
	if(matWater != nullptr)
	{
		mdl->AddTexture(matWater->GetName(),matWater);
		auto *texGroup = mdl->GetTextureGroup(0);
		texGroup->textures.push_back(0);
	}
	subMesh->SetSkinTextureIndex(0);

	auto mesh = std::make_shared<CModelMesh>();
	mesh->AddSubMesh(subMesh);
	meshGroup->AddMesh(mesh);
	mdl->Update(ModelUpdateFlags::All); // TODO: Don't update vertex and index buffers
	auto mdlComponent = GetEntity().GetModelComponent();
	if(mdlComponent.valid())
		mdlComponent->SetModel(mdl);

	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit;
	bufCreateInfo.size = verts.size() *sizeof(verts.front());
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	auto vertBuffer = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo,verts.data());

	bufCreateInfo.size = triangles.size() *sizeof(triangles.front());
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::IndexBufferBit;
	auto indexBuffer = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo,triangles.data());

	auto &vkMesh = subMesh->GetSceneMesh();
	vkMesh->SetVertexBuffer(vertBuffer); //sim.GetPositionBuffer());
	vkMesh->SetIndexBuffer(indexBuffer);
	//

	m_waterSurfaceMesh = subMesh;

	m_cbRenderSurface = c_game->AddCallback("PreRenderScenes",FunctionCallback<void>::Create([this]() {
		if(m_surfaceSimulator == nullptr)
			return;
		auto *sim = static_cast<CPhysWaterSurfaceSimulator*>(m_surfaceSimulator.get());
		if(sim == nullptr)
			return;
		sim->Simulate(0.01);//m_entity->GetNetworkState()->GetGameState()->DeltaTime()); // TODO
	}));
}

void CWaterSurfaceComponent::DestroySurface()
{
	m_surfaceSimulator = nullptr;
	if(m_cbRenderSurface.IsValid())
		m_cbRenderSurface.Remove();
}
luabind::object CWaterSurfaceComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CWaterSurfaceComponentHandleWrapper>(l);}
const Vector3 &CWaterSurfaceComponent::GetPosition() const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	return pTrComponent != nullptr ? pTrComponent->GetPosition() : uvec::ORIGIN;
}
const Quat &CWaterSurfaceComponent::GetOrientation() const
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	static auto identity = uquat::identity();
	return pTrComponent != nullptr ? pTrComponent->GetRotation() : identity;
}

////////////

void CWaterSurface::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWaterSurfaceComponent>();
}
