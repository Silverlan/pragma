#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_brute_force.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_bsp.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_chc.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_inert.hpp"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler_octtree.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/shaders/post_processing/c_shader_pp_hdr.hpp"
#include "pragma/rendering/renderers/rasterization/hdr_data.hpp"
#include "pragma/rendering/renderers/rasterization/glow_data.hpp"
#include <image/prosper_render_target.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <prosper_descriptor_set_group.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

static void cl_render_ssao_callback(NetworkState*,ConVar*,bool,bool val)
{
	if(c_game == nullptr)
		return;
	auto &scene = c_game->GetScene();
	if(scene == nullptr)
		return;
	auto *renderer = dynamic_cast<RasterizationRenderer*>(scene->GetRenderer());
	if(renderer == nullptr)
		return;
	renderer->SetSSAOEnabled(val);
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_ssao,cl_render_ssao_callback);

RasterizationRenderer::RasterizationRenderer(Scene &scene)
	: BaseRenderer{scene},m_hdrInfo{*this}
{
	m_whShaderWireframe = c_engine->GetShader("wireframe");

	InitializeLightDescriptorSets();
	ReloadOcclusionCullingHandler();
}

RasterizationRenderer::~RasterizationRenderer()
{
	m_occlusionCullingHandler = nullptr;
}

bool RasterizationRenderer::Initialize() {return true;}

CulledMeshData *RasterizationRenderer::GetRenderInfo(RenderMode renderMode) const
{
	auto it = m_renderInfo.find(renderMode);
	if(it == m_renderInfo.end())
		return nullptr;
	return it->second.get();
}

void RasterizationRenderer::UpdateLightDescriptorSets(const std::vector<pragma::CLightComponent*> &lightSources)
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
		// TODO: Dynamic!
		auto hShadowMap = light.GetShadowMap(pragma::CLightComponent::ShadowMapType::Static);
		auto texture = hShadowMap.valid() ? hShadowMap->GetDepthTexture() : nullptr;
		if(texture != nullptr)
		{
			auto numLayers = hShadowMap->GetLayerCount();
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

Anvil::DescriptorSet *RasterizationRenderer::GetDepthDescriptorSet() const {return (m_hdrInfo.descSetGroupDepth != nullptr) ? (*m_hdrInfo.descSetGroupDepth)->get_descriptor_set(0u) : nullptr;}

void RasterizationRenderer::InitializeLightDescriptorSets()
{
	if(pragma::ShaderTextured3D::DESCRIPTOR_SET_CSM.IsValid())
		m_descSetGroupCSM = prosper::util::create_descriptor_set_group(c_engine->GetDevice(),pragma::ShaderTextured3D::DESCRIPTOR_SET_CSM);
}

void RasterizationRenderer::SetFogOverride(const std::shared_ptr<prosper::DescriptorSetGroup> &descSetGroup) {m_descSetGroupFogOverride = descSetGroup;}

bool RasterizationRenderer::ReloadRenderTarget()
{
	auto &scene = GetScene();
	auto width = scene.GetWidth();
	auto height = scene.GetHeight();
	auto bSsao = IsSSAOEnabled();
	if(
		m_hdrInfo.Initialize(*this,width,height,m_sampleCount,bSsao) == false || 
		m_glowInfo.Initialize(width,height,m_hdrInfo) == false ||
		m_hdrInfo.InitializeDescriptorSets() == false
		)
		return false;

	auto &descSetHdrResolve = *(*m_hdrInfo.descSetGroupHdrResolve)->get_descriptor_set(0u);
	auto resolvedGlowTex = GetGlowInfo().renderTarget->GetTexture();
	if(resolvedGlowTex->IsMSAATexture())
		resolvedGlowTex = static_cast<prosper::MSAATexture&>(*resolvedGlowTex).GetResolvedTexture();
	prosper::util::set_descriptor_set_binding_texture(descSetHdrResolve,*resolvedGlowTex,umath::to_integral(pragma::ShaderPPHDR::TextureBinding::Glow));

	auto &descSetCam = *scene.GetCameraDescriptorSetGraphics();
	auto &descSetCamView = *scene.GetViewCameraDescriptorSet();
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
	return true;
}
void RasterizationRenderer::SetFrameDepthBufferSamplingRequired() {m_bFrameDepthBufferSamplingRequired = true;}
void RasterizationRenderer::EndRendering() {}
void RasterizationRenderer::BeginRendering(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	BaseRenderer::BeginRendering(drawCmd);
	umath::set_flag(m_stateFlags,StateFlags::DepthResolved | StateFlags::BloomResolved | StateFlags::RenderResolved,false);
}

bool RasterizationRenderer::IsSSAOEnabled() const {return umath::is_flag_set(m_stateFlags,StateFlags::SSAOEnabled);}
void RasterizationRenderer::SetSSAOEnabled(bool b)
{
	umath::set_flag(m_stateFlags,StateFlags::SSAOEnabled,b);
	UpdateRenderSettings(GetScene().GetRenderSettings());
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
void RasterizationRenderer::UpdateCameraData(pragma::CameraData &cameraData)
{
	UpdateFrustumPlanes();
}
void RasterizationRenderer::UpdateRenderSettings(pragma::RenderSettings &renderSettings)
{
	auto &scene = GetScene();
	auto &tileInfo = renderSettings.tileInfo;
	tileInfo = static_cast<uint32_t>(pragma::ShaderForwardPLightCulling::TILE_SIZE)<<16;
	tileInfo |= static_cast<uint32_t>(pragma::rendering::ForwardPlusInstance::CalcWorkGroupCount(scene.GetWidth(),scene.GetHeight()).first);

	if(IsSSAOEnabled() == true)
		renderSettings.flags |= umath::to_integral(Scene::FRenderSetting::SSAOEnabled);
}
void RasterizationRenderer::SetShaderOverride(const std::string &srcShaderId,const std::string &shaderOverrideId)
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
pragma::ShaderTextured3D *RasterizationRenderer::GetShaderOverride(pragma::ShaderTextured3D *srcShader)
{
	if(srcShader == nullptr)
		return nullptr;
	auto it = m_shaderOverrides.find(typeid(*srcShader).hash_code());
	if(it == m_shaderOverrides.end())
		return srcShader;
	return static_cast<pragma::ShaderTextured3D*>(it->second.get());
}
void RasterizationRenderer::ClearShaderOverride(const std::string &srcShaderId)
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

const std::vector<pragma::OcclusionMeshInfo> &RasterizationRenderer::GetCulledMeshes() const {return m_culledMeshes;}
std::vector<pragma::OcclusionMeshInfo> &RasterizationRenderer::GetCulledMeshes() {return m_culledMeshes;}
const std::vector<pragma::CParticleSystemComponent*> &RasterizationRenderer::GetCulledParticles() const {return m_culledParticles;}
std::vector<pragma::CParticleSystemComponent*> &RasterizationRenderer::GetCulledParticles() {return m_culledParticles;}

void RasterizationRenderer::SetPrepassMode(PrepassMode mode)
{
	auto &prepass = GetPrepass();
	switch(static_cast<PrepassMode>(mode))
	{
	case PrepassMode::NoPrepass:
		umath::set_flag(m_stateFlags,StateFlags::PrepassEnabled,false);
		break;
	case PrepassMode::DepthOnly:
		umath::set_flag(m_stateFlags,StateFlags::PrepassEnabled,true);
		prepass.SetUseExtendedPrepass(false);
		break;
	case PrepassMode::Extended:
		umath::set_flag(m_stateFlags,StateFlags::PrepassEnabled,true);
		prepass.SetUseExtendedPrepass(true);
		break;
	}
}
RasterizationRenderer::PrepassMode RasterizationRenderer::GetPrepassMode() const
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::PrepassEnabled) == false)
		return PrepassMode::NoPrepass;
	auto &prepass = const_cast<RasterizationRenderer*>(this)->GetPrepass();
	return prepass.IsExtended() ? PrepassMode::Extended : PrepassMode::DepthOnly;
}

