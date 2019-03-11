#include "stdafx_client.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include <pragma/model/animation/vertex_animation.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void RenderSystem::RenderPrepass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Camera &cam,std::vector<pragma::OcclusionMeshInfo> &renderMeshes,RenderMode renderMode,bool bReflection)
{
	auto &scene = c_game->GetRenderScene();
	auto *renderInfo = scene->GetRenderInfo(renderMode);
	if(renderInfo == nullptr)
		return;
	auto &containers = renderInfo->containers;
	//auto &descSetLightSources = scene->GetLightSourceDescriptorSet();
	// Render depth, positions and normals

	//shaderDepthStage.BindLights(lights,descSetShadowmps,descSetLightSources);
	CBaseEntity *entPrev = nullptr;
	auto &shaderDepthStage = scene->GetPrepass().GetShader();
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
					auto bWeighted = false;
					shaderDepthStage.BindEntity(*ent);//,bWeighted); // prosper TODO
				}

				for(auto *cmesh : pair.second.meshes)
				{
					auto &mdlComponent = ent->GetRenderComponent()->GetModelComponent();
					auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
					assert(mdl != nullptr);
					auto &vertAnimBuffer = static_cast<CModel&>(*mdl).GetVertexAnimationBuffer();
					auto bUseVertexAnim = false;
					if(vertAnimBuffer != nullptr)
					{
						// Vertex animation buffer barrier
						prosper::util::record_buffer_barrier(
							**drawCmd,*vertAnimBuffer,
							Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::VERTEX_SHADER_BIT,
							Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::SHADER_READ_BIT
						);

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
