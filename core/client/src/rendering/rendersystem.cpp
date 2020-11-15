/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/render_queue.hpp"
#include "cmaterialmanager.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/c_baseentity.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/shaders/world/c_shader_wireframe.hpp"
#include "pragma/rendering/sortedrendermeshcontainer.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/renderers/rasterization/culled_mesh_data.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/render_stats.hpp"
#include "pragma/debug/renderdebuginfo.hpp"
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
#pragma optimize("",off)
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
void RenderSystem::Render(
	const util::DrawSceneInfo &drawSceneInfo,pragma::CCameraComponent &cam,RenderMode renderMode,
	RenderFlags flags,std::vector<std::unique_ptr<RenderSystem::TranslucentMesh>> &translucentMeshes,const Vector4 &drawOrigin
)
{
	if(translucentMeshes.empty())
		return;
	auto &scene = drawSceneInfo.scene;
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto bReflection = umath::is_flag_set(flags,RenderFlags::Reflection);
	auto renderAs3dSky = umath::is_flag_set(flags,RenderFlags::RenderAs3DSky);
	//auto &lights = scene->GetCulledLights();
	auto &rasterizer = *static_cast<const pragma::rendering::RasterizationRenderer*>(renderer);
	auto pipelineType = pragma::ShaderTextured3DBase::GetPipelineIndex(rasterizer.GetSampleCount(),bReflection);
	pragma::ShaderTextured3DBase *shaderPrev = nullptr;
	CBaseEntity *entPrev = nullptr;
	pragma::CRenderComponent *renderC = nullptr;
	std::optional<Vector4> clipPlane {};
	auto depthBiasActive = false;
	auto debugMode = scene->GetDebugMode();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	for(auto it=translucentMeshes.rbegin();it!=translucentMeshes.rend();++it) // Render back-to-front
	{
		auto &meshInfo = *it;
		auto &whShader = meshInfo->shader;
		if(whShader.expired())
			continue;
		auto *shader = static_cast<pragma::ShaderTextured3DBase*>(whShader.get());
		if(shader != shaderPrev)
		{
			if(shaderPrev != nullptr)
			{
				shaderPrev->EndDraw();
				shaderPrev = nullptr;
				entPrev = nullptr;
			}
			if(shader->BeginDraw(
				drawCmd,c_game->GetRenderClipPlane(),drawOrigin,
				pipelineType
			) == false)
				continue;
			if(debugMode != pragma::CSceneComponent::DebugMode::None)
				shader->SetDebugMode(debugMode);
			shader->Set3DSky(renderAs3dSky);
			shaderPrev = shader;
			if(shader->BindScene(*scene.get(),const_cast<pragma::rendering::RasterizationRenderer&>(rasterizer),renderMode == RenderMode::View) == false)
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
			// TODO: Enable/disable shadows
				entPrev = ent;
				renderC = entPrev->GetRenderComponent().get();
				if(shader->BindEntity(*meshInfo->ent) == false || renderC == nullptr || (drawSceneInfo.renderFilter && drawSceneInfo.renderFilter(*ent) == false))
				{
					renderC = nullptr;
					continue;
				}
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
				auto *entClipPlane = renderC->GetRenderClipPlane();
				clipPlane = entClipPlane ? *entClipPlane : std::optional<Vector4>{};
			}
			if(renderC == nullptr)
				continue;
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
				
				if(clipPlane.has_value())
					shader->BindClipPlane(*clipPlane);

				shader->Draw(*mesh);

				if(clipPlane.has_value())
					shader->BindClipPlane(c_game->GetRenderClipPlane());
			}
		}
	}
	if(shaderPrev != nullptr)
		shaderPrev->EndDraw();
}

