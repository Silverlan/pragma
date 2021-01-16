/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/debug/renderdebuginfo.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/render_stats.hpp"
#include "pragma/model/vk_mesh.h"
#include "pragma/debug/debug_render_filter.hpp"
#include <sharedutils/magic_enum.hpp>
#include <prosper_framebuffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)
static bool g_collectRenderStats = false;
static CallbackHandle g_cbPreRenderScene = {};
static CallbackHandle g_cbPostRenderScene = {};
static std::unique_ptr<DebugRenderFilter> g_debugRenderFilter = nullptr;
void set_debug_render_filter(std::unique_ptr<DebugRenderFilter> filter)
{
	g_debugRenderFilter = std::move(filter);
}
static std::string nanoseconds_to_ms(std::chrono::nanoseconds t)
{
	return std::to_string(static_cast<long double>(t.count()) /static_cast<long double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds{1}).count())) +"ms";
}
static void print_pass_stats(const RenderPassStats &stats,bool full)
{
	auto *cam = c_game->GetRenderCamera();
	struct EntityData
	{
		EntityHandle hEntity {};
		float distance = 0.f;
	};
	std::vector<EntityData> entities;
	entities.reserve(stats.entities.size());
	for(auto &hEnt : stats.entities)
	{
		if(hEnt.IsValid() == false)
			continue;
		entities.push_back({});
		entities.back().hEntity = hEnt;
		if(cam == nullptr)
			continue;
		auto dist = uvec::distance(hEnt.get()->GetPosition(),cam->GetEntity().GetPosition());
		entities.back().distance = dist;
	}
	std::sort(entities.begin(),entities.end(),[](const EntityData &entData0,const EntityData &entData1) {
		return entData0.distance < entData1.distance;
	});

	Con::cout<<"\nEntities:";
	if(full == false)
		Con::cout<<" "<<entities.size()<<Con::endl;
	else
	{
		Con::cout<<Con::endl;
		for(auto &entData : entities)
		{
			auto &hEnt = entData.hEntity;
			if(hEnt.IsValid() == false)
				continue;
			uint32_t lod = 0;
			auto mdlC = hEnt.get()->GetComponent<pragma::CModelComponent>();
			if(mdlC.valid())
				lod = mdlC->GetLOD();
			hEnt.get()->print(Con::cout);
			Con::cout<<" (Distance: "<<entData.distance<<") (Lod: "<<lod<<")"<<Con::endl;
		}
	}
	
	Con::cout<<"\nMaterials:";
	if(full == false)
		Con::cout<<" "<<stats.materials.size()<<Con::endl;
	else
	{
		Con::cout<<Con::endl;
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
	}

	Con::cout<<"\nShaders:"<<Con::endl;
	for(auto &hShader : stats.shaders)
	{
		if(hShader.expired())
			continue;
		Con::cout<<hShader->GetIdentifier()<<Con::endl;
	}

	std::unordered_set<CBaseEntity*> uniqueEntities;
	for(auto &entData : entities)
		uniqueEntities.insert(static_cast<CBaseEntity*>(entData.hEntity.get()));
	
	Con::cout<<"\nUnique meshes: "<<stats.meshes.size()<<Con::endl;
	auto n = umath::to_integral(RenderPassStats::Counter::Count);
	for(auto i=decltype(n){0u};i<n;++i)
		Con::cout<<magic_enum::enum_name(static_cast<RenderPassStats::Counter>(i))<<": "<<stats->GetCount(static_cast<RenderPassStats::Counter>(i))<<Con::endl;

	n = umath::to_integral(RenderPassStats::Timer::Count);
	for(auto i=decltype(n){0u};i<n;++i)
		Con::cout<<magic_enum::enum_name(static_cast<RenderPassStats::Timer>(i))<<": "<<nanoseconds_to_ms(stats->GetTime(static_cast<RenderPassStats::Timer>(i)))<<Con::endl;
}
DLLCLIENT void print_debug_render_stats(const RenderStats &renderStats,bool full)
{
	g_collectRenderStats = false;
	
	auto n = umath::to_integral(RenderStats::RenderStage::Count);
	for(auto i=decltype(n){0u};i<n;++i)
		Con::cout<<magic_enum::enum_name(static_cast<RenderStats::RenderStage>(i))<<": "<<nanoseconds_to_ms(renderStats->GetTime(static_cast<RenderStats::RenderStage>(i)))<<Con::endl;

	/*auto t = renderStats.lightingPass.cpuExecutionTime +renderStats.lightingPassTranslucent.cpuExecutionTime +renderStats.prepass.cpuExecutionTime +renderStats.shadowPass.cpuExecutionTime;
	Con::cout<<"Total CPU Execution time: "<<nanoseconds_to_ms(t)<<Con::endl;
	Con::cout<<"Light culling time: "<<nanoseconds_to_ms(renderStats.lightCullingTime)<<Con::endl;
	Con::cout<<"Prepass execution time: "<<nanoseconds_to_ms(renderStats.prepassExecutionTime)<<Con::endl;
	Con::cout<<"Lighting pass execution time: "<<nanoseconds_to_ms(renderStats.lightingPassExecutionTime)<<Con::endl;
	Con::cout<<"Post processing execution time: "<<nanoseconds_to_ms(renderStats.postProcessingExecutionTime)<<Con::endl;
	Con::cout<<"Render buffer update time: "<<nanoseconds_to_ms(renderStats.updateRenderBufferTime)<<Con::endl;*/

	Con::cout<<"\n----- Render queue builder stats: -----"<<Con::endl;
	n = umath::to_integral(RenderQueueBuilderStats::Timer::Count);
	for(auto i=decltype(n){0u};i<n;++i)
		Con::cout<<magic_enum::enum_name(static_cast<RenderQueueBuilderStats::Timer>(i))<<": "<<nanoseconds_to_ms(renderStats.renderQueueBuilderStats->GetTime(static_cast<RenderQueueBuilderStats::Timer>(i)))<<Con::endl;

	Con::cout<<"Workers:"<<Con::endl;
	uint32_t workerId = 0;
	for(auto &workerStats : renderStats.renderQueueBuilderStats.workerStats)
		Con::cout<<"Worker #"<<workerId++<<": "<<workerStats.numJobs<<" jobs with total execution time of "<<nanoseconds_to_ms(workerStats.totalExecutionTime)<<Con::endl;

	n = umath::to_integral(RenderStats::RenderPass::Count);
	for(auto i=decltype(n){0u};i<n;++i)
	{
		Con::cout<<"\n----- "<<magic_enum::enum_name(static_cast<RenderStats::RenderPass>(i))<<": -----"<<Con::endl;
		print_pass_stats(renderStats.GetPassStats(static_cast<RenderStats::RenderPass>(i)),full);
	}
}
struct SceneStats
{
	std::shared_ptr<RenderStats> stats;
	std::string sceneName;
};
struct FrameRenderStats
{
	std::vector<SceneStats> stats;
	RenderStats GetAccumulated() const;
	void Print(bool full);
	bool Available() const
	{
		for(auto &renderStats : stats)
		{
			for(auto &pass : renderStats.stats->passes)
			{
				if(pass->IsGPUExecutionTimeAvailable() == false)
					return false;
			}
		}
		return true;
	}
};
RenderStats FrameRenderStats::GetAccumulated() const
{
	RenderStats accumulated {};
	for(auto &renderStats : stats)
	{
		auto &stats = *renderStats.stats;
		accumulated += stats;
	}
	return accumulated;
}
void FrameRenderStats::Print(bool full)
{
	Con::cout<<stats.size()<<" scenes have been rendered:"<<Con::endl;
	auto accumulated = GetAccumulated();
	for(auto &renderStats : stats)
	{
		util::set_console_color(util::ConsoleColorFlags::BackgroundGreen);
		Con::cout<<"########### "<<renderStats.sceneName<<": ###########"<<Con::endl;
		util::reset_console_color();
		print_debug_render_stats(*renderStats.stats,full);
	}

	if(stats.size() > 1)
	{
		util::set_console_color(util::ConsoleColorFlags::BackgroundGreen);
		Con::cout<<"\n----- Accumulated: -----"<<Con::endl;
		util::reset_console_color();
		print_debug_render_stats(accumulated,full);
	}
}
struct RenderStatsQueue
{
	std::queue<FrameRenderStats> frameStats;
};
DLLCLIENT void debug_render_stats(bool enabled,bool full,bool print,bool continuous)
{
	if(g_cbPreRenderScene.IsValid())
		g_cbPreRenderScene.Remove();
	if(g_cbPostRenderScene.IsValid())
		g_cbPostRenderScene.Remove();
	g_collectRenderStats = enabled;
	if(enabled == false)
		return;
	auto stats = std::make_shared<RenderStatsQueue>();
	auto first = true;
	g_cbPreRenderScene = c_game->AddCallback("OnRenderScenes",FunctionCallback<void>::Create([stats,first,full,print]() mutable {
		auto swapchainIdx = c_engine->GetRenderContext().GetLastAcquiredSwapchainImageIndex();
		while(stats->frameStats.empty() == false)
		{
			auto &frameStats = stats->frameStats.front();
			if(frameStats.Available() == false)
				break;
			if(print)
				frameStats.Print(full);

			auto *l = c_game->GetLuaState();
			auto t = luabind::newtable(l);
			auto tTimes = luabind::newtable(l);
			tTimes["gui"] = c_engine->GetGpuExecutionTime(swapchainIdx,CEngine::GPUTimer::GUI).count() /static_cast<long double>(1'000'000.0);
			tTimes["scene"] = c_engine->GetGpuExecutionTime(swapchainIdx,CEngine::GPUTimer::Scene).count() /static_cast<long double>(1'000'000.0);
			tTimes["frame"] = c_engine->GetGpuExecutionTime(swapchainIdx,CEngine::GPUTimer::Frame).count() /static_cast<long double>(1'000'000.0);
			tTimes["present"] = c_engine->GetGpuExecutionTime(swapchainIdx,CEngine::GPUTimer::Present).count() /static_cast<long double>(1'000'000.0);
			t["numberOfScenes"] = frameStats.stats.size();
			t["times"] = tTimes;
			auto tStats = luabind::newtable(l);
			t["stats"] = tStats;
			int32_t i = 1;
			for(auto &stats : frameStats.stats)
			{
				auto td = luabind::newtable(l);
				td["stats"] = stats.stats.get();
				td["scene"] = stats.sceneName;
				tStats[i++] = td;
			}
			RenderStats accumulated;
			if(frameStats.stats.size() > 1)
			{
				accumulated = frameStats.GetAccumulated();
				auto td = luabind::newtable(l);
				td["stats"] = &accumulated;
				td["scene"] = "accumulated";
				tStats[i++] = td;
			}
			c_game->CallLuaCallbacks<void,luabind::object>("OnFrameRenderStatsAvailable",t);

			stats->frameStats.pop();
		}

		if(print && !first)
		{
			if(stats->frameStats.empty())
			{
				if(g_cbPreRenderScene.IsValid())
					g_cbPreRenderScene.Remove();
				if(g_cbPostRenderScene.IsValid())
					g_cbPostRenderScene.Remove();
			}
			return;
		}
		first = false;
		for(auto &drawSceneInfo : c_game->GetQueuedRenderScenes())
		{
			drawSceneInfo.renderStats = std::make_unique<RenderStats>();
			drawSceneInfo.renderStats->swapchainImageIndex = swapchainIdx;
		}
	}));
	g_cbPostRenderScene = c_game->AddCallback("PostRenderScenes",FunctionCallback<void>::Create([full,stats]() {
		auto &renderScenes = c_game->GetQueuedRenderScenes();
		FrameRenderStats frameStats {};
		for(auto &drawSceneInfo : renderScenes)
		{
			if(drawSceneInfo.renderStats == nullptr || drawSceneInfo.scene.expired())
				continue;
			SceneStats sceneStats {};
			std::stringstream ss;
			const_cast<BaseEntity&>(drawSceneInfo.scene->GetEntity()).print(ss);
			sceneStats.sceneName = ss.str();
			sceneStats.stats = std::move(drawSceneInfo.renderStats);
			frameStats.stats.push_back(sceneStats);
		}
		stats->frameStats.push(frameStats);
	}));
}

