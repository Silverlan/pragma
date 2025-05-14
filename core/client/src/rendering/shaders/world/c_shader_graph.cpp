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
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/shader_material/shader_material.hpp"
#include "pragma/rendering/shader_graph/manager.hpp"
#include "pragma/rendering/shader_graph/module.hpp"
#include "pragma/rendering/shader_graph/nodes/shader_material.hpp"
#include "pragma/rendering/shader_graph/nodes/input_parameter.hpp"
#include "pragma/rendering/shader_graph/nodes/scene_output.hpp"
#include "pragma/model/vk_mesh.h"
#include "pragma/model/c_modelmesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <prosper_command_buffer.hpp>
#include <sharedutils/util_debug.h>
#include <cmaterial_manager2.hpp>
#include <texture_type.h>
#include <cmaterial.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

import pragma.shadergraph;

using namespace pragma;
#pragma optimize("", off)
ShaderGraph::ShaderGraph(prosper::IPrContext &context, const std::shared_ptr<pragma::shadergraph::Graph> &sg, const std::string &identifier, const std::string &fsShader) : ShaderGameWorldLightingPass {context, identifier, "programs/scene/textured", fsShader}, m_shaderGraph {sg} {}

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

const pragma::shadergraph::Graph *ShaderGraph::GetGraph() const { return m_shaderGraph.get(); }

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

	for(auto &mod : m_modules)
		mod->InitializeShaderResources();

	m_alphaMode = AlphaMode::Opaque;
	if(graph) {
		for(auto &node : graph->GetNodes()) {
			auto *outputNode = dynamic_cast<const pragma::rendering::shader_graph::SceneOutputNode *>(&node->node);
			if(!outputNode)
				continue;
			node->GetInputValue(pragma::rendering::shader_graph::SceneOutputNode::CONST_ALPHA_MODE, m_alphaMode);
		}
	}

	ShaderGameWorldLightingPass::InitializeShaderResources();
}

void ShaderGraph::InitializeMaterialData(const CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, pragma::rendering::ShaderInputData &inOutMatData)
{
	// If graph has "pbr" module, pbr descriptor set should be added
	//prosper::DescriptorSetInfo

	ShaderGameWorldLightingPass::InitializeMaterialData(mat, shaderMat, inOutMatData);
	float specularFactor;
	if(mat.GetProperty("specular_factor", &specularFactor)) {
		auto roughnessFactor = inOutMatData.GetValue<float>("roughness_factor");
		if(!roughnessFactor)
			roughnessFactor = 1.f;
		*roughnessFactor *= (1.f - specularFactor);
		inOutMatData.SetValue<float>("roughness_factor", *roughnessFactor);
	}
}

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGraph::InitializeMaterialDescriptorSet(CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo) { return ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(mat, descSetInfo); }
void ShaderGraph::OnPipelinesInitialized() { ShaderGameWorldLightingPass::OnPipelinesInitialized(); }
void ShaderGraph::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo, pipelineIdx);

	switch(m_alphaMode) {
	case AlphaMode::Blend:
		SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
		break;
	case AlphaMode::Mask:
		SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);
		pipelineInfo.ToggleDepthWrites(true);
		break;
	}
}

void ShaderGraph::InitializeShaderMaterial()
{
	m_shaderMaterial = GenerateShaderMaterial();
	if(m_shaderMaterial)
		m_materialDescSetInfo = CreateMaterialDescriptorSetInfo(*m_shaderMaterial);
	else
		ShaderGameWorldLightingPass::InitializeShaderMaterial(); // Use default shader material
}

static size_t get_size(pragma::shadergraph::DataType type)
{
	switch(type) {
	case pragma::shadergraph::DataType::Boolean:
	case pragma::shadergraph::DataType::Int:
	case pragma::shadergraph::DataType::UInt:
	case pragma::shadergraph::DataType::Float:
	case pragma::shadergraph::DataType::UInt16:
		return sizeof(float);
	case pragma::shadergraph::DataType::Color:
	case pragma::shadergraph::DataType::Vector:
	case pragma::shadergraph::DataType::Point:
	case pragma::shadergraph::DataType::Normal:
		return sizeof(Vector3);
	case pragma::shadergraph::DataType::Vector4:
		return sizeof(Vector4);
	case pragma::shadergraph::DataType::Point2:
		return sizeof(Vector2);
	case pragma::shadergraph::DataType::Transform:
		return sizeof(Mat4);
	}
	return 0;
}

