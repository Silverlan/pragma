#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/opengl/renderobject.h"
#include "cmaterialmanager.h"
#include "pragma/rendering/scene/camera.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/c_baseentity.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/opengl/renderhierarchy.h"
#include "pragma/rendering/shaders/world/c_shader_wireframe.hpp"
#include "pragma/rendering/shaders/debug/c_shader_debug_normals.h"
#include "pragma/rendering/sortedrendermeshcontainer.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/debug/renderdebuginfo.hpp"
#include "pragma/rendering/uniformbinding.h"
#include "textureinfo.h"
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree_impl.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include <cmaterial.h>
#include <pragma/rendering/c_sci_gpu_timer_manager.hpp>
#include <pragma/model/animation/vertex_animation.hpp>
#include <prosper_util.hpp>
#include <image/prosper_render_target.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

// Disables rendering of meshes and shadows; For debug purposes only!
#define DEBUG_RENDER_DISABLED 0

RenderSystem::TranslucentMesh::TranslucentMesh(CBaseEntity *_ent,CModelSubMesh *_mesh,Material *_mat,::util::WeakHandle<prosper::Shader> shader,float _distance)
	: ent(_ent),mesh(_mesh),distance(_distance),material(_mat),shader(shader)
{}

RenderSystem::MaterialMeshContainer::MaterialMeshContainer(Material *mat)
	: material(mat)
{}

/*
static void FillLightContainerData(CBaseEntity *ent,ModelMesh *mesh,std::vector<std::unique_ptr<LightContainer>> &containers)
{
	if(ent->ShouldDrawShadow() && ent->GetRenderMode() == RenderMode::World) // TODO: Check ShouldDrawShadow in occlusion culling to avoid having to go through all meshes
	{
		auto &meshes = mesh->GetSubMeshes();
		auto *data = ent->GetLightData();
		for(unsigned int j=0;j<data->count;j++)
		{
			auto *hLight = data->hLight[j];
			if(hLight->IsValid())
			{
				auto *light = hLight->get();
				LightContainer *lightContainer = nullptr;
				for(auto itLight=containers.begin();itLight!=containers.end();++itLight)
				{
					auto &c = *itLight;
					if(c->light == light)
					{
						lightContainer = c.get();
						break;
					}
				}
				if(lightContainer == nullptr)
				{
					if(containers.size() == containers.capacity())
						containers.reserve(containers.capacity() +5);
					containers.push_back(std::make_unique<LightContainer>(light));
					lightContainer = containers.back().get();
				}
				EntityContainer *entContainer = nullptr;
				for(auto itEnt=lightContainer->containers.begin();itEnt!=lightContainer->containers.end();itEnt++)
				{
					auto &c = *itEnt;
					if(c->entity == ent)
					{
						entContainer = c.get();
						break;
					}
				}
				if(entContainer == nullptr)
				{
					if(lightContainer->containers.size() == lightContainer->containers.capacity())
						lightContainer->containers.reserve(lightContainer->containers.capacity() +10);
					lightContainer->containers.push_back(std::make_unique<EntityContainer>(ent));
					entContainer = lightContainer->containers.back().get();
				}
				entContainer->meshes.reserve(meshes.size());
				for(unsigned int k=0;k<meshes.size();k++)
					entContainer->meshes.push_back(static_cast<CModelSubMesh*>(meshes[k]));
			}
		}
	}
}
*/
/*
static void FillLightContainerData(CLightBase *light,std::vector<CullingMeshInfo> &meshes,std::vector<std::unique_ptr<LightContainer>> &containers)
{
	LightContainer *lightContainer = nullptr;
	for(auto itLight=containers.begin();itLight!=containers.end();++itLight)
	{
		auto &c = *itLight;
		if(c->light == light)
		{
			lightContainer = c.get();
			break;
		}
	}
	if(lightContainer == nullptr)
	{
		if(containers.size() == containers.capacity())
			containers.reserve(containers.capacity() +5);
		containers.push_back(std::make_unique<LightContainer>(light));
		lightContainer = containers.back().get();
	}
	for(auto it=meshes.begin();it!=meshes.end();++it)
	{
		auto &info = *it;
		if(info.hEntity.IsValid())
		{
			auto *ent = static_cast<CBaseEntity*>(info.hEntity.get());
			if(ent->ShouldDrawShadow()) // TODO: Check ShouldDrawShadow in occlusion culling to avoid having to go through all meshes
			{
				EntityContainer *entContainer = nullptr;
				for(auto itEnt=lightContainer->containers.begin();itEnt!=lightContainer->containers.end();itEnt++)
				{
					auto &c = *itEnt;
					if(c->entity == ent)
					{
						entContainer = c.get();
						break;
					}
				}
				if(entContainer == nullptr)
				{
					if(lightContainer->containers.size() == lightContainer->containers.capacity())
						lightContainer->containers.reserve(lightContainer->containers.capacity() +10);
					lightContainer->containers.push_back(std::make_unique<EntityContainer>(ent));
					entContainer = lightContainer->containers.back().get();
				}
				auto &subMeshes = info.mesh->GetSubMeshes();
				entContainer->meshes.reserve(entContainer->meshes.size() +subMeshes.size());
				for(auto it=subMeshes.begin();it!=subMeshes.end();++it)
					entContainer->meshes.push_back(static_cast<CModelSubMesh*>(*it));
			}
		}
	}
}
*/