pragma::rendering::BaseRenderProcessor::BaseRenderProcessor(const util::RenderPassDrawInfo &drawSceneInfo,RenderFlags flags,const Vector4 &drawOrigin)
	: m_drawSceneInfo{drawSceneInfo},m_drawOrigin{drawOrigin},m_renderFlags{flags},m_shaderProcessor{*drawSceneInfo.commandBuffer}
{
	auto &scene = drawSceneInfo.drawSceneInfo.scene;
	auto *renderer = scene->GetRenderer();
	if(renderer == nullptr || renderer->IsRasterizationRenderer() == false)
		return;
	auto bReflection = umath::is_flag_set(flags,RenderFlags::Reflection);
	m_renderer = static_cast<const pragma::rendering::RasterizationRenderer*>(renderer);
	m_pipelineType = pragma::ShaderTextured3DBase::GetPipelineIndex(m_renderer->GetSampleCount(),bReflection);
}
pragma::rendering::BaseRenderProcessor::~BaseRenderProcessor()
{
	UnbindShader();
}

void pragma::rendering::BaseRenderProcessor::SetCountNonOpaqueMaterialsOnly(bool b) {umath::set_flag(m_stateFlags,StateFlags::CountNonOpaqueMaterialsOnly,b);}

void pragma::rendering::BaseRenderProcessor::UnbindShader()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false)
		return;
	//m_shaderScene->EndDraw();
	m_drawSceneInfo.commandBuffer->RecordUnbindShaderPipeline();
	m_curShader = nullptr;
	m_curShaderIndex = std::numeric_limits<decltype(m_curShaderIndex)>::max();
	m_curInstanceSet = nullptr;
	umath::set_flag(m_stateFlags,StateFlags::ShaderBound,false);
}

