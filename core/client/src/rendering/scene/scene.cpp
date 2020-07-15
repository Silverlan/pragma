/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/renderers/base_renderer.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_generic_component.hpp"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <sharedutils/property/util_property.hpp>
#include <sharedutils/property/util_property_color.hpp>
#include <sharedutils/property/util_property_vector.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

Scene::CSMCascadeDescriptor::CSMCascadeDescriptor()
{}

///////////////////////////

ShaderMeshContainer::ShaderMeshContainer(pragma::ShaderTextured3DBase *shader)
	: shader(shader->GetHandle())
{}

///////////////////////////

Scene::CreateInfo::CreateInfo()
	: sampleCount{static_cast<prosper::SampleCountFlags>(c_game->GetMSAASampleCount())}
{}

///////////////////////////

using SceneCount = uint32_t;
static std::array<SceneCount,32> g_sceneUseCount {};
static std::array<Scene*,32> g_scenes {};
std::shared_ptr<Scene> Scene::Create(const CreateInfo &createInfo,Scene *optParent)
{
	SceneIndex sceneIndex;
	if(optParent)
		sceneIndex = optParent->GetSceneIndex();
	else
	{
		// Find unused slot
		auto it = std::find(g_sceneUseCount.begin(),g_sceneUseCount.end(),0);
		if(it == g_sceneUseCount.end())
			return nullptr;
		sceneIndex = (it -g_sceneUseCount.begin());
	}
	++g_sceneUseCount.at(sceneIndex);
	auto scene = std::shared_ptr<Scene>{new Scene{createInfo,sceneIndex},[](Scene *ptr) {
		auto sceneIndex = ptr->GetSceneIndex();
		assert(g_sceneUseCount > 0);
		--g_sceneUseCount.at(sceneIndex);
		g_scenes.at(sceneIndex) = nullptr;

		// Clear all entities from this scene
		std::vector<CBaseEntity*> *ents;
		c_game->GetEntities(&ents);
		for(auto *ent : *ents)
		{
			if(ent == nullptr)
				continue;
			ent->RemoveFromScene(*ptr);
		}
		delete ptr;
	}};
	g_scenes.at(sceneIndex) = scene.get();
	return scene;
}

void Scene::InitializeShadowDescriptorSet()
{
	if(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_SHADOWS.IsValid())
		m_shadowDsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_SHADOWS);
}

Scene *Scene::GetByIndex(SceneIndex sceneIndex)
{
	return (sceneIndex < g_scenes.size()) ? g_scenes.at(sceneIndex) : nullptr;
}

Scene::Scene(const CreateInfo &createInfo,SceneIndex sceneIndex)
	: std::enable_shared_from_this<Scene>(),
	m_sceneIndex{sceneIndex}
{
	for(auto i=decltype(pragma::CShadowCSMComponent::MAX_CASCADE_COUNT){0};i<pragma::CShadowCSMComponent::MAX_CASCADE_COUNT;++i)
		m_csmDescriptors.push_back(std::unique_ptr<CSMCascadeDescriptor>(new CSMCascadeDescriptor()));
	InitializeCameraBuffer();
	InitializeFogBuffer();
	InitializeDescriptorSetLayouts();

	InitializeRenderSettingsBuffer();
	InitializeSwapDescriptorBuffers();
	InitializeShadowDescriptorSet();
}

Scene::~Scene()
{
	ClearWorldEnvironment();
	//c_engine->FlushCommandBuffers(); // We need to make sure all rendering commands have been completed, in case this scene is still in use somewhere // prosper TODO
}

