/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_skybox.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/entities/entity_instance_index_buffer.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/model/c_model.h"
#include "pragma/logging.hpp"
#include <cmaterial_manager2.hpp>
#include <prosper_command_buffer.hpp>
#include <cmaterial.h>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

bool pragma::rendering::ShaderProcessor::RecordBindScene(const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, const pragma::ShaderGameWorld &shader, bool view)
{
	auto &hCam = scene.GetActiveCamera();
	if(hCam.expired()) {
		spdlog::trace("Attempted to bind scene '{}' with no active camera!", scene.GetEntity().ToString());
		return false;
	}
	auto *dsScene = view ? scene.GetViewCameraDescriptorSet() : scene.GetCameraDescriptorSetGraphics();
	auto *dsRenderer = renderer.GetRendererDescriptorSet();
	auto &dsRenderSettings = c_game->GetGlobalRenderSettingsDescriptorSet();
	auto *dsShadows = pragma::CShadowComponent::GetDescriptorSet();
	assert(dsScene);
	assert(dsRenderer);
	assert(dsShadows);
	m_sceneC = &scene;
	m_rendererC = &renderer;
	// m_sceneFlags = ShaderGameWorld::SceneFlags::None;
	shader.RecordBindScene(*this, scene, renderer, *dsScene, *dsRenderer, dsRenderSettings, *dsShadows, m_drawOrigin, m_sceneFlags);
	return true;
}
void pragma::rendering::ShaderProcessor::SetDrawOrigin(const Vector4 &drawOrigin) { m_drawOrigin = drawOrigin; }
bool pragma::rendering::ShaderProcessor::RecordBindShader(const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, bool view, ShaderGameWorld::SceneFlags sceneFlags, pragma::ShaderGameWorld &shader, uint32_t pipelineIdx)
{
	auto &context = c_engine->GetRenderContext();
	m_curShader = &shader;
	auto matDsIdx = m_curShader->GetMaterialDescriptorSetIndex();
	m_currentPipelineLayout = context.GetShaderPipelineLayout(shader, pipelineIdx);
	m_materialDescriptorSetIndex = matDsIdx ? *matDsIdx : std::numeric_limits<uint32_t>::max();
	m_entityInstanceDescriptorSetIndex = m_curShader->GetInstanceDescriptorSetIndex();
	m_curInstanceSet = nullptr;
	// m_clipPlane = {};
	m_boundClipPlane = {};
	m_depthBias = {};
	m_vertexAnimC = nullptr;
	m_modelC = nullptr;
	m_lightMapReceiverC = nullptr;
	m_materialDescSetBound = false;
	m_curVertexAnimationOffset = std::numeric_limits<uint32_t>::max();
	m_sceneFlags = sceneFlags;
	m_alphaCutoff = std::numeric_limits<float>::max();
	m_depthPrepass = shader.IsDepthPrepassShader();

	if(m_cmdBuffer.RecordBindShaderPipeline(shader, pipelineIdx) == false) {
		if(VERBOSE_RENDER_OUTPUT_ENABLED)
			Con::cwar << "[Render] WARNING: Failed to bind pipeline " << pipelineIdx << " of shader " << shader.GetIdentifier() << "!" << Con::endl;
		return false;
	}

#if 0
	// Reset depth bias
	// TODO: This kind of depth bias isn't used anymore and should be removed from the shaders entirely!
	m_cmdBuffer.RecordSetDepthBias();
#endif
	// m_cmdBuffer.RecordBindVertexBuffer(shader,*CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer(),umath::to_integral(ShaderEntity::VertexBinding::RenderBufferIndex));
	return RecordBindScene(scene, renderer, shader, view);
}
void pragma::rendering::ShaderProcessor::UpdateSceneFlags(ShaderGameWorld::SceneFlags sceneFlags)
{
	if(sceneFlags == m_sceneFlags)
		return;
	m_sceneFlags = sceneFlags;
	m_curShader->RecordSceneFlags(*this, sceneFlags);
}
bool pragma::rendering::ShaderProcessor::RecordBindLight(CLightComponent &light, uint32_t layerId)
{
	m_curShader->RecordBindLight(*this, light, layerId);
	return true;
}
bool pragma::rendering::ShaderProcessor::RecordBindMaterial(CMaterial &mat)
{
	if(m_curShader->RecordBindMaterial(*this, mat) == false) {
		if(!m_materialDescSetBound) {
			m_materialDescSetBound = true;
			auto *errMat = client->GetMaterialManager().GetErrorMaterial();
			if(!errMat)
				return false;
			// Bind a dummy material
			if(!m_curShader->ShaderGameWorld::RecordBindMaterial(*this, static_cast<CMaterial &>(*errMat)))
				return false;
		}
		return true; // TODO: This should only return true if we're doing a depth pre-pass and the material isn't transparent
	}
	m_materialDescSetBound = true;
	auto flags = m_sceneFlags;
	auto alphaMode = mat.GetAlphaMode();
	if(alphaMode != AlphaMode::Opaque) {
		auto alphaCutoff = mat.GetAlphaCutoff();
		if(alphaCutoff != m_alphaCutoff) {
			m_alphaCutoff = alphaCutoff;
			m_curShader->RecordAlphaCutoff(*this, alphaCutoff);
		}
		flags |= ShaderGameWorld::SceneFlags::AlphaTest;
	}

	// TODO
	//shaderScene->Set3DSky(umath::is_flag_set(m_renderFlags,RenderFlags::RenderAs3DSky));
	UpdateSceneFlags(flags);
	return true;
}
void pragma::rendering::ShaderProcessor::UpdateClipPlane()
{
	auto clipPlane = m_entityClipPlane.has_value() ? *m_entityClipPlane : m_clipPlane.has_value() ? *m_clipPlane : Vector4 {};
	if(clipPlane != m_boundClipPlane) {
		m_curShader->RecordClipPlane(*this, clipPlane);
		m_boundClipPlane = clipPlane;
	}
}
bool pragma::rendering::ShaderProcessor::RecordBindEntity(CBaseEntity &ent)
{
	auto *renderC = ent.GetRenderComponent();
	assert(renderC);
	auto *descSet = renderC->GetRenderDescriptorSet();
	assert(descSet);
	if(descSet == nullptr) {
		if(VERBOSE_RENDER_OUTPUT_ENABLED) {
			Con::cwar << "[Render] WARNING: Entity ";
			ent.print(Con::cout);
			Con::cwar << " has invalid render descriptor set!" << Con::endl;
		}
		return false;
	}
	auto sceneFlags = m_sceneFlags;
	if(m_curShader->RecordBindEntity(*this, *renderC, *m_currentPipelineLayout, m_entityInstanceDescriptorSetIndex) == false) {
		if(VERBOSE_RENDER_OUTPUT_ENABLED) {
			Con::cwar << "[Render] WARNING: Failed to bind entity ";
			ent.print(Con::cout);
			Con::cwar << " to shader " << m_curShader->GetIdentifier() << "!" << Con::endl;
		}
		return false;
	}

	auto *clipPlane = renderC->GetRenderClipPlane();
	if(clipPlane)
		m_entityClipPlane = *clipPlane;
	else
		m_entityClipPlane = {};
	UpdateClipPlane();

	auto *depthBias = renderC->GetDepthBias();
	if(static_cast<bool>(depthBias) != m_depthBias.has_value() && (!depthBias || !m_depthBias.has_value() || *depthBias != *m_depthBias)) {
		auto vDepthBias = depthBias ? *depthBias : Vector4 {};
		m_curShader->RecordDepthBias(*this, vDepthBias);

		m_depthBias = depthBias ? *depthBias : std::optional<Vector2> {};
	}

	m_vertexAnimC = nullptr;
	m_modelC = renderC->GetModelComponent();
	if(m_modelC) {
		auto &vertAnimBuffer = static_cast<CModel &>(*m_modelC->GetModel()).GetVertexAnimationBuffer();
		if(vertAnimBuffer != nullptr)
			m_vertexAnimC = ent.GetComponent<pragma::CVertexAnimatedComponent>().get();
	}

	if(m_curShader->IsUsingLightmaps()) {
		m_lightMapReceiverC = renderC->GetLightMapReceiverComponent();
		if(m_lightMapReceiverC) {
			sceneFlags |= ShaderGameWorld::SceneFlags::LightmapsEnabled;
			if(m_rendererC->HasIndirectLightmap()) {
				sceneFlags |= ShaderGameWorld::SceneFlags::IndirectLightmapsEnabled;
				if(m_rendererC->HasDirectionalLightmap())
					sceneFlags |= ShaderGameWorld::SceneFlags::DirectionalLightmapsEnabled;
			}
		}
		umath::set_flag(sceneFlags, ShaderGameWorld::SceneFlags::DisableShadows, !renderC->IsReceivingShadows());
	}

	UpdateSceneFlags(sceneFlags);

	// TODO: Extended vertex weights
	// TODO: 3d sky
	return true;
}
bool pragma::rendering::ShaderProcessor::RecordDraw(CModelSubMesh &mesh, pragma::RenderMeshIndex meshIdx, const pragma::rendering::RenderQueue::InstanceSet *instanceSet)
{
	uint32_t vertexAnimationOffset = 0;
	if(m_vertexAnimC) {
		// TODO: Skip this if shader doesn't support morph target animations
		// (Check for specialization flag)
		auto offset = 0u;
		auto animCount = 0u;
		if(m_vertexAnimC->GetVertexAnimationBufferMeshOffset(mesh, offset, animCount) == true) {
			auto vaData = ((offset << 16) >> 16) | animCount << 16;
			vertexAnimationOffset = vaData;
		}
	}
	if(vertexAnimationOffset != m_curVertexAnimationOffset) {
		m_curVertexAnimationOffset = vertexAnimationOffset;
		m_curShader->RecordVertexAnimationOffset(*this, vertexAnimationOffset);
	}
	// TODO
	// umath::set_flag(renderFlags,RenderFlags::UseExtendedVertexWeights,mesh.GetExtendedVertexWeights().empty() == false);

	auto numIndices = mesh.GetIndexCount();
	if(numIndices > umath::to_integral(GameLimits::MaxMeshVertices)) {
		Con::cerr << "Attempted to draw mesh with more than maximum (" << umath::to_integral(GameLimits::MaxMeshVertices) << ") amount of vertices!" << Con::endl;
		return false;
	}
	auto &vkMesh = mesh.GetSceneMesh();
	auto &bufferData = *m_modelC->GetRenderBufferData(meshIdx);
	if(bufferData.renderBuffer == nullptr) {
		if(VERBOSE_RENDER_OUTPUT_ENABLED) {
			Con::cwar << "[Render] WARNING: Render buffer data of entity ";
			m_modelC->GetEntity().print(Con::cout);
			Con::cwar << " has invalid render buffer!" << Con::endl;
		}
		return false;
	}
	if((m_depthPrepass && !bufferData.IsDepthPrepassEnabled()))
		return false;

	if(m_cmdBuffer.RecordBindRenderBuffer(*bufferData.renderBuffer) == false) {
		if(VERBOSE_RENDER_OUTPUT_ENABLED) {
			Con::cwar << "[Render] WARNING: Failed to bind render buffer of entity ";
			m_modelC->GetEntity().print(Con::cout);
			Con::cwar << "!" << Con::endl;
		}
		return false;
	}

	uint32_t instanceCount = 1;
	if(instanceSet) {
		instanceCount = instanceSet->instanceCount;
		if(instanceSet != m_curInstanceSet) {
			m_curInstanceSet = instanceSet;
			m_cmdBuffer.RecordBindVertexBuffer(*m_curShader, *instanceSet->instanceBuffer, umath::to_integral(ShaderEntity::VertexBinding::RenderBufferIndex));
		}
	}
	else if(m_curInstanceSet) {
		m_curInstanceSet = nullptr;
		m_cmdBuffer.RecordBindVertexBuffer(*m_curShader, *CSceneComponent::GetEntityInstanceIndexBuffer()->GetBuffer(), umath::to_integral(ShaderEntity::VertexBinding::RenderBufferIndex));
	}
	if(m_stats) {
		(*m_stats)->Increment(RenderPassStats::Counter::DrawnMeshes, instanceCount);
		(*m_stats)->Increment(RenderPassStats::Counter::DrawnVertices, mesh.GetVertexCount() * instanceCount);
		(*m_stats)->Increment(RenderPassStats::Counter::DrawnTriangles, mesh.GetTriangleCount() * instanceCount);
		m_stats->meshes.push_back(std::static_pointer_cast<CModelSubMesh>(mesh.shared_from_this()));

		(*m_stats)->Increment(RenderPassStats::Counter::DrawCalls);
	}
	return m_curShader->OnRecordDrawMesh(*this, mesh) && m_cmdBuffer.RecordDrawIndexed(mesh.GetIndexCount(), instanceCount);
}
CBaseEntity &pragma::rendering::ShaderProcessor::GetCurrentEntity() const { return static_cast<CBaseEntity &>(m_modelC->GetEntity()); }
const pragma::CSceneComponent &pragma::rendering::ShaderProcessor::GetCurrentScene() const { return *m_sceneC; }
