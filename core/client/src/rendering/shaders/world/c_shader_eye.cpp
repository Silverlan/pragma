/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_eye.hpp"
#include "pragma/rendering/render_processor.hpp"
#include "pragma/entities/components/c_eye_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

ShaderEye::ShaderEye(prosper::IPrContext &context,const std::string &identifier)
	: ShaderPBR{context,identifier,"world/eye/vs_eye","world/eye/fs_eye"}
{}
bool ShaderEye::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount)
{
	return false;//BindEyeball(mesh.GetSkinTextureIndex()) && ShaderPBR::Draw(mesh,meshIdx,renderBufferIndexBuffer,instanceCount);
}
bool ShaderEye::Draw(pragma::rendering::ShaderProcessor &shaderProcessor)
{
	return false;
}
bool ShaderEye::BindEyeball(rendering::ShaderProcessor &shaderProcessor,uint32_t skinMatIdx) const
{
	auto &ent = shaderProcessor.GetCurrentEntity();
	auto eyeC = ent.GetComponent<CEyeComponent>();
	uint32_t eyeballIndex;
	if(eyeC.expired() || eyeC->FindEyeballIndex(skinMatIdx,eyeballIndex) == false)
		return false;
	auto mdl = ent.GetModel();
	auto *eyeballData = eyeC->GetEyeballData(eyeballIndex);
	auto *eyeball = mdl->GetEyeball(eyeballIndex);
	if(eyeballData == nullptr || eyeball == nullptr)
		return false;
	auto &eyeballState = eyeballData->state;
	auto eyeOrigin = eyeC->CalcEyeballPose(eyeballIndex).GetOrigin();
	PushConstants pushConstants {};
	eyeC->GetEyeballProjectionVectors(eyeballIndex,pushConstants.irisProjectionU,pushConstants.irisProjectionV);
	pushConstants.dilationFactor = eyeballData->config.dilation;
	pushConstants.maxDilationFactor = eyeball->maxDilationFactor;
	pushConstants.irisUvRadius = eyeball->irisUvRadius;
	pushConstants.eyeOrigin.x = eyeOrigin.x;
	pushConstants.eyeOrigin.y = eyeOrigin.y;
	pushConstants.eyeOrigin.z = eyeOrigin.z;
	shaderProcessor.GetCommandBuffer().RecordPushConstants(
		shaderProcessor.GetCurrentPipelineLayout(),prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit,sizeof(ShaderPBR::PushConstants),sizeof(pushConstants),&pushConstants
	);
	return true;
}
void ShaderEye::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderPBR::PushConstants) +sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}

//

void ShaderEye::RecordBindScene(
	rendering::ShaderProcessor &shaderProcessor,
	const pragma::CSceneComponent &scene,const pragma::CRasterizationRendererComponent &renderer,
	prosper::IDescriptorSet &dsScene,prosper::IDescriptorSet &dsRenderer,
	prosper::IDescriptorSet &dsRenderSettings,prosper::IDescriptorSet &dsLights,
	prosper::IDescriptorSet &dsShadows,prosper::IDescriptorSet &dsMaterial,
	ShaderGameWorld::SceneFlags &inOutSceneFlags
) const
{
	PushSceneConstants(shaderProcessor,scene);
	auto iblStrength = 1.f;
	RecordBindSceneDescriptorSets(
		shaderProcessor,scene,renderer,
		dsScene,dsRenderer,dsRenderSettings,
		dsLights,dsShadows,dsMaterial,inOutSceneFlags,iblStrength
	);
}

bool ShaderEye::OnRecordDrawMesh(rendering::ShaderProcessor &shaderProcessor,CModelSubMesh &mesh) const
{
	return BindEyeball(shaderProcessor,mesh.GetSkinTextureIndex());
}
