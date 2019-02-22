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
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_generic_component.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_brute_force.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_bsp.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_chc.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_inert.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_octtree.hpp"
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



extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;
Scene::CSMCascadeDescriptor::CSMCascadeDescriptor()
	//: descBuffer(nullptr) // prosper TODO
{
	//static auto hShaderShadow = c_engine->GetShader("shadowcsmstatic");
	/*if(hShaderShadow.IsValid())
	{
		auto &shaderShadow = static_cast<Shader::ShadowCSMStatic&>(*hShaderShadow.get());
		shaderShadow.GenerateSwapDescriptorBuffer(umath::to_integral(Shader::ShadowCSMStatic::DescSet::DepthMatrix),Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT,sizeof(Mat4),descBuffer);
	}*/ // prosper TODO
}

///////////////////////////

REGISTER_CONVAR_CALLBACK_CL(cl_render_ssao,[](NetworkState*,ConVar*,bool,bool val) {
	if(c_game == nullptr)
		return;
	auto &scene = c_game->GetScene();
	if(scene == nullptr)
		return;
	scene->SetSSAOEnabled(val);
});

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

Scene::CreateInfo::CreateInfo(uint32_t width,uint32_t height,float fov,float fovView,float nearZ,float farZ)
	: width{width},height{height},fov{fov},fovView{fovView},nearZ{nearZ},farZ{farZ},sampleCount{static_cast<Anvil::SampleCountFlagBits>(c_game->GetMSAASampleCount())}
{}

///////////////////////////

decltype(Scene::s_scenes) Scene::s_scenes;
std::shared_ptr<Scene> Scene::Create(const CreateInfo &createInfo)
{
	return std::shared_ptr<Scene>(new Scene{createInfo});
}

Scene::Scene(const CreateInfo &createInfo)
	: std::enable_shared_from_this<Scene>(),
	camera(Camera::Create(createInfo.fov,createInfo.fovView,static_cast<float>(createInfo.width) /static_cast<float>(createInfo.height),createInfo.nearZ,createInfo.farZ)),
	m_width(createInfo.width),m_height(createInfo.height),m_hdrInfo(),m_glowInfo(),//m_renderTarget(nullptr),m_descSetScreen(nullptr), // prosper TODO
	m_bDepthResolved(false),m_bBloomResolved(false),m_bRenderResolved(false),
	m_lightSources(std::make_shared<LightListInfo>()),
	m_entityList(std::make_shared<EntityListInfo>()),
	m_sampleCount{createInfo.sampleCount}
{
	for(auto i=decltype(ShadowMapCasc::MAX_CASCADE_COUNT){0};i<ShadowMapCasc::MAX_CASCADE_COUNT;++i)
		m_csmDescriptors.push_back(std::unique_ptr<CSMCascadeDescriptor>(new CSMCascadeDescriptor()));
	InitializeLightDescriptorSets();
	InitializeRenderSettingsBuffer();
	InitializeCameraBuffer();
	InitializeFogBuffer();
	InitializeDescriptorSetLayouts();
	InitializeSwapDescriptorBuffers();
	Resize(createInfo.width,createInfo.height);
	s_scenes.push_back(this);

	m_whShaderWireframe = c_engine->GetShader("wireframe");

	ReloadOcclusionCullingHandler();
	m_occlusionOctree = std::make_shared<OcclusionOctree<CBaseEntity*>>(256.f,1'073'741'824.f,4096.f,[](const CBaseEntity *ent,Vector3 &min,Vector3 &max) {
		auto pRenderComponent = ent->GetRenderComponent();
		auto pTrComponent = ent->GetTransformComponent();
		if(pRenderComponent.valid())
			pRenderComponent->GetRenderBounds(&min,&max);
		else
		{
			min = {};
			max = {};
		}
		if(pTrComponent.expired())
			return;
		auto &pos = pTrComponent->GetPosition();
		min += pos;
		max += pos;
	});
	m_occlusionOctree->Initialize();
	m_occlusionOctree->SetSingleReferenceMode(true);
	m_occlusionOctree->SetToStringCallback([](CBaseEntity *ent) -> std::string {
		return ent->GetClass() +" " +std::to_string(ent->GetIndex());
	});
}

