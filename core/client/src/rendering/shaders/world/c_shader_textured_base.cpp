#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_textured_base.hpp"

using namespace pragma;

std::shared_ptr<prosper::DescriptorSetGroup> pragma::ShaderTexturedBase::InitializeMaterialDescriptorSet(CMaterial &mat,bool bReload)
{
	auto &shader = *dynamic_cast<prosper::Shader*>(this);
	auto descSetGroup = mat.GetDescriptorSetGroup(shader);
	if(descSetGroup != nullptr)
	{
		if(bReload == false)
			return descSetGroup;
		mat.SetDescriptorSetGroup(shader,nullptr);
	}
	return InitializeMaterialDescriptorSet(mat);
}

std::shared_ptr<prosper::DescriptorSetGroup> pragma::ShaderTexturedBase::InitializeMaterialDescriptorSet(CMaterial &mat) {return nullptr;}
