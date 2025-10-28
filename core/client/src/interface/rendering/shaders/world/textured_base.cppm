// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "prosper_descriptor_set_group.hpp"

export module pragma.client:rendering.shaders.textured_base;

export namespace pragma {
	class DLLCLIENT ShaderTexturedBase {
	  public:
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat, bool bReload);
	  protected:
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat);
	};
};
