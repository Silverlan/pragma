#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_flat.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderFlat::VERTEX_BINDING_VERTEX) ShaderFlat::VERTEX_BINDING_VERTEX = {Anvil::VertexInputRate::VERTEX,sizeof(VertexBufferData)};
decltype(ShaderFlat::VERTEX_ATTRIBUTE_POSITION) ShaderFlat::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32B32_SFLOAT};
decltype(ShaderFlat::VERTEX_ATTRIBUTE_UV) ShaderFlat::VERTEX_ATTRIBUTE_UV = {VERTEX_BINDING_VERTEX,Anvil::Format::R32G32_SFLOAT};

decltype(ShaderFlat::DESCRIPTOR_SET_INSTANCE) ShaderFlat::DESCRIPTOR_SET_INSTANCE = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Instance
			Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Bone Matrices
			Anvil::DescriptorType::UNIFORM_BUFFER_DYNAMIC,
			Anvil::ShaderStageFlagBits::VERTEX_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Vertex Animations
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::VERTEX_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Vertex Animation Frame Data
			Anvil::DescriptorType::STORAGE_BUFFER,
			Anvil::ShaderStageFlagBits::VERTEX_BIT
		}
	}
};
decltype(ShaderFlat::DESCRIPTOR_SET_CAMERA) ShaderFlat::DESCRIPTOR_SET_CAMERA = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Camera
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Render Settings
			Anvil::DescriptorType::UNIFORM_BUFFER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT | Anvil::ShaderStageFlagBits::VERTEX_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // SSAO Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
decltype(ShaderFlat::DESCRIPTOR_SET_MATERIAL) ShaderFlat::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::Shader::DescriptorSetInfo::Binding { // Diffuse Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Normal Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Specular Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Parallax Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		},
		prosper::Shader::DescriptorSetInfo::Binding { // Glow Map
			Anvil::DescriptorType::COMBINED_IMAGE_SAMPLER,
			Anvil::ShaderStageFlagBits::FRAGMENT_BIT
		}
	}
};
ShaderFlat::ShaderFlat(prosper::Context &context,const std::string &identifier)
	: ShaderScene(context,identifier,"world/vs_flat","world/fs_flat")
{
	SetBaseShader<ShaderTextured3D>();
}
bool ShaderFlat::BindScene(const Scene &scene,bool bView)
{
	auto descSet = (bView == true) ? scene.GetViewCameraDescriptorSet() : scene.GetCameraDescriptorSetGraphics();
	return RecordBindDescriptorSet(*descSet,DESCRIPTOR_SET_CAMERA.setIndex);
}
bool ShaderFlat::BindEntity(CBaseEntity &ent)
{
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent.expired())
		return false;
	pRenderComponent->UpdateRenderData(c_game->GetCurrentDrawCommandBuffer());
	return RecordBindDescriptorSet(*pRenderComponent->GetRenderDescriptorSet(),DESCRIPTOR_SET_INSTANCE.setIndex,{0u,0u});
}
bool ShaderFlat::BindMaterial(CMaterial &mat)
{
	auto descSetGroup = mat.GetDescriptorSetGroup(*this);
	if(descSetGroup == nullptr)
		return false;
	return RecordBindDescriptorSet(*(*descSetGroup)->get_descriptor_set(0u),DESCRIPTOR_SET_MATERIAL.setIndex);
}
void ShaderFlat::InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderScene::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_UV);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_CAMERA);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_MATERIAL);
}
bool ShaderFlat::Draw(CModelSubMesh &mesh)
{
	auto numTriangleVertices = mesh.GetTriangleVertexCount();
	if(numTriangleVertices > umath::to_integral(GameLimits::MaxMeshVertices))
	{
		Con::cerr<<"ERROR: Attempted to draw mesh with more than maximum ("<<umath::to_integral(GameLimits::MaxMeshVertices)<<") amount of vertices!"<<Con::endl;
		return false;
	}
	auto &vkMesh = mesh.GetVKMesh();
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
		return false;
	}

	auto &scene = static_cast<CGame*>(c_engine->GetClientState()->GetGameState())->GetRenderScene();
	auto &cam = scene->GetActiveCamera();
	auto mvp = cam.valid() ? cam->GetProjectionMatrix() *cam->GetViewMatrix() : Mat4{1.f};
	RecordBindVertexBuffer(vertexBuffer->GetAnvilBuffer());
	RecordBindIndexBuffer(indexBuffer->GetAnvilBuffer(),Anvil::IndexType::UINT16);
	RecordDrawIndexed(mesh.GetTriangleVertexCount());
	return true;
}
