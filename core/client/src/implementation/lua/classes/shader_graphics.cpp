// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.lua;
import :scripting.lua.classes.shader;
import :engine;

void Lua::GraphicsPipelineCreateInfo::SetBlendingProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, const ::Vector4 &blendingProperties) { pipelineInfo.SetBlendingProperties(reinterpret_cast<const float *>(&blendingProperties)); }
void Lua::GraphicsPipelineCreateInfo::SetCommonAlphaBlendProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo); }
void Lua::GraphicsPipelineCreateInfo::SetColorBlendAttachmentProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t attId, bool blendingEnabled, uint32_t blendOpColor, uint32_t blendOpAlpha, uint32_t srcColorBlendFactor, uint32_t dstColorBlendFactor,
  uint32_t srcAlphaBlendFactor, uint32_t dstAlphaBlendFactor, uint32_t channelWriteMask)
{
	pipelineInfo.SetColorBlendAttachmentProperties(attId, blendingEnabled, static_cast<prosper::BlendOp>(blendOpColor), static_cast<prosper::BlendOp>(blendOpAlpha), static_cast<prosper::BlendFactor>(srcColorBlendFactor), static_cast<prosper::BlendFactor>(dstColorBlendFactor),
	  static_cast<prosper::BlendFactor>(srcAlphaBlendFactor), static_cast<prosper::BlendFactor>(dstAlphaBlendFactor), static_cast<prosper::ColorComponentFlags>(channelWriteMask));
}
void Lua::GraphicsPipelineCreateInfo::SetMultisamplingProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t sampleCount, float sampleShading, uint32_t sampleMask)
{
	pipelineInfo.SetMultisamplingProperties(static_cast<prosper::SampleCountFlags>(sampleCount), sampleShading, static_cast<prosper::SampleMask>(sampleMask));
}
void Lua::GraphicsPipelineCreateInfo::SetSampleCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t sampleCount)
{
	const prosper::SampleMask *sampleMask;
	float minSampleShading;
	pipelineInfo.GetMultisamplingProperties(nullptr, &sampleMask);
	pipelineInfo.GetSampleShadingState(nullptr, &minSampleShading);
	pipelineInfo.SetMultisamplingProperties(static_cast<prosper::SampleCountFlags>(sampleCount), minSampleShading, sampleMask ? *sampleMask : 0u);
}
void Lua::GraphicsPipelineCreateInfo::SetMinSampleShading(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float minSampleShading)
{
	prosper::SampleCountFlags sampleCount;
	const prosper::SampleMask *sampleMask;
	pipelineInfo.GetMultisamplingProperties(&sampleCount, &sampleMask);
	pipelineInfo.SetMultisamplingProperties(static_cast<prosper::SampleCountFlags>(sampleCount), minSampleShading, sampleMask ? *sampleMask : 0u);
}
void Lua::GraphicsPipelineCreateInfo::SetSampleMask(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t sampleMask)
{
	prosper::SampleCountFlags sampleCount;
	float minSampleShading;
	pipelineInfo.GetMultisamplingProperties(&sampleCount, nullptr);
	pipelineInfo.GetSampleShadingState(nullptr, &minSampleShading);
	pipelineInfo.SetMultisamplingProperties(static_cast<prosper::SampleCountFlags>(sampleCount), minSampleShading, sampleMask);
}
void Lua::GraphicsPipelineCreateInfo::SetDynamicScissorBoxesCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t numDynamicScissorBoxes) { pipelineInfo.SetDynamicScissorBoxesCount(numDynamicScissorBoxes); }
void Lua::GraphicsPipelineCreateInfo::SetDynamicViewportsCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t numDynamicViewports) { pipelineInfo.SetDynamicViewportCount(numDynamicViewports); }
void Lua::GraphicsPipelineCreateInfo::SetPrimitiveTopology(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t primitiveTopology) { pipelineInfo.SetPrimitiveTopology(static_cast<prosper::PrimitiveTopology>(primitiveTopology)); }
void Lua::GraphicsPipelineCreateInfo::SetRasterizationProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t polygonMode, uint32_t cullMode, uint32_t frontFace, float lineWidth)
{
	pipelineInfo.SetRasterizationProperties(static_cast<prosper::PolygonMode>(polygonMode), static_cast<prosper::CullModeFlags>(cullMode), static_cast<prosper::FrontFace>(frontFace), lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetPolygonMode(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t polygonMode)
{
	prosper::CullModeFlags cullMode;
	prosper::FrontFace frontFace;
	float lineWidth;
	pipelineInfo.GetRasterizationProperties(nullptr, &cullMode, &frontFace, &lineWidth);
	pipelineInfo.SetRasterizationProperties(static_cast<prosper::PolygonMode>(polygonMode), cullMode, frontFace, lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetCullMode(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t cullMode)
{
	prosper::PolygonMode polygonMode;
	prosper::FrontFace frontFace;
	float lineWidth;
	pipelineInfo.GetRasterizationProperties(&polygonMode, nullptr, &frontFace, &lineWidth);
	pipelineInfo.SetRasterizationProperties(polygonMode, static_cast<prosper::CullModeFlags>(cullMode), frontFace, lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetFrontFace(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t frontFace)
{
	prosper::PolygonMode polygonMode;
	prosper::CullModeFlags cullMode;
	float lineWidth;
	pipelineInfo.GetRasterizationProperties(&polygonMode, &cullMode, nullptr, &lineWidth);
	pipelineInfo.SetRasterizationProperties(polygonMode, cullMode, static_cast<prosper::FrontFace>(frontFace), lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetLineWidth(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float lineWidth)
{
	prosper::PolygonMode polygonMode;
	prosper::CullModeFlags cullMode;
	prosper::FrontFace frontFace;
	pipelineInfo.GetRasterizationProperties(&polygonMode, &cullMode, &frontFace, nullptr);
	pipelineInfo.SetRasterizationProperties(polygonMode, cullMode, frontFace, lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::SetScissorBoxProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iScissorBox, int32_t x, int32_t y, uint32_t w, uint32_t h) { pipelineInfo.SetScissorBoxProperties(iScissorBox, x, y, w, h); }
void Lua::GraphicsPipelineCreateInfo::SetStencilTestProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool updateFrontFaceState, uint32_t stencilFailOp, uint32_t stencilPassOp, uint32_t stencilDepthFailOp, uint32_t stencilCompareOp, uint32_t stencilCompareMask,
  uint32_t stencilWriteMask, uint32_t stencilReference)
{
	pipelineInfo.SetStencilTestProperties(updateFrontFaceState, static_cast<prosper::StencilOp>(stencilFailOp), static_cast<prosper::StencilOp>(stencilPassOp), static_cast<prosper::StencilOp>(stencilDepthFailOp), static_cast<prosper::CompareOp>(stencilCompareOp), stencilCompareMask,
	  stencilWriteMask, stencilReference);
}
void Lua::GraphicsPipelineCreateInfo::SetViewportProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iViewport, float originX, float originY, float w, float h, float minDepth, float maxDepth)
{
	pipelineInfo.SetViewportProperties(iViewport, originX, originY, w, h, minDepth, maxDepth);
}
void Lua::GraphicsPipelineCreateInfo::AreDepthWritesEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushBool(l, pipelineInfo.AreDepthWritesEnabled()); }
void Lua::GraphicsPipelineCreateInfo::GetBlendingProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	const float *blendingProperties;
	pipelineInfo.GetBlendingProperties(&blendingProperties, nullptr);
	Lua::Push<::Vector4>(l, ::Vector4 {blendingProperties[0], blendingProperties[1], blendingProperties[2], blendingProperties[3]});
}
void Lua::GraphicsPipelineCreateInfo::GetColorBlendAttachmentProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t attId)
{
	bool blendingEnabled;
	prosper::BlendOp blendOpColor, blendOpAlpha;
	prosper::BlendFactor srcColorBlendFactor, dstColorBlendFactor, srcAlphaBlendFactor, dstAlphaBlendFactor;
	prosper::ColorComponentFlags channelWriteMask;
	auto r = pipelineInfo.GetColorBlendAttachmentProperties(attId, &blendingEnabled, &blendOpColor, &blendOpAlpha, &srcColorBlendFactor, &dstColorBlendFactor, &srcAlphaBlendFactor, &dstAlphaBlendFactor, &channelWriteMask);
	if(r == false)
		return;
	PushBool(l, blendingEnabled);
	PushInt(l, static_cast<uint32_t>(blendOpColor));
	PushInt(l, static_cast<uint32_t>(blendOpAlpha));
	PushInt(l, static_cast<uint32_t>(srcColorBlendFactor));
	PushInt(l, static_cast<uint32_t>(dstColorBlendFactor));
	PushInt(l, static_cast<uint32_t>(srcAlphaBlendFactor));
	PushInt(l, static_cast<uint32_t>(dstAlphaBlendFactor));
	PushInt(l, static_cast<uint32_t>(channelWriteMask));
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasState(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	float depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor;
	pipelineInfo.GetDepthBiasState(&bEnabled, &depthBiasConstantFactor, &depthBiasClamp, &depthBiasSlopeFactor);
	PushBool(l, bEnabled);
	if(bEnabled == false)
		return;
	PushNumber(l, depthBiasConstantFactor);
	PushNumber(l, depthBiasClamp);
	PushNumber(l, depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasConstantFactor(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float depthBiasConstantFactor;
	pipelineInfo.GetDepthBiasState(nullptr, &depthBiasConstantFactor, nullptr, nullptr);
	PushNumber(l, depthBiasConstantFactor);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasClamp(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float depthBiasClamp;
	pipelineInfo.GetDepthBiasState(nullptr, nullptr, &depthBiasClamp, nullptr);
	PushNumber(l, depthBiasClamp);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBiasSlopeFactor(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float depthBiasSlopeFactor;
	pipelineInfo.GetDepthBiasState(nullptr, nullptr, nullptr, &depthBiasSlopeFactor);
	PushNumber(l, depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthBoundsState(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	float minDepthBounds, maxDepthBounds;
	pipelineInfo.GetDepthBoundsState(&bEnabled, &minDepthBounds, &maxDepthBounds);
	PushBool(l, bEnabled);
	if(bEnabled == false)
		return;
	PushNumber(l, minDepthBounds);
	PushNumber(l, maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetMinDepthBounds(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float minDepthBounds;
	pipelineInfo.GetDepthBoundsState(nullptr, &minDepthBounds, nullptr);
	PushNumber(l, minDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetMaxDepthBounds(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float maxDepthBounds;
	pipelineInfo.GetDepthBoundsState(nullptr, nullptr, &maxDepthBounds);
	PushNumber(l, maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthClamp(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float minDepthBounds, maxDepthBounds;
	pipelineInfo.GetDepthBoundsState(nullptr, &minDepthBounds, &maxDepthBounds);
	PushNumber(l, minDepthBounds);
	PushNumber(l, maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::GetDepthTestState(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	prosper::CompareOp compareOp;
	pipelineInfo.GetDepthTestState(&bEnabled, &compareOp);
	PushBool(l, bEnabled);
	if(bEnabled == false)
		return;
	PushInt(l, static_cast<uint32_t>(compareOp));
}
void Lua::GraphicsPipelineCreateInfo::GetDynamicStates(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushInt(l, prosper::util::get_enabled_dynamic_states(pipelineInfo)); }
void Lua::GraphicsPipelineCreateInfo::IsDynamicStateEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t dynamicState) { PushBool(l, prosper::util::are_dynamic_states_enabled(pipelineInfo, static_cast<prosper::util::DynamicStateFlags>(dynamicState))); }
void Lua::GraphicsPipelineCreateInfo::GetScissorCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	uint32_t scissorCount;
	pipelineInfo.GetGraphicsPipelineProperties(&scissorCount, nullptr, nullptr, nullptr, nullptr);
	PushInt(l, scissorCount);
}
void Lua::GraphicsPipelineCreateInfo::GetViewportCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	uint32_t viewportCount;
	pipelineInfo.GetGraphicsPipelineProperties(nullptr, &viewportCount, nullptr, nullptr, nullptr);
	PushInt(l, viewportCount);
}
void Lua::GraphicsPipelineCreateInfo::GetVertexAttributeCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	uint32_t vertexAttributeCount;
	pipelineInfo.GetGraphicsPipelineProperties(nullptr, nullptr, &vertexAttributeCount, nullptr, nullptr);
	PushInt(l, vertexAttributeCount);
}
void Lua::GraphicsPipelineCreateInfo::GetLogicOpState(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	prosper::LogicOp logicOp;
	pipelineInfo.GetLogicOpState(&bEnabled, &logicOp);
	PushBool(l, bEnabled);
	if(bEnabled == false)
		return;
	PushInt(l, static_cast<uint32_t>(logicOp));
}
void Lua::GraphicsPipelineCreateInfo::GetMultisamplingProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::SampleCountFlags samples;
	const prosper::SampleMask *sampleMask;
	pipelineInfo.GetMultisamplingProperties(&samples, &sampleMask);
	PushInt(l, pragma::math::to_integral(samples));
	PushInt(l, sampleMask ? static_cast<uint32_t>(*sampleMask) : 0u);
}
void Lua::GraphicsPipelineCreateInfo::GetSampleCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::SampleCountFlags samples;
	pipelineInfo.GetMultisamplingProperties(&samples, nullptr);
	PushInt(l, pragma::math::to_integral(samples));
}
void Lua::GraphicsPipelineCreateInfo::GetMinSampleShading(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	const prosper::SampleMask *sampleMask;
	pipelineInfo.GetMultisamplingProperties(nullptr, &sampleMask);
	PushInt(l, sampleMask ? static_cast<uint32_t>(*sampleMask) : 0u);
}
void Lua::GraphicsPipelineCreateInfo::GetSampleMask(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float minSampleShading;
	pipelineInfo.GetSampleShadingState(nullptr, &minSampleShading);
	PushNumber(l, minSampleShading);
}
void Lua::GraphicsPipelineCreateInfo::GetDynamicScissorBoxesCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushInt(l, pipelineInfo.GetDynamicScissorBoxesCount()); }
void Lua::GraphicsPipelineCreateInfo::GetDynamicViewportsCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushInt(l, pipelineInfo.GetDynamicViewportsCount()); }
void Lua::GraphicsPipelineCreateInfo::GetScissorBoxesCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushInt(l, pipelineInfo.GetScissorBoxesCount()); }
void Lua::GraphicsPipelineCreateInfo::GetViewportsCount(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushInt(l, pipelineInfo.GetViewportCount()); }
void Lua::GraphicsPipelineCreateInfo::GetPrimitiveTopology(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushInt(l, static_cast<uint32_t>(pipelineInfo.GetPrimitiveTopology())); }
void Lua::GraphicsPipelineCreateInfo::GetPushConstantRanges(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	auto &ranges = pipelineInfo.GetPushConstantRanges();
	auto t = CreateTable(l);
	auto idx = 1u;
	for(auto &range : ranges) {
		PushInt(l, idx++);
		auto tRange = CreateTable(l);

		PushString(l, "offset");
		PushInt(l, range.offset);
		SetTableValue(l, tRange);

		PushString(l, "size");
		PushInt(l, range.size);
		SetTableValue(l, tRange);

		PushString(l, "stages");
		PushInt(l, static_cast<uint32_t>(range.stages));
		SetTableValue(l, tRange);

		SetTableValue(l, t);
	}
}
void Lua::GraphicsPipelineCreateInfo::GetRasterizationProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::PolygonMode polygonMode;
	prosper::CullModeFlags cullMode;
	prosper::FrontFace frontFace;
	float lineWidth;
	pipelineInfo.GetRasterizationProperties(&polygonMode, &cullMode, &frontFace, &lineWidth);
	PushInt(l, static_cast<uint32_t>(polygonMode));
	PushInt(l, static_cast<uint32_t>(cullMode));
	PushInt(l, static_cast<uint32_t>(frontFace));
	PushNumber(l, lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::GetPolygonMode(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::PolygonMode polygonMode;
	pipelineInfo.GetRasterizationProperties(&polygonMode, nullptr, nullptr, nullptr);
	PushInt(l, static_cast<uint32_t>(polygonMode));
}
void Lua::GraphicsPipelineCreateInfo::GetCullMode(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::CullModeFlags cullMode;
	pipelineInfo.GetRasterizationProperties(nullptr, &cullMode, nullptr, nullptr);
	PushInt(l, static_cast<uint32_t>(cullMode));
}
void Lua::GraphicsPipelineCreateInfo::GetFrontFace(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	prosper::FrontFace frontFace;
	pipelineInfo.GetRasterizationProperties(nullptr, nullptr, &frontFace, nullptr);
	PushInt(l, static_cast<uint32_t>(frontFace));
}
void Lua::GraphicsPipelineCreateInfo::GetLineWidth(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	float lineWidth;
	pipelineInfo.GetRasterizationProperties(nullptr, nullptr, nullptr, &lineWidth);
	PushNumber(l, lineWidth);
}
void Lua::GraphicsPipelineCreateInfo::GetSampleShadingState(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	float minSampleShading;
	pipelineInfo.GetSampleShadingState(&bEnabled, &minSampleShading);
	PushBool(l, bEnabled);
	if(bEnabled == false)
		return;
	PushNumber(l, minSampleShading);
}
void Lua::GraphicsPipelineCreateInfo::GetScissorBoxProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iScissor)
{
	int32_t x, y;
	uint32_t w, h;
	auto r = pipelineInfo.GetScissorBoxProperties(iScissor, &x, &y, &w, &h);
	if(r == false)
		return;
	PushInt(l, x);
	PushInt(l, y);
	PushInt(l, w);
	PushInt(l, h);
}
void Lua::GraphicsPipelineCreateInfo::GetStencilTestProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo)
{
	bool bEnabled;
	prosper::StencilOp frontStencilFairOp, frontStencilPassOp, frontStencilDepthFailOp;
	prosper::CompareOp frontStencilCompareOp, backStencilCompareOp;
	uint32_t frontStencilCompareMask, frontStencilWriteMask, frontStencilReference, backStencilCompareMask, backStencilWriteMask, backStencilReference;
	prosper::StencilOp backStencilFailOp, backStencilPassOp, backStencilDepthFailOp;

	pipelineInfo.GetStencilTestProperties(&bEnabled, &frontStencilFairOp, &frontStencilPassOp, &frontStencilDepthFailOp, &frontStencilCompareOp, &frontStencilCompareMask, &frontStencilWriteMask, &frontStencilReference, &backStencilFailOp, &backStencilPassOp, &backStencilDepthFailOp,
	  &backStencilCompareOp, &backStencilCompareMask, &backStencilWriteMask, &backStencilReference);
	PushBool(l, bEnabled);
	if(bEnabled == false)
		return;
	PushInt(l, static_cast<uint32_t>(frontStencilFairOp));
	PushInt(l, static_cast<uint32_t>(frontStencilPassOp));
	PushInt(l, static_cast<uint32_t>(frontStencilDepthFailOp));
	PushInt(l, static_cast<uint32_t>(frontStencilCompareOp));
	PushInt(l, static_cast<uint32_t>(frontStencilCompareMask));
	PushInt(l, static_cast<uint32_t>(frontStencilWriteMask));
	PushInt(l, static_cast<uint32_t>(frontStencilReference));
	PushInt(l, static_cast<uint32_t>(backStencilFailOp));
	PushInt(l, static_cast<uint32_t>(backStencilPassOp));
	PushInt(l, static_cast<uint32_t>(backStencilDepthFailOp));
	PushInt(l, static_cast<uint32_t>(backStencilCompareOp));
	PushInt(l, static_cast<uint32_t>(backStencilCompareMask));
	PushInt(l, static_cast<uint32_t>(backStencilWriteMask));
	PushInt(l, static_cast<uint32_t>(backStencilReference));
}
void Lua::GraphicsPipelineCreateInfo::GetSubpassId(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushInt(l, pipelineInfo.GetSubpassId()); }
void Lua::GraphicsPipelineCreateInfo::GetVertexAttributeProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iVerexInputAttribute)
{
	uint32_t location, offset, explicitVertexBindingIndex, stride;
	prosper::Format format;
	prosper::VertexInputRate rate;
	auto numAttributes = 0u;
	auto r = pipelineInfo.GetVertexBindingProperties(iVerexInputAttribute, &explicitVertexBindingIndex, &stride, &rate, &numAttributes);
	if(r == false)
		return;
	std::vector<const prosper::VertexInputAttribute *> attributes(numAttributes);
	r = pipelineInfo.GetVertexBindingProperties(iVerexInputAttribute, nullptr, nullptr, nullptr, nullptr, attributes.data());
	if(r == false || attributes.empty())
		return;
	// TODO: Push all attributes as table?
	location = attributes.front()->location;
	format = attributes.front()->format;
	PushInt(l, location);
	PushInt(l, static_cast<uint32_t>(format));
	PushInt(l, stride);
	PushInt(l, static_cast<uint32_t>(rate));
}
void Lua::GraphicsPipelineCreateInfo::GetViewportProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t iViewport)
{
	float originX, originY, w, h, minDepth, maxDepth;
	auto r = pipelineInfo.GetViewportProperties(iViewport, &originX, &originY, &w, &h, &minDepth, &maxDepth);
	if(r == false)
		return;
	PushNumber(l, originX);
	PushNumber(l, originY);
	PushNumber(l, w);
	PushNumber(l, h);
	PushNumber(l, minDepth);
	PushNumber(l, maxDepth);
}
void Lua::GraphicsPipelineCreateInfo::IsAlphaToCoverageEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushBool(l, pipelineInfo.IsAlphaToCoverageEnabled()); }
void Lua::GraphicsPipelineCreateInfo::IsAlphaToOneEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushBool(l, pipelineInfo.IsAlphaToOneEnabled()); }
void Lua::GraphicsPipelineCreateInfo::IsDepthClampEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushBool(l, pipelineInfo.IsDepthClampEnabled()); }
void Lua::GraphicsPipelineCreateInfo::IsPrimitiveRestartEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushBool(l, pipelineInfo.IsPrimitiveRestartEnabled()); }
void Lua::GraphicsPipelineCreateInfo::IsRasterizerDiscardEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushBool(l, pipelineInfo.IsRasterizerDiscardEnabled()); }
void Lua::GraphicsPipelineCreateInfo::IsSampleMaskEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo) { PushBool(l, pipelineInfo.IsSampleMaskEnabled()); }
void Lua::Shader::Graphics::AttachVertexAttribute(lua::State *l, pragma::LuaShaderWrapperGraphicsBase &shader, const pragma::LuaVertexBinding &binding, luabind::object attributes)
{
	auto vertexAttributes = Lua::get_table_values<pragma::LuaVertexAttribute>(l, 3u, [](lua::State *l, int32_t idx) { return Lua::Check<pragma::LuaVertexAttribute>(l, idx); });
	shader.AttachVertexAttribute(binding, vertexAttributes);
}
void Lua::GraphicsPipelineCreateInfo::AddSpecializationConstant(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t shaderStage, uint32_t constantId, pragma::util::DataStream &ds)
{
	PushBool(l, pipelineInfo.AddSpecializationConstant(static_cast<prosper::ShaderStage>(shaderStage), constantId, ds->GetSize(), ds->GetData()));
}
void Lua::GraphicsPipelineCreateInfo::SetAlphaToCoverageEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled) { pipelineInfo.ToggleAlphaToCoverage(bEnabled); }
void Lua::GraphicsPipelineCreateInfo::SetAlphaToOneEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled) { pipelineInfo.ToggleAlphaToOne(bEnabled); }
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
	pipelineInfo.ToggleDepthBias(bEnabled, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasConstantFactor(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float depthBiasConstantFactor)
{
	bool bEnabled;
	float depthBiasClamp, depthBiasSlopeFactor;
	pipelineInfo.GetDepthBiasState(&bEnabled, nullptr, &depthBiasClamp, &depthBiasSlopeFactor);
	pipelineInfo.ToggleDepthBias(bEnabled, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasClamp(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float depthBiasClamp)
{
	bool bEnabled;
	float depthBiasConstantFactor, depthBiasSlopeFactor;
	pipelineInfo.GetDepthBiasState(&bEnabled, &depthBiasConstantFactor, nullptr, &depthBiasSlopeFactor);
	pipelineInfo.ToggleDepthBias(bEnabled, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasSlopeFactor(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float depthBiasSlopeFactor)
{
	bool bEnabled;
	float depthBiasConstantFactor, depthBiasClamp;
	pipelineInfo.GetDepthBiasState(&bEnabled, &depthBiasConstantFactor, &depthBiasClamp, nullptr);
	pipelineInfo.ToggleDepthBias(bEnabled, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBiasEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled)
{
	float depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor;
	pipelineInfo.GetDepthBiasState(nullptr, &depthBiasConstantFactor, &depthBiasClamp, &depthBiasSlopeFactor);
	pipelineInfo.ToggleDepthBias(bEnabled, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBoundsTestProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, float minDepthBounds, float maxDepthBounds) { pipelineInfo.ToggleDepthBoundsTest(bEnabled, minDepthBounds, maxDepthBounds); }
void Lua::GraphicsPipelineCreateInfo::SetMinDepthBounds(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float minDepthBounds)
{
	bool bEnabled;
	float maxDepthBounds;
	pipelineInfo.GetDepthBoundsState(&bEnabled, nullptr, &maxDepthBounds);
	pipelineInfo.ToggleDepthBoundsTest(bEnabled, minDepthBounds, maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetMaxDepthBounds(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, float maxDepthBounds)
{
	bool bEnabled;
	float minDepthBounds;
	pipelineInfo.GetDepthBoundsState(&bEnabled, &minDepthBounds, nullptr);
	pipelineInfo.ToggleDepthBoundsTest(bEnabled, minDepthBounds, maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthBoundsTestEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled)
{
	float minDepthBounds, maxDepthBounds;
	pipelineInfo.GetDepthBoundsState(nullptr, &minDepthBounds, &maxDepthBounds);
	pipelineInfo.ToggleDepthBoundsTest(bEnabled, minDepthBounds, maxDepthBounds);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthClampEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled) { pipelineInfo.ToggleDepthClamp(bEnabled); }
void Lua::GraphicsPipelineCreateInfo::SetDepthTestProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, uint32_t compareOp) { pipelineInfo.ToggleDepthTest(bEnabled, static_cast<prosper::CompareOp>(compareOp)); }
void Lua::GraphicsPipelineCreateInfo::SetDepthTestEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled)
{
	// prosper::CompareOp compareOp;
	// pipelineInfo.GetDepthTestState(nullptr,&compareOp);
	pipelineInfo.ToggleDepthTest(bEnabled, prosper::CompareOp::LessOrEqual);
}
void Lua::GraphicsPipelineCreateInfo::SetDepthWritesEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled) { pipelineInfo.ToggleDepthWrites(bEnabled); }
void Lua::GraphicsPipelineCreateInfo::SetDynamicStates(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t states)
{
	prosper::util::set_dynamic_states_enabled(pipelineInfo, prosper::util::get_enabled_dynamic_states(pipelineInfo), false);
	prosper::util::set_dynamic_states_enabled(pipelineInfo, static_cast<prosper::util::DynamicStateFlags>(states), true);
}
void Lua::GraphicsPipelineCreateInfo::SetDynamicStateEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t state, bool bEnabled) { prosper::util::set_dynamic_states_enabled(pipelineInfo, static_cast<prosper::util::DynamicStateFlags>(state), bEnabled); }
void Lua::GraphicsPipelineCreateInfo::SetLogicOpProperties(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled, uint32_t logicOp) { pipelineInfo.ToggleLogicOp(bEnabled, static_cast<prosper::LogicOp>(logicOp)); }
void Lua::GraphicsPipelineCreateInfo::SetLogicOpEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled)
{
	prosper::LogicOp logicOp;
	pipelineInfo.GetLogicOpState(nullptr, &logicOp);
	pipelineInfo.ToggleLogicOp(bEnabled, static_cast<prosper::LogicOp>(logicOp));
}
void Lua::GraphicsPipelineCreateInfo::SetPrimitiveRestartEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled) { pipelineInfo.TogglePrimitiveRestart(bEnabled); }
void Lua::GraphicsPipelineCreateInfo::SetRasterizerDiscardEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled) { pipelineInfo.ToggleRasterizerDiscard(bEnabled); }
void Lua::GraphicsPipelineCreateInfo::SetSampleMaskEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled) { pipelineInfo.ToggleSampleMask(bEnabled); }
void Lua::GraphicsPipelineCreateInfo::SetSampleShadingEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled) { pipelineInfo.ToggleSampleShading(bEnabled); }
void Lua::GraphicsPipelineCreateInfo::SetStencilTestEnabled(lua::State *l, prosper::GraphicsPipelineCreateInfo &pipelineInfo, bool bEnabled) { pipelineInfo.ToggleStencilTest(bEnabled); }

void Lua::Shader::Graphics::RecordBindVertexBuffer(lua::State *l, prosper::ShaderGraphics &shader, prosper::ShaderBindState &bindState, Vulkan::Buffer &buffer, uint32_t startBinding, uint32_t offset) { PushBool(l, shader.RecordBindVertexBuffer(bindState, buffer, startBinding, offset)); }
void Lua::Shader::Graphics::RecordBindVertexBuffers(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, luabind::object buffers, uint32_t startBinding, luabind::object offsets)
{
	auto vBuffers = Lua::get_table_values<prosper::IBuffer *>(l, 3, [](lua::State *l, int32_t idx) { return &Lua::Check<Vulkan::Buffer>(l, idx); });
	std::vector<uint64_t> vOffsets;
	if(IsSet(l, 4)) {
		vOffsets = Lua::get_table_values<uint64_t>(l, 5, [](lua::State *l, int32_t idx) { return static_cast<ptrdiff_t>(CheckInt(l, idx)); });
	}
	auto *bindState = recordTarget.GetBindState();
	if(bindState)
		return PushBool(l, shader.RecordBindVertexBuffers(*bindState, vBuffers, startBinding, vOffsets));

	recordTarget.GetPcb()->PushCommand(
	  [&shader, vBuffers = std::move(vBuffers), vOffsets = std::move(vOffsets), startBinding](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool { return shader.RecordBindVertexBuffers(*recordState.shaderBindState, vBuffers, startBinding, vOffsets); });
}
void Lua::Shader::Graphics::RecordBindIndexBuffer(lua::State *l, prosper::ShaderGraphics &shader, prosper::ShaderBindState &bindState, Vulkan::Buffer &indexBuffer, uint32_t indexType, uint32_t offset)
{
	PushBool(l, shader.RecordBindIndexBuffer(bindState, indexBuffer, static_cast<prosper::IndexType>(indexType), offset));
}
void Lua::Shader::Graphics::RecordDraw(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, uint32_t vertCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	auto *bindState = recordTarget.GetBindState();
	if(bindState)
		return PushBool(l, shader.RecordDraw(*bindState, vertCount, instanceCount, firstVertex, firstInstance));
	recordTarget.GetPcb()->PushCommand([=, &shader](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool { return shader.RecordDraw(*recordState.shaderBindState, vertCount, instanceCount, firstVertex, firstInstance); });
}
void Lua::Shader::Graphics::RecordDrawIndexed(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance)
{
	auto *bindState = recordTarget.GetBindState();
	if(bindState)
		return PushBool(l, shader.RecordDrawIndexed(*bindState, indexCount, instanceCount, firstIndex, firstInstance));
	recordTarget.GetPcb()->PushCommand([=, &shader](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool { return shader.RecordDrawIndexed(*recordState.shaderBindState, indexCount, instanceCount, firstIndex, firstInstance); });
}
void Lua::Shader::Graphics::RecordBeginDraw(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget, uint32_t pipelineIdx)
{
	auto *bindState = recordTarget.GetBindState();
	if(bindState)
		PushBool(l, shader.RecordBeginDraw(*bindState, pipelineIdx));
	else {
		auto hShader = shader.GetHandle();
		recordTarget.GetPcb()->PushCommand([hShader, pipelineIdx](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
			if(hShader.expired()) // We only have to check the validity in BeginDraw!
				return false;
			recordState.shaderBindState = std::make_unique<prosper::ShaderBindState>(recordState.commandBuffer);
			return static_cast<prosper::ShaderGraphics &>(*hShader.get()).RecordBeginDraw(*recordState.shaderBindState, pipelineIdx);
		});
	}
}
void Lua::Shader::Graphics::RecordDraw(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget)
{
	auto *bindState = recordTarget.GetBindState();
	if(bindState)
		return PushBool(l, shader.RecordDraw(*bindState));
	recordTarget.GetPcb()->PushCommand([=, &shader](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool { return shader.RecordDraw(*recordState.shaderBindState); });
}
void Lua::Shader::Graphics::RecordEndDraw(lua::State *l, prosper::ShaderGraphics &shader, const LuaShaderRecordTarget &recordTarget)
{
	auto *bindState = recordTarget.GetBindState();
	if(bindState) {
		shader.RecordEndDraw(*bindState);
		PushBool(l, true);
		return;
	}
	recordTarget.GetPcb()->PushCommand([&shader](const prosper::util::PreparedCommandBufferRecordState &recordState) -> bool {
		shader.RecordEndDraw(*recordState.shaderBindState);
		return true;
	});
}
void Lua::Shader::Graphics::GetRenderPass(lua::State *l, prosper::ShaderGraphics &shader, uint32_t pipelineIdx)
{
	auto &rp = shader.GetRenderPass(pipelineIdx);
	if(rp == nullptr)
		return;
	Push(l, rp);
}
void Lua::Shader::Scene3D::GetRenderPass(lua::State *l, uint32_t pipelineIdx)
{
	auto &rp = prosper::ShaderGraphics::GetRenderPass<pragma::ShaderScene>(pragma::get_cengine()->GetRenderContext(), pipelineIdx);
	if(rp == nullptr)
		return;
	Push(l, rp);
}
