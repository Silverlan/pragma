#ifndef __C_SHADER_PARTICLE_POLYBOARD_HPP__
#define __C_SHADER_PARTICLE_POLYBOARD_HPP__

#include "pragma/rendering/shaders/particles/c_shader_particle_2d_base.hpp"

namespace pragma
{
	class DLLCLIENT ShaderParticlePolyboard
		: public ShaderParticle2DBase
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_COLOR;

		ShaderParticlePolyboard(prosper::Context &context,const std::string &identifier);

#pragma pack(push,1)
		struct GeometryPushConstants
		{
			Vector3 posCam;
			float radius;
			float curvature;
		};
		struct FragmentPushConstants
		{
			int32_t renderFlags;
			float viewportW;
			float viewportH;
		};
#pragma pack(pop)
		bool Draw(Scene &scene,const pragma::CParticleSystemComponent &ps,prosper::Buffer &vertexBuffer,prosper::Buffer &indexBuffer,uint32_t numIndices,float radius,float curvature);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	private:
		bool Draw(Scene &scene,const pragma::CParticleSystemComponent &ps,bool bloom)=delete;
	};
};

// prosper TODO
#if 0
#include "pragma/rendering/shaders/particles/c_shader_particle_base.h"
#include <cmaterial.h>

namespace Shader
{
	class DLLCLIENT ParticlePolyboard
		: public ParticleBase
	{
	public:
		enum class DLLCLIENT Location : uint32_t
		{
			Vertex = 0,
			Color = 1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Vertex = 0,
			Color = 0
		};
	protected:
		ParticlePolyboard(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader="");
		virtual void InitializeShaderPipelines(const Vulkan::Context &context) override;
		virtual void InitializePipelineLayout(const Vulkan::Context &context,std::vector<Vulkan::DescriptorSetLayout> &setLayouts,std::vector<Vulkan::PushConstantRange> &pushConstants) override;
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
		virtual void SetupPipeline(std::size_t pipelineIdx,vk::GraphicsPipelineCreateInfo &info) override;
		virtual bool BeginDraw(Vulkan::CommandBufferObject *cmdBuffer,Vulkan::ShaderPipeline *shaderPipeline) override;
		using ParticleBase::Draw;
	public:
		ParticlePolyboard();
		virtual void InitializeMaterial(Material *mat,bool bReload=false) override;
		void GenerateParticleMapDescriptorSet(Vulkan::DescriptorSet &texture);
		void GenerateAnimationDescriptorSet(Vulkan::DescriptorSet &anim);
		void Draw(Camera &cam,CParticleSystem *particle,Bool bloom,const Vulkan::Buffer &vertexBuffer,const Vulkan::Buffer &indexBuffer,uint32_t indexCount,float radius,float curvature);
		using Base::BeginDraw;
	};
};
#endif
#endif