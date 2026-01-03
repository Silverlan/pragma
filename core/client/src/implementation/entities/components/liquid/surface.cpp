// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.liquid_surface;
import :client_state;
import :engine;
import :entities.components.rasterization_renderer;
import :entities.components.render;
import :entities.components.surface;
import :game;
import :rendering.shaders;

using namespace pragma;

void CLiquidSurfaceComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	m_surfaceComponent = GetEntity().AddComponent<CSurfaceComponent>()->GetHandle<CSurfaceComponent>();
	auto portalC = GetEntity().AddComponent("portal");
	if(portalC.valid()) {
		portalC->CallLuaMethod<void>("SetMirrored", true);
		portalC->CallLuaMethod<void>("SetPortalOriginEnabled", false);
	}
	auto evId = GetComponentManager().FindEventId("render_target", "on_render_scene_initialized");
	if(evId.has_value()) {
		BindEventUnhandled(*evId, [this](std::reference_wrapper<ComponentEvent> evData) {
			auto renderTargetC = GetEntity().FindComponent("render_target");
			if(!renderTargetC.valid())
				return;
			auto *drawSceneInfo = renderTargetC->CallLuaMethod<rendering::DrawSceneInfo *>("GetDrawSceneInfo");
			if(drawSceneInfo) {
				auto maskWater = get_cgame()->GetRenderMask("water");
				if(maskWater.has_value())
					drawSceneInfo->exclusionMask |= *maskWater; // Don't render water surfaces
			}

			auto *renderer = renderTargetC->CallLuaMethod<CRendererComponent *>("GetRenderer");
			if(renderer) {
				auto *tex = renderer->GetHDRPresentationTexture();
				if(tex && m_waterScene && m_waterScene->descSetGroupTexEffects) {
					auto *descSetEffects = m_waterScene->descSetGroupTexEffects->GetDescriptorSet();
					descSetEffects->SetBindingTexture(*tex, math::to_integral(ShaderWater::WaterBinding::ReflectionMap));
					descSetEffects->Update();

					auto renderC = GetEntity().GetComponent<CModelComponent>();
					renderC->SetRenderMeshesDirty();
					renderC->UpdateRenderMeshes();
				}
			}
		});
	}
	auto renderC = GetEntity().GetComponent<CRenderComponent>();
	if(renderC.valid())
		renderC->AddToRenderGroup("water");

	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	if(mdlC.valid()) {
		// TODO: This shouldn't be needed, since the shader already causes the water surface
		// to be rendered with translucency, which should already disable the prepass.
		// For some reason this doesn't work at the moment, however. FIXME
		mdlC->SetDepthPrepassEnabled(false);
	}
}
void CLiquidSurfaceComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_hEntUnderwater.IsValid())
		m_hEntUnderwater.Remove();
}
void CLiquidSurfaceComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();

	auto *entRt = get_cgame()->CreateEntity("render_target");
	if(entRt) {
		m_hEntUnderwater = entRt->GetHandle();

		auto evId = GetComponentManager().FindEventId("render_target", "on_render_scene_initialized");
		auto rtC = entRt->FindComponent("render_target");
		if(rtC.valid() && evId.has_value()) {
			auto &cam = get_cgame()->GetRenderScene<CSceneComponent>()->GetActiveCamera();
			if(cam.valid())
				rtC->CallLuaMethod<void>("SetCamera", cam->GetLuaObject());

			rtC->AddEventCallback(*evId, [this, rtC](std::reference_wrapper<ComponentEvent> evData) mutable {
				InitializeRenderData();
				return util::EventReply::Unhandled;
			});

			auto evPreRenderScene = GetComponentManager().FindEventId("render_target", "pre_render_scene");
			if(evPreRenderScene.has_value()) {
				rtC->AddEventCallback(*evPreRenderScene, [this, rtC](std::reference_wrapper<ComponentEvent> evData) {
					auto &data = static_cast<LuaComponentEvent &>(evData.get());
					if(data.arguments.size() > 0) {
						auto &arg = data.arguments[0];
						try {
							auto *drawSceneInfo = luabind::object_cast_nothrow<rendering::DrawSceneInfo *>(arg, static_cast<rendering::DrawSceneInfo *>(nullptr));
							if(drawSceneInfo) {
								auto maskWater = get_cgame()->GetRenderMask("water");
								if(maskWater.has_value())
									drawSceneInfo->exclusionMask |= *maskWater; // Don't render water surfaces
								get_cgame()->GetPrimaryCameraRenderMask(drawSceneInfo->inclusionMask, drawSceneInfo->exclusionMask);
								if(m_surfaceComponent.valid()) {
									auto &plane = m_surfaceComponent->GetPlane();
									// We need to add an offset to the clipping plane to avoid
									// artifacts at the water's edges.
									static Vector4 offset {0.f, 0.f, 0.f, 0.4f};
									drawSceneInfo->clipPlane = plane.ToVector4() + offset;
								}
							}
						}
						catch(const luabind::error &err) {
						}
					}
					return util::EventReply::Unhandled;
				});
			}
			/*auto rasterC = pragma::get_cgame()->GetRenderScene()->GetRenderer<pragma::CRendererComponent>()->GetEntity().GetComponent<CRasterizationRendererComponent>();
			rasterC->AddEventCallback(CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_WATER,
				[](std::reference_wrapper<pragma::ComponentEvent> evData) -> pragma::util::EventReply {
					//auto *tex = pragma::get_cgame()->GetRenderScene()->GetRenderer<pragma::CRendererComponent>()->GetHDRPresentationTexture();
					//	m_waterScene->texScene
				return pragma::util::EventReply::Unhandled;
			});*/
		}
		entRt->Spawn();
	}

	//InvokeEventCallbacks(EVENT_MT_BEGIN_RECORD_WATER,evDataLightingStage);
}

