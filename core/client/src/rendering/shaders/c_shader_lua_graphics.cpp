#include "stdafx_client.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/classes/ldef_entity.h>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;

void Lua::GraphicsPipelineCreateInfo::SetBlendingProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,const Vector4 &blendingProperties)
{
	pipelineInfo.set_blending_properties(reinterpret_cast<const float*>(&blendingProperties));
}
void Lua::GraphicsPipelineCreateInfo::SetCommonAlphaBlendProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
}
void Lua::GraphicsPipelineCreateInfo::SetColorBlendAttachmentProperties(
	lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t attId,bool blendingEnabled,
	uint32_t blendOpColor,uint32_t blendOpAlpha,uint32_t srcColorBlendFactor,uint32_t dstColorBlendFactor,uint32_t srcAlphaBlendFactor,
	uint32_t dstAlphaBlendFactor,uint32_t channelWriteMask
)
{
	pipelineInfo.set_color_blend_attachment_properties(
		attId,blendingEnabled,static_cast<Anvil::BlendOp>(blendOpColor),static_cast<Anvil::BlendOp>(blendOpAlpha),static_cast<Anvil::BlendFactor>(srcColorBlendFactor),
		static_cast<Anvil::BlendFactor>(dstColorBlendFactor),static_cast<Anvil::BlendFactor>(srcAlphaBlendFactor),static_cast<Anvil::BlendFactor>(dstAlphaBlendFactor),
		static_cast<Anvil::ColorComponentFlagBits>(channelWriteMask)
	);
}
void Lua::GraphicsPipelineCreateInfo::SetMultisamplingProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t sampleCount,float sampleShading,uint32_t sampleMask)
{
	pipelineInfo.set_multisampling_properties(static_cast<Anvil::SampleCountFlagBits>(sampleCount),sampleShading,static_cast<vk::SampleMask>(sampleMask));
}
void Lua::GraphicsPipelineCreateInfo::SetSampleCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t sampleCount)
{
	const vk::SampleMask *sampleMask;
	float minSampleShading;
	pipelineInfo.get_multisampling_properties(nullptr,&sampleMask);
	pipelineInfo.get_sample_shading_state(nullptr,&minSampleShading);
	pipelineInfo.set_multisampling_properties(static_cast<Anvil::SampleCountFlagBits>(sampleCount),minSampleShading,sampleMask ? *sampleMask : 0u);
}
void Lua::GraphicsPipelineCreateInfo::SetMinSampleShading(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,float minSampleShading)
{
	Anvil::SampleCountFlagBits sampleCount;
	const vk::SampleMask *sampleMask;
	pipelineInfo.get_multisampling_properties(&sampleCount,&sampleMask);
	pipelineInfo.set_multisampling_properties(static_cast<Anvil::SampleCountFlagBits>(sampleCount),minSampleShading,sampleMask ? *sampleMask : 0u);
}
void Lua::GraphicsPipelineCreateInfo::SetSampleMask(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t sampleMask)
{
	Anvil::SampleCountFlagBits sampleCount;
	float minSampleShading;
	pipelineInfo.get_multisampling_properties(&sampleCount,nullptr);
	pipelineInfo.get_sample_shading_state(nullptr,&minSampleShading);
	pipelineInfo.set_multisampling_properties(static_cast<Anvil::SampleCountFlagBits>(sampleCount),minSampleShading,sampleMask);
}
void Lua::GraphicsPipelineCreateInfo::SetDynamicScissorBoxesCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t numDynamicScissorBoxes)
{
	pipelineInfo.set_n_dynamic_scissor_boxes(numDynamicScissorBoxes);
}
void Lua::GraphicsPipelineCreateInfo::SetDynamicViewportsCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t numDynamicViewports)
{
	pipelineInfo.set_n_dynamic_viewports(numDynamicViewports);
}
void Lua::GraphicsPipelineCreateInfo::SetPatchControlPointsCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t numPathControlPoints)
{
	pipelineInfo.set_n_patch_control_points(numPathControlPoints);
}
void Lua::GraphicsPipelineCreateInfo::SetPrimitiveTopology(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t primitiveTopology)
{
	pipelineInfo.set_primitive_topology(static_cast<Anvil::PrimitiveTopology>(primitiveTopology));
}
void Lua::GraphicsPipelineCreateInfo::SetRasterizationOrder(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t rasterizationOrder)
{
	pipelineInfo.set_rasterization_order(static_cast<Anvil::RasterizationOrderAMD>(rasterizationOrder));
}
void Lua::GraphicsPipelineCreateInfo::SetRasterizationProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t polygonMode,uint32_t cullMode,uint32_t frontFace,float lineWidth)
{
	pipelineInfo.set_rasterization_properties(static_cast<Anvil::PolygonMode>(polygonMode),static_cast<Anvil::CullModeFlagBits>(cullMode),static_cast<Anvil::FrontFace>(frontFace),lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetPolygonMode(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t polygonMode)
{
	Anvil::CullModeFlags cullMode;
	Anvil::FrontFace frontFace;
	float lineWidth;
	pipelineInfo.get_rasterization_properties(nullptr,&cullMode,&frontFace,&lineWidth);
	pipelineInfo.set_rasterization_properties(static_cast<Anvil::PolygonMode>(polygonMode),cullMode,frontFace,lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetCullMode(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t cullMode)
{
	Anvil::PolygonMode polygonMode;
	Anvil::FrontFace frontFace;
	float lineWidth;
	pipelineInfo.get_rasterization_properties(&polygonMode,nullptr,&frontFace,&lineWidth);
	pipelineInfo.set_rasterization_properties(polygonMode,static_cast<Anvil::CullModeFlagBits>(cullMode),frontFace,lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetFrontFace(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t frontFace)
{
	Anvil::PolygonMode polygonMode;
	Anvil::CullModeFlags cullMode;
	float lineWidth;
	pipelineInfo.get_rasterization_properties(&polygonMode,&cullMode,nullptr,&lineWidth);
	pipelineInfo.set_rasterization_properties(polygonMode,cullMode,static_cast<Anvil::FrontFace>(frontFace),lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetLineWidth(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,float lineWidth)
{
	Anvil::PolygonMode polygonMode;
	Anvil::CullModeFlags cullMode;
	Anvil::FrontFace frontFace;
	pipelineInfo.get_rasterization_properties(&polygonMode,&cullMode,&frontFace,nullptr);
	pipelineInfo.set_rasterization_properties(polygonMode,cullMode,frontFace,lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetScissorBoxProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iScissorBox,int32_t x,int32_t y,uint32_t w,uint32_t h)
{
	pipelineInfo.set_scissor_box_properties(iScissorBox,x,y,w,h);
}
void Lua::GraphicsPipelineCreateInfo::SetStencilTestProperties(
	lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool updateFrontFaceState,uint32_t stencilFailOp,uint32_t stencilPassOp,
	uint32_t stencilDepthFailOp,uint32_t stencilCompareOp,uint32_t stencilCompareMask,uint32_t stencilWriteMask,uint32_t stencilReference
)
{
	pipelineInfo.set_stencil_test_properties(
		updateFrontFaceState,static_cast<Anvil::StencilOp>(stencilFailOp),static_cast<Anvil::StencilOp>(stencilPassOp),static_cast<Anvil::StencilOp>(stencilDepthFailOp),
		static_cast<Anvil::CompareOp>(stencilCompareOp),stencilCompareMask,stencilWriteMask,stencilReference
	);
}
void Lua::GraphicsPipelineCreateInfo::SetViewportProperties(
	lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iViewport,float originX,float originY,
	float w,float h,float minDepth,float maxDepth
)
{
	pipelineInfo.set_viewport_properties(iViewport,originX,originY,w,h,minDepth,maxDepth);
}
void Lua::GraphicsPipelineCreateInfo::AreDepthWritesEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushBool(l,pipelineInfo.are_depth_writes_enabled());
}
void Lua::GraphicsPipelineCreateInfo::GetBlendingProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	const float *blendingProperties;
	pipelineInfo.get_blending_properties(&blendingProperties,nullptr);
	Lua::Push<Vector4>(l,Vector4{blendingProperties[0],blendingProperties[1],blendingProperties[2],blendingProperties[3]});
}
void Lua::GraphicsPipelineCreateInfo::GetColorBlendAttachmentProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t attId)
{
	bool blendingEnabled;
	Anvil::BlendOp blendOpColor,blendOpAlpha;
	Anvil::BlendFactor srcColorBlendFactor,dstColorBlendFactor,srcAlphaBlendFactor,dstAlphaBlendFactor;
	Anvil::ColorComponentFlags channelWriteMask;
	auto r = pipelineInfo.get_color_blend_attachment_properties(
		attId,&blendingEnabled,&blendOpColor,&blendOpAlpha,&srcColorBlendFactor,&dstColorBlendFactor,
		&srcAlphaBlendFactor,&dstAlphaBlendFactor,&channelWriteMask
	);
	if(r == false)
		return;
	Lua::PushBool(l,blendingEnabled);
	Lua::PushInt(l,static_cast<uint32_t>(blendOpColor));
	Lua::PushInt(l,static_cast<uint32_t>(blendOpAlpha));
	Lua::PushInt(l,static_cast<uint32_t>(srcColorBlendFactor));
	Lua::PushInt(l,static_cast<uint32_t>(dstColorBlendFactor));
	Lua::PushInt(l,static_cast<uint32_t>(srcAlphaBlendFactor));
	Lua::PushInt(l,static_cast<uint32_t>(dstAlphaBlendFactor));
	Lua::PushInt(l,static_cast<uint32_t>(channelWriteMask.get_vk()));
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasState(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	float depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor;
	pipelineInfo.get_depth_bias_state(&bEnabled,&depthBiasConstantFactor,&depthBiasClamp,&depthBiasSlopeFactor);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushNumber(l,depthBiasConstantFactor);
	Lua::PushNumber(l,depthBiasClamp);
	Lua::PushNumber(l,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasConstantFactor(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float depthBiasConstantFactor;
	pipelineInfo.get_depth_bias_state(nullptr,&depthBiasConstantFactor,nullptr,nullptr);
	Lua::PushNumber(l,depthBiasConstantFactor);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasClamp(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float depthBiasClamp;
	pipelineInfo.get_depth_bias_state(nullptr,nullptr,&depthBiasClamp,nullptr);
	Lua::PushNumber(l,depthBiasClamp);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasSlopeFactor(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float depthBiasSlopeFactor;
	pipelineInfo.get_depth_bias_state(nullptr,nullptr,nullptr,&depthBiasSlopeFactor);
	Lua::PushNumber(l,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBoundsState(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	float minDepthBounds,maxDepthBounds;
	pipelineInfo.get_depth_bounds_state(&bEnabled,&minDepthBounds,&maxDepthBounds);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushNumber(l,minDepthBounds);
	Lua::PushNumber(l,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetMinDepthBounds(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float minDepthBounds;
	pipelineInfo.get_depth_bounds_state(nullptr,&minDepthBounds,nullptr);
	Lua::PushNumber(l,minDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetMaxDepthBounds(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float maxDepthBounds;
	pipelineInfo.get_depth_bounds_state(nullptr,nullptr,&maxDepthBounds);
	Lua::PushNumber(l,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthClamp(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float minDepthBounds,maxDepthBounds;
	pipelineInfo.get_depth_bounds_state(nullptr,&minDepthBounds,&maxDepthBounds);
	Lua::PushNumber(l,minDepthBounds);
	Lua::PushNumber(l,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthTestState(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	Anvil::CompareOp compareOp;
	pipelineInfo.get_depth_test_state(&bEnabled,&compareOp);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushInt(l,static_cast<uint32_t>(compareOp));
}
void Lua::GraphicsPipelineCreateInfo::GetDynamicStates(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,prosper::util::get_enabled_dynamic_states(pipelineInfo));
}
void Lua::GraphicsPipelineCreateInfo::IsDynamicStateEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t dynamicState)
{
	Lua::PushBool(l,prosper::util::are_dynamic_states_enabled(pipelineInfo,static_cast<prosper::util::DynamicStateFlags>(dynamicState)));
}
void Lua::GraphicsPipelineCreateInfo::GetScissorCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	uint32_t scissorCount;
	pipelineInfo.get_graphics_pipeline_properties(&scissorCount,nullptr,nullptr,nullptr,nullptr);
	Lua::PushInt(l,scissorCount);
}
void Lua::GraphicsPipelineCreateInfo::GetViewportCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	uint32_t viewportCount;
	pipelineInfo.get_graphics_pipeline_properties(nullptr,&viewportCount,nullptr,nullptr,nullptr);
	Lua::PushInt(l,viewportCount);
}
void Lua::GraphicsPipelineCreateInfo::GetVertexAttributeCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	uint32_t vertexAttributeCount;
	pipelineInfo.get_graphics_pipeline_properties(nullptr,nullptr,&vertexAttributeCount,nullptr,nullptr);
	Lua::PushInt(l,vertexAttributeCount);
}
void Lua::GraphicsPipelineCreateInfo::GetLogicOpState(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	Anvil::LogicOp logicOp;
	pipelineInfo.get_logic_op_state(&bEnabled,&logicOp);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushInt(l,static_cast<uint32_t>(logicOp));
}
void Lua::GraphicsPipelineCreateInfo::GetMultisamplingProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Anvil::SampleCountFlagBits samples;
	const vk::SampleMask *sampleMask;
	pipelineInfo.get_multisampling_properties(&samples,&sampleMask);
	Lua::PushInt(l,umath::to_integral(samples));
	Lua::PushInt(l,sampleMask ? static_cast<uint32_t>(*sampleMask) : 0u);
}
void Lua::GraphicsPipelineCreateInfo::GetSampleCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Anvil::SampleCountFlagBits samples;
	pipelineInfo.get_multisampling_properties(&samples,nullptr);
	Lua::PushInt(l,umath::to_integral(samples));
}
void Lua::GraphicsPipelineCreateInfo::GetMinSampleShading(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	const vk::SampleMask *sampleMask;
	pipelineInfo.get_multisampling_properties(nullptr,&sampleMask);
	Lua::PushInt(l,sampleMask ? static_cast<uint32_t>(*sampleMask) : 0u);
}
void Lua::GraphicsPipelineCreateInfo::GetSampleMask(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float minSampleShading;
	pipelineInfo.get_sample_shading_state(nullptr,&minSampleShading);
	Lua::PushNumber(l,minSampleShading);
}
void Lua::GraphicsPipelineCreateInfo::GetDynamicScissorBoxesCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.get_n_dynamic_scissor_boxes());
}
void Lua::GraphicsPipelineCreateInfo::GetDynamicViewportsCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.get_n_dynamic_viewports());
}
void Lua::GraphicsPipelineCreateInfo::GetPatchControlPointsCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.get_n_patch_control_points());
}
void Lua::GraphicsPipelineCreateInfo::GetScissorBoxesCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.get_n_scissor_boxes());
}
void Lua::GraphicsPipelineCreateInfo::GetViewportsCount(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.get_n_viewports());
}
void Lua::GraphicsPipelineCreateInfo::GetPrimitiveTopology(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,static_cast<uint32_t>(pipelineInfo.get_primitive_topology()));
}
void Lua::GraphicsPipelineCreateInfo::GetPushConstantRanges(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	auto &ranges = pipelineInfo.get_push_constant_ranges();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto &range : ranges)
	{
		Lua::PushInt(l,idx++);
		auto tRange = Lua::CreateTable(l);

		Lua::PushString(l,"offset");
		Lua::PushInt(l,range.offset);
		Lua::SetTableValue(l,tRange);

		Lua::PushString(l,"size");
		Lua::PushInt(l,range.size);
		Lua::SetTableValue(l,tRange);

		Lua::PushString(l,"stages");
		Lua::PushInt(l,static_cast<uint32_t>(range.stages.get_vk()));
		Lua::SetTableValue(l,tRange);

		Lua::SetTableValue(l,t);
	}
}
void Lua::GraphicsPipelineCreateInfo::GetRasterizationOrder(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,static_cast<uint32_t>(pipelineInfo.get_rasterization_order()));
}
void Lua::GraphicsPipelineCreateInfo::GetRasterizationProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Anvil::PolygonMode polygonMode;
	Anvil::CullModeFlags cullMode;
	Anvil::FrontFace frontFace;
	float lineWidth;
	pipelineInfo.get_rasterization_properties(&polygonMode,&cullMode,&frontFace,&lineWidth);
	Lua::PushInt(l,static_cast<uint32_t>(polygonMode));
	Lua::PushInt(l,static_cast<uint32_t>(cullMode.get_vk()));
	Lua::PushInt(l,static_cast<uint32_t>(frontFace));
	Lua::PushNumber(l,lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::GetPolygonMode(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Anvil::PolygonMode polygonMode;
	pipelineInfo.get_rasterization_properties(&polygonMode,nullptr,nullptr,nullptr);
	Lua::PushInt(l,static_cast<uint32_t>(polygonMode));
}
void Lua::GraphicsPipelineCreateInfo::GetCullMode(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Anvil::CullModeFlags cullMode;
	pipelineInfo.get_rasterization_properties(nullptr,&cullMode,nullptr,nullptr);
	Lua::PushInt(l,static_cast<uint32_t>(cullMode.get_vk()));
}
void Lua::GraphicsPipelineCreateInfo::GetFrontFace(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Anvil::FrontFace frontFace;
	pipelineInfo.get_rasterization_properties(nullptr,nullptr,&frontFace,nullptr);
	Lua::PushInt(l,static_cast<uint32_t>(frontFace));
}
void Lua::GraphicsPipelineCreateInfo::GetLineWidth(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float lineWidth;
	pipelineInfo.get_rasterization_properties(nullptr,nullptr,nullptr,&lineWidth);
	Lua::PushNumber(l,lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::GetSampleShadingState(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	float minSampleShading;
	pipelineInfo.get_sample_shading_state(&bEnabled,&minSampleShading);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushNumber(l,minSampleShading);
}
void Lua::GraphicsPipelineCreateInfo::GetScissorBoxProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iScissor)
{
	int32_t x,y;
	uint32_t w,h;
	auto r = pipelineInfo.get_scissor_box_properties(iScissor,&x,&y,&w,&h);
	if(r == false)
		return;
	Lua::PushInt(l,x);
	Lua::PushInt(l,y);
	Lua::PushInt(l,w);
	Lua::PushInt(l,h);
}
void Lua::GraphicsPipelineCreateInfo::GetStencilTestProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	Anvil::StencilOp frontStencilFairOp,frontStencilPassOp,frontStencilDepthFailOp;
	Anvil::CompareOp frontStencilCompareOp,backStencilCompareOp;
	uint32_t frontStencilCompareMask,frontStencilWriteMask,frontStencilReference,backStencilCompareMask,backStencilWriteMask,backStencilReference;
	Anvil::StencilOp backStencilFailOp,backStencilPassOp,backStencilDepthFailOp;

	pipelineInfo.get_stencil_test_properties(
		&bEnabled,&frontStencilFairOp,&frontStencilPassOp,&frontStencilDepthFailOp,
		&frontStencilCompareOp,&frontStencilCompareMask,&frontStencilWriteMask,&frontStencilReference,&backStencilFailOp,&backStencilPassOp,
		&backStencilDepthFailOp,&backStencilCompareOp,&backStencilCompareMask,
		&backStencilWriteMask,&backStencilReference
	);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushInt(l,static_cast<uint32_t>(frontStencilFairOp));
	Lua::PushInt(l,static_cast<uint32_t>(frontStencilPassOp));
	Lua::PushInt(l,static_cast<uint32_t>(frontStencilDepthFailOp));
	Lua::PushInt(l,static_cast<uint32_t>(frontStencilCompareOp));
	Lua::PushInt(l,static_cast<uint32_t>(frontStencilCompareMask));
	Lua::PushInt(l,static_cast<uint32_t>(frontStencilWriteMask));
	Lua::PushInt(l,static_cast<uint32_t>(frontStencilReference));
	Lua::PushInt(l,static_cast<uint32_t>(backStencilFailOp));
	Lua::PushInt(l,static_cast<uint32_t>(backStencilPassOp));
	Lua::PushInt(l,static_cast<uint32_t>(backStencilDepthFailOp));
	Lua::PushInt(l,static_cast<uint32_t>(backStencilCompareOp));
	Lua::PushInt(l,static_cast<uint32_t>(backStencilCompareMask));
	Lua::PushInt(l,static_cast<uint32_t>(backStencilWriteMask));
	Lua::PushInt(l,static_cast<uint32_t>(backStencilReference));
}
void Lua::GraphicsPipelineCreateInfo::GetSubpassId(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.get_subpass_id());
}
void Lua::GraphicsPipelineCreateInfo::GetVertexAttributeProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iVerexInputAttribute)
{
	uint32_t location,offset,explicitVertexBindingIndex,stride;
	Anvil::Format format;
	Anvil::VertexInputRate rate;
	auto r = pipelineInfo.get_vertex_attribute_properties(iVerexInputAttribute,&location,&format,&offset,&explicitVertexBindingIndex,&stride,&rate);
	if(r == false)
		return;
	Lua::PushInt(l,location);
	Lua::PushInt(l,static_cast<uint32_t>(format));
	Lua::PushInt(l,stride);
	Lua::PushInt(l,static_cast<uint32_t>(rate));
}
void Lua::GraphicsPipelineCreateInfo::GetViewportProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iViewport)
{
	float originX,originY,w,h,minDepth,maxDepth;
	auto r = pipelineInfo.get_viewport_properties(iViewport,&originX,&originY,&w,&h,&minDepth,&maxDepth);
	if(r == false)
		return;
	Lua::PushNumber(l,originX);
	Lua::PushNumber(l,originY);
	Lua::PushNumber(l,w);
	Lua::PushNumber(l,h);
	Lua::PushNumber(l,minDepth);
	Lua::PushNumber(l,maxDepth);
}
void Lua::GraphicsPipelineCreateInfo::IsAlphaToCoverageEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.is_alpha_to_coverage_enabled());}
void Lua::GraphicsPipelineCreateInfo::IsAlphaToOneEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.is_alpha_to_one_enabled());}
void Lua::GraphicsPipelineCreateInfo::IsDepthClampEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.is_depth_clamp_enabled());}
void Lua::GraphicsPipelineCreateInfo::IsPrimitiveRestartEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.is_primitive_restart_enabled());}
void Lua::GraphicsPipelineCreateInfo::IsRasterizerDiscardEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.is_rasterizer_discard_enabled());}
void Lua::GraphicsPipelineCreateInfo::IsSampleMaskEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.is_sample_mask_enabled());}
void Lua::GraphicsPipelineCreateInfo::AttachVertexAttribute(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,const pragma::LuaVertexBinding &binding,luabind::object attributes)
{
	auto vertexAttributes = Lua::get_table_values<pragma::LuaVertexAttribute>(l,3u,[](lua_State *l,int32_t idx) {
		return *Lua::CheckVertexAttribute(l,idx);
	});
	auto *shader = static_cast<pragma::LuaShaderGraphicsBase*>(pragma::LuaShaderBase::GetShader(pipelineInfo));
	if(shader == nullptr)
	{
		Lua::PushBool(l,false);
		return;
	}
	Lua::PushBool(l,shader->AttachVertexAttribute(binding,vertexAttributes));
}
void Lua::GraphicsPipelineCreateInfo::AddSpecializationConstant(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t shaderStage,uint32_t constantId,::DataStream &ds)
{
	Lua::PushBool(l,pipelineInfo.add_specialization_constant(static_cast<Anvil::ShaderStage>(shaderStage),constantId,ds->GetSize(),ds->GetData()));
}
void Lua::GraphicsPipelineCreateInfo::SetAlphaToCoverageEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.toggle_alpha_to_coverage(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetAlphaToOneEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.toggle_alpha_to_one(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,float depthBiasConstantFactor,float depthBiasClamp,float depthBiasSlopeFactor)
{
	pipelineInfo.toggle_depth_bias(bEnabled,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasConstantFactor(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,float depthBiasConstantFactor)
{
	bool bEnabled;
	float depthBiasClamp,depthBiasSlopeFactor;
	pipelineInfo.get_depth_bias_state(&bEnabled,nullptr,&depthBiasClamp,&depthBiasSlopeFactor);
	pipelineInfo.toggle_depth_bias(bEnabled,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasClamp(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,float depthBiasClamp)
{
	bool bEnabled;
	float depthBiasConstantFactor,depthBiasSlopeFactor;
	pipelineInfo.get_depth_bias_state(&bEnabled,&depthBiasConstantFactor,nullptr,&depthBiasSlopeFactor);
	pipelineInfo.toggle_depth_bias(bEnabled,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasSlopeFactor(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,float depthBiasSlopeFactor)
{
	bool bEnabled;
	float depthBiasConstantFactor,depthBiasClamp;
	pipelineInfo.get_depth_bias_state(&bEnabled,&depthBiasConstantFactor,&depthBiasClamp,nullptr);
	pipelineInfo.toggle_depth_bias(bEnabled,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled)
{
	float depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor;
	pipelineInfo.get_depth_bias_state(nullptr,&depthBiasConstantFactor,&depthBiasClamp,&depthBiasSlopeFactor);
	pipelineInfo.toggle_depth_bias(bEnabled,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBoundsTestProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,float minDepthBounds,float maxDepthBounds)
{
	pipelineInfo.toggle_depth_bounds_test(bEnabled,minDepthBounds,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetMinDepthBounds(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,float minDepthBounds)
{
	bool bEnabled;
	float maxDepthBounds;
	pipelineInfo.get_depth_bounds_state(&bEnabled,nullptr,&maxDepthBounds);
	pipelineInfo.toggle_depth_bounds_test(bEnabled,minDepthBounds,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetMaxDepthBounds(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,float maxDepthBounds)
{
	bool bEnabled;
	float minDepthBounds;
	pipelineInfo.get_depth_bounds_state(&bEnabled,&minDepthBounds,nullptr);
	pipelineInfo.toggle_depth_bounds_test(bEnabled,minDepthBounds,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBoundsTestEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled)
{
	float minDepthBounds,maxDepthBounds;
	pipelineInfo.get_depth_bounds_state(nullptr,&minDepthBounds,&maxDepthBounds);
	pipelineInfo.toggle_depth_bounds_test(bEnabled,minDepthBounds,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthClampEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.toggle_depth_clamp(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetDepthTestProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,uint32_t compareOp)
{
	pipelineInfo.toggle_depth_test(bEnabled,static_cast<Anvil::CompareOp>(compareOp));
}
void Lua::GraphicsPipelineCreateInfo::SetDepthTestEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled)
{
	Anvil::CompareOp compareOp;
	pipelineInfo.get_depth_test_state(nullptr,&compareOp);
	pipelineInfo.toggle_depth_test(bEnabled,static_cast<Anvil::CompareOp>(compareOp));
}
void Lua::GraphicsPipelineCreateInfo::SetDepthWritesEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.toggle_depth_writes(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetDynamicStates(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t states)
{
	prosper::util::set_dynamic_states_enabled(pipelineInfo,prosper::util::get_enabled_dynamic_states(pipelineInfo),false);
	prosper::util::set_dynamic_states_enabled(pipelineInfo,static_cast<prosper::util::DynamicStateFlags>(states),true);
}
void Lua::GraphicsPipelineCreateInfo::SetDynamicStateEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t state,bool bEnabled)
{
	prosper::util::set_dynamic_states_enabled(pipelineInfo,static_cast<prosper::util::DynamicStateFlags>(state),bEnabled);
}
void Lua::GraphicsPipelineCreateInfo::SetLogicOpProperties(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,uint32_t logicOp)
{
	pipelineInfo.toggle_logic_op(bEnabled,static_cast<Anvil::LogicOp>(logicOp));
}
void Lua::GraphicsPipelineCreateInfo::SetLogicOpEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled)
{
	Anvil::LogicOp logicOp;
	pipelineInfo.get_logic_op_state(nullptr,&logicOp);
	pipelineInfo.toggle_logic_op(bEnabled,static_cast<Anvil::LogicOp>(logicOp));
}
void Lua::GraphicsPipelineCreateInfo::SetPrimitiveRestartEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.toggle_primitive_restart(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetRasterizerDiscardEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.toggle_rasterizer_discard(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetSampleMaskEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.toggle_sample_mask(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetSampleShadingEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.toggle_sample_shading(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetStencilTestEnabled(lua_State *l,Anvil::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.toggle_stencil_test(bEnabled);}

void Lua::Shader::Graphics::RecordBindVertexBuffer(lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::Buffer &buffer,uint32_t startBinding,uint32_t offset)
{
	Lua::PushBool(l,shader.RecordBindVertexBuffer(buffer->GetAnvilBuffer(),startBinding,offset));
}
void Lua::Shader::Graphics::RecordBindVertexBuffers(lua_State *l,prosper::ShaderGraphics &shader,luabind::object buffers,uint32_t startBinding,luabind::object offsets)
{
	auto vBuffers = Lua::get_table_values<Anvil::Buffer*>(l,2,[](lua_State *l,int32_t idx) {
		return &(*Lua::CheckVKBuffer(l,idx))->GetAnvilBuffer();
	});
	std::vector<uint64_t> vOffsets;
	if(Lua::IsSet(l,4))
	{
		vOffsets = Lua::get_table_values<uint64_t>(l,4,[](lua_State *l,int32_t idx) {
			return static_cast<ptrdiff_t>(Lua::CheckInt(l,idx));
		});
	}
	Lua::PushBool(l,shader.RecordBindVertexBuffers(vBuffers,startBinding,vOffsets));
}
void Lua::Shader::Graphics::RecordBindIndexBuffer(lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::Buffer &indexBuffer,uint32_t indexType,uint32_t offset)
{
	Lua::PushBool(l,shader.RecordBindIndexBuffer(indexBuffer->GetAnvilBuffer(),static_cast<Anvil::IndexType>(indexType),offset));
}
void Lua::Shader::Graphics::RecordDraw(lua_State *l,prosper::ShaderGraphics &shader,uint32_t vertCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance)
{
	Lua::PushBool(l,shader.RecordDraw(vertCount,instanceCount,firstVertex,firstInstance));
}
void Lua::Shader::Graphics::RecordDrawIndexed(lua_State *l,prosper::ShaderGraphics &shader,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,int32_t vertexOffset,uint32_t firstInstance)
{
	Lua::PushBool(l,shader.RecordDrawIndexed(indexCount,instanceCount,firstIndex,vertexOffset,firstInstance));
}
void Lua::Shader::Graphics::RecordBeginDraw(lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t pipelineIdx)
{
	if((*hCommandBuffer)->get_command_buffer_type() != Anvil::CommandBufferType::COMMAND_BUFFER_TYPE_PRIMARY)
	{
		Lua::PushBool(l,false);
		return;
	}
	Lua::PushBool(l,shader.BeginDraw(std::static_pointer_cast<prosper::PrimaryCommandBuffer>(hCommandBuffer),pipelineIdx));
}
void Lua::Shader::Graphics::RecordDraw(lua_State *l,prosper::ShaderGraphics &shader)
{
	Lua::PushBool(l,shader.Draw());
}
void Lua::Shader::Graphics::RecordEndDraw(lua_State *l,prosper::ShaderGraphics &shader)
{
	shader.EndDraw();
	Lua::PushBool(l,true);
}
void Lua::Shader::Graphics::GetRenderPass(lua_State *l,prosper::ShaderGraphics &shader,uint32_t pipelineIdx)
{
	auto &rp = shader.GetRenderPass(pipelineIdx);
	if(rp == nullptr)
		return;
	Lua::Push(l,rp);
}
void Lua::Shader::Scene3D::GetRenderPass(lua_State *l,uint32_t pipelineIdx)
{
	auto &rp = prosper::ShaderGraphics::GetRenderPass<pragma::ShaderScene>(*c_engine,pipelineIdx);
	if(rp == nullptr)
		return;
	Lua::Push(l,rp);
}
void Lua::Shader::Scene3D::BindSceneCamera(lua_State *l,pragma::ShaderScene &shader,std::shared_ptr<::Scene> &scene,bool bView)
{
	Lua::PushBool(l,shader.BindSceneCamera(*scene,bView));
}
void Lua::Shader::Scene3D::BindRenderSettings(lua_State *l,pragma::ShaderScene &shader,std::shared_ptr<Anvil::DescriptorSetGroup> &descSet)
{
	Lua::PushBool(l,shader.BindRenderSettings(*descSet->get_descriptor_set(0u)));
}
void Lua::Shader::SceneLit3D::BindLights(lua_State *l,pragma::ShaderSceneLit &shader,std::shared_ptr<Anvil::DescriptorSetGroup> &descSetShadowMaps,std::shared_ptr<Anvil::DescriptorSetGroup> &descSetLightSources)
{
	Lua::PushBool(l,shader.BindLights(*descSetShadowMaps->get_descriptor_set(0u),*descSetLightSources->get_descriptor_set(0u)));
}
void Lua::Shader::SceneLit3D::BindScene(lua_State *l,pragma::ShaderSceneLit &shader,std::shared_ptr<::Scene> &scene,bool bView)
{
	Lua::PushBool(l,shader.BindScene(*scene,bView));
}
void Lua::Shader::ShaderEntity::BindInstanceDescriptorSet(lua_State *l,pragma::ShaderEntity &shader,std::shared_ptr<Anvil::DescriptorSetGroup> &descSet)
{
	Lua::PushBool(l,shader.BindInstanceDescriptorSet(*descSet->get_descriptor_set(0u)));
}
void Lua::Shader::ShaderEntity::BindEntity(lua_State *l,pragma::ShaderEntity &shader,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::Push(l,shader.BindEntity(static_cast<CBaseEntity&>(*hEnt.get())));
}
void Lua::Shader::ShaderEntity::BindVertexAnimationOffset(lua_State *l,pragma::ShaderEntity &shader,uint32_t offset)
{
	Lua::PushBool(l,shader.BindVertexAnimationOffset(offset));
}
void Lua::Shader::ShaderEntity::Draw(lua_State *l,pragma::ShaderEntity &shader,std::shared_ptr<::ModelSubMesh> &mesh)
{
	Lua::PushBool(l,shader.Draw(static_cast<CModelSubMesh&>(*mesh)));
}
void Lua::Shader::TexturedLit3D::BindMaterial(lua_State *l,pragma::ShaderTextured3DBase &shader,::Material &mat)
{
	Lua::PushBool(l,shader.BindMaterial(static_cast<CMaterial&>(mat)));
}
void Lua::Shader::TexturedLit3D::RecordBindClipPlane(lua_State *l,pragma::ShaderTextured3DBase &shader,const Vector4 &clipPlane)
{
	Lua::PushBool(l,shader.BindClipPlane(clipPlane));
}
