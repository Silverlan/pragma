/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LSHADER_H__
#define __C_LSHADER_H__

#include "pragma/lua/libraries/c_lua_vulkan.h"

namespace pragma {
	class ShaderLua;
	class ShaderScene;
	class ShaderSceneLit;
	class ShaderEntity;
	class LuaShaderWrapperBase;
	class LuaShaderWrapperGraphicsBase;
	class LuaShaderWrapperComputeBase;
	struct LuaVertexBinding;
	struct LuaVertexAttribute;
	struct LuaDescriptorSetInfo;
	class CRasterizationRendererComponent;
};
namespace prosper {
	class BasePipelineCreateInfo;
	class ShaderGraphics;
	class ShaderCompute;
	struct ShaderBindState;
	namespace util {
		class PreparedCommandBuffer;
	};
};

// LuaShaderRecordTarget converter
struct LuaShaderRecordTarget {
	luabind::object target;
	prosper::ShaderBindState *GetBindState() const;
	prosper::util::PreparedCommandBuffer *GetPcb() const;
};
namespace luabind {
	template<>
	struct DLLNETWORK default_converter<LuaShaderRecordTarget> : native_converter_base<LuaShaderRecordTarget> {
		enum { consumed_args = 1 };

		template<typename U>
		LuaShaderRecordTarget to_cpp(lua_State *L, U u, int index);

		template<class U>
		static int match(lua_State *l, U u, int index);

		template<class U>
		void converter_postcall(lua_State *, U u, int)
		{
		}

		void to_lua(lua_State *L, LuaShaderRecordTarget const &x);
		void to_lua(lua_State *L, LuaShaderRecordTarget *x);
	  public:
		static value_type to_cpp_deferred(lua_State *, int) { return {}; }
		static void to_lua_deferred(lua_State *, param_type) {}
		static int compute_score(lua_State *, int) { return no_match; }
	};

	template<>
	struct DLLNETWORK default_converter<const LuaShaderRecordTarget> : default_converter<LuaShaderRecordTarget> {};

	template<>
	struct DLLNETWORK default_converter<LuaShaderRecordTarget const &> : default_converter<LuaShaderRecordTarget> {};

	template<>
	struct DLLNETWORK default_converter<LuaShaderRecordTarget &&> : default_converter<LuaShaderRecordTarget> {};
}

template<typename U>
LuaShaderRecordTarget luabind::default_converter<LuaShaderRecordTarget>::to_cpp(lua_State *L, U u, int index)
{
	return LuaShaderRecordTarget {luabind::object {luabind::from_stack(L, index)}};
}

template<class U>
int luabind::default_converter<LuaShaderRecordTarget>::match(lua_State *l, U u, int index)
{
	return (Lua::IsType<prosper::ShaderBindState>(l, index) || Lua::IsType<prosper::util::PreparedCommandBuffer>(l, index)) ? 1 : no_match;
}
//