void CLiquidSurfaceComponent::InitializeRenderData()
{
	if(m_renderDataInitialized || !m_hEntUnderwater.IsValid())
		return;
	auto rtC = m_hEntUnderwater->FindComponent("render_target");
	if(rtC.expired())
		return;
	auto *renderer = rtC->CallLuaMethod<CRendererComponent *>("GetRenderer");
	if(renderer) {
		auto *tex = renderer->GetHDRPresentationTexture();
		if(tex && m_waterScene && m_waterScene->descSetGroupTexEffects) {
			m_renderDataInitialized = true;

			auto *descSetEffects = m_waterScene->descSetGroupTexEffects->GetDescriptorSet();
			descSetEffects->SetBindingTexture(*tex, math::to_integral(ShaderWater::WaterBinding::RefractionMap));

			auto rasterC = renderer->GetEntity().GetComponent<CRasterizationRendererComponent>();
			if(rasterC.valid()) {
				auto &texDepth = rasterC->GetHDRInfo().prepass.textureDepth;
				descSetEffects->SetBindingTexture(*texDepth, math::to_integral(ShaderWater::WaterBinding::RefractionDepth));
				//descSetEffects.SetBindingUniformBuffer(*m_waterScene->settingsBuffer,pragma::math::to_integral(pragma::ShaderWater::WaterBinding::WaterSettings));
			}
			descSetEffects->Update();

			auto mdlC = GetEntity().GetComponent<CModelComponent>();
			if(mdlC.valid()) {
				mdlC->SetRenderMeshesDirty();
				mdlC->UpdateRenderMeshes();
			}
		}
	}
}

void CLiquidSurfaceComponent::ReceiveData(NetPacket &packet) {}

WaterScene::~WaterScene()
{
	for(auto &cb : hPostProcessing) {
		if(cb.IsValid())
			cb.Remove();
	}
	if(hComponentCreationCallback.IsValid())
		hComponentCreationCallback.Remove();
}

material::CMaterial *CLiquidSurfaceComponent::GetWaterMaterial() const
{
	if(m_surfaceComponent.expired())
		return nullptr;
	auto *mesh = m_surfaceComponent->GetMesh();
	if(!mesh)
		return nullptr;
	auto &ent = GetEntity();
	auto &mdl = ent.GetModel();
	if(mdl == nullptr)
		return nullptr;
	auto matIdx = mdl->GetMaterialIndex(*mesh);
	auto &mats = mdl->GetMaterials();
	return (matIdx.has_value() && matIdx < mats.size()) ? static_cast<material::CMaterial *>(mats.at(*matIdx).get()) : nullptr;
}

