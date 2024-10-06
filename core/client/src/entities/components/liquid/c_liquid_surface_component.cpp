/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/liquid/c_liquid_surface_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/c_surface_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/shaders/world/water/c_shader_water.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_water.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include <image/prosper_render_target.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/lua_component_event.hpp>
#include <pragma/math/intersection.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/util/util_game.hpp>
#include <prosper_command_buffer.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/base_lua_handle_method.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

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
		BindEventUnhandled(*evId, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
			auto renderTargetC = GetEntity().FindComponent("render_target");
			if(!renderTargetC.valid())
				return;
			auto *drawSceneInfo = renderTargetC->CallLuaMethod<util::DrawSceneInfo *>("GetDrawSceneInfo");
			if(drawSceneInfo) {
				auto maskWater = c_game->GetRenderMask("water");
				if(maskWater.has_value())
					drawSceneInfo->exclusionMask |= *maskWater; // Don't render water surfaces
			}

			auto *renderer = renderTargetC->CallLuaMethod<pragma::CRendererComponent *>("GetRenderer");
			if(renderer) {
				auto *tex = renderer->GetHDRPresentationTexture();
				if(tex && m_waterScene && m_waterScene->descSetGroupTexEffects) {
					auto *descSetEffects = m_waterScene->descSetGroupTexEffects->GetDescriptorSet();
					descSetEffects->SetBindingTexture(*tex, umath::to_integral(pragma::ShaderWater::WaterBinding::ReflectionMap));
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

	auto *entRt = c_game->CreateEntity("render_target");
	if(entRt) {
		m_hEntUnderwater = entRt->GetHandle();

		auto evId = GetComponentManager().FindEventId("render_target", "on_render_scene_initialized");
		auto rtC = entRt->FindComponent("render_target");
		if(rtC.valid() && evId.has_value()) {
			auto &cam = c_game->GetRenderScene()->GetActiveCamera();
			if(cam.valid())
				rtC->CallLuaMethod<void>("SetCamera", cam->GetLuaObject());

			rtC->AddEventCallback(*evId, [this, rtC](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				InitializeRenderData();
				return util::EventReply::Unhandled;
			});

			auto evPreRenderScene = GetComponentManager().FindEventId("render_target", "pre_render_scene");
			if(evPreRenderScene.has_value()) {
				rtC->AddEventCallback(*evPreRenderScene, [this, rtC](std::reference_wrapper<pragma::ComponentEvent> evData) {
					auto &data = static_cast<LuaComponentEvent &>(evData.get());
					if(data.arguments.size() > 0) {
						auto &arg = data.arguments[0];
						try {
							auto *drawSceneInfo = luabind::object_cast_nothrow<util::DrawSceneInfo *>(arg, static_cast<util::DrawSceneInfo *>(nullptr));
							if(drawSceneInfo) {
								auto maskWater = c_game->GetRenderMask("water");
								if(maskWater.has_value())
									drawSceneInfo->exclusionMask |= *maskWater; // Don't render water surfaces
								c_game->GetPrimaryCameraRenderMask(drawSceneInfo->inclusionMask, drawSceneInfo->exclusionMask);
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
			/*auto rasterC = c_game->GetRenderScene()->GetRenderer()->GetEntity().GetComponent<CRasterizationRendererComponent>();
			rasterC->AddEventCallback(CRasterizationRendererComponent::EVENT_MT_BEGIN_RECORD_WATER,
				[](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
					//auto *tex = c_game->GetRenderScene()->GetRenderer()->GetHDRPresentationTexture();
					//	m_waterScene->texScene
				return util::EventReply::Unhandled;
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
	auto *renderer = rtC->CallLuaMethod<pragma::CRendererComponent *>("GetRenderer");
	if(renderer) {
		auto *tex = renderer->GetHDRPresentationTexture();
		if(tex && m_waterScene && m_waterScene->descSetGroupTexEffects) {
			m_renderDataInitialized = true;

			auto *descSetEffects = m_waterScene->descSetGroupTexEffects->GetDescriptorSet();
			descSetEffects->SetBindingTexture(*tex, umath::to_integral(pragma::ShaderWater::WaterBinding::RefractionMap));

			auto rasterC = renderer->GetEntity().GetComponent<CRasterizationRendererComponent>();
			if(rasterC.valid()) {
				auto &texDepth = rasterC->GetHDRInfo().prepass.textureDepth;
				descSetEffects->SetBindingTexture(*texDepth, umath::to_integral(pragma::ShaderWater::WaterBinding::RefractionDepth));
				//descSetEffects.SetBindingUniformBuffer(*m_waterScene->settingsBuffer,umath::to_integral(pragma::ShaderWater::WaterBinding::WaterSettings));
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

CMaterial *CLiquidSurfaceComponent::GetWaterMaterial() const
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
	return (matIdx.has_value() && matIdx < mats.size()) ? static_cast<CMaterial *>(mats.at(*matIdx).get()) : nullptr;
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

void CLiquidSurfaceComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

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

static void is_camera_submerged(const pragma::CCameraComponent &cam, const Vector3 &n, float planeDist, const Vector3 &waterAabbMin, const Vector3 &waterAabbMax, bool &bCameraSubmerged, bool &bCameraFullySubmerged, umath::geometry::PlaneSide &camCenterPlaneSide)
{
	bCameraSubmerged = false;
	bCameraFullySubmerged = true;
	std::array<Vector3, 4> nearPlaneBounds;
	cam.GetNearPlaneBoundaries(nearPlaneBounds);

	auto posCenter = cam.GetNearPlaneCenter();
	camCenterPlaneSide = umath::geometry::get_side_of_point_to_plane(n, planeDist, posCenter);

	auto isPointInAabb = false;
	for(auto &p : nearPlaneBounds) {
		if(isPointInAabb == false && umath::intersection::vector_in_bounds(p, waterAabbMin, waterAabbMax))
			isPointInAabb = true;
		auto side = umath::geometry::get_side_of_point_to_plane(n, planeDist, p);
		if(side == umath::geometry::PlaneSide::Back)
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
	camCenterPlaneSide = umath::geometry::PlaneSide::Front;
}

static auto cvReflectionQuality = GetClientConVar("cl_render_reflection_quality");
static auto cvDrawWater = GetClientConVar("render_draw_water");
void CLiquidSurfaceComponent::InitializeWaterScene(const Vector3 &refPos, const Vector3 &planeNormal, const Vector3 &waterAabbMin, const Vector3 &waterAabbMax)
{
	c_engine->GetShaderManager().GetShader("water"); // Make sure water shader is loaded
	m_waterAabbBounds = {waterAabbMin, waterAabbMax};
	auto *mat = GetWaterMaterial();
	if(mat == nullptr || pragma::ShaderWater::DESCRIPTOR_SET_WATER.IsValid() == false || pragma::ShaderPPFog::DESCRIPTOR_SET_FOG.IsValid() == false)
		return;
	auto whShader = mat->GetPrimaryShader();
	auto *scene = c_game->GetScene();
	if(scene == nullptr)
		return;
	auto renderer = dynamic_cast<pragma::CRendererComponent *>(scene->GetRenderer());
	if(!whShader || renderer == nullptr)
		return;
	auto rasterC = renderer->GetEntity().GetComponent<CRasterizationRendererComponent>();
	if(rasterC.expired())
		return;
	auto *shader = dynamic_cast<pragma::ShaderWater *>(whShader);
	auto whShaderPPWater = c_engine->GetShader("pp_water");
	if(shader == nullptr || whShaderPPWater.expired())
		return;
	auto &shaderPPWater = static_cast<pragma::ShaderPPWater &>(*whShaderPPWater.get());
	m_shaderPpWater = shaderPPWater.GetHandle();

	auto *cam = c_game->GetPrimaryCamera();
	auto width = scene->GetWidth() / 2u;
	auto height = scene->GetHeight() / 2u;
	auto fov = cam ? cam->GetFOV() : pragma::BaseEnvCameraComponent::DEFAULT_FOV;
	auto nearZ = cam ? cam->GetNearZ() : pragma::BaseEnvCameraComponent::DEFAULT_NEAR_Z;
	auto farZ = cam ? cam->GetFarZ() : pragma::BaseEnvCameraComponent::DEFAULT_FAR_Z; // TODO: shared Property?
	auto *camReflection = c_game->CreateCamera(width, height, fov, nearZ, farZ);
	if(camReflection == nullptr)
		return;
	m_waterScene = std::make_unique<WaterScene>();
	m_waterScene->descSetGroupTexEffects = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderWater::DESCRIPTOR_SET_WATER);
	auto *sceneC = pragma::CSceneComponent::Create(pragma::CSceneComponent::CreateInfo {});
	auto &sceneReflection = m_waterScene->sceneReflection;
	sceneReflection = sceneC ? sceneC->GetHandle<pragma::CSceneComponent>() : pragma::ComponentHandle<pragma::CSceneComponent> {};
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
	m_waterScene->settingsBuffer = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo, waterSettings.data());
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
		auto img = c_engine->GetRenderContext().CreateImage(imgCreateInfo);
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
		samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
		m_waterScene->texScene = c_engine->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
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
		auto img = c_engine->GetRenderContext().CreateImage(imgCreateInfo);
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToEdge;
		samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToEdge;
		m_waterScene->texSceneDepth = c_engine->GetRenderContext().CreateTexture({}, *img, imgViewCreateInfo, samplerCreateInfo);
	}

	auto &descSetEffects = *m_waterScene->descSetGroupTexEffects->GetDescriptorSet();
	auto &reflectionTex = rasterC->GetHDRInfo().sceneRenderTarget->GetTexture();
	descSetEffects.SetBindingTexture(reflectionTex, umath::to_integral(pragma::ShaderWater::WaterBinding::ReflectionMap));
	descSetEffects.SetBindingTexture(*m_waterScene->texScene, umath::to_integral(pragma::ShaderWater::WaterBinding::RefractionMap));
	descSetEffects.SetBindingTexture(*m_waterScene->texSceneDepth, umath::to_integral(pragma::ShaderWater::WaterBinding::RefractionDepth));
	descSetEffects.SetBindingUniformBuffer(*m_waterScene->settingsBuffer, umath::to_integral(pragma::ShaderWater::WaterBinding::WaterSettings));

	pragma::ShaderPPFog::Fog fog {};
	auto &dataBlock = mat->GetDataBlock();
	auto fogEnabled = false;
	auto fogBlock = dataBlock->GetBlock("fog");
	if(fogBlock) {
		fogEnabled = fogBlock->GetBool("enabled", true);
		fog.start = nearZ;
		if(fogBlock->GetFloat("start", &fog.start))
			fog.start = pragma::metres_to_units(fog.start);
		fog.end = farZ;
		if(fogBlock->GetFloat("end", &fog.end))
			fog.end = pragma::metres_to_units(fog.end);
		fog.density = fogBlock->GetFloat("density", 0.008f);
		Color color {68, 140, 200, 255};
		if(fogBlock->GetColor("color", &color))
			;
		fog.color = color.ToVector4();
	}
	if(fogEnabled)
		fog.flags = pragma::ShaderPPFog::Fog::Flag::Enabled;
	fog.type = pragma::ShaderPPFog::Fog::Type::Exponential;

	prosper::util::BufferCreateInfo fogBufCreateInfo {};
	fogBufCreateInfo.size = sizeof(fog);
	fogBufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	fogBufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	m_waterScene->fogBuffer = c_engine->GetRenderContext().CreateBuffer(fogBufCreateInfo, &fog);
	m_waterScene->fogDescSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderPPFog::DESCRIPTOR_SET_FOG);
	auto &descSetFog = *m_waterScene->fogDescSetGroup->GetDescriptorSet();
	descSetFog.SetBindingUniformBuffer(*m_waterScene->fogBuffer, 0u);
	descSetEffects.SetBindingUniformBuffer(*m_waterScene->fogBuffer, umath::to_integral(pragma::ShaderWater::WaterBinding::WaterFog));

	shaderPPWater.InitializeMaterialDescriptorSet(*mat);

	EntityIterator entIt {*c_game, EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRendererComponent>>();
	for(auto *ent : entIt)
		InitializeRenderer(*ent->GetComponent<pragma::CRendererComponent>());

	m_waterScene->hComponentCreationCallback = c_game->GetEntityComponentManager().AddCreationCallback("renderer", [this](std::reference_wrapper<pragma::BaseEntityComponent> component) { InitializeRenderer(static_cast<pragma::CRendererComponent &>(component.get())); });

	InitializeRenderData();
}

void CLiquidSurfaceComponent::InitializeRenderer(pragma::CRendererComponent &component)
{
	auto handle = component.AddPostProcessingEffect(
	  "pp_water_overlay", [this](const util::DrawSceneInfo &drawSceneInfo) { RenderPostProcessingOverlay(drawSceneInfo); }, 380'000);
	m_waterScene->hPostProcessing.push_back(handle);
}

void CLiquidSurfaceComponent::RenderPostProcessingOverlay(const util::DrawSceneInfo &drawSceneInfo)
{
	auto renderFlags = drawSceneInfo.renderFlags;
	if(cvDrawWater->GetBool() == false || (renderFlags & RenderFlags::Water) == RenderFlags::None)
		return;
	if(c_game->GetRenderScene() != c_game->GetScene())
		return;
	auto *scene = c_game->GetRenderScene();
	auto *renderer = scene->GetRenderer();
	auto rasterC = renderer ? renderer->GetEntity().GetComponent<pragma::CRasterizationRendererComponent>() : pragma::ComponentHandle<pragma::CRasterizationRendererComponent> {};
	auto camScene = scene ? scene->GetActiveCamera() : pragma::ComponentHandle<pragma::CCameraComponent> {};
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
	umath::geometry::PlaneSide camCenterPlaneSide;
	is_camera_submerged(*camScene, n, planeDist, m_waterAabbBounds.first, m_waterAabbBounds.second, bCameraSubmerged, bCameraFullySubmerged, camCenterPlaneSide);

	if(camCenterPlaneSide != m_curCameraSurfaceSide) {
		m_curCameraSurfaceSide = camCenterPlaneSide;
		auto portalC = GetEntity().FindComponent("portal");
		if(portalC.valid())
			portalC->CallLuaMethod<void>("SetMirrored", camCenterPlaneSide == umath::geometry::PlaneSide::Front);
	}

	if(bCameraSubmerged == true) {
		auto &hdrInfo = rasterC->GetHDRInfo();

		auto &descSetHdr = *hdrInfo.dsgHDRPostProcessing->GetDescriptorSet();
		auto *imgTex = descSetHdr.GetBoundImage(umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Texture));
		auto drawCmd = c_game->GetCurrentDrawCommandBuffer();
		if(imgTex != nullptr)
			drawCmd->RecordImageBarrier(*imgTex, prosper::ImageLayout::ColorAttachmentOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal);

		std::function<void(prosper::ICommandBuffer &)> fTransitionSampleImgToTransferDst = nullptr;
		// hdrInfo.BlitMainDepthBufferToSamplableDepthBuffer(drawSceneInfo.get(),fTransitionSampleImgToTransferDst);
		if(drawCmd->RecordBeginRenderPass(*hdrInfo.hdrPostProcessingRenderTarget) == true) {
			// drawCmd->RecordClearAttachment(hdrInfo.hdrPostProcessingRenderTarget->GetTexture().GetImage(),std::array<float,4>{1.f,0.f,0.f,1.f});

			auto *mat = GetWaterMaterial();
			auto &shaderPPWater = static_cast<pragma::ShaderPPWater &>(*m_shaderPpWater.get());
			prosper::ShaderBindState bindState {*drawCmd};
			if(mat != nullptr && mat->IsLoaded() == true && shaderPPWater.RecordBeginDraw(bindState) == true && shaderPPWater.RecordRefractionMaterial(bindState, *mat)) {
				shaderPPWater.RecordDraw(bindState, descSetHdr, *hdrInfo.dsgDepthPostProcessing->GetDescriptorSet(), *scene->GetCameraDescriptorSetGraphics(), c_game->GetGlobalRenderSettingsDescriptorSet(), *waterScene.fogDescSetGroup->GetDescriptorSet(),
				  Vector4 {n.x, n.y, n.z, planeDist});
				shaderPPWater.RecordEndDraw(bindState);
			}

			// hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo.get());

			drawCmd->RecordEndRenderPass();
			hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo);
		}
	}
}
