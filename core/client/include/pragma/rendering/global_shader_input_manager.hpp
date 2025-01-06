/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Copyright (c) 2024 Silverlan
*/

#ifndef __PRAGMA_GLOBAL_SHADER_INPUT_MANAGER_HPP__
#define __PRAGMA_GLOBAL_SHADER_INPUT_MANAGER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shader_material/shader_material.hpp"
#include <buffers/prosper_buffer.hpp>
#include <string>
#include <vector>

import pragma.shadergraph;

namespace prosper {
	class ICommandBuffer;
};

namespace pragma::rendering {
	struct ShaderInputData;
	struct ShaderInputDescriptor;
	class DLLCLIENT DirtyRangeTracker {
	  public:
		void MarkRange(size_t offset, size_t size)
		{
			auto start = offset;
			auto end = offset + size;

			auto it = ranges.begin();
			while(it != ranges.end()) {
				auto existingStart = it->first;
				auto existingEnd = it->first + it->second;

				// Check for overlap or adjacency
				if(end >= existingStart && start <= existingEnd) {
					// Merge ranges
					start = std::min(start, existingStart);
					end = std::max(end, existingEnd);
					it = ranges.erase(it); // Remove the old range
				}
				else
					++it;
			}

			// Add the merged range
			ranges.emplace_back(start, end - start);
		}

		const std::vector<std::pair<size_t, size_t>> &GetRanges() const { return ranges; }

		void Clear() { ranges.clear(); }
	  private:
		std::vector<std::pair<size_t, size_t>> ranges;
	};

	class DLLCLIENT GlobalShaderInputDataManager {
	  public:
		static constexpr size_t BUFFER_BASE_SIZE = 128;

		GlobalShaderInputDataManager();
		template<typename T>
		bool SetValue(const std::string_view &name, const T &val)
		{
			auto *prop = m_inputDescriptor->FindProperty(name.data());
			if(!prop)
				return false;
			UpdateInputData();
			if(!m_inputData->SetValue<T>(name.data(), val))
				return false;
			m_dirtyTracker.MarkRange(prop->offset, sizeof(T));
			return true;
		}

		template<typename T>
		bool GetValue(const std::string_view &name, T &outVal) const
		{
			auto *prop = m_inputDescriptor->FindProperty(name.data());
			if(!prop)
				return false;
			const_cast<GlobalShaderInputDataManager *>(this)->UpdateInputData();
			auto val = m_inputData->GetValue<T>(name.data());
			if(!val)
				return false;
			outVal = *val;
			return true;
		}

		const pragma::rendering::ShaderInputData &GetData() const { return *m_inputData; }
		const pragma::rendering::ShaderInputDescriptor &GetDescriptor() const { return *m_inputDescriptor; }
		const std::shared_ptr<prosper::IBuffer> &GetBuffer() const { return m_inputDataBuffer; }

		void UpdateBufferData(prosper::ICommandBuffer &cmd);
		void AddProperty(pragma::rendering::Property &&prop);
		void PopulateProperties(const pragma::shadergraph::Graph &graph);
	  private:
		void ResetInputDescriptor();
		void AllocateInputData();
		void ReallocateBuffer();
		void UpdateInputData();
		void ClearBuffer();

		DirtyRangeTracker m_dirtyTracker;

		std::unique_ptr<pragma::rendering::ShaderInputData> m_inputData;
		bool m_inputDataDirty = false;

		std::unique_ptr<pragma::rendering::ShaderInputDescriptor> m_inputDescriptor;
		std::shared_ptr<prosper::IBuffer> m_inputDataBuffer;
	};
};

#endif
