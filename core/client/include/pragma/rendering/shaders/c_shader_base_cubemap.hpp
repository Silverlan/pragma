#ifndef __C_SHADER_BASE_CUBEMAP_HPP__
#define __C_SHADER_BASE_CUBEMAP_HPP__

#include "pragma/clientdefinitions.h"
#include <mathutil/umath.h>
#include <mathutil/umat.h>
#include <shader/prosper_shader.hpp>

namespace prosper
{
	class RenderTarget; class Image;
	namespace util
	{
		struct TextureCreateInfo;
		struct SamplerCreateInfo;
	};
};
namespace pragma
{
	class DLLCLIENT ShaderCubemap
		: public prosper::ShaderGraphics
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

#pragma pack(push,1)
		struct PushConstants
		{
			Mat4 projection;
			Mat4 view;
		};
#pragma pack(pop)
	protected:
		ShaderCubemap(prosper::Context &context,const std::string &identifier,const std::string &vertexShader,const std::string &fragmentShader);
		ShaderCubemap(prosper::Context &context,const std::string &identifier,const std::string &fragmentShader);
		std::shared_ptr<prosper::Buffer> CreateCubeMesh(uint32_t &outNumVerts) const;
		std::shared_ptr<prosper::Image> CreateCubeMap(uint32_t width,uint32_t height,prosper::util::ImageCreateInfo::Flags flags=prosper::util::ImageCreateInfo::Flags::None) const;
		std::shared_ptr<prosper::RenderTarget> CreateCubeMapRenderTarget(uint32_t width,uint32_t height,prosper::util::ImageCreateInfo::Flags flags=prosper::util::ImageCreateInfo::Flags::None) const;
		static void InitializeSamplerCreateInfo(prosper::util::ImageCreateInfo::Flags flags,prosper::util::SamplerCreateInfo &inOutSamplerCreateInfo);
		static void InitializeTextureCreateInfo(prosper::util::TextureCreateInfo &inOutTextureCreateInfo);
		const Mat4 &GetProjectionMatrix(float aspectRatio) const;
		const Mat4 &GetViewMatrix(uint8_t layerId) const;
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
	};
};

#endif