RenderStats g_renderStats;
bool g_collectRenderStats = false;
static void print_pass_stats(const RenderPassStats &stats)
{
	Con::cout<<"\nEntities:"<<Con::endl;
	for(auto &hEnt : stats.entities)
	{
		if(hEnt.IsValid() == false)
			continue;
		hEnt.get()->print(Con::cout);
		Con::cout<<Con::endl;
	}

	Con::cout<<"\nMaterials:"<<Con::endl;
	for(auto &hMat : stats.materials)
	{
		if(hMat.IsValid() == false)
			continue;
		auto *albedoMap = hMat.get()->GetAlbedoMap();
		Con::cout<<hMat.get()->GetName();
		if(albedoMap)
			Con::cout<<" ["<<albedoMap->name<<"]";
		Con::cout<<Con::endl;
	}

	Con::cout<<"\nShaders:"<<Con::endl;
	for(auto &hShader : stats.shaders)
	{
		if(hShader.expired())
			continue;
		Con::cout<<hShader->GetIdentifier()<<Con::endl;
	}
	
	Con::cout<<"\nMeshes:"<<Con::endl;
	for(auto &mesh : stats.meshes)
	{
		Con::cout<<"Mesh["<<mesh->GetVertexCount()<<"]["<<mesh->GetTriangleCount()<<"]"<<Con::endl;
	}
}
DLLCLIENT void debug_render_stats()
{
	g_renderStats = {};
	g_collectRenderStats = true;
}
DLLCLIENT void print_debug_render_stats()
{
	g_collectRenderStats = false;
	Con::cout<<"Lighting pass:"<<Con::endl;
	print_pass_stats(g_renderStats.lightingPass);

	Con::cout<<"Depth prepass:"<<Con::endl;
	print_pass_stats(g_renderStats.prepass);

	Con::cout<<"Transparency pass:"<<Con::endl;
	print_pass_stats(g_renderStats.transparencyPass);
}

class RenderSys
{
public:
	enum class StateFlags : uint8_t
	{
		None = 0u,
		ShaderBound = 1u,
		MaterialBound = ShaderBound<<1u,
		EntityBound = MaterialBound<<1u
	};
	RenderSys(const util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode,RenderSystem::RenderFlags flags,const Vector4 &drawOrigin);
	~RenderSys();
	void Render(const pragma::rendering::RenderQueue &renderQueue,std::optional<uint32_t> worldRenderQueueIndex={});
	bool BindShader(prosper::Shader &shader);
	bool BindMaterial(CMaterial &mat);
	bool BindEntity(CBaseEntity &ent);
	bool Render(CModelSubMesh &mesh);
private:
	void UnbindShader();
	void UnbindMaterial();
	void UnbindEntity();
	prosper::ShaderIndex m_curShaderIndex = std::numeric_limits<prosper::ShaderIndex>::max();
	MaterialIndex m_curMaterialIndex = std::numeric_limits<MaterialIndex>::max();
	EntityIndex m_curEntityIndex = std::numeric_limits<EntityIndex>::max();

	prosper::Shader *m_curShader = nullptr;
	pragma::ShaderTextured3DBase *m_shaderScene = nullptr;
	CMaterial *m_curMaterial = nullptr;
	CBaseEntity *m_curEntity = nullptr;
	pragma::CRenderComponent *m_curRenderC = nullptr;
	std::vector<std::shared_ptr<ModelSubMesh>> *m_curEntityMeshList = nullptr;

	const util::DrawSceneInfo &m_drawSceneInfo;
	RenderMode m_renderMode;
	const Vector4 &m_drawOrigin;
	pragma::ShaderTextured3DBase::Pipeline m_pipelineType;
	RenderPassStats *m_stats = nullptr;
	const pragma::rendering::RasterizationRenderer *m_renderer = nullptr;
	RenderDebugInfo &m_debugInfo;
	RenderSystem::RenderFlags m_renderFlags;
	uint32_t m_numShaderInvocations = 0;
	StateFlags m_stateFlags = StateFlags::None;
};
REGISTER_BASIC_BITWISE_OPERATORS(RenderSys::StateFlags);

