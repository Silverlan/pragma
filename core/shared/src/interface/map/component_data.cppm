// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:map.component_data;

export import pragma.udm;

export {
	namespace pragma::asset {
		class DLLNETWORK ComponentData : public std::enable_shared_from_this<ComponentData> {
		  public:
			enum class Flags : uint64_t {
				None = 0u,
				ClientsideOnly = 1u,
			};

			static std::shared_ptr<ComponentData> Create();

			Flags GetFlags() const;
			void SetFlags(Flags flags);
			udm::PProperty GetData() const { return m_data; }
		  private:
			ComponentData();
			udm::PProperty m_data;
			Flags m_flags = Flags::None;
		};
	};
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::asset::ComponentData &componentData);
};
