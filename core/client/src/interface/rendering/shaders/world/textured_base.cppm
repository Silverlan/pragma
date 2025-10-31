// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:rendering.shaders.textured_base;

export namespace pragma {
	class DLLCLIENT ShaderTexturedBase {
	  public:
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(msys::CMaterial &mat, bool bReload);
	  protected:
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(msys::CMaterial &mat);
	};
};
