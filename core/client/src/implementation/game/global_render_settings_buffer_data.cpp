// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :game;
import :engine;
import :entities.components;

prosper::IDescriptorSet &pragma::CGame::GetGlobalRenderSettingsDescriptorSet() { return *m_globalRenderSettingsBufferData->descSetGroup->GetDescriptorSet(); }
pragma::rendering::GlobalRenderSettingsBufferData &pragma::CGame::GetGlobalRenderSettingsBufferData() { return *m_globalRenderSettingsBufferData; }

pragma::rendering::GlobalRenderSettingsBufferData::GlobalRenderSettingsBufferData()
{
	ShaderGameWorldLightingPass::DebugData debugData {0};
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit | prosper::BufferUsageFlags::TransferDstBit | prosper::BufferUsageFlags::TransferSrcBit;
	createInfo.size = sizeof(ShaderGameWorldLightingPass::DebugData);
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	debugBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo, &debugData);
	debugBuffer->SetDebugName("render_settings_debug_buf");

	ShaderGameWorldLightingPass::CSMData csmData {
	  {umat::identity(), umat::identity(), umat::identity(), umat::identity()}, // View-projection matrices
	  Vector4(0.f, 0.f, 0.f, 0.f),                                              // Far distances
	  0                                                                         // Cascade Count
	};
	createInfo.size = sizeof(ShaderGameWorldLightingPass::CSMData);
	csmBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo, &csmData);
	csmBuffer->SetDebugName("csm_data_buf");

	ShaderGameWorldLightingPass::TimeData timeData {0.f, 0.f, 0.f, 0.f};
	createInfo.size = sizeof(timeData);
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	timeBuffer = get_cengine()->GetRenderContext().CreateBuffer(createInfo, &timeData);
	timeBuffer->SetDebugName("time_data_buf");

	if(ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDER_SETTINGS.IsValid() == false)
		return;
	descSetGroup = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDER_SETTINGS);
	auto &descSet = *descSetGroup->GetDescriptorSet();
	descSet.SetBindingUniformBuffer(*debugBuffer, math::to_integral(ShaderScene::RenderSettingsBinding::Debug));
	descSet.SetBindingUniformBuffer(*timeBuffer, math::to_integral(ShaderScene::RenderSettingsBinding::Time));
	descSet.SetBindingUniformBuffer(*csmBuffer, math::to_integral(ShaderScene::RenderSettingsBinding::CSMData));
	auto &entInstanceBuffer = *CRenderComponent::GetInstanceBuffer();
	descSet.SetBindingStorageBuffer(entInstanceBuffer, math::to_integral(ShaderScene::RenderSettingsBinding::GlobalInstance));

#ifdef PRAGMA_ENABLE_SHADER_DEBUG_PRINT
	{
		struct DebugPrintData {
			Mat4 value;
			uint32_t type;
		};
		DebugPrintData initialDebugPrintData {};
		prosper::util::BufferCreateInfo createInfo {};
		createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit | prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
		createInfo.size = sizeof(DebugPrintData);
		createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
		createInfo.flags = prosper::util::BufferCreateInfo::Flags::Persistent;
		debugPrintBuffer = pragma::get_cengine()->GetRenderContext().CreateBuffer(createInfo, &initialDebugPrintData);
		debugPrintBuffer->SetDebugName("render_settings_debug_print_buf");
		debugPrintBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::ReadBit);
		descSet.SetBindingStorageBuffer(*debugPrintBuffer, pragma::math::to_integral(pragma::ShaderScene::RenderSettingsBinding::DebugPrint));
	}
#endif
}

#ifdef PRAGMA_ENABLE_SHADER_DEBUG_PRINT
void pragma::rendering::GlobalRenderSettingsBufferData::EvaluateDebugPrint()
{
	// Note: The debug string buffer is *not* procted for concurrent read
	// or write access and should only be used for debugging purposes!
	auto debugPrintStr = GetDebugPrintString();
	if(debugPrintStr.has_value()) {
		Con::COUT << "Shader Debug Print: " << *debugPrintStr << Con::endl;
		ResetDebugPrintData();
	}
}
void pragma::rendering::GlobalRenderSettingsBufferData::ResetDebugPrintData()
{
	DebugPrintData data {};
	debugPrintBuffer->Write(0, data);
}
std::optional<std::string> pragma::rendering::GlobalRenderSettingsBufferData::GetDebugPrintString() const
{
	DebugPrintData data {};
	if(!debugPrintBuffer->Read(0, data))
		return "No data.";
	if(data.type == GlslType::NotSet)
		return {};
	if(pragma::math::to_integral(data.type) >= pragma::math::to_integral(GlslType::Count))
		return "Invalid type.";
	return udm::visit(glsl_type_to_udm(data.type), [&data](auto tag) -> std::string {
		using T = typename decltype(tag)::type;
		if constexpr(is_glsl_type<T>) {
			T val;
			auto *fdata = reinterpret_cast<float *>(&data.data[0]);
			if constexpr(std::is_same_v<T, udm::Float>)
				val = fdata[0];
			else if constexpr(std::is_same_v<T, udm::Boolean>)
				val = (fdata[0] > 0.5f) ? true : false;
			else if constexpr(std::is_same_v<T, udm::Int32>)
				val = static_cast<int32_t>(fdata[0]);
			else if constexpr(std::is_same_v<T, udm::UInt32>)
				val = static_cast<uint32_t>(fdata[0]);
			else if constexpr(std::is_same_v<T, Mat4>)
				val = *reinterpret_cast<Mat4 *>(&fdata[0]);
			else if constexpr(std::is_same_v<T, Vector4>)
				val = *reinterpret_cast<Vector4 *>(&fdata[0]);
			else if constexpr(std::is_same_v<T, Vector3>)
				val = *reinterpret_cast<Vector3 *>(&fdata[0]);
			else if constexpr(std::is_same_v<T, Vector2>)
				val = *reinterpret_cast<Vector2 *>(&fdata[0]);
			else if constexpr(std::is_same_v<T, Vector4i>) {
				Vector4i vec;
				for(auto i = 0u; i < 4; ++i)
					vec[i] = static_cast<int32_t>(fdata[i]);
				val = vec;
			}
			else if constexpr(std::is_same_v<T, Vector3i>) {
				Vector3i vec;
				for(auto i = 0u; i < 3; ++i)
					vec[i] = static_cast<int32_t>(fdata[i]);
				val = vec;
			}
			else if constexpr(std::is_same_v<T, Vector2i>) {
				Vector2i vec;
				for(auto i = 0u; i < 2; ++i)
					vec[i] = static_cast<int32_t>(fdata[i]);
				val = vec;
			}
			auto str = udm::convert<T, udm::String>(val);
			return str;
		}
		// Unreachable
		return "Invalid type.";
	});
}
#endif