pragma::ShaderPrepassBase &RasterizationRenderer::GetPrepassShader() const {return const_cast<RasterizationRenderer*>(this)->GetPrepass().GetShader();}

Anvil::DescriptorSet *RasterizationRenderer::GetCSMDescriptorSet() const {return (*m_descSetGroupCSM)->get_descriptor_set(0u);}

HDRData &RasterizationRenderer::GetHDRInfo() {return m_hdrInfo;}
GlowData &RasterizationRenderer::GetGlowInfo() {return m_glowInfo;}
SSAOInfo &RasterizationRenderer::GetSSAOInfo() {return m_hdrInfo.ssaoInfo;}
pragma::rendering::Prepass &RasterizationRenderer::GetPrepass() {return m_hdrInfo.prepass;}
const pragma::rendering::ForwardPlusInstance &RasterizationRenderer::GetForwardPlusInstance() const {return const_cast<RasterizationRenderer*>(this)->GetForwardPlusInstance();}
pragma::rendering::ForwardPlusInstance &RasterizationRenderer::GetForwardPlusInstance() {return m_hdrInfo.forwardPlusInstance;}

Float RasterizationRenderer::GetHDRExposure() const {return m_hdrInfo.exposure;}
Float RasterizationRenderer::GetMaxHDRExposure() const {return m_hdrInfo.max_exposure;}
void RasterizationRenderer::SetMaxHDRExposure(Float exposure) {m_hdrInfo.max_exposure = exposure;}

const pragma::OcclusionCullingHandler &RasterizationRenderer::GetOcclusionCullingHandler() const {return const_cast<RasterizationRenderer*>(this)->GetOcclusionCullingHandler();}
pragma::OcclusionCullingHandler &RasterizationRenderer::GetOcclusionCullingHandler() {return *m_occlusionCullingHandler;}
void RasterizationRenderer::SetOcclusionCullingHandler(const std::shared_ptr<pragma::OcclusionCullingHandler> &handler) {m_occlusionCullingHandler = handler;}
void RasterizationRenderer::ReloadOcclusionCullingHandler()
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
		auto &scene = GetScene();
		auto &entityList = *scene.GetEntityListInfo();
		auto itWorld = std::find_if(entityList.entities.begin(),entityList.entities.end(),[](const EntityHandle &hEnt) {
			return hEnt.IsValid() && hEnt.get()->IsWorld();
			});
		if(itWorld != entityList.entities.end())
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