static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
static CVar cvShaderQuality = GetClientConVar("cl_render_shader_quality");
void Scene::InitializeRenderSettingsBuffer()
{
	// Initialize Render Settings
	auto szShadowMap = cvShadowmapSize->GetFloat();
	auto w = GetWidth();
	auto h = GetHeight();

	m_renderSettings.ambientColor = Vector4(1.f,1.f,1.f,1.f);
	m_renderSettings.posCam = Vector3(0.f,0.f,0.f);
	m_renderSettings.flags = umath::to_integral(FRenderSetting::None);
	m_renderSettings.shadowRatioX = 1.f /szShadowMap;
	m_renderSettings.shadowRatioY = 1.f /szShadowMap;
	m_renderSettings.shaderQuality = cvShaderQuality->GetInt();
	m_renderSettings.lightmapIntensity = 1.f;
	m_renderSettings.lightmapSqrt = 0.f;

	if(m_renderer)
		m_renderer->UpdateRenderSettings();

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.size = sizeof(m_renderSettings);
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	m_renderSettingsBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,&m_renderSettings);
	m_renderSettingsBuffer->SetDebugName("render_settings_buf");
	UpdateRenderSettings();
	//
}
void Scene::InitializeCameraBuffer()
{
	// Camera
	m_cameraData.P = umat::identity();
	m_cameraData.V = umat::identity();
	m_cameraData.VP = umat::identity();

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.size = sizeof(m_cameraData);
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	m_cameraBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,&m_cameraData);
	m_cameraBuffer->SetDebugName("camera_buf");
	//

	// View Camera
	m_cameraViewBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,&m_cameraData);
	m_cameraViewBuffer->SetDebugName("camera_view_buf");
	//
}
void Scene::InitializeFogBuffer()
{
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.size = sizeof(m_fogData);
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	m_fogBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,&m_cameraData);
	m_fogBuffer->SetDebugName("fog_buf");
}
pragma::COcclusionCullerComponent *Scene::FindOcclusionCuller()
{
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::COcclusionCullerComponent>>();
	entIt.AttachFilter<EntityIteratorFilterUser>([this](BaseEntity &ent) -> bool {
		return static_cast<CBaseEntity&>(ent).IsInScene(*this);
	});
	auto it = entIt.begin();
	auto *ent = (it != entIt.end()) ? *it : nullptr;
	return ent ? ent->GetComponent<pragma::COcclusionCullerComponent>().get() : nullptr;
}
const std::shared_ptr<prosper::IBuffer> &Scene::GetFogBuffer() const {return m_fogBuffer;}
void Scene::UpdateCameraBuffer(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd,bool bView)
{
	auto &cam = GetActiveCamera();
	if(cam.expired())
		return;
	auto &bufCam = (bView == true) ? GetViewCameraBuffer() : GetCameraBuffer();
	auto &v = cam->GetViewMatrix();
	auto &p = (bView == true) ? pragma::CCameraComponent::CalcProjectionMatrix(c_game->GetViewModelFOV(),cam->GetAspectRatio(),cam->GetNearZ(),cam->GetFarZ()) : cam->GetProjectionMatrix();
	m_cameraData.V = v;
	m_cameraData.P = p;
	m_cameraData.VP = p *v;

	if(bView == false && m_renderer)
		m_renderer->UpdateCameraData(m_cameraData);

	drawCmd->RecordBufferBarrier(
		*bufCam,
		prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::VertexShaderBit | prosper::PipelineStageFlags::GeometryShaderBit,prosper::PipelineStageFlags::TransferBit,
		prosper::AccessFlags::ShaderReadBit,prosper::AccessFlags::TransferWriteBit
	);
	drawCmd->RecordUpdateBuffer(*bufCam,0ull,m_cameraData);
}
void Scene::UpdateBuffers(std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd)
{
	UpdateCameraBuffer(drawCmd,false);
	UpdateCameraBuffer(drawCmd,true);

	// Update Render Buffer
	auto &cam = GetActiveCamera();
	auto camPos = cam.valid() ? cam->GetEntity().GetPosition() : Vector3{};
	m_renderSettings.posCam = camPos;

	drawCmd->RecordUpdateBuffer(*m_renderSettingsBuffer,0ull,m_renderSettings);
	// prosper TODO: Move camPos to camera buffer, and don't update render settings buffer every frame (update when needed instead)

	if(m_renderer && m_renderer->IsRasterizationRenderer())
		static_cast<pragma::rendering::RasterizationRenderer*>(m_renderer.get())->UpdateRendererBuffer(drawCmd);
}
void Scene::InitializeDescriptorSetLayouts()
{
	/*auto &context = c_engine->GetRenderContext();
	m_descSetLayoutCamGraphics = Vulkan::DescriptorSetLayout::Create(context,{
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit}, // Camera
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit}, // Render Settings
		{prosper::DescriptorType::CombinedImageSampler,prosper::ShaderStageFlags::FragmentBit} // SSAO Map
	});
	m_descSetLayoutCamCompute = Vulkan::DescriptorSetLayout::Create(context,{
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::ComputeBit}, // Camera
		{prosper::DescriptorType::UniformBuffer,prosper::ShaderStageFlags::ComputeBit} // Render Settings
	});*/ // prosper TODO
}
void Scene::InitializeSwapDescriptorBuffers()
{
	if(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA.IsValid() == false || pragma::ShaderPPFog::DESCRIPTOR_SET_FOG.IsValid() == false)
		return;
	m_camDescSetGroupGraphics = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA);
	auto &descSetGraphics = *m_camDescSetGroupGraphics->GetDescriptorSet();
	descSetGraphics.SetBindingUniformBuffer(
		*m_cameraBuffer,umath::to_integral(pragma::ShaderTextured3DBase::CameraBinding::Camera)
	);
	descSetGraphics.SetBindingUniformBuffer(
		*m_renderSettingsBuffer,umath::to_integral(pragma::ShaderTextured3DBase::CameraBinding::RenderSettings)
	);

	m_camDescSetGroupCompute = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderForwardPLightCulling::DESCRIPTOR_SET_CAMERA);
	auto &descSetCompute = *m_camDescSetGroupCompute->GetDescriptorSet();
	descSetCompute.SetBindingUniformBuffer(
		*m_cameraBuffer,umath::to_integral(pragma::ShaderForwardPLightCulling::CameraBinding::Camera)
	);
	descSetCompute.SetBindingUniformBuffer(
		*m_renderSettingsBuffer,umath::to_integral(pragma::ShaderForwardPLightCulling::CameraBinding::RenderSettings)
	);

	m_camViewDescSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA);
	auto &descSetViewGraphics = *m_camViewDescSetGroup->GetDescriptorSet();
	descSetViewGraphics.SetBindingUniformBuffer(
		*m_cameraViewBuffer,umath::to_integral(pragma::ShaderTextured3DBase::CameraBinding::Camera)
	);
	descSetViewGraphics.SetBindingUniformBuffer(
		*m_renderSettingsBuffer,umath::to_integral(pragma::ShaderTextured3DBase::CameraBinding::RenderSettings)
	);

	m_fogDescSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderPPFog::DESCRIPTOR_SET_FOG);
	m_fogDescSetGroup->GetDescriptorSet()->SetBindingUniformBuffer(*m_fogBuffer,0u);
}
const std::shared_ptr<prosper::IBuffer> &Scene::GetRenderSettingsBuffer() const {return m_renderSettingsBuffer;}
pragma::RenderSettings &Scene::GetRenderSettings() {return m_renderSettings;}
const pragma::RenderSettings &Scene::GetRenderSettings() const {return const_cast<Scene*>(this)->GetRenderSettings();}
const std::shared_ptr<prosper::IBuffer> &Scene::GetCameraBuffer() const {return m_cameraBuffer;}
const std::shared_ptr<prosper::IBuffer> &Scene::GetViewCameraBuffer() const {return m_cameraViewBuffer;}
const std::shared_ptr<prosper::IDescriptorSetGroup> &Scene::GetCameraDescriptorSetGroup(prosper::PipelineBindPoint bindPoint) const
{
	switch(bindPoint)
	{
		case prosper::PipelineBindPoint::Graphics:
			return m_camDescSetGroupGraphics;
		case prosper::PipelineBindPoint::Compute:
			return m_camDescSetGroupCompute;
	}
	static std::shared_ptr<prosper::IDescriptorSetGroup> nptr = nullptr;
	return nptr;
}
const std::shared_ptr<prosper::IDescriptorSetGroup> &Scene::GetViewCameraDescriptorSetGroup() const {return m_camViewDescSetGroup;}
prosper::IDescriptorSet *Scene::GetCameraDescriptorSetGraphics() const {return m_camDescSetGroupGraphics->GetDescriptorSet();}
prosper::IDescriptorSet *Scene::GetCameraDescriptorSetCompute() const {return m_camDescSetGroupCompute->GetDescriptorSet();}
prosper::IDescriptorSet *Scene::GetViewCameraDescriptorSet() const {return m_camViewDescSetGroup->GetDescriptorSet();}
const std::shared_ptr<prosper::IDescriptorSetGroup> &Scene::GetFogDescriptorSetGroup() const {return m_fogDescSetGroup;}

