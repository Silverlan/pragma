/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

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

		ShaderDebug(prosper::IPrContext &context,const std::string &identifier);

		bool BeginDraw(const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,Pipeline pipelineIdx=Pipeline::Triangle);
		bool Draw(prosper::IBuffer &vertexBuffer,uint32_t vertexCount,const Mat4 &mvp=umat::identity(),const Vector4 &color=Vector4(1.f,1.f,1.f,1.f));
	protected:
		ShaderDebug(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		bool Draw(const std::vector<prosper::IBuffer*> &buffers,uint32_t vertexCount,const Mat4 &mvp,const Vector4 &color=Vector4(1.f,1.f,1.f,1.f));
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		virtual bool ShouldInitializePipeline(uint32_t pipelineIdx) override;
	private:
		// These are unused
		virtual bool BindSceneCamera(pragma::CSceneComponent &scene,const CRasterizationRendererComponent &renderer,bool bView) override {return false;}
		virtual bool BindRenderSettings(prosper::IDescriptorSet &descSetRenderSettings) override {return false;}
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

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		ShaderDebugTexture(prosper::IPrContext &context,const std::string &identifier);
		bool Draw(prosper::IDescriptorSet &descSetTexture,const ShaderDebug::PushConstants &pushConstants);
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	private:
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
		virtual uint32_t GetCameraDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
	};

	/////////////////////

	class DLLCLIENT ShaderDebugVertexColor
		: public ShaderDebug
	{
	public:
		ShaderDebugVertexColor(prosper::IPrContext &context,const std::string &identifier);

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_COLOR;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_COLOR;

		bool Draw(prosper::IBuffer &vertexBuffer,prosper::IBuffer &colorBuffer,uint32_t vertexCount,const Mat4 &modelMatrix=umat::identity());
	protected:
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
