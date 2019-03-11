#include "stdafx_client.h"
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/shaders/world/c_shader_wireframe.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <sharedutils/util_shaderinfo.hpp>
#include <pragma/model/model.h>
#include <image/prosper_render_target.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

Scene::RenderInfo *Scene::GetRenderInfo(RenderMode renderMode) const
{
	auto it = m_renderInfo.find(renderMode);
	if(it == m_renderInfo.end())
		return nullptr;
	return it->second.get();
}

Anvil::SampleCountFlagBits Scene::GetSampleCount() const {return const_cast<Scene*>(this)->GetHDRInfo().hdrRenderTarget->GetTexture()->GetImage()->GetSampleCount();}
bool Scene::IsMultiSampled() const {return GetSampleCount() != Anvil::SampleCountFlagBits::_1_BIT;}

static auto cvDrawWorld = GetClientConVar("render_draw_world");
void Scene::PrepareRendering(RenderMode renderMode,bool bUpdateTranslucentMeshes,bool bUpdateGlowMeshes)
{
	auto it = m_renderInfo.find(renderMode);
	if(it == m_renderInfo.end())
		it = m_renderInfo.insert(decltype(m_renderInfo)::value_type(renderMode,std::make_shared<RenderInfo>())).first;

	auto &renderInfo = it->second;
	auto &posCam = GetCamera()->GetPos();

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
