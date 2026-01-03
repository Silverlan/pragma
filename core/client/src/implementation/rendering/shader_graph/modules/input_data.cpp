// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.module_input_data;

import :client_state;
import :engine;
import :game;
import :rendering.shaders;

using namespace pragma::rendering::shader_graph;

InputDataModule::InputDataModule(ShaderGraph &shader) : ShaderGraphModule {shader} {}
InputDataModule::~InputDataModule() {}

void InputDataModule::InitializeShaderResources()
{
	std::vector<prosper::DescriptorSetInfo::Binding> bindings;

	auto *graph = m_shader.GetGraph();
	if(!graph)
		return;
	bindings.push_back({"GLOBAL_INPUT_DATA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit});

	// TODO
	m_resolvedGraph = std::make_unique<shadergraph::Graph>(*graph);
	m_resolvedGraph->Resolve();
	for(auto &graphNode : m_resolvedGraph->GetNodes()) {
		auto *node = dynamic_cast<const ImageTextureNodeBase *>(&graphNode->node);
		if(!node)
			continue;
		auto texVarName = node->GetTextureVariableName(*graphNode);
		string::to_upper(texVarName);
		bindings.push_back({register_global_string(texVarName), prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit});
		if(m_imageTextureNodes.size() == m_imageTextureNodes.capacity())
			m_imageTextureNodes.reserve(m_imageTextureNodes.size() * 2 + 10);
		m_imageTextureNodes.push_back(graphNode.get());
	}

	m_globalInputDataDsInfo = {"SHADER_GRAPH", bindings};
	ShaderGraphModule::InitializeShaderResources();
}

void InputDataModule::GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode)
{
	//auto &inputDataManager = pragma::get_cgame()->GetGlobalShaderInputDataManager();

	std::ostringstream code;
	//code << "\nlayout(LAYOUT_ID(SHADER_GRAPH, GLOBAL_INPUT_DATA)) uniform GlobalInputData\n";
	//code << "{\n";
	//for(auto &prop : inputDataManager.GetDescriptor().properties) {
	//	if(prop.parameter.name.empty())
	//		continue;
	//	code << "\t" << pragma::shadergraph::to_glsl_type(prop.parameter.type) << " " << prop.parameter.name << ";\n";
	//}
	//code << "} u_globalInputData;\n";

	for(auto *graphNode : m_imageTextureNodes) {
		auto *node = dynamic_cast<const ImageTextureNodeBase *>(&graphNode->node);
		auto texVarName = node->GetTextureVariableName(*graphNode);
		auto texVarNameUpper = texVarName;
		string::to_upper(texVarNameUpper);
		code << "layout(LAYOUT_ID(SHADER_GRAPH, " << texVarNameUpper << ")) uniform sampler2D " << texVarName << ";\n";
	}

	outPrefixCode += code.str();
}

void InputDataModule::InitializeGfxPipelineDescriptorSets()
{
	//auto &inputDataManager = pragma::get_cgame()->GetGlobalShaderInputDataManager();

	// TODO:
	auto &graphManager = get_cengine()->GetShaderGraphManager();
	auto &typeManagers = graphManager.GetShaderGraphTypeManagers();
	auto it = typeManagers.find("object");
	if(it == typeManagers.end())
		return;
	auto &typeManager = *it->second;
	//for(auto &[name, graphData] : typeManager.GetGraphs())
	//	inputDataManager.PopulateProperties(*graphData->GetGraph());
	auto cmd = get_cengine()->GetRenderContext().GetSetupCommandBuffer();
	//inputDataManager.UpdateBufferData(*cmd);
	get_cengine()->GetRenderContext().FlushSetupCommandBuffer();

	m_shader.AddDescriptorSetGroup(m_globalInputDataDsInfo);
	auto &context = get_cengine()->GetRenderContext();
	//

	// TODO: One per shader
	m_globalInputDsg = context.CreateDescriptorSetGroup(m_globalInputDataDsInfo);
	auto &ds = *m_globalInputDsg->GetDescriptorSet(0);
	constexpr uint32_t BINDING_IDX = 0;
	//auto buf = inputDataManager.GetBuffer();
	//if(!buf)
	//	buf = pragma::get_cengine()->GetRenderContext().GetDummyBuffer();
	// TODO: Each shader should have its own global input buffer.
	// Alternativly: Push-constants?
	//ds.SetBindingUniformBuffer(*buf, BINDING_IDX);

	// Image texture nodes
	auto &texManager = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager()).GetTextureManager();
	uint32_t bindingIdx = 1; // Binding 0 is global input data
	for(auto *node : m_imageTextureNodes) {
		std::string fileName;
		node->GetInputValue<std::string>(ImageTextureNode::IN_FILENAME, fileName);
		auto tex = texManager.LoadAsset(fileName);
		std::shared_ptr<prosper::Texture> prosperTex;
		if(tex)
			prosperTex = tex->GetVkTexture();
		if(!prosperTex)
			prosperTex = context.GetDummyTexture();
		ds.SetBindingTexture(*prosperTex, bindingIdx++);
	}
}
void InputDataModule::RecordBindScene(ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), m_globalInputDataDsInfo.setIndex, *m_globalInputDsg->GetDescriptorSet());
}