WorldEnvironment *Scene::GetWorldEnvironment() const {return m_worldEnvironment.get();}
void Scene::SetWorldEnvironment(WorldEnvironment &env)
{
	ClearWorldEnvironment();

	m_worldEnvironment = env.shared_from_this();
	m_envCallbacks.push_back(m_worldEnvironment->GetAmbientColorProperty()->AddCallback([this](std::reference_wrapper<const Vector4> oldColor,std::reference_wrapper<const Vector4> newColor) {
		m_renderSettings.ambientColor = newColor.get();
	}));
	m_envCallbacks.push_back(m_worldEnvironment->GetShaderQualityProperty()->AddCallback([this](std::reference_wrapper<const int32_t> oldVal,std::reference_wrapper<const int32_t> newVal) {
		m_renderSettings.shaderQuality = newVal.get();
	}));
	m_envCallbacks.push_back(m_worldEnvironment->GetShadowResolutionProperty()->AddCallback([this](std::reference_wrapper<const uint32_t> oldVal,std::reference_wrapper<const uint32_t> newVal) {
		Vector2 shadowRatio{
			1.f /static_cast<float>(newVal.get()),
			1.f /static_cast<float>(newVal.get())
		};
		m_renderSettings.shadowRatioX = shadowRatio.x;
		m_renderSettings.shadowRatioY = shadowRatio.y;
	}));
	m_envCallbacks.push_back(m_worldEnvironment->GetUnlitProperty()->AddCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		UpdateRenderSettings();
	}));

	// Fog
	auto &fog = m_worldEnvironment->GetFogSettings();
	m_envCallbacks.push_back(fog.GetColorProperty()->AddCallback([this](std::reference_wrapper<const Color> oldVal,std::reference_wrapper<const Color> newVal) {
		m_fogData.color = newVal.get().ToVector4();
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),color),m_fogData.color);
	}));
	m_envCallbacks.push_back(fog.GetStartProperty()->AddCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
		m_fogData.start = newVal.get();
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),start),m_fogData.start);
	}));
	m_envCallbacks.push_back(fog.GetEndProperty()->AddCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
		m_fogData.end = newVal.get();
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),end),m_fogData.end);
	}));
	m_envCallbacks.push_back(fog.GetMaxDensityProperty()->AddCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
		m_fogData.density = newVal.get();
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),density),m_fogData.density);
	}));
	m_envCallbacks.push_back(fog.GetTypeProperty()->AddCallback([this](std::reference_wrapper<const uint8_t> oldVal,std::reference_wrapper<const uint8_t> newVal) {
		m_fogData.type = static_cast<uint32_t>(newVal.get());
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),type),m_fogData.type);
	}));
	m_envCallbacks.push_back(fog.GetEnabledProperty()->AddCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		m_fogData.flags = static_cast<uint32_t>(newVal.get());
		c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),flags),m_fogData.flags);
	}));
	m_fogData.color = fog.GetColor().ToVector4();
	m_fogData.start = fog.GetStart();
	m_fogData.end = fog.GetEnd();
	m_fogData.density = fog.GetMaxDensity();
	m_fogData.type = umath::to_integral(fog.GetType());
	m_fogData.flags = fog.IsEnabled();
	c_engine->GetRenderContext().ScheduleRecordUpdateBuffer(m_fogBuffer,0ull,m_fogData);
}
void Scene::SetLightMap(pragma::CLightMapComponent &lightMapC)
{
	auto &renderSettings = GetRenderSettings();
	renderSettings.lightmapIntensity = lightMapC.GetLightMapIntensity();
	renderSettings.lightmapSqrt = lightMapC.GetLightMapSqrtFactor();
	m_lightMap = lightMapC.GetHandle<pragma::CLightMapComponent>();
	UpdateRenderSettings();
	UpdateRendererLightMap();
}
void Scene::UpdateRendererLightMap()
{
	if(m_renderer == nullptr || m_renderer->IsRasterizationRenderer() == false || m_lightMap.expired())
		return;
	auto &texLightMap = m_lightMap->GetLightMap();
	if(texLightMap == nullptr)
		return;
	// TODO: Not ideal to have this here; How to handle this in a better way?
	static_cast<pragma::rendering::RasterizationRenderer*>(m_renderer.get())->SetLightMap(texLightMap);
}
void Scene::UpdateRenderSettings()
{
	if(m_worldEnvironment == nullptr)
		return;
	auto &unlitProperty = m_worldEnvironment->GetUnlitProperty();
	auto flags = FRenderSetting::None;
	if(unlitProperty->GetValue() == true)
		flags |= FRenderSetting::Unlit;
	m_renderSettings.flags = umath::to_integral(flags);
	if(m_renderer)
		m_renderer->UpdateRenderSettings();
}
void Scene::ClearWorldEnvironment()
{
	for(auto &hCb : m_envCallbacks)
	{
		if(hCb.IsValid())
			hCb.Remove();
	}
	m_envCallbacks.clear();
	if(m_cbFogCallback.IsValid() == true)
		m_cbFogCallback.Remove();
	m_worldEnvironment = nullptr;
}

