#include "stdafx_client.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include <pragma/model/animation/vertex_animation.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
void RenderSystem::RenderPrepass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam,std::vector<pragma::OcclusionMeshInfo> &renderMeshes,RenderMode renderMode,bool bReflection)
{
	auto &scene = c_game->GetRenderScene();
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto *rasterizer = static_cast<pragma::rendering::RasterizationRenderer*>(renderer);
	auto *renderInfo = rasterizer->GetRenderInfo(renderMode);
	if(renderInfo == nullptr)
		return;
	auto &containers = renderInfo->containers;
	//auto &descSetLightSources = scene->GetLightSourceDescriptorSet();
	// Render depth, positions and normals

	//shaderDepthStage.BindLights(lights,descSetShadowmps,descSetLightSources);
	CBaseEntity *entPrev = nullptr;
	pragma::CRenderComponent *renderC = nullptr;
	auto depthBiasActive = false;
	auto &shaderDepthStage = rasterizer->GetPrepass().GetShader();
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
					auto bWeighted = false;
					shaderDepthStage.BindEntity(*ent);//,bWeighted); // prosper TODO

					if(umath::is_flag_set(renderC->GetStateFlags(),pragma::CRenderComponent::StateFlags::HasDepthBias))
					{
						float constantFactor,biasClamp,slopeFactor;
						renderC->GetDepthBias(constantFactor,biasClamp,slopeFactor);
						prosper::util::record_set_depth_bias(**drawCmd,constantFactor,biasClamp,slopeFactor);

						depthBiasActive = true;
					}
					else if(depthBiasActive)
					{
						// Clear depth bias
						depthBiasActive = false;
						prosper::util::record_set_depth_bias(**drawCmd);
					}
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

					//if(ent->Render(&shaderDepthStage,mat,cmesh) == false)
						shaderDepthStage.Draw(*cmesh);
				}
			}
		}
	}
	//
}
#pragma optimize("",on)
