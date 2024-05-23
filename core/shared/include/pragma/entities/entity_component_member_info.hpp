/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENTITY_COMPONENT_MEMBER_INFO_HPP__
#define __ENTITY_COMPONENT_MEMBER_INFO_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/util/global_string_table.hpp"
#include <mathutil/transform.hpp>
#include <functional>
#include <optional>
#include <typeindex>

namespace udm {
	enum class Type : uint8_t;
	struct Element;
	struct Property;
	using PProperty = std::shared_ptr<Property>;
};

namespace pragma {
	namespace ents {
		enum class EntityMemberType : uint8_t;

		struct DLLNETWORK TypeMetaData {
			virtual ~TypeMetaData() = default;
		};
		struct DLLNETWORK RangeTypeMetaData : public TypeMetaData {
			std::optional<float> min {};
			std::optional<float> max {};
			std::optional<float> stepSize {};
		};

		struct DLLNETWORK CoordinateTypeMetaData : public TypeMetaData {
			umath::CoordinateSpace space = umath::CoordinateSpace::World;
			pragma::GString parentProperty = nullptr;
		};

		struct DLLNETWORK PoseTypeMetaData : public TypeMetaData {
			pragma::GString posProperty = nullptr;
			pragma::GString rotProperty = nullptr;
			pragma::GString scaleProperty = nullptr;
		};

		struct DLLNETWORK PoseComponentTypeMetaData : public TypeMetaData {
			pragma::GString poseProperty = nullptr;
		};

		struct DLLNETWORK OptionalTypeMetaData : public TypeMetaData {
			pragma::GString enabledProperty = nullptr;
		};

		struct DLLNETWORK EnablerTypeMetaData : public TypeMetaData {
			pragma::GString targetProperty = nullptr;
		};

		struct DLLNETWORK ParentTypeMetaData : public TypeMetaData {
			pragma::GString parentProperty = nullptr;
		};
	};
	class BaseEntityComponent;
	enum class AttributeSpecializationType : uint8_t;
	enum class ComponentMemberFlags : uint32_t;

	struct DLLNETWORK ComponentMemberInfo {
		struct DLLNETWORK EnumConverter {
			using NameToEnumFunction = std::function<std::optional<int64_t>(const std::string &)>;
			using EnumToNameFunction = std::function<std::optional<std::string>(int64_t)>;
			using EnumValueGetFunction = std::function<std::vector<int64_t>()>;
			NameToEnumFunction nameToEnum;
			EnumToNameFunction enumToName;
			EnumValueGetFunction getValues;
			EnumConverter() = default;
			EnumConverter(NameToEnumFunction nameToEnum, EnumToNameFunction enumToName, EnumValueGetFunction getValues) : nameToEnum(std::move(nameToEnum)), enumToName(std::move(enumToName)), getValues(std::move(getValues)) {}
		};
		using ApplyFunction = void (*)(const ComponentMemberInfo &, BaseEntityComponent &, const void *);
		using GetFunction = void (*)(const ComponentMemberInfo &, BaseEntityComponent &, void *);
		using InterpolationFunction = void (*)(const void *, const void *, double, void *);
		using UpdateDependenciesFunction = void (*)(BaseEntityComponent &, std::vector<std::string> &);
		static ComponentMemberInfo CreateDummy();
		ComponentMemberInfo(const char *name, ents::EntityMemberType type, const ApplyFunction &applyFunc, const GetFunction &getFunc);
		ComponentMemberInfo(const std::string &name, ents::EntityMemberType type, const ApplyFunction &applyFunc, const GetFunction &getFunc);

		template<typename TComponent, typename T, void (*TApply)(const ComponentMemberInfo &, TComponent &, const T &)>
		void SetSetterFunction()
		{
			setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) { TApply(memberInfo, static_cast<TComponent &>(component), *static_cast<const T *>(value)); };
		}
		template<typename TComponent, typename T, void (*TApply)(const ComponentMemberInfo &, TComponent &, T)>
		void SetSetterFunction()
		{
			setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) { TApply(memberInfo, static_cast<TComponent &>(component), *static_cast<const T *>(value)); };
		}
		template<typename TComponent, typename T, void (TComponent::*TApply)(const T &)>
		void SetSetterFunction()
		{
			setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) { (static_cast<TComponent &>(component).*TApply)(*static_cast<const T *>(value)); };
		}
		template<typename TComponent, typename T, void (TComponent::*TApply)(T)>
		void SetSetterFunction()
		{
			setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) { (static_cast<TComponent &>(component).*TApply)(*static_cast<const T *>(value)); };
		}

		template<typename TComponent, typename T, void (*TGetter)(const ComponentMemberInfo &, TComponent &, T &)>
		void SetGetterFunction()
		{
			getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) { TGetter(memberInfo, static_cast<TComponent &>(component), *static_cast<T *>(value)); };
		}
		template<typename TComponent, typename T, auto(*TGetter)()>
		void SetGetterFunction()
		{
			getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) { *static_cast<T *>(value) = TGetter(); };
		}
		template<typename TComponent, typename T, auto(TComponent::*TGetter)()>
		void SetGetterFunction()
		{
			getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) { *static_cast<T *>(value) = (static_cast<TComponent &>(component).*TGetter)(); };
		}
		template<typename TComponent, typename T, auto(TComponent::*TGetter)() const>
		void SetGetterFunction()
		{
			getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) { *static_cast<T *>(value) = (static_cast<TComponent &>(component).*TGetter)(); };
		}

