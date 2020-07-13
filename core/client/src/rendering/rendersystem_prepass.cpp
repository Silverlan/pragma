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
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include <pragma/model/animation/vertex_animation.hpp>
#include <prosper_command_buffer.hpp>
#include <image/prosper_render_target.hpp>
#include <pragma/model/c_vertex_buffer_data.hpp>
#include <pragma/model/vk_mesh.h>
#pragma optimize("",off)
extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void RenderSystem::RenderPrepass(const util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode)
{
	auto &scene = c_game->GetRenderScene();
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto *rasterizer = static_cast<pragma::rendering::RasterizationRenderer*>(renderer);
	auto *renderInfo = rasterizer->GetRenderInfo(renderMode);
	if(renderInfo == nullptr)
		return;
	RenderPrepass(drawSceneInfo,*renderInfo);
}
void RenderSystem::RenderPrepass(const util::DrawSceneInfo &drawSceneInfo,const pragma::rendering::CulledMeshData &renderMeshes)
{
	auto &scene = c_game->GetRenderScene();
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto *rasterizer = static_cast<pragma::rendering::RasterizationRenderer*>(renderer);
	auto &containers = renderMeshes.containers;
	//auto &descSetLightSources = scene->GetLightSourceDescriptorSet();
	// Render depth, positions and normals

	//shaderDepthStage.BindLights(lights,descSetShadowmps,descSetLightSources);
	CBaseEntity *entPrev = nullptr;
	pragma::CRenderComponent *renderC = nullptr;
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
				}
				if(renderC == nullptr)
					continue;
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

					//if(ent->Render(&shaderDepthStage,mat,cmesh) == false)
						shaderDepthStage.Draw(*cmesh);
				}
			}
		}
	}
	//
}
#pragma optimize("",on)
