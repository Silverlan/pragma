// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.json_skin;

export import pragma.lua;
export import pragma.gui;

namespace pragma::gui {
	using JsonData = const void*;

	template<typename T>
	concept is_json_type = std::is_same_v<T, udm::Boolean> || std::is_same_v<T, udm::Double> || std::is_same_v<T, udm::Int64> || std::is_same_v<T, udm::Nil> || std::is_same_v<T, udm::String> || std::is_same_v<T, udm::UInt64>;
	template<typename T>
	concept is_valid_json_udm_type = udm::is_ng_type(udm::type_to_enum<T>()) || std::is_same_v<T, udm::String>;

	udm::Type get_json_udm_type(JsonData j);
	bool is_json_array(JsonData j);
	size_t get_json_array_size(JsonData j);
	JsonData get_json_array_value(JsonData j, size_t index);
	template<typename T> requires(is_json_type<T>)
	T get_json_value(JsonData j);

	template<typename T> requires(pragma::gui::is_valid_json_udm_type<T>)
	std::optional<T> get_property_value(JsonData j)
	{
		auto jtype = get_json_udm_type(j);
		if (jtype == udm::Type::Invalid)
			return {};
		return udm::visit(jtype, [&j](auto tag) -> std::optional<T> {
			using TJson = typename decltype(tag)::type;
			if constexpr(pragma::gui::is_json_type<TJson> && pragma::gui::is_valid_json_udm_type<T> && !std::is_same_v<T, udm::Nil> && !std::is_same_v<TJson, udm::Nil> && udm::is_convertible<TJson, T>())
			{
				auto val = get_json_value<TJson>(j);
				return udm::convert<TJson, T>(val);
			}
			else
				return {};
		});
	}

	export template<typename T> requires(pragma::gui::is_valid_json_udm_type<T>)
	std::optional<std::vector<T>> get_array_values(JsonData j)
	{
		if (!is_json_array(j))
			return {};
		auto size = get_json_array_size(j);
		std::vector<T> values;
		values.reserve(size);
		for (size_t i=0;i<size;++i)
		{
			auto jval = get_json_array_value(j, i);
			auto val = get_property_value<T>(jval);
			if (!val)
				continue;
			values.push_back(*val);
		}
		return values;
	}

	export struct DLLCLIENT StyleProperty
	{
		std::string_view name;
		JsonData property = nullptr;
		void LogInvalid() const;
		std::optional<StyleProperty> FindProperty(std::string_view name) const;

		template<typename T> requires(is_valid_json_udm_type<T>)
		std::optional<T> GetValue() const
		{
			return get_property_value<T>(property);
		}
		template<typename T> requires(is_valid_json_udm_type<T>)
		std::optional<T> GetValue(std::string_view name) const
		{
			auto childProp = FindProperty(name);
			if (!childProp)
				return {};
			return childProp->GetValue<T>();
		}

		template<typename T> requires(pragma::gui::is_valid_json_udm_type<T>)
		std::optional<std::vector<T>> GetArrayValues() const
		{
			return get_array_values<T>(property);
		}
		template<typename T> requires(pragma::gui::is_valid_json_udm_type<T>)
		std::optional<std::vector<T>> GetArrayValues(std::string_view name) const
		{
			auto childProp = FindProperty(name);
			if (!childProp)
				return {};
			return childProp->GetArrayValues<T>();
		}
		std::optional<std::vector<StyleProperty>> GetArrayProperties() const;
		std::optional<std::vector<StyleProperty>> GetArrayProperties(std::string_view name) const;
	};

	class DLLCLIENT JsonSkinBase;
	export class DLLCLIENT JsonSkin : public WISkin
	{
	public:
		JsonSkin();
		void Initialize(types::WIBase *el) override;
		void Release(types::WIBase *el) override;

		void Load(JsonSkin *base, JsonData data);
		JsonSkinBase *GetBaseJsonSkin() { return m_baseJsonSkin.get(); }
	private:
		std::shared_ptr<JsonSkinBase> m_baseJsonSkin;
	};
};