/*Vulkan::Texture &Scene::ResolveRenderTexture(Vulkan::CommandBufferObject *cmdBuffer) {return const_cast<Vulkan::Texture&>(m_hdrInfo.texture->Resolve(cmdBuffer));}
Vulkan::Texture &Scene::ResolveDepthTexture(Vulkan::CommandBufferObject *cmdBuffer) {return const_cast<Vulkan::Texture&>(m_hdrInfo.prepass.textureDepth->Resolve(cmdBuffer));}
Vulkan::Texture &Scene::ResolveBloomTexture(Vulkan::CommandBufferObject *cmdBuffer) {return const_cast<Vulkan::Texture&>(m_hdrInfo.textureBloom->Resolve(cmdBuffer));}*/ // prosper TODO

void Scene::Resize(uint32_t width,uint32_t height,bool reload)
{
	if(m_renderer == nullptr)
		return;
	if(reload == false && width == GetWidth() && height == GetHeight())
		return;
	ReloadRenderTarget(width,height);
}
void Scene::LinkWorldEnvironment(Scene &other)
{
	// T
	auto &worldEnv = *GetWorldEnvironment();
	auto &worldEnvOther = *other.GetWorldEnvironment();
	worldEnv.GetAmbientColorProperty()->Link(*worldEnvOther.GetAmbientColorProperty());
	worldEnv.GetShaderQualityProperty()->Link(*worldEnvOther.GetShaderQualityProperty());
	worldEnv.GetShadowResolutionProperty()->Link(*worldEnvOther.GetShadowResolutionProperty());
	worldEnv.GetUnlitProperty()->Link(*worldEnvOther.GetUnlitProperty());

	auto &fogSettings = worldEnv.GetFogSettings();
	auto &fogSettingsOther = worldEnvOther.GetFogSettings();
	fogSettings.GetColorProperty()->Link(*fogSettingsOther.GetColorProperty());
	fogSettings.GetEnabledProperty()->Link(*fogSettingsOther.GetEnabledProperty());
	fogSettings.GetEndProperty()->Link(*fogSettingsOther.GetEndProperty());
	fogSettings.GetMaxDensityProperty()->Link(*fogSettingsOther.GetMaxDensityProperty());
	fogSettings.GetStartProperty()->Link(*fogSettingsOther.GetStartProperty());
	fogSettings.GetTypeProperty()->Link(*fogSettingsOther.GetTypeProperty());
}

