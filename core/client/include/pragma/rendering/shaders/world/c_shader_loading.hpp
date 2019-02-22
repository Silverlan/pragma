#ifndef __C_SHADER_LOADING_H__
#define __C_SHADER_LOADING_H__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"

namespace pragma
{
	class DLLCLIENT ShaderLoading
		: public ShaderTextured3DBase
	{
	public:
		ShaderLoading(prosper::Context &context,const std::string &identifier);
		virtual bool BindMaterial(CMaterial &mat) override;
	};
};

// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"

namespace Shader
{
	class DLLCLIENT Loading
		: public Textured3D
	{
	protected:
		virtual void InitializeMaterialBindings(std::vector<Vulkan::DescriptorSetLayout::Binding> &bindings) override;
		virtual void BindMaterialParameters(Material *mat) override;
	public:
		Loading();
		virtual void InitializeMaterial(Material *mat,bool bReload=false) override;
		virtual void BindMaterial(Material *mat) override;
	};
};
#endif
#endif