Scene::~Scene()
{
	ClearWorldEnvironment();
	//c_engine->FlushCommandBuffers(); // We need to make sure all rendering commands have been completed, in case this scene is still in use somewhere // prosper TODO
	auto it = std::find(s_scenes.begin(),s_scenes.end(),this);
	if(it != s_scenes.end())
		s_scenes.erase(it);
}

bool Scene::IsSSAOEnabled() const {return m_bSSAOEnabled;}
void Scene::SetSSAOEnabled(bool b)
{
	m_bSSAOEnabled = b;
	UpdateRenderSettings();
	ReloadRenderTarget();
	/*m_hdrInfo.prepass.SetUseExtendedPrepass(b);
	if(b == true)
	{
		auto &context = c_engine->GetRenderContext();
		m_hdrInfo.ssaoInfo.Initialize(context,GetWidth(),GetHeight(),static_cast<Anvil::SampleCountFlagBits>(c_game->GetMSAASampleCount()),m_hdrInfo.prepass.texturePositions,m_hdrInfo.prepass.textureNormals,m_hdrInfo.prepass.textureDepth);
	}
	else
		m_hdrInfo.ssaoInfo.Clear();
	UpdateRenderSettings();*/
}

void Scene::BeginDraw()
{
	m_bDepthResolved = false;
	m_bBloomResolved = false;
	m_bRenderResolved = false;
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
	m_renderSettings.nearZ = GetZNear();
	m_renderSettings.farZ = GetZFar();
	m_renderSettings.viewportW = w;
	m_renderSettings.viewportH = h;
	m_renderSettings.numberOfTilesX = pragma::rendering::ForwardPlusInstance::CalcWorkGroupCount(w,h).first;
	m_renderSettings.shaderQuality = cvShaderQuality->GetInt();

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
void Scene::SetShaderOverride(const std::string &srcShaderId,const std::string &shaderOverrideId)
{
	auto hSrcShader = c_engine->GetShader(srcShaderId);
	if(hSrcShader.get()->GetBaseTypeHashCode() != pragma::ShaderTextured3DBase::HASH_TYPE)
		return;
	auto *srcShader = dynamic_cast<pragma::ShaderTextured3D*>(hSrcShader.get());
	if(srcShader == nullptr)
		return;
	auto hDstShader = c_engine->GetShader(shaderOverrideId);
	auto dstShader = dynamic_cast<pragma::ShaderTextured3D*>(hDstShader.get());
	if(dstShader == nullptr)
		return;
	m_shaderOverrides[typeid(*srcShader).hash_code()] = dstShader->GetHandle();
}
pragma::ShaderTextured3D *Scene::GetShaderOverride(pragma::ShaderTextured3D *srcShader)
{
	if(srcShader == nullptr)
		return nullptr;
	auto it = m_shaderOverrides.find(typeid(*srcShader).hash_code());
	if(it == m_shaderOverrides.end())
		return srcShader;
	return static_cast<pragma::ShaderTextured3D*>(it->second.get());
}
void Scene::ClearShaderOverride(const std::string &srcShaderId)
{
	auto hSrcShader = c_engine->GetShader(srcShaderId);
	auto *srcShader = dynamic_cast<pragma::ShaderTextured3D*>(hSrcShader.get());
	if(srcShader == nullptr)
		return;
	auto it = m_shaderOverrides.find(typeid(*srcShader).hash_code());
	if(it == m_shaderOverrides.end())
		return;
	m_shaderOverrides.erase(it);
}
void Scene::UpdateCameraBuffer(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,bool bView)
{
	auto &cam = *GetCamera();
	auto &bufCam = (bView == true) ? GetViewCameraBuffer() : GetCameraBuffer();
	auto &v = cam.GetViewMatrix();
	auto &p = (bView == true) ? cam.GetViewProjectionMatrix() : cam.GetProjectionMatrix();
	m_cameraData.V = v;
	m_cameraData.P = p;
	m_cameraData.VP = p *v;

	if(bView == false)
		UpdateFrustumPlanes();

	prosper::util::record_update_buffer(**drawCmd,*bufCam,0ull,m_cameraData);
}
void Scene::UpdateBuffers(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	auto &cam = *GetCamera();
	UpdateCameraBuffer(drawCmd,false);
	UpdateCameraBuffer(drawCmd,true);

	// Update Render Buffer
	auto &pos = cam.GetPos();
	m_renderSettings.posCam = pos;

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
void Scene::UpdateRenderSettings()
{
	if(m_worldEnvironment == nullptr)
		return;
	auto &unlitProperty = m_worldEnvironment->GetUnlitProperty();
	auto flags = FRenderSetting::None;
	if(unlitProperty->GetValue() == true)
		flags |= FRenderSetting::Unlit;
	if(IsSSAOEnabled() == true)
		flags |= FRenderSetting::SSAOEnabled;
	m_renderSettings.flags = umath::to_integral(flags);
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
void Scene::LinkEntities(Scene &other) {m_entityList = other.m_entityList;}
void Scene::LinkLightSources(Scene &other) {m_lightSources = other.m_lightSources;}
const std::shared_ptr<Scene::LightListInfo> &Scene::GetLightSourceListInfo() const {return m_lightSources;}
const std::vector<util::WeakHandle<pragma::CLightComponent>> &Scene::GetLightSources() const {return const_cast<Scene&>(*this).GetLightSources();}
std::vector<util::WeakHandle<pragma::CLightComponent>> &Scene::GetLightSources() {return m_lightSources->lightSources;}
bool Scene::HasLightSource(pragma::CLightComponent &lightSource) const
{
	auto it = m_lightSources->lightSourceLookupTable.find(&lightSource);
	return it != m_lightSources->lightSourceLookupTable.end();
}

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

	// Add entity to octree
	auto cbRenderMode = FunctionCallback<void,std::reference_wrapper<const RenderMode>,std::reference_wrapper<const RenderMode>>::Create(nullptr);
	auto callbacks = std::make_shared<std::vector<CallbackHandle>>();
	callbacks->push_back(cbRenderMode); // Render mode callback has to be removed in the EVENT_ON_REMOVE event, otherwise the callback will cause the entity to be re-added to the tree AFTER it just has been removed
	auto fInsertOctreeObject = [this,callbacks](CBaseEntity *ent) {
		m_occlusionOctree->InsertObject(ent);
		auto pTrComponent = ent->GetTransformComponent();
		if(pTrComponent.valid())
		{
			callbacks->push_back(pTrComponent->GetPosProperty()->AddCallback([this,ent](std::reference_wrapper<const Vector3> oldPos,std::reference_wrapper<const Vector3> pos) {
				m_occlusionOctree->UpdateObject(ent);
			}));
		}
		auto pGenericComponent = ent->GetComponent<pragma::CGenericComponent>();
		if(pGenericComponent.valid())
		{
			callbacks->push_back(pGenericComponent->BindEventUnhandled(pragma::CModelComponent::EVENT_ON_MODEL_CHANGED,[this,pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				auto *ent = static_cast<CBaseEntity*>(&pGenericComponent->GetEntity());
				m_occlusionOctree->UpdateObject(ent);
			}));
			callbacks->push_back(pGenericComponent->BindEventUnhandled(pragma::CRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED,[this,pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				auto *ent = static_cast<CBaseEntity*>(&pGenericComponent->GetEntity());
				m_occlusionOctree->UpdateObject(ent);
			}));
			callbacks->push_back(pGenericComponent->BindEventUnhandled(BaseEntity::EVENT_ON_REMOVE,[this,callbacks,pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) mutable {
				auto *ent = static_cast<CBaseEntity*>(&pGenericComponent->GetEntity());
				m_occlusionOctree->RemoveObject(ent);
				m_occlusionOctree->IterateObjects([](const OcclusionOctree<CBaseEntity*>::Node &node) -> bool {

					return true;
				},[&](const CBaseEntity *entOther) {
					if(entOther == ent)
						throw std::runtime_error("!!");
				});
				for(auto &cb : *callbacks)
				{
					if(cb.IsValid() == false)
						continue;
					cb.Remove();
				}
			}));
		}
	};
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.expired())
		return;
	static_cast<Callback<void,std::reference_wrapper<const RenderMode>,std::reference_wrapper<const RenderMode>>*>(cbRenderMode.get())->SetFunction([this,&ent,fInsertOctreeObject,callbacks,wpScene,cbRenderMode](std::reference_wrapper<const RenderMode> old,std::reference_wrapper<const RenderMode> newMode) mutable {
		if(wpScene.expired())
		{
			if(cbRenderMode.IsValid())
				cbRenderMode.Remove();
			return;
		}
		auto pRenderComponent = ent.GetComponent<pragma::CRenderComponent>();
		auto renderMode = pRenderComponent.valid() ? pRenderComponent->GetRenderMode() : RenderMode::None;
		auto &occlusionTree = GetOcclusionOctree();
		if(renderMode == RenderMode::World || renderMode == RenderMode::Skybox || renderMode == RenderMode::Water)
		{
			if(occlusionTree.ContainsObject(&ent) == false)
				fInsertOctreeObject(&ent);
		}
		else
			occlusionTree.RemoveObject(&ent);
	});
	pRenderComponent->GetRenderModeProperty()->AddCallback(cbRenderMode);
	auto renderMode = pRenderComponent->GetRenderMode();
	if(renderMode != RenderMode::World && renderMode != RenderMode::Skybox && renderMode != RenderMode::Water)
		return;
	fInsertOctreeObject(&ent);
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
const OcclusionOctree<CBaseEntity*> &Scene::GetOcclusionOctree() const {return const_cast<Scene*>(this)->GetOcclusionOctree();}
OcclusionOctree<CBaseEntity*> &Scene::GetOcclusionOctree() {return *m_occlusionOctree;}

const std::vector<pragma::OcclusionMeshInfo> &Scene::GetCulledMeshes() const {return m_culledMeshes;}
std::vector<pragma::OcclusionMeshInfo> &Scene::GetCulledMeshes() {return m_culledMeshes;}
const std::vector<pragma::CParticleSystemComponent*> &Scene::GetCulledParticles() const {return m_culledParticles;}
std::vector<pragma::CParticleSystemComponent*> &Scene::GetCulledParticles() {return m_culledParticles;}

void Scene::SetPrepassMode(PrepassMode mode)
{
	auto &prepass = GetPrepass();
	switch(static_cast<PrepassMode>(mode))
	{
		case PrepassMode::NoPrepass:
			m_bPrepassEnabled = false;
			break;
		case PrepassMode::DepthOnly:
			m_bPrepassEnabled = true;
			prepass.SetUseExtendedPrepass(false);
			break;
		case PrepassMode::Extended:
			m_bPrepassEnabled = true;
			prepass.SetUseExtendedPrepass(true);
			break;
	}
}
Scene::PrepassMode Scene::GetPrepassMode() const
{
	if(m_bPrepassEnabled == false)
		return PrepassMode::NoPrepass;
	auto &prepass = const_cast<Scene*>(this)->GetPrepass();
	return prepass.IsExtended() ? PrepassMode::Extended : PrepassMode::DepthOnly;
}

bool Scene::IsValid() const {return m_bValid;}

pragma::ShaderPrepassBase &Scene::GetPrepassShader() const {return const_cast<Scene*>(this)->GetPrepass().GetShader();}

void Scene::UpdateLightDescriptorSets(const std::vector<pragma::CLightComponent*> &lightSources)
{
	auto *descSetShadowMaps = GetCSMDescriptorSet();
	if(descSetShadowMaps == nullptr)
		return;
	auto numLights = lightSources.size();
	for(auto i=decltype(numLights){0};i<numLights;++i)
	{
		auto &light = *lightSources.at(i);
		auto type = LightType::Invalid;
		auto *pLight = light.GetLight(type);
		if(type != LightType::Directional)
			continue;
		auto *shadowMap = light.GetShadowMap();
		auto texture = (shadowMap != nullptr) ? shadowMap->GetDepthTexture() : nullptr;
		if(texture != nullptr)
		{
			auto numLayers = shadowMap->GetLayerCount();
			for(auto i=decltype(numLayers){0};i<numLayers;++i)
			{
				prosper::util::set_descriptor_set_binding_array_texture(
					*descSetShadowMaps,*texture,0u,i,i
				);
			}
		}
		break;
	}
}

void Scene::InitializeLightDescriptorSets()
{
	if(pragma::ShaderTextured3D::DESCRIPTOR_SET_CSM.IsValid())
		m_descSetGroupCSM = prosper::util::create_descriptor_set_group(c_engine->GetDevice(),pragma::ShaderTextured3D::DESCRIPTOR_SET_CSM);
}

Anvil::DescriptorSet *Scene::GetCSMDescriptorSet() const {return (*m_descSetGroupCSM)->get_descriptor_set(0u);}

Scene::HDRInfo &Scene::GetHDRInfo() {return m_hdrInfo;}
Scene::GlowInfo &Scene::GetGlowInfo() {return m_glowInfo;}
SSAOInfo &Scene::GetSSAOInfo() {return m_hdrInfo.ssaoInfo;}
pragma::rendering::Prepass &Scene::GetPrepass() {return m_hdrInfo.prepass;}
pragma::rendering::ForwardPlusInstance &Scene::GetForwardPlusInstance() {return m_hdrInfo.forwardPlusInstance;}

uint32_t Scene::GetWidth() const {return m_width;}
uint32_t Scene::GetHeight() const {return m_height;}

Float Scene::GetHDRExposure() const {return m_hdrInfo.exposure;}
Float Scene::GetMaxHDRExposure() const {return m_hdrInfo.max_exposure;}
void Scene::SetMaxHDRExposure(Float exposure) {m_hdrInfo.max_exposure = exposure;}

//const Vulkan::DescriptorSet &Scene::GetBloomGlowDescriptorSet() const {return m_descSetBloomGlow;} // prosper TODO

void Scene::ReloadRenderTarget()
{
	m_bValid = false;
	//auto &context = c_engine->GetRenderContext(); // prosper TODO

	auto width = GetWidth();
	auto height = GetHeight();
	auto bSsao = IsSSAOEnabled();
	if(
		m_hdrInfo.Initialize(*this,width,height,m_sampleCount,bSsao) == false || 
		m_glowInfo.Initialize(width,height,m_hdrInfo) == false ||
		m_hdrInfo.InitializeDescriptorSets() == false
	)
		return;

	auto &descSetHdrResolve = *(*m_hdrInfo.descSetGroupHdrResolve)->get_descriptor_set(0u);
	auto resolvedGlowTex = GetGlowInfo().renderTarget->GetTexture();
	if(resolvedGlowTex->IsMSAATexture())
		resolvedGlowTex = static_cast<prosper::MSAATexture&>(*resolvedGlowTex).GetResolvedTexture();
	prosper::util::set_descriptor_set_binding_texture(descSetHdrResolve,*resolvedGlowTex,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));

	auto &descSetCam = *(*m_camDescSetGroupGraphics)->get_descriptor_set(0u);
	auto &descSetCamView = *(*m_camViewDescSetGroup)->get_descriptor_set(0u);
	if(bSsao == true)
	{
		auto &ssaoInfo = GetSSAOInfo();
		auto ssaoBlurTexResolved = ssaoInfo.renderTargetBlur->GetTexture();
		if(ssaoBlurTexResolved->IsMSAATexture())
			ssaoBlurTexResolved = static_cast<prosper::MSAATexture&>(*ssaoBlurTexResolved).GetResolvedTexture();
		prosper::util::set_descriptor_set_binding_texture(descSetCam,*ssaoBlurTexResolved,umath::to_integral(pragma::ShaderScene::CameraBinding::SSAOMap));
		prosper::util::set_descriptor_set_binding_texture(descSetCamView,*ssaoBlurTexResolved,umath::to_integral(pragma::ShaderScene::CameraBinding::SSAOMap));
	}
	auto &dummyTex = c_engine->GetDummyTexture();
	prosper::util::set_descriptor_set_binding_texture(descSetCam,*dummyTex,umath::to_integral(pragma::ShaderScene::CameraBinding::LightMap));
	prosper::util::set_descriptor_set_binding_texture(descSetCamView,*dummyTex,umath::to_integral(pragma::ShaderScene::CameraBinding::LightMap));
	/*auto layoutBloomGlow = Vulkan::DescriptorSetLayout::Create(context,{
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT}, // Bloom
		{Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,Anvil::ShaderStageFlagBits::FRAGMENT_BIT} // Glow
	});
	m_descSetBloomGlow = Vulkan::DescriptorSet::Create(context,context.GetDescriptorPool(Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER),layoutBloomGlow);
	m_descSetBloomGlow->Update(umath::to_integral(Shader::PPHDR::Binding::Bloom),m_hdrInfo.blurBuffer.tmpBlurTexture);
	m_descSetBloomGlow->Update(umath::to_integral(Shader::PPHDR::Binding::Glow),m_glowInfo.renderTarget.GetTexture());

	auto numTiles = static_cast<uint32_t>(m_hdrInfo.forwardPlusInstance.GetWorkGroupCount().first);
	m_renderSettingsData.Write(umath::to_integral(UB_RENDER_SETTINGS_OFFSET::NUMBER_OF_TILES_X),sizeof(numTiles),&numTiles);

	// Update scene descriptor set (Include blurred ssao map and fog buffer)
	if(m_hdrInfo.ssaoInfo.rtOcclusionBlur != nullptr)
	{
		auto &texSSAO = m_hdrInfo.ssaoInfo.rtOcclusionBlur->GetTexture();
		if(texSSAO != nullptr)
		{
			auto numDescSets = m_swapDescBufferCamGraphics->GetDescriptorCount();
			for(auto i=decltype(numDescSets){0};i<numDescSets;++i)
			{
				auto &descSetCam = *m_swapDescBufferCamGraphics->GetDescriptorSet(i);
				auto &descSetViewCam = *m_swapDescBufferViewCam->GetDescriptorSet(i);

				descSetCam->Update(umath::to_integral(Shader::TexturedBase3D::Binding::SSAOMap),texSSAO);
				descSetViewCam->Update(umath::to_integral(Shader::TexturedBase3D::Binding::SSAOMap),texSSAO);
			}
		}
	}*/ // prosper TODO
	//
	//if(m_renderTarget != nullptr) // prosper TODO
	//	InitializeRenderTarget(); // prosper TODO
	m_bValid = true;
}

const pragma::OcclusionCullingHandler &Scene::GetOcclusionCullingHandler() const {return const_cast<Scene*>(this)->GetOcclusionCullingHandler();}
pragma::OcclusionCullingHandler &Scene::GetOcclusionCullingHandler() {return *m_occlusionCullingHandler;}
void Scene::SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler) {m_occlusionCullingHandler = handler;}
void Scene::ReloadOcclusionCullingHandler()
{
	auto occlusionCullingMode = c_game->GetConVarInt("cl_render_occlusion_culling");
	switch(occlusionCullingMode)
	{
		case 1: /* Brute-force */
			m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerBruteForce>();
			break;
		case 2: /* CHC++ */
			m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerCHC>();
			break;
		case 4: /* BSP */
		{
			auto itWorld = std::find_if(m_entityList->entities.begin(),m_entityList->entities.end(),[](const EntityHandle &hEnt) {
				return hEnt.IsValid() && hEnt.get()->IsWorld();
			});
			if(itWorld != m_entityList->entities.end())
			{
				auto *entWorld = itWorld->get();
				auto pWorldComponent = entWorld->GetComponent<pragma::CWorldComponent>();
				auto bspTree = pWorldComponent.valid() ? pWorldComponent->GetBSPTree() : nullptr;
				if(bspTree != nullptr && bspTree->GetNodes().size() > 1u)
				{
					m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerBSP>(bspTree);
					break;
				}
			}
		}
		case 3: /* Octtree */
			m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerOctTree>();
			break;
		case 0: /* Off */
		default:
			m_occlusionCullingHandler = std::make_shared<pragma::OcclusionCullingHandlerInert>();
			break;
	}
	m_occlusionCullingHandler->Initialize();
}

