#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>

extern DLLCENGINE CEngine *c_engine;

Anvil::DescriptorSet &CGame::GetGlobalRenderSettingsDescriptorSet() {return *(*m_globalRenderSettingsBufferData->descSetGroup)->get_descriptor_set(0u);}
CGame::GlobalRenderSettingsBufferData &CGame::GetGlobalRenderSettingsBufferData() {return *m_globalRenderSettingsBufferData;}

CGame::GlobalRenderSettingsBufferData::GlobalRenderSettingsBufferData()
{
	auto &dev = c_engine->GetDevice();

	pragma::ShaderTextured3DBase::DebugData debugData {0};
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::UNIFORM_BUFFER_BIT | Anvil::BufferUsageFlagBits::TRANSFER_DST_BIT;
	createInfo.size = sizeof(pragma::ShaderTextured3DBase::DebugData);
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	debugBuffer = prosper::util::create_buffer(dev,createInfo,&debugData);
	debugBuffer->SetDebugName("render_settings_debug_buf");

	pragma::ShaderTextured3DBase::CSMData csmData {
		{umat::identity(),umat::identity(),umat::identity(),umat::identity()}, // View-projection matrices
		Vector4(0.f,0.f,0.f,0.f), // Far distances
		0 // Cascade Count
	};
	createInfo.size = sizeof(pragma::ShaderTextured3DBase::CSMData);
	csmBuffer = prosper::util::create_buffer(c_engine->GetDevice(),createInfo,&csmData);
	csmBuffer->SetDebugName("csm_data_buf");

	pragma::ShaderTextured3DBase::TimeData timeData {0.f,0.f,0.f,0.f};
	createInfo.size = sizeof(timeData);
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	timeBuffer = prosper::util::create_buffer(c_engine->GetDevice(),createInfo,&timeData);
	timeBuffer->SetDebugName("time_data_buf");

	if(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_RENDER_SETTINGS.IsValid() == false)
		return;
	descSetGroup = prosper::util::create_descriptor_set_group(dev,pragma::ShaderTextured3DBase::DESCRIPTOR_SET_RENDER_SETTINGS);
	auto &descSet = *(*descSetGroup)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		descSet,*debugBuffer,umath::to_integral(pragma::ShaderScene::RenderSettingsBinding::Debug)
	);
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		descSet,*timeBuffer,umath::to_integral(pragma::ShaderScene::RenderSettingsBinding::Time)
	);
	prosper::util::set_descriptor_set_binding_uniform_buffer(
		descSet,*csmBuffer,umath::to_integral(pragma::ShaderScene::RenderSettingsBinding::CSMData)
	);
}