RenderSys::RenderSys(const util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode,RenderSystem::RenderFlags flags,const Vector4 &drawOrigin)
	: m_drawSceneInfo{drawSceneInfo},m_drawOrigin{drawOrigin},m_renderMode{renderMode},m_renderFlags{flags},m_debugInfo{get_render_debug_info()}
{
	auto &scene = drawSceneInfo.scene;
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto bReflection = umath::is_flag_set(flags,RenderSystem::RenderFlags::Reflection);
	m_renderer = static_cast<const pragma::rendering::RasterizationRenderer*>(renderer);
	m_pipelineType = pragma::ShaderTextured3DBase::GetPipelineIndex(m_renderer->GetSampleCount(),bReflection);
	m_stats = g_collectRenderStats ? &g_renderStats.lightingPass : nullptr;
}
RenderSys::~RenderSys()
{
	UnbindShader();
}

void RenderSys::Render(const pragma::rendering::RenderQueue &renderQueue,std::optional<uint32_t> worldRenderQueueIndex)
{
	if(m_renderer == nullptr)
		return;
	UnbindShader();
	auto &shaderManager = c_engine->GetShaderManager();
	auto &matManager = client->GetMaterialManager();
	uint32_t numShaderStateChanges = 0;
	uint32_t numMaterialStateChanges = 0;
	uint32_t numEntityStateChanges = 0;
	uint32_t numMeshes = 0;
	for(auto &itemSortPair : renderQueue.sortedItemIndices)
	{
		auto &item = renderQueue.queue.at(itemSortPair.first);
		
		if(worldRenderQueueIndex.has_value() && m_drawSceneInfo.scene->GetSceneRenderDesc().IsWorldMeshVisible(*worldRenderQueueIndex,item.mesh) == false)
			continue;

		if(item.shader != m_curShaderIndex)
		{
			auto *shader = shaderManager.GetShader(item.shader);
			assert(shader);
			BindShader(*shader);
			++numShaderStateChanges;
		}
		if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false)
			continue;
		if(item.material != m_curMaterialIndex)
		{
			auto *mat = matManager.GetMaterial(item.material);
			assert(mat);
			BindMaterial(static_cast<CMaterial&>(*mat));
			++numMaterialStateChanges;
		}
		if(umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound) == false)
			continue;
		if(item.entity != m_curEntityIndex)
		{
			auto *ent = c_game->GetEntityByLocalIndex(item.entity);
			assert(ent);
			BindEntity(static_cast<CBaseEntity&>(*ent));
			++numEntityStateChanges;
		}
		if(umath::is_flag_set(m_stateFlags,StateFlags::EntityBound) == false || item.mesh >= m_curEntityMeshList->size())
			continue;
		Render(static_cast<CModelSubMesh&>(*m_curEntityMeshList->at(item.mesh)));
		++numMeshes;
	}
	std::cout<<"";
}

void RenderSys::UnbindShader()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false)
		return;
	m_shaderScene->EndDraw();
	m_curShader = nullptr;
	m_curShaderIndex = std::numeric_limits<decltype(m_curShaderIndex)>::max();
	umath::set_flag(m_stateFlags,StateFlags::ShaderBound,false);
}

void RenderSys::UnbindMaterial()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound) == false)
		return;
	m_curMaterial = nullptr;
	m_curMaterialIndex = std::numeric_limits<decltype(m_curMaterialIndex)>::max();
	umath::set_flag(m_stateFlags,StateFlags::MaterialBound,false);
}

void RenderSys::UnbindEntity()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::EntityBound) == false)
		return;
	m_curEntity = nullptr;
	m_curEntityIndex = std::numeric_limits<decltype(m_curEntityIndex)>::max();
	umath::set_flag(m_stateFlags,StateFlags::EntityBound,false);
}

