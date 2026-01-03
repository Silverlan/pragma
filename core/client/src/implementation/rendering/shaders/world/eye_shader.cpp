// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.shaders.world_eye;

import :client_state;
import :engine;
import :entities.components;

using namespace pragma;

ShaderEye::ShaderEye(prosper::IPrContext &context, const std::string &identifier, const std::string &vsShader, const std::string &fsShader, const std::string &gsShader) : ShaderPBR {context, identifier, vsShader, fsShader, gsShader} {}
ShaderEye::ShaderEye(prosper::IPrContext &context, const std::string &identifier) : ShaderEye {context, identifier, "programs/scene/eye/eye", "programs/scene/eye/eye"} {}
bool ShaderEye::BindEyeball(rendering::ShaderProcessor &shaderProcessor, uint32_t skinMatIdx) const
{
	auto &ent = shaderProcessor.GetCurrentEntity();
	auto eyeC = ent.GetComponent<CEyeComponent>();
	uint32_t eyeballIndex;
	if(eyeC.expired() || eyeC->FindEyeballIndex(skinMatIdx, eyeballIndex) == false)
		return false;
	auto mdl = ent.GetModel();
	auto *eyeballData = eyeC->GetEyeballData(eyeballIndex);
	auto *eyeball = mdl->GetEyeball(eyeballIndex);
	if(eyeballData == nullptr || eyeball == nullptr)
		return false;
	auto &eyeballState = eyeballData->state;
	auto eyeOrigin = eyeC->CalcEyeballPose(eyeballIndex).GetOrigin();
	PushConstants pushConstants {};
	eyeC->GetEyeballProjectionVectors(eyeballIndex, pushConstants.irisProjectionU, pushConstants.irisProjectionV);
	pushConstants.dilationFactor = eyeballData->config.dilation;
	pushConstants.maxDilationFactor = eyeball->maxDilationFactor;
	pushConstants.irisUvRadius = eyeball->irisUvRadius;
	pushConstants.eyeOrigin.x = eyeOrigin.x;
	pushConstants.eyeOrigin.y = eyeOrigin.y;
	pushConstants.eyeOrigin.z = eyeOrigin.z;
	if(IsLegacyShader()) {
		auto &irisUvClampRange = eyeballData->config.irisUvClampRange;
		memcpy(&pushConstants.irisUvClampRange, &irisUvClampRange, sizeof(irisUvClampRange));
	}
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, sizeof(ShaderPBR::PushConstants), sizeof(pushConstants), &pushConstants);
	return true;
}
void ShaderEye::InitializeGfxPipelinePushConstantRanges() { AttachPushConstantRange(0u, sizeof(ShaderPBR::PushConstants) + sizeof(PushConstants), prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit); }

void ShaderEye::RecordBindScene(rendering::ShaderProcessor &shaderProcessor, const CSceneComponent &scene, const CRasterizationRendererComponent &renderer, prosper::IDescriptorSet &dsScene, prosper::IDescriptorSet &dsRenderer, prosper::IDescriptorSet &dsRenderSettings,
  prosper::IDescriptorSet &dsShadows, const Vector4 &drawOrigin, SceneFlags &inOutSceneFlags) const
{
	RecordPushSceneConstants(shaderProcessor, scene, drawOrigin);

	// I'm not sure why we have to push these at this point in time (since the actual constants we want are pushed in :BindEyeball),
	// but for some reason if we don't do it here, it can cause the Engine to permanently freeze on AMD GPUs
	static constexpr PushConstants p {};
	shaderProcessor.GetCommandBuffer().RecordPushConstants(shaderProcessor.GetCurrentPipelineLayout(), prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit, sizeof(ShaderPBR::PushConstants), sizeof(p), &p);
	//

	auto iblStrength = 1.f;
	RecordBindSceneDescriptorSets(shaderProcessor, scene, renderer, dsScene, dsRenderer, dsRenderSettings, dsShadows, inOutSceneFlags, iblStrength);
}

bool ShaderEye::OnRecordDrawMesh(rendering::ShaderProcessor &shaderProcessor, geometry::CModelSubMesh &mesh) const { return BindEyeball(shaderProcessor, mesh.GetSkinTextureIndex()); }

///////////////////////

ShaderEyeLegacy::ShaderEyeLegacy(prosper::IPrContext &context, const std::string &identifier) : ShaderEye {context, identifier, "programs/scene/eye/eye", "programs/scene/eye/eye_legacy"} { m_shaderMaterialName = "eye_legacy"; }