void pragma::rendering::BaseRenderProcessor::UnbindMaterial()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound) == false)
		return;
	m_curMaterial = nullptr;
	m_curMaterialIndex = std::numeric_limits<decltype(m_curMaterialIndex)>::max();
	umath::set_flag(m_stateFlags,StateFlags::MaterialBound,false);
}

void pragma::rendering::BaseRenderProcessor::UnbindEntity()
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::EntityBound) == false)
		return;
	m_curEntity = nullptr;
	m_curEntityIndex = std::numeric_limits<decltype(m_curEntityIndex)>::max();
	umath::set_flag(m_stateFlags,StateFlags::EntityBound,false);
}

bool pragma::rendering::BaseRenderProcessor::BindInstanceSet(pragma::ShaderGameWorld &shaderScene,const RenderQueue::InstanceSet *instanceSet)
{
	if(instanceSet == m_curInstanceSet)
		return true;
	m_curInstanceSet = instanceSet;
	return true;
}

bool pragma::rendering::BaseRenderProcessor::BindShader(prosper::Shader &shader)
{
	if(&shader == m_curShader)
		return umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound);
	UnbindShader();
	UnbindMaterial();
	UnbindEntity();
	m_curShader = &shader;
	if(shader.GetBaseTypeHashCode() != pragma::ShaderGameWorld::HASH_TYPE || shader.IsValid() == false)
		return false;
	auto *shaderScene = static_cast<pragma::ShaderGameWorld*>(&shader);
	if((g_debugRenderFilter && g_debugRenderFilter->shaderFilter && g_debugRenderFilter->shaderFilter(*shaderScene) == false))
		return false;
	
	auto &scene = *m_drawSceneInfo.drawSceneInfo.scene;
	auto bView = (m_camType == CameraType::View) ? true : false;
	m_shaderProcessor.RecordBindShader(scene,static_cast<const pragma::rendering::RasterizationRenderer&>(*scene.GetRenderer()),bView,*shaderScene,umath::to_integral(m_pipelineType));
	
	if(m_stats)
	{
		(*m_stats)->Increment(RenderPassStats::Counter::ShaderStateChanges);
		m_stats->shaders.push_back(shader.GetHandle());
	}
	umath::set_flag(m_stateFlags,StateFlags::ShaderBound);

	m_shaderScene = shaderScene;
	m_curShaderIndex = shader.GetIndex();
	return true;
}
void pragma::rendering::BaseRenderProcessor::SetCameraType(CameraType camType)
{
	m_camType = camType;
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false || m_shaderScene == nullptr)
		return;
	auto &scene = *m_drawSceneInfo.drawSceneInfo.scene.get();
	auto *renderer = scene.GetRenderer();
	if(renderer == nullptr)
		return;
	//m_shaderScene->BindSceneCamera(scene,*static_cast<pragma::rendering::RasterizationRenderer*>(renderer),camType == CameraType::View);
}
void pragma::rendering::BaseRenderProcessor::Set3DSky(bool enabled)
{
	umath::set_flag(m_renderFlags,RenderFlags::RenderAs3DSky,enabled);
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false || m_shaderScene == nullptr)
		return;
	//m_shaderScene->Set3DSky(enabled);
}
void pragma::rendering::BaseRenderProcessor::SetDrawOrigin(const Vector4 &drawOrigin)
{
	m_drawOrigin = drawOrigin;
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false || m_shaderScene == nullptr)
		return;
	//m_shaderScene->BindDrawOrigin(drawOrigin);
}
void pragma::rendering::BaseRenderProcessor::SetDepthBias(float d,float delta)
{
	m_depthBias = (d > 0.f && delta > 0.f) ? Vector2{d,delta} : std::optional<Vector2>{};
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false || m_shaderScene == nullptr)
		return;
	//m_shaderScene->SetDepthBias(m_depthBias.has_value() ? *m_depthBias : Vector2{});
}
bool pragma::rendering::BaseRenderProcessor::BindMaterial(CMaterial &mat)
{
	if(&mat == m_curMaterial)
		return umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound);
	UnbindMaterial();
	m_curMaterial = &mat;
	if(
		umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false || mat.IsInitialized() == false || 
		(g_debugRenderFilter && g_debugRenderFilter->materialFilter && g_debugRenderFilter->materialFilter(mat) == false) || 
		m_shaderProcessor.RecordBindMaterial(mat) == false
		//m_shaderScene->BindMaterial(mat) == false
	)
		return false;

	if(m_stats)
	{
		if(umath::is_flag_set(m_stateFlags,StateFlags::CountNonOpaqueMaterialsOnly) == false || mat.GetAlphaMode() != AlphaMode::Opaque)
		{
			(*m_stats)->Increment(RenderPassStats::Counter::MaterialStateChanges);
			m_stats->materials.push_back(mat.GetHandle());
		}
	}
	umath::set_flag(m_stateFlags,StateFlags::MaterialBound);

	m_curMaterialIndex = mat.GetIndex();
	return true;
}
bool pragma::rendering::BaseRenderProcessor::BindEntity(CBaseEntity &ent)
{
	if(&ent == m_curEntity)
		return umath::is_flag_set(m_stateFlags,StateFlags::EntityBound);
	UnbindEntity();
	m_curEntity = &ent;
	auto *renderC = ent.GetRenderComponent();
	if(umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound) == false || renderC == nullptr || (g_debugRenderFilter && g_debugRenderFilter->entityFilter && g_debugRenderFilter->entityFilter(ent,*m_curMaterial) == false))
		return false;
	// if(m_stats && umath::is_flag_set(renderC->GetStateFlags(),CRenderComponent::StateFlags::RenderBufferDirty))
	// 	++m_stats->numEntityBufferUpdates;
	// renderC->UpdateRenderBuffers(m_drawSceneInfo.commandBuffer);
	//if(m_shaderScene->BindEntity(ent) == false)
	//	return false;
	if(m_shaderProcessor.RecordBindEntity(ent) == false)
		return false;
	if(m_drawSceneInfo.drawSceneInfo.renderFilter && m_drawSceneInfo.drawSceneInfo.renderFilter(ent) == false)
		return false;
	
	m_curRenderC = renderC;
	m_curEntityMeshList = &renderC->GetRenderMeshes();
	//auto *entClipPlane = m_curRenderC->GetRenderClipPlane();
	//m_shaderScene->BindClipPlane(entClipPlane ? *entClipPlane : Vector4{});

	/*if(umath::is_flag_set(m_curRenderC->GetStateFlags(),pragma::CRenderComponent::StateFlags::HasDepthBias))
	{
		float constantFactor,biasClamp,slopeFactor;
		m_curRenderC->GetDepthBias(constantFactor,biasClamp,slopeFactor);
		m_drawSceneInfo.commandBuffer->RecordSetDepthBias(constantFactor,biasClamp,slopeFactor);
	}
	else*/
		m_drawSceneInfo.commandBuffer->RecordSetDepthBias();
	
	if(m_stats)
	{
		(*m_stats)->Increment(RenderPassStats::Counter::EntityStateChanges);
		m_stats->entities.push_back(ent.GetHandle());
	}
	umath::set_flag(m_stateFlags,StateFlags::EntityBound);

	m_curEntityIndex = ent.GetLocalIndex();
	return true;
}