bool RenderSys::BindShader(prosper::Shader &shader)
{
	if(&shader == m_curShader)
		return umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound);
	UnbindShader();
	UnbindMaterial();
	UnbindEntity();
	m_curShader = &shader;
	auto bView = (m_renderMode == RenderMode::View) ? true : false;
	auto *shaderScene = dynamic_cast<pragma::ShaderTextured3DBase*>(&shader);
	if(shaderScene == nullptr)
		return false;
	if(shaderScene->BeginDraw(
		m_drawSceneInfo.commandBuffer,c_game->GetRenderClipPlane(),m_drawOrigin,
		m_pipelineType
	) == false)
		return false;
	if(m_stats)
		m_stats->shaders.push_back(shader.GetHandle());
	auto &scene = *m_drawSceneInfo.scene;
	if(shaderScene->BindScene(const_cast<pragma::CSceneComponent&>(scene),const_cast<pragma::rendering::RasterizationRenderer&>(*m_renderer),bView) == false)
		return false;
	auto debugMode = scene.GetDebugMode();
	if(debugMode != ::pragma::CSceneComponent::DebugMode::None)
		shaderScene->SetDebugMode(debugMode);
	shaderScene->Set3DSky(umath::is_flag_set(m_renderFlags,RenderSystem::RenderFlags::RenderAs3DSky));
	
	++m_debugInfo.shaderCount;
	umath::set_flag(m_stateFlags,StateFlags::ShaderBound);

	m_shaderScene = shaderScene;
	m_curShaderIndex = shader.GetIndex();
	return true;
}
bool RenderSys::BindMaterial(CMaterial &mat)
{
	if(&mat == m_curMaterial)
		return umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound);
	UnbindMaterial();
	// UnbindEntity();
	m_curMaterial = &mat;
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false || mat.IsInitialized() == false || m_shaderScene->BindMaterial(mat) == false)
		return false;
	if(m_stats)
		m_stats->materials.push_back(mat.GetHandle());
	++m_debugInfo.materialCount;
	
	umath::set_flag(m_stateFlags,StateFlags::MaterialBound);

	m_curMaterialIndex = mat.GetIndex();
	return true;
}
bool RenderSys::BindEntity(CBaseEntity &ent)
{
	if(&ent == m_curEntity)
		return umath::is_flag_set(m_stateFlags,StateFlags::EntityBound);
	UnbindEntity();
	m_curEntity = &ent;
	auto *renderC = ent.GetRenderComponent().get();
	if(umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound) == false || renderC == nullptr || m_shaderScene->BindEntity(ent) == false)
		return false;
	if(m_stats)
		m_stats->entities.push_back(ent.GetHandle());
	if(m_drawSceneInfo.renderFilter && m_drawSceneInfo.renderFilter(ent) == false)
		return false;
	
	m_curRenderC = renderC;
	m_curEntityMeshList = &renderC->GetRenderMeshes();
	auto *entClipPlane = m_curRenderC->GetRenderClipPlane();
	m_shaderScene->BindClipPlane(entClipPlane ? *entClipPlane : Vector4{});

	if(umath::is_flag_set(m_curRenderC->GetStateFlags(),pragma::CRenderComponent::StateFlags::HasDepthBias))
	{
		float constantFactor,biasClamp,slopeFactor;
		m_curRenderC->GetDepthBias(constantFactor,biasClamp,slopeFactor);
		m_drawSceneInfo.commandBuffer->RecordSetDepthBias(constantFactor,biasClamp,slopeFactor);
	}
	else
		m_drawSceneInfo.commandBuffer->RecordSetDepthBias();

	umath::set_flag(m_stateFlags,StateFlags::EntityBound);

	m_curEntityIndex = ent.GetLocalIndex();
	return true;
}

