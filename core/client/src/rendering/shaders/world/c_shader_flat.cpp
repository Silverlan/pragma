/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_flat.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include <shader/prosper_pipeline_create_info.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_util.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <cmaterial.h>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

decltype(ShaderFlat::VERTEX_BINDING_VERTEX) ShaderFlat::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex,sizeof(VertexBufferData)};
decltype(ShaderFlat::VERTEX_ATTRIBUTE_POSITION) ShaderFlat::VERTEX_ATTRIBUTE_POSITION = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32B32_SFloat};
decltype(ShaderFlat::VERTEX_ATTRIBUTE_UV) ShaderFlat::VERTEX_ATTRIBUTE_UV = {VERTEX_BINDING_VERTEX,prosper::Format::R32G32_SFloat};

decltype(ShaderFlat::DESCRIPTOR_SET_INSTANCE) ShaderFlat::DESCRIPTOR_SET_INSTANCE = {
	{
		prosper::DescriptorSetInfo::Binding { // Instance
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit
		},
		prosper::DescriptorSetInfo::Binding { // Bone Matrices
			prosper::DescriptorType::UniformBuffer,
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
decltype(ShaderFlat::DESCRIPTOR_SET_SCENE) ShaderFlat::DESCRIPTOR_SET_SCENE = {
	{
		prosper::DescriptorSetInfo::Binding { // Camera
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit
		},
		prosper::DescriptorSetInfo::Binding { // Render Settings
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::GeometryBit
		}
	}
};
decltype(ShaderFlat::DESCRIPTOR_SET_MATERIAL) ShaderFlat::DESCRIPTOR_SET_MATERIAL = {
	{
		prosper::DescriptorSetInfo::Binding { // Material settings
			prosper::DescriptorType::UniformBuffer,
			prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit | prosper::ShaderStageFlags::GeometryBit
		},
		prosper::DescriptorSetInfo::Binding { // Albedo Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Normal Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // RMA Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Emission Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Parallax Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Wrinkle Stretch Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Wrinkle Compress Map
			prosper::DescriptorType::CombinedImageSampler,
			prosper::ShaderStageFlags::FragmentBit
		},
		prosper::DescriptorSetInfo::Binding { // Exponent Map
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
void ShaderFlat::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderScene::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_UV);

	AddDescriptorSetGroup(pipelineInfo,pipelineIdx,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,pipelineIdx,DESCRIPTOR_SET_MATERIAL);
	AddDescriptorSetGroup(pipelineInfo,pipelineIdx,DESCRIPTOR_SET_SCENE);
}