pragma::ShaderGameWorld *pragma::rendering::BaseRenderProcessor::GetCurrentShader()
{
	return umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) ? m_shaderScene : nullptr;
}

bool pragma::rendering::BaseRenderProcessor::Render(CModelSubMesh &mesh,pragma::RenderMeshIndex meshIdx,const RenderQueue::InstanceSet *instanceSet)
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::EntityBound) == false || m_curRenderC == nullptr || (g_debugRenderFilter && g_debugRenderFilter->meshFilter && g_debugRenderFilter->meshFilter(*m_curEntity,m_curMaterial,mesh,meshIdx) == false))
		return false;
	++m_numShaderInvocations;
	
	return m_shaderProcessor.RecordDraw(mesh,meshIdx,instanceSet);
#if 0
	auto bUseVertexAnim = false;
	auto *mdlComponent = m_curRenderC->GetModelComponent();
	if(mdlComponent)
	{
		auto &vertAnimBuffer = static_cast<CModel&>(*mdlComponent->GetModel()).GetVertexAnimationBuffer();
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
	}
	if(bUseVertexAnim == false)
		m_shaderScene->BindVertexAnimationOffset(0u);

	BindInstanceSet(*m_shaderScene,instanceSet);
	auto instanceCount = instanceSet ? instanceSet->instanceCount : 1;
	if(m_stats)
	{
		m_stats->numDrawnMeshes += instanceCount;
		m_stats->numDrawnVertices += mesh.GetVertexCount() *instanceCount;
		m_stats->numDrawnTrianges += mesh.GetTriangleCount() *instanceCount;
		m_stats->meshes.push_back(std::static_pointer_cast<CModelSubMesh>(mesh.shared_from_this()));

		++m_stats->numDrawCalls;
	}
	auto instanceBuffer = m_curInstanceSet ? m_curInstanceSet->instanceBuffer : CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer();
	m_shaderScene->Draw(mesh,meshIdx,*instanceBuffer,instanceCount);
	return true;
