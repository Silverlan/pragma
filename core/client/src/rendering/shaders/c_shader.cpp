#include "stdafx_client.h"
 // prosper TODO
#if 0
#include "pragma/clientstate/clientstate.h"
#include <algorithm>
#include "shader.h"
#include "pragma/rendering/shaders/c_shader.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_side.h"
#include "pragma/entities/c_baseentity.h"
#include <mathutil/umat.h>
#include <pragma/console/convars.h>
#include "gluniformblockmanager.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/rendering/uniformbinding.h"
#include "pragma/model/c_normalmesh.h"
#include "pragma/rendering/shaders/world/c_shader_textured.h"
#include "c_shader_particle.h"
#include "pragma/rendering/shaders/debug/c_shader_debug.h"
#include "pragma/model/c_modelmesh.h"

using namespace Shader;

////////////////////////////

#pragma optimize("",off)
extern ClientState *client;
extern DLLCLIENT CGame *c_game;
Base3D::Base3D(const std::string &identifier,const std::string &vsShader,const std::string &fsShader,const std::string &gsShader)
	: Base(identifier,vsShader,fsShader,gsShader),m_bDynamicLineWidth(true)
{
}
Base3D::Base3D()
	: Base(),m_bDynamicLineWidth(true)
{}
Base3D::~Base3D() {}
void Base3D::InitializeDynamicStates(std::vector<vk::DynamicState> &states)
{
	Base::InitializeDynamicStates(states);
	if(m_bDynamicLineWidth == true)
		states.push_back(vk::DynamicState::eLineWidth);
}
void Base3D::Render(Camera*,Material*)
{
	//Use(); // Vulkan TODO
}

void Base3D::InitializeMaterial(Material*) {}
/*
void Shader3DBase::InitializeShader(std::string identifier,std::string vsShader,std::string fsShader)
{
	//ShaderBase::InitializeShader(identifier,vsShader,fsShader); // Vulkan TODO
}
*/ // Vulkan TODO
void Base3D::Render(Camera *cam,Material *mat,CParticleSystem*)
{
	Render(cam,mat);
}

void Base3D::Render(Camera *cam,Material *mat,CBaseEntity*)
{
	Render(cam,mat);
}

void Base3D::Render(Camera *cam,Material *mat,CBaseEntity *ent,::Model*,CModelMesh*,CModelSubMesh*)
{
	Render(cam,mat,ent);
}

void Base3D::Render(Camera *cam,Material *mat,CBaseEntity *ent,CBrushMesh*,CSide*)
{
	Render(cam,mat,ent);
}

// New
void Base3D::Bind() {}
void Base3D::Bind(Material*) {}
void Base3D::Bind(Camera*,CBaseEntity*) {}
void Base3D::Render(Camera*,GLMesh*) {}
void Base3D::Unbind() {}

void Base3D::Draw(CModelSubMesh *mesh,const std::function<void(const Vulkan::CommandBufferObject*,std::size_t)> &fDraw)
{
	auto numTriangleVertices = mesh->GetTriangleVertexCount();
	if(numTriangleVertices > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return;
	}
	auto &context = *m_context.get();
	auto &drawCmd = context.GetDrawCmd();
	//auto &pipeline = *GetPipeline();
	//auto &layout = m_pipeline->GetPipelineLayout();

	auto &vkMesh = mesh->GetVKMesh();
	auto &vertexBuffer = vkMesh->GetVertexBuffer();
	auto &indexBuffer = vkMesh->GetIndexBuffer();
	auto &vertexWeightBuffer = vkMesh->GetVertexWeightBuffer();
	if(vkMesh == nullptr || vertexBuffer == nullptr || indexBuffer == nullptr)
	{
		Con::cwar<<"WARNING: Attempted to render mesh with invalid ";
		if(vkMesh == nullptr)
			Con::cwar<<"VKMesh";
		else if(vertexBuffer == nullptr)
			Con::cwar<<"Vertex Buffer";
		else
			Con::cwar<<"Index Buffer";
		Con::cwar<<"! Skipping..."<<Con::endl;
		return;
	}

	if(m_bAnimated == true && vertexWeightBuffer.IsValid() == true)
	{
		drawCmd->BindVertexBuffer({
			vertexBuffer,
			vertexWeightBuffer
		});
	}
	else
		drawCmd->BindVertexBuffer({vertexBuffer,c_game->GetDummyVertexBuffer()});

	drawCmd->BindIndexBuffer(indexBuffer,vk::IndexType::eUint16);
	fDraw(drawCmd,numTriangleVertices);
}

void Base3D::Draw(CModelSubMesh *mesh)
{
	Draw(mesh,[](const Vulkan::CommandBufferObject *drawCmd,std::size_t numTriangleVertices) {
		drawCmd->DrawIndexed(numTriangleVertices);
	});
}

bool Base3D::IsWeighted(CBaseEntity *ent) const
{
	auto mdl = ent->GetModel();
	return (mdl != nullptr && ent->IsAnimated() && mdl->GetBoneCount() > 1) ? true : false;
}

bool Base3D::BindEntity(Vulkan::CommandBufferObject *cmdBuffer,CBaseEntity *ent,uint32_t descSetId,bool &bWeighted)
{
	auto &renderBuffer = ent->GetRenderBuffer();
	if(renderBuffer == nullptr)
		return false;
	ent->UpdateRenderData();
	auto *boneBuffer = ent->GetBoneBuffer();

	auto *pipeline = GetPipeline();
	auto &layout = pipeline->GetPipelineLayout();
	m_bAnimated = IsWeighted(ent);

	cmdBuffer->BindDescriptorSet(descSetId,layout,ent->GetRenderDescriptorSet(),{0,0}); // Instance and bone matrices
	/*auto &descSetInstance = *pipeline->GetDescriptorSet(umath::to_integral(DescSet::Instance),umath::to_integral(Binding::Instance),renderBuffer->GetBaseIndex());
	BindInstanceDescriptorSet(cmdBuffer,descSetInstance,static_cast<uint32_t>(renderBuffer->GetBaseOffset()));
	if(m_bUseBoneWeights == true && m_bAnimated == true && boneBuffer != nullptr)
	{
		auto &descSetBones = *pipeline->GetDescriptorSet(umath::to_integral(DescSet::BoneMatrix),umath::to_integral(Binding::BoneMatrix),(*boneBuffer)->GetBaseIndex());
		cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::BoneMatrix),layout,descSetBones,static_cast<uint32_t>((*boneBuffer)->GetBaseOffset())); // Bone Matrices
		cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::BoneMatrix),layout,ent->GetBoneDescriptorSet(),0); // Bone Matrices
	}*/
	bWeighted = m_bAnimated;
	return true;
}
#endif
#pragma optimize("",on)