void CLiquidSurfaceComponent::ClearWaterScene() { m_waterScene = nullptr; }
const WaterScene &CLiquidSurfaceComponent::GetWaterScene() const { return *m_waterScene; }
bool CLiquidSurfaceComponent::IsWaterSceneValid() const { return m_waterScene != nullptr; }

void CLiquidSurfaceComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseLiquidSurfaceComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CSurfaceComponent))
		m_surfaceComponent = component.GetHandle<CSurfaceComponent>();
}

prosper::IDescriptorSet *CLiquidSurfaceComponent::GetEffectDescriptorSet() const
{
	if(m_waterScene == nullptr)
		return nullptr;
	return m_waterScene->descSetGroupTexEffects->GetDescriptorSet();
}

void CLiquidSurfaceComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CLiquidSurfaceComponent::InitializeWaterScene(const WaterScene &scene)
{
	m_waterScene = std::make_unique<WaterScene>();
	m_waterScene->sceneReflection = scene.sceneReflection;
	m_waterScene->descSetGroupTexEffects = scene.descSetGroupTexEffects;
	m_waterScene->hPostProcessing = scene.hPostProcessing;
	m_waterScene->hComponentCreationCallback = scene.hComponentCreationCallback;
	m_waterScene->settingsBuffer = scene.settingsBuffer;

	m_waterScene->fogBuffer = scene.fogBuffer;
	m_waterScene->fogDescSetGroup = scene.fogDescSetGroup;

	m_waterScene->waterScale = scene.waterScale;
	m_waterScene->waveStrength = scene.waveStrength;
	m_waterScene->waveSpeed = scene.waveSpeed;
	m_waterScene->reflectiveIntensity = scene.reflectiveIntensity;
}

static void is_camera_submerged(const CCameraComponent &cam, const Vector3 &n, float planeDist, const Vector3 &waterAabbMin, const Vector3 &waterAabbMax, bool &bCameraSubmerged, bool &bCameraFullySubmerged, math::geometry::PlaneSide &camCenterPlaneSide)
{
	bCameraSubmerged = false;
	bCameraFullySubmerged = true;
	std::array<Vector3, 4> nearPlaneBounds;
	cam.GetNearPlaneBoundaries(nearPlaneBounds);

	auto posCenter = cam.GetNearPlaneCenter();
	camCenterPlaneSide = math::geometry::get_side_of_point_to_plane(n, planeDist, posCenter);

	auto isPointInAabb = false;
	for(auto &p : nearPlaneBounds) {
		if(isPointInAabb == false && math::intersection::vector_in_bounds(p, waterAabbMin, waterAabbMax))
			isPointInAabb = true;
		auto side = math::geometry::get_side_of_point_to_plane(n, planeDist, p);
		if(side == math::geometry::PlaneSide::Back)
			bCameraSubmerged = true;
		else
			bCameraFullySubmerged = false;
	}
	if(bCameraSubmerged == false)
		return; // Camera is above water plane; we can skip the remaining checks altogether

	// Camera is intersecting or very close to water bounds; Do a more precise check to be certain
	// For now we assume the water is a simple axis-aligned box. Since the near plane corner points are
	// very close to each other, we'll also assume the water mesh is large enough so it cannot be enclosed within
	// the near plane bounds. This way we can just check if one of the near plane corner points is within the
	// water AABB.
	if(isPointInAabb == true)
		return;
	bCameraSubmerged = false;
	bCameraFullySubmerged = false;
	camCenterPlaneSide = math::geometry::PlaneSide::Front;
}

