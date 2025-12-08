// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :entities.components.liquid;
import :entities.components.liquid_surface;
import :entities.components.particle_system;
import :particle_system;
import :physics;

using namespace pragma;

CLiquidComponent::CLiquidComponent(pragma::ecs::BaseEntity &ent) : BaseFuncLiquidComponent(ent) { pragma::ecs::CParticleSystemComponent::Precache("water"); }
CLiquidComponent::~CLiquidComponent() {}
void CLiquidComponent::Initialize()
{
	BaseFuncLiquidComponent::Initialize();

	GetEntity().AddComponent<CLiquidSurfaceComponent>();
	/*BindEvent(CRenderComponent::EVENT_SHOULD_DRAW,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		// TODO: Run CRenderComponent::UpdateShouldDrawState when any of these change
		if(!(m_hWaterSurface.valid() == false && (m_waterScene == nullptr || m_waterScene->descSetGroupTexEffects != nullptr)))
		{
			static_cast<CEShouldDraw&>(evData.get()).shouldDraw = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(GetEntity().IsSpawned() == false)
			return;
		BaseFuncLiquidComponent::InitializeWaterSurface();
		SetupWater();
		ReloadSurfaceSimulator();
	});*/
	BindEventUnhandled(cModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		BaseFuncLiquidComponent::InitializeWaterSurface();
		SetupWater();
	});
}
void CLiquidComponent::OnEntitySpawn()
{
	BaseFuncLiquidComponent::OnEntitySpawn();
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	//auto pPhysComponent = ent.GetPhysicsComponent();
	//if(pPhysComponent != nullptr)
	//	pPhysComponent->InitializePhysics(pragma::physics::PhysicsType::Static);
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent) {
		// pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::Water);
		// pRenderComponent->GetRenderModeProperty()->SetLocked(true);
	}
}
void CLiquidComponent::ReceiveData(NetPacket &packet) {}

void CLiquidComponent::SetupWater()
{
	auto renderC = GetEntity().GetComponent<CLiquidSurfaceComponent>();
	if(renderC.expired())
		return;
	renderC->ClearWaterScene();
	/*auto &mdl = GetModel();
	if(mdl == nullptr)
		return;
	auto &mats = mdl->GetMaterials();
	std::unordered_map<std::size_t,bool> waterMaterialIds;
	waterMaterialIds.reserve(mats.size());
	for(auto it=mats.begin();it!=mats.end();++it)
	{
		auto &hMat = *it;
		if(hMat.IsValid() == false)
			continue;
		auto *mat = static_cast<msys::CMaterial*>(hMat.get());
		auto hShader = mat->GetShader();
		if(hShader.IsValid() == false || dynamic_cast<Shader::Water*>(hShader.get()) == nullptr)
			continue;
		waterMaterialIds.insert(decltype(waterMaterialIds)::value_type({it -mats.begin(),true}));
	}
	if(waterMaterialIds.empty() == true)
		return;
	std::vector<pragma::geometry::CModelSubMesh*> waterSurfaces;
	for(auto meshId : mdl->GetBaseMeshes())
	{
		auto meshGroup = mdl->GetMeshGroup(meshId);
		if(meshGroup == nullptr)
			continue;
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto *cSubMesh = static_cast<pragma::geometry::CModelSubMesh*>(subMesh.get());
				auto texId = cSubMesh->GetTexture();
				auto it = waterMaterialIds.find(texId);
				if(it != waterMaterialIds.end())
					waterSurfaces.push_back(cSubMesh);
			}
		}
	}
	if(waterSurfaces.empty() == true)
		return;
	auto *meshSurface = waterSurfaces.front(); // TODO: All surfaces?
	*/
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto surfC = GetEntity().GetComponent<CSurfaceComponent>();
	auto *mesh = surfC.valid() ? surfC->GetMesh() : nullptr;
	auto &mats = mdl->GetMaterials();
	if(!mesh)
		return;
	auto *meshSurface = static_cast<pragma::geometry::CModelSubMesh *>(mesh);
	auto matIdx = mdl->GetMaterialIndex(*meshSurface);
	auto *mat = matIdx.has_value() ? static_cast<msys::CMaterial *>(mats.at(*matIdx).get()) : nullptr;
	Vector3 min, max;
	mdl->GetCollisionBounds(min, max);
	renderC->InitializeWaterScene(meshSurface->GetVertexPosition(0), meshSurface->GetVertexNormal(0), min, max);
}

void CLiquidComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////////

void CFuncWater::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLiquidComponent>();
}

