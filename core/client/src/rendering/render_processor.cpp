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
#include "pragma/rendering/render_stats.hpp"

extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)
static bool g_collectRenderStats = false;
static CallbackHandle g_cbPreRenderScene = {};
static CallbackHandle g_cbPostRenderScene = {};
static void print_pass_stats(const RenderPassStats &stats)
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

	Con::cout<<"\nEntities:"<<Con::endl;
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
	
	Con::cout<<"\nMeshes: "<<stats.meshes.size()<<Con::endl;
	Con::cout<<"Shader state changes: "<<stats.numShaderStateChanges<<Con::endl;
	Con::cout<<"Material state changes: "<<stats.numMaterialStateChanges<<Con::endl;
	Con::cout<<"Entity state changes: "<<stats.numEntityStateChanges<<Con::endl;
	Con::cout<<"Number of meshes drawn: "<<stats.numDrawnMeshes<<Con::endl;
	Con::cout<<"Number of vertices drawn: "<<stats.numDrawnVertices<<Con::endl;
	Con::cout<<"Number of triangles drawn: "<<stats.numDrawnTrianges<<Con::endl;
}
DLLCLIENT void print_debug_render_stats(const RenderStats &renderStats)
{
	g_collectRenderStats = false;
	Con::cout<<"----- Lighting pass: -----"<<Con::endl;
	print_pass_stats(renderStats.lightingPass);

	Con::cout<<"\n----- Lighting translucent pass: -----"<<Con::endl;
	print_pass_stats(renderStats.lightingPassTranslucent);

	Con::cout<<"\n----- Depth prepass: -----"<<Con::endl;
	print_pass_stats(renderStats.prepass);
}
DLLCLIENT void debug_render_stats()
{
	g_collectRenderStats = true;
	g_cbPreRenderScene = c_game->AddCallback("PreRenderScene",FunctionCallback<void,std::reference_wrapper<const util::DrawSceneInfo>>::Create([](std::reference_wrapper<const util::DrawSceneInfo> drawSceneInfo) {
		drawSceneInfo.get().renderStats = RenderStats{};
	}));
	g_cbPostRenderScene = c_game->AddCallback("PostRenderScene",FunctionCallback<void,std::reference_wrapper<const util::DrawSceneInfo>>::Create([](std::reference_wrapper<const util::DrawSceneInfo> drawSceneInfo) {
		if(drawSceneInfo.get().renderStats.has_value())
			print_debug_render_stats(*drawSceneInfo.get().renderStats);
		if(g_cbPreRenderScene.IsValid())
			g_cbPreRenderScene.Remove();
		if(g_cbPostRenderScene.IsValid())
			g_cbPostRenderScene.Remove();
	}));
}

pragma::rendering::BaseRenderProcessor::BaseRenderProcessor(const util::DrawSceneInfo &drawSceneInfo,RenderFlags flags,const Vector4 &drawOrigin)
	: m_drawSceneInfo{drawSceneInfo},m_drawOrigin{drawOrigin},m_renderFlags{flags}
{
	auto &scene = drawSceneInfo.scene;
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
	m_shaderScene->EndDraw();
	m_curShader = nullptr;
	m_curShaderIndex = std::numeric_limits<decltype(m_curShaderIndex)>::max();
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

bool pragma::rendering::BaseRenderProcessor::BindShader(prosper::Shader &shader)
{
	if(&shader == m_curShader)
		return umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound);
	UnbindShader();
	UnbindMaterial();
	UnbindEntity();
	m_curShader = &shader;
	auto *shaderScene = dynamic_cast<pragma::ShaderGameWorld*>(&shader);
	if(shaderScene == nullptr)
		return false;
	if(shaderScene->BeginDraw(
		m_drawSceneInfo.commandBuffer,c_game->GetRenderClipPlane(),m_drawOrigin,
		m_pipelineType
	) == false)
		return false;
	auto &scene = *m_drawSceneInfo.scene;
	auto bView = (m_camType == CameraType::View) ? true : false;
	if(shaderScene->BindScene(const_cast<pragma::CSceneComponent&>(scene),const_cast<pragma::rendering::RasterizationRenderer&>(*m_renderer),bView) == false)
		return false;
	auto debugMode = scene.GetDebugMode();
	if(debugMode != ::pragma::CSceneComponent::DebugMode::None)
		shaderScene->SetDebugMode(debugMode);
	shaderScene->Set3DSky(umath::is_flag_set(m_renderFlags,RenderFlags::RenderAs3DSky));
	
	if(m_stats)
	{
		++m_stats->numShaderStateChanges;
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
	auto &scene = *m_drawSceneInfo.scene.get();
	auto *renderer = scene.GetRenderer();
	if(renderer == nullptr)
		return;
	m_shaderScene->BindSceneCamera(scene,*static_cast<pragma::rendering::RasterizationRenderer*>(renderer),camType == CameraType::View);
}
void pragma::rendering::BaseRenderProcessor::Set3DSky(bool enabled)
{
	umath::set_flag(m_renderFlags,RenderFlags::RenderAs3DSky,enabled);
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false || m_shaderScene == nullptr)
		return;
	m_shaderScene->Set3DSky(enabled);
}
void pragma::rendering::BaseRenderProcessor::SetDrawOrigin(const Vector4 &drawOrigin)
{
	m_drawOrigin = drawOrigin;
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false || m_shaderScene == nullptr)
		return;
	m_shaderScene->BindDrawOrigin(drawOrigin);
}
bool pragma::rendering::BaseRenderProcessor::BindMaterial(CMaterial &mat)
{
	if(&mat == m_curMaterial)
		return umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound);
	UnbindMaterial();
	m_curMaterial = &mat;
	if(umath::is_flag_set(m_stateFlags,StateFlags::ShaderBound) == false || mat.IsInitialized() == false || m_shaderScene->BindMaterial(mat) == false)
		return false;

	if(m_stats)
	{
		if(umath::is_flag_set(m_stateFlags,StateFlags::CountNonOpaqueMaterialsOnly) == false || mat.GetAlphaMode() != AlphaMode::Opaque)
		{
			++m_stats->numMaterialStateChanges;
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
	auto *renderC = ent.GetRenderComponent().get();
	if(umath::is_flag_set(m_stateFlags,StateFlags::MaterialBound) == false || renderC == nullptr || m_shaderScene->BindEntity(ent) == false)
		return false;
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
	
	if(m_stats)
	{
		++m_stats->numEntityStateChanges;
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

bool pragma::rendering::BaseRenderProcessor::Render(CModelSubMesh &mesh)
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::EntityBound) == false || m_curRenderC == nullptr)
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
	{
		++m_stats->numDrawnMeshes;
		m_stats->numDrawnVertices += mesh.GetVertexCount();
		m_stats->numDrawnTrianges += mesh.GetTriangleCount();
		m_stats->meshes.push_back(std::static_pointer_cast<CModelSubMesh>(mesh.shared_from_this()));
	}

	m_shaderScene->Draw(mesh);
	return true;
}
#pragma optimize("",on)
