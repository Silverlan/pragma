#ifndef __C_SHADER_PREPASS_HPP__
#define __C_SHADER_PREPASS_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderPrepassBase
		: public ShaderEntity
	{
	public:
		enum class Pipeline : uint32_t
		{
			Regular = umath::to_integral(ShaderEntity::Pipeline::Regular),
			MultiSample = umath::to_integral(ShaderEntity::Pipeline::MultiSample),

			Reflection = umath::to_integral(ShaderEntity::Pipeline::Count),
			Count
		};
		static Pipeline GetPipelineIndex(Anvil::SampleCountFlagBits sampleCount);
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;

#pragma pack(push,1)
		struct PushConstants
		{
			Vector4 clipPlane;
			uint32_t vertexAnimInfo;
		};
#pragma pack(pop)

		ShaderPrepassBase(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		ShaderPrepassBase(prosper::Context &context,const std::string &identifier);

		bool BeginDraw(const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,Pipeline pipelineIdx=Pipeline::Regular);
		bool BindClipPlane(const Vector4 &clipPlane);
	protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;

		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
	private:
		// These are unused
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
		virtual uint32_t GetLightDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
		virtual bool BindRenderSettings(Anvil::DescriptorSet &descSetRenderSettings) override {return false;}
		virtual bool BindLights(Anvil::DescriptorSet &descSetShadowMaps,Anvil::DescriptorSet &descSetLightSources) override {return false;}
	};
	using ShaderPrepassDepth = ShaderPrepassBase;

	//////////////////

	class DLLCLIENT ShaderPrepass
		: public ShaderPrepassBase
	{
	public:
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_NORMAL;

		static Anvil::Format RENDER_PASS_NORMAL_FORMAT;

		ShaderPrepass(prosper::Context &context,const std::string &identifier);
	protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::RenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif