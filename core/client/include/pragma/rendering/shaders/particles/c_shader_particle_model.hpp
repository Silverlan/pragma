#ifndef __C_SHADER_PARTICLE_MODEL_HPP__
#define __C_SHADER_PARTICLE_MODEL_HPP__

#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/particles/c_shader_particle_base.hpp"

namespace pragma
{
	class DLLCLIENT ShaderParticleModel
		: public ShaderTextured3DBase,
		public ShaderParticleBase
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_PARTICLE;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_PARTICLE;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_COLOR;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_ROTATION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ROTATION;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_ANIMATION_START;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_ANIMATION_START;

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_ANIMATION;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_BONE_MATRICES;

#pragma pack(push,1)
		struct PushConstants
		{
			uint32_t renderFlags;
			uint32_t alphaMode;
		};
#pragma pack(pop)

		ShaderParticleModel(prosper::Context &context,const std::string &identifier);
		bool BindParticleBuffers(prosper::Buffer &particleBuffer,prosper::Buffer &rotBuffer,prosper::Buffer &animStartBuffer);
		bool Draw(CModelSubMesh &mesh,uint32_t numInstances,uint32_t firstInstance=0u);
		bool BindParticleSystem(pragma::CParticleSystemComponent &pSys);

		bool BeginDraw(
			const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,
			pragma::CParticleSystemComponent &pSys,Pipeline pipelineIdx=Pipeline::Regular,
			ShaderScene::RecordFlags recordFlags=ShaderScene::RecordFlags::RenderPassTargetAsViewportAndScissor
		);
	protected:
		virtual prosper::Shader::DescriptorSetInfo &GetAnimationDescriptorSetInfo() const override;
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual bool ShouldInitializePipeline(uint32_t pipelineIdx) override;
	private:
		std::shared_ptr<prosper::DescriptorSetGroup> m_dummyAnimDescSetGroup = nullptr;
		// These are unused
		virtual bool Draw(CModelSubMesh &mesh) override {return false;};
	};
};

// prosper TODO
#if 0
#include "pragma/rendering/shaders/world/c_shader_textured.h"

namespace Shader
{
	class DLLCLIENT ParticleModel
		: public Textured3D
	{
	public:
		ParticleModel();
		//using ParticleBase::Draw;
		enum class DLLCLIENT Location : uint32_t
		{
			Xyzs = umath::to_integral(TexturedBase3D::Location::BiTangent) +1,
			Color = Xyzs +1,
			Rotation = Color +1
		};
		enum class DLLCLIENT Binding : uint32_t
		{
			Xyzs = umath::to_integral(TexturedBase3D::Binding::BoneWeight) +1,
			Color = Xyzs,
			Rotation = 3
		};
		void Draw(CModelSubMesh *mesh,uint32_t instanceCount);
		bool BeginDraw(Vulkan::BufferObject *particleBuffer,Vulkan::BufferObject *rotBuffer,Vulkan::CommandBufferObject *cmdBuffer);
	protected:
		virtual void InitializeVertexDescriptions(std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions) override;
	};
};
#endif
#endif
