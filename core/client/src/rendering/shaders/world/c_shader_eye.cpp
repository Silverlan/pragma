#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_eye.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

#pragma optimize("",off)
ShaderEye::ShaderEye(prosper::Context &context,const std::string &identifier)
	: ShaderPBR{context,identifier,"world/eye/vs_eye","world/eye/fs_eye"}
{}
static Vector4 GetMatAsVec4(const Mat3x4 m,int32_t i)
{
	//return Vector4(m[0][i],m[1][i],m[2][i],m[3][i]); // TODO: Might be the other way around (x,i)
	return Vector4(m[i][0],m[i][1],m[i][2],m[i][3]); // TODO: Might be the other way around (x,i)
}
bool ShaderEye::Draw(CModelSubMesh &mesh)
{
	return BindEyeball(mesh.GetSkinTextureIndex()) && ShaderPBR::Draw(mesh);
}
bool ShaderEye::BindEyeball(uint32_t skinMatIdx)
{
	auto *ent = GetBoundEntity();
	auto *animC = static_cast<pragma::CAnimatedComponent*>(ent->GetAnimatedComponent().get());
	auto mdl = ent ? ent->GetModel() : nullptr;
	auto *texGroup = mdl ? mdl->GetTextureGroup(0) : nullptr;
	if(animC == nullptr || texGroup == nullptr || skinMatIdx >= texGroup->textures.size())
		return false;
	// TODO: Do this properly (so no search lookup is required)
	/*auto matIdx = texGroup->textures.at(skinMatIdx);
	auto &eyeballs = mdl->GetEyeballs();
	auto it = std::find_if(eyeballs.begin(),eyeballs.end(),[matIdx](const Eyeball &eyeball) {
		return eyeball.irisMaterialIndex == matIdx;
	});
	if(it == eyeballs.end())
		return false;
	auto eyeballIndex = it -eyeballs.begin();*/
	static uint32_t eyeballIndex = 0u; // TODO: HACK! How do we actually find the right eyeball for this mesh?
	eyeballIndex = 1u;//(eyeballIndex == 0u) ? 1u : 0u;
	auto *eyeballData = animC->GetEyeballData(eyeballIndex);
	auto *eyeball = mdl->GetEyeball(eyeballIndex);
	if(eyeballData == nullptr || eyeball == nullptr)
		return false;
	auto &eyeballState = eyeballData->state;
	PushConstants pushConstants {};
	animC->GetEyeballProjectionVectors(eyeballIndex,pushConstants.irisProjectionU,pushConstants.irisProjectionV);
	auto pose = animC->CalcEyeballPose(eyeballIndex);
	pushConstants.eyeballOrigin = pose.GetOrigin();
#if 0
	local eyeball = eyeballs[2]
		local animC = ent:GetComponent(ents.COMPONENT_ANIMATED)
		local pos,rot = animC:GetGlobalBoneTransform(eyeball.boneIndex)

		local function convert_vertex(v)
		--v:Set(Vector(v.x,v.z,-v.y))
		return Vector(v.x,-v.z,v.y)

		--[[
			auto y = v.position.y;
			v.position.y = v.position.z;
			v.position.z = -y;
		]]
		end

			local origin = convert_vertex(eyeball.origin)
			--origin = Vector(origin.x,-origin.z,origin.y)

			local forward = convert_vertex(eyeball.forward)

			local up = convert_vertex(eyeball.up)

			local eyePose = phys.Transform(origin,Quaternion(forward,up))

			local pose = phys.Transform(pos,rot) *eyePose

			--up = pose:GetRotation():GetForward()
			print("ORIGIN: ",pose:GetOrigin())
			debug.draw_line(pose:GetOrigin(),pose:GetOrigin() +up *10,Color.Lime,12)
#endif
	return RecordPushConstants(pushConstants,sizeof(ShaderPBR::PushConstants));
}
void ShaderEye::InitializeGfxPipelinePushConstantRanges(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	AttachPushConstantRange(pipelineInfo,0u,sizeof(ShaderPBR::PushConstants) +sizeof(PushConstants),Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT);
}
#pragma optimize("",on)
