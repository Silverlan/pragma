// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_test;

import :client_state;
import :engine;
import :entities.components;
import :model;

using namespace pragma;

static_assert(math::to_integral(ShaderTest::MaterialBinding::Count) == 9, "Number of bindings in material descriptor set does not match MaterialBinding enum count!");

decltype(ShaderTest::DESCRIPTOR_SET_PBR) ShaderTest::DESCRIPTOR_SET_PBR = {
  "PBR",
  {prosper::DescriptorSetInfo::Binding {"IRRADIANCE_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
    prosper::DescriptorSetInfo::Binding {"PREFILTER_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit, prosper::PrDescriptorSetBindingFlags::Cubemap},
    prosper::DescriptorSetInfo::Binding {"BRDF_MAP", prosper::DescriptorType::CombinedImageSampler, prosper::ShaderStageFlags::FragmentBit}},
};
ShaderTest::ShaderTest(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderGameWorldLightingPass {context, identifier, vsShader, fsShader, gsShader}
{
	//SetPipelineCount(pragma::math::to_integral(Pipeline::Count));
}
ShaderTest::ShaderTest(prosper::IPrContext &context, const std::string &identifier) : ShaderTest {context, identifier, "world/vs_test", "world/fs_test"} {}
void ShaderTest::DrawTest(prosper::IBuffer &buf, prosper::IBuffer &ibuf, uint32_t count)
{
	/*auto x = RecordBindIndexBuffer(ibuf) && RecordBindVertexBuffer(buf) &&
		ShaderScene::RecordPushConstants(sizeof(Mat4),&m_testMvp) &&
		RecordDrawIndexed(count);*/
}
void ShaderTest::SetForceNonIBLMode(bool b) { m_bNonIBLMode = b; }
#if 0
bool ShaderTest::BeginDraw(
	const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,
	const Vector4 &drawOrigin,RecordFlags recordFlags
)
{
	return ShaderGraphics::BeginDraw(cmdBuffer) == true &&
		cmdBuffer->RecordSetDepthBias(1.f,0.f,0.f);
}
#endif
void ShaderTest::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(Mat4), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }
void ShaderTest::UpdateRenderFlags(geometry::CModelSubMesh &mesh, SceneFlags &inOutFlags)
{
	ShaderGameWorldLightingPass::UpdateRenderFlags(mesh, inOutFlags);
	inOutFlags |= m_extRenderFlags;
}
void ShaderTest::InitializeGfxPipelineDescriptorSets()
{
	//ShaderGameWorldLightingPass::InitializeGfxPipelineDescriptorSets(pipelineInfo,pipelineIdx);
	//AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_PBR);
}
void ShaderTest::InitializeShaderResources()
{
	ShaderScene::InitializeShaderResources();

	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT);

	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	//AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	/*AddVertexAttribute(VERTEX_ATTRIBUTE_UV);
	AddVertexAttribute(VERTEX_ATTRIBUTE_NORMAL);
	AddVertexAttribute(VERTEX_ATTRIBUTE_TANGENT);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BI_TANGENT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_LIGHTMAP_UV);*/
	InitializeGfxPipelinePushConstantRanges();

	//InitializeGfxPipelineDescriptorSets(pipelineIdx);

	/*ShaderScene::InitializeGfxPipeline();

	VERTEX_BINDING_VERTEX.stride = std::numeric_limits<decltype(VERTEX_BINDING_VERTEX.stride)>::max();

	//AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(VERTEX_ATTRIBUTE_UV);
	AddVertexAttribute(VERTEX_ATTRIBUTE_NORMAL);
	AddVertexAttribute(VERTEX_ATTRIBUTE_TANGENT);
	AddVertexAttribute(VERTEX_ATTRIBUTE_BI_TANGENT);

	AddVertexAttribute(VERTEX_ATTRIBUTE_LIGHTMAP_UV);


	AttachPushConstantRange(0u,sizeof(Mat4),prosper::ShaderStageFlags::VertexBit);*/
}
void ShaderTest::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo, uint32_t pipelineIdx)
{
	ShaderScene::InitializeGfxPipeline(pipelineInfo, pipelineIdx);
	//ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	//ShaderEntity::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	//ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	//if(pipelineIdx == pragma::math::to_integral(ShaderGameWorldPipeline::Reflection))
	//	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::FrontBit);

	//pipelineInfo.ToggleDepthWrites(false);
	//pipelineInfo.ToggleDepthTest(true,prosper::CompareOp::LessOrEqual);

	//pipelineInfo.ToggleDepthBias(true,0.f,0.f,0.f);
	//pipelineInfo.ToggleDynamicState(true,prosper::DynamicState::DepthBias); // Required for decals

	//SetGenericAlphaColorBlendAttachmentProperties(pipelineInfo);

	prosper::util::set_generic_alpha_color_blend_attachment_properties(pipelineInfo);
	pipelineInfo.ToggleDepthBias(true, 1.f, 0.f, 0.f);
	pipelineInfo.ToggleDynamicStates(true, {prosper::DynamicState::DepthBias});
}

static bool bind_texture(material::Material &mat, prosper::IDescriptorSet &ds, TextureInfo *texInfo, uint32_t bindingIndex, material::Texture *optDefaultTex = nullptr)
{
	auto &matManager = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

	std::shared_ptr<material::Texture> tex = nullptr;
	if(texInfo && texInfo->texture)
		tex = std::static_pointer_cast<material::Texture>(texInfo->texture);
	else if(optDefaultTex == nullptr)
		return false;
	else
		tex = optDefaultTex->shared_from_this();
	if(tex && tex->HasValidVkTexture())
		ds.SetBindingTexture(*tex->GetVkTexture(), bindingIndex);
	return true;
}

static bool bind_default_texture(prosper::IDescriptorSet &ds, const std::string &defaultTexName, uint32_t bindingIndex)
{
	auto &matManager = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();
	auto ptrTex = texManager.LoadAsset(defaultTexName);
	if(ptrTex == nullptr)
		return false;
	if(ptrTex && ptrTex->HasValidVkTexture())
		ds.SetBindingTexture(*ptrTex->GetVkTexture(), bindingIndex);
	return true;
}

static bool bind_texture(material::Material &mat, prosper::IDescriptorSet &ds, TextureInfo *texInfo, uint32_t bindingIndex, const std::string &defaultTexName)
{
	auto &matManager = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager());
	auto &texManager = matManager.GetTextureManager();

	std::shared_ptr<material::Texture> tex = nullptr;
	if(texInfo && texInfo->texture) {
		tex = std::static_pointer_cast<material::Texture>(texInfo->texture);
		if(tex->HasValidVkTexture()) {
			ds.SetBindingTexture(*tex->GetVkTexture(), bindingIndex);
			return true;
		}
	}
	else if(defaultTexName.empty())
		return false;
	return bind_default_texture(ds, defaultTexName, bindingIndex);
}
