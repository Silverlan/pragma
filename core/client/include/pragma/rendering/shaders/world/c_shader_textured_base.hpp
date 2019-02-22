#ifndef __C_SHADER_TEXTURED_BASE_HPP__
#define __C_SHADER_TEXTURED_BASE_HPP__

#include "pragma/clientdefinitions.h"

class CMaterial;
namespace pragma
{
	class DLLCLIENT ShaderTexturedBase
	{
	public:
		virtual std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat,bool bReload);
	protected:
		virtual std::shared_ptr<prosper::DescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat);
	};
};

#endif
