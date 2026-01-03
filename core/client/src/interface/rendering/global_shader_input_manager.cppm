// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.global_shader_input_manager;

export import :rendering.shader_input_data;
export import pragma.prosper;
export import pragma.shadergraph;

export namespace pragma::rendering {
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

		const ShaderInputData &GetData() const { return *m_inputData; }
		const ShaderInputDescriptor &GetDescriptor() const { return *m_inputDescriptor; }
		const std::shared_ptr<prosper::IBuffer> &GetBuffer() const { return m_inputDataBuffer; }

		void UpdateBufferData(prosper::ICommandBuffer &cmd);
		void AddProperty(Property &&prop);
		void PopulateProperties(const shadergraph::Graph &graph);
	  private:
		void ResetInputDescriptor();
		void AllocateInputData();
		void ReallocateBuffer();
		void UpdateInputData();
		void ClearBuffer();

		DirtyRangeTracker m_dirtyTracker;

		std::unique_ptr<ShaderInputData> m_inputData;
		bool m_inputDataDirty = false;

		std::unique_ptr<ShaderInputDescriptor> m_inputDescriptor;
		std::shared_ptr<prosper::IBuffer> m_inputDataBuffer;
	};
};