void Scene::SetRenderer(const std::shared_ptr<pragma::rendering::BaseRenderer> &renderer)
{
	m_renderer = renderer;
	UpdateRenderSettings();
	UpdateRendererLightMap();
}
pragma::rendering::BaseRenderer *Scene::GetRenderer() {return m_renderer.get();}

Scene::DebugMode Scene::GetDebugMode() const {return m_debugMode;}
void Scene::SetDebugMode(Scene::DebugMode debugMode) {m_debugMode = debugMode;}

void Scene::SetParticleSystemColorFactor(const Vector4 &colorFactor) {m_particleSystemColorFactor = colorFactor;}
const Vector4 &Scene::GetParticleSystemColorFactor() const {return m_particleSystemColorFactor;}

bool Scene::IsValid() const {return m_bValid;}

Scene::SceneIndex Scene::GetSceneIndex() const {return m_sceneIndex;}

uint32_t Scene::GetWidth() const {return m_renderer ? m_renderer->GetWidth() : 0;}
uint32_t Scene::GetHeight() const {return m_renderer ? m_renderer->GetHeight() : 0;}

//const Vulkan::DescriptorSet &Scene::GetBloomGlowDescriptorSet() const {return m_descSetBloomGlow;} // prosper TODO

void Scene::ReloadRenderTarget(uint32_t width,uint32_t height)
{
	m_bValid = false;

	if(m_renderer == nullptr || m_renderer->ReloadRenderTarget(width,height) == false)
		return;

	m_bValid = true;
}

const util::WeakHandle<pragma::CCameraComponent> &Scene::GetActiveCamera() const {return const_cast<Scene*>(this)->GetActiveCamera();}
util::WeakHandle<pragma::CCameraComponent> &Scene::GetActiveCamera() {return m_camera;}
void Scene::SetActiveCamera(pragma::CCameraComponent &cam)
{
	m_camera = cam.GetHandle<pragma::CCameraComponent>();
	m_renderSettings.nearZ = cam.GetNearZ();
	m_renderSettings.farZ = cam.GetFarZ();
}
void Scene::SetActiveCamera() {m_camera = {};}
