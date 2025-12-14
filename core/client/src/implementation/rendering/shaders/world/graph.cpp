// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.shaders.world_graph;

import :client_state;
import :engine;
import :entities.components;
import :model;
import pragma.shadergraph;

using namespace pragma;

ShaderGraph::ShaderGraph(prosper::IPrContext &context, const std::shared_ptr<shadergraph::Graph> &sg, const std::string &identifier, const std::string &fsShader) : ShaderGameWorldLightingPass {context, identifier, "programs/scene/textured", fsShader}, m_shaderGraph {sg} {}

ShaderGraph::~ShaderGraph() {}

void ShaderGraph::UpdateRenderFlags(geometry::CModelSubMesh &mesh, SceneFlags &inOutFlags)
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

static const rendering::shader_material::ShaderMaterial *find_shader_material(const shadergraph::Graph &graph)
{
	for(auto &node : graph.GetNodes()) {
		auto *matNode = dynamic_cast<const rendering::shader_graph::ShaderMaterialNode *>(&node->node);
		if(matNode)
			return &matNode->GetShaderMaterial();
	}
	return nullptr;
}

const shadergraph::Graph *ShaderGraph::GetGraph() const { return m_shaderGraph.get(); }

void ShaderGraph::InitializeShaderResources()
{
	auto &graphManager = get_cengine()->GetShaderGraphManager();
	auto *graph = GetGraph();
	if(graph) {
		auto *shaderMat = find_shader_material(*graph);
		if(shaderMat)
			SetShaderMaterialName(shaderMat->name);

		struct ModuleData {
			std::vector<shadergraph::GraphNode *> nodes;
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
			auto *outputNode = dynamic_cast<const rendering::shader_graph::SceneOutputNode *>(&node->node);
			if(!outputNode)
				continue;
			node->GetInputValue(rendering::shader_graph::SceneOutputNode::CONST_ALPHA_MODE, m_alphaMode);
		}
	}

	ShaderGameWorldLightingPass::InitializeShaderResources();
}

void ShaderGraph::InitializeMaterialData(const material::CMaterial &mat, const rendering::shader_material::ShaderMaterial &shaderMat, rendering::ShaderInputData &inOutMatData)
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

std::shared_ptr<prosper::IDescriptorSetGroup> ShaderGraph::InitializeMaterialDescriptorSet(material::CMaterial &mat, const prosper::DescriptorSetInfo &descSetInfo) { return ShaderGameWorldLightingPass::InitializeMaterialDescriptorSet(mat, descSetInfo); }
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

static size_t get_size(shadergraph::DataType type)
{
	switch(type) {
	case shadergraph::DataType::Boolean:
	case shadergraph::DataType::Int:
	case shadergraph::DataType::UInt:
	case shadergraph::DataType::Float:
	case shadergraph::DataType::UInt16:
		return sizeof(float);
	case shadergraph::DataType::Color:
	case shadergraph::DataType::Vector:
	case shadergraph::DataType::Point:
	case shadergraph::DataType::Normal:
		return sizeof(Vector3);
	case shadergraph::DataType::Vector4:
		return sizeof(Vector4);
	case shadergraph::DataType::Point2:
		return sizeof(Vector2);
	case shadergraph::DataType::Transform:
		return sizeof(Mat4);
	}
	return 0;
}

static size_t get_alignment(shadergraph::DataType type)
{
	switch(type) {
	case shadergraph::DataType::Boolean:
	case shadergraph::DataType::Int:
	case shadergraph::DataType::UInt:
	case shadergraph::DataType::Float:
	case shadergraph::DataType::UInt16:
		return sizeof(float);
	case shadergraph::DataType::Color:
	case shadergraph::DataType::Vector:
	case shadergraph::DataType::Vector4:
	case shadergraph::DataType::Point:
	case shadergraph::DataType::Normal:
		return sizeof(Vector4);
	case shadergraph::DataType::Point2:
		return sizeof(Vector2);
	case shadergraph::DataType::Transform:
		return sizeof(Mat4);
	}
	return 0;
}

