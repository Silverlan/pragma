#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/console/c_cvar.h"
#include <cmaterialmanager.h>
#include <pragma/console/convars.h>
#include <prosper_util.hpp>
#include <prosper_command_buffer.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <sharedutils/util_shaderinfo.hpp>

using namespace pragma::rendering;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

void RasterizationRenderer::RenderPrepass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,FRender renderFlags)
{
	auto prepassMode = GetPrepassMode();
	if(prepassMode == PrepassMode::NoPrepass)
		return;
	auto &scene = GetScene();
	auto &hCam = scene.GetActiveCamera();
	// Pre-render depths and normals (if SSAO is enabled)
	c_game->StartProfilingStage(CGame::CPUProfilingPhase::Prepass);
	c_game->StartProfilingStage(CGame::GPUProfilingPhase::Prepass);
	auto &prepass = GetPrepass();
	if(prepass.textureDepth->IsMSAATexture())
		static_cast<prosper::MSAATexture&>(*prepass.textureDepth).Reset();
	if(prepass.textureNormals != nullptr && prepass.textureNormals->IsMSAATexture())
		static_cast<prosper::MSAATexture&>(*prepass.textureNormals).Reset();

	// Entity instance buffer barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,*pragma::CRenderComponent::GetInstanceBuffer(),
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	// Entity bone buffer barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,*pragma::get_instance_bone_buffer(),
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	// Camera buffer barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,*scene.GetCameraBuffer(),
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::GEOMETRY_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	// View camera buffer barrier
	prosper::util::record_buffer_barrier(
		**drawCmd,*scene.GetViewCameraBuffer(),
		Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
		Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
	);

	prepass.BeginRenderPass(*drawCmd);
	auto bReflection = ((renderFlags &FRender::Reflection) != FRender::None) ? true : false;
	auto pipelineType = (bReflection == true) ? pragma::ShaderPrepassBase::Pipeline::Reflection :
		(GetSampleCount() == Anvil::SampleCountFlagBits::_1_BIT) ? pragma::ShaderPrepassBase::Pipeline::Regular :
		pragma::ShaderPrepassBase::Pipeline::MultiSample;
	auto &shaderDepthStage = GetPrepass().GetShader();
	if(shaderDepthStage.BeginDraw(drawCmd,pipelineType) == true)
	{
		shaderDepthStage.BindClipPlane(c_game->GetRenderClipPlane());
		shaderDepthStage.BindSceneCamera(*this,false);
		if((renderFlags &FRender::Skybox) != FRender::None)
		{
			c_game->StartProfilingStage(CGame::GPUProfilingPhase::PrepassSkybox);
			if(hCam.valid())
				RenderSystem::RenderPrepass(drawCmd,*hCam,GetCulledMeshes(),RenderMode::Skybox,bReflection);
			c_game->StopProfilingStage(CGame::GPUProfilingPhase::PrepassSkybox);
		}
		if((renderFlags &FRender::World) != FRender::None)
		{
			c_game->StartProfilingStage(CGame::GPUProfilingPhase::PrepassWorld);
			if(hCam.valid())
				RenderSystem::RenderPrepass(drawCmd,*hCam,GetCulledMeshes(),RenderMode::World,bReflection);
			c_game->StopProfilingStage(CGame::GPUProfilingPhase::PrepassWorld);
		}
		c_game->CallCallbacks<void>("RenderPrepass");
		c_game->CallLuaCallbacks("RenderPrepass");

		shaderDepthStage.BindSceneCamera(*this,true);
		if((renderFlags &FRender::View) != FRender::None)
		{
			c_game->StartProfilingStage(CGame::GPUProfilingPhase::PrepassView);
			if(hCam.valid())
				RenderSystem::RenderPrepass(drawCmd,*hCam,GetCulledMeshes(),RenderMode::View,bReflection);
			c_game->StopProfilingStage(CGame::GPUProfilingPhase::PrepassView);
		}
		shaderDepthStage.EndDraw();
	}
	prepass.EndRenderPass(*drawCmd);
	c_game->StopProfilingStage(CGame::GPUProfilingPhase::Prepass);
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::Prepass);
}