#endif
}

prosper::Extent2D pragma::rendering::BaseRenderProcessor::GetExtents() const
{
	prosper::Extent2D extents;
	if(m_drawSceneInfo.commandBuffer->IsPrimary())
	{
		prosper::IImage *img;
		if(m_drawSceneInfo.commandBuffer->GetPrimaryCommandBufferPtr()->GetActiveRenderPassTarget(nullptr,&img) == false || img == nullptr)
			return false;
		extents = img->GetExtents();
	}
	else
	{
		auto *fb = m_drawSceneInfo.commandBuffer->GetSecondaryCommandBufferPtr()->GetCurrentFramebuffer();
		if(fb == nullptr)
			return false;
		extents = {fb->GetWidth(),fb->GetHeight()};
	}
	return extents;
}

void pragma::rendering::BaseRenderProcessor::RecordViewport()
{
	auto extents = GetExtents();
	m_drawSceneInfo.commandBuffer->RecordSetViewport(extents.width,extents.height,0,0,0.f,1.f);
	m_drawSceneInfo.commandBuffer->RecordSetScissor(extents.width,extents.height);
}

uint32_t pragma::rendering::BaseRenderProcessor::Render(const pragma::rendering::RenderQueue &renderQueue,bool prepass,RenderPassStats *optStats,std::optional<uint32_t> worldRenderQueueIndex)
{
	std::chrono::steady_clock::time_point t;
	if(optStats)
		t = std::chrono::steady_clock::now();

	renderQueue.WaitForCompletion(optStats);
	if(m_renderer == nullptr || (prepass && umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false))
		return 0;
	m_stats = optStats;
	m_shaderProcessor.SetStats(m_stats);
	if(!prepass)
		UnbindShader();

	auto &scene = *m_drawSceneInfo.drawSceneInfo.scene;
	auto &referenceShader = prepass ? c_game->GetGameShader(CGame::GameShader::Prepass) : c_game->GetGameShader(CGame::GameShader::Pbr);
	auto view = (m_camType == CameraType::View) ? true : false;
	if(referenceShader.expired())
		return 0;
	RecordViewport();
	
	auto &shaderManager = c_engine->GetShaderManager();
	auto &matManager = client->GetMaterialManager();
	auto &sceneRenderDesc = m_drawSceneInfo.drawSceneInfo.scene->GetSceneRenderDesc();
	uint32_t numShaderInvocations = 0;
	const RenderQueue::InstanceSet *curInstanceSet = nullptr;
	auto inInstancedEntityGroup = false;
	std::chrono::steady_clock::time_point ttmp;
	for(auto i=decltype(renderQueue.sortedItemIndices.size()){0u};i<renderQueue.sortedItemIndices.size();++i)
	{
		auto &itemSortPair = renderQueue.sortedItemIndices[i];
		auto &item = renderQueue.queue.at(itemSortPair.first);
		auto newInstance = false;
		if(item.instanceSetIndex == RenderQueueItem::INSTANCED)
		{
			// We've already covered this item through instancing,
			// we can skip this and all other instanced items for this set
			assert(curInstanceSet);
			i = static_cast<size_t>(curInstanceSet->startSkipIndex) +curInstanceSet->GetSkipCount() -1; // -1, since it'll get incremented again with the next iteration
			curInstanceSet = nullptr;
			inInstancedEntityGroup = false;
			continue;
		}
		else if(item.instanceSetIndex != RenderQueueItem::UNIQUE && curInstanceSet == nullptr)
		{
			curInstanceSet = &renderQueue.instanceSets[item.instanceSetIndex];
			newInstance = true;
		}
		if(worldRenderQueueIndex.has_value() && sceneRenderDesc.IsWorldMeshVisible(*worldRenderQueueIndex,item.mesh) == false)
			continue;
		if(!prepass)
		{
			if(item.shader != m_curShaderIndex)
			{
				if(optStats)
					ttmp = std::chrono::steady_clock::now();
				auto *shader = shaderManager.GetShader(item.shader);
				assert(shader);
				BindShader(*shader);
				if(optStats)
					(*optStats)->AddTime(RenderPassStats::Timer::ShaderBind,std::chrono::steady_clock::now() -ttmp);
			}
			if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false)
				continue;
		}
		if(item.material != m_curMaterialIndex)
		{
			if(optStats)
				ttmp = std::chrono::steady_clock::now();
			auto *mat = matManager.GetMaterial(item.material);
			assert(mat);
			BindMaterial(static_cast<CMaterial&>(*mat));
			if(optStats)
				(*optStats)->AddTime(RenderPassStats::Timer::MaterialBind,std::chrono::steady_clock::now() -ttmp);
		}
		if(umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound) == false)
			continue;
		if(item.entity != m_curEntityIndex)
		{
			if(optStats)
				ttmp = std::chrono::steady_clock::now();
			// During instanced rendering, the entity index may flip between the mesh instances (because entity indices are *not* included in the sort key),
			// but we don't really care about which of the entity is bound, as long as *one of them*
			// is bound. That means if we have already bound one, we can skip this block.
			if(inInstancedEntityGroup == false)
			{
				auto *ent = c_game->GetEntityByLocalIndex(item.entity);
				assert(ent);
				// TODO: If we're instancing, there's technically no need to bind
				// the entity (except for resetting the clip plane, etc.)
				BindEntity(static_cast<CBaseEntity&>(*ent));
				if(m_stats && umath::is_flag_set(m_stateFlags,StateFlags::EntityBound))
				{
					if(item.instanceSetIndex == RenderQueueItem::UNIQUE)
						(*m_stats)->Increment(RenderPassStats::Counter::EntitiesWithoutInstancing);
				}
				if(newInstance)
					inInstancedEntityGroup = true;
			}
			if(optStats)
				(*optStats)->AddTime(RenderPassStats::Timer::EntityBind,std::chrono::steady_clock::now() -ttmp);
		}
		if(umath::is_flag_set(m_stateFlags,StateFlags::EntityBound) == false || item.mesh >= m_curEntityMeshList->size())
			continue;
		if(m_stats && curInstanceSet)
		{
			(*m_stats)->Increment(RenderPassStats::Counter::InstanceSets);
			if(newInstance)
			{
				(*m_stats)->Increment(RenderPassStats::Counter::InstanceSetMeshes,curInstanceSet->meshCount);
				(*m_stats)->Increment(RenderPassStats::Counter::InstancedMeshes,curInstanceSet->meshCount *curInstanceSet->instanceCount);
				(*m_stats)->Increment(RenderPassStats::Counter::InstancedSkippedRenderItems,curInstanceSet->GetSkipCount() -curInstanceSet->meshCount);
			}
		}
		if(optStats)
			ttmp = std::chrono::steady_clock::now();
		auto &mesh = static_cast<CModelSubMesh&>(*m_curEntityMeshList->at(item.mesh));
		if(BaseRenderProcessor::Render(mesh,item.mesh,curInstanceSet))
			++numShaderInvocations;
		if(optStats)
			(*optStats)->AddTime(RenderPassStats::Timer::DrawCall,std::chrono::steady_clock::now() -ttmp);
	}
	if(optStats)
	{
		(*optStats)->AddTime(RenderPassStats::Timer::CpuExecution,std::chrono::steady_clock::now() -t);

		for(auto &item : renderQueue.queue)
		{
			if(item.instanceSetIndex == RenderQueueItem::UNIQUE)
				continue;
			optStats->instancedEntities.insert(item.entity);
		}
	}
	return numShaderInvocations;
}
#pragma optimize("",on)