#if defined(__GNUC__) && !defined(__clang__)
		template<typename TComponent, typename T, bool (TComponent::*TGetter)() const>
		void SetGetterFunction()
		{
			getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) { *static_cast<T *>(value) = (static_cast<TComponent &>(component).*TGetter)(); };
		}
#endif

		template<typename TComponent, typename T, void (*TInterp)(const T &, const T &, double, T &)>
		void SetInterpolationFunction()
		{
			interpolationFunction = [](const void *v0, const void *v1, double t, void *out) { TInterp(*static_cast<const T *>(v0), *static_cast<const T *>(v1), t, *static_cast<T *>(out)); };
		}

		void SetName(const pragma::GString &name);
		const pragma::GString &GetName() const { return m_name; }
		size_t GetNameHash() const { return m_nameHash; }

		AttributeSpecializationType GetSpecializationType() const { return m_specializationType; }
		const std::string *GetCustomSpecializationType() const { return m_customSpecializationType.get(); }
		void SetSpecializationType(AttributeSpecializationType type);
		void SetSpecializationType(std::string customType);

		ComponentMemberInfo(const ComponentMemberInfo &);
		ComponentMemberInfo &operator=(const ComponentMemberInfo &other);

		void SetMin(float min);
		void SetMax(float max);
		void SetStepSize(float stepSize);
		std::optional<float> GetMin() const;
		std::optional<float> GetMax() const;
		std::optional<float> GetStepSize() const;
		udm::Property &AddMetaData();
		void AddMetaData(const udm::PProperty &prop);
		const udm::PProperty &GetMetaData() const;
		void AddTypeMetaData(const std::shared_ptr<ents::TypeMetaData> &typeMetaData);
		const ents::TypeMetaData *FindTypeMetaData(std::type_index typeId) const;
		template<class T>
		T &AddTypeMetaData();
		template<class T>
		const T *FindTypeMetaData() const;
		template<typename T>
		bool GetDefault(T &outValue) const;
		template<typename T>
		void SetDefault(T value);

		void SetEnum(const EnumConverter::NameToEnumFunction &nameToEnum, const EnumConverter::EnumToNameFunction &enumToName, const EnumConverter::EnumValueGetFunction &getValues);
		bool IsEnum() const;
		std::optional<int64_t> EnumNameToValue(const std::string &name) const;
		std::optional<std::string> ValueToEnumName(int64_t value) const;
		bool GetEnumValues(std::vector<int64_t> &outValues) const;

		void UpdateDependencies(BaseEntityComponent &component, std::vector<std::string> &outAffectedProps);
		void ResetToDefault(BaseEntityComponent &component);

		void SetFlags(ComponentMemberFlags flags);
		ComponentMemberFlags GetFlags() const;
		bool HasFlag(ComponentMemberFlags flag) const;
		void SetFlag(ComponentMemberFlags flag, bool set = true);

		ents::EntityMemberType type;
		ApplyFunction setterFunction = nullptr;
		GetFunction getterFunction = nullptr;
		InterpolationFunction interpolationFunction = nullptr;
		UpdateDependenciesFunction updateDependenciesFunction = nullptr;

		union {
			uint64_t userIndex;
			void *userData = nullptr;
		};
	  private:
		ComponentMemberInfo();
		pragma::GString m_name = "";
		size_t m_nameHash = 0;
		ComponentMemberFlags m_flags = static_cast<ComponentMemberFlags>(0);

		AttributeSpecializationType m_specializationType;
		std::unique_ptr<std::string> m_customSpecializationType = nullptr;

		udm::PProperty m_metaData = nullptr;
		std::vector<std::shared_ptr<ents::TypeMetaData>> m_typeMetaData;
		std::unique_ptr<void, void (*)(void *)> m_default = std::unique_ptr<void, void (*)(void *)> {nullptr, [](void *) {}};
		std::unique_ptr<EnumConverter> m_enumConverter = nullptr;
	};
};

template<class T>
T &pragma::ComponentMemberInfo::AddTypeMetaData()
{
	auto *data = const_cast<ComponentMemberInfo *>(this)->FindTypeMetaData<T>();
	if(data)
		return *const_cast<T *>(data);
	auto newData = std::shared_ptr<ents::TypeMetaData>(new T {});
	AddTypeMetaData(newData);
	return *static_cast<T *>(newData.get());
}

template<class T>
const T *pragma::ComponentMemberInfo::FindTypeMetaData() const
{
	auto *data = FindTypeMetaData(typeid(T));
	if(!data)
		return nullptr;
	return static_cast<const T *>(data);
}

#endif
