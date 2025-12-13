// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.member_info;

export import :entities.enums;
import :entities.member_type;
export import :util.global_string_table;

#ifdef _WIN32
// Due to a compiler bug with c++20 modules in msvc, we have to wrap the getter
// and setter functions in lambdas to avoid compiler errors for now.
#define USE_LAMBDA_GETTER_SETTER_WRAPPERS
#endif

export {
	namespace pragma {
		namespace ents {
			struct DLLNETWORK TypeMetaData {
				virtual ~TypeMetaData() = default;
			};
			struct DLLNETWORK RangeTypeMetaData : public TypeMetaData {
				std::optional<float> min {};
				std::optional<float> max {};
				std::optional<float> stepSize {};
			};

			struct DLLNETWORK CoordinateTypeMetaData : public TypeMetaData {
				math::CoordinateSpace space = math::CoordinateSpace::World;
				GString parentProperty = nullptr;
			};

			struct DLLNETWORK PoseTypeMetaData : public TypeMetaData {
				GString posProperty = nullptr;
				GString rotProperty = nullptr;
				GString scaleProperty = nullptr;
			};

			struct DLLNETWORK PoseComponentTypeMetaData : public TypeMetaData {
				GString poseProperty = nullptr;
			};

			struct DLLNETWORK OptionalTypeMetaData : public TypeMetaData {
				GString enabledProperty = nullptr;
			};

			struct DLLNETWORK EnablerTypeMetaData : public TypeMetaData {
				GString targetProperty = nullptr;
			};

			struct DLLNETWORK ParentTypeMetaData : public TypeMetaData {
				GString parentProperty = nullptr;
			};
		};
		class BaseEntityComponent;

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
#ifdef USE_LAMBDA_GETTER_SETTER_WRAPPERS
				setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) {
					auto *setterFunc = TApply;
					setterFunc(memberInfo, static_cast<TComponent &>(component), *static_cast<const T *>(value));
				};
#else
				setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) { TApply(memberInfo, static_cast<TComponent &>(component), *static_cast<const T *>(value)); };
#endif
			}
			template<typename TComponent, typename T, void (*TApply)(const ComponentMemberInfo &, TComponent &, T)>
			void SetSetterFunction()
			{
#ifdef USE_LAMBDA_GETTER_SETTER_WRAPPERS
				setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) {
					auto *setterFunc = TApply;
					setterFunc(memberInfo, static_cast<TComponent &>(component), *static_cast<const T *>(value));
				};
#else
				setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) { TApply(memberInfo, static_cast<TComponent &>(component), *static_cast<const T *>(value)); };
#endif
			}
			template<typename TComponent, typename T, void (TComponent::*TApply)(const T &)>
			void SetSetterFunction()
			{
#ifdef USE_LAMBDA_GETTER_SETTER_WRAPPERS
				setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) {
					auto &comp = static_cast<TComponent &>(component);
					auto setterFunc = [&comp](const T &v) { (comp.*TApply)(v); };
					setterFunc(*static_cast<const T *>(value));
				};
#else
				setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) { (static_cast<TComponent &>(component).*TApply)(*static_cast<const T *>(value)); };
#endif
			}
			template<typename TComponent, typename T, void (TComponent::*TApply)(T)>
			void SetSetterFunction()
			{
#ifdef USE_LAMBDA_GETTER_SETTER_WRAPPERS
				setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) {
					auto &comp = static_cast<TComponent &>(component);
					auto setterFunc = TApply;
					(comp.*setterFunc)(*static_cast<const T *>(value));
				};
#else
				setterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, const void *value) { (static_cast<TComponent &>(component).*TApply)(*static_cast<const T *>(value)); };
