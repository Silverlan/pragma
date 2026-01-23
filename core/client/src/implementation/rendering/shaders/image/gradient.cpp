// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.gradient;
import :engine;

using namespace pragma;

static bool get_line_line_intersection(const Vector2 &p0, const Vector2 &p1, const Vector2 &p3, Vector2 &intersection)
{
	Vector2 p2(0.f, 0.f);
	Vector2 s1 = p1 - p0;
	Vector2 s2 = p3 - p2;

	float s = (-s1.y * (p0.x - p2.x) + s1.x * (p0.y - p2.y)) / (-s2.x * s1.y + s1.x * s2.y);
	float t = (s2.x * (p0.y - p2.y) - s2.y * (p0.x - p2.x)) / (-s2.x * s1.y + s1.x * s2.y);
	if(s >= 0.f && s <= 1.f && t >= 0.f && t <= 1.f) {
		intersection = Vector2(p0.x + (t * s1.x), p0.y + (t * s1.y));
		return true;
	}
	return false;
}

static ShaderGradient *s_shaderGradient = nullptr;
ShaderGradient::ShaderGradient(prosper::IPrContext &context, const std::string &identifier) : ShaderBaseImageProcessing(context, identifier, "programs/effects/gradient")
{
	s_shaderGradient = this;
	SetBaseShader<prosper::ShaderCopyImage>();
}

ShaderGradient::~ShaderGradient() { s_shaderGradient = nullptr; }

void ShaderGradient::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx) { ShaderGraphics::InitializeGfxPipeline(pipelineInfo, pipelineIdx); }

void ShaderGradient::InitializeShaderResources()
{
	ShaderGraphics::InitializeShaderResources();

	AddDefaultVertexAttributes();
	AttachPushConstantRange(0u, sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit);
}

bool ShaderGradient::RecordDraw(prosper::ShaderBindState &bindState, const PushConstants &pushConstants) const { return RecordPushConstants(bindState, pushConstants) && ShaderBaseImageProcessing::RecordDraw(bindState); }

/////////////////////////

bool pragma::record_draw_gradient(prosper::IPrContext &context, const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer, prosper::RenderTarget &rt, const Vector2 &dir, const std::vector<ShaderGradient::Node> &nodes)
{
	if(s_shaderGradient == nullptr)
		return false;
	auto ndir = glm::normalize(dir);
	Vector2 pFar = ndir * (glm::length(Vector2(1.f, 1.f)) + 0.001f);
	Vector2 pointBox = {};
	auto &tex = rt.GetTexture();
	auto &img = tex.GetImage();
	if(cmdBuffer->RecordBeginRenderPass(rt) == false)
		return false;
	if(get_line_line_intersection(Vector2(-1, -1), Vector2(1, -1), pFar, pointBox) == false && get_line_line_intersection(Vector2(-1, 1), Vector2(1, 1), pFar, pointBox) == false && get_line_line_intersection(Vector2(-1, -1), Vector2(-1, 1), pFar, pointBox) == false
	  && get_line_line_intersection(Vector2(1, 1), Vector2(1, -1), pFar, pointBox) == false) {
		auto result = cmdBuffer->RecordClearAttachment(img, {0.f, 0.f, 0.f, 1.f});
		return cmdBuffer->RecordEndRenderPass();
	}
	auto extents = img.GetExtents();
	auto &shader = static_cast<ShaderGradient &>(*s_shaderGradient);
	prosper::ShaderBindState bindState {*cmdBuffer};
	if(shader.RecordBeginDraw(bindState) == true) {
		auto nodeCount = nodes.size();
		if(nodeCount > ShaderGradient::MAX_GRADIENT_NODES)
			nodeCount = ShaderGradient::MAX_GRADIENT_NODES;

		ShaderGradient::PushConstants pushConstants {Vector2i(extents.width, extents.height), pointBox, nodeCount, {}};
		for(auto i = decltype(nodeCount) {0}; i < nodeCount; ++i)
			pushConstants.nodes.at(i) = nodes.at(i);
		shader.RecordDraw(bindState, pushConstants);

		shader.RecordEndDraw(bindState);
	}
	return cmdBuffer->RecordEndRenderPass();
}
