/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_skybox.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/model/c_model.h"

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;
#pragma optimize("",off)
bool pragma::rendering::ShaderProcessor::RecordBindScene(const pragma::CSceneComponent &scene,const pragma::rendering::RasterizationRenderer &renderer,const pragma::ShaderGameWorld &referenceShader,bool view)
{
	// Note: The reference shader is not the shader we're actually using for rendering, but a base shader that represents all shader types for the current render pass (e.g. prepass / pbr).
	// We need this information to properly bind the descriptor sets with OpenGL (reference shader is ignored for Vulkan).
	
	auto passType = referenceShader.GetPassType();
	m_passType = passType;
	auto *dsScene = view ? scene.GetViewCameraDescriptorSet() : scene.GetCameraDescriptorSetGraphics();
	auto *dsRenderer = renderer.GetRendererDescriptorSet();
	auto &dsRenderSettings = c_game->GetGlobalRenderSettingsDescriptorSet();
	auto *dsLights = renderer.GetLightSourceDescriptorSet();
	auto *dsShadows = pragma::CShadowComponent::GetDescriptorSet();
	auto &dsMat = referenceShader.GetDefaultMaterialDescriptorSet();
	assert(dsScene);
	assert(dsRenderer);
	assert(dsLights);
	assert(dsShadows);
	m_sceneFlags = ShaderGameWorld::SceneFlags::None;
	m_view = view;
	std::vector<prosper::IDescriptorSet*> descSets {};
	switch(passType)
	{
	case pragma::ShaderGameWorld::PassType::LightingPass:
	{
		descSets.resize(7);
		descSets[0] = &dsMat;
		descSets[1] = dsScene;
		descSets[2] = dsRenderer;
		descSets[3] = &dsRenderSettings;
		descSets[4] = dsLights;
		descSets[5] = dsShadows;
		
		ShaderTextured3DBase::PushConstants pushConstants {};
		pushConstants.Initialize();
		auto &hCam = scene.GetActiveCamera();
		assert(hCam.valid());
		auto iblStrength = 1.f;
		auto *dsPbr = CReflectionProbeComponent::FindDescriptorSetForClosestProbe(scene,hCam->GetEntity().GetPosition(),iblStrength);
		if(dsPbr == nullptr) // No reflection probe and therefore no IBL available. Fallback to non-IBL rendering.
		{
			dsPbr = &static_cast<const pragma::ShaderPBR&>(referenceShader).GetDefaultPbrDescriptorSet();
			m_sceneFlags |= ShaderGameWorld::SceneFlags::NoIBL;
		}
		descSets[6] = dsPbr;

		pushConstants.debugMode = scene.GetDebugMode();
		pushConstants.reflectionProbeIntensity = iblStrength;
		pushConstants.flags = m_sceneFlags;
		m_cmdBuffer.RecordPushConstants(*m_currentPipelineLayout,prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,0u,sizeof(pushConstants),&pushConstants);
		break;
	}
	case pragma::ShaderGameWorld::PassType::DepthPrepass:
	{
		descSets.resize(3);
		descSets[0] = &dsMat;
		descSets[1] = dsScene;
		descSets[2] = &dsRenderSettings;

		ShaderPrepass::PushConstants pushConstants {};
		pushConstants.Initialize();
		m_cmdBuffer.RecordPushConstants(*m_currentPipelineLayout,prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,0u,sizeof(pushConstants),&pushConstants);
		break;
	}
	case pragma::ShaderGameWorld::PassType::ShadowPass:
	{
		descSets.resize(3);
		descSets[0] = &dsMat;
		descSets[1] = dsScene;
		descSets[2] = &dsRenderSettings;

		ShaderShadow::PushConstants pushConstants {};
		pushConstants.Initialize();
		m_cmdBuffer.RecordPushConstants(*m_currentPipelineLayout,prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,0u,sizeof(pushConstants),&pushConstants);
		break;
	}
	case pragma::ShaderGameWorld::PassType::SkyPass:
	{
		descSets.resize(3);
		descSets[0] = &dsMat;
		descSets[1] = dsScene;
		descSets[2] = dsRenderer;
		break;
	}
	}

	// We can bind all of these in one go
	static const std::vector<uint32_t> dynamicOffsets {};
	return m_cmdBuffer.RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics,const_cast<pragma::ShaderGameWorld&>(referenceShader),0u /* pipelineId */,pragma::ShaderGameWorld::MATERIAL_DESCRIPTOR_SET_INDEX,descSets,dynamicOffsets);
}
bool pragma::rendering::ShaderProcessor::RecordBindShader(const pragma::CSceneComponent &scene,const pragma::rendering::RasterizationRenderer &renderer,bool view,pragma::ShaderGameWorld &shader,uint32_t pipelineIdx)
{
	auto &context = c_engine->GetRenderContext();
	m_curShader = &shader;
	m_currentPipelineLayout = context.GetShaderPipelineLayout(shader,pipelineIdx);
	m_materialDescriptorSetIndex = m_curShader->GetMaterialDescriptorSetIndex();
	m_entityInstanceDescriptorSetIndex = m_curShader->GetInstanceDescriptorSetIndex();
	m_curInstanceSet = nullptr;

	if(m_cmdBuffer.RecordBindShaderPipeline(shader,pipelineIdx) == false)
		return false;

	// Reset depth bias
	// TODO: This kind of depth bias isn't used anymore and should be removed from the shaders entirely!
	m_cmdBuffer.RecordSetDepthBias();
	// m_cmdBuffer.RecordBindVertexBuffer(shader,*CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer(),umath::to_integral(ShaderEntity::VertexBinding::RenderBufferIndex));
	return RecordBindScene(scene,renderer,shader,m_view);
}
void pragma::rendering::ShaderProcessor::UpdateSceneFlags(ShaderGameWorld::SceneFlags sceneFlags)
{
	if(sceneFlags == m_sceneFlags)
		return;
	m_sceneFlags = sceneFlags;
	size_t offset = 0;
	switch(m_passType)
	{
	case ShaderGameWorld::PassType::ShadowPass:
		offset = offsetof(ShaderShadow::PushConstants,flags);
		break;
	case ShaderGameWorld::PassType::SkyPass:
		return;
	default:
		offset = offsetof(ShaderGameWorld::ScenePushConstants,flags);
		break;
	}
	m_cmdBuffer.RecordPushConstants(*m_currentPipelineLayout,prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,offset,sizeof(sceneFlags),&sceneFlags);
}
bool pragma::rendering::ShaderProcessor::RecordBindLight(CLightComponent &light,uint32_t layerId)
{
#pragma pack(push,1)
	struct {
		Mat4 depthMVP;
		Vector4 lightPos;
	} pushData;
#pragma pack(pop)

	auto pRadiusComponent = light.GetEntity().GetComponent<CRadiusComponent>();
	auto &pos = light.GetEntity().GetPosition();
	pushData.depthMVP = light.GetTransformationMatrix(layerId);
	pushData.lightPos = Vector4{pos.x,pos.y,pos.z,static_cast<float>(pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f)};

	return m_cmdBuffer.RecordPushConstants(*m_currentPipelineLayout,prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,offsetof(ShaderShadow::PushConstants,depthMVP),sizeof(pushData),&pushData);
}
bool pragma::rendering::ShaderProcessor::RecordBindMaterial(CMaterial &mat)
{
	auto alphaMode = mat.GetAlphaMode();
	if(m_passType != ShaderGameWorld::PassType::LightingPass && m_passType != ShaderGameWorld::PassType::SkyPass && alphaMode == AlphaMode::Opaque)
		return true; // TODO: Notify shader?

	auto flags = m_sceneFlags;
	if(alphaMode != AlphaMode::Opaque)
	{
		auto alphaCutoff = mat.GetAlphaCutoff();
		if(alphaCutoff != m_alphaCutoff)
		{
			m_alphaCutoff = alphaCutoff;
			if(m_passType != ShaderGameWorld::PassType::LightingPass && m_passType != ShaderGameWorld::PassType::SkyPass)
			{
				static_assert(std::is_same_v<decltype(alphaCutoff),decltype(ShaderPrepass::PushConstants::alphaCutoff)>);
				size_t offset = 0;
				switch(m_passType)
				{
				case ShaderGameWorld::PassType::DepthPrepass:
					offset = offsetof(ShaderPrepass::PushConstants,alphaCutoff);
					break;
				case ShaderGameWorld::PassType::ShadowPass:
					offset = offsetof(ShaderShadow::PushConstants,alphaCutoff);
					break;
				}
				m_cmdBuffer.RecordPushConstants(*m_currentPipelineLayout,prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,offset,sizeof(alphaCutoff),&alphaCutoff);
			}
		}
		flags |= ShaderGameWorld::SceneFlags::AlphaTest;
	}

	// TODO
	//shaderScene->Set3DSky(umath::is_flag_set(m_renderFlags,RenderFlags::RenderAs3DSky));
	UpdateSceneFlags(flags);

	// TODO: GetDescriptorSetGroup requires a map lookup and is fairly expensive, optimize this!
	auto descSetGroup = mat.GetDescriptorSetGroup(*m_curShader);
	if(descSetGroup == nullptr)
		descSetGroup = m_curShader->InitializeMaterialDescriptorSet(mat,false); // Attempt to initialize on the fly (TODO: Is this thread safe?)
	if(descSetGroup == nullptr)
		return false;
	return m_cmdBuffer.RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics,*m_currentPipelineLayout,m_materialDescriptorSetIndex,*descSetGroup->GetDescriptorSet(0));
}
bool pragma::rendering::ShaderProcessor::RecordBindEntity(CBaseEntity &ent)
{
	auto *renderC = ent.GetRenderComponent();
	assert(renderC);
	auto *descSet = renderC->GetRenderDescriptorSet();
	assert(descSet);
	if(descSet == nullptr)
		return false;
	auto sceneFlags = m_sceneFlags;
	m_cmdBuffer.RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics,*m_currentPipelineLayout,m_entityInstanceDescriptorSetIndex,*descSet);
	auto *clipPlane = renderC->GetRenderClipPlane();
	if(static_cast<bool>(clipPlane) != m_clipPlane.has_value() && (!clipPlane || *clipPlane != *m_clipPlane))
	{
		if(m_passType != ShaderGameWorld::PassType::ShadowPass && m_passType != ShaderGameWorld::PassType::SkyPass)
		{
			static_assert(sizeof(*clipPlane) == sizeof(ShaderGameWorld::ScenePushConstants::clipPlane));
			auto vClipPlane = clipPlane ? *clipPlane : Vector4{};
			m_cmdBuffer.RecordPushConstants(*m_currentPipelineLayout,prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,offsetof(ShaderGameWorld::ScenePushConstants,clipPlane),sizeof(*clipPlane),&vClipPlane);
		}
		m_clipPlane = clipPlane ? *clipPlane : std::optional<Vector4>{};
	}
	
	m_vertexAnimC = nullptr;
	m_modelC = renderC->GetModelComponent();
	if(m_modelC)
	{
		auto &vertAnimBuffer = static_cast<CModel&>(*m_modelC->GetModel()).GetVertexAnimationBuffer();
		if(vertAnimBuffer != nullptr)
			m_vertexAnimC = ent.GetComponent<pragma::CVertexAnimatedComponent>().get();
	}
	
	if(m_passType == ShaderGameWorld::PassType::LightingPass)
	{
		m_lightMapReceiverC = renderC->GetLightMapReceiverComponent();
		if(m_lightMapReceiverC)
			sceneFlags |= ShaderGameWorld::SceneFlags::LightmapsEnabled;
		umath::set_flag(sceneFlags,ShaderGameWorld::SceneFlags::DisableShadows,!renderC->IsReceivingShadows());
	}

	UpdateSceneFlags(sceneFlags);

	// TODO: Extended vertex weights
	// TODO: 3d sky
	return true;
}
bool pragma::rendering::ShaderProcessor::RecordDraw(CModelSubMesh &mesh,pragma::RenderMeshIndex meshIdx,const pragma::rendering::RenderQueue::InstanceSet *instanceSet)
{
	uint32_t vertexAnimationOffset = 0;
	if(m_vertexAnimC)
	{
		auto offset = 0u;
		auto animCount = 0u;
		if(m_vertexAnimC->GetVertexAnimationBufferMeshOffset(mesh,offset,animCount) == true)
		{
			auto vaData = ((offset<<16)>>16) | animCount<<16;
			vertexAnimationOffset = vaData;
		}
	}
	if(vertexAnimationOffset != m_curVertexAnimationOffset)
	{
		m_curVertexAnimationOffset = vertexAnimationOffset;
		uint32_t pushConstantOffset;
		m_curShader->GetVertexAnimationPushConstantInfo(pushConstantOffset);
		m_cmdBuffer.RecordPushConstants(*m_currentPipelineLayout,prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,pushConstantOffset,sizeof(vertexAnimationOffset),&vertexAnimationOffset);
	}
	// TODO
	// umath::set_flag(renderFlags,RenderFlags::UseExtendedVertexWeights,mesh.GetExtendedVertexWeights().empty() == false);

	auto numTriangleVertices = mesh.GetTriangleVertexCount();
	if(numTriangleVertices > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return false;
	}
	auto &vkMesh = mesh.GetSceneMesh();
	auto &renderBuffer = m_modelC->GetRenderBuffer(meshIdx);
	if(renderBuffer == nullptr || m_cmdBuffer.RecordBindRenderBuffer(*renderBuffer) == false)
		return false;

	uint32_t instanceCount = 1;
	if(instanceSet)
	{
		instanceCount = instanceSet->instanceCount;
		if(instanceSet != m_curInstanceSet)
		{
			m_curInstanceSet = instanceSet;
			m_cmdBuffer.RecordBindVertexBuffer(*m_curShader,*instanceSet->instanceBuffer,umath::to_integral(ShaderEntity::VertexBinding::RenderBufferIndex));
		}
	}
	else if(m_curInstanceSet)
	{
		m_curInstanceSet = nullptr;
		m_cmdBuffer.RecordBindVertexBuffer(*m_curShader,*CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer(),umath::to_integral(ShaderEntity::VertexBinding::RenderBufferIndex));
	}
	if(m_stats)
	{
		m_stats->numDrawnMeshes += instanceCount;
		m_stats->numDrawnVertices += mesh.GetVertexCount() *instanceCount;
		m_stats->numDrawnTrianges += mesh.GetTriangleCount() *instanceCount;
		m_stats->meshes.push_back(std::static_pointer_cast<CModelSubMesh>(mesh.shared_from_this()));

		++m_stats->numDrawCalls;
	}
	return m_cmdBuffer.RecordDrawIndexed(mesh.GetTriangleVertexCount(),instanceCount);
}
#pragma optimize("",on)