#if DEBUG_SHADOWS == 1
DLLCLIENT uint32_t s_shadowMeshCount = 0;
DLLCLIENT uint32_t s_shadowIndexCount = 0;
DLLCLIENT uint32_t s_shadowTriangleCount = 0;
DLLCLIENT uint32_t s_shadowVertexCount = 0;
#endif
void RenderSystem::Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,Camera &cam,RenderMode renderMode,bool bReflection,std::vector<std::unique_ptr<RenderSystem::TranslucentMesh>> &translucentMeshes)
{
	if(translucentMeshes.empty())
		return;
	auto &scene = c_game->GetRenderScene();
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	//auto &lights = scene->GetCulledLights();
	auto &rasterizer = *static_cast<pragma::rendering::RasterizationRenderer*>(renderer);
	auto pipelineType = pragma::ShaderTextured3D::GetPipelineIndex(rasterizer.GetSampleCount(),bReflection);
	pragma::ShaderTextured3D *shaderPrev = nullptr;
	CBaseEntity *entPrev = nullptr;
	for(auto it=translucentMeshes.rbegin();it!=translucentMeshes.rend();++it) // Render back-to-front
	{
		auto &meshInfo = *it;
		auto &whShader = meshInfo->shader;
		if(whShader.expired())
			continue;
		auto *shader = static_cast<pragma::ShaderTextured3D*>(whShader.get());
		if(shader != shaderPrev)
		{
			if(shaderPrev != nullptr)
			{
				shaderPrev->EndDraw();
				shaderPrev = nullptr;
				entPrev = nullptr;
			}
			if(shader->BeginDraw(
				drawCmd,c_game->GetRenderClipPlane(),
				pipelineType
			) == false)
				continue;
			shaderPrev = shader;
			if(shader->BindScene(rasterizer,renderMode == RenderMode::View) == false)
			{
				shaderPrev = nullptr;
				continue;
			}
		}

		auto &mat = static_cast<CMaterial&>(*meshInfo->material);
		if(
			mat.IsInitialized() == true &&
			shader->BindMaterial(mat) == true
		)
		{
			auto *ent = meshInfo->ent;
			if(ent != entPrev)
			{
				entPrev = ent;
				if(shader->BindEntity(*meshInfo->ent) == false)
					continue;
			}
			auto *mesh = meshInfo->mesh;
			auto pRenderComponent = ent->GetRenderComponent();
			if(pRenderComponent.valid() && pRenderComponent->Render(shader,&mat,mesh) == false)
			{
				auto pVertAnimComponent = ent->GetComponent<pragma::CVertexAnimatedComponent>();
				auto &mdlComponent = pRenderComponent->GetModelComponent();
				auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
				auto &vertAnimBuffer = static_cast<CModel&>(*mdl).GetVertexAnimationBuffer();
				auto bUseVertexAnim = false;
				if(pVertAnimComponent.valid() && vertAnimBuffer != nullptr)
				{
					auto offset = 0u;
					auto animCount = 0u;
					if(pVertAnimComponent->GetVertexAnimationBufferMeshOffset(*mesh,offset,animCount) == true)
					{
						auto vaData = ((offset<<16)>>16) | animCount<<16;
						shader->BindVertexAnimationOffset(vaData);
						bUseVertexAnim = true;
					}
				}
				if(bUseVertexAnim == false)
					shader->BindVertexAnimationOffset(0u);

				shader->Draw(*mesh);
			}
		}
	}
	if(shaderPrev != nullptr)
		shaderPrev->EndDraw();
}

