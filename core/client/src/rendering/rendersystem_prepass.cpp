/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/render_queue.hpp"
#include "pragma/rendering/render_stats.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include <sharedutils/alpha_mode.hpp>
#include <pragma/model/animation/vertex_animation.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_render_target.hpp>
#include <pragma/model/c_vertex_buffer_data.hpp>
#include <pragma/model/vk_mesh.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)

#include "pragma/rendering/render_processor.hpp"

pragma::rendering::DepthStageRenderProcessor::DepthStageRenderProcessor(const util::DrawSceneInfo &drawSceneInfo,RenderFlags flags,const Vector4 &drawOrigin)
	: BaseRenderProcessor{drawSceneInfo,flags,drawOrigin}
{
	SetCountNonOpaqueMaterialsOnly(true);
}
uint32_t pragma::rendering::DepthStageRenderProcessor::Render(const pragma::rendering::RenderQueue &renderQueue,RenderPassStats *optStats,std::optional<uint32_t> worldRenderQueueIndex)
{
	if(m_renderer == nullptr || umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false)
		return 0;
	m_stats = optStats;
	auto &matManager = client->GetMaterialManager();
	auto &sceneRenderDesc = m_drawSceneInfo.scene->GetSceneRenderDesc();
	uint32_t numShaderInvocations = 0;
	for(auto &itemSortPair : renderQueue.sortedItemIndices)
	{
		auto &item = renderQueue.queue.at(itemSortPair.first);
		
		if(worldRenderQueueIndex.has_value() && sceneRenderDesc.IsWorldMeshVisible(*worldRenderQueueIndex,item.mesh) == false)
			continue;
		if(item.material != m_curMaterialIndex)
		{
			auto *mat = matManager.GetMaterial(item.material);
			assert(mat);
			BindMaterial(static_cast<CMaterial&>(*mat));
		}
		if(umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound) == false)
			continue;
		if(item.entity != m_curEntityIndex)
		{
			auto *ent = c_game->GetEntityByLocalIndex(item.entity);
			assert(ent);
			BindEntity(static_cast<CBaseEntity&>(*ent));
		}
		if(umath::is_flag_set(m_stateFlags,StateFlags::EntityBound) == false || item.mesh >= m_curEntityMeshList->size())
			continue;
		auto &mesh = static_cast<CModelSubMesh&>(*m_curEntityMeshList->at(item.mesh));
		if(BaseRenderProcessor::Render(mesh))
			++numShaderInvocations;
	}
	return numShaderInvocations;
}

