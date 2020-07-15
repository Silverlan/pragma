/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LSHADER_H__
#define __C_LSHADER_H__

#include "pragma/lua/libraries/c_lua_vulkan.h"

namespace pragma
{
	class ShaderLua;
	class ShaderScene;
	class ShaderSceneLit;
	class ShaderEntity;
	class LuaShaderBase;
	class LuaShaderGraphicsBase;
	class LuaShaderComputeBase;
	struct LuaVertexBinding;
	struct LuaVertexAttribute;
	struct LuaDescriptorSetInfo;
	namespace rendering {class RasterizationRenderer;};
};

namespace Lua
{
	namespace BasePipelineCreateInfo
	{
		DLLCLIENT void AttachPushConstantRange(lua_State *l,prosper::BasePipelineCreateInfo &pipelineInfo,uint32_t offset,uint32_t size,uint32_t shaderStages);
		DLLCLIENT void AttachDescriptorSetInfo(lua_State *l,prosper::BasePipelineCreateInfo &pipelineInfo,pragma::LuaDescriptorSetInfo &descSetInfo);
	};

	namespace GraphicsPipelineCreateInfo
	{
		DLLCLIENT void SetBlendingProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,const Vector4 &blendingProperties);
		DLLCLIENT void SetCommonAlphaBlendProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void SetColorBlendAttachmentProperties(
			lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t attId,bool blendingEnabled,uint32_t blendOpColor,
			uint32_t blendOpAlpha,uint32_t srcColorBlendFactor,uint32_t dstColorBlendFactor,uint32_t srcAlphaBlendFactor,uint32_t dstAlphaBlendFactor,
			uint32_t channelWriteMask
		);
		DLLCLIENT void SetMultisamplingProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t sampleCount,float sampleShading,uint32_t sampleMask);
		DLLCLIENT void SetSampleCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t sampleCount);
		DLLCLIENT void SetMinSampleShading(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float sampleShading);
		DLLCLIENT void SetSampleMask(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t sampleMask);
		DLLCLIENT void SetDynamicScissorBoxesCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t numDynamicScissorBoxes);
		DLLCLIENT void SetDynamicViewportsCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t numDynamicViewports);
		DLLCLIENT void SetPrimitiveTopology(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t primitiveTopology);
		DLLCLIENT void SetRasterizationProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t polygonMode,uint32_t cullMode,uint32_t frontFace,float lineWidth);
		DLLCLIENT void SetPolygonMode(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t polygonMode);
		DLLCLIENT void SetCullMode(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t cullMode);
		DLLCLIENT void SetFrontFace(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t frontFace);
		DLLCLIENT void SetLineWidth(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float lineWidth);
		DLLCLIENT void SetScissorBoxProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iScissorBox,int32_t x,int32_t y,uint32_t w,uint32_t h);
		DLLCLIENT void SetStencilTestProperties(
			lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool updateFrontFaceState,uint32_t stencilFailOp,uint32_t stencilPassOp,
			uint32_t stencilDepthFailOp,uint32_t stencilCompareOp,uint32_t stencilCompareMask,uint32_t stencilWriteMask,uint32_t stencilReference
		);
		DLLCLIENT void SetViewportProperties(
			lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iViewport,float originX,float originY,
			float w,float h,float minDepth,float maxDepth
		);
		DLLCLIENT void AreDepthWritesEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetBlendingProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetColorBlendAttachmentProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t attId);
		DLLCLIENT void GetDepthBiasState(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthBiasConstantFactor(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthBiasClamp(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthBiasSlopeFactor(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthBoundsState(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetMinDepthBounds(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetMaxDepthBounds(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthClamp(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDepthTestState(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		//DLLCLIENT void GetDsInfoItems(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo); // TODO
		DLLCLIENT void GetDynamicStates(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsDynamicStateEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t dynamicState);
		DLLCLIENT void GetScissorCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetViewportCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetVertexAttributeCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetLogicOpState(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetMultisamplingProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetSampleCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetMinSampleShading(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetSampleMask(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDynamicScissorBoxesCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetDynamicViewportsCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetScissorBoxesCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetViewportsCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetPrimitiveTopology(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetPushConstantRanges(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetRasterizationProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetPolygonMode(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetCullMode(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetFrontFace(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetLineWidth(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		//DLLCLIENT void GetRenderpass(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo); // TODO
		DLLCLIENT void GetSampleShadingState(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetScissorBoxProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iScissor);
		//DLLCLIENT void GetSpecializationConstants(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo); // TODO
		DLLCLIENT void GetStencilTestProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetSubpassId(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void GetVertexAttributeProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iVerexInputAttribute);
		DLLCLIENT void GetViewportProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iViewport);
		DLLCLIENT void IsAlphaToCoverageEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsAlphaToOneEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsDepthClampEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsPrimitiveRestartEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsRasterizerDiscardEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void IsSampleMaskEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo);
		DLLCLIENT void AttachVertexAttribute(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,const pragma::LuaVertexBinding &binding,luabind::object attributes);
		DLLCLIENT void AddSpecializationConstant(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t shaderStage,uint32_t constantId,::DataStream &ds);
		DLLCLIENT void SetAlphaToCoverageEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetAlphaToOneEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetDepthBiasProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,float depthBiasConstantFactor,float depthBiasClamp,float depthBiasSlopeFactor);
		DLLCLIENT void SetDepthBiasEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetDepthBiasConstantFactor(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float depthBiasConstantFactor);
		DLLCLIENT void SetDepthBiasClamp(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float depthBiasClamp);
		DLLCLIENT void SetDepthBiasSlopeFactor(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float depthBiasSlopeFactor);
		DLLCLIENT void SetDepthBoundsTestProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,float minDepthBounds,float maxDepthBounds);
		DLLCLIENT void SetDepthBoundsTestEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetMinDepthBounds(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float minDepthBounds);
		DLLCLIENT void SetMaxDepthBounds(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float maxDepthBounds);
		DLLCLIENT void SetDepthClampEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetDepthTestProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,uint32_t compareOp);
		DLLCLIENT void SetDepthTestEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetDepthWritesEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetDynamicStates(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t states);
		DLLCLIENT void SetDynamicStateEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t state,bool bEnabled);
		DLLCLIENT void SetLogicOpProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,uint32_t logicOp);
		DLLCLIENT void SetLogicOpEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetPrimitiveRestartEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetRasterizerDiscardEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetSampleMaskEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetSampleShadingEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
		DLLCLIENT void SetStencilTestEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled);
	};

	namespace ComputePipelineCreateInfo
	{
		DLLCLIENT void AddSpecializationConstant(lua_State *l,prosper::ComputePipelineCreateInfo &pipelineInfo,uint32_t constantId,::DataStream &ds);
	};

	namespace shader
	{
		DLLCLIENT void push_shader(lua_State *l,prosper::Shader &shader);
	};

	namespace Shader
	{
		DLLCLIENT void CreateDescriptorSetGroup(lua_State *l,prosper::Shader &shader,uint32_t setIdx,uint32_t pipelineIdx);
		DLLCLIENT void GetPipelineInfo(lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx);
		DLLCLIENT void GetEntrypointName(lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx);
		DLLCLIENT void GetGlslSourceCode(lua_State *l,prosper::Shader &shader,uint32_t shaderStage,uint32_t pipelineIdx);
		DLLCLIENT void IsGraphicsShader(lua_State *l,prosper::Shader &shader);
		DLLCLIENT void IsComputeShader(lua_State *l,prosper::Shader &shader);
		DLLCLIENT void GetPipelineBindPoint(lua_State *l,prosper::Shader &shader);
		DLLCLIENT void IsValid(lua_State *l,prosper::Shader &shader);
		DLLCLIENT void GetIdentifier(lua_State *l,prosper::Shader &shader);
		DLLCLIENT void GetSourceFilePath(lua_State *l,prosper::Shader &shader,uint32_t shaderStage);
		DLLCLIENT void GetSourceFilePaths(lua_State *l,prosper::Shader &shader);
		DLLCLIENT void RecordPushConstants(lua_State *l,prosper::Shader &shader,::DataStream &ds,uint32_t offset);
		DLLCLIENT void RecordBindDescriptorSet(lua_State *l,prosper::Shader &shader,Lua::Vulkan::DescriptorSet &ds,uint32_t firstSet,luabind::object dynamicOffsets);
		DLLCLIENT void RecordBindDescriptorSets(lua_State *l,prosper::Shader &shader,luabind::object descSets,uint32_t firstSet,luabind::object ynamicOffsets);

		namespace Graphics
		{
			DLLCLIENT void RecordBindVertexBuffer(lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::Buffer &buffer,uint32_t startBinding,uint32_t offset);
			DLLCLIENT void RecordBindVertexBuffers(lua_State *l,prosper::ShaderGraphics &shader,luabind::object buffers,uint32_t startBinding,luabind::object offsets);
			DLLCLIENT void RecordBindIndexBuffer(lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::Buffer &indexBuffer,uint32_t indexType,uint32_t offset);
			DLLCLIENT void RecordDraw(lua_State *l,prosper::ShaderGraphics &shader,uint32_t vertCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance);
			DLLCLIENT void RecordDrawIndexed(lua_State *l,prosper::ShaderGraphics &shader,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,uint32_t firstInstance);
			DLLCLIENT void RecordBeginDraw(lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t pipelineIdx);
			DLLCLIENT void RecordDraw(lua_State *l,prosper::ShaderGraphics &shader);
			DLLCLIENT void RecordEndDraw(lua_State *l,prosper::ShaderGraphics &shader);
			DLLCLIENT void GetRenderPass(lua_State *l,prosper::ShaderGraphics &shader,uint32_t pipelineIdx);
		};

		namespace Scene3D
		{
			DLLCLIENT void GetRenderPass(lua_State *l,uint32_t pipelineIdx);
			DLLCLIENT void BindSceneCamera(lua_State *l,pragma::ShaderScene &shader,pragma::rendering::RasterizationRenderer &renderer,bool bView);
			DLLCLIENT void BindRenderSettings(lua_State *l,pragma::ShaderScene &shader,std::shared_ptr<prosper::IDescriptorSetGroup> &descSet);
		};

		namespace SceneLit3D
		{
			DLLCLIENT void BindLights(lua_State *l,pragma::ShaderSceneLit &shader,std::shared_ptr<prosper::IDescriptorSetGroup> &dsLights);
			DLLCLIENT void BindScene(lua_State *l,pragma::ShaderSceneLit &shader,pragma::rendering::RasterizationRenderer &renderer,bool bView);
		};

		namespace ShaderEntity
		{
			DLLCLIENT void BindInstanceDescriptorSet(lua_State *l,pragma::ShaderEntity &shader,std::shared_ptr<prosper::IDescriptorSetGroup> &descSet);
			DLLCLIENT void BindEntity(lua_State *l,pragma::ShaderEntity &shader,EntityHandle &hEnt);
			DLLCLIENT void BindVertexAnimationOffset(lua_State *l,pragma::ShaderEntity &shader,uint32_t offset);
			DLLCLIENT void Draw(lua_State *l,pragma::ShaderEntity &shader,::ModelSubMesh &mesh);
		};

		namespace TexturedLit3D
		{
			DLLCLIENT void BindMaterial(lua_State *l,pragma::ShaderTextured3DBase &shader,::Material &mat);
			DLLCLIENT void RecordBindClipPlane(lua_State *l,pragma::ShaderTextured3DBase &shader,const Vector4 &clipPlane);
		};

		namespace Compute
		{
			DLLCLIENT void RecordDispatch(lua_State *l,prosper::ShaderCompute &shader,uint32_t x,uint32_t y,uint32_t z);
			DLLCLIENT void RecordBeginCompute(lua_State *l,prosper::ShaderCompute &shader,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t pipelineIdx);
			DLLCLIENT void RecordCompute(lua_State *l,prosper::ShaderCompute &shader);
			DLLCLIENT void RecordEndCompute(lua_State *l,prosper::ShaderCompute &shader);
		};

		// Custom shaders
		DLLCLIENT void SetStageSourceFilePath(lua_State *l,pragma::LuaShaderBase &shader,uint32_t shaderStage,const std::string &fpath);
		DLLCLIENT void SetPipelineCount(lua_State *l,pragma::LuaShaderBase &shader,uint32_t pipelineCount);
		DLLCLIENT void GetCurrentCommandBuffer(lua_State *l,pragma::LuaShaderBase &shader);
	};
};

#endif
