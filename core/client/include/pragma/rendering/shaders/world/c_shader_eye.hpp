#ifndef __C_SHADER_EYE_HPP__
#define __C_SHADER_EYE_HPP__

#include "pragma/rendering/shaders/world/c_shader_pbr.hpp"

namespace pragma
{
	class DLLCLIENT ShaderEye
		: public ShaderPBR
	{
	public:
#pragma pack(push,1)
		struct PushConstants
		{
			Vector4 irisProjectionU = {};
			Vector4 irisProjectionV = {};
			Vector3 eyeballOrigin = {};
		};
#pragma pack(pop)

		ShaderEye(prosper::Context &context,const std::string &identifier);
		virtual bool Draw(CModelSubMesh &mesh) override;
	protected:
		bool BindEyeball(uint32_t skinMatIdx);
		virtual void InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