void RenderSystem::RenderPrepass(const util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode)
{
#if 0
	auto &scene = drawSceneInfo.scene;
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto *rasterizer = const_cast<pragma::rendering::RasterizationRenderer*>(static_cast<const pragma::rendering::RasterizationRenderer*>(renderer));
	auto &shaderPrepass = rasterizer->GetPrepassShader();
	// TODO: Bind shader?
	if(true)
	{
		if(renderMode != RenderMode::World)
			return; // TODO
		// TODO: Draw origin?
		DepthStageRenderProcessor rsys {drawSceneInfo,renderMode,RenderFlags::None,{} /* drawOrigin */};
		if(rsys.BindShader(shaderPrepass) == false)
			return;
		auto &worldRenderQueues = drawSceneInfo.scene->GetSceneRenderDesc().GetWorldRenderQueues();
		for(auto i=decltype(worldRenderQueues.size()){0u};i<worldRenderQueues.size();++i)
			rsys.Render(*worldRenderQueues.at(i),i);

		// Note: The non-translucent render queues also include transparent (alpha masked) objects
		auto *renderQueue = drawSceneInfo.scene->GetSceneRenderDesc().GetRenderQueue(renderMode,false /* translucent */);
		if(renderQueue)
			rsys.Render(*renderQueue);
		return;
	}
#endif
#if 0
	auto &scene = drawSceneInfo.scene;
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto *rasterizer = const_cast<pragma::rendering::RasterizationRenderer*>(static_cast<const pragma::rendering::RasterizationRenderer*>(renderer));
	auto &containers = renderMeshes.containers;
	//auto &descSetLightSources = scene->GetLightSourceDescriptorSet();
	// Render depth, positions and normals

	//shaderDepthStage.BindLights(lights,descSetShadowmps,descSetLightSources);
	CBaseEntity *entPrev = nullptr;
	pragma::CRenderComponent *renderC = nullptr;
	std::optional<Vector4> clipPlane {};
	auto depthBiasActive = false;
	auto &shaderDepthStage = rasterizer->GetPrepass().GetShader();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	for(auto &meshInfo : containers)
	{
		for(auto &matMeshInfo : meshInfo->containers)
		{
			for(auto &pair : matMeshInfo->containers)
			{
				auto *ent = pair.first;
				if(ent != entPrev)
				{
					entPrev = ent;
					renderC = entPrev->GetRenderComponent().get();
					if(renderC == nullptr || renderC->IsDepthPassEnabled() == false || (drawSceneInfo.prepassFilter && drawSceneInfo.prepassFilter(*ent) == false))
					{
						renderC = nullptr;
						continue;
					}
					auto bWeighted = false;
					shaderDepthStage.BindEntity(*ent);//,bWeighted); // prosper TODO

					auto *entClipPlane = renderC->GetRenderClipPlane();
					clipPlane = entClipPlane ? *entClipPlane : std::optional<Vector4>{};
				}
				if(renderC == nullptr)
					continue;
					if(umath::is_flag_set(renderC->GetStateFlags(),pragma::CRenderComponent::StateFlags::HasDepthBias))
					{
						float constantFactor,biasClamp,slopeFactor;
						renderC->GetDepthBias(constantFactor,biasClamp,slopeFactor);
						drawCmd->RecordSetDepthBias(constantFactor,biasClamp,slopeFactor);

						depthBiasActive = true;
					}
					else if(depthBiasActive)
					{
						// Clear depth bias
						depthBiasActive = false;
						drawCmd->RecordSetDepthBias();
					}
				for(auto *cmesh : pair.second.meshes)
				{
					if(cmesh->GetGeometryType() != ModelSubMesh::GeometryType::Triangles)
						continue;
					auto &mdlComponent = renderC->GetModelComponent();
					auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
					assert(mdl != nullptr);
					auto &vertAnimBuffer = static_cast<CModel&>(*mdl).GetVertexAnimationBuffer();
					auto bUseVertexAnim = false;
					if(vertAnimBuffer != nullptr)
					{
						auto pVertexAnimatedComponent = ent->GetComponent<pragma::CVertexAnimatedComponent>();
						if(pVertexAnimatedComponent.valid())
						{
							auto offset = 0u;
							auto animCount = 0u;
							if(pVertexAnimatedComponent->GetVertexAnimationBufferMeshOffset(*cmesh,offset,animCount) == true)
							{
								auto vaData = ((offset<<16)>>16) | animCount<<16;
								shaderDepthStage.BindVertexAnimationOffset(vaData);
								bUseVertexAnim = true;
							}
						}
					}
					if(bUseVertexAnim == false)
						shaderDepthStage.BindVertexAnimationOffset(0u);
					
					if(clipPlane.has_value())
						shaderDepthStage.BindClipPlane(*clipPlane);

					if(matMeshInfo->material && matMeshInfo->material->GetAlphaMode() == AlphaMode::Mask)
						shaderDepthStage.Draw(*cmesh,*matMeshInfo->material);
					else
						shaderDepthStage.Draw(*cmesh);


					if(clipPlane.has_value())
						shaderDepthStage.BindClipPlane(c_game->GetRenderClipPlane());
				}
			}
		}
	}
	//
#endif
}
#pragma optimize("",on)