std::shared_ptr<rendering::shader_material::ShaderMaterial> ShaderGraph::GenerateShaderMaterial()
{
	auto *graph = GetGraph();
	if(!graph)
		return nullptr;
	auto sm = pragma::util::make_shared<rendering::shader_material::ShaderMaterial>(GetIdentifier());
	struct ParamNodeInfo {
		std::string name;
		shadergraph::GraphNode *node;
		size_t alignment;
		size_t size;
	};
	std::vector<ParamNodeInfo> globalParamNodes;
	for(auto &node : graph->GetNodes()) {
		auto *paramNode = dynamic_cast<const rendering::shader_graph::BaseInputParameterNode *>(&node->node);
		if(!paramNode)
			continue;
		rendering::shader_graph::BaseInputParameterNode::Scope scope;
		if(!node->GetInputValue(rendering::shader_graph::BaseInputParameterNode::CONST_SCOPE, scope))
			continue;
		if(scope != rendering::shader_graph::BaseInputParameterNode::Scope::Global)
			continue;
		std::string name;
		if(!node->GetInputValue(rendering::shader_graph::BaseInputParameterNode::CONST_NAME, name))
			continue;
		if(name.empty())
			continue;

		auto *texNode = dynamic_cast<const rendering::shader_graph::InputParameterTextureNode *>(paramNode);
		if(texNode) {
			sm->textures.push_back({});
			auto &tex = sm->textures.back();
			tex.name = name;
			tex.defaultTexturePath = "white";
			node->GetInputValue(rendering::shader_graph::InputParameterTextureNode::CONST_DEFAULT_TEXTURE, *tex.defaultTexturePath);

			auto colorSpace = node->GetConstantInputValue<rendering::shader_graph::InputParameterTextureNode::ColorSpace>(rendering::shader_graph::InputParameterTextureNode::CONST_COLOR_SPACE);
			if(!colorSpace)
				colorSpace = rendering::shader_graph::InputParameterTextureNode::ColorSpace::Srgb;
			switch(*colorSpace) {
			case rendering::shader_graph::InputParameterTextureNode::ColorSpace::Srgb:
				tex.colorMap = true;
				break;
			}

			auto imageType = node->GetConstantInputValue<rendering::shader_graph::InputParameterTextureNode::ImageType>(rendering::shader_graph::InputParameterTextureNode::CONST_IMAGE_TYPE);
			if(!imageType)
				imageType = rendering::shader_graph::InputParameterTextureNode::ImageType::e2D;
			switch(*imageType) {
			case rendering::shader_graph::InputParameterTextureNode::ImageType::Cube:
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
			assert((padding % sizeof(float)) == 0);
			auto paddingName = "padding" + std::to_string(paddingIdx++);
			it = globalParamNodes.insert(it, {paddingName, nullptr, sizeof(float), sizeof(float)});
			continue;
		}
		offset += info->size;
		++it;
	}

	std::vector<rendering::Property> params;
	params.reserve(globalParamNodes.size());
	for(auto &info : globalParamNodes) {
		auto type = shadergraph::DataType::Float;
		if(info.node) {
			auto &paramNode = *dynamic_cast<const rendering::shader_graph::BaseInputParameterNode *>(&info.node->node);
			type = paramNode.GetParameterType();
		}
		rendering::Property prop {info.name, type};
		auto res = true;
		if(info.node) {
			res = pragma::shadergraph::visit(type, [this, &info, &prop](auto tag) -> bool {
				using T = typename decltype(tag)::type;

				T defaultVal;
				if(!info.node->GetInputValue(rendering::shader_graph::BaseInputParameterNode::CONST_DEFAULT, defaultVal))
					return false;
				prop->defaultValue.Set(defaultVal);

				if constexpr(std::is_same_v<T, float>) {
					float minVal;
					float maxVal;
					float stepSize;
					if(!info.node->GetInputValue(rendering::shader_graph::InputParameterFloatNode::CONST_MIN, minVal))
						return false;
					if(!info.node->GetInputValue(rendering::shader_graph::InputParameterFloatNode::CONST_MAX, maxVal))
						return false;
					if(!info.node->GetInputValue(rendering::shader_graph::InputParameterFloatNode::CONST_STEP_SIZE, stepSize))
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

void ShaderGraph::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const
{
	std::array<prosper::IDescriptorSet *, 4> descSets {&dsScene, &dsRenderer, &dsRenderSettings, &dsShadows};

	static const std::vector<uint32_t> dynamicOffsets {};
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), GetSceneDescriptorSetIndex(), descSets, dynamicOffsets);

	ShaderGameWorldLightingPass::RecordBindScene(shaderProcessor, scene, renderer, dsScene, dsRenderer, dsRenderSettings, dsShadows, drawOrigin, inOutSceneFlags);
	PushConstants pushConstants {};
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
bool ShaderGraph::RecordBindMaterial(rendering::ShaderProcessor &shaderProcessor, material::CMaterial &mat) const
{
	if(!ShaderGameWorldLightingPass::RecordBindMaterial(shaderProcessor, mat))
		return false;
	for(auto &mod : m_modules)
		mod->RecordBindMaterial(shaderProcessor, mat);
	return true;
}
bool ShaderGraph::IsTranslucentPipeline(uint32_t pipelineIdx) const { return m_alphaMode != AlphaMode::Opaque; }
