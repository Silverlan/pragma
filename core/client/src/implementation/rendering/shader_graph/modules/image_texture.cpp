// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shader_graph.module_image_texture;

import pragma.shadergraph;

import :client_state;
import :engine;

using namespace pragma::rendering::shader_graph;

#pragma optimize("", off)
ImageTextureModule::ImageTextureModule(ShaderGraph &shader) : ShaderGraphModule {shader}
{
	//Global settings +textures?
	/* prosper::PrDescriptorSetBindingFlags::Cubemap */
}
ImageTextureModule::~ImageTextureModule() {}
void ImageTextureModule::InitializeGfxPipelineDescriptorSets()
{
	// TODO: Move this somewhere else
	/*std::vector<prosper::DescriptorSetInfo::Binding> bindings;
	bindings.reserve(m_nodes.size());
	for(auto *node : m_nodes) {
		auto texName = node->GetBaseVarName() + "_tex";
		pragma::string::to_upper(texName);
		prosper::DescriptorSetInfo::Binding binding {pragma::register_global_string(texName), prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::None};
		bindings.push_back(binding);
	}
	m_descSetInfo = {
	  "TEST",
	  bindings,
	};*/

	//

	//m_shader.AddDescriptorSetGroup(m_descSetInfo);

	/*auto &context = pragma::get_cengine()->GetRenderContext();
	auto dsg = context.CreateDescriptorSetGroup(m_descSetInfo);
	auto &ds = *dsg->GetDescriptorSet(0);
	auto &texManager = static_cast<material::CMaterialManager &>(pragma::get_client_state()->GetMaterialManager()).GetTextureManager();
	uint32_t bindingIdx = 0;
	for(auto *node : m_nodes) {
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

	m_dsg = dsg;*/
}
void ImageTextureModule::RecordBindScene(ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	//shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), m_descSetInfo.setIndex, *m_dsg->GetDescriptorSet());
}