static void debug_water(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	dbg = nullptr;
	if(pragma::get_cgame() == nullptr || pl == nullptr)
		return;
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	auto ents = pragma::console::find_target_entity(state, *charComponent, argv, [](pragma::physics::TraceData &trData) { trData.SetCollisionFilterMask(trData.GetCollisionFilterGroup() | pragma::physics::CollisionMask::Water | pragma::physics::CollisionMask::WaterSurface); });
	auto bFoundWater = false;
	if(ents.empty() == false) {
		for(auto *ent : ents) {
			auto *entWater = dynamic_cast<CFuncWater *>(ent);
			if(entWater == nullptr)
				continue;
			static gui::WIHandle hDepthTex = {};
			static EntityHandle hWater = {};
			hWater = entWater->GetHandle();
			dbg = std::make_unique<DebugGameGUI>([entWater]() {
				auto &wgui = gui::WGUI::GetInstance();
				auto *r = wgui.Create<gui::types::WIBase>();
				const auto size = 256u;
				r->SetSize(size * 4, size);

				auto pWaterComponent = entWater->GetComponent<pragma::CLiquidSurfaceComponent>();
				if(pWaterComponent.valid() == false || pWaterComponent->IsWaterSceneValid() == false)
					return gui::WIHandle {};
				auto &waterScene = pWaterComponent->GetWaterScene();
				auto *renderer = waterScene.sceneReflection.valid() ? dynamic_cast<const pragma::CRasterizationRendererComponent *>(waterScene.sceneReflection->GetRenderer<pragma::CRendererComponent>()) : nullptr;
				if(renderer == nullptr)
					return gui::WIHandle {};
				// Debug GUI
				auto &hdrInfo = renderer->GetHDRInfo();
				auto *pReflection = wgui.Create<gui::types::WITexturedRect>(r);
				pReflection->SetSize(size, size);
				pReflection->SetTexture(hdrInfo.sceneRenderTarget->GetTexture());
				pReflection->SetName("dbg_water_reflection");

				auto *pRefractionDepth = wgui.Create<pragma::gui::types::WIDebugDepthTexture>(r);
				pRefractionDepth->SetSize(size, size);
				pRefractionDepth->SetX(size);
				pRefractionDepth->SetTexture(*hdrInfo.prepass.textureDepth,
				  {//*waterScene.texSceneDepth
				    prosper::PipelineStageFlags::LateFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit},
				  {prosper::PipelineStageFlags::EarlyFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit});
				pRefractionDepth->SetShouldResolveImage(true);
				pRefractionDepth->SetName("dbg_water_refraction_depth");
				hDepthTex = pRefractionDepth->GetHandle();

				auto *pSceneNoWater = wgui.Create<pragma::gui::types::WIDebugMSAATexture>(r);
				pSceneNoWater->SetSize(size, size);
				pSceneNoWater->SetX(size * 2u);
				pSceneNoWater->SetTexture(*waterScene.texScene);
				pSceneNoWater->SetShouldResolveImage(false);
				pSceneNoWater->SetName("dbg_water_scene");
				return r->GetHandle();
			});
			dbg->AddCallback("PostRenderScenes", FunctionCallback<void>::Create([]() {
				if(hWater.valid() == false)
					return;
				auto *entWater = static_cast<CFuncWater *>(hWater.get());
				auto *cam = pragma::get_cgame()->GetRenderCamera<pragma::CCameraComponent>();
				// Update debug depth GUI element
				if(hDepthTex.IsValid() && cam != nullptr) {
					auto *pDepthTex = static_cast<pragma::gui::types::WIDebugDepthTexture *>(hDepthTex.get());
					pDepthTex->Setup(cam->GetNearZ(), cam->GetFarZ());
				}
			}));

			// Debug surface points
			auto pWaterComponent = entWater->GetComponent<pragma::CLiquidSurfaceSimulationComponent>();
			auto *entSurface = pWaterComponent.valid() ? pWaterComponent->GetSurfaceEntity() : nullptr;
			if(entSurface != nullptr) {
				auto pWaterSurfComponent = entSurface->GetComponent<pragma::CWaterSurfaceComponent>();
				auto *meshSurface = pWaterSurfComponent.valid() ? pWaterSurfComponent->GetWaterSurfaceMesh() : nullptr;
				if(meshSurface != nullptr) {
					auto &vkMesh = meshSurface->GetSceneMesh();
					auto *sim = static_cast<const pragma::physics::CPhysWaterSurfaceSimulator *>(pWaterComponent->GetSurfaceSimulator());
					//auto &buf = sim->GetPositionBuffer();
					auto dbgPoints = pragma::debug::DebugRenderer::DrawPoints(vkMesh->GetVertexBuffer(), meshSurface->GetVertexCount(), colors::Yellow);
					if(dbgPoints != nullptr) {
						dbg->CallOnRemove([dbgPoints]() mutable { dbgPoints = nullptr; });
					}
				}
			}
			bFoundWater = true;
			break;
		}
	}
	if(bFoundWater == false)
		Con::cwar << "No water entity found!" << Con::endl;
}
namespace {
	auto UVN = console::client::register_command("debug_water", &debug_water, console::ConVarFlags::None,
	  "Displays the reflection, refraction and refraction depth map for the given water-entity on screen. Call without arguments to turn the display off. Usage: debug_light_water <waterEntityIndex>");
}