static size_t get_alignment(pragma::shadergraph::DataType type)
{
	switch(type) {
	case pragma::shadergraph::DataType::Boolean:
	case pragma::shadergraph::DataType::Int:
	case pragma::shadergraph::DataType::UInt:
	case pragma::shadergraph::DataType::Float:
	case pragma::shadergraph::DataType::UInt16:
		return sizeof(float);
	case pragma::shadergraph::DataType::Color:
	case pragma::shadergraph::DataType::Vector:
	case pragma::shadergraph::DataType::Vector4:
	case pragma::shadergraph::DataType::Point:
	case pragma::shadergraph::DataType::Normal:
		return sizeof(Vector4);
	case pragma::shadergraph::DataType::Point2:
		return sizeof(Vector2);
	case pragma::shadergraph::DataType::Transform:
		return sizeof(Mat4);
	}
	return 0;
}

std::shared_ptr<pragma::rendering::shader_material::ShaderMaterial> ShaderGraph::GenerateShaderMaterial()
{
	auto *graph = GetGraph();
	if(!graph)
		return nullptr;
	auto sm = std::make_shared<pragma::rendering::shader_material::ShaderMaterial>(GetIdentifier());
	struct ParamNodeInfo {
		std::string name;
		pragma::shadergraph::GraphNode *node;
		size_t alignment;
		size_t size;
	};
	std::vector<ParamNodeInfo> globalParamNodes;
	for(auto &node : graph->GetNodes()) {
		auto *paramNode = dynamic_cast<const pragma::rendering::shader_graph::BaseInputParameterNode *>(&node->node);
		if(!paramNode)
			continue;
		pragma::rendering::shader_graph::BaseInputParameterNode::Scope scope;
		if(!node->GetInputValue(pragma::rendering::shader_graph::BaseInputParameterNode::CONST_SCOPE, scope))
			continue;
		if(scope != pragma::rendering::shader_graph::BaseInputParameterNode::Scope::Global)
			continue;
		std::string name;
		if(!node->GetInputValue(pragma::rendering::shader_graph::BaseInputParameterNode::CONST_NAME, name))
			continue;
		if(name.empty())
			continue;

		auto *texNode = dynamic_cast<const pragma::rendering::shader_graph::InputParameterTextureNode *>(paramNode);
		if(texNode) {
			sm->textures.push_back({});
			auto &tex = sm->textures.back();
			tex.name = name;
			tex.defaultTexturePath = "white";
			node->GetInputValue(pragma::rendering::shader_graph::InputParameterTextureNode::CONST_DEFAULT_TEXTURE, *tex.defaultTexturePath);

			auto colorSpace = node->GetConstantInputValue<pragma::rendering::shader_graph::InputParameterTextureNode::ColorSpace>(pragma::rendering::shader_graph::InputParameterTextureNode::CONST_COLOR_SPACE);
			if(!colorSpace)
				colorSpace = pragma::rendering::shader_graph::InputParameterTextureNode::ColorSpace::Srgb;
			switch(*colorSpace) {
			case pragma::rendering::shader_graph::InputParameterTextureNode::ColorSpace::Srgb:
				tex.colorMap = true;
				break;
			}

			auto imageType = node->GetConstantInputValue<pragma::rendering::shader_graph::InputParameterTextureNode::ImageType>(pragma::rendering::shader_graph::InputParameterTextureNode::CONST_IMAGE_TYPE);
			if(!imageType)
				imageType = pragma::rendering::shader_graph::InputParameterTextureNode::ImageType::e2D;
			switch(*imageType) {
			case pragma::rendering::shader_graph::InputParameterTextureNode::ImageType::Cube:
				tex.cubemap = true;
				break;
			}
			continue;
		}

		if(globalParamNodes.size() == globalParamNodes.capacity())
			globalParamNodes.reserve(globalParamNodes.size() * 2 + 10);
		globalParamNodes.push_back({});
		auto &info = globalParamNodes.back();
		info.node = node.get();
		info.alignment = get_alignment(paramNode->GetParameterType());
		info.size = get_size(paramNode->GetParameterType());
		info.name = std::move(name);
	}

	// TODO: Throw error if two parameters have the same name
	std::sort(globalParamNodes.begin(), globalParamNodes.end(), [](auto &a, auto &b) {
		if(a.alignment == b.alignment)
			return a.name < b.name;
		return a.alignment > b.alignment;
	});

	size_t offset = 0;
	if(!sm->properties.empty()) {
		auto &lastProp = sm->properties.back();
		offset = lastProp.offset + lastProp.GetSize();
	}
	size_t paddingIdx = 0;
	for(auto it = globalParamNodes.begin(); it != globalParamNodes.end();) {
		auto *info = &*it;
		auto alignment = info->alignment;
		if((offset % alignment) != 0) {
			auto padding = alignment - (offset % alignment);
			UTIL_ASSERT((padding % sizeof(float)) == 0);
			auto paddingName = "padding" + std::to_string(paddingIdx++);
			it = globalParamNodes.insert(it, {paddingName, nullptr, sizeof(float), sizeof(float)});
			continue;
		}
		offset += info->size;
		++it;
	}

	std::vector<pragma::rendering::Property> params;
	params.reserve(globalParamNodes.size());
	for(auto &info : globalParamNodes) {
		auto type = pragma::shadergraph::DataType::Float;
		if(info.node) {
			auto &paramNode = *dynamic_cast<const pragma::rendering::shader_graph::BaseInputParameterNode *>(&info.node->node);
			type = paramNode.GetParameterType();
		}
		pragma::rendering::Property prop {info.name, type};
		auto res = true;
		if(info.node) {
			res = pragma::shadergraph::visit(type, [this, &info, &prop](auto tag) -> bool {
				using T = typename decltype(tag)::type;

				T defaultVal;
				if(!info.node->GetInputValue(pragma::rendering::shader_graph::BaseInputParameterNode::CONST_DEFAULT, defaultVal))
					return false;
				prop->defaultValue.Set(defaultVal);

				if constexpr(std::is_same_v<T, float>) {
					float minVal;
					float maxVal;
					float stepSize;
					if(!info.node->GetInputValue(pragma::rendering::shader_graph::InputParameterFloatNode::CONST_MIN, minVal))
						return false;
					if(!info.node->GetInputValue(pragma::rendering::shader_graph::InputParameterFloatNode::CONST_MAX, maxVal))
						return false;
					if(!info.node->GetInputValue(pragma::rendering::shader_graph::InputParameterFloatNode::CONST_STEP_SIZE, stepSize))
						return false;
					prop->min = minVal;
					prop->max = maxVal;
					// prop->stepSize = stepSize;
				}

				return true;
			});
		}
		else
			prop->defaultValue = 0.f;
		if(!res)
			continue;
		params.push_back(prop);
	}

	if(params.empty())
		return nullptr;

	sm->properties.reserve(sm->properties.size() + params.size());
	for(auto &param : params)
		sm->AddProperty(std::move(param));
	return sm;
}

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

bool ShaderGraph::RecordBindEntity(rendering::ShaderProcessor &shaderProcessor, CRenderComponent &renderC, prosper::IShaderPipelineLayout &layout, uint32_t entityInstanceDescriptorSetIndex) const
{
	if(!ShaderGameWorldLightingPass::RecordBindEntity(shaderProcessor, renderC, layout, entityInstanceDescriptorSetIndex))
		return false;
	for(auto &mod : m_modules)
		mod->RecordBindEntity(shaderProcessor, renderC, layout, entityInstanceDescriptorSetIndex);
	return true;
}
bool ShaderGraph::RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, CMaterial &mat) const
{
	if(!ShaderGameWorldLightingPass::RecordBindMaterial(shaderProcessor, mat))
		return false;
	for(auto &mod : m_modules)
		mod->RecordBindMaterial(shaderProcessor, mat);
	return true;
}
bool ShaderGraph::IsTranslucentPipeline(uint32_t pipelineIdx) const { return m_alphaMode != AlphaMode::Opaque; }
