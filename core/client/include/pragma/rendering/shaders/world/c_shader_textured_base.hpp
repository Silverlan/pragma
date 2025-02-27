/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
