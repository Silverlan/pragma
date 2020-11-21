/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_eye.hpp"
#include "pragma/entities/components/c_eye_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <shader/prosper_pipeline_create_info.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;


ShaderEye::ShaderEye(prosper::IPrContext &context,const std::string &identifier)
	: ShaderPBR{context,identifier,"world/eye/vs_eye","world/eye/fs_eye"}
{}
bool ShaderEye::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx)
{
	return BindEyeball(mesh.GetSkinTextureIndex()) && ShaderPBR::Draw(mesh,meshIdx);
}
bool ShaderEye::BindEyeball(uint32_t skinMatIdx)
{
	auto *ent = GetBoundEntity();
	if(ent == nullptr)
		return false;
	auto eyeC = ent->GetComponent<CEyeComponent>();
	uint32_t eyeballIndex;
	if(eyeC.expired() || eyeC->FindEyeballIndex(skinMatIdx,eyeballIndex) == false)
		return false;
	auto mdl = ent->GetModel();
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
	return RecordPushConstants(pushConstants,sizeof(ShaderPBR::PushConstants));
}
void ShaderEye::InitializeGfxPipelinePushConstantRanges(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderPBR::PushConstants) +sizeof(PushConstants),prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit);
}