bool RenderSys::Render(CModelSubMesh &mesh)
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::EntityBound) == false || m_curRenderC == nullptr || m_curRenderC->Render(m_shaderScene,m_curMaterial,&mesh) == true)
		return false;
	++m_numShaderInvocations;

	auto &mdlComponent = m_curRenderC->GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	auto &vertAnimBuffer = static_cast<CModel&>(*mdl).GetVertexAnimationBuffer();
	auto bUseVertexAnim = false;
	if(vertAnimBuffer != nullptr)
	{
		auto pVertexAnimatedComponent = m_curEntity->GetComponent<pragma::CVertexAnimatedComponent>();
		if(pVertexAnimatedComponent.valid())
		{
			auto offset = 0u;
			auto animCount = 0u;
			if(pVertexAnimatedComponent->GetVertexAnimationBufferMeshOffset(mesh,offset,animCount) == true)
			{
				auto vaData = ((offset<<16)>>16) | animCount<<16;
				m_shaderScene->BindVertexAnimationOffset(vaData);
				bUseVertexAnim = true;
			}
		}
	}
	if(bUseVertexAnim == false)
		m_shaderScene->BindVertexAnimationOffset(0u);

	if(m_stats)
		m_stats->meshes.push_back(std::static_pointer_cast<CModelSubMesh>(mesh.shared_from_this()));
	m_shaderScene->Draw(mesh);

	auto numTriangles = mesh.GetTriangleCount();
	++m_debugInfo.meshCount;
	if(m_curEntity->IsStatic() == true)
		++m_debugInfo.staticMeshCount;
	m_debugInfo.triangleCount += numTriangles;
	m_debugInfo.vertexCount += mesh.GetVertexCount();
	return true;
}

