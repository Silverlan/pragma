#ifndef __C_SHADER_UNLIT_HPP__
#define __C_SHADER_UNLIT_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <mathutil/uvec.h>

namespace pragma
{
	class DLLCLIENT ShaderUnlit
		: public ShaderTextured3DBase
	{
	public:
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

		enum class MaterialBinding : uint32_t
		{
			MaterialSettings = umath::to_integral(ShaderTextured3DBase::MaterialBinding::MaterialSettings),
			AlbedoMap,

			Count
		};

		ShaderUnlit(prosper::Context &context,const std::string &identifier);

		virtual std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
	protected:
		virtual prosper::Shader::DescriptorSetInfo &GetMaterialDescriptorSetInfo() const override;
		std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat,const prosper::Shader::DescriptorSetInfo &descSetInfo);
	};
};

#endif