namespace Lua {
	namespace GraphicsPipelineCreateInfo {
		DLLCLIENT void SetBlendingProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, const Vector4 &blendingProperties);
		DLLCLIENT void SetCommonAlphaBlendProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void SetColorBlendAttachmentProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t attId, bool blendingEnabled, uint32_t blendOpColor, uint32_t blendOpAlpha, uint32_t srcColorBlendFactor, uint32_t dstColorBlendFactor,
		  uint32_t srcAlphaBlendFactor, uint32_t dstAlphaBlendFactor, uint32_t channelWriteMask);
		DLLCLIENT void SetMultisamplingProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t sampleCount, float sampleShading, uint32_t sampleMask);
		DLLCLIENT void SetSampleCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t sampleCount);
		DLLCLIENT void SetMinSampleShading(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float sampleShading);
		DLLCLIENT void SetSampleMask(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t sampleMask);
		DLLCLIENT void SetDynamicScissorBoxesCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t numDynamicScissorBoxes);
		DLLCLIENT void SetDynamicViewportsCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t numDynamicViewports);
		DLLCLIENT void SetPrimitiveTopology(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t primitiveTopology);
		DLLCLIENT void SetRasterizationProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t polygonMode, uint32_t cullMode, uint32_t frontFace, float lineWidth);
		DLLCLIENT void SetPolygonMode(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t polygonMode);
		DLLCLIENT void SetCullMode(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t cullMode);
		DLLCLIENT void SetFrontFace(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t frontFace);
		DLLCLIENT void SetLineWidth(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float lineWidth);
		DLLCLIENT void SetScissorBoxProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iScissorBox, int32_t x, int32_t y, uint32_t w, uint32_t h);
		DLLCLIENT void SetStencilTestProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool updateFrontFaceState, uint32_t stencilFailOp, uint32_t stencilPassOp, uint32_t stencilDepthFailOp, uint32_t stencilCompareOp, uint32_t stencilCompareMask,
		  uint32_t stencilWriteMask, uint32_t stencilReference);
		DLLCLIENT void SetViewportProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iViewport, float originX, float originY, float w, float h, float minDepth, float maxDepth);
		DLLCLIENT void AreDepthWritesEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetBlendingProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetColorBlendAttachmentProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t attId);
		DLLCLIENT void GetDepthBiasState(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthBiasConstantFactor(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthBiasClamp(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthBiasSlopeFactor(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthBoundsState(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetMinDepthBounds(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetMaxDepthBounds(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthClamp(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthTestState(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		//DLLCLIENT void GetDsInfoItems(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo); // TODO
		DLLCLIENT void GetDynamicStates(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsDynamicStateEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t dynamicState);
		DLLCLIENT void GetScissorCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetViewportCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetVertexAttributeCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetLogicOpState(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetMultisamplingProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetSampleCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetMinSampleShading(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetSampleMask(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDynamicScissorBoxesCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDynamicViewportsCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetScissorBoxesCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetViewportsCount(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetPrimitiveTopology(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetPushConstantRanges(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetRasterizationProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetPolygonMode(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetCullMode(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetFrontFace(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetLineWidth(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		//DLLCLIENT void GetRenderpass(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo); // TODO
		DLLCLIENT void GetSampleShadingState(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetScissorBoxProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iScissor);
		//DLLCLIENT void GetSpecializationConstants(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo); // TODO
		DLLCLIENT void GetStencilTestProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetSubpassId(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetVertexAttributeProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iVerexInputAttribute);
		DLLCLIENT void GetViewportProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iViewport);
		DLLCLIENT void IsAlphaToCoverageEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsAlphaToOneEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsDepthClampEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsPrimitiveRestartEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsRasterizerDiscardEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsSampleMaskEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void AddSpecializationConstant(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t shaderStage, uint32_t constantId, ::DataStream &ds);
		DLLCLIENT void SetAlphaToCoverageEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetAlphaToOneEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetDepthBiasProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
		DLLCLIENT void SetDepthBiasEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetDepthBiasConstantFactor(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float depthBiasConstantFactor);
		DLLCLIENT void SetDepthBiasClamp(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float depthBiasClamp);
		DLLCLIENT void SetDepthBiasSlopeFactor(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float depthBiasSlopeFactor);
		DLLCLIENT void SetDepthBoundsTestProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, float minDepthBounds, float maxDepthBounds);
		DLLCLIENT void SetDepthBoundsTestEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetMinDepthBounds(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float minDepthBounds);
		DLLCLIENT void SetMaxDepthBounds(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float maxDepthBounds);
		DLLCLIENT void SetDepthClampEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetDepthTestProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, uint32_t compareOp);
		DLLCLIENT void SetDepthTestEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetDepthWritesEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetDynamicStates(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t states);
		DLLCLIENT void SetDynamicStateEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t state, bool bEnabled);
		DLLCLIENT void SetLogicOpProperties(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, uint32_t logicOp);
		DLLCLIENT void SetLogicOpEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetPrimitiveRestartEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetRasterizerDiscardEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetSampleMaskEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetSampleShadingEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
		DLLCLIENT void SetStencilTestEnabled(lua_State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled);
	};

	namespace ComputePipelineCreateInfo {
		DLLCLIENT void AddSpecializationConstant(lua_State *l, prosper::ComputePipelineCreateInfo &pipelineInfo, uint32_t constantId, ::DataStream &ds);
	};

	namespace shader {
		DLLCLIENT void push_shader(lua_State *l, prosper::Shader &shader);
	};

	namespace Shader {
		DLLCLIENT void CreateDescriptorSetGroup(lua_State *l, prosper::Shader &shader, uint32_t setIdx);
		DLLCLIENT void GetPipelineInfo(lua_State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx);
		DLLCLIENT void GetEntrypointName(lua_State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx);
		DLLCLIENT void GetGlslSourceCode(lua_State *l, prosper::Shader &shader, uint32_t shaderStage, uint32_t pipelineIdx);
		DLLCLIENT void IsGraphicsShader(lua_State *l, prosper::Shader &shader);
		DLLCLIENT void IsComputeShader(lua_State *l, prosper::Shader &shader);
		DLLCLIENT void GetPipelineBindPoint(lua_State *l, prosper::Shader &shader);
		DLLCLIENT void IsValid(lua_State *l, prosper::Shader &shader);
		DLLCLIENT void GetIdentifier(lua_State *l, prosper::Shader &shader);
		DLLCLIENT void GetSourceFilePath(lua_State *l, prosper::Shader &shader, uint32_t shaderStage);
		DLLCLIENT void GetSourceFilePaths(lua_State *l, prosper::Shader &shader);
		DLLCLIENT void RecordPushConstants(lua_State *l, prosper::Shader &shader, const LuaShaderRecordTarget &recordTarget, ::DataStream &ds, uint32_t offset);
		DLLCLIENT void RecordPushConstants(lua_State *l, prosper::Shader &shader, prosper::util::PreparedCommandBuffer &pcb, udm::Type type, const Lua::Vulkan::PreparedCommandLuaArg &value, uint32_t offset);
		DLLCLIENT void RecordBindDescriptorSet(lua_State *l, prosper::Shader &shader, prosper::util::PreparedCommandBuffer &pcb, Lua::Vulkan::DescriptorSet &ds, uint32_t firstSet, luabind::object dynamicOffsets, std::optional<uint32_t> dynamicOffsetIndex = {});
		DLLCLIENT void RecordBindDescriptorSet(lua_State *l, prosper::Shader &shader, prosper::ShaderBindState &bindState, Lua::Vulkan::DescriptorSet &ds, uint32_t firstSet, luabind::object dynamicOffsets, std::optional<uint32_t> dynamicOffsetIndex = {});
		DLLCLIENT void RecordBindDescriptorSets(lua_State *l, prosper::Shader &shader, prosper::ShaderBindState &bindState, luabind::object descSets, uint32_t firstSet, luabind::object dynamicOffsets);
		DLLCLIENT void AttachPushConstantRange(lua_State *l, pragma::LuaShaderWrapperBase &shader, uint32_t offset, uint32_t size, uint32_t shaderStages);
		DLLCLIENT uint32_t AttachDescriptorSetInfo(lua_State *l, pragma::LuaShaderWrapperBase &shader, pragma::LuaDescriptorSetInfo &descSetInfo);

		namespace Graphics {
			DLLCLIENT void RecordBindVertexBuffer(lua_State *l, prosper::ShaderGraphics &shader, prosper::ShaderBindState &bindState, Lua::Vulkan::Buffer &buffer, uint32_t startBinding, uint32_t offset);
			DLLCLIENT void RecordBindVertexBuffers(lua_State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, luabind::object buffers, uint32_t startBinding, luabind::object offsets);
			DLLCLIENT void RecordBindIndexBuffer(lua_State *l, prosper::ShaderGraphics &shader, prosper::ShaderBindState &bindState, Lua::Vulkan::Buffer &indexBuffer, uint32_t indexType, uint32_t offset);
			DLLCLIENT void RecordDraw(lua_State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, uint32_t vertCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
			DLLCLIENT void RecordDrawIndexed(lua_State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance);
			DLLCLIENT void RecordBeginDraw(lua_State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, uint32_t pipelineIdx);
			DLLCLIENT void RecordDraw(lua_State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget);
			DLLCLIENT void RecordEndDraw(lua_State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget);
			DLLCLIENT void GetRenderPass(lua_State *l, prosper::ShaderGraphics &shader, uint32_t pipelineIdx);
			DLLCLIENT void AttachVertexAttribute(lua_State *l, pragma::LuaShaderWrapperGraphicsBase &shader, const pragma::LuaVertexBinding &binding, luabind::object attributes);
		};

		namespace Scene3D {
			DLLCLIENT void GetRenderPass(lua_State *l, uint32_t pipelineIdx);
		};

		namespace Compute {
			DLLCLIENT void RecordDispatch(lua_State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState, uint32_t x, uint32_t y, uint32_t z);
			DLLCLIENT void RecordBeginCompute(lua_State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState, uint32_t pipelineIdx);
			DLLCLIENT void RecordCompute(lua_State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState);
			DLLCLIENT void RecordEndCompute(lua_State *l, prosper::ShaderCompute &shader, prosper::ShaderBindState &bindState);
		};

		// Custom shaders
		DLLCLIENT void SetStageSourceFilePath(lua_State *l, pragma::LuaShaderWrapperBase &shader, uint32_t shaderStage, const std::string &fpath);
		DLLCLIENT void SetPipelineCount(lua_State *l, pragma::LuaShaderWrapperBase &shader, uint32_t pipelineCount);
	};
};

#endif
