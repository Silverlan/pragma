#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/opengl/renderhierarchy.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_hdr.hpp"
#include "pragma/rendering/shaders/image/c_shader_additive.h"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_fog.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/lighting/shadows/c_shadowmapcasc.h"
#include "pragma/rendering/renderers/base_renderer.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_generic_component.hpp"
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


extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

Scene::CSMCascadeDescriptor::CSMCascadeDescriptor()
{}

///////////////////////////

ShaderMeshContainer::ShaderMeshContainer(pragma::ShaderTextured3D *shader)
	: shader(shader->GetHandle())
{}

///////////////////////////

void Scene::LightListInfo::AddLightSource(pragma::CLightComponent &lightSource)
{
	RemoveLightSource(lightSource);
	if(lightSources.size() == lightSources.capacity())
		lightSources.reserve(lightSources.size() +20u);
	lightSources.push_back(lightSource.GetHandle<pragma::CLightComponent>());
	lightSourceLookupTable.insert(&lightSource);
}
void Scene::LightListInfo::RemoveLightSource(pragma::CLightComponent &lightSource)
{
	auto it = std::find_if(lightSources.begin(),lightSources.end(),[&lightSource](const util::WeakHandle<pragma::CLightComponent> &hLight) {
		return hLight.get() == &lightSource;
	});
	if(it != lightSources.end())
		lightSources.erase(it);

	auto itLookup = lightSourceLookupTable.find(&lightSource);
	if(itLookup != lightSourceLookupTable.end())
		lightSourceLookupTable.erase(itLookup);
}
void Scene::EntityListInfo::AddEntity(CBaseEntity &ent)
{
	RemoveEntity(ent);
	if(entities.size() == entities.capacity())
		entities.reserve(entities.size() +20u);
	entities.push_back(ent.GetHandle());
	entityLookupTable.insert(&ent);
}
void Scene::EntityListInfo::RemoveEntity(CBaseEntity &ent)
{
	auto it = std::find_if(entities.begin(),entities.end(),[&ent](const EntityHandle &hEnt) {
		return hEnt.get() == &ent;
	});
	if(it != entities.end())
		entities.erase(it);

	auto itLookup = entityLookupTable.find(&ent);
	if(itLookup != entityLookupTable.end())
		entityLookupTable.erase(itLookup);
}

///////////////////////////

Scene::CreateInfo::CreateInfo(uint32_t width,uint32_t height)
	: width{width},height{height},sampleCount{static_cast<Anvil::SampleCountFlagBits>(c_game->GetMSAASampleCount())}
{}

///////////////////////////

decltype(Scene::s_scenes) Scene::s_scenes;
std::shared_ptr<Scene> Scene::Create(const CreateInfo &createInfo)
{
	return std::shared_ptr<Scene>(new Scene{createInfo});
}

Scene::Scene(const CreateInfo &createInfo)
	: std::enable_shared_from_this<Scene>(),
	m_width(createInfo.width),m_height(createInfo.height),
	m_lightSources(std::make_shared<LightListInfo>()),
	m_entityList(std::make_shared<EntityListInfo>())
{
	for(auto i=decltype(ShadowMapCasc::MAX_CASCADE_COUNT){0};i<ShadowMapCasc::MAX_CASCADE_COUNT;++i)
		m_csmDescriptors.push_back(std::unique_ptr<CSMCascadeDescriptor>(new CSMCascadeDescriptor()));
	InitializeCameraBuffer();
	InitializeFogBuffer();
	InitializeDescriptorSetLayouts();
	s_scenes.push_back(this);
}

