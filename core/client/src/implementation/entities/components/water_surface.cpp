// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.water_surface;
import :engine;
import :game;
import :model;
import :physics;

using namespace pragma;

void CWaterSurfaceComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(cRenderComponent::EVENT_ON_UPDATE_RENDER_MATRICES, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto pTrComponent = ent.GetTransformComponent();
		auto scale = pTrComponent != nullptr ? pTrComponent->GetScale() : Vector3 {1.f, 1.f, 1.f};
		auto &matData = static_cast<CEOnUpdateRenderMatrices &>(evData.get());
		matData.pose = {};
		matData.transformation = glm::gtc::scale(umat::identity(), scale);
		//matData.transformation = matData.translation *matData.rotation *glm::gtc::scale(umat::identity(),scale);
	});
	// TODO
	//BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
	//	if(static_cast<CEOnUpdateRenderData&>(evData.get()).bufferUpdateRequired)
	//		UpdateSurfaceMesh();
	//});

	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetCastShadows(false);
}

CWaterSurfaceComponent::~CWaterSurfaceComponent() { DestroySurface(); }

void CWaterSurfaceComponent::UpdateSurfaceMesh()
{
	if(m_hFuncWater.expired() || m_waterSurfaceMesh.expired() == true)
		return;
	auto *entWater = static_cast<ecs::CBaseEntity *>(&m_hFuncWater->GetEntity());
	auto *svEntWater = entWater->GetServersideEntity();
	physics::CPhysWaterSurfaceSimulator *sim = nullptr;
	if(svEntWater != nullptr) {
		auto *pWaterComponent = static_cast<BaseLiquidSurfaceSimulationComponent *>(svEntWater->FindComponent("liquid_surface_simulation").get());
		if(pWaterComponent != nullptr)
			sim = const_cast<physics::CPhysWaterSurfaceSimulator *>(static_cast<const physics::CPhysWaterSurfaceSimulator *>(pWaterComponent->GetSurfaceSimulator()));
	}
	auto *pWaterComponent = static_cast<BaseLiquidSurfaceSimulationComponent *>(entWater->FindComponent("liquid_surface_simulation").get());
	if(sim == nullptr && pWaterComponent != nullptr)
		sim = const_cast<physics::CPhysWaterSurfaceSimulator *>(static_cast<const physics::CPhysWaterSurfaceSimulator *>(pWaterComponent->GetSurfaceSimulator()));

	if(sim == nullptr)
		return;
	auto drawCmd = get_cgame()->GetCurrentDrawCommandBuffer();
	if(drawCmd == nullptr)
		return;
	auto *cmesh = static_cast<geometry::CModelSubMesh *>(m_waterSurfaceMesh.lock().get());
	sim->Draw(drawCmd, *cmesh);
}

void CWaterSurfaceComponent::SetWaterObject(BaseLiquidSurfaceSimulationComponent *ent)
{
	// TODO
	//m_hFuncWater = (ent != nullptr) ? ent->GetHandle<CWaterComponent>() : pragma::ComponentHandle<CWaterComponent>{};
	//if(ent != nullptr && ent->IsWaterSceneValid())
	//	InitializeWaterScene(ent->GetWaterScene());
}

material::CMaterial *CWaterSurfaceComponent::GetWaterMaterial() const
{
	return nullptr; // TODO
	                //auto *pFuncWaterComponent = m_hFuncWater.get();
	                //return (pFuncWaterComponent != nullptr) ? pFuncWaterComponent->GetWaterMaterial() : nullptr;
}

geometry::CModelSubMesh *CWaterSurfaceComponent::GetWaterSurfaceMesh() const
{
	if(m_waterSurfaceMesh.expired() == true)
		return nullptr;
	return m_waterSurfaceMesh.lock().get();
}

void CWaterSurfaceComponent::SetSurfaceSimulator(const std::shared_ptr<physics::PhysWaterSurfaceSimulator> &simulator)
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
	auto &sim = *static_cast<physics::CPhysWaterSurfaceSimulator *>(m_surfaceSimulator.get());
	auto &simTriangles = sim.GetTriangleIndices();
	if(simTriangles.empty() == true)
		return;
	auto mdl = get_cgame()->CreateModel();
	auto meshGroup = mdl->GetMeshGroup(0);

	auto subMesh = pragma::util::make_shared<geometry::CModelSubMesh>();
	// Initialize surface
	auto &verts = subMesh->GetVertices();
	subMesh->SetIndexType(geometry::IndexType::UInt16);
	subMesh->SetIndices(simTriangles);

	sim.LockParticleHeights();
	auto numParticles = sim.GetParticleCount();
	verts.reserve(numParticles);
	for(auto i = decltype(numParticles) {0}; i < numParticles; ++i) {
		auto pos = sim.CalcParticlePosition(i);
		verts.push_back(math::Vertex(pos, {pos.x / (10.f * sim.GetWidth()), pos.z / (10.f * sim.GetLength())}, {0.f, 1.f, 0.f})); // TODO
	}
	sim.UnlockParticleHeights();

	auto *matWater = GetWaterMaterial();
	if(matWater != nullptr) {
		mdl->AddTexture(matWater->GetName(), matWater);
		auto *texGroup = mdl->GetTextureGroup(0);
		texGroup->textures.push_back(0);
	}
	subMesh->SetSkinTextureIndex(0);

	auto mesh = pragma::util::make_shared<geometry::CModelMesh>();
	mesh->AddSubMesh(subMesh);
	meshGroup->AddMesh(mesh);
	mdl->Update(asset::ModelUpdateFlags::All); // TODO: Don't update vertex and index buffers
	auto mdlComponent = GetEntity().GetModelComponent();
	if(mdlComponent)
		mdlComponent->SetModel(mdl);

	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::VertexBufferBit;
	bufCreateInfo.size = verts.size() * sizeof(verts.front());
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	auto vertBuffer = get_cengine()->GetRenderContext().CreateBuffer(bufCreateInfo, verts.data());

	auto &indexData = subMesh->GetIndexData();
	bufCreateInfo.size = indexData.size();
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::IndexBufferBit;
	auto indexBuffer = get_cengine()->GetRenderContext().CreateBuffer(bufCreateInfo, indexData.data());

	auto &vkMesh = subMesh->GetSceneMesh();
	vkMesh->SetVertexBuffer(vertBuffer); //sim.GetPositionBuffer());
	vkMesh->SetIndexBuffer(indexBuffer, geometry::IndexType::UInt16);
	//

	m_waterSurfaceMesh = subMesh;

	m_cbRenderSurface = get_cgame()->AddCallback("PreRenderScenes", FunctionCallback<void>::Create([this]() {
		if(m_surfaceSimulator == nullptr)
			return;
		auto *sim = static_cast<physics::CPhysWaterSurfaceSimulator *>(m_surfaceSimulator.get());
		if(sim == nullptr)
			return;
		sim->Simulate(0.01); //m_entity->GetNetworkState()->GetGameState()->DeltaTime()); // TODO
	}));
}

void CWaterSurfaceComponent::DestroySurface()
{
	m_surfaceSimulator = nullptr;
	if(m_cbRenderSurface.IsValid())
		m_cbRenderSurface.Remove();
}
void CWaterSurfaceComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////

void CWaterSurface::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWaterSurfaceComponent>();
}
