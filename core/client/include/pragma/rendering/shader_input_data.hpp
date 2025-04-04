/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_SHADER_INPUT_DATA_HPP__
#define __PRAGMA_SHADER_INPUT_DATA_HPP__

#include "pragma/clientdefinitions.h"
#include <udm.hpp>

import pragma.shadergraph;

namespace pragma::rendering {
	struct DLLCLIENT Property {
		enum class Flags : uint32_t {
			None = 0u,
			HideInEditor = 1u,
		};
		Property(const std::string &name, pragma::shadergraph::DataType type);
		Property(const Property &other);
		Property &operator=(const Property &other);

		pragma::shadergraph::Parameter *operator->() { return &parameter; }
		const pragma::shadergraph::Parameter *operator->() const { return &parameter; }

		pragma::shadergraph::Parameter parameter;

		std::optional<GString> specializationType {};
		Flags propertyFlags = Flags::None;
		size_t offset = 0;
		size_t padding = 0;

		std::unique_ptr<std::unordered_map<std::string, uint32_t>> flags {};
		size_t GetSize() const { return udm::size_of(pragma::shadergraph::to_udm_type(parameter.type)); }
	};

	struct DLLCLIENT ShaderInputDescriptor {
		static std::optional<pragma::shadergraph::Value> parse_flags_expression(const std::string &strVal, const std::unordered_map<std::string, uint32_t> &flags, udm::Type propType);

		ShaderInputDescriptor(const pragma::GString &name);
		bool AddProperty(Property &&prop);
		const pragma::GString name;
		std::vector<Property> properties;

		Property *FindProperty(const char *key)
		{
			auto it = m_propertyMap.find(key);
			if(it == m_propertyMap.end())
				return nullptr;
			return &properties[it->second];
		}
		const Property *FindProperty(const char *key) const { return const_cast<ShaderInputDescriptor *>(this)->FindProperty(key); }
		const std::unordered_map<std::string, size_t> &GetPropertyMap() const { return m_propertyMap; }

		std::optional<std::pair<size_t, size_t>> GetPropertyDataRange(const char *key) const
		{
			auto *prop = FindProperty(key);
			if(!prop)
				return {};
			size_t offset = prop->offset;
			return std::pair<size_t, size_t> {offset, udm::size_of(pragma::shadergraph::to_udm_type(prop->parameter.type))};
		}

		bool LoadFromUdmData(udm::LinkedPropertyWrapperArg prop, std::string &outErr);
		std::string ToGlslStruct() const;
	  protected:
		virtual ShaderInputDescriptor *Import(const std::string &name) { return nullptr; }
	  private:
		std::unordered_map<std::string, size_t> m_propertyMap;
	};

	struct DLLCLIENT ShaderInputData {
		ShaderInputData(const ShaderInputDescriptor &descriptor) : m_inputDescriptor {descriptor} {}
		void ResetToDefault();
		void ResetToDefault(size_t idx);
		void ResizeToDescriptor();
		const ShaderInputDescriptor &GetDescriptor() const { return m_inputDescriptor; }
		void DebugPrint();

		template<typename T>
		    requires pragma::shadergraph::is_data_type_v<T>
		std::optional<T> GetValue(const char *key) const
		{
			constexpr auto type = udm::type_to_enum<T>();
			auto *prop = m_inputDescriptor.FindProperty(key);
			if(!prop)
				return {};
			size_t offset = prop->offset;
			auto *ptr = data.data() + offset;
			return udm::visit_ng(pragma::shadergraph::to_udm_type(prop->parameter.type), [prop, ptr](auto tag) -> std::optional<T> {
				using TProp = typename decltype(tag)::type;
				if constexpr(udm::is_convertible<TProp, T>()) {
					auto *val = reinterpret_cast<const TProp *>(ptr);
					return udm::convert<TProp, T>(*val);
				}
				return {};
			});
		}
		template<typename T>
		    requires pragma::shadergraph::is_data_type_v<T>
		bool SetValue(const char *key, const T &val)
		{
			constexpr auto type = udm::type_to_enum<T>();
			auto *prop = m_inputDescriptor.FindProperty(key);
			if(!prop)
				return false;
			auto *ptr = data.data() + prop->offset;
			return udm::visit_ng(pragma::shadergraph::to_udm_type(prop->parameter.type), [prop, ptr, &val](auto tag) -> bool {
				using TProp = typename decltype(tag)::type;
				if constexpr(udm::is_convertible<T, TProp>()) {
					auto convVal = udm::convert<T, TProp>(val);
					if(memcmp(ptr, &convVal, sizeof(convVal)) == 0)
						return false; // No need to update
					memcpy(ptr, &convVal, sizeof(convVal));
					return true;
				}
				return false;
			});
		}

		std::vector<uint8_t> data;
	  private:
		const ShaderInputDescriptor &m_inputDescriptor;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::rendering::Property::Flags)

#endif
