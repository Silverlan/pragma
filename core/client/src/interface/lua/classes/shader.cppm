// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.classes.shader;

export import :rendering.shaders.lua;
export import :scripting.lua.libraries.vulkan;

export {
	// LuaShaderRecordTarget converter
	struct LuaShaderRecordTarget {
		luabind::object target;
		prosper::ShaderBindState *GetBindState() const;
		prosper::util::PreparedCommandBuffer *GetPcb() const;
	};
	namespace luabind {
		template<>
		struct default_converter<LuaShaderRecordTarget> : native_converter_base<LuaShaderRecordTarget> {
			enum { consumed_args = 1 };

			template<typename U>
			LuaShaderRecordTarget to_cpp(lua::State *L, U u, int index);

			template<class U>
			static int match(lua::State *l, U u, int index);

			template<class U>
			void converter_postcall(lua::State *, U u, int)
			{
			}

			void to_lua(lua::State *L, LuaShaderRecordTarget const &x);
			void to_lua(lua::State *L, LuaShaderRecordTarget *x);
		  public:
			static value_type to_cpp_deferred(lua::State *, int) { return {}; }
			static void to_lua_deferred(lua::State *, param_type) {}
			static int compute_score(lua::State *, int) { return no_match; }
		};

		template<>
		struct default_converter<const LuaShaderRecordTarget> : default_converter<LuaShaderRecordTarget> {};

		template<>
		struct default_converter<LuaShaderRecordTarget const &> : default_converter<LuaShaderRecordTarget> {};

		template<>
		struct default_converter<LuaShaderRecordTarget &&> : default_converter<LuaShaderRecordTarget> {};
	}

	namespace luabind {
		template<typename U>
		LuaShaderRecordTarget default_converter<LuaShaderRecordTarget>::to_cpp(lua::State *L, U u, int index)
		{
			return LuaShaderRecordTarget {object {from_stack(L, index)}};
		}

		template<class U>
		int default_converter<LuaShaderRecordTarget>::match(lua::State *l, U u, int index)
		{
			return (Lua::IsType<prosper::ShaderBindState>(l, index) || Lua::IsType<prosper::util::PreparedCommandBuffer>(l, index)) ? 1 : no_match;
		}
	}
	//

	namespace Lua {
		namespace GraphicsPipelineCreateInfo {
			DLLCLIENT void SetBlendingProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, const ::Vector4 &blendingProperties);
			DLLCLIENT void SetCommonAlphaBlendProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void SetColorBlendAttachmentProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t attId, bool blendingEnabled, uint32_t blendOpColor, uint32_t blendOpAlpha, uint32_t srcColorBlendFactor, uint32_t dstColorBlendFactor,
			  uint32_t srcAlphaBlendFactor, uint32_t dstAlphaBlendFactor, uint32_t channelWriteMask);
			DLLCLIENT void SetMultisamplingProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t sampleCount, float sampleShading, uint32_t sampleMask);
			DLLCLIENT void SetSampleCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t sampleCount);
			DLLCLIENT void SetMinSampleShading(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float sampleShading);
			DLLCLIENT void SetSampleMask(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t sampleMask);
			DLLCLIENT void SetDynamicScissorBoxesCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t numDynamicScissorBoxes);
			DLLCLIENT void SetDynamicViewportsCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t numDynamicViewports);
			DLLCLIENT void SetPrimitiveTopology(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t primitiveTopology);
			DLLCLIENT void SetRasterizationProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t polygonMode, uint32_t cullMode, uint32_t frontFace, float lineWidth);
			DLLCLIENT void SetPolygonMode(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t polygonMode);
			DLLCLIENT void SetCullMode(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t cullMode);
			DLLCLIENT void SetFrontFace(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t frontFace);
			DLLCLIENT void SetLineWidth(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float lineWidth);
			DLLCLIENT void SetScissorBoxProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iScissorBox, int32_t x, int32_t y, uint32_t w, uint32_t h);
			DLLCLIENT void SetStencilTestProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool updateFrontFaceState, uint32_t stencilFailOp, uint32_t stencilPassOp, uint32_t stencilDepthFailOp, uint32_t stencilCompareOp, uint32_t stencilCompareMask,
			  uint32_t stencilWriteMask, uint32_t stencilReference);
			DLLCLIENT void SetViewportProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iViewport, float originX, float originY, float w, float h, float minDepth, float maxDepth);
			DLLCLIENT void AreDepthWritesEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetBlendingProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetColorBlendAttachmentProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t attId);
			DLLCLIENT void GetDepthBiasState(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetDepthBiasConstantFactor(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetDepthBiasClamp(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetDepthBiasSlopeFactor(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetDepthBoundsState(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetMinDepthBounds(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetMaxDepthBounds(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetDepthClamp(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetDepthTestState(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			//DLLCLIENT void GetDsInfoItems(lua::State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo); // TODO
			DLLCLIENT void GetDynamicStates(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void IsDynamicStateEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t dynamicState);
			DLLCLIENT void GetScissorCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetViewportCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetVertexAttributeCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetLogicOpState(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetMultisamplingProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetSampleCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetMinSampleShading(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetSampleMask(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetDynamicScissorBoxesCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetDynamicViewportsCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetScissorBoxesCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetViewportsCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetPrimitiveTopology(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetPushConstantRanges(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetRasterizationProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetPolygonMode(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetCullMode(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetFrontFace(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetLineWidth(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			//DLLCLIENT void GetRenderpass(lua::State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo); // TODO
			DLLCLIENT void GetSampleShadingState(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetScissorBoxProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iScissor);
			//DLLCLIENT void GetSpecializationConstants(lua::State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo); // TODO
			DLLCLIENT void GetStencilTestProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetSubpassId(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void GetVertexAttributeProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iVerexInputAttribute);
			DLLCLIENT void GetViewportProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iViewport);
			DLLCLIENT void IsAlphaToCoverageEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void IsAlphaToOneEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void IsDepthClampEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void IsPrimitiveRestartEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void IsRasterizerDiscardEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void IsSampleMaskEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
			DLLCLIENT void AddSpecializationConstant(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t shaderStage, uint32_t constantId, pragma::util::DataStream &ds);
			DLLCLIENT void SetAlphaToCoverageEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetAlphaToOneEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetDepthBiasProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
			DLLCLIENT void SetDepthBiasEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetDepthBiasConstantFactor(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float depthBiasConstantFactor);
			DLLCLIENT void SetDepthBiasClamp(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float depthBiasClamp);
			DLLCLIENT void SetDepthBiasSlopeFactor(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float depthBiasSlopeFactor);
			DLLCLIENT void SetDepthBoundsTestProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, float minDepthBounds, float maxDepthBounds);
			DLLCLIENT void SetDepthBoundsTestEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetMinDepthBounds(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float minDepthBounds);
			DLLCLIENT void SetMaxDepthBounds(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float maxDepthBounds);
			DLLCLIENT void SetDepthClampEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetDepthTestProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, uint32_t compareOp);
			DLLCLIENT void SetDepthTestEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetDepthWritesEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetDynamicStates(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t states);
			DLLCLIENT void SetDynamicStateEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t state, bool bEnabled);
			DLLCLIENT void SetLogicOpProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, uint32_t logicOp);
			DLLCLIENT void SetLogicOpEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetPrimitiveRestartEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetRasterizerDiscardEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetSampleMaskEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetSampleShadingEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
			DLLCLIENT void SetStencilTestEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		};

		namespace ComputePipelineCreateInfo {
			DLLCLIENT void AddSpecializationConstant(lua::State *l, prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t constantId, pragma::util::DataStream &ds);
		};

		namespace shader {
			DLLCLIENT void push_shader(lua::State *l, prosper::Shader &shader);
		};

		namespace Shader {
			DLLCLIENT void CreateDescriptorSetGroup(lua::State *l, prosper::Shader &shader, uint32_t setIdx);
			DLLCLIENT void GetPipelineInfo(lua::State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx);
			DLLCLIENT void GetEntrypointName(lua::State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx);
			DLLCLIENT void GetGlslSourceCode(lua::State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx);
			DLLCLIENT void IsGraphicsShader(lua::State *l, prosper::Shader &shader);
			DLLCLIENT void IsComputeShader(lua::State *l, prosper::Shader &shader);
			DLLCLIENT void GetPipelineBindPoint(lua::State *l, prosper::Shader &shader);
			DLLCLIENT void IsValid(lua::State *l, prosper::Shader &shader);
			DLLCLIENT void GetIdentifier(lua::State *l, prosper::Shader &shader);
			DLLCLIENT void GetSourceFilePath(lua::State *l, prosper::Shader &shader, uint32_t shaderStage);
			DLLCLIENT void GetSourceFilePaths(lua::State *l, prosper::Shader &shader);
			DLLCLIENT void RecordPushConstants(lua::State *l, prosper::Shader &shader, const LuaShaderRecordTarget &recordTarget, pragma::util::DataStream &ds, uint32_t offset);
			DLLCLIENT void RecordPushConstants(lua::State *l, prosper::Shader &shader, prosper::util::PreparedCommandBuffer &pcb, ::udm::Type type, const Vulkan::PreparedCommandLuaArg &value, uint32_t offset);
			DLLCLIENT void RecordBindDescriptorSet(lua::State *l, prosper::Shader &shader, prosper::util::PreparedCommandBuffer &pcb, Vulkan::DescriptorSet &ds, uint32_t firstSet, luabind::object dynamicOffsets, std::optional<uint32_t> dynamicOffsetIndex = {});
			DLLCLIENT void RecordBindDescriptorSet(lua::State *l, prosper::Shader &shader, prosper::ShaderBindState &bindState, Vulkan::DescriptorSet &ds, uint32_t firstSet, luabind::object dynamicOffsets, std::optional<uint32_t> dynamicOffsetIndex = {});
			DLLCLIENT void RecordBindDescriptorSets(lua::State *l, prosper::Shader &shader, prosper::ShaderBindState &bindState, luabind::object descSets, uint32_t firstSet, luabind::object dynamicOffsets);
			DLLCLIENT void AttachPushConstantRange(lua::State *l, pragma::LuaShaderWrapperBase &shader, uint32_t offset, uint32_t size, uint32_t shaderStages);
			DLLCLIENT uint32_t AttachDescriptorSetInfo(lua::State *l, pragma::LuaShaderWrapperBase &shader, pragma::LuaDescriptorSetInfo &descSetInfo);

			namespace Graphics {
				DLLCLIENT void RecordBindVertexBuffer(lua::State *l, prosper::ShaderGraphics &shader, prosper::ShaderBindState &bindState, Vulkan::Buffer &buffer, uint32_t startBinding, uint32_t offset);
				DLLCLIENT void RecordBindVertexBuffers(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, luabind::object buffers, uint32_t startBinding, luabind::object offsets);
				DLLCLIENT void RecordBindIndexBuffer(lua::State *l, prosper::ShaderGraphics &shader, prosper::ShaderBindState &bindState, Vulkan::Buffer &indexBuffer, uint32_t indexType, uint32_t offset);
				DLLCLIENT void RecordDraw(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, uint32_t vertCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
				DLLCLIENT void RecordDrawIndexed(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance);
				DLLCLIENT void RecordBeginDraw(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, uint32_t pipelineIdx);
				DLLCLIENT void RecordDraw(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget);
				DLLCLIENT void RecordEndDraw(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget);
				DLLCLIENT void GetRenderPass(lua::State *l, prosper::ShaderGraphics &shader, uint32_t pipelineIdx);
				DLLCLIENT void AttachVertexAttribute(lua::State *l, pragma::LuaShaderWrapperGraphicsBase &shader, const pragma::LuaVertexBinding &binding, luabind::object attributes);
			};

			namespace Scene3D {
				DLLCLIENT void GetRenderPass(lua::State *l, uint32_t pipelineIdx);
			};

			namespace Compute {
				DLLCLIENT void RecordDispatch(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState, uint32_t x, uint32_t y, uint32_t z);
				DLLCLIENT void RecordBeginCompute(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState, uint32_t pipelineIdx);
				DLLCLIENT void RecordCompute(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState);
				DLLCLIENT void RecordEndCompute(lua::State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState);
			};

			// Custom shaders
			DLLCLIENT void SetStageSourceFilePath(lua::State *l, pragma::LuaShaderWrapperBase &shader, uint32_t shaderStage, const std::string &fpath);
			DLLCLIENT void SetPipelineCount(lua::State *l, pragma::LuaShaderWrapperBase &shader, uint32_t pipelineCount);
		};
	};
};
