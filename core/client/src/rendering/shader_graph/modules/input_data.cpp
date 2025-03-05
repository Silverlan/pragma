/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/rendering/shader_graph/modules/input_data.hpp"
#include "pragma/rendering/shader_graph/nodes/image_texture.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/shader_material/shader_material.hpp"
#include "pragma/rendering/global_shader_input_manager.hpp"
#include "pragma/rendering/shader_graph/manager.hpp"
#include "pragma/rendering/shaders/world/c_shader_graph.hpp"
#include <pragma/util/global_string_table.hpp>
#include <cmaterial_manager2.hpp>
#include <texturemanager/texture_manager2.hpp>
#include <texturemanager/texture.h>
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_buffer_create_info.hpp>
#include <prosper_command_buffer.hpp>

using namespace pragma::rendering::shader_graph;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

InputDataModule::InputDataModule(ShaderGraph &shader) : pragma::rendering::ShaderGraphModule {shader} {}
InputDataModule::~InputDataModule() {}

void InputDataModule::InitializeShaderResources()
{
	std::vector<prosper::DescriptorSetInfo::Binding> bindings;

	auto *graph = m_shader.GetGraph();
	if(!graph)
		return;
	bindings.push_back({"GLOBAL_INPUT_DATA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit});

	// TODO
	m_resolvedGraph = std::make_unique<pragma::shadergraph::Graph>(*graph);
	m_resolvedGraph->Resolve();
	for(auto &graphNode : m_resolvedGraph->GetNodes()) {
		auto *node = dynamic_cast<const ImageTextureNodeBase *>(&graphNode->node);
		if(!node)
			continue;
		auto texVarName = node->GetTextureVariableName(*graphNode);
		ustring::to_upper(texVarName);
		bindings.push_back({pragma::register_global_string(texVarName), prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit});
		if(m_imageTextureNodes.size() == m_imageTextureNodes.capacity())
			m_imageTextureNodes.reserve(m_imageTextureNodes.size() * 2 + 10);
		m_imageTextureNodes.push_back(graphNode.get());
	}

	m_globalInputDataDsInfo = {"SHADER_GRAPH", bindings};
	ShaderGraphModule::InitializeShaderResources();
}

void InputDataModule::GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode)
{
	//auto &inputDataManager = c_game->GetGlobalShaderInputDataManager();

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
		ustring::to_upper(texVarNameUpper);
		code << "layout(LAYOUT_ID(SHADER_GRAPH, " << texVarNameUpper << ")) uniform sampler2D " << texVarName << ";\n";
	}

	outPrefixCode += code.str();
}

void InputDataModule::InitializeGfxPipelineDescriptorSets()
{
	//auto &inputDataManager = c_game->GetGlobalShaderInputDataManager();

	// TODO:
	auto &graphManager = c_engine->GetShaderGraphManager();
	auto &typeManagers = graphManager.GetShaderGraphTypeManagers();
	auto it = typeManagers.find("object");
	if(it == typeManagers.end())
		return;
	auto &typeManager = *it->second;
	//for(auto &[name, graphData] : typeManager.GetGraphs())
	//	inputDataManager.PopulateProperties(*graphData->GetGraph());
	auto cmd = c_engine->GetRenderContext().GetSetupCommandBuffer();
	//inputDataManager.UpdateBufferData(*cmd);
	c_engine->GetRenderContext().FlushSetupCommandBuffer();

	m_shader.AddDescriptorSetGroup(m_globalInputDataDsInfo);
	auto &context = c_engine->GetRenderContext();
	//

	// TODO: One per shader
	m_globalInputDsg = context.CreateDescriptorSetGroup(m_globalInputDataDsInfo);
	auto &ds = *m_globalInputDsg->GetDescriptorSet(0);
	constexpr uint32_t BINDING_IDX = 0;
	//auto buf = inputDataManager.GetBuffer();
	//if(!buf)
	//	buf = c_engine->GetRenderContext().GetDummyBuffer();
	// TODO: Each shader should have its own global input buffer.
	// Alternativly: Push-constants?
	//ds.SetBindingUniformBuffer(*buf, BINDING_IDX);

	// Image texture nodes
	auto &texManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager()).GetTextureManager();
	uint32_t bindingIdx = 1; // Binding 0 is global input data
	for(auto *node : m_imageTextureNodes) {
		std::string fileName;
		node->GetInputValue<std::string>(pragma::rendering::shader_graph::ImageTextureNode::IN_FILENAME, fileName);
		auto tex = texManager.LoadAsset(fileName);
		std::shared_ptr<prosper::Texture> prosperTex;
		if(tex)
			prosperTex = tex->GetVkTexture();
		if(!prosperTex)
			prosperTex = context.GetDummyTexture();
		ds.SetBindingTexture(*prosperTex, bindingIdx++);
	}
}
void InputDataModule::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), m_globalInputDataDsInfo.setIndex, *m_globalInputDsg->GetDescriptorSet());
}