void Scene::InitializeRenderTarget()
{
	/*auto &context = c_engine->GetRenderContext();
	auto width = GetWidth();
	auto height = GetHeight();
	m_renderTarget = Vulkan::RenderTarget::Create(context,width,height,Anvil::Format::R8G8B8A8_UNORM,false,[](vk::ImageCreateInfo &info,vk::MemoryPropertyFlags&) {
		info.usage = Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	});
	static auto hShaderScreen = ShaderSystem::get_shader("screen");
	if(hShaderScreen.IsValid())
	{
		if(m_descSetScreen == nullptr)
		{
			auto &shaderScreen = static_cast<Shader::Screen&>(*hShaderScreen.get());
			shaderScreen.InitializeInstance(m_descSetScreen);
		}
		m_descSetScreen->Update(m_renderTarget->GetTexture());
	}*/ // prosper TODO
}

Anvil::DescriptorSet *Scene::GetDepthDescriptorSet() const {return (m_hdrInfo.descSetGroupDepth != nullptr) ? (*m_hdrInfo.descSetGroupDepth)->get_descriptor_set(0u) : nullptr;}
/*const Vulkan::DescriptorSet &Scene::GetRenderDepthDescriptorSet() const {return m_hdrInfo.descSetDepth;}
const Vulkan::DescriptorSet &Scene::GetScreenDescriptorSet() const {return m_descSetScreen;}
const Vulkan::Texture &Scene::GetRenderDepthBuffer() const {return m_hdrInfo.prepass.textureDepth->GetTexture();}
const Vulkan::Texture &Scene::GetRenderTexture() const {return m_hdrInfo.GetRenderTexture();}
const Vulkan::Texture &Scene::GetBloomTexture() const {return m_hdrInfo.GetTargetBloomTexture();}
const Vulkan::RenderTarget &Scene::GetRenderTarget() const {return m_renderTarget;}
const Vulkan::Texture &Scene::GetGlowTexture() const {return m_glowInfo.renderTarget.GetTexture();}
Vulkan::Texture &Scene::GetDepthTexture() {return const_cast<Vulkan::Texture&>(m_hdrInfo.prepass.textureDepth->GetTexture());}

const Vulkan::DescriptorSet *Scene::GetCSMShadowDescriptorSet(uint32_t layer,uint32_t swapIdx)
{
	if(layer >= m_csmDescriptors.size())
		return nullptr;
	return m_csmDescriptors[layer]->descBuffer->GetDescriptorSet(swapIdx);
}
const Vulkan::Buffer *Scene::GetCSMShadowBuffer(uint32_t layer,uint32_t swapIdx)
{
	if(layer >= m_csmDescriptors.size())
		return nullptr;
	return m_csmDescriptors[layer]->descBuffer->GetBuffer(0,swapIdx);
}*/ // prosper TODO

float Scene::GetFOV() {return camera->GetFOV();}
float Scene::GetViewFOV() {return camera->GetViewFOV();}
float Scene::GetAspectRatio() {return camera->GetAspectRatio();}
float Scene::GetZNear() {return camera->GetZNear();}
float Scene::GetZFar() {return camera->GetZFar();}
const std::shared_ptr<Camera> &Scene::GetCamera() const {return camera;}