void RasterizationRenderer::PerformOcclusionCulling()
{
	c_game->StartProfilingStage(CGame::CPUProfilingPhase::OcclusionCulling);
	GetOcclusionCullingHandler().PerformCulling(*this,GetCulledParticles());

	auto &renderMeshes = GetCulledMeshes();
	GetOcclusionCullingHandler().PerformCulling(*this,renderMeshes);
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::OcclusionCulling);
}

static auto cvDrawGlow = GetClientConVar("render_draw_glow");
static auto cvDrawTranslucent = GetClientConVar("render_draw_translucent");
static auto cvDrawSky = GetClientConVar("render_draw_sky");
static auto cvDrawWater = GetClientConVar("render_draw_water");
static auto cvDrawView = GetClientConVar("render_draw_view");
void RasterizationRenderer::CollectRenderObjects(FRender renderFlags)
{
	// Prepare rendering
	c_game->StartProfilingStage(CGame::CPUProfilingPhase::PrepareRendering);
	auto bGlow = cvDrawGlow->GetBool();
	auto bTranslucent = cvDrawTranslucent->GetBool();
	if((renderFlags &FRender::Skybox) == FRender::Skybox && c_game->IsRenderModeEnabled(RenderMode::Skybox) && cvDrawSky->GetBool() == true)
		PrepareRendering(RenderMode::Skybox,renderFlags,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::Skybox;

	if((renderFlags &FRender::World) == FRender::World && c_game->IsRenderModeEnabled(RenderMode::World))
		PrepareRendering(RenderMode::World,renderFlags,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::World;

	if((renderFlags &FRender::Water) == FRender::Water && c_game->IsRenderModeEnabled(RenderMode::Water) && cvDrawWater->GetBool() == true)
		PrepareRendering(RenderMode::Water,renderFlags,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::Water;

	auto *pl = c_game->GetLocalPlayer();
	if((renderFlags &FRender::View) == FRender::View && c_game->IsRenderModeEnabled(RenderMode::View) && pl != nullptr && pl->IsInFirstPersonMode() == true && cvDrawView->GetBool() == true)
		PrepareRendering(RenderMode::View,renderFlags,bTranslucent,bGlow);
	else
		renderFlags &= ~FRender::View;
	c_game->StopProfilingStage(CGame::CPUProfilingPhase::PrepareRendering);
}

static auto cvDrawWorld = GetClientConVar("render_draw_world");
void RasterizationRenderer::PrepareRendering(RenderMode renderMode,FRender renderFlags,bool bUpdateTranslucentMeshes,bool bUpdateGlowMeshes)
{
	auto it = m_renderInfo.find(renderMode);
	if(it == m_renderInfo.end())
		it = m_renderInfo.insert(decltype(m_renderInfo)::value_type(renderMode,std::make_shared<CulledMeshData>())).first;

	auto &renderInfo = it->second;
	auto &cam = GetScene().GetActiveCamera();
	auto &posCam = cam.valid() ? cam->GetEntity().GetPosition() : uvec::ORIGIN;

	auto drawWorld = cvDrawWorld->GetInt();
	auto *matLoad = c_game->GetLoadMaterial();
	auto &renderMeshes = GetCulledMeshes();
	auto &glowMeshes = renderInfo->glowMeshes;
	auto &translucentMeshes = renderInfo->translucentMeshes;
	auto &processed = renderInfo->processed;
	auto &containers = renderInfo->containers;
	glowMeshes.clear();
	translucentMeshes.clear();
	processed.clear();
	containers.clear();

	for(auto it=renderMeshes.begin();it!=renderMeshes.end();++it)
	{
		auto &info = *it;
		auto *ent = static_cast<CBaseEntity*>(info.hEntity.get());
		auto isStatic = ent->IsStatic();
		if(
			(umath::is_flag_set(renderFlags,FRender::Static) == false && isStatic) ||
			(umath::is_flag_set(renderFlags,FRender::Dynamic) == false && isStatic == false)
			)
			continue;
		auto pRenderComponent = ent->GetRenderComponent();
		if(pRenderComponent.expired())
			continue;
		auto rm = pRenderComponent->GetRenderMode();
		if(renderMode == rm)
		{
			auto itProcessed = renderInfo->processed.find(ent);
			if(itProcessed == renderInfo->processed.end())
			{
				auto drawCmd = c_game->GetCurrentDrawCommandBuffer();
				pRenderComponent->UpdateRenderData(drawCmd);//,true);
				pRenderComponent->Render(renderMode);

				auto wpRenderBuffer = pRenderComponent->GetRenderBuffer();
				if(wpRenderBuffer.expired() == false)
				{
					prosper::util::record_buffer_barrier(
						**drawCmd,*wpRenderBuffer.lock(),
						Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
						Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
					);
					auto pAnimComponent = ent->GetAnimatedComponent();
					if(pAnimComponent.valid())
					{
						auto wpBoneBuffer = static_cast<pragma::CAnimatedComponent*>(pAnimComponent.get())->GetBoneBuffer();
						if(wpBoneBuffer.expired() == false)
						{
							prosper::util::record_buffer_barrier(
								**drawCmd,*wpBoneBuffer.lock(),
								Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT | Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,
								Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
							);
						}
					}
				}
				processed.insert(std::remove_reference_t<decltype(processed)>::value_type(ent,true));
			}

			auto &mdlComponent = pRenderComponent->GetModelComponent();
			auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
			assert(mdl != nullptr);
			auto &materials = mdl->GetMaterials();
			if(!materials.empty())
			{
				auto *mesh = static_cast<CModelMesh*>(info.mesh);
				auto &meshes = mesh->GetSubMeshes();
				for(auto it=meshes.begin();it!=meshes.end();++it)
				{
					auto *subMesh = static_cast<CModelSubMesh*>(it->get());
					auto idxTexture = subMesh->GetTexture();
					auto *mat = mdl->GetMaterial(mdlComponent->GetSkin(),idxTexture);
					if(mat == nullptr)
						mat = client->GetMaterialManager().GetErrorMaterial();
					/*else
					{
					auto *diffuse = mat->GetDiffuseMap();
					if(diffuse == nullptr || diffuse->texture == nullptr)
					mat = client->GetMaterialManager().GetErrorMaterial();
					}*/
					if(mat != nullptr)
					{
						if(!mat->IsLoaded())
							mat = matLoad;
						//auto &hMat = materials[idxTexture];
						//if(hMat.IsValid())
						//{
						if(mat != nullptr)// && mat->GetDiffuseMap() != nullptr && static_cast<Texture*>(mat->GetDiffuseMap()->texture) != nullptr &&static_cast<Texture*>(mat->GetDiffuseMap()->texture)->error == false && static_cast<Texture*>(mat->GetDiffuseMap()->texture)->GetTextureID() != 0)
						{
							// Fill glow map
							auto *glowMap = mat->GetGlowMap();
							if(bUpdateGlowMeshes == true)
							{
								if(glowMap != nullptr)
								{
									auto itMat = std::find_if(glowMeshes.begin(),glowMeshes.end(),[&mat](const std::unique_ptr<RenderSystem::MaterialMeshContainer> &m) {
										return (m->material == mat) ? true : false;
										});
									if(itMat == glowMeshes.end())
									{
										glowMeshes.push_back(std::make_unique<RenderSystem::MaterialMeshContainer>(mat));
										itMat = glowMeshes.end() -1;
									}
									auto itEnt = (*itMat)->containers.find(ent);
									if(itEnt == (*itMat)->containers.end())
										itEnt = (*itMat)->containers.emplace(ent,EntityMeshInfo{ent}).first;
									itEnt->second.meshes.push_back(subMesh);
								}
							}
							//
							auto *info = mat->GetShaderInfo();
							if(info != nullptr)
							{
								auto *base = static_cast<::util::WeakHandle<prosper::Shader>*>(const_cast<util::ShaderInfo*>(info)->GetShader().get())->get();
								prosper::Shader *shader = nullptr;
								if(drawWorld == 2)
									shader = m_whShaderWireframe.get();
								else if(base != nullptr && base->GetBaseTypeHashCode() == pragma::ShaderTextured3DBase::HASH_TYPE)
									shader = GetShaderOverride(static_cast<pragma::ShaderTextured3D*>(base));
								if(shader != nullptr && shader->GetBaseTypeHashCode() == pragma::ShaderTextured3DBase::HASH_TYPE)
								{
									// Translucent?
									if(mat->IsTranslucent() == true)
									{
										if(bUpdateTranslucentMeshes == true)
										{
											auto pTrComponent = ent->GetTransformComponent();
											auto pos = subMesh->GetCenter();
											if(pTrComponent.valid())
											{
												uvec::rotate(&pos,pTrComponent->GetOrientation());
												pos += pTrComponent->GetPosition();
											}
											auto distance = uvec::length_sqr(pos -posCam);
											translucentMeshes.push_back(std::make_unique<RenderSystem::TranslucentMesh>(ent,subMesh,mat,shader->GetHandle(),distance));
										}
										continue; // Skip translucent meshes
									}
									//
									ShaderMeshContainer *shaderContainer = nullptr;
									auto itShader = std::find_if(containers.begin(),containers.end(),[shader](const std::unique_ptr<ShaderMeshContainer> &c) {
										return (c->shader.get() == shader) ? true : false;
										});
									if(itShader != containers.end())
										shaderContainer = itShader->get();
									if(shaderContainer == nullptr)
									{
										if(containers.size() == containers.capacity())
											containers.reserve(containers.capacity() +10);
										containers.push_back(std::make_unique<ShaderMeshContainer>(static_cast<pragma::ShaderTextured3D*>(shader)));
										shaderContainer = containers.back().get();
									}
									RenderSystem::MaterialMeshContainer *matContainer = nullptr;
									auto itMat = std::find_if(shaderContainer->containers.begin(),shaderContainer->containers.end(),[mat](const std::unique_ptr<RenderSystem::MaterialMeshContainer> &m) {
										return (m->material == mat) ? true : false;
										});
									if(itMat != shaderContainer->containers.end())
										matContainer = itMat->get();
									if(matContainer == nullptr)
									{
										if(shaderContainer->containers.size() == shaderContainer->containers.capacity())
											shaderContainer->containers.reserve(shaderContainer->containers.capacity() +10);
										shaderContainer->containers.push_back(std::make_unique<RenderSystem::MaterialMeshContainer>(mat));
										matContainer = shaderContainer->containers.back().get();
									}
									EntityMeshInfo *entContainer = nullptr;
									auto itEnt = matContainer->containers.find(ent);
									if(itEnt != matContainer->containers.end())
										entContainer = &itEnt->second;
									if(entContainer == nullptr)
										entContainer = &matContainer->containers.emplace(ent,EntityMeshInfo{ent}).first->second;
									entContainer->meshes.push_back(subMesh);
								}
							}
						}
					}
				}
			}
		}
	}
	if(glowMeshes.empty() == false)
		m_glowInfo.bGlowScheduled = true;
	if(bUpdateTranslucentMeshes == true)
	{
		// Sort translucent meshes by distance
		std::sort(translucentMeshes.begin(),translucentMeshes.end(),[](const std::unique_ptr<RenderSystem::TranslucentMesh> &a,const std::unique_ptr<RenderSystem::TranslucentMesh> &b) {
			return a->distance < b->distance;
			});
	}
}