Scene::~Scene()
{
	ClearWorldEnvironment();
	//c_engine->FlushCommandBuffers(); // We need to make sure all rendering commands have been completed, in case this scene is still in use somewhere // prosper TODO
	auto it = std::find(s_scenes.begin(),s_scenes.end(),this);
	if(it != s_scenes.end())
		s_scenes.erase(it);
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
	m_renderSettings.viewportW = w;
	m_renderSettings.viewportH = h;
	m_renderSettings.shaderQuality = cvShaderQuality->GetInt();

	if(m_renderer)
		m_renderer->UpdateRenderSettings(m_renderSettings);

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.size = sizeof(m_renderSettings);
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	m_renderSettingsBuffer = prosper::util::create_buffer(c_engine->GetDevice(),createInfo,&m_renderSettings);
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
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.size = sizeof(m_cameraData);
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	m_cameraBuffer = prosper::util::create_buffer(c_engine->GetDevice(),createInfo,&m_cameraData);
	m_cameraBuffer->SetDebugName("camera_buf");
	//

	// View Camera
	m_cameraViewBuffer = prosper::util::create_buffer(c_engine->GetDevice(),createInfo,&m_cameraData);
	m_cameraViewBuffer->SetDebugName("camera_view_buf");
	//
}
void Scene::InitializeFogBuffer()
{
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	createInfo.size = sizeof(m_fogData);
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	m_fogBuffer = prosper::util::create_buffer(c_engine->GetDevice(),createInfo,&m_cameraData);
	m_fogBuffer->SetDebugName("fog_buf");
}
const std::shared_ptr<prosper::Buffer> &Scene::GetFogBuffer() const {return m_fogBuffer;}
void Scene::UpdateCameraBuffer(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,bool bView)
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

	prosper::util::record_update_buffer(**drawCmd,*bufCam,0ull,m_cameraData);
}
void Scene::UpdateBuffers(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	UpdateCameraBuffer(drawCmd,false);
	UpdateCameraBuffer(drawCmd,true);

	// Update Render Buffer
	auto &cam = GetActiveCamera();
	auto camPos = cam.valid() ? cam->GetEntity().GetPosition() : Vector3{};
	m_renderSettings.posCam = camPos;

	prosper::util::record_update_buffer(**drawCmd,*m_renderSettingsBuffer,0ull,m_renderSettings);
	// prosper TODO: Move camPos to camera buffer, and don't update render settings buffer every frame (update when needed instead)
}
void Scene::InitializeDescriptorSetLayouts()
{
	/*auto &context = c_engine->GetRenderContext();
	m_descSetLayoutCamGraphics = Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Camera
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT}, // Render Settings
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // SSAO Map
	});
	m_descSetLayoutCamCompute = Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT}, // Camera
		{Anvil::DescriptorType::UNIFORM_BUFFER,Anvil::ShaderStageFlagBits::COMPUTE_BIT} // Render Settings
	});*/ // prosper TODO
}
void Scene::InitializeSwapDescriptorBuffers()
{
	if(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA.IsValid() == false || pragma::ShaderPPFog::DESCRIPTOR_SET_FOG.IsValid() == false)
		return;
	auto &dev = c_engine->GetDevice();
	m_camDescSetGroupGraphics = prosper::util::create_descriptor_set_group(dev,pragma::ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA);
	auto &descSetGraphics = *(*m_camDescSetGroupGraphics)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		descSetGraphics,*m_cameraBuffer,umath::to_integral(pragma::ShaderTextured3DBase::CameraBinding::Camera)
	);
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		descSetGraphics,*m_renderSettingsBuffer,umath::to_integral(pragma::ShaderTextured3DBase::CameraBinding::RenderSettings)
	);

	m_camDescSetGroupCompute = prosper::util::create_descriptor_set_group(dev,pragma::ShaderForwardPLightCulling::DESCRIPTOR_SET_CAMERA);
	auto &descSetCompute = *(*m_camDescSetGroupCompute)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		descSetCompute,*m_cameraBuffer,umath::to_integral(pragma::ShaderForwardPLightCulling::CameraBinding::Camera)
	);
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		descSetCompute,*m_renderSettingsBuffer,umath::to_integral(pragma::ShaderForwardPLightCulling::CameraBinding::RenderSettings)
	);

	m_camViewDescSetGroup = prosper::util::create_descriptor_set_group(dev,pragma::ShaderTextured3DBase::DESCRIPTOR_SET_CAMERA);
	auto &descSetViewGraphics = *(*m_camViewDescSetGroup)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		descSetViewGraphics,*m_cameraViewBuffer,umath::to_integral(pragma::ShaderTextured3DBase::CameraBinding::Camera)
	);
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		descSetViewGraphics,*m_renderSettingsBuffer,umath::to_integral(pragma::ShaderTextured3DBase::CameraBinding::RenderSettings)
	);

	m_fogDescSetGroup = prosper::util::create_descriptor_set_group(dev,pragma::ShaderPPFog::DESCRIPTOR_SET_FOG);
	prosper::util::set_descriptor_set_binding_uniform_buffer(*(*m_fogDescSetGroup)->get_descriptor_set(0u),*m_fogBuffer,0u);
}
const std::shared_ptr<prosper::Buffer> &Scene::GetRenderSettingsBuffer() const {return m_renderSettingsBuffer;}
pragma::RenderSettings &Scene::GetRenderSettings() {return m_renderSettings;}
const pragma::RenderSettings &Scene::GetRenderSettings() const {return const_cast<Scene*>(this)->GetRenderSettings();}
const std::shared_ptr<prosper::Buffer> &Scene::GetCameraBuffer() const {return m_cameraBuffer;}
const std::shared_ptr<prosper::Buffer> &Scene::GetViewCameraBuffer() const {return m_cameraViewBuffer;}
const std::shared_ptr<prosper::DescriptorSetGroup> &Scene::GetCameraDescriptorSetGroup(vk::PipelineBindPoint bindPoint) const
{
	switch(bindPoint)
	{
		case vk::PipelineBindPoint::eGraphics:
			return m_camDescSetGroupGraphics;
		case vk::PipelineBindPoint::eCompute:
			return m_camDescSetGroupCompute;
	}
	static std::shared_ptr<prosper::DescriptorSetGroup> nptr = nullptr;
	return nptr;
}
const std::shared_ptr<prosper::DescriptorSetGroup> &Scene::GetViewCameraDescriptorSetGroup() const {return m_camViewDescSetGroup;}
Anvil::DescriptorSet *Scene::GetCameraDescriptorSetGraphics() const {return (*m_camDescSetGroupGraphics)->get_descriptor_set(0u);}
Anvil::DescriptorSet *Scene::GetCameraDescriptorSetCompute() const {return (*m_camDescSetGroupCompute)->get_descriptor_set(0u);}
Anvil::DescriptorSet *Scene::GetViewCameraDescriptorSet() const {return (*m_camViewDescSetGroup)->get_descriptor_set(0u);}
const std::shared_ptr<prosper::DescriptorSetGroup> &Scene::GetFogDescriptorSetGroup() const {return m_fogDescSetGroup;}

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
		c_engine->ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),color),m_fogData.color);
	}));
	m_envCallbacks.push_back(fog.GetStartProperty()->AddCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
		m_fogData.start = newVal.get();
		c_engine->ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),start),m_fogData.start);
	}));
	m_envCallbacks.push_back(fog.GetEndProperty()->AddCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
		m_fogData.end = newVal.get();
		c_engine->ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),end),m_fogData.end);
	}));
	m_envCallbacks.push_back(fog.GetMaxDensityProperty()->AddCallback([this](std::reference_wrapper<const float> oldVal,std::reference_wrapper<const float> newVal) {
		m_fogData.density = newVal.get();
		c_engine->ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),density),m_fogData.density);
	}));
	m_envCallbacks.push_back(fog.GetTypeProperty()->AddCallback([this](std::reference_wrapper<const uint8_t> oldVal,std::reference_wrapper<const uint8_t> newVal) {
		m_fogData.type = static_cast<uint32_t>(newVal.get());
		c_engine->ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),type),m_fogData.type);
	}));
	m_envCallbacks.push_back(fog.GetEnabledProperty()->AddCallback([this](std::reference_wrapper<const bool> oldVal,std::reference_wrapper<const bool> newVal) {
		m_fogData.flags = static_cast<uint32_t>(newVal.get());
		c_engine->ScheduleRecordUpdateBuffer(m_fogBuffer,offsetof(decltype(m_fogData),flags),m_fogData.flags);
	}));
	m_fogData.color = fog.GetColor().ToVector4();
	m_fogData.start = fog.GetStart();
	m_fogData.end = fog.GetEnd();
	m_fogData.density = fog.GetMaxDensity();
	m_fogData.type = umath::to_integral(fog.GetType());
	m_fogData.flags = fog.IsEnabled();
	c_engine->ScheduleRecordUpdateBuffer(m_fogBuffer,0ull,m_fogData);
}
void Scene::SetLights(const std::vector<pragma::CLightComponent*> &lights)
{
	auto &info = *m_lightSources;
	info.lightSourceLookupTable.clear();
	info.lightSources.clear();

	info.lightSourceLookupTable.reserve(lights.size());
	info.lightSources.reserve(lights.size());
	for(auto *l : lights)
	{
		info.lightSources.push_back(l->GetHandle<pragma::CLightComponent>());
		info.lightSourceLookupTable.insert(l);
	}
}
void Scene::SetLights(const std::shared_ptr<LightListInfo> &lights)
{
	if(lights == nullptr)
	{
		m_lightSources = std::make_shared<LightListInfo>();
		SetLights(std::vector<pragma::CLightComponent*>{});
		return;
	}
	m_lightSources = lights;
}
void Scene::AddLight(pragma::CLightComponent *light) {m_lightSources->AddLightSource(*light);}
void Scene::RemoveLight(pragma::CLightComponent *light) {m_lightSources->RemoveLightSource(*light);}
const std::shared_ptr<Scene::LightListInfo> &Scene::GetLightSourceListInfo() const {return m_lightSources;}
const std::vector<util::WeakHandle<pragma::CLightComponent>> &Scene::GetLightSources() const {return const_cast<Scene&>(*this).GetLightSources();}
std::vector<util::WeakHandle<pragma::CLightComponent>> &Scene::GetLightSources() {return m_lightSources->lightSources;}
bool Scene::HasLightSource(pragma::CLightComponent &lightSource) const
{
	auto it = m_lightSources->lightSourceLookupTable.find(&lightSource);
	return it != m_lightSources->lightSourceLookupTable.end();
}
void Scene::UpdateRenderSettings()
{
	if(m_worldEnvironment == nullptr)
		return;
	auto &unlitProperty = m_worldEnvironment->GetUnlitProperty();
	auto flags = FRenderSetting::None;
	if(unlitProperty->GetValue() == true)
		flags |= FRenderSetting::Unlit;\
	m_renderSettings.flags = umath::to_integral(flags);
	if(m_renderer)
		m_renderer->UpdateRenderSettings(m_renderSettings);
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

void Scene::Resize(uint32_t width,uint32_t height)
{
	m_width = width;
	m_height = height;
	ReloadRenderTarget();
}
void Scene::LinkEntities(Scene &other) {m_entityList = other.m_entityList;}
void Scene::LinkLightSources(Scene &other) {m_lightSources = other.m_lightSources;}
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
	InitializeRenderSettingsBuffer();
	InitializeSwapDescriptorBuffers();
	Resize(m_width,m_height);
}
pragma::rendering::BaseRenderer *Scene::GetRenderer() {return m_renderer.get();}

