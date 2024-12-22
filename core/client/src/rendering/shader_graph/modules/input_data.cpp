/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/modules/input_data.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/rendering/shader_material/shader_material.hpp"
#include "pragma/rendering/global_shader_input_manager.hpp"
#include "pragma/rendering/shader_graph/manager.hpp"
#include <buffers/prosper_buffer.hpp>
#include <buffers/prosper_buffer_create_info.hpp>
#include <prosper_command_buffer.hpp>

using namespace pragma::rendering::shader_graph;

extern DLLCLIENT CEngine *c_engine;

static std::unique_ptr<pragma::rendering::GlobalShaderInputDataManager> g_globalShaderInputDataManager {};
size_t InputDataModule::g_instanceCount = 0;
InputDataModule::InputDataModule(prosper::Shader &shader) : pragma::rendering::ShaderGraphModule {shader}
{
	if(g_instanceCount == 0)
		g_globalShaderInputDataManager = std::make_unique<pragma::rendering::GlobalShaderInputDataManager>();
	m_globalInputDataDsInfo = {
	  "SHADER_GRAPH",
	  {prosper::DescriptorSetInfo::Binding {"GLOBAL_INPUT_DATA", prosper::DescriptorType::UniformBuffer, prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit}},
	};
}
InputDataModule::~InputDataModule()
{
	if(--g_instanceCount == 0) {
		g_globalShaderInputDataManager = nullptr;
	}
}

void pragma::rendering::shader_graph::InputDataModule::set_shader_input_value(const std::string &name, float val)
{
	g_globalShaderInputDataManager->SetValue<float>(name, val);

	auto cmd = c_engine->GetRenderContext().GetSetupCommandBuffer();
	g_globalShaderInputDataManager->UpdateBufferData(*cmd);
	c_engine->GetRenderContext().FlushSetupCommandBuffer();
}

void InputDataModule::GetShaderPreprocessorDefinitions(std::unordered_map<std::string, std::string> &outDefinitions, std::string &outPrefixCode)
{
	std::ostringstream code;
	code << "\nlayout(LAYOUT_ID(SHADER_GRAPH, GLOBAL_INPUT_DATA)) uniform GlobalInputData\n";
	code << "{\n";
	for(auto &prop : g_globalShaderInputDataManager->GetDescriptor().properties) {
		if(prop.parameter.name.empty())
			continue;
		code << "\t" << pragma::shadergraph::to_glsl_type(prop.parameter.type) << " " << prop.parameter.name << ";\n";
	}
	code << "} u_globalInputData;\n";
	outPrefixCode += code.str();
}

void InputDataModule::InitializeGfxPipelineDescriptorSets()
{
	// TODO:
	auto testPbr = c_engine->GetShaderGraphManager().GetGraph("z");
	g_globalShaderInputDataManager->PopulateProperties(*testPbr->GetGraph());
	auto cmd = c_engine->GetRenderContext().GetSetupCommandBuffer();
	g_globalShaderInputDataManager->UpdateBufferData(*cmd);
	c_engine->GetRenderContext().FlushSetupCommandBuffer();

	m_shader.AddDescriptorSetGroup(m_globalInputDataDsInfo);
	auto &context = c_engine->GetRenderContext();
	//

	// TODO: One per shader
	m_globalInputDsg = context.CreateDescriptorSetGroup(m_globalInputDataDsInfo);
	auto &dummyTex = context.GetDummyTexture();
	auto &dummyCubemapTex = context.GetDummyCubemapTexture();
	auto &ds = *m_globalInputDsg->GetDescriptorSet(0);
	constexpr uint32_t BINDING_IDX = 0;
	auto buf = g_globalShaderInputDataManager->GetBuffer();
	if(!buf)
		buf = c_engine->GetRenderContext().GetDummyBuffer();
	ds.SetBindingUniformBuffer(*buf, BINDING_IDX);
}
void InputDataModule::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), m_globalInputDataDsInfo.setIndex, *m_globalInputDsg->GetDescriptorSet());
}
