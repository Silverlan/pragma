#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_unlit.hpp"
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;


decltype(ShaderUnlit::DESCRIPTOR_SET_MATERIAL) ShaderUnlit::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Material settings
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::VERTEX_BIT | Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::GEOMETRY_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Albedo Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderUnlit::ShaderUnlit(prosper::Context &context,const std::string &identifier)
	: ShaderTextured3DBase{context,identifier,"world/vs_textured","world/fs_unlit"}
{
	SetPipelineCount(umath::to_integral(Pipeline::Count));
}
prosper::Shader::DescriptorSetInfo &ShaderUnlit::GetMaterialDescriptorSetInfo() const {return DESCRIPTOR_SET_MATERIAL;}
std::shared_ptr<prosper::DescriptorSetGroup> ShaderUnlit::InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::Shader::DescriptorSetInfo &descSetInfo)
{
	auto &dev = c_engine->GetDevice();
	auto *albedoMap = mat.GetDiffuseMap();
	if(albedoMap == nullptr || albedoMap->texture == nullptr)
		return nullptr;
	auto albedoTexture = std::static_pointer_cast<Texture>(albedoMap->texture);
	if(albedoTexture->HasValidVkTexture() == false)
		return nullptr;
	auto descSetGroup = prosper::util::create_descriptor_set_group(dev,descSetInfo);
	mat.SetDescriptorSetGroup(*this,descSetGroup);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	prosper::util::set_descriptor_set_binding_texture(descSet,*albedoTexture->GetVkTexture(),umath::to_integral(MaterialBinding::AlbedoMap));
	InitializeMaterialBuffer(descSet,mat);

	// TODO: FIXME: It would probably be a good idea to update the descriptor set lazily (i.e. not update it here), but
	// that seems to cause crashes in some cases
	if(descSet->update() == false)
		return false;
	return descSetGroup;
}
std::shared_ptr<prosper::DescriptorSetGroup> ShaderUnlit::InitializeMaterialDescriptorSet(CMaterial &mat)
{
	return InitializeMaterialDescriptorSet(mat,DESCRIPTOR_SET_MATERIAL);
}

