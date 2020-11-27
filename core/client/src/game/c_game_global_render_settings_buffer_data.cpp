/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;

prosper::IDescriptorSet &CGame::GetGlobalRenderSettingsDescriptorSet() {return *m_globalRenderSettingsBufferData->descSetGroup->GetDescriptorSet();}
CGame::GlobalRenderSettingsBufferData &CGame::GetGlobalRenderSettingsBufferData() {return *m_globalRenderSettingsBufferData;}

CGame::GlobalRenderSettingsBufferData::GlobalRenderSettingsBufferData()
{
	pragma::ShaderTextured3DBase::DebugData debugData {0};
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit;
	createInfo.size = sizeof(pragma::ShaderTextured3DBase::DebugData);
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	debugBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,&debugData);
	debugBuffer->SetDebugName("render_settings_debug_buf");

	pragma::ShaderTextured3DBase::CSMData csmData {
		{umat::identity(),umat::identity(),umat::identity(),umat::identity()}, // View-projection matrices
		Vector4(0.f,0.f,0.f,0.f), // Far distances
		0 // Cascade Count
	};
	createInfo.size = sizeof(pragma::ShaderTextured3DBase::CSMData);
	csmBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,&csmData);
	csmBuffer->SetDebugName("csm_data_buf");

	pragma::ShaderTextured3DBase::TimeData timeData {0.f,0.f,0.f,0.f};
	createInfo.size = sizeof(timeData);
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	timeBuffer = c_engine->GetRenderContext().CreateBuffer(createInfo,&timeData);
	timeBuffer->SetDebugName("time_data_buf");

	if(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_RENDER_SETTINGS.IsValid() == false)
		return;
	descSetGroup = c_engine->GetRenderContext().CreateDescriptorSetGroup(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_RENDER_SETTINGS);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingUniformBuffer(
		*debugBuffer,umath::to_integral(pragma::ShaderScene::RenderSettingsBinding::Debug)
	);
	descSet.SetBindingUniformBuffer(
		*timeBuffer,umath::to_integral(pragma::ShaderScene::RenderSettingsBinding::Time)
	);
	descSet.SetBindingUniformBuffer(
		*csmBuffer,umath::to_integral(pragma::ShaderScene::RenderSettingsBinding::CSMData)
	);
	auto &entInstanceBuffer = *pragma::CRenderComponent::GetInstanceBuffer();
	descSet.SetBindingStorageBuffer(
		entInstanceBuffer,umath::to_integral(pragma::ShaderScene::RenderSettingsBinding::GlobalInstance)
	);
}
