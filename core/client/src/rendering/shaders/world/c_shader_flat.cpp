/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_flat.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

decltype(ShaderFlat::VERTEX_BINDING_VERTEX) ShaderFlat::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex,sizeof(VertexBufferData)};
decltype(ShaderFlat::VERTEX_ATTRIBUTE_POSITION) ShaderFlat::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32B32_SFloat};
decltype(ShaderFlat::VERTEX_ATTRIBUTE_UV) ShaderFlat::VERTEX_ATTRIBUTE_UV = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32_SFloat};

decltype(ShaderFlat::DESCRIPTOR_SET_INSTANCE) ShaderFlat::DESCRIPTOR_SET_INSTANCE = {
	{
		prosper::DescriptorSetInfo::Binding { // Instance
			prosper::DescriptorType::UniformBufferDynamic,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Bone Matrices
			prosper::DescriptorType::UniformBufferDynamic,
			prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Vertex Animations
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Vertex Animation Frame Data
			prosper::DescriptorType::StorageBuffer,
			prosper::ShaderStageFlags::VertexBit
		}
	}
};
decltype(ShaderFlat::DESCRIPTOR_SET_CAMERA) ShaderFlat::DESCRIPTOR_SET_CAMERA = {
	{
		prosper::DescriptorSetInfo::Binding { // Camera
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Render Settings
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // SSAO Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
decltype(ShaderFlat::DESCRIPTOR_SET_MATERIAL) ShaderFlat::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::DescriptorSetInfo::Binding { // Diffuse Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Normal Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Specular Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Parallax Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Glow Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		}
	}
};
ShaderFlat::ShaderFlat(prosper::IPrContext &context,const std::string &identifier)
	: ShaderScene(context,identifier,"world/vs_flat","world/fs_flat")
{
	// SetBaseShader<ShaderTextured3DBase>();
}
bool ShaderFlat::BindScene(const pragma::CSceneComponent &scene,bool bView)
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
	return RecordBindDescriptorSet(*descSetGroup->GetDescriptorSet(),DESCRIPTOR_SET_MATERIAL.setIndex);
}
void ShaderFlat::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
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
	auto &vkMesh = mesh.GetSceneMesh();
	auto &vertexBuffer = vkMesh->GetVertexBuffer();
	auto &indexBuffer = vkMesh->GetIndexBuffer();
	auto &vertexWeightBuffer = vkMesh->GetVertexWeightBuffer();
	if(vkMesh == nullptr || vertexBuffer == nullptr || indexBuffer == nullptr)
	{
		// TODO: Re-enable this once a logging system with categories is in place
		/*Con::cwar<<"WARNING: Attempted to render mesh with invalid ";
		if(vkMesh == nullptr)
			Con::cwar<<"VKMesh";
		else if(vertexBuffer == nullptr)
			Con::cwar<<"Vertex Buffer";
		else
			Con::cwar<<"Index Buffer";
		Con::cwar<<"! Skipping..."<<Con::endl;*/
		return false;
	}

	auto *scene = static_cast<CGame*>(c_engine->GetClientState()->GetGameState())->GetRenderScene();
	if(scene == nullptr)
		return false;
	auto &cam = scene->GetActiveCamera();
	auto mvp = cam.valid() ? cam->GetProjectionMatrix() *cam->GetViewMatrix() : Mat4{1.f};
	RecordBindVertexBuffer(*vertexBuffer);
	RecordBindIndexBuffer(*indexBuffer,prosper::IndexType::UInt16);
	RecordDrawIndexed(mesh.GetTriangleVertexCount());
	return true;
}
