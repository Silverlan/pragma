/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/c_water_object.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_water.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/c_renderflags.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <glm/gtx/projection.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/math/intersection.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

CWaterObject::WaterScene::~WaterScene()
{
	if(hRenderScene.IsValid())
		hRenderScene.Remove();
	if(hRender.IsValid())
		hRender.Remove();
	if(hPostProcessing.IsValid())
		hPostProcessing.Remove();
}

const CWaterObject::WaterScene &CWaterObject::GetWaterScene() const {return *m_waterScene;}
bool CWaterObject::IsWaterSceneValid() const {return m_waterScene != nullptr;}

prosper::IDescriptorSet *CWaterObject::GetEffectDescriptorSet() const
{
	if(m_waterScene == nullptr)
		return nullptr;
	return m_waterScene->descSetGroupTexEffects->GetDescriptorSet();
}

void CWaterObject::InitializeWaterScene(const WaterScene &scene)
{
	m_waterScene = std::make_unique<WaterScene>();
	m_waterScene->sceneReflection = scene.sceneReflection;
	m_waterScene->descSetGroupTexEffects = scene.descSetGroupTexEffects;
	m_waterScene->hRenderScene = scene.hRenderScene;
	m_waterScene->hRender = scene.hRender;
	m_waterScene->hPostProcessing = scene.hPostProcessing;
	m_waterScene->settingsBuffer = scene.settingsBuffer;

	m_waterScene->fogBuffer = scene.fogBuffer;
	m_waterScene->fogDescSetGroup = scene.fogDescSetGroup;

	m_waterScene->waterScale = scene.waterScale;
	m_waterScene->waveStrength = scene.waveStrength;
	m_waterScene->waveSpeed = scene.waveSpeed;
	m_waterScene->reflectiveIntensity = scene.reflectiveIntensity;
}

