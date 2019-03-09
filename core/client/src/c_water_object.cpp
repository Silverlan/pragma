#include "stdafx_client.h"
#include "pragma/c_water_object.hpp"
#include "pragma/rendering/shaders/world/water/c_shader_water.hpp"
#include "pragma/rendering/shaders/c_shader_refraction.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_water.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/c_renderflags.h"
#include <glm/gtx/projection.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
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

Anvil::DescriptorSet *CWaterObject::GetEffectDescriptorSet() const
{
	if(m_waterScene == nullptr)
		return nullptr;
	return (*m_waterScene->descSetGroupTexEffects)->get_descriptor_set(0u);
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

static void is_camera_submerged(const Camera &cam,const Vector3 &n,float planeDist,bool &bCameraSubmerged,bool &bCameraFullySubmerged)
{
	bCameraSubmerged = false;
	bCameraFullySubmerged = true;
	std::vector<Vector3> nearPlaneBounds;
	nearPlaneBounds.reserve(4);
	cam.GetNearPlaneBoundaries(&nearPlaneBounds);
	for(auto &p : nearPlaneBounds)
	{
		auto dot = glm::dot(Vector4(p.x,p.y,p.z,1.f),Vector4(-n.x,-n.y,-n.z,-planeDist));
		if(dot > 0.f)
			bCameraSubmerged = true;
		else
			bCameraFullySubmerged = false;
	}
}

static auto cvReflectionQuality = GetClientConVar("cl_render_reflection_quality");
static auto cvDrawWater = GetClientConVar("render_draw_water");
void CWaterObject::InitializeWaterScene(const Vector3 &refPos,const Vector3 &planeNormal)
{
	auto *mat = GetWaterMaterial();
	if(mat == nullptr || pragma::ShaderWater::DESCRIPTOR_SET_WATER.IsValid() == false || pragma::ShaderPPFog::DESCRIPTOR_SET_FOG.IsValid() == false)
		return;
	auto whShader = mat->GetPrimaryShader();
	if(whShader.expired())
		return;
	auto *shader = dynamic_cast<pragma::ShaderWater*>(whShader.get());
	if(shader == nullptr)
		return;
	auto whShaderPPWater = c_engine->GetShader("pp_water");
	if(whShaderPPWater.expired())
		return;
	auto &shaderPPWater = static_cast<pragma::ShaderPPWater&>(*whShaderPPWater.get());

	auto &scene = c_game->GetScene();
	auto &cam = c_game->GetSceneCamera();
	auto width = scene->GetWidth() /2u;//512;//2048;
	auto height = scene->GetHeight() /2u;//512;//2048; // TODO
	auto fov = cam.GetFOV();
	auto fovView = cam.GetViewFOV();
	auto nearZ = cam.GetZNear();
	auto farZ = cam.GetZFar(); // TODO: shared Property?

	auto &dev = c_engine->GetDevice();
	m_waterScene = std::make_unique<WaterScene>();
	m_waterScene->descSetGroupTexEffects = prosper::util::create_descriptor_set_group(dev,pragma::ShaderWater::DESCRIPTOR_SET_WATER);
	auto &sceneReflection = m_waterScene->sceneReflection = Scene::Create(Scene::CreateInfo{width,height,fov,fovView,nearZ,farZ});
	sceneReflection->InitializeRenderTarget();
	auto &camReflection = sceneReflection->camera;
	camReflection->GetFOVProperty()->Link(*cam.GetFOVProperty());
	camReflection->GetViewFOVProperty()->Link(*cam.GetViewFOVProperty());
	camReflection->GetNearZProperty()->Link(*cam.GetNearZProperty());
	camReflection->GetFarZProperty()->Link(*cam.GetFarZProperty());
	sceneReflection->LinkLightSources(*scene);
	sceneReflection->LinkEntities(*scene);

	// Initialize water settings
	m_waterScene->waterScale = util::FloatProperty::Create(4.f);
	m_waterScene->waveStrength = util::FloatProperty::Create(0.004f);
	m_waterScene->waveSpeed = util::FloatProperty::Create(1.f); // ?
	m_waterScene->reflectiveIntensity = util::FloatProperty::Create(1.2f);

	std::array<float,4> waterSettings = {*m_waterScene->waterScale,*m_waterScene->waveStrength,*m_waterScene->waveSpeed,*m_waterScene->reflectiveIntensity};
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	bufCreateInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT;
	bufCreateInfo.size = waterSettings.size() *sizeof(waterSettings.front());
	m_waterScene->settingsBuffer = prosper::util::create_buffer(dev,bufCreateInfo,waterSettings.data());
	//

	if(m_waterScene->texScene == nullptr)
	{
		auto &sceneImg = c_game->GetScene()->GetHDRInfo().hdrRenderTarget->GetTexture()->GetImage();
		auto extents = sceneImg->GetExtents();
		prosper::util::ImageCreateInfo imgCreateInfo {};
		imgCreateInfo.width = extents.width;
		imgCreateInfo.height = extents.height;
		imgCreateInfo.format = sceneImg->GetFormat();
		imgCreateInfo.usage = Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
		imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
		imgCreateInfo.postCreateLayout = Anvil::ImageLayout::TRANSFER_DST_OPTIMAL;
		auto img = prosper::util::create_image(dev,imgCreateInfo);
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		m_waterScene->texScene = prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
	}
	if(m_waterScene->texSceneDepth == nullptr)
	{
		auto &sceneDepthImg = c_game->GetScene()->GetPrepass().textureDepth->GetImage();
		auto extents = sceneDepthImg->GetExtents();
		prosper::util::ImageCreateInfo imgCreateInfo {};
		imgCreateInfo.width = extents.width;
		imgCreateInfo.height = extents.height;
		imgCreateInfo.format = sceneDepthImg->GetFormat();
		imgCreateInfo.usage = Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
		imgCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
		imgCreateInfo.postCreateLayout = Anvil::ImageLayout::TRANSFER_DST_OPTIMAL;
		auto img = prosper::util::create_image(dev,imgCreateInfo);
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		m_waterScene->texSceneDepth = prosper::util::create_texture(dev,{},img,&imgViewCreateInfo,&samplerCreateInfo);
	}

	auto &descSetEffects = *(*m_waterScene->descSetGroupTexEffects)->get_descriptor_set(0u);
	auto &reflectionTex = m_waterScene->sceneReflection->GetHDRInfo().hdrRenderTarget->GetTexture();
	prosper::util::set_descriptor_set_binding_texture(descSetEffects,*reflectionTex,umath::to_integral(pragma::ShaderWater::WaterBinding::ReflectionMap));
	prosper::util::set_descriptor_set_binding_texture(descSetEffects,*m_waterScene->texScene,umath::to_integral(pragma::ShaderWater::WaterBinding::RefractionMap));
	prosper::util::set_descriptor_set_binding_texture(descSetEffects,*m_waterScene->texSceneDepth,umath::to_integral(pragma::ShaderWater::WaterBinding::RefractionDepth));
	prosper::util::set_descriptor_set_binding_uniform_buffer(descSetEffects,*m_waterScene->settingsBuffer,umath::to_integral(pragma::ShaderWater::WaterBinding::WaterSettings));

	pragma::ShaderPPFog::Fog fog {};
	fog.flags = pragma::ShaderPPFog::Fog::Flag::Enabled;
	fog.start = 400.f;
	fog.end = 1'600.f;
	fog.color = Color(68,140,200,255).ToVector4();
	fog.type = pragma::ShaderPPFog::Fog::Type::Exponential;
	fog.density = 0.008f;
	prosper::util::BufferCreateInfo fogBufCreateInfo {};
	fogBufCreateInfo.size = sizeof(fog);
	fogBufCreateInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	fogBufCreateInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT;
	m_waterScene->fogBuffer = prosper::util::create_buffer(dev,fogBufCreateInfo,&fog);
	m_waterScene->fogDescSetGroup = prosper::util::create_descriptor_set_group(dev,pragma::ShaderPPFog::DESCRIPTOR_SET_FOG);
	auto &descSetFog = *(*m_waterScene->fogDescSetGroup)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_uniform_buffer(descSetFog,*m_waterScene->fogBuffer,0u);
	prosper::util::set_descriptor_set_binding_uniform_buffer(descSetEffects,*m_waterScene->fogBuffer,umath::to_integral(pragma::ShaderWater::WaterBinding::WaterFog));

	auto pos = refPos;
	auto surfaceNormal = planeNormal;
	auto v = std::numeric_limits<float>::max();
	Vector3 lastPos = {v,v,v};
	Quat lastRot = {v,v,v,v};
	auto waterPlaneDist = std::make_shared<float>(std::numeric_limits<float>::max());
	auto waterNormal = std::make_shared<Vector3>(surfaceNormal);
	Mat4 matReflect;
	m_waterScene->hRender = c_game->AddCallback("Render",FunctionCallback<void>::Create([this]() {
		// This is called before the water render pass
		if(cvDrawWater->GetBool() == false)
			return;
		auto &scene = c_game->GetRenderScene();
		if(scene != c_game->GetScene())
			return;
		auto drawCmd = c_game->GetCurrentDrawCommandBuffer();
		scene->EndRenderPass(drawCmd); // The current render pass needs to be ended so we can blit the scene texture and depth texture

		auto &hdrInfo = scene->GetHDRInfo();
		auto &tex = hdrInfo.hdrRenderTarget->GetTexture();
		//auto &tex = hdrInfo.texture->GetTexture();

		auto &waterScene = *m_waterScene;
		//tex->GetImage()->SetDrawLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		//waterScene.texScene->GetImage()->SetDrawLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);
		//auto &context = tex->GetContext(); // prosper TODO
		//auto &drawCmd = context.GetDrawCmd(); // prosper TODO

		auto &imgWaterScene = *waterScene.texScene->GetImage();
		prosper::util::record_image_barrier(*(*drawCmd),*(*tex->GetImage()),Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),*imgWaterScene,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);

		prosper::util::record_blit_texture(*(*drawCmd),*tex,*imgWaterScene);

		prosper::util::record_image_barrier(*(*drawCmd),*(*tex->GetImage()),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),*imgWaterScene,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);
		//tex->GetImage()->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL); // prosper TODO

		// Depth
		auto &prepass = scene->GetPrepass();
		auto &depthTex = *prepass.textureDepth;

		// We need the depth buffer in the water shader, but the water shader has depth write enabled, so we need to copy
		// the depth buffer before the water is being rendered.
		// 'CopyImage' automatically resolves the image if it's multi-sampled.
		//depthTex->GetImage()->SetDrawLayout(Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL); // prosper TODO
		//waterScene.texSceneDepth->GetImage()->SetDrawLayout(Anvil::ImageLayout::TRANSFER_DST_OPTIMAL); // prosper TODO

		auto &imgWaterSceneDepth = *waterScene.texSceneDepth->GetImage();
		prosper::util::record_image_barrier(*(*drawCmd),*(*depthTex.GetImage()),Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),*imgWaterSceneDepth,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL);

		prosper::util::record_blit_texture(*(*drawCmd),depthTex,*imgWaterSceneDepth);

		prosper::util::record_image_barrier(*(*drawCmd),*(*depthTex.GetImage()),Anvil::ImageLayout::TRANSFER_SRC_OPTIMAL,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		prosper::util::record_image_barrier(*(*drawCmd),*imgWaterSceneDepth,Anvil::ImageLayout::TRANSFER_DST_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

		// TODO: The destination depth image contains red-color components after the copy, why? (Compare debug_prepass and debug_water)

		//waterScene.texScene->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL); // prosper TODO
		//waterScene.texSceneDepth->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL); // prosper TODO
		//depthTex->GetImage()->SetDrawLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // prosper TODO
		scene->BeginRenderPass(drawCmd,prosper::ShaderGraphics::GetRenderPass<pragma::ShaderParticle2DBase>(*c_engine).get()); // Restart the render pass
	}));
	m_waterScene->hPostProcessing = c_game->AddCallback("RenderPostProcessing",FunctionCallback<void,FRender>::Create([this,waterNormal,waterPlaneDist,whShaderPPWater](FRender renderFlags) {
		if(cvDrawWater->GetBool() == false || (renderFlags &FRender::Water) == FRender::None)
			return;
		if(c_game->GetRenderScene() != c_game->GetScene())
			return;
		auto &scene = c_game->GetRenderScene();
		auto &waterScene = *m_waterScene;

		//scene->GetDepthTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL); // prosper TODO
		auto &camGame = *scene->GetCamera();
		auto &n = *waterNormal;
		auto &planeDist = *waterPlaneDist;

		auto bCameraSubmerged = false;
		auto bCameraFullySubmerged = true;
		is_camera_submerged(camGame,n,planeDist,bCameraSubmerged,bCameraFullySubmerged);
		if(bCameraSubmerged == true)
		{
			auto &hdrInfo = scene->GetHDRInfo();

			auto &descSetHdr = *(*hdrInfo.descSetGroupHdr)->get_descriptor_set(0u);
			auto *imgTex = prosper::util::get_descriptor_set_image(descSetHdr,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Texture));
			auto drawCmd = c_game->GetCurrentDrawCommandBuffer();
			if(imgTex != nullptr)
				prosper::util::record_image_barrier(*(*drawCmd),*imgTex,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

			std::function<void(prosper::CommandBuffer&)> fTransitionSampleImgToTransferDst = nullptr;
			hdrInfo.BlitMainDepthBufferToSamplableDepthBuffer(*drawCmd,fTransitionSampleImgToTransferDst);
			if(prosper::util::record_begin_render_pass(*(*drawCmd),*hdrInfo.hdrStagingRenderTarget) == true)
			{
				auto &prepass = scene->GetPrepass();
				//auto &texDepth = prepass.textureDepth->Resolve(); // prosper TODO
				//texDepth->GetImage()->SetDrawLayout(Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL); // prosper TODO

				auto *mat = GetWaterMaterial();
				auto &shaderPPWater = static_cast<pragma::ShaderPPWater&>(*whShaderPPWater.get());
				if(mat != nullptr && mat->IsLoaded() == true && shaderPPWater.BeginDraw(drawCmd) == true && shaderPPWater.BindRefractionMaterial(*mat))
				{
					shaderPPWater.Draw(
						descSetHdr,
						*(*hdrInfo.descSetGroupDepth)->get_descriptor_set(0u),
						*scene->GetCameraDescriptorSetGraphics(),
						c_game->GetGlobalRenderSettingsDescriptorSet(),
						*(*waterScene.fogDescSetGroup)->get_descriptor_set(0u),
						Vector4{n.x,n.y,n.z,planeDist}
					);
					shaderPPWater.EndDraw();
				}
				prosper::util::record_end_render_pass(*(*drawCmd));
			}
			//fTransitionSampleImgToTransferDst(*drawCmd);
			hdrInfo.BlitStagingRenderTargetToMainRenderTarget(*drawCmd);
		}
	}));
	m_waterScene->hRenderScene = c_game->AddCallback("DrawScene",FunctionCallback<
		bool,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>>,std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>>
	>::CreateWithOptionalReturn([this,shader,pos,surfaceNormal,waterNormal,lastPos,lastRot,waterPlaneDist,matReflect](bool *bSkipMainScene,std::reference_wrapper<std::shared_ptr<prosper::PrimaryCommandBuffer>> drawCmd,std::reference_wrapper<std::shared_ptr<prosper::RenderTarget>> rt) mutable -> CallbackReturnType {
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
		auto &camGame = *scene->GetCamera();
		auto &matView = camGame.GetViewMatrix();
		auto &matProj = camGame.GetProjectionMatrix();
		auto &camPos = camGame.GetPos();
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
				auto &rtReflection = sceneReflection->GetHDRInfo().hdrRenderTarget;
				auto &camReflection = *sceneReflection->GetCamera();

				camReflection.SetPos(camPos);
				camReflection.SetForward(camGame.GetForward());
				camReflection.SetUp(camGame.GetUp());
				camReflection.UpdateMatrices();

				camReflection.SetProjectionMatrix(matProj);
				auto matReflView = camReflection.GetViewMatrix();
				matReflView *= matReflect;
				camReflection.SetViewMatrix(matReflView);

				// Reflect camera position (Has to be done AFTER matrices have been updated!)
				auto posReflected = Vector4(camPos.x,camPos.y,camPos.z,1.f);
				posReflected = glm::inverse(camGame.GetViewMatrix()) *matReflView *posReflected;
				camReflection.SetPos({posReflected.x,posReflected.y,posReflected.z});

				auto renderFlags = FRender::World | FRender::Skybox | FRender::Reflection;
				if(reflectionQuality == 0 || reflectionQuality > 1)
					renderFlags |= FRender::Glow;
				if(reflectionQuality > 1)
					renderFlags |= FRender::Particles;
				//rtReflection->GetTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL); // prosper TODO
				//sceneReflection->GetDepthTexture()->GetImage()->SetDrawLayout(Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL); // prosper TODO

				auto &imgReflection = *rtReflection->GetTexture()->GetImage();
				prosper::util::record_image_barrier(*(*drawCmd.get()),*imgReflection,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);

				c_game->SetRenderClipPlane({n.x *planeSign,n.y *planeSign,n.z *planeSign,(planeDist -offset *planeSign) *planeSign});
					c_game->SetRenderScene(sceneReflection);
						c_game->RenderScene(drawCmd,rtReflection,renderFlags);
					c_game->SetRenderScene(nullptr);
				c_game->SetRenderClipPlane({});

				prosper::util::record_image_barrier(*(*drawCmd.get()),*imgReflection,Anvil::ImageLayout::COLOR_ATTACHMENT_OPTIMAL,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL);

				m_reflectionRendered = (bRenderDynamic == true) ? 1 : 2;
			}
		}
		return CallbackReturnType::NoReturnValue;
	}));
}
#pragma optimize("",on)
