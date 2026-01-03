// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.shaders.textured_base;

export import pragma.cmaterialsystem;

export namespace pragma {
	class DLLCLIENT ShaderTexturedBase {
	  public:
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(material::CMaterial &mat, bool bReload);
	  protected:
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(material::CMaterial &mat);
	};
};