void Scene::SetEntities(const std::vector<CBaseEntity*> &ents)
{
	auto &info = *m_entityList;
	info.entityLookupTable.clear();
	info.entities.clear();

	info.entityLookupTable.reserve(ents.size());
	info.entities.reserve(ents.size());
	for(auto *ent : ents)
	{
		info.entities.push_back(ent->GetHandle());
		info.entityLookupTable.insert(ent);
	}
}
void Scene::SetEntities(const std::shared_ptr<EntityListInfo> &ents)
{
	if(ents == nullptr)
	{
		m_entityList = std::make_shared<EntityListInfo>();
		SetEntities(std::vector<CBaseEntity*>{});
		return;
	}
	m_entityList = ents;
}
void Scene::AddEntity(CBaseEntity &ent)
{
	m_entityList->AddEntity(ent);
	std::weak_ptr<Scene> wpScene = shared_from_this();
	auto cb = FunctionCallback<void>::Create(nullptr);
	static_cast<Callback<void>*>(cb.get())->SetFunction([wpScene,&ent,cb]() mutable {
		if(wpScene.expired())
		{
			if(cb.IsValid())
				cb.Remove();
			return;
		}
		wpScene.lock()->RemoveEntity(ent);
	});
	ent.CallOnRemove(cb);

	if(m_renderer)
		m_renderer->OnEntityAddedToScene(ent);
}
void Scene::RemoveEntity(CBaseEntity &ent) {m_entityList->RemoveEntity(ent);}
const std::shared_ptr<Scene::EntityListInfo> &Scene::GetEntityListInfo() const {return m_entityList;}
const std::vector<EntityHandle> &Scene::GetEntities() const {return const_cast<Scene&>(*this).GetEntities();}
std::vector<EntityHandle> &Scene::GetEntities(){return m_entityList->entities;}
bool Scene::HasEntity(CBaseEntity &ent) const
{
	auto it = m_entityList->entityLookupTable.find(&ent);
	return it != m_entityList->entityLookupTable.end();
}

bool Scene::IsValid() const {return m_bValid;}

uint32_t Scene::GetWidth() const {return m_width;}
uint32_t Scene::GetHeight() const {return m_height;}

//const Vulkan::DescriptorSet &Scene::GetBloomGlowDescriptorSet() const {return m_descSetBloomGlow;} // prosper TODO

void Scene::ReloadRenderTarget()
{
	m_bValid = false;

	if(m_renderer == nullptr || m_renderer->ReloadRenderTarget() == false)
		return;

	m_bValid = true;
}

void Scene::InitializeRenderTarget() {}

const util::WeakHandle<pragma::CCameraComponent> &Scene::GetActiveCamera() const {return const_cast<Scene*>(this)->GetActiveCamera();}
util::WeakHandle<pragma::CCameraComponent> &Scene::GetActiveCamera() {return m_camera;}
void Scene::SetActiveCamera(pragma::CCameraComponent &cam)
{
	m_camera = cam.GetHandle<pragma::CCameraComponent>();
	m_renderSettings.nearZ = cam.GetNearZ();
	m_renderSettings.farZ = cam.GetFarZ();
}
void Scene::SetActiveCamera() {m_camera = {};}