static CVar cvDebugNormals = GetClientConVar("debug_render_normals");
uint32_t RenderSystem::Render(
	const util::DrawSceneInfo &drawSceneInfo,const pragma::rendering::CulledMeshData &renderMeshes,
	RenderMode renderMode,RenderFlags flags,const Vector4 &drawOrigin
)
{

	{
		RenderSys rsys {drawSceneInfo,renderMode,flags,drawOrigin};
		if(renderMode == RenderMode::World)
		{
			// For optimization purposes, world geometry is stored in separate render queues.
			// This could be less efficient if many models in the scene use the same materials as
			// the world, but this generally doesn't happen.
			// By rendering world geometry first, we can also avoid overdraw.
			// This does *not* include translucent geometry, which is instead copied over to the
			// general translucency world render queue.
			auto &worldRenderQueues = drawSceneInfo.scene->GetSceneRenderDesc().GetWorldRenderQueues();
			for(auto i=decltype(worldRenderQueues.size()){0u};i<worldRenderQueues.size();++i)
				rsys.Render(*worldRenderQueues.at(i),i);
		}
		auto *renderQueue = drawSceneInfo.scene->GetSceneRenderDesc().GetRenderQueue(renderMode,false /* translucent */);
		if(renderQueue)
		{
			rsys.Render(*renderQueue);
			// TODO: Get world queue
			/*auto *worldC = static_cast<pragma::CWorldComponent*>(c_game->GetWorld());
			if(worldC)
			{
				auto *node = worldC->GetBSPTree()->FindLeafNode(drawSceneInfo.scene->GetActiveCamera().get()->GetEntity().GetPosition());
				if(node)
				{
					auto *renderQueue = worldC->GetClusterRenderQueue(node->cluster);
					if(renderQueue == nullptr)
						worldC->BuildOptimizedLeafMeshTable();
					if(renderQueue)
					{
						RenderSys rsys {drawSceneInfo,renderMode,flags,drawOrigin};
						rsys.Render(*renderQueue);
					}
				}
			}*/
		}
		return 0;
	}

	auto &debugInfo = get_render_debug_info();
	auto &scene = drawSceneInfo.scene;
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return 0;
	auto bReflection = umath::is_flag_set(flags,RenderFlags::Reflection);
	auto renderAs3dSky = umath::is_flag_set(flags,RenderFlags::RenderAs3DSky);
	auto &rasterizer = *static_cast<const pragma::rendering::RasterizationRenderer*>(renderer);
	auto numShaderInvocations = 0u;

	auto &containers = renderMeshes.containers;
	auto &processed = renderMeshes.processed;

	auto pipelineType = pragma::ShaderTextured3DBase::GetPipelineIndex(rasterizer.GetSampleCount(),bReflection);
	//auto frameId = c_engine->GetRenderContext().GetLastFrameId();
	CBaseEntity *entLast = nullptr;
	std::optional<Vector4> clipPlane {};
	pragma::CRenderComponent *renderC = nullptr;
	pragma::ShaderTextured3DBase *shaderLast = nullptr;
	auto depthBiasActive = false;
	auto debugMode = scene->GetDebugMode();
	auto &drawCmd = drawSceneInfo.commandBuffer;
	auto *stats = g_collectRenderStats ? &g_renderStats.lightingPass : nullptr;
	for(auto itShader=containers.begin();itShader!=containers.end();itShader++)
	{
		auto &shaderContainer = *itShader;
		auto &whShader = shaderContainer->shader;
		auto *shader = static_cast<pragma::ShaderTextured3DBase*>(whShader.get());
		auto bView = (renderMode == RenderMode::View) ? true : false;
		if(shader->BeginDraw(
			drawCmd,c_game->GetRenderClipPlane(),drawOrigin,
			pipelineType
		) == true
			)
		{
			if(stats)
				stats->shaders.push_back(shader->GetHandle());
			if(shader->BindScene(*scene.get(),const_cast<pragma::rendering::RasterizationRenderer&>(rasterizer),bView) == true)
			{
				if(debugMode != ::pragma::CSceneComponent::DebugMode::None)
					shader->SetDebugMode(debugMode);
				shader->Set3DSky(renderAs3dSky);

				++debugInfo.shaderCount;
				for(auto itMat=shaderContainer->containers.begin();itMat!=shaderContainer->containers.end();itMat++)
				{
					auto &matContainer = *itMat;
					auto &mat = static_cast<CMaterial&>(*matContainer->material);
					if(mat.IsInitialized() && shader->BindMaterial(mat) == true)
					{
						if(stats)
							stats->materials.push_back(mat.GetHandle());
						++debugInfo.materialCount;
						for(auto &pair : matContainer->containers)
						{
							auto *ent = pair.first;
							if(ent != entLast || shader != shaderLast)
							{
								if(ent->IsWorld())
									continue;
								if(shader->BindEntity(*ent) == false)
									continue;
								if(stats)
									stats->entities.push_back(ent->GetHandle());
								entLast = ent;
								shaderLast = shader;
								renderC = entLast->GetRenderComponent().get();
								if(drawSceneInfo.renderFilter && drawSceneInfo.renderFilter(*ent) == false)
								{
									renderC = nullptr;
									continue;
								}

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
							for(auto *mesh : pair.second.meshes)
							{
#if DEBUG_RENDER_DISABLED == 0
								if(renderC && renderC->Render(shader,&mat,mesh) == false)
								{
									++numShaderInvocations;

									auto &mdlComponent = renderC->GetModelComponent();
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
									
									if(clipPlane.has_value())
										shader->BindClipPlane(*clipPlane);

									if(stats)
										stats->meshes.push_back(std::static_pointer_cast<CModelSubMesh>(mesh->shared_from_this()));
									shader->Draw(*mesh);

									auto numTriangles = mesh->GetTriangleCount();
									++debugInfo.meshCount;
									if(ent->IsStatic() == true)
										++debugInfo.staticMeshCount;
									debugInfo.triangleCount += numTriangles;
									debugInfo.vertexCount += mesh->GetVertexCount();
									
									if(clipPlane.has_value())
										shader->BindClipPlane(c_game->GetRenderClipPlane());
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

	for(auto it : processed)
		static_cast<CBaseEntity*>(it.first)->GetRenderComponent()->PostRender(renderMode);

	return numShaderInvocations;
}
uint32_t RenderSystem::Render(const util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode,RenderFlags flags,const Vector4 &drawOrigin)
{
	auto &scene = drawSceneInfo.scene;
	auto *renderInfo = scene->GetSceneRenderDesc().GetRenderInfo(renderMode);
	return renderInfo ? Render(drawSceneInfo,*renderInfo,renderMode,flags,drawOrigin) : 0;
}
#pragma optimize("",on)
