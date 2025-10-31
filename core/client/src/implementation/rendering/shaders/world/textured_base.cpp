// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client;


import :rendering.shaders.textured_base;

using namespace pragma;

std::shared_ptr<prosper::IDescriptorSetGroup> pragma::ShaderTexturedBase::InitializeMaterialDescriptorSet(msys::CMaterial &mat, bool bReload)
{
	auto &shader = *dynamic_cast<prosper::Shader *>(this);
	auto descSetGroup = mat.GetDescriptorSetGroup(shader);
	if(descSetGroup != nullptr) {
		if(bReload == false)
			return descSetGroup;
		mat.SetDescriptorSetGroup(shader, nullptr);
	}
	return InitializeMaterialDescriptorSet(mat);
}

std::shared_ptr<prosper::IDescriptorSetGroup> pragma::ShaderTexturedBase::InitializeMaterialDescriptorSet(msys::CMaterial &mat) { return nullptr; }