static void is_camera_submerged(const pragma::CCameraComponent &cam,const Vector3 &n,float planeDist,const Vector3 &waterAabbMin,const Vector3 &waterAabbMax,bool &bCameraSubmerged,bool &bCameraFullySubmerged)
{
	bCameraSubmerged = false;
	bCameraFullySubmerged = true;
	std::array<Vector3,4> nearPlaneBounds;
	cam.GetNearPlaneBoundaries(nearPlaneBounds);

	auto isPointInAabb = false;
	for(auto &p : nearPlaneBounds)
	{
		if(isPointInAabb == false && Intersection::VectorInBounds(p,waterAabbMin,waterAabbMax))
			isPointInAabb = true;

		auto dot = glm::dot(Vector4(p.x,p.y,p.z,1.f),Vector4(-n.x,-n.y,-n.z,-planeDist));
		if(dot > 0.f)
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
}

static auto cvReflectionQuality = GetClientConVar("cl_render_reflection_quality");
static auto cvDrawWater = GetClientConVar("render_draw_water");
void CWaterObject::InitializeWaterScene(const Vector3 &refPos,const Vector3 &planeNormal,const Vector3 &waterAabbMin,const Vector3 &waterAabbMax)
{
	m_waterAabbBounds = {waterAabbMin,waterAabbMax};
	auto *mat = GetWaterMaterial();
	if(mat == nullptr || pragma::ShaderWater::DESCRIPTOR_SET_WATER.IsValid() == false || pragma::ShaderPPFog::DESCRIPTOR_SET_FOG.IsValid() == false)
		return;
	auto whShader = mat->GetPrimaryShader();
	auto &scene = c_game->GetScene();
	auto renderer = dynamic_cast<pragma::rendering::RasterizationRenderer*>(scene->GetRenderer());
	if(whShader.expired() || renderer == nullptr)
		return;
	auto *shader = dynamic_cast<pragma::ShaderWater*>(whShader.get());
	auto whShaderPPWater = c_engine->GetShader("pp_water");
	if(shader == nullptr || whShaderPPWater.expired())
		return;
	auto &shaderPPWater = static_cast<pragma::ShaderPPWater&>(*whShaderPPWater.get());

	auto *cam = c_game->GetPrimaryCamera();
	auto width = scene->GetWidth() /2u;
	auto height = scene->GetHeight() /2u;
	auto fov = cam ? cam->GetFOV() : pragma::BaseEnvCameraComponent::DEFAULT_FOV;
	auto nearZ = cam ? cam->GetNearZ() : pragma::BaseEnvCameraComponent::DEFAULT_NEAR_Z;
	auto farZ = cam ? cam->GetFarZ() : pragma::BaseEnvCameraComponent::DEFAULT_FAR_Z; // TODO: shared Property?
	auto *camReflection = c_game->CreateCamera(width,height,fov,nearZ,farZ);
	if(camReflection == nullptr)
		return;
	m_waterScene = std::make_unique<WaterScene>();
	m_waterScene->descSetGroupTexEffects = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderWater::DESCRIPTOR_SET_WATER);
	auto &sceneReflection = m_waterScene->sceneReflection = Scene::Create(Scene::CreateInfo{});
	sceneReflection->ReloadRenderTarget(width,height);
	if(cam)
	{
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

	std::array<float,4> waterSettings = {*m_waterScene->waterScale,*m_waterScene->waveStrength,*m_waterScene->waveSpeed,*m_waterScene->reflectiveIntensity};
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	bufCreateInfo.size = waterSettings.size() *sizeof(waterSettings.front());
	m_waterScene->settingsBuffer = c_engine->GetRenderContext().CreateBuffer(bufCreateInfo,waterSettings.data());
	//

	if(m_waterScene->texScene == nullptr)
	{
		auto &sceneImg = renderer->GetHDRInfo().sceneRenderTarget->GetTexture().GetImage();
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
		m_waterScene->texScene = c_engine->GetRenderContext().CreateTexture({},*img,imgViewCreateInfo,samplerCreateInfo);
	}
	if(m_waterScene->texSceneDepth == nullptr)
	{
		auto &sceneDepthImg = renderer->GetPrepass().textureDepth->GetImage();
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
		m_waterScene->texSceneDepth = c_engine->GetRenderContext().CreateTexture({},*img,imgViewCreateInfo,samplerCreateInfo);
	}

	auto &descSetEffects = *m_waterScene->descSetGroupTexEffects->GetDescriptorSet();
	auto &reflectionTex = renderer->GetHDRInfo().sceneRenderTarget->GetTexture();
	descSetEffects.SetBindingTexture(reflectionTex,umath::to_integral(pragma::ShaderWater::WaterBinding::ReflectionMap));
	descSetEffects.SetBindingTexture(*m_waterScene->texScene,umath::to_integral(pragma::ShaderWater::WaterBinding::RefractionMap));
	descSetEffects.SetBindingTexture(*m_waterScene->texSceneDepth,umath::to_integral(pragma::ShaderWater::WaterBinding::RefractionDepth));
	descSetEffects.SetBindingUniformBuffer(*m_waterScene->settingsBuffer,umath::to_integral(pragma::ShaderWater::WaterBinding::WaterSettings));

	pragma::ShaderPPFog::Fog fog {};
	fog.flags = pragma::ShaderPPFog::Fog::Flag::Enabled;
	fog.start = 400.f;
	fog.end = 1'600.f;
	fog.color = Color(68,140,200,255).ToVector4();
	fog.type = pragma::ShaderPPFog::Fog::Type::Exponential;
	fog.density = 0.008f;
	prosper::util::BufferCreateInfo fogBufCreateInfo {};
	fogBufCreateInfo.size = sizeof(fog);
	fogBufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	fogBufCreateInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	m_waterScene->fogBuffer = c_engine->GetRenderContext().CreateBuffer(fogBufCreateInfo,&fog);
	m_waterScene->fogDescSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderPPFog::DESCRIPTOR_SET_FOG);
	auto &descSetFog = *m_waterScene->fogDescSetGroup->GetDescriptorSet();
	descSetFog.SetBindingUniformBuffer(*m_waterScene->fogBuffer,0u);
	descSetEffects.SetBindingUniformBuffer(*m_waterScene->fogBuffer,umath::to_integral(pragma::ShaderWater::WaterBinding::WaterFog));

	auto pos = refPos;
	auto surfaceNormal = planeNormal;
	auto v = std::numeric_limits<float>::max();
	Vector3 lastPos = {v,v,v};
	Quat lastRot = {v,v,v,v};
	auto waterPlaneDist = std::make_shared<float>(std::numeric_limits<float>::max());
	auto waterNormal = std::make_shared<Vector3>(surfaceNormal);
	Mat4 matReflect;
	m_waterScene->hRender = c_game->AddCallback("Render",FunctionCallback<void,std::reference_wrapper<const util::DrawSceneInfo>>::Create([this](std::reference_wrapper<const util::DrawSceneInfo> drawSceneInfo) {
		// This is called before the water render pass
		if(cvDrawWater->GetBool() == false)
			return;
		auto &scene = c_game->GetRenderScene();
		if(scene != c_game->GetScene())
			return;
		auto *renderer = scene ? dynamic_cast<pragma::rendering::RasterizationRenderer*>(scene->GetRenderer()) : nullptr;
		if(renderer == nullptr)
			return;
		auto drawCmd = drawSceneInfo.get().commandBuffer;
		renderer->EndRenderPass(drawSceneInfo.get()); // The current render pass needs to be ended so we can blit the scene texture and depth texture

		auto &hdrInfo = renderer->GetHDRInfo();
		auto &tex = hdrInfo.sceneRenderTarget->GetTexture();
		//auto &tex = hdrInfo.texture->GetTexture();

		auto &waterScene = *m_waterScene;
		//tex->GetImage()->SetDrawLayout(prosper::ImageLayout::TransferSrcOptimal);
		//waterScene.texScene->GetImage()->SetDrawLayout(prosper::ImageLayout::TransferDstOptimal);
		//auto &context = tex->GetContext(); // prosper TODO
		//auto &drawCmd = context.GetDrawCmd(); // prosper TODO

		auto &imgWaterScene = waterScene.texScene->GetImage();
		drawCmd->RecordImageBarrier(tex.GetImage(),prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal);
		drawCmd->RecordImageBarrier(imgWaterScene,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);

		drawCmd->RecordBlitTexture(tex,imgWaterScene);

		drawCmd->RecordImageBarrier(tex.GetImage(),prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::ColorAttachmentOptimal);
		drawCmd->RecordImageBarrier(imgWaterScene,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);
		//tex->GetImage()->SetDrawLayout(prosper::ImageLayout::ColorAttachmentOptimal); // prosper TODO

		// Depth
		auto &prepass = renderer->GetPrepass();
		auto &depthTex = *prepass.textureDepth;

		// We need the depth buffer in the water shader, but the water shader has depth write enabled, so we need to copy
		// the depth buffer before the water is being rendered.
		// 'CopyImage' automatically resolves the image if it's multi-sampled.
		//depthTex->GetImage()->SetDrawLayout(prosper::ImageLayout::TransferSrcOptimal); // prosper TODO
		//waterScene.texSceneDepth->GetImage()->SetDrawLayout(prosper::ImageLayout::TransferDstOptimal); // prosper TODO

		auto &imgWaterSceneDepth = waterScene.texSceneDepth->GetImage();
		drawCmd->RecordImageBarrier(depthTex.GetImage(),prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::ImageLayout::TransferSrcOptimal);
		drawCmd->RecordImageBarrier(imgWaterSceneDepth,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::TransferDstOptimal);

		drawCmd->RecordBlitTexture(depthTex,imgWaterSceneDepth);

		drawCmd->RecordImageBarrier(depthTex.GetImage(),prosper::ImageLayout::TransferSrcOptimal,prosper::ImageLayout::DepthStencilAttachmentOptimal);
		drawCmd->RecordImageBarrier(imgWaterSceneDepth,prosper::ImageLayout::TransferDstOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);

		// TODO: The destination depth image contains red-color components after the copy, why? (Compare debug_prepass and debug_water)

		//waterScene.texScene->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal); // prosper TODO
		//waterScene.texSceneDepth->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal); // prosper TODO
		//depthTex->GetImage()->SetDrawLayout(prosper::ImageLayout::DepthStencilAttachmentOptimal); // prosper TODO
		renderer->BeginRenderPass(drawSceneInfo.get(),prosper::ShaderGraphics::GetRenderPass<pragma::ShaderParticle2DBase>(c_engine->GetRenderContext()).get()); // Restart the render pass
	}));
	m_waterScene->hPostProcessing = c_game->AddCallback("RenderPostProcessing",FunctionCallback<void,std::reference_wrapper<const util::DrawSceneInfo>>::Create([this,waterNormal,waterPlaneDist,whShaderPPWater](std::reference_wrapper<const util::DrawSceneInfo> drawSceneInfo) {
		auto renderFlags = drawSceneInfo.get().renderFlags;
		if(cvDrawWater->GetBool() == false || (renderFlags &FRender::Water) == FRender::None)
			return;
		if(c_game->GetRenderScene() != c_game->GetScene())
			return;
		auto &scene = c_game->GetRenderScene();
		auto *renderer = scene ? dynamic_cast<pragma::rendering::RasterizationRenderer*>(scene->GetRenderer()) : nullptr;
		auto camScene = scene ? scene->GetActiveCamera() : util::WeakHandle<pragma::CCameraComponent>{};
		if(renderer == nullptr || camScene.expired())
			return;
		auto &waterScene = *m_waterScene;

		//scene->GetDepthTexture()->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal); // prosper TODO
		auto &n = *waterNormal;
		auto &planeDist = *waterPlaneDist;

		auto bCameraSubmerged = false;
		auto bCameraFullySubmerged = true;
		is_camera_submerged(*camScene,n,planeDist,m_waterAabbBounds.first,m_waterAabbBounds.second,bCameraSubmerged,bCameraFullySubmerged);
		if(bCameraSubmerged == true)
		{
			auto &hdrInfo = renderer->GetHDRInfo();

			auto &descSetHdr = *hdrInfo.dsgHDRPostProcessing->GetDescriptorSet();
			auto *imgTex = descSetHdr.GetBoundImage(umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Texture));
			auto drawCmd = c_game->GetCurrentDrawCommandBuffer();
			if(imgTex != nullptr)
				drawCmd->RecordImageBarrier(*imgTex,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);

			std::function<void(prosper::ICommandBuffer&)> fTransitionSampleImgToTransferDst = nullptr;
			hdrInfo.BlitMainDepthBufferToSamplableDepthBuffer(drawSceneInfo.get(),fTransitionSampleImgToTransferDst);
			if(drawCmd->RecordBeginRenderPass(*hdrInfo.hdrPostProcessingRenderTarget) == true)
			{
				auto &prepass = renderer->GetPrepass();
				//auto &texDepth = prepass.textureDepth->Resolve(); // prosper TODO
				//texDepth->GetImage()->SetDrawLayout(prosper::ImageLayout::ShaderReadOnlyOptimal); // prosper TODO

				auto *mat = GetWaterMaterial();
				auto &shaderPPWater = static_cast<pragma::ShaderPPWater&>(*whShaderPPWater.get());
				if(mat != nullptr && mat->IsLoaded() == true && shaderPPWater.BeginDraw(drawCmd) == true && shaderPPWater.BindRefractionMaterial(*mat))
				{
					shaderPPWater.Draw(
						descSetHdr,
						*hdrInfo.dsgSceneDepth->GetDescriptorSet(),
						*scene->GetCameraDescriptorSetGraphics(),
						c_game->GetGlobalRenderSettingsDescriptorSet(),
						*waterScene.fogDescSetGroup->GetDescriptorSet(),
						Vector4{n.x,n.y,n.z,planeDist}
					);
					shaderPPWater.EndDraw();
				}
				drawCmd->RecordEndRenderPass();
			}
			//fTransitionSampleImgToTransferDst(*drawCmd);
			hdrInfo.BlitStagingRenderTargetToMainRenderTarget(drawSceneInfo.get());
		}
	}));
	m_waterScene->hRenderScene = c_game->AddCallback("DrawScene",FunctionCallback<
		bool,std::reference_wrapper<const util::DrawSceneInfo>
	>::CreateWithOptionalReturn([this,shader,pos,surfaceNormal,waterNormal,lastPos,lastRot,waterPlaneDist,matReflect](
		bool *bSkipMainScene,std::reference_wrapper<const util::DrawSceneInfo> drawSceneInfo
	) mutable -> CallbackReturnType {
		if(cvDrawWater->GetBool() == false)
			return CallbackReturnType::NoReturnValue;
		const auto offset = 0.005f; // Offset to avoid rendering artifacts
		auto &n = *waterNormal;
		auto &planeDist = *waterPlaneDist;

		auto &newPos = GetPosition();
		auto &newRot = GetOrientation();
		if(newPos != lastPos || newRot != lastRot) // Only update these if position has changed
		{
			lastPos = newPos;
			lastRot = newRot;

			auto waterDist = 0.0;
			GetWaterPlaneWs(n,waterDist);
			planeDist = -waterDist;

			// Update reflection matrix
			auto d = planeDist -offset;
			matReflect = Mat4{
				1.0 -2.0 *n.x *n.x,-2.0 *n.x *n.y,-2.0 *n.x *n.z,0,
				-2.0 *n.x *n.y,1.0 -2.0 *n.y *n.y,-2.0 *n.y *n.z,0,
				-2.0 *n.x *n.z,-2.0 *n.y *n.z,1.0 -2.0 *n.z *n.z,0,
				0,0,0,1
			};
			matReflect = glm::translate(matReflect,n *d *2.f);
		}

		auto &scene = c_game->GetRenderScene();
		auto &camScene = scene->GetActiveCamera();
		if(camScene.expired())
			return CallbackReturnType::NoReturnValue;
		auto &matView = camScene->GetViewMatrix();
		auto &matProj = camScene->GetProjectionMatrix();
		auto &camPos = camScene->GetEntity().GetPosition();
		auto camProj = uvec::project_to_plane(camPos,n,-planeDist);
		auto dot = uvec::dot(n,camPos -camProj);
		auto bBelowSurface = (dot < 0.f) ? true : false;
		auto planeSign = (bBelowSurface == true) ? -1.f : 1.f; // Flip the plane if under water

		shader->SetReflectionEnabled(!bBelowSurface); // No need for reflection effects if we're below the surface
		if(bBelowSurface == false)
		{
			shader->SetReflectionEnabled(true);
			// Render reflection
			auto reflectionQuality = cvReflectionQuality->GetInt();
			auto bRenderDynamic = (reflectionQuality > 0) ? true : false;
			auto bRenderReflection = (bRenderDynamic == true || m_reflectionRendered != 2) ? true : false;
			if(bRenderReflection == true)
			{
				auto &sceneReflection = m_waterScene->sceneReflection;
				auto *renderer = sceneReflection ? dynamic_cast<pragma::rendering::RasterizationRenderer*>(sceneReflection->GetRenderer()) : nullptr;
				if(renderer)
				{
					auto &rtReflection = renderer->GetHDRInfo().sceneRenderTarget;
					auto &camReflection = sceneReflection->GetActiveCamera();
					if(camReflection.valid())
					{
						camReflection->GetEntity().SetPosition(camPos);
						camReflection->GetEntity().SetRotation(camScene->GetEntity().GetRotation());
						camReflection->UpdateMatrices();

						camReflection->SetProjectionMatrix(matProj);
						auto matReflView = camReflection->GetViewMatrix();
						matReflView *= matReflect;
						camReflection->SetViewMatrix(matReflView);

						// Reflect camera position (Has to be done AFTER matrices have been updated!)
						auto posReflected = Vector4(camPos.x,camPos.y,camPos.z,1.f);
						posReflected = glm::inverse(camScene->GetViewMatrix()) *matReflView *posReflected;
						camReflection->GetEntity().SetPosition({posReflected.x,posReflected.y,posReflected.z});
					}

					auto renderFlags = FRender::World | FRender::Skybox | FRender::Reflection;
					if(reflectionQuality == 0 || reflectionQuality > 1)
						renderFlags |= FRender::Glow;
					if(reflectionQuality > 1)
						renderFlags |= FRender::Particles;
					//rtReflection->GetTexture()->GetImage()->SetDrawLayout(prosper::ImageLayout::ColorAttachmentOptimal); // prosper TODO
					//sceneReflection->GetDepthTexture()->GetImage()->SetDrawLayout(prosper::ImageLayout::DepthStencilAttachmentOptimal); // prosper TODO

					auto &imgReflection = rtReflection->GetTexture().GetImage();
					auto &drawCmd = drawSceneInfo.get().commandBuffer;
					drawCmd.get()->RecordImageBarrier(imgReflection,prosper::ImageLayout::ShaderReadOnlyOptimal,prosper::ImageLayout::ColorAttachmentOptimal);

					util::DrawSceneInfo reflectionDrawSceneInfo {};
					reflectionDrawSceneInfo.outputImage = imgReflection.shared_from_this();
					reflectionDrawSceneInfo.renderFlags = renderFlags;
					reflectionDrawSceneInfo.outputLayerId = 0u;
					c_game->SetRenderClipPlane({n.x *planeSign,n.y *planeSign,n.z *planeSign,(planeDist -offset *planeSign) *planeSign});
						c_game->SetRenderScene(sceneReflection);
							c_game->RenderScene(drawSceneInfo.get());
						c_game->SetRenderScene(nullptr);
					c_game->SetRenderClipPlane({});

					drawCmd.get()->RecordImageBarrier(imgReflection,prosper::ImageLayout::ColorAttachmentOptimal,prosper::ImageLayout::ShaderReadOnlyOptimal);

					m_reflectionRendered = (bRenderDynamic == true) ? 1 : 2;
				}
			}
		}
		return CallbackReturnType::NoReturnValue;
	}));
}