static CVar cvDebugNormals = GetClientConVar("debug_render_normals");
uint32_t RenderSystem::Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,RenderMode renderMode,bool bReflection)
{
	auto &debugInfo = get_render_debug_info();
	auto &scene = c_game->GetRenderScene();
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return 0;
	auto &rasterizer = *static_cast<pragma::rendering::RasterizationRenderer*>(renderer);
	auto numShaderInvocations = 0u;

	auto *renderInfo = rasterizer.GetRenderInfo(renderMode);
	if(renderInfo == nullptr)
		return numShaderInvocations;
	auto &containers = renderInfo->containers;
	auto &processed = renderInfo->processed;

	auto pipelineType = pragma::ShaderTextured3D::GetPipelineIndex(rasterizer.GetSampleCount(),bReflection);
	//auto frameId = c_engine->GetLastFrameId();
	CBaseEntity *entLast = nullptr;
	pragma::ShaderTextured3D *shaderLast = nullptr;
	for(auto itShader=containers.begin();itShader!=containers.end();itShader++)
	{
		auto &shaderContainer = *itShader;
		auto &whShader = shaderContainer->shader;
		auto *shader = static_cast<pragma::ShaderTextured3D*>(whShader.get());
		auto bView = (renderMode == RenderMode::View) ? true : false;
		if(shader->BeginDraw(
				drawCmd,c_game->GetRenderClipPlane(),
				pipelineType
			) == true
		)
		{
			if(shader->BindScene(rasterizer,bView) == true)
			{
				++debugInfo.shaderCount;
				for(auto itMat=shaderContainer->containers.begin();itMat!=shaderContainer->containers.end();itMat++)
				{
					auto &matContainer = *itMat;
					auto &mat = static_cast<CMaterial&>(*matContainer->material);
					if(mat.IsInitialized() && shader->BindMaterial(mat) == true)
					{
						++debugInfo.materialCount;
						for(auto &pair : matContainer->containers)
						{
							auto *ent = pair.first;
							if(ent != entLast || shader != shaderLast)
							{
								if(shader->BindEntity(*ent) == false)
									continue;
								entLast = ent;
								shaderLast = shader;
							}
							for(auto *mesh : pair.second.meshes)
							{
#if DEBUG_RENDER_DISABLED == 0
								auto pRenderComponent = ent->GetRenderComponent();
								if(pRenderComponent.valid() && pRenderComponent->Render(shader,&mat,mesh) == false)
								{
									++numShaderInvocations;

									auto &mdlComponent = pRenderComponent->GetModelComponent();
									auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
									auto &vertAnimBuffer = static_cast<CModel&>(*mdl).GetVertexAnimationBuffer();
									auto bUseVertexAnim = false;
									if(vertAnimBuffer != nullptr)
									{
										auto pVertexAnimatedComponent = ent->GetComponent<pragma::CVertexAnimatedComponent>();
										if(pVertexAnimatedComponent.valid())
										{
											auto offset = 0u;
											auto animCount = 0u;
											if(pVertexAnimatedComponent->GetVertexAnimationBufferMeshOffset(*mesh,offset,animCount) == true)
											{
												auto vaData = ((offset<<16)>>16) | animCount<<16;
												shader->BindVertexAnimationOffset(vaData);
												bUseVertexAnim = true;
											}
										}
									}
									if(bUseVertexAnim == false)
										shader->BindVertexAnimationOffset(0u);

									shader->Draw(*mesh);

									auto numTriangles = mesh->GetTriangleCount();
									++debugInfo.meshCount;
									if(ent->IsStatic() == true)
										++debugInfo.staticMeshCount;
									debugInfo.triangleCount += numTriangles;
									debugInfo.vertexCount += mesh->GetVertexCount();
								}
#endif
							}
						}
					}
				}
				shader->EndDraw();
				//shader->Unbind(); // Vulkan TODO
			}
		}
	}
	// Render normals if enabled
	/*static auto hShaderDebugNormals = c_engine->GetShader("debugnormals");
	if(hShaderDebugNormals.IsValid())
	{
		auto drawNormals = cvDebugNormals->GetInt();
		if(drawNormals > 0)
		{
			entLast = nullptr;
			auto normalRenderMode = static_cast<Shader::DebugNormals::RenderMode>(drawNormals -1);
			auto *shader = static_cast<Shader::DebugNormals*>(hShaderDebugNormals.get());
			if(shader->BeginDraw(normalRenderMode) == true)
			{
				shader->BindScene(*scene);
				for(auto itShader=containers.begin();itShader!=containers.end();itShader++)
				{
					auto &shaderContainer = *itShader;
					for(auto itMat=shaderContainer->containers.begin();itMat!=shaderContainer->containers.end();itMat++)
					{
						auto &matContainer = *itMat;
						for(auto itEnt=matContainer->containers.begin();itEnt!=matContainer->containers.end();itEnt++)
						{
							auto &entContainer = *itEnt;
							auto *ent = entContainer->entity;
							if(ent != entLast)
							{
								shader->BindEntity(ent);
								entLast = ent;
							}
							for(auto itMesh=entContainer->meshes.begin();itMesh!=entContainer->meshes.end();itMesh++)
							{
								auto *mesh = *itMesh;
								shader->Draw(mesh);
							}
						}
					}
				}
				shader->EndDraw();
			}
		}
	}*/ // prosper TODO
	//

	for(auto it : processed)
		static_cast<CBaseEntity*>(it.first)->GetRenderComponent()->PostRender(renderMode);

	//OpenGL::BindVertexArray(0); // Vulkan TODO
	//vao->Unbind();
	return numShaderInvocations;
}
