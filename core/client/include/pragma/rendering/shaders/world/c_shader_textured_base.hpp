// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_SHADER_TEXTURED_BASE_HPP__
#define __C_SHADER_TEXTURED_BASE_HPP__

#include "pragma/clientdefinitions.h"
#include "prosper_descriptor_set_group.hpp"

class CMaterial;
namespace pragma {
	class DLLCLIENT ShaderTexturedBase {
	  public:
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat, bool bReload);
	  protected:
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat);
	};
};

#endif