const std::vector<Plane> &RasterizationRenderer::GetFrustumPlanes() const {return m_frustumPlanes;}
const std::vector<Plane> &RasterizationRenderer::GetClippedFrustumPlanes() const {return m_clippedFrustumPlanes;}

void RasterizationRenderer::UpdateFrustumPlanes()
{
	m_frustumPlanes.clear();
	m_clippedFrustumPlanes.clear();
	auto &scene = GetScene();
	auto &cam = scene.GetActiveCamera();
	if(cam.expired())
		return;
	cam->GetFrustumPlanes(m_frustumPlanes);
	m_clippedFrustumPlanes = m_frustumPlanes;
/*	auto forward = camera->GetForward();
	auto up = camera->GetUp();
	auto rot = camera->GetRotation();
	auto pos = camera->GetPos();
	camera->SetForward(uvec::FORWARD);
	camera->SetUp(uvec::UP);
	camera->SetPos(Vector3{});

	std::vector<Vector3> frustumPoints {};
	camera->GetFrustumPoints(frustumPoints);
	for(auto &p : frustumPoints)
	{
		uvec::rotate(&p,rot);
		p += pos;
	}
	camera->GetFrustumPlanes(frustumPoints,m_frustumPlanes);
	m_clippedFrustumPlanes = m_frustumPlanes;

	camera->SetForward(forward);
	camera->SetUp(up);
	camera->SetPos(pos);*/


	/*if(FogController::IsFogEnabled() == true)
	{
		float fogDist = FogController::GetFarDistance();
		float farZ = cam->GetZFar();
		if(fogDist < farZ)
			farZ = fogDist;
		Plane &farPlane = planesClipped[static_cast<int>(FrustumPlane::Far)];
		Vector3 &start = planesClipped[static_cast<int>(FrustumPlane::Near)].GetCenterPos();
		Vector3 dir = farPlane.GetCenterPos() -start;
		uvec::normalize(&dir);
		farPlane.MoveToPos(start +dir *farZ); // TODO Checkme
	}*/
}

void RasterizationRenderer::SetLightMap(const std::shared_ptr<prosper::Texture> &lightMapTexture)
{
	m_lightMapInfo.lightMapTexture = lightMapTexture;
	auto &descSetCam = *GetScene().GetCameraDescriptorSetGraphics();
	auto &descSetCamView = *GetScene().GetViewCameraDescriptorSet();
	prosper::util::set_descriptor_set_binding_texture(descSetCam,*lightMapTexture,umath::to_integral(pragma::ShaderScene::CameraBinding::LightMap));
	prosper::util::set_descriptor_set_binding_texture(descSetCamView,*lightMapTexture,umath::to_integral(pragma::ShaderScene::CameraBinding::LightMap));
}
const std::shared_ptr<prosper::Texture> &RasterizationRenderer::GetLightMap() const {return m_lightMapInfo.lightMapTexture;}
const std::shared_ptr<prosper::Texture> &RasterizationRenderer::GetSceneTexture() const {return m_hdrInfo.hdrRenderTarget->GetTexture();}
const std::shared_ptr<prosper::Texture> &RasterizationRenderer::GetPresentationTexture() const {return m_hdrInfo.postHdrRenderTarget->GetTexture();}
const std::shared_ptr<prosper::Texture> &RasterizationRenderer::GetHDRPresentationTexture() const {return m_hdrInfo.hdrRenderTarget->GetTexture();}
bool RasterizationRenderer::IsRasterizationRenderer() const {return true;}
void RasterizationRenderer::OnEntityAddedToScene(CBaseEntity &ent)
{
	BaseRenderer::OnEntityAddedToScene(ent);
}

Anvil::SampleCountFlagBits RasterizationRenderer::GetSampleCount() const {return const_cast<RasterizationRenderer*>(this)->GetHDRInfo().hdrRenderTarget->GetTexture()->GetImage()->GetSampleCount();}
bool RasterizationRenderer::IsMultiSampled() const {return GetSampleCount() != Anvil::SampleCountFlagBits::_1_BIT;}

bool RasterizationRenderer::BeginRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::RenderPass *customRenderPass)
{
	auto &hdrInfo = GetHDRInfo();
	auto &tex = hdrInfo.hdrRenderTarget->GetTexture();
	if(tex->IsMSAATexture())
		static_cast<prosper::MSAATexture&>(*tex).Reset();
	return hdrInfo.BeginRenderPass(drawCmd,customRenderPass);
}
bool RasterizationRenderer::EndRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	auto &hdrInfo = GetHDRInfo();
	return hdrInfo.EndRenderPass(drawCmd);
}
bool RasterizationRenderer::ResolveRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd)
{
	auto &hdrInfo = GetHDRInfo();
	return hdrInfo.ResolveRenderPass(drawCmd);
}
