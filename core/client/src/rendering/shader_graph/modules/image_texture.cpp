/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shader_graph/modules/image_texture.hpp"
#include "pragma/rendering/shader_graph/nodes/image_texture.hpp"
#include "pragma/rendering/render_processor.hpp"
#include <pragma/util/global_string_table.hpp>
#include <cmaterial_manager2.hpp>
#include <texturemanager/texture_manager2.hpp>
#include <texturemanager/texture.h>
#include <prosper_command_buffer.hpp>

import pragma.shadergraph;

using namespace pragma::rendering::shader_graph;

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
#pragma optimize("", off)
ImageTextureModule::ImageTextureModule(ShaderGraph &shader) : pragma::rendering::ShaderGraphModule {shader}
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
		ustring::to_upper(texName);
		prosper::DescriptorSetInfo::Binding binding {pragma::register_global_string(texName), prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::None};
		bindings.push_back(binding);
	}
	m_descSetInfo = {
	  "TEST",
	  bindings,
	};*/

	//

	//m_shader.AddDescriptorSetGroup(m_descSetInfo);

	/*auto &context = c_engine->GetRenderContext();
	auto dsg = context.CreateDescriptorSetGroup(m_descSetInfo);
	auto &ds = *dsg->GetDescriptorSet(0);
	auto &texManager = static_cast<msys::CMaterialManager &>(client->GetMaterialManager()).GetTextureManager();
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
void ImageTextureModule::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const pragma::CSceneComponent &scene, const pragma::CRasterizationRendererComponent &renderer, ShaderGameWorld::SceneFlags &inOutSceneFlags) const
{
	//shaderProcessor.GetCommandBuffer().RecordBindDescriptorSets(prosper::PipelineBindPoint::Graphics, shaderProcessor.GetCurrentPipelineLayout(), m_descSetInfo.setIndex, *m_dsg->GetDescriptorSet());
}