static auto cvReflectionQuality = console::get_client_con_var("cl_render_reflection_quality");
static auto cvDrawWater = console::get_client_con_var("render_draw_water");
void CLiquidSurfaceComponent::InitializeWaterScene(const Vector3 &refPos, const Vector3 &planeNormal, const Vector3 &waterAabbMin, const Vector3 &waterAabbMax)
{
	get_cengine()->GetShaderManager().GetShader("water"); // Make sure water shader is loaded
	m_waterAabbBounds = {waterAabbMin, waterAabbMax};
	auto *mat = GetWaterMaterial();
	if(mat == nullptr || ShaderWater::DESCRIPTOR_SET_WATER.IsValid() == false || ShaderPPFog::DESCRIPTOR_SET_FOG.IsValid() == false)
		return;
	auto whShader = mat->GetPrimaryShader();
	auto *scene = get_cgame()->GetScene<CSceneComponent>();
	if(scene == nullptr)
		return;
	auto renderer = dynamic_cast<CRendererComponent *>(scene->GetRenderer<CRendererComponent>());
	if(!whShader || renderer == nullptr)
		return;
	auto rasterC = renderer->GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rasterC.expired())
		return;
	auto *shader = dynamic_cast<ShaderWater *>(whShader);
	auto whShaderPPWater = get_cengine()->GetShader("pp_water");
	if(shader == nullptr || whShaderPPWater.expired())
		return;
	auto &shaderPPWater = static_cast<ShaderPPWater &>(*whShaderPPWater.get());
	m_shaderPpWater = shaderPPWater.GetHandle();

	auto *cam = get_cgame()->GetPrimaryCamera<CCameraComponent>();
	auto width = scene->GetWidth() / 2u;
	auto height = scene->GetHeight() / 2u;
	auto fov = cam ? cam->GetFOV() : baseEnvCameraComponent::DEFAULT_FOV;
	auto nearZ = cam ? cam->GetNearZ() : baseEnvCameraComponent::DEFAULT_NEAR_Z;
	auto farZ = cam ? cam->GetFarZ() : baseEnvCameraComponent::DEFAULT_FAR_Z; // TODO: shared Property?
	auto *camReflection = get_cgame()->CreateCamera<CCameraComponent>(width, height, fov, nearZ, farZ);
	if(camReflection == nullptr)
		return;
	m_waterScene = std::make_unique<WaterScene>();
	m_waterScene->descSetGroupTexEffects = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderWater::DESCRIPTOR_SET_WATER);
	auto *sceneC = CSceneComponent::Create(CSceneComponent::CreateInfo {});
	auto &sceneReflection = m_waterScene->sceneReflection;
	sceneReflection = sceneC ? sceneC->GetHandle<CSceneComponent>() : pragma::ComponentHandle<CSceneComponent> {};
	sceneReflection->ReloadRenderTarget(width, height);
	if(cam) {
		camReflection->GetFOVProperty()->Link(*cam->GetFOVProperty());
		camReflection->GetNearZProperty()->Link(*cam->GetNearZProperty());
		camReflection->GetFarZProperty()->Link(*cam->GetFarZProperty());
	}
	sceneReflection->SetWorldEnvironment(*scene->GetWorldEnvironment());

	// Initialize water settings
	m_waterScene->waterScale = util::FloatProperty::Create(4.f);
	m_waterScene->waveStrength = util::FloatProperty::Create(0.004f);
	m_waterScene->waveSpeed = util::FloatProperty::Create(1.f); // ?
	m_waterScene->reflectiveIntensity = util::FloatProperty::Create(1.2f);

	std::array<float, 4> waterSettings = {*m_waterScene->waterScale, *m_waterScene->waveStrength, *m_waterScene->waveSpeed, *m_waterScene->reflectiveIntensity};
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	bufCreateInfo.size = waterSettings.size() * sizeof(waterSettings.front());
	m_waterScene->settingsBuffer = get_cengine()->GetRenderContext().CreateBuffer(bufCreateInfo, waterSettings.data());
	//

	if(m_waterScene->texScene == nullptr) {
		auto &sceneImg = rasterC->GetHDRInfo().sceneRenderTarget->GetTexture().GetImage();
		auto extents = sceneImg.GetExtents();
		prosper::util::ImageCreateInfo imgCreateInfo {};
		imgCreateInfo.width = extents.width;
		imgCreateInfo.height = extents.height;
		imgCreateInfo.format = sceneImg.GetFormat();
		imgCreateInfo.usage = prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::SampledBit;
		imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
		imgCreateInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
		auto img = get_cengine()->GetRenderContext().CreateImage(imgCreateInfo);
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
		samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
		m_waterScene->texScene = get_cengine()->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	}
	if(m_waterScene->texSceneDepth == nullptr) {
		auto &sceneDepthImg = rasterC->GetPrepass().textureDepth->GetImage();
		auto extents = sceneDepthImg.GetExtents();
		prosper::util::ImageCreateInfo imgCreateInfo {};
		imgCreateInfo.width = extents.width;
		imgCreateInfo.height = extents.height;
		imgCreateInfo.format = sceneDepthImg.GetFormat();
		imgCreateInfo.usage = prosper::ImageUsageFlags::TransferDstBit | prosper::ImageUsageFlags::SampledBit;
		imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
		imgCreateInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
		auto img = get_cengine()->GetRenderContext().CreateImage(imgCreateInfo);
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
		samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
		m_waterScene->texSceneDepth = get_cengine()->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	}

	auto &descSetEffects = *m_waterScene->descSetGroupTexEffects->GetDescriptorSet();
	auto &reflectionTex = rasterC->GetHDRInfo().sceneRenderTarget->GetTexture();
	descSetEffects.SetBindingTexture(reflectionTex, math::to_integral(ShaderWater::WaterBinding::ReflectionMap));
	descSetEffects.SetBindingTexture(*m_waterScene->texScene, math::to_integral(ShaderWater::WaterBinding::RefractionMap));
	descSetEffects.SetBindingTexture(*m_waterScene->texSceneDepth, math::to_integral(ShaderWater::WaterBinding::RefractionDepth));
	descSetEffects.SetBindingUniformBuffer(*m_waterScene->settingsBuffer, math::to_integral(ShaderWater::WaterBinding::WaterSettings));

	ShaderPPFog::Fog fog {};
	auto fogEnabled = false;
	if(mat->HasPropertyBlock("fog")) {
		fogEnabled = mat->GetProperty("fog/enabled", true);
		fog.start = nearZ;
		if(mat->GetProperty("fog/start", &fog.start))
			fog.start = metres_to_units(fog.start);
		fog.end = farZ;
		if(mat->GetProperty("fog/end", &fog.end))
			fog.end = metres_to_units(fog.end);
		fog.density = mat->GetProperty("density", 0.008f);
		auto color = Color {68, 140, 200, 255}.ToVector4();
		if(mat->GetProperty("fog/color", &color))
			;
		fog.color = color;
	}
	if(fogEnabled)
		fog.flags = ShaderPPFog::Fog::Flag::Enabled;
	fog.type = ShaderPPFog::Fog::Type::Exponential;

	prosper::util::BufferCreateInfo fogBufCreateInfo {};
	fogBufCreateInfo.size = sizeof(fog);
	fogBufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	fogBufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	m_waterScene->fogBuffer = get_cengine()->GetRenderContext().CreateBuffer(fogBufCreateInfo, &fog);
	m_waterScene->fogDescSetGroup = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderPPFog::DESCRIPTOR_SET_FOG);
	auto &descSetFog = *m_waterScene->fogDescSetGroup->GetDescriptorSet();
	descSetFog.SetBindingUniformBuffer(*m_waterScene->fogBuffer, 0u);
	descSetEffects.SetBindingUniformBuffer(*m_waterScene->fogBuffer, math::to_integral(ShaderWater::WaterBinding::WaterFog));

	shaderPPWater.InitializeMaterialDescriptorSet(*mat);

	ecs::EntityIterator entIt {*get_cgame(), ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CRendererComponent>>();
	for(auto *ent : entIt)
		InitializeRenderer(*ent->GetComponent<CRendererComponent>());

	m_waterScene->hComponentCreationCallback = get_cgame()->GetEntityComponentManager().AddCreationCallback("renderer", [this](std::reference_wrapper<BaseEntityComponent> component) { InitializeRenderer(static_cast<CRendererComponent &>(component.get())); });

	InitializeRenderData();
}

