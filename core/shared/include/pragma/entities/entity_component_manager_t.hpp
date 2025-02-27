/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENTITY_COMPONENT_MANAGER_T_HPP__
#define __ENTITY_COMPONENT_MANAGER_T_HPP__

#include "pragma/entities/entity_component_manager.hpp"
#include "pragma/entities/attribute_specialization_type.hpp"
#include "pragma/entities/member_type.hpp"
#include <sharedutils/util.h>
#include <udm.hpp>

import panima;

class BaseEntity;
namespace pragma {
	constexpr bool is_animatable_type(udm::Type type) { return panima::is_animatable_type(type); }
	constexpr bool is_animatable_type(pragma::ents::EntityMemberType type) { return (umath::to_integral(type) < umath::to_integral(udm::Type::Count)) ? is_animatable_type(static_cast<udm::Type>(type)) : false; }
	constexpr bool is_valid_component_property_type(udm::Type type) { return is_animatable_type(type) || type == udm::Type::String || type == udm::Type::Transform || type == udm::Type::ScaledTransform; }
	constexpr bool is_valid_component_property_type(pragma::ents::EntityMemberType type)
	{
		static_assert(umath::to_integral(pragma::ents::EntityMemberType::VersionIndex) == 0);
		return is_valid_component_property_type(static_cast<udm::Type>(type)) || type == pragma::ents::EntityMemberType::Entity || type == pragma::ents::EntityMemberType::MultiEntity || type == pragma::ents::EntityMemberType::ComponentProperty
		  || type == pragma::ents::EntityMemberType::Element;
	}
	template<typename T>
	concept is_animatable_type_v = is_animatable_type(udm::type_to_enum<T>());
	template<typename T>
	concept is_valid_component_property_type_v = is_valid_component_property_type(pragma::ents::member_type_to_enum<T>());

	template<typename TComponent, typename T, auto TSetter, auto TGetter, typename TSpecializationType>
	    requires(is_valid_component_property_type_v<T> && (std::is_same_v<TSpecializationType, AttributeSpecializationType> || util::is_string<TSpecializationType>::value))
	static ComponentMemberInfo create_component_member_info(std::string &&name, std::optional<T> defaultValue, TSpecializationType specialization)
	{
		auto memberInfo = ComponentMemberInfo::CreateDummy();
		memberInfo.SetName(std::move(name));
		memberInfo.type = ents::member_type_to_enum<T>();
		memberInfo.SetGetterFunction<TComponent, T, TGetter>();
		memberInfo.SetSetterFunction<TComponent, T, TSetter>();
		memberInfo.SetSpecializationType(specialization);
		if constexpr(std::is_enum_v<T>) {
			memberInfo.SetEnum(
			  [](const std::string &name) -> int64_t {
				  auto v = magic_enum::enum_cast<T>(name);
				  if(!v.has_value())
					  return {};
				  return static_cast<int64_t>(*v);
			  },
			  [](int64_t value) -> std::optional<std::string> {
				  auto e = magic_enum::enum_name<T>(static_cast<T>(value));
				  if(e.empty())
					  return {};
				  return std::string {e};
			  },
			  []() -> std::vector<int64_t> {
				  auto &enumValues = magic_enum::enum_values<T>();
				  std::vector<int64_t> rvalues;
				  rvalues.resize(enumValues.size());
				  for(auto i = decltype(enumValues.size()) {0u}; i < enumValues.size(); ++i)
					  rvalues[i] = static_cast<int64_t>(enumValues[i]);
				  return rvalues;
			  });
			auto &enumValues = magic_enum::enum_values<T>();
			if(!enumValues.empty()) {
				memberInfo.SetMin(static_cast<float>(enumValues.front()));
				memberInfo.SetMax(static_cast<float>(enumValues.back()));
			}
		}
		if(defaultValue.has_value())
			memberInfo.SetDefault<T>(*defaultValue);
		return memberInfo;
	}

	template<typename TComponent, typename T, auto TSetter, auto TGetter>
	    requires(is_valid_component_property_type_v<T>)
	static ComponentMemberInfo create_component_member_info(std::string &&name, std::optional<T> defaultValue = {})
	{
		return create_component_member_info<TComponent, T, TSetter, TGetter, AttributeSpecializationType>(std::move(name), std::move(defaultValue), AttributeSpecializationType::None);
	}

	template<typename T>
	void ComponentMemberInfo::SetDefault(T value)
	{
		if(ents::member_type_to_enum<T>() != type)
			throw std::runtime_error {"Unable to set default member value: Value type " + std::string {magic_enum::enum_name(ents::member_type_to_enum<T>())} + " does not match member type " + std::string {magic_enum::enum_name(type)} + "!"};
		m_default = std::unique_ptr<void, void (*)(void *)> {new T {std::move(value)}, [](void *ptr) { delete static_cast<T *>(ptr); }};
	}

	template<typename T>
	bool ComponentMemberInfo::GetDefault(T &outValue) const
	{
		if(!m_default || ents::member_type_to_enum<T>() != type)
			return false;
		outValue = *static_cast<T *>(m_default.get());
		return true;
	}
};

#endif
