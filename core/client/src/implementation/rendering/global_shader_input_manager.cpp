// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.global_shader_input_manager;
import :engine;
import :rendering.shader_graph;
import pragma.shadergraph;

pragma::rendering::GlobalShaderInputDataManager::GlobalShaderInputDataManager() { ResetInputDescriptor(); }

void pragma::rendering::GlobalShaderInputDataManager::ResetInputDescriptor() { m_inputDescriptor = std::make_unique<ShaderInputDescriptor>("GlobalInputData"); }

void pragma::rendering::GlobalShaderInputDataManager::AddProperty(Property &&prop)
{
	if(!m_inputDescriptor->AddProperty(std::move(prop)))
		return;
	m_inputDataDirty = true;
}

void pragma::rendering::GlobalShaderInputDataManager::PopulateProperties(const shadergraph::Graph &graph)
{
	std::vector<shadergraph::GraphNode *> globalParamNodes;
	for(auto &node : graph.GetNodes()) {
		auto *paramNode = dynamic_cast<const shader_graph::BaseInputParameterNode *>(&node->node);
		if(!paramNode)
			continue;
		shader_graph::BaseInputParameterNode::Scope scope;
		if(!node->GetInputValue(shader_graph::BaseInputParameterNode::CONST_SCOPE, scope))
			continue;
		if(scope != shader_graph::BaseInputParameterNode::Scope::Global)
			continue;
		if(globalParamNodes.size() == globalParamNodes.capacity())
			globalParamNodes.reserve(globalParamNodes.size() * 2 + 10);
		globalParamNodes.push_back(node.get());
	}

	// TODO: Sort by type?

	std::sort(globalParamNodes.begin(), globalParamNodes.end(), [](auto *a, auto *b) { return a->GetName() < b->GetName(); });

	std::vector<Property> params;
	params.reserve(globalParamNodes.size());
	for(auto *node : globalParamNodes) {
		auto &paramNode = *dynamic_cast<const shader_graph::BaseInputParameterNode *>(&node->node);
		std::string name;
		if(!node->GetInputValue(shader_graph::BaseInputParameterNode::CONST_NAME, name))
			continue;

		if(name.empty())
			continue;

		if(m_inputDescriptor->FindProperty(name.c_str()) != nullptr)
			continue; // TODO: What if a parameter is used in multiple shader graphs with different types?

		auto type = paramNode.GetParameterType();
		Property prop {name, type};

		auto res = pragma::shadergraph::visit(type, [this, node, &prop](auto tag) -> bool {
			using T = typename decltype(tag)::type;

			T defaultVal;
			if(!node->GetInputValue(shader_graph::BaseInputParameterNode::CONST_DEFAULT, defaultVal))
				return false;
			prop->defaultValue.Set(defaultVal);

			if constexpr(std::is_same_v<T, float>) {
				float minVal;
				float maxVal;
				float stepSize;
				if(!node->GetInputValue(shader_graph::InputParameterFloatNode::CONST_MIN, minVal))
					return false;
				if(!node->GetInputValue(shader_graph::InputParameterFloatNode::CONST_MAX, maxVal))
					return false;
				if(!node->GetInputValue(shader_graph::InputParameterFloatNode::CONST_STEP_SIZE, stepSize))
					return false;
				prop->min = minVal;
				prop->max = maxVal;
				// prop->stepSize = stepSize;
			}

			return true;
		});
		if(!res)
			continue;
		params.push_back(prop);
	}

	if(params.empty())
		return;

	m_inputDescriptor->properties.reserve(m_inputDescriptor->properties.size() + params.size());
	for(auto &param : params)
		m_inputDescriptor->AddProperty(std::move(param));
	// TODO: We need to re-allocate the data buffer
	// TODO: If the data size exceeds the buffer size, we need to re-allocate the buffer

	m_inputDataDirty = true;
}

void pragma::rendering::GlobalShaderInputDataManager::ClearBuffer()
{
	get_cengine()->GetRenderContext().WaitIdle();
	m_inputDataBuffer = nullptr;
	m_dirtyTracker.Clear();
}

void pragma::rendering::GlobalShaderInputDataManager::UpdateInputData()
{
	AllocateInputData();
	if(!m_inputDataDirty)
		return;
	m_inputDataDirty = false;
	auto size = m_inputData->data.size();
	m_inputData->ResizeToDescriptor();
	auto newSize = m_inputData->data.size();
	m_dirtyTracker.MarkRange(size, newSize - size);

	if(m_inputDataBuffer && newSize > m_inputDataBuffer->GetSize())
		ClearBuffer();
}

void pragma::rendering::GlobalShaderInputDataManager::UpdateBufferData(prosper::ICommandBuffer &cmd)
{
	if(m_inputDescriptor->properties.empty())
		return;
	UpdateInputData();
	if(!m_inputDataBuffer)
		ReallocateBuffer();
	for(auto &[offset, size] : m_dirtyTracker.GetRanges())
		cmd.RecordUpdateBuffer(*m_inputDataBuffer, offset, size, m_inputData->data.data() + offset);
	m_dirtyTracker.Clear();
}

void pragma::rendering::GlobalShaderInputDataManager::AllocateInputData()
{
	if(m_inputData)
		return;
	std::vector<uint8_t> oldData;
	if(m_inputData)
		oldData = m_inputData->data;
	m_inputData = std::make_unique<ShaderInputData>(*m_inputDescriptor);
	if(!m_inputDescriptor->properties.empty()) {
		auto &lastProp = m_inputDescriptor->properties.back();
		m_inputData->data.resize(lastProp.offset + udm::size_of(pragma::shadergraph::to_udm_type(lastProp.parameter.type)));
	}
	m_inputData->ResetToDefault();
	if(!oldData.empty()) {
		assert(m_inputData->data.size() >= oldData.size());
		memcpy(m_inputData->data.data(), oldData.data(), oldData.size());
	}
}

void pragma::rendering::GlobalShaderInputDataManager::ReallocateBuffer()
{
	ClearBuffer();

	// TODO: Iterate all input parameter nodes of all shader graphs and add them to the input data
	// TODO: What if a parameter is used in multiple shader graphs with different types?

	auto bufferSize = math::get_aligned_offset(m_inputData->data.size(), BUFFER_BASE_SIZE);
	if(bufferSize == 0)
		return;
	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;
	bufCreateInfo.size = bufferSize;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit | prosper::BufferUsageFlags::UniformBufferBit;
	bufCreateInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	m_inputDataBuffer = get_cengine()->GetRenderContext().CreateBuffer(bufCreateInfo, m_inputData->data.data());
	m_inputDataBuffer->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit);
}