void CLiquidSurfaceComponent::InitializeRenderer(CRendererComponent &component)
{
	auto handle = component.AddPostProcessingEffect("pp_water_overlay", [this](const rendering::DrawSceneInfo &drawSceneInfo) { RenderPostProcessingOverlay(drawSceneInfo); }, 380'000);
	m_waterScene->hPostProcessing.push_back(handle);
}

void CLiquidSurfaceComponent::RenderPostProcessingOverlay(const rendering::DrawSceneInfo &drawSceneInfo)
{
	auto renderFlags = drawSceneInfo.renderFlags;
	if(cvDrawWater->GetBool() == false || (renderFlags & rendering::RenderFlags::Water) == rendering::RenderFlags::None)
		return;
	if(get_cgame()->GetRenderScene<CSceneComponent>() != get_cgame()->GetScene<CSceneComponent>())
		return;
	auto *scene = get_cgame()->GetRenderScene<CSceneComponent>();
	auto *renderer = scene->GetRenderer<CRendererComponent>();
	auto rasterC = renderer ? renderer->GetEntity().GetComponent<CRasterizationRendererComponent>() : pragma::ComponentHandle<CRasterizationRendererComponent> {};
	auto camScene = scene ? scene->GetActiveCamera() : pragma::ComponentHandle<CCameraComponent> {};
	if(rasterC.expired() || camScene.expired())
		return;
	auto &waterScene = *m_waterScene;

	//scene->GetDepthTexture()->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal); // prosper TODO
	//auto &n = *waterNormal;
	//auto &planeDist = *waterPlaneDist;
	auto surfC = GetEntity().GetComponent<CSurfaceComponent>();
	if(surfC.expired())
		return;
	auto plane = surfC->GetPlaneWs();
	auto &n = plane.GetNormal();
	auto planeDist = plane.GetDistance();

	bool bCameraSubmerged;
	bool bCameraFullySubmerged;
	math::geometry::PlaneSide camCenterPlaneSide;
	is_camera_submerged(*camScene, n, planeDist, m_waterAabbBounds.first, m_waterAabbBounds.second, bCameraSubmerged, bCameraFullySubmerged, camCenterPlaneSide);

	if(camCenterPlaneSide != m_curCameraSurfaceSide) {
		m_curCameraSurfaceSide = camCenterPlaneSide;
		auto portalC = GetEntity().FindComponent("portal");
		if(portalC.valid())
			portalC->CallLuaMethod<void>("SetMirrored", camCenterPlaneSide == math::geometry::PlaneSide::Front);
	}

	if(bCameraSubmerged == true) {
		auto &hdrInfo = rasterC->GetHDRInfo();

		auto &descSetHdr = *hdrInfo.dsgHDRPostProcessing->GetDescriptorSet();
		auto *imgTex = descSetHdr.GetBoundImage(math::to_integral(ShaderPPHDR::TextureBinding::Texture));
		auto drawCmd = get_cgame()->GetCurrentDrawCommandBuffer();
		if(imgTex != nullptr)
			drawCmd->RecordImageBarrier(*imgTex, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);

		std::function<void(prosper::ICommandBuffer &)> fTransitionSampleImgToTransferDst = nullptr;
		// hdrInfo.BlitMainDepthBufferToSamplableDepthBuffer(drawSceneInfo.get(),fTransitionSampleImgToTransferDst);
		if(drawCmd->RecordBeginRenderPass(*hdrInfo.hdrPostProcessingRenderTarget) == true) {
			// drawCmd->RecordClearAttachment(hdrInfo.hdrPostProcessingRenderTarget->GetTexture().GetImage(),std::array<float,4>{1.f,0.f,0.f,1.f});

			auto *mat = GetWaterMaterial();
			auto &shaderPPWater = static_cast<ShaderPPWater &>(*m_shaderPpWater.get());
			prosper::ShaderBindState bindState {*drawCmd};
			if(mat != nullptr && mat->IsLoaded() == true && shaderPPWater.RecordBeginDraw(bindState) == true && shaderPPWater.RecordRefractionMaterial(bindState, *mat)) {
				shaderPPWater.RecordDraw(bindState, descSetHdr, *hdrInfo.dsgDepthPostProcessing->GetDescriptorSet(), *scene->GetCameraDescriptorSetGraphics(), get_cgame()->GetGlobalRenderSettingsDescriptorSet(), *waterScene.fogDescSetGroup->GetDescriptorSet(),
				  Vector4 {n.x, n.y, n.z, planeDist});
				shaderPPWater.RecordEndDraw(bindState);
			}

			// hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo.get());

			drawCmd->RecordEndRenderPass();
			hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
		}
	}
}
