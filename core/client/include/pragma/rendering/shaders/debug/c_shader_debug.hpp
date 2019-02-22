#ifndef __C_SHADER_DEBUG_HPP__
#define __C_SHADER_DEBUG_HPP__

#include "pragma/rendering/shaders/world/c_shader_scene.hpp"

namespace pragma
{
	class DLLCLIENT ShaderDebug
		: public ShaderScene
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

#pragma pack(push,1)
		struct PushConstants
		{
			Mat4 MVP;
			Vector4 color;
		};
#pragma pack(pop)

		enum class Pipeline : uint32_t
		{
			Triangle,
			Line,
			Wireframe,
			LineStrip,
			Point,
			Vertex,

			Count
		};

		ShaderDebug(prosper::Context &context,const std::string &identifier);

		bool BeginDraw(const std::shared_ptr<prosper::PrimaryCommandBuffer> &cmdBuffer,Pipeline pipelineIdx=Pipeline::Triangle);
		bool Draw(Anvil::Buffer &vertexBuffer,uint32_t vertexCount,const Mat4 &mvp=umat::identity(),const Vector4 &color=Vector4(1.f,1.f,1.f,1.f));
	protected:
		ShaderDebug(prosper::Context &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		bool Draw(const std::vector<Anvil::Buffer*> &buffers,uint32_t vertexCount,const Mat4 &mvp,const Vector4 &color=Vector4(1.f,1.f,1.f,1.f));
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual bool ShouldInitializePipeline(uint32_t pipelineIdx) override;
	private:
		// These are unused
		virtual bool BindSceneCamera(const Scene &scene,bool bView) override {return false;}
		virtual bool BindRenderSettings(Anvil::DescriptorSet &descSetRenderSettings) override {return false;}
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
		virtual uint32_t GetCameraDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
	};

	/////////////////////

	class DLLCLIENT ShaderDebugTexture
		: public ShaderScene
	{
	public:
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;

		static prosper::Shader::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		ShaderDebugTexture(prosper::Context &context,const std::string &identifier);
		bool Draw(Anvil::DescriptorSet &descSetTexture,const ShaderDebug::PushConstants &pushConstants);
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	private:
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
		virtual uint32_t GetCameraDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
	};

	/////////////////////

	class DLLCLIENT ShaderDebugVertexColor
		: public ShaderDebug
	{
	public:
		ShaderDebugVertexColor(prosper::Context &context,const std::string &identifier);

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_COLOR;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_COLOR;

		bool Draw(Anvil::Buffer &vertexBuffer,Anvil::Buffer &colorBuffer,uint32_t vertexCount,const Mat4 &modelMatrix=umat::identity());
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
