/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/c_shader_lua.hpp"
#include "pragma/model/c_modelmesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <pragma/lua/classes/ldef_entity.h>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_render_pass.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;

void Lua::GraphicsPipelineCreateInfo::SetBlendingProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,const Vector4 &blendingProperties)
{
	pipelineInfo.SetBlendingProperties(reinterpret_cast<const float*>(&blendingProperties));
}
void Lua::GraphicsPipelineCreateInfo::SetCommonAlphaBlendProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
}
void Lua::GraphicsPipelineCreateInfo::SetColorBlendAttachmentProperties(
	lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t attId,bool blendingEnabled,
	uint32_t blendOpColor,uint32_t blendOpAlpha,uint32_t srcColorBlendFactor,uint32_t dstColorBlendFactor,uint32_t srcAlphaBlendFactor,
	uint32_t dstAlphaBlendFactor,uint32_t channelWriteMask
)
{
	pipelineInfo.SetColorBlendAttachmentProperties(
		attId,blendingEnabled,static_cast<prosper::BlendOp>(blendOpColor),static_cast<prosper::BlendOp>(blendOpAlpha),static_cast<prosper::BlendFactor>(srcColorBlendFactor),
		static_cast<prosper::BlendFactor>(dstColorBlendFactor),static_cast<prosper::BlendFactor>(srcAlphaBlendFactor),static_cast<prosper::BlendFactor>(dstAlphaBlendFactor),
		static_cast<prosper::ColorComponentFlags>(channelWriteMask)
	);
}
void Lua::GraphicsPipelineCreateInfo::SetMultisamplingProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t sampleCount,float sampleShading,uint32_t sampleMask)
{
	pipelineInfo.SetMultisamplingProperties(static_cast<prosper::SampleCountFlags>(sampleCount),sampleShading,static_cast<prosper::SampleMask>(sampleMask));
}
void Lua::GraphicsPipelineCreateInfo::SetSampleCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t sampleCount)
{
	const prosper::SampleMask *sampleMask;
	float minSampleShading;
	pipelineInfo.GetMultisamplingProperties(nullptr,&sampleMask);
	pipelineInfo.GetSampleShadingState(nullptr,&minSampleShading);
	pipelineInfo.SetMultisamplingProperties(static_cast<prosper::SampleCountFlags>(sampleCount),minSampleShading,sampleMask ? *sampleMask : 0u);
}
void Lua::GraphicsPipelineCreateInfo::SetMinSampleShading(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float minSampleShading)
{
	prosper::SampleCountFlags sampleCount;
	const prosper::SampleMask *sampleMask;
	pipelineInfo.GetMultisamplingProperties(&sampleCount,&sampleMask);
	pipelineInfo.SetMultisamplingProperties(static_cast<prosper::SampleCountFlags>(sampleCount),minSampleShading,sampleMask ? *sampleMask : 0u);
}
void Lua::GraphicsPipelineCreateInfo::SetSampleMask(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t sampleMask)
{
	prosper::SampleCountFlags sampleCount;
	float minSampleShading;
	pipelineInfo.GetMultisamplingProperties(&sampleCount,nullptr);
	pipelineInfo.GetSampleShadingState(nullptr,&minSampleShading);
	pipelineInfo.SetMultisamplingProperties(static_cast<prosper::SampleCountFlags>(sampleCount),minSampleShading,sampleMask);
}
void Lua::GraphicsPipelineCreateInfo::SetDynamicScissorBoxesCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t numDynamicScissorBoxes)
{
	pipelineInfo.SetDynamicScissorBoxesCount(numDynamicScissorBoxes);
}
void Lua::GraphicsPipelineCreateInfo::SetDynamicViewportsCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t numDynamicViewports)
{
	pipelineInfo.SetDynamicViewportCount(numDynamicViewports);
}
void Lua::GraphicsPipelineCreateInfo::SetPrimitiveTopology(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t primitiveTopology)
{
	pipelineInfo.SetPrimitiveTopology(static_cast<prosper::PrimitiveTopology>(primitiveTopology));
}
void Lua::GraphicsPipelineCreateInfo::SetRasterizationProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t polygonMode,uint32_t cullMode,uint32_t frontFace,float lineWidth)
{
	pipelineInfo.SetRasterizationProperties(static_cast<prosper::PolygonMode>(polygonMode),static_cast<prosper::CullModeFlags>(cullMode),static_cast<prosper::FrontFace>(frontFace),lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetPolygonMode(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t polygonMode)
{
	prosper::CullModeFlags cullMode;
	prosper::FrontFace frontFace;
	float lineWidth;
	pipelineInfo.GetRasterizationProperties(nullptr,&cullMode,&frontFace,&lineWidth);
	pipelineInfo.SetRasterizationProperties(static_cast<prosper::PolygonMode>(polygonMode),cullMode,frontFace,lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetCullMode(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t cullMode)
{
	prosper::PolygonMode polygonMode;
	prosper::FrontFace frontFace;
	float lineWidth;
	pipelineInfo.GetRasterizationProperties(&polygonMode,nullptr,&frontFace,&lineWidth);
	pipelineInfo.SetRasterizationProperties(polygonMode,static_cast<prosper::CullModeFlags>(cullMode),frontFace,lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetFrontFace(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t frontFace)
{
	prosper::PolygonMode polygonMode;
	prosper::CullModeFlags cullMode;
	float lineWidth;
	pipelineInfo.GetRasterizationProperties(&polygonMode,&cullMode,nullptr,&lineWidth);
	pipelineInfo.SetRasterizationProperties(polygonMode,cullMode,static_cast<prosper::FrontFace>(frontFace),lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetLineWidth(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float lineWidth)
{
	prosper::PolygonMode polygonMode;
	prosper::CullModeFlags cullMode;
	prosper::FrontFace frontFace;
	pipelineInfo.GetRasterizationProperties(&polygonMode,&cullMode,&frontFace,nullptr);
	pipelineInfo.SetRasterizationProperties(polygonMode,cullMode,frontFace,lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetScissorBoxProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iScissorBox,int32_t x,int32_t y,uint32_t w,uint32_t h)
{
	pipelineInfo.SetScissorBoxProperties(iScissorBox,x,y,w,h);
}
void Lua::GraphicsPipelineCreateInfo::SetStencilTestProperties(
	lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool updateFrontFaceState,uint32_t stencilFailOp,uint32_t stencilPassOp,
	uint32_t stencilDepthFailOp,uint32_t stencilCompareOp,uint32_t stencilCompareMask,uint32_t stencilWriteMask,uint32_t stencilReference
)
{
	pipelineInfo.SetStencilTestProperties(
		updateFrontFaceState,static_cast<prosper::StencilOp>(stencilFailOp),static_cast<prosper::StencilOp>(stencilPassOp),static_cast<prosper::StencilOp>(stencilDepthFailOp),
		static_cast<prosper::CompareOp>(stencilCompareOp),stencilCompareMask,stencilWriteMask,stencilReference
	);
}
void Lua::GraphicsPipelineCreateInfo::SetViewportProperties(
	lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iViewport,float originX,float originY,
	float w,float h,float minDepth,float maxDepth
)
{
	pipelineInfo.SetViewportProperties(iViewport,originX,originY,w,h,minDepth,maxDepth);
}
void Lua::GraphicsPipelineCreateInfo::AreDepthWritesEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushBool(l,pipelineInfo.AreDepthWritesEnabled());
}
void Lua::GraphicsPipelineCreateInfo::GetBlendingProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	const float *blendingProperties;
	pipelineInfo.GetBlendingProperties(&blendingProperties,nullptr);
	Lua::Push<Vector4>(l,Vector4{blendingProperties[0],blendingProperties[1],blendingProperties[2],blendingProperties[3]});
}
void Lua::GraphicsPipelineCreateInfo::GetColorBlendAttachmentProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t attId)
{
	bool blendingEnabled;
	prosper::BlendOp blendOpColor,blendOpAlpha;
	prosper::BlendFactor srcColorBlendFactor,dstColorBlendFactor,srcAlphaBlendFactor,dstAlphaBlendFactor;
	prosper::ColorComponentFlags channelWriteMask;
	auto r = pipelineInfo.GetColorBlendAttachmentProperties(
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
	Lua::PushInt(l,static_cast<uint32_t>(channelWriteMask));
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasState(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	float depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor;
	pipelineInfo.GetDepthBiasState(&bEnabled,&depthBiasConstantFactor,&depthBiasClamp,&depthBiasSlopeFactor);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushNumber(l,depthBiasConstantFactor);
	Lua::PushNumber(l,depthBiasClamp);
	Lua::PushNumber(l,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasConstantFactor(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float depthBiasConstantFactor;
	pipelineInfo.GetDepthBiasState(nullptr,&depthBiasConstantFactor,nullptr,nullptr);
	Lua::PushNumber(l,depthBiasConstantFactor);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasClamp(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float depthBiasClamp;
	pipelineInfo.GetDepthBiasState(nullptr,nullptr,&depthBiasClamp,nullptr);
	Lua::PushNumber(l,depthBiasClamp);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasSlopeFactor(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float depthBiasSlopeFactor;
	pipelineInfo.GetDepthBiasState(nullptr,nullptr,nullptr,&depthBiasSlopeFactor);
	Lua::PushNumber(l,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBoundsState(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	float minDepthBounds,maxDepthBounds;
	pipelineInfo.GetDepthBoundsState(&bEnabled,&minDepthBounds,&maxDepthBounds);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushNumber(l,minDepthBounds);
	Lua::PushNumber(l,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetMinDepthBounds(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float minDepthBounds;
	pipelineInfo.GetDepthBoundsState(nullptr,&minDepthBounds,nullptr);
	Lua::PushNumber(l,minDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetMaxDepthBounds(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float maxDepthBounds;
	pipelineInfo.GetDepthBoundsState(nullptr,nullptr,&maxDepthBounds);
	Lua::PushNumber(l,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthClamp(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float minDepthBounds,maxDepthBounds;
	pipelineInfo.GetDepthBoundsState(nullptr,&minDepthBounds,&maxDepthBounds);
	Lua::PushNumber(l,minDepthBounds);
	Lua::PushNumber(l,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthTestState(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	prosper::CompareOp compareOp;
	pipelineInfo.GetDepthTestState(&bEnabled,&compareOp);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushInt(l,static_cast<uint32_t>(compareOp));
}
void Lua::GraphicsPipelineCreateInfo::GetDynamicStates(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,prosper::util::get_enabled_dynamic_states(pipelineInfo));
}
void Lua::GraphicsPipelineCreateInfo::IsDynamicStateEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t dynamicState)
{
	Lua::PushBool(l,prosper::util::are_dynamic_states_enabled(pipelineInfo,static_cast<prosper::util::DynamicStateFlags>(dynamicState)));
}
void Lua::GraphicsPipelineCreateInfo::GetScissorCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	uint32_t scissorCount;
	pipelineInfo.GetGraphicsPipelineProperties(&scissorCount,nullptr,nullptr,nullptr,nullptr);
	Lua::PushInt(l,scissorCount);
}
void Lua::GraphicsPipelineCreateInfo::GetViewportCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	uint32_t viewportCount;
	pipelineInfo.GetGraphicsPipelineProperties(nullptr,&viewportCount,nullptr,nullptr,nullptr);
	Lua::PushInt(l,viewportCount);
}
void Lua::GraphicsPipelineCreateInfo::GetVertexAttributeCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	uint32_t vertexAttributeCount;
	pipelineInfo.GetGraphicsPipelineProperties(nullptr,nullptr,&vertexAttributeCount,nullptr,nullptr);
	Lua::PushInt(l,vertexAttributeCount);
}
void Lua::GraphicsPipelineCreateInfo::GetLogicOpState(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	prosper::LogicOp logicOp;
	pipelineInfo.GetLogicOpState(&bEnabled,&logicOp);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushInt(l,static_cast<uint32_t>(logicOp));
}
void Lua::GraphicsPipelineCreateInfo::GetMultisamplingProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::SampleCountFlags samples;
	const prosper::SampleMask *sampleMask;
	pipelineInfo.GetMultisamplingProperties(&samples,&sampleMask);
	Lua::PushInt(l,umath::to_integral(samples));
	Lua::PushInt(l,sampleMask ? static_cast<uint32_t>(*sampleMask) : 0u);
}
void Lua::GraphicsPipelineCreateInfo::GetSampleCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::SampleCountFlags samples;
	pipelineInfo.GetMultisamplingProperties(&samples,nullptr);
	Lua::PushInt(l,umath::to_integral(samples));
}
void Lua::GraphicsPipelineCreateInfo::GetMinSampleShading(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	const prosper::SampleMask *sampleMask;
	pipelineInfo.GetMultisamplingProperties(nullptr,&sampleMask);
	Lua::PushInt(l,sampleMask ? static_cast<uint32_t>(*sampleMask) : 0u);
}
void Lua::GraphicsPipelineCreateInfo::GetSampleMask(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float minSampleShading;
	pipelineInfo.GetSampleShadingState(nullptr,&minSampleShading);
	Lua::PushNumber(l,minSampleShading);
}
void Lua::GraphicsPipelineCreateInfo::GetDynamicScissorBoxesCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.GetDynamicScissorBoxesCount());
}
void Lua::GraphicsPipelineCreateInfo::GetDynamicViewportsCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.GetDynamicViewportsCount());
}
void Lua::GraphicsPipelineCreateInfo::GetScissorBoxesCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.GetScissorBoxesCount());
}
void Lua::GraphicsPipelineCreateInfo::GetViewportsCount(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.GetViewportCount());
}
void Lua::GraphicsPipelineCreateInfo::GetPrimitiveTopology(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,static_cast<uint32_t>(pipelineInfo.GetPrimitiveTopology()));
}
void Lua::GraphicsPipelineCreateInfo::GetPushConstantRanges(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	auto &ranges = pipelineInfo.GetPushConstantRanges();
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
		Lua::PushInt(l,static_cast<uint32_t>(range.stages));
		Lua::SetTableValue(l,tRange);

		Lua::SetTableValue(l,t);
	}
}
void Lua::GraphicsPipelineCreateInfo::GetRasterizationProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::PolygonMode polygonMode;
	prosper::CullModeFlags cullMode;
	prosper::FrontFace frontFace;
	float lineWidth;
	pipelineInfo.GetRasterizationProperties(&polygonMode,&cullMode,&frontFace,&lineWidth);
	Lua::PushInt(l,static_cast<uint32_t>(polygonMode));
	Lua::PushInt(l,static_cast<uint32_t>(cullMode));
	Lua::PushInt(l,static_cast<uint32_t>(frontFace));
	Lua::PushNumber(l,lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::GetPolygonMode(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::PolygonMode polygonMode;
	pipelineInfo.GetRasterizationProperties(&polygonMode,nullptr,nullptr,nullptr);
	Lua::PushInt(l,static_cast<uint32_t>(polygonMode));
}
void Lua::GraphicsPipelineCreateInfo::GetCullMode(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::CullModeFlags cullMode;
	pipelineInfo.GetRasterizationProperties(nullptr,&cullMode,nullptr,nullptr);
	Lua::PushInt(l,static_cast<uint32_t>(cullMode));
}
void Lua::GraphicsPipelineCreateInfo::GetFrontFace(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::FrontFace frontFace;
	pipelineInfo.GetRasterizationProperties(nullptr,nullptr,&frontFace,nullptr);
	Lua::PushInt(l,static_cast<uint32_t>(frontFace));
}
void Lua::GraphicsPipelineCreateInfo::GetLineWidth(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float lineWidth;
	pipelineInfo.GetRasterizationProperties(nullptr,nullptr,nullptr,&lineWidth);
	Lua::PushNumber(l,lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::GetSampleShadingState(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	float minSampleShading;
	pipelineInfo.GetSampleShadingState(&bEnabled,&minSampleShading);
	Lua::PushBool(l,bEnabled);
	if(bEnabled == false)
		return;
	Lua::PushNumber(l,minSampleShading);
}
void Lua::GraphicsPipelineCreateInfo::GetScissorBoxProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iScissor)
{
	int32_t x,y;
	uint32_t w,h;
	auto r = pipelineInfo.GetScissorBoxProperties(iScissor,&x,&y,&w,&h);
	if(r == false)
		return;
	Lua::PushInt(l,x);
	Lua::PushInt(l,y);
	Lua::PushInt(l,w);
	Lua::PushInt(l,h);
}
void Lua::GraphicsPipelineCreateInfo::GetStencilTestProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	prosper::StencilOp frontStencilFairOp,frontStencilPassOp,frontStencilDepthFailOp;
	prosper::CompareOp frontStencilCompareOp,backStencilCompareOp;
	uint32_t frontStencilCompareMask,frontStencilWriteMask,frontStencilReference,backStencilCompareMask,backStencilWriteMask,backStencilReference;
	prosper::StencilOp backStencilFailOp,backStencilPassOp,backStencilDepthFailOp;

	pipelineInfo.GetStencilTestProperties(
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
void Lua::GraphicsPipelineCreateInfo::GetSubpassId(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	Lua::PushInt(l,pipelineInfo.GetSubpassId());
}
void Lua::GraphicsPipelineCreateInfo::GetVertexAttributeProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iVerexInputAttribute)
{
	uint32_t location,offset,explicitVertexBindingIndex,stride;
	prosper::Format format;
	prosper::VertexInputRate rate;
	auto numAttributes = 0u;
	auto r = pipelineInfo.GetVertexBindingProperties(
		iVerexInputAttribute,&explicitVertexBindingIndex,
		&stride,&rate,&numAttributes);
	if(r == false)
		return;
	std::vector<const prosper::VertexInputAttribute*> attributes(numAttributes);
	r = pipelineInfo.GetVertexBindingProperties(iVerexInputAttribute,nullptr,nullptr,nullptr,nullptr,attributes.data());
	if(r == false || attributes.empty())
		return;
	// TODO: Push all attributes as table?
	location = attributes.front()->location;
	format = attributes.front()->format;
	Lua::PushInt(l,location);
	Lua::PushInt(l,static_cast<uint32_t>(format));
	Lua::PushInt(l,stride);
	Lua::PushInt(l,static_cast<uint32_t>(rate));
}
void Lua::GraphicsPipelineCreateInfo::GetViewportProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t iViewport)
{
	float originX,originY,w,h,minDepth,maxDepth;
	auto r = pipelineInfo.GetViewportProperties(iViewport,&originX,&originY,&w,&h,&minDepth,&maxDepth);
	if(r == false)
		return;
	Lua::PushNumber(l,originX);
	Lua::PushNumber(l,originY);
	Lua::PushNumber(l,w);
	Lua::PushNumber(l,h);
	Lua::PushNumber(l,minDepth);
	Lua::PushNumber(l,maxDepth);
}
void Lua::GraphicsPipelineCreateInfo::IsAlphaToCoverageEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.IsAlphaToCoverageEnabled());}
void Lua::GraphicsPipelineCreateInfo::IsAlphaToOneEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.IsAlphaToOneEnabled());}
void Lua::GraphicsPipelineCreateInfo::IsDepthClampEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.IsDepthClampEnabled());}
void Lua::GraphicsPipelineCreateInfo::IsPrimitiveRestartEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.IsPrimitiveRestartEnabled());}
void Lua::GraphicsPipelineCreateInfo::IsRasterizerDiscardEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.IsRasterizerDiscardEnabled());}
void Lua::GraphicsPipelineCreateInfo::IsSampleMaskEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo) {Lua::PushBool(l,pipelineInfo.IsSampleMaskEnabled());}
void Lua::GraphicsPipelineCreateInfo::AttachVertexAttribute(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,const pragma::LuaVertexBinding &binding,luabind::object attributes)
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
void Lua::GraphicsPipelineCreateInfo::AddSpecializationConstant(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t shaderStage,uint32_t constantId,::DataStream &ds)
{
	Lua::PushBool(l,pipelineInfo.AddSpecializationConstant(static_cast<prosper::ShaderStage>(shaderStage),constantId,ds->GetSize(),ds->GetData()));
}
void Lua::GraphicsPipelineCreateInfo::SetAlphaToCoverageEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.ToggleAlphaToCoverage(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetAlphaToOneEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.ToggleAlphaToOne(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,float depthBiasConstantFactor,float depthBiasClamp,float depthBiasSlopeFactor)
{
	pipelineInfo.ToggleDepthBias(bEnabled,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasConstantFactor(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float depthBiasConstantFactor)
{
	bool bEnabled;
	float depthBiasClamp,depthBiasSlopeFactor;
	pipelineInfo.GetDepthBiasState(&bEnabled,nullptr,&depthBiasClamp,&depthBiasSlopeFactor);
	pipelineInfo.ToggleDepthBias(bEnabled,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasClamp(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float depthBiasClamp)
{
	bool bEnabled;
	float depthBiasConstantFactor,depthBiasSlopeFactor;
	pipelineInfo.GetDepthBiasState(&bEnabled,&depthBiasConstantFactor,nullptr,&depthBiasSlopeFactor);
	pipelineInfo.ToggleDepthBias(bEnabled,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasSlopeFactor(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float depthBiasSlopeFactor)
{
	bool bEnabled;
	float depthBiasConstantFactor,depthBiasClamp;
	pipelineInfo.GetDepthBiasState(&bEnabled,&depthBiasConstantFactor,&depthBiasClamp,nullptr);
	pipelineInfo.ToggleDepthBias(bEnabled,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled)
{
	float depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor;
	pipelineInfo.GetDepthBiasState(nullptr,&depthBiasConstantFactor,&depthBiasClamp,&depthBiasSlopeFactor);
	pipelineInfo.ToggleDepthBias(bEnabled,depthBiasConstantFactor,depthBiasClamp,depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBoundsTestProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,float minDepthBounds,float maxDepthBounds)
{
	pipelineInfo.ToggleDepthBoundsTest(bEnabled,minDepthBounds,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetMinDepthBounds(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float minDepthBounds)
{
	bool bEnabled;
	float maxDepthBounds;
	pipelineInfo.GetDepthBoundsState(&bEnabled,nullptr,&maxDepthBounds);
	pipelineInfo.ToggleDepthBoundsTest(bEnabled,minDepthBounds,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetMaxDepthBounds(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,float maxDepthBounds)
{
	bool bEnabled;
	float minDepthBounds;
	pipelineInfo.GetDepthBoundsState(&bEnabled,&minDepthBounds,nullptr);
	pipelineInfo.ToggleDepthBoundsTest(bEnabled,minDepthBounds,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBoundsTestEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled)
{
	float minDepthBounds,maxDepthBounds;
	pipelineInfo.GetDepthBoundsState(nullptr,&minDepthBounds,&maxDepthBounds);
	pipelineInfo.ToggleDepthBoundsTest(bEnabled,minDepthBounds,maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthClampEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.ToggleDepthClamp(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetDepthTestProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,uint32_t compareOp)
{
	pipelineInfo.ToggleDepthTest(bEnabled,static_cast<prosper::CompareOp>(compareOp));
}
void Lua::GraphicsPipelineCreateInfo::SetDepthTestEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled)
{
	// prosper::CompareOp compareOp;
	// pipelineInfo.GetDepthTestState(nullptr,&compareOp);
	pipelineInfo.ToggleDepthTest(bEnabled,prosper::CompareOp::LessOrEqual);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthWritesEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.ToggleDepthWrites(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetDynamicStates(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t states)
{
	prosper::util::set_dynamic_states_enabled(pipelineInfo,prosper::util::get_enabled_dynamic_states(pipelineInfo),false);
	prosper::util::set_dynamic_states_enabled(pipelineInfo,static_cast<prosper::util::DynamicStateFlags>(states),true);
}
void Lua::GraphicsPipelineCreateInfo::SetDynamicStateEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t state,bool bEnabled)
{
	prosper::util::set_dynamic_states_enabled(pipelineInfo,static_cast<prosper::util::DynamicStateFlags>(state),bEnabled);
}
void Lua::GraphicsPipelineCreateInfo::SetLogicOpProperties(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled,uint32_t logicOp)
{
	pipelineInfo.ToggleLogicOp(bEnabled,static_cast<prosper::LogicOp>(logicOp));
}
void Lua::GraphicsPipelineCreateInfo::SetLogicOpEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled)
{
	prosper::LogicOp logicOp;
	pipelineInfo.GetLogicOpState(nullptr,&logicOp);
	pipelineInfo.ToggleLogicOp(bEnabled,static_cast<prosper::LogicOp>(logicOp));
}
void Lua::GraphicsPipelineCreateInfo::SetPrimitiveRestartEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.TogglePrimitiveRestart(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetRasterizerDiscardEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.ToggleRasterizerDiscard(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetSampleMaskEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.ToggleSampleMask(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetSampleShadingEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.ToggleSampleShading(bEnabled);}
void Lua::GraphicsPipelineCreateInfo::SetStencilTestEnabled(lua_State *l,prosper::GraphicsPipelineCreateInfo &pipelineInfo,bool bEnabled) {pipelineInfo.ToggleStencilTest(bEnabled);}

void Lua::Shader::Graphics::RecordBindVertexBuffer(lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::Buffer &buffer,uint32_t startBinding,uint32_t offset)
{
	Lua::PushBool(l,shader.RecordBindVertexBuffer(buffer,startBinding,offset));
}
void Lua::Shader::Graphics::RecordBindVertexBuffers(lua_State *l,prosper::ShaderGraphics &shader,luabind::object buffers,uint32_t startBinding,luabind::object offsets)
{
	auto vBuffers = Lua::get_table_values<prosper::IBuffer*>(l,2,[](lua_State *l,int32_t idx) {
		return &Lua::Check<Lua::Vulkan::Buffer>(l,idx);
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
	Lua::PushBool(l,shader.RecordBindIndexBuffer(indexBuffer,static_cast<prosper::IndexType>(indexType),offset));
}
void Lua::Shader::Graphics::RecordDraw(lua_State *l,prosper::ShaderGraphics &shader,uint32_t vertCount,uint32_t instanceCount,uint32_t firstVertex,uint32_t firstInstance)
{
	Lua::PushBool(l,shader.RecordDraw(vertCount,instanceCount,firstVertex,firstInstance));
}
void Lua::Shader::Graphics::RecordDrawIndexed(lua_State *l,prosper::ShaderGraphics &shader,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,uint32_t firstInstance)
{
	Lua::PushBool(l,shader.RecordDrawIndexed(indexCount,instanceCount,firstIndex,firstInstance));
}
void Lua::Shader::Graphics::RecordBeginDraw(lua_State *l,prosper::ShaderGraphics &shader,Lua::Vulkan::CommandBuffer &hCommandBuffer,uint32_t pipelineIdx)
{
	if(hCommandBuffer.IsPrimary() == false)
	{
		Lua::PushBool(l,false);
		return;
	}
	Lua::PushBool(l,shader.BeginDraw(std::dynamic_pointer_cast<prosper::IPrimaryCommandBuffer>(hCommandBuffer.shared_from_this()),pipelineIdx));
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
	auto &rp = prosper::ShaderGraphics::GetRenderPass<pragma::ShaderScene>(c_engine->GetRenderContext(),pipelineIdx);
	if(rp == nullptr)
		return;
	Lua::Push(l,rp);
}
void Lua::Shader::Scene3D::BindSceneCamera(lua_State *l,pragma::ShaderScene &shader,pragma::rendering::RasterizationRenderer &renderer,bool bView)
{
	Lua::PushBool(l,shader.BindSceneCamera(renderer,bView));
}
void Lua::Shader::Scene3D::BindRenderSettings(lua_State *l,pragma::ShaderScene &shader,std::shared_ptr<prosper::IDescriptorSetGroup> &descSet)
{
	Lua::PushBool(l,shader.BindRenderSettings(*descSet->GetDescriptorSet()));
}
void Lua::Shader::SceneLit3D::BindLights(lua_State *l,pragma::ShaderSceneLit &shader,std::shared_ptr<prosper::IDescriptorSetGroup> &descSetShadowMaps,std::shared_ptr<prosper::IDescriptorSetGroup> &descSetLightSources)
{
	Lua::PushBool(l,shader.BindLights(*descSetShadowMaps->GetDescriptorSet(),*descSetLightSources->GetDescriptorSet()));
}
void Lua::Shader::SceneLit3D::BindScene(lua_State *l,pragma::ShaderSceneLit &shader,pragma::rendering::RasterizationRenderer &renderer,bool bView)
{
	Lua::PushBool(l,shader.BindScene(renderer,bView));
}
void Lua::Shader::ShaderEntity::BindInstanceDescriptorSet(lua_State *l,pragma::ShaderEntity &shader,std::shared_ptr<prosper::IDescriptorSetGroup> &descSet)
{
	Lua::PushBool(l,shader.BindInstanceDescriptorSet(*descSet->GetDescriptorSet()));
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
void Lua::Shader::ShaderEntity::Draw(lua_State *l,pragma::ShaderEntity &shader,::ModelSubMesh &mesh)
{
	Lua::PushBool(l,shader.Draw(static_cast<CModelSubMesh&>(mesh)));
}
void Lua::Shader::TexturedLit3D::BindMaterial(lua_State *l,pragma::ShaderTextured3DBase &shader,::Material &mat)
{
	Lua::PushBool(l,shader.BindMaterial(static_cast<CMaterial&>(mat)));
}
void Lua::Shader::TexturedLit3D::RecordBindClipPlane(lua_State *l,pragma::ShaderTextured3DBase &shader,const Vector4 &clipPlane)
{
	Lua::PushBool(l,shader.BindClipPlane(clipPlane));
}
