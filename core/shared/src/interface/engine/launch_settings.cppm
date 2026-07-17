// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "type_traits"

export module pragma.shared:engine.launch_settings;

export import pragma.udm;

export namespace pragma {
	struct DLLNETWORK LaunchSettings {
	  public:
		template<typename T>
		    requires(std::is_scoped_enum_v<T>)
		void Set(std::string_view key, T value)
		{
			Set(key, math::to_integral(value));
		}

		template<typename T>
		    requires(udm::type_to_enum_s<std::remove_cvref_t<T>>() != udm::Type::Invalid && !std::is_scoped_enum_v<T>)
		void Set(std::string_view key, T &&value)
		{
			m_settings[std::string {key}] = udm::Property::Create(std::forward<T>(value));
		}

		template<typename T>
		    requires(udm::type_to_enum_s<T>() != udm::Type::Invalid || std::is_scoped_enum_v<T>)
		std::optional<T> Get(std::string_view key) const
		{
			if constexpr(std::is_scoped_enum_v<T>) {
				auto val = Get<std::underlying_type_t<T>>(key);
				if(!val)
					return {};
				return static_cast<T>(*val);
			}
			else {
				auto it = m_settings.find(key);
				if(it == m_settings.end())
					return std::nullopt;
				return it->second->ToValue<T>();
			}
		}

		template<typename T>
		    requires(udm::type_to_enum_s<T>() != udm::Type::Invalid || std::is_scoped_enum_v<T>)
		T Get(std::string_view key, const T &defaultValue) const
		{
			auto val = Get<T>(key);
			if(!val)
				return defaultValue;
			return *val;
		}

		udm::PProperty GetProperty(std::string_view key) const
		{
			auto it = m_settings.find(key);
			if(it == m_settings.end())
				return nullptr;
			return it->second;
		}
		void SetProperty(std::string_view key, const std::shared_ptr<udm::Property> &prop) { m_settings[std::string {key}] = prop; }
	  private:
		string::StringMap<udm::PProperty> m_settings;
	};
};
