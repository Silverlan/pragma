/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_unlit.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;


decltype(ShaderUnlit::DESCRIPTOR_SET_MATERIAL) ShaderUnlit::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::DescriptorSetInfo::Binding { // Material settings
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::GeometryBit
		},
		prosper::DescriptorSetInfo::Binding { // Albedo Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderUnlit::ShaderUnlit(prosper::IPrContext &context,const std::string &identifier)
	: ShaderTextured3DBase{context,identifier,"world/vs_textured","world/fs_unlit"}
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}
prosper::DescriptorSetInfo &ShaderUnlit::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
bool ShaderUnlit::BindLights(prosper::IDescriptorSet &dsLights) {return true;}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderUnlit::InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::DescriptorSetInfo &descSetInfo)
{
	auto *albedoMap = mat.GetDiffuseMap();
	if(albedoMap == nullptr || albedoMap->texture == nullptr)
		return nullptr;
	auto albedoTexture = std::static_pointer_cast<Texture>(albedoMap->texture);
	if(albedoTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(descSetInfo);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingTexture(*albedoTexture->GetVkTexture(),umath::to_integral(MaterialBinding::AlbedoMap));
	InitializeMaterialBuffer(descSet,mat);

	// TODO: FIXME: It would probably be a good idea to update the descriptor set lazily (i.e. not update it here), but
	// that seems to cause crashes in some cases
	if(descSet.Update() == false)
		return false;
	return descSetGroup;
}
std::shared_ptr<prosper::IDescriptorSetGroup> ShaderUnlit::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	return InitializeMaterialDescriptorSet(mat,DESCRIPTOR_SET_MATERIAL);
}

