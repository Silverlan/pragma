/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_graph.hpp"
#include "cmaterialmanager.h"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/shader_material/shader_material.hpp"
#include "pragma/rendering/shader_graph/manager.hpp"
#include "pragma/rendering/shader_graph/module.hpp"
#include "pragma/rendering/shader_graph/nodes/shader_material.hpp"
#include "pragma/rendering/shader_graph/nodes/input_parameter_float.hpp"
#include "pragma/model/vk_mesh.h"
#include "pragma/model/c_modelmesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <cmaterial_manager2.hpp>
#include <texture_type.h>
#include <cmaterial.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

import pragma.shadergraph;

using namespace pragma;

ShaderGraph::ShaderGraph(prosper::IPrContext &context, const std::string &identifier, const std::string &fsShader) : ShaderGameWorldLightingPass {context, identifier, "programs/scene/textured", fsShader} {}

ShaderGraph::~ShaderGraph() {}

void ShaderGraph::UpdateRenderFlags(CModelSubMesh &mesh, SceneFlags &inOutFlags)
{
	ShaderGameWorldLightingPass::UpdateRenderFlags(mesh, inOutFlags);
	for(auto &mod : m_modules)
		mod->UpdateRenderFlags(mesh, inOutFlags);
}
void ShaderGraph::InitializeGfxPipelineDescriptorSets()
{
	ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets();
	for(auto &mod : m_modules)
		mod->InitializeGfxPipelineDescriptorSets();
}

void ShaderGraph::GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode)
{
	ShaderGameWorldLightingPass::GetShaderPreprocessorDefinitions(outDefinitions, outPrefixCode);
	for(auto &mod : m_modules)
		mod->GetShaderPreprocessorDefinitions(outDefinitions, outPrefixCode);
}

void ShaderGraph::ClearShaderResources()
{
	m_modules.clear();
	ShaderGameWorldLightingPass::ClearShaderResources();
}

static const pragma::rendering::shader_material::ShaderMaterial *find_shader_material(const pragma::shadergraph::Graph &graph)
{
	for(auto &node : graph.GetNodes()) {
		auto *matNode = dynamic_cast<const pragma::rendering::shader_graph::ShaderMaterialNode *>(&node->node);
		if(matNode)
			return &matNode->GetShaderMaterial();
	}
	return nullptr;
}

const pragma::shadergraph::Graph *ShaderGraph::GetGraph() const
{
	auto &graphManager = c_engine->GetShaderGraphManager();
	auto graphData = graphManager.GetGraph(GetIdentifier());
	if(graphData)
		return graphData->GetGraph().get();
	return nullptr;
}

void ShaderGraph::InitializeShaderResources()
{
	auto &graphManager = c_engine->GetShaderGraphManager();
	auto *graph = GetGraph();
	if(graph) {
		auto *shaderMat = find_shader_material(*graph);
		if(shaderMat)
			SetShaderMaterialName(shaderMat->name);

		struct ModuleData {
			std::vector<pragma::shadergraph::GraphNode *> nodes;
		};
		std::unordered_map<std::string, ModuleData> moduleData;
		for(auto &node : graph->GetNodes()) {
			auto &deps = (*node)->GetModuleDependencies();
			moduleData.reserve(moduleData.size() + deps.size());
			for(auto &dep : deps) {
				auto &modData = moduleData[dep];
				modData.nodes.push_back(node.get());
			}
		}
		for(auto &modData : moduleData) {
			auto &modName = modData.first;
			auto mod = graphManager.GetModuleManager().CreateModule(modName, *this, std::move(modData.second.nodes));
			if(mod)
				m_modules.emplace_back(std::move(mod));
		}
	}

	ShaderGameWorldLightingPass::InitializeShaderResources();
}

void ShaderGraph::InitializeMaterialData(const CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::shader_material::ShaderInputData &inOutMatData)
{

	// If graph has "pbr" module, pbr descriptor set should be added
	//prosper::DescriptorSetInfo

	ShaderGameWorldLightingPass::InitializeMaterialData(mat, shaderMat, inOutMatData);
	auto &data = mat.GetDataBlock();
	float specularFactor;
	if(data->GetFloat("specular_factor", &specularFactor)) {
		auto roughnessFactor = inOutMatData.GetValue<float>("roughness_factor");
		if(!roughnessFactor)
			roughnessFactor = 1.f;
		*roughnessFactor *= (1.f - specularFactor);
		inOutMatData.SetValue<float>("roughness_factor", *roughnessFactor);
	}
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGraph::InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo) { return ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(mat, descSetInfo); }
void ShaderGraph::OnPipelinesInitialized() { ShaderGameWorldLightingPass::OnPipelinesInitialized(); }
void ShaderGraph::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderGraph::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	std::array<prosper::IDescriptorSet *, 4> descSets {&dsScene, &dsRenderer, &dsRenderSettings, &dsShadows};

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), GetSceneDescriptorSetIndex(), descSets, dynamicOffsets);

	ShaderGameWorldLightingPass::RecordBindScene(shaderProcessor, scene, renderer, dsScene, dsRenderer, dsRenderSettings, dsShadows, drawOrigin, inOutSceneFlags);
	ShaderGameWorldLightingPass::PushConstants pushConstants {};
	pushConstants.Initialize();
	pushConstants.debugMode = scene.GetDebugMode();
	pushConstants.reflectionProbeIntensity = 1.f;
	pushConstants.flags = inOutSceneFlags;
	pushConstants.drawOrigin = drawOrigin;
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, 0u, sizeof(pushConstants), &pushConstants);

	for(auto &mod : m_modules)
		mod->RecordBindScene(shaderProcessor, scene, renderer, inOutSceneFlags);
}