#endif
			}

			template<typename TComponent, typename T, void (*TGetter)(const ComponentMemberInfo &, TComponent &, T &)>
			inline void SetGetterFunction()
			{
#ifdef USE_LAMBDA_GETTER_SETTER_WRAPPERS
				getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) {
					auto *getterFunc = TGetter;
					getterFunc(memberInfo, static_cast<TComponent &>(component), *static_cast<T *>(value));
				};
#else
				getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) { TGetter(memberInfo, static_cast<TComponent &>(component), *static_cast<T *>(value)); };
#endif
			}
			template<typename TComponent, typename T, auto (*TGetter)()>
			inline void SetGetterFunction()
			{
#ifdef USE_LAMBDA_GETTER_SETTER_WRAPPERS
				getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) {
					auto *getterFunc = TGetter;
					*static_cast<T *>(value) = getterFunc();
				};
#else
				getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) { *static_cast<T *>(value) = TGetter(); };
#endif
			}
			template<typename TComponent, typename T, auto (TComponent::*TGetter)()>
			inline void SetGetterFunction()
			{
#ifdef USE_LAMBDA_GETTER_SETTER_WRAPPERS
				getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) {
					auto &comp = static_cast<TComponent &>(component);
					auto getterFunc = TGetter;
					*static_cast<T *>(value) = (comp.*getterFunc)();
				};
#else
				getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) { *static_cast<T *>(value) = (static_cast<TComponent &>(component).*TGetter)(); };
#endif
			}
			template<typename TComponent, typename T, auto (TComponent::*TGetter)() const>
			inline void SetGetterFunction()
			{
#ifdef USE_LAMBDA_GETTER_SETTER_WRAPPERS
				getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) {
					auto &comp = static_cast<const TComponent &>(component); // call const getter
					auto getterFunc = TGetter;
					*static_cast<T *>(value) = (comp.*getterFunc)();
				};
#else
				getterFunction = [](const ComponentMemberInfo &memberInfo, BaseEntityComponent &component, void *value) { *static_cast<T *>(value) = (static_cast<TComponent &>(component).*TGetter)(); };
#endif
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
#ifdef USE_LAMBDA_GETTER_SETTER_WRAPPERS
				interpolationFunction = [](const void *v0, const void *v1, double t, void *out) {
					auto *lerpFunc = TInterp;
					lerpFunc(*static_cast<const T *>(v0), *static_cast<const T *>(v1), t, *static_cast<T *>(out));
				};
#else
				interpolationFunction = [](const void *v0, const void *v1, double t, void *out) { TInterp(*static_cast<const T *>(v0), *static_cast<const T *>(v1), t, *static_cast<T *>(out)); };
#endif
			}

			void SetName(const GString &name);
			const GString &GetName() const { return m_name; }
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
			GString m_name = "";
			size_t m_nameHash = 0;
			ComponentMemberFlags m_flags = static_cast<ComponentMemberFlags>(0);

			AttributeSpecializationType m_specializationType;
			std::unique_ptr<std::string> m_customSpecializationType = nullptr;

			udm::PProperty m_metaData = nullptr;
			std::vector<std::shared_ptr<ents::TypeMetaData>> m_typeMetaData;
			std::unique_ptr<void, void (*)(void *)> m_default = std::unique_ptr<void, void (*)(void *)> {nullptr, [](void *) {}};
			std::unique_ptr<EnumConverter> m_enumConverter = nullptr;
		};

		template<class T>
		T &ComponentMemberInfo::AddTypeMetaData()
		{
			auto *data = const_cast<ComponentMemberInfo *>(this)->FindTypeMetaData<T>();
			if(data)
				return *const_cast<T *>(data);
			auto newData = std::shared_ptr<ents::TypeMetaData>(new T {});
			AddTypeMetaData(newData);
			return *static_cast<T *>(newData.get());
		}

		template<class T>
		const T *ComponentMemberInfo::FindTypeMetaData() const
		{
			auto *data = FindTypeMetaData(typeid(T));
			if(!data)
				return nullptr;
			return static_cast<const T *>(data);
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
	}
};
