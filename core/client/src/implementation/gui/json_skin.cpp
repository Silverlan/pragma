// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <glaze/glaze.hpp>

module pragma.client;

import :gui.json_skin;

import :client_state;

static spdlog::logger &LOGGER = pragma::register_logger("gui");

namespace pragma::gui
{
	struct DLLCLIENT JsonSkinClass {
		JsonSkinClass() = default;
		JsonSkinClass(const JsonSkinClass &) = delete;
		JsonSkinClass &operator=(const JsonSkinClass &) = delete;

		string::StringMap<glz::generic_u64> properties;
		string::StringMap<std::unique_ptr<JsonSkinClass>> children;
		string::StringMap<std::unique_ptr<JsonSkinClass>> states;
		string::StringMap<std::unique_ptr<JsonSkinClass>> decorators;
		std::vector<std::string> inherits;

		JsonSkinClass *Copy() const;

		void MergeFrom(const JsonSkinClass &baseClass);
	};

	udm::Type get_json_udm_type(JsonData jptr)
	{
		auto &j = *static_cast<const glz::generic_u64 *>(jptr);
		if (j.is_boolean())
			return udm::Type::Boolean;
		if (j.is_double())
			return udm::Type::Double;
		if (j.is_int64())
			return udm::Type::Int64;
		if (j.is_null())
			return udm::Type::Nil;
		if (j.is_string())
			return udm::Type::String;
		if (j.is_uint64())
			return udm::Type::UInt64;
		return udm::Type::Invalid;
	}

	class DLLCLIENT JsonSkinBase {
	  public:
		struct Settings {
			JsonSkin *base = nullptr;
			glz::generic_u64 jsonData;
		};

		struct Style {
			string::OrderedStringMap<glz::generic_u64> properties;
			string::OrderedStringMap<Style> children;
			string::OrderedStringMap<Style> decorators;

			std::optional<StyleProperty> FindProperty(std::string_view name) const;
			template<typename T> requires(is_valid_json_udm_type<T>)
			std::optional<T> GetPropertyValue(std::string_view name) const
			{
				auto prop = FindProperty(name);
				if (!prop)
					return {};
				auto val = prop->GetValue<T>();
				if (!val)
				{
					prop->LogInvalid();
					return {};
				}
				return val;
			}
			template<typename T> requires(is_valid_json_udm_type<T>)
			std::optional<std::vector<T>> GetArrayPropertyValues(std::string_view name) const
			{
				auto prop = FindProperty(name);
				if (!prop)
					return {};
				auto values = prop->GetArrayValues<T>();
				if (!values)
				{
					prop->LogInvalid();
					return {};
				}
				return values;
			}
			void LogInvalidProperty(std::string_view name) const
			{
				auto prop = FindProperty(name);
				if (!prop)
					return;
				prop->LogInvalid();
			}
		};
	  protected:
		JsonSkinClass m_rootClass;
		std::unordered_map<std::string, glz::generic_u64> m_constants;

		void ParseConstants(const glz::generic_u64 &j);
		void ParseFonts(const glz::generic_u64 &j);
		void ParseClass(const glz::generic_u64 &j, JsonSkinClass &outClass, bool rootLevel = false);

		void ResolveVariables(JsonSkinClass &cl);
		void ResolveVariables(string::StringMap<glz::generic_u64> &properties);
		void ResolveMixins(JsonSkinClass &target, const JsonSkinClass &root);
		void MergeBaseSkin(const JsonSkinClass &baseRoot, JsonSkinClass &cl);
	  public:
		JsonSkinBase();

		void Load(const Settings &settings);

		std::optional<Style> ComputeStyle(types::WIBase &el) const;
		void Initialize(types::WIBase *el);
		void ApplyStyle(types::WIBase &el, const Style &style) const;
	};

	static const glz::generic_u64 &get_json_data(JsonData json)
	{
		return *static_cast<const glz::generic_u64*>(json);
	}
}

pragma::gui::JsonSkinClass *pragma::gui::JsonSkinClass::Copy() const
{
	JsonSkinClass *other = new JsonSkinClass();
	other->properties = properties;
	other->inherits = inherits;

	for(const auto &[key, child] : children)
		other->children.insert({key, std::unique_ptr<JsonSkinClass>(child->Copy())});
	for(const auto &[key, state] : states)
		other->states.insert({key, std::unique_ptr<JsonSkinClass>(state->Copy())});
	for(const auto &[key, decorator] : decorators)
		other->decorators.insert({key, std::unique_ptr<JsonSkinClass>(decorator->Copy())});
	return other;
}

void pragma::gui::JsonSkinClass::MergeFrom(const JsonSkinClass &baseClass)
{
	for(const auto &[key, val] : baseClass.properties) {
		if(properties.find(key) == properties.end())
			properties[key] = val;
	}

	for(const auto &[key, baseChild] : baseClass.children) {
		auto it = children.find(key);
		if(it == children.end())
			children[key] = std::unique_ptr<JsonSkinClass>(baseChild->Copy());
		else
			it->second->MergeFrom(*baseChild);
	}

	for(const auto &[key, baseState] : baseClass.states) {
		auto it = states.find(key);
		if(it == states.end())
			states[key] = std::unique_ptr<JsonSkinClass>(baseState->Copy());
		else
			it->second->MergeFrom(*baseState);
	}

	for(const auto &[key, baseDecorator] : baseClass.decorators) {
		auto it = decorators.find(key);
		if(it == decorators.end())
			decorators[key] = std::unique_ptr<JsonSkinClass>(baseDecorator->Copy());
		else
			it->second->MergeFrom(*baseDecorator);
	}
}

pragma::gui::JsonSkin::JsonSkin() : WISkin {}, m_baseJsonSkin{std::make_unique<JsonSkinBase>()}
{}
void pragma::gui::JsonSkin::Initialize(types::WIBase *el)
{
	WISkin::Initialize(el);
	m_baseJsonSkin->Initialize(el);
}
void pragma::gui::JsonSkin::Release(types::WIBase *el)
{
	WISkin::Release(el);
}
void pragma::gui::JsonSkin::Load(JsonSkin *base, JsonData data)
{
	m_baseJsonSkin->Load(JsonSkinBase::Settings{base,*static_cast<const glz::generic_u64*>(data)});
}

pragma::gui::JsonSkinBase::JsonSkinBase() {}

void pragma::gui::JsonSkinBase::Load(const Settings &settings)
{
	const auto &j = settings.jsonData;

	auto *baseSkin = settings.base;

	// Note: Make sure to skip these in "ParseClass" when adding new root blocks
	if(j.contains("theme")) {
		auto theme = j["theme"];

		if (theme.contains("inherits"))
		{
			auto inherits = theme["inherits"];
			if (inherits.is_string())
			{
				auto baseTheme = inherits.get_string();
				baseSkin = dynamic_cast<JsonSkin*>(WGUI::GetInstance().GetSkin(baseTheme));
			}
		}

		ParseConstants(theme);
		ParseFonts(theme);
	}

	ParseClass(j, m_rootClass, true);

	if(baseSkin) {
		auto *baseJsonSkin = baseSkin->GetBaseJsonSkin();
		for(const auto &[k, v] : baseJsonSkin->m_constants)
			m_constants.insert({k, v});
		MergeBaseSkin(baseJsonSkin->m_rootClass, m_rootClass);
	}

	ResolveVariables(m_rootClass);
	ResolveMixins(m_rootClass, m_rootClass);
}

static void resolve_property(const std::unordered_map<std::string, glz::generic_u64> &constants, glz::generic_u64 &val) {
	if(val.is_string()) {
		auto strVal = val.get<std::string>();
		if(!strVal.empty() && strVal[0] == '$') {
			auto varName = strVal.substr(1);
			auto it = constants.find(varName);
			if(it != constants.end())
				val = it->second;
		}
	}
	else if(val.is_object()) {
		for(auto &[key, child] : val.get_object())
			resolve_property(constants, child);
	}
	else if(val.is_array()) {
		for(auto &child : val.get_array())
			resolve_property(constants, child);
	}
}

void pragma::gui::JsonSkinBase::ParseConstants(const glz::generic_u64 &j)
{
	if(j.contains("constants") && j["constants"].is_object()) {
		for(const auto &[key, val] : j["constants"].get_object())
			m_constants[key] = val;

		for(auto &[key, val] : m_constants)
			resolve_property(m_constants, val);
	}
}

void pragma::gui::JsonSkinBase::ParseFonts(const glz::generic_u64 &j)
{
	if(!j.contains("fonts") || !j["fonts"].is_object())
		return;

	for(const auto &[fontName, fontData] : j["fonts"].get_object()) {
		std::string fontSet = fontData.contains("fontSet") ? fontData["fontSet"].get<std::string>() : "default";

		if(!fontSet.empty() && fontSet[0] == '$') {
			auto it = m_constants.find(fontSet.substr(1));
			if(it != m_constants.end())
				fontSet = it->second.get<std::string>();
		}

		if(fontSet == "default")
			fontSet = get_cengine()->GetDefaultFontSetName();

		int32_t size = fontData.contains("size") ? fontData["size"].as<double>() : 16;

		auto featureFlags = FontSetFlag::None;
		if(fontData.contains("flags") && fontData["flags"].is_array()) {
			for(const auto &flagVal : fontData["flags"].get_array()) {
				auto flag = flagVal.get<std::string>();

				if(flag == "sans")
					featureFlags |= FontSetFlag::Sans;
				else if(flag == "mono")
					featureFlags |= FontSetFlag::Mono;
				else if(flag == "bold")
					featureFlags |= FontSetFlag::Bold;
				else if(flag == "italic")
					featureFlags |= FontSetFlag::Italic;
				else if(flag == "serif")
					featureFlags |= FontSetFlag::Serif;
			}
		}

		Lua::engine::create_font(nullptr, fontName, fontSet, featureFlags, size, false);
	}
}

void pragma::gui::JsonSkinBase::ParseClass(const glz::generic_u64 &j, JsonSkinClass &outClass, bool rootLevel)
{
	if(!j.is_object())
		return;

	if(rootLevel) {
		for(const auto &[key, value] : j.get_object()) {
			if(key == "constants" || key == "fonts")
				continue; // Already handled
			if(value.is_object()) {
				outClass.children[key] = std::make_unique<JsonSkinClass>();
				ParseClass(value, *outClass.children[key]);
			}
		}
		return;
	}

	for(const auto &[key, value] : j.get_object()) {
		if(key == "inherits") {
			if(value.is_string())
				outClass.inherits.push_back(value.get<std::string>());
			else if(value.is_array()) {
				for(const auto &val : value.get_array()) {
					if(val.is_string())
						outClass.inherits.push_back(val.get<std::string>());
				}
			}
		}
		else if(key == "states") {
			if(value.is_object()) {
				for(const auto &[stateKey, stateVal] : value.get_object()) {
					outClass.states[stateKey] = std::make_unique<JsonSkinClass>();
					ParseClass(stateVal, *outClass.states[stateKey]);
				}
			}
		}
		else if(key == "decorators") {
			if(value.is_object()) {
				for(const auto &[decKey, decVal] : value.get_object()) {
					outClass.decorators[decKey] = std::make_unique<JsonSkinClass>();
					ParseClass(decVal, *outClass.decorators[decKey]);
				}
			}
		}
		else if(key == "children") {
			if(value.is_object()) {
				for(const auto &[key, child] : value.get_object()) {
					outClass.children[key] = std::make_unique<JsonSkinClass>();
					ParseClass(child, *outClass.children[key]);
				}
			}
		}
		else
			outClass.properties[key] = value;
	}
}

void pragma::gui::JsonSkinBase::ResolveVariables(string::StringMap<glz::generic_u64> &properties)
{
	for(auto &[key, val] : properties)
		resolve_property(m_constants, val);
}

void pragma::gui::JsonSkinBase::ResolveVariables(JsonSkinClass &cl)
{
	ResolveVariables(cl.properties);

	for(auto &[key, child] : cl.children)
		ResolveVariables(*child);
	for(auto &[key, state] : cl.states)
		ResolveVariables(*state);
	for(auto &[key, decorator] : cl.decorators)
		ResolveVariables(*decorator);
}

void pragma::gui::JsonSkinBase::ResolveMixins(JsonSkinClass &target, const JsonSkinClass &root)
{
	for(const std::string &baseName : target.inherits) {
		auto it = root.children.find(baseName);
		if(it != root.children.end())
			target.MergeFrom(*it->second);
	}

	for(auto &[key, child] : target.children)
		ResolveMixins(*child, root);
	for(auto &[key, state] : target.states)
		ResolveMixins(*state, root);
	for(auto &[key, decorator] : target.decorators)
		ResolveMixins(*decorator, root);
}

void pragma::gui::JsonSkinBase::MergeBaseSkin(const JsonSkinClass &baseRoot, JsonSkinClass &cl) { cl.MergeFrom(baseRoot); }

namespace pragma::gui {
	struct SkinContext {
		JsonSkinClass *skinClass;
		int specificity;

		// Sort by specificity
		bool operator<(const SkinContext &other) const { return specificity < other.specificity; }
	};
	JsonSkinClass *find_skin_class(std::string_view className, const string::StringMap<std::unique_ptr<JsonSkinClass>> &classes)
	{
		auto it = classes.find(className);
		return (it != classes.end()) ? it->second.get() : nullptr;
	}
	void find_skin_classes(types::WIBase *el, const string::StringMap<std::unique_ptr<JsonSkinClass>> &classes, std::vector<SkinContext> &outClasses, int baseSpecificity = 0)
	{
		// By Class (specificity = 1)
		auto className = el->GetClass();
		if(!className.empty()) {
			auto *cl = find_skin_class(className, classes);
			if(cl != nullptr)
				outClasses.push_back({cl, baseSpecificity + 1});
		}

		// Style Classes (specificity = 10)
		auto &styleClasses = el->GetStyleClasses();
		for(auto &styleClass : styleClasses) {
			auto *cl = find_skin_class("." + std::string {styleClass}, classes);
			if(cl != nullptr)
				outClasses.push_back({cl, baseSpecificity + 10});
		}

		// By element name (specificity = 100)
		auto &name = el->GetName();
		if(!name.empty()) {
			auto *cl = find_skin_class("#" + name, classes);
			if(cl != nullptr)
				outClasses.push_back({cl, baseSpecificity + 100});
		}
	}
}

static std::string_view get_state_name(pragma::gui::InputState state)
{
	switch(state) {
	case pragma::gui::InputState::Hover:
		return "hover";
	case pragma::gui::InputState::Pressed:
		return "pressed";
	case pragma::gui::InputState::Focused:
		return "focused";
	}
	// TODO: InputState::Active?
	static_assert(pragma::math::to_integral(pragma::gui::InputState::Count) == 4, "Update this switch-case when new states are added.");
	return "";
}

std::optional<pragma::gui::JsonSkinBase::Style> pragma::gui::JsonSkinBase::ComputeStyle(types::WIBase &el) const
{
	std::vector<types::WIBase *> els;
	auto *parent = &el;
	while(parent != nullptr) {
		els.push_back(parent);
		parent = parent->GetParent();
	}

	// Initialize the root with 0 specificity
	std::vector<SkinContext> classes = {{const_cast<JsonSkinClass *>(&m_rootClass), 0}};
	for(auto &el_iter : els | std::views::reverse) {
		auto numClasses = classes.size();
		for(size_t i = 0; i < numClasses; ++i) // Note: Loop modifies 'classes'
			find_skin_classes(el_iter, classes[i].skinClass->children, classes, classes[i].specificity);
	}

	std::vector<SkinContext> elClassesContexts;
	for(auto &c : classes)
		find_skin_classes(&el, c.skinClass->children, elClassesContexts, c.specificity);

	// Sort rules by specificity
	std::stable_sort(elClassesContexts.begin(), elClassesContexts.end());

	// To list
	std::vector<JsonSkinClass *> elClasses;
	elClasses.reserve(elClassesContexts.size());
	for(auto &c : elClassesContexts)
		elClasses.push_back(c.skinClass);

	if(elClasses.empty())
		return {};

	uint32_t styledStatesMask = 0;
	for(auto *cl : elClasses) {
		if(cl->states.empty())
			continue;
		for(size_t i = 0; i < math::to_integral(InputState::Count); ++i) {
			auto sName = get_state_name(static_cast<InputState>(i));
			if(!sName.empty() && cl->states.find(sName) != cl->states.end())
				styledStatesMask |= (1 << i);
		}
	}
	el.SetStyledStatesMask(styledStatesMask);

	auto currentStateStr = get_state_name(el.GetInputState());

	Style style;
	for(auto *cl : elClasses) {
		// Base properties
		for(const auto &[name, prop] : cl->properties)
			style.properties[name] = prop;

		// Property overrides from active state (e.g. hover)
		JsonSkinClass *activeState = nullptr;
		if(!currentStateStr.empty()) {
			auto stateIt = cl->states.find(currentStateStr);
			if(stateIt != cl->states.end()) {
				activeState = stateIt->second.get();
				for(const auto &[name, prop] : activeState->properties)
					style.properties[name] = prop;
			}
		}

		auto mergeSubItems = [](const auto &baseMap, const auto *stateMap, string::OrderedStringMap<Style> &targetMap) {
			// Base properties
			for(const auto &[name, baseItem] : baseMap) {
				auto &targetItem = targetMap[name];
				for(const auto &[pName, pVal] : baseItem->properties)
					targetItem.properties[pName] = pVal;
			}

			// Apply states
			if(stateMap) {
				for(const auto &[name, stateItem] : *stateMap) {
					auto &targetItem = targetMap[name];
					for(const auto &[pName, pVal] : stateItem->properties)
						targetItem.properties[pName] = pVal;
				}
			}
		};

		mergeSubItems(cl->children, activeState ? &activeState->children : nullptr, style.children);
		mergeSubItems(cl->decorators, activeState ? &activeState->decorators : nullptr, style.decorators);
	}
	return style;
}

/*template<typename T> requires(is_valid_json_udm_type<T>)
static std::optional<T> get_property_value(const pragma::string::OrderedStringMap<glz::generic_u64> &props, std::string_view propName)
{
	auto it = props.find(propName);
	if (it == props.end())
		return {};
	return get_property_value<T>(it->second);
}*/

static const glz::generic_u64 *find_property(const pragma::string::OrderedStringMap<glz::generic_u64> &props, std::string_view propName)
{
	auto it = props.find(propName);
	if (it == props.end())
		return nullptr;
	return &it->second;
}

std::optional<pragma::gui::StyleProperty> pragma::gui::JsonSkinBase::Style::FindProperty(std::string_view name) const
{
	auto *prop = find_property(properties, name);
	if (!prop)
		return {};
	return StyleProperty {name, static_cast<JsonData>(prop)};
}

static void log_invalid_property(const glz::generic_u64 &prop, std::string_view propertyName)
{
	auto propertyValue = prop.as<std::string>();
	LOGGER.warn("Invalid value for property '{}': {}", propertyName, propertyValue);
}

void pragma::gui::StyleProperty::LogInvalid() const
{
	log_invalid_property(get_json_data(property), name);
}

std::optional<pragma::gui::StyleProperty> pragma::gui::StyleProperty::FindProperty(std::string_view name) const
{
	auto &j = get_json_data(property);
	if (!j.is_object())
		return {};
	auto &o = j.get_object();
	auto it = o.find(name);
	if (it == o.end())
		return {};
	return StyleProperty {name, &it->second};
}

std::optional<std::vector<pragma::gui::StyleProperty>> pragma::gui::StyleProperty::GetArrayProperties() const
{
	auto &j = get_json_data(property);
	if (!j.is_array())
		return {};
	auto &a = j.get_array();
	std::vector<pragma::gui::StyleProperty> childProps {};
	childProps.reserve(a.size());
	for (auto &child : a)
		childProps.push_back({{}, &child});
	return childProps;
}
std::optional<std::vector<pragma::gui::StyleProperty>> pragma::gui::StyleProperty::GetArrayProperties(std::string_view name) const
{
	auto childProp = FindProperty(name);
	if (!childProp)
		return {};
	return childProp->GetArrayProperties();
}

static std::optional<pragma::gui::Alignment> to_alignment_enum(std::string_view alignmentStr)
{
	using namespace pragma::string::string_switch;
	switch(hash(alignmentStr)) {
	case "none"_:
		return pragma::gui::Alignment::None;
	case "start"_:
		return pragma::gui::Alignment::Start;
	case "center"_:
		return pragma::gui::Alignment::Center;
	case "end"_:
		return pragma::gui::Alignment::End;
	case "fill"_:
		return pragma::gui::Alignment::Fill;
	}
	return {};
}

static std::optional<pragma::platform::Cursor::Shape> to_cursor_enum(std::string_view cursorStr)
{
	using namespace pragma::string::string_switch;
	switch(hash(cursorStr)) {
	case "hidden"_:
		return pragma::platform::Cursor::Shape::Hidden;
	case "arrow"_:
		return pragma::platform::Cursor::Shape::Arrow;
	case "ibeam"_:
		return pragma::platform::Cursor::Shape::IBeam;
	case "crosshair"_:
		return pragma::platform::Cursor::Shape::Crosshair;
	case "hand"_:
		return pragma::platform::Cursor::Shape::Hand;
	case "hresize"_:
		return pragma::platform::Cursor::Shape::HResize;
	case "vresize"_:
		return pragma::platform::Cursor::Shape::VResize;
	}
	return {};
}

static std::optional<pragma::gui::types::FlexAlign> to_flex_align_enum(std::string_view alignStr)
{
	using namespace pragma::string::string_switch;
	switch(hash(alignStr)) {
	case "start"_:
		return pragma::gui::types::FlexAlign::Start;
	case "center"_:
		return pragma::gui::types::FlexAlign::Center;
	case "end"_:
		return pragma::gui::types::FlexAlign::End;
	case "stretch"_:
		return pragma::gui::types::FlexAlign::Stretch;
	}
	return {};
}

static std::optional<pragma::gui::types::FlexJustify> to_flex_justify_enum(std::string_view alignStr)
{
	using namespace pragma::string::string_switch;
	switch(hash(alignStr)) {
	case "start"_:
		return pragma::gui::types::FlexJustify::Start;
	case "center"_:
		return pragma::gui::types::FlexJustify::Center;
	case "end"_:
		return pragma::gui::types::FlexJustify::End;
	case "spaceBetween"_:
		return pragma::gui::types::FlexJustify::SpaceBetween;
	case "spaceEvenly"_:
		return pragma::gui::types::FlexJustify::SpaceEvenly;
	}
	return {};
}

static bool is_match(pragma::gui::types::WIBase &child, std::string_view matchName)
{
	if (child.HasStyleClass(matchName))
		return true;
	auto &childName = child.GetName();
	if (childName == matchName)
		return true;
	auto childClass = child.GetClass();
	return childName.length() == childClass.length() && pragma::string::compare(childName.c_str(), childClass.str, false, childName.length());
}

static void collect_children(pragma::gui::types::WIBase &currentParent, std::string_view matchName, bool isDirectOnly, std::vector<pragma::gui::types::WIBase*> &results)
{
	auto &children = *currentParent.GetChildren();
	for (auto &hChild : children)
	{
		if (!hChild.IsValid())
			continue;
		auto &child = *hChild;
		if (is_match(child, matchName))
		{
			if (results.size() == results.capacity())
				results.reserve(results.size() *2 +10);
			results.push_back(&child);
		}
		if (!isDirectOnly)
			collect_children(child, matchName, isDirectOnly, results);
	}
}

static std::vector<pragma::gui::types::WIBase*> find_matching_children(pragma::gui::types::WIBase &parent, std::string_view selector)
{
	auto isDirectOnly = false;
	std::string matchName {selector};

	// ">" can be used to target direct child only
	if (!matchName.empty() && matchName.front() == '>')
	{
		isDirectOnly = true;
		// Strip '>' and trim
		matchName.erase(matchName.begin());
		pragma::string::remove_whitespace(matchName);
	}

	std::vector<pragma::gui::types::WIBase*> results;
	collect_children(parent, matchName, isDirectOnly, results);
	return results;
}

static std::optional<Color> string_to_color(std::string_view strColor)
{
	if (strColor.empty() || strColor.front() != '#')
		return {};
	return Color::CreateFromHexColor(std::string{strColor.substr(1)});
}

static std::optional<Color> get_property_color(const pragma::gui::StyleProperty &colorProp)
{
	if (auto colorStr = colorProp.GetValue<udm::String>(); colorStr)
	{
		auto color = string_to_color(*colorStr);
		if (color)
			return color;
	}
	else if (auto a = colorProp.GetArrayValues<udm::UInt8>(); a)
	{
		std::array<uint8_t, 4> color {255, 255, 255, 255};
		for (size_t i=0;i<pragma::math::min(a->size(), static_cast<size_t>(4));++i)
			color[i] = (*a)[i];
		return Color {color[0], color[1], color[2], color[3]};
	}
	colorProp.LogInvalid();
	return {};
}

static std::array<int32_t, 4> unpack_spacing(const pragma::gui::StyleProperty &prop)
{
	std::array<int32_t, 4> result {0, 0, 0, 0};
	if (auto a = prop.GetArrayValues<udm::Int32>(); a)
	{
		for (size_t i=0;i<pragma::math::min(a->size(), static_cast<size_t>(4));++i)
			result[i] = (*a)[i];
	}
	else if (auto n = prop.GetValue<udm::Int32>(); n)
	{
		for (auto &v : result)
			v = *n;
	}
	else
		prop.LogInvalid();
	return result;
}

struct DimensionInfo
{
	enum class Type : uint8_t
	{
		Undefined = 0,
		Value,
		Percentage,
		Auto
	};
	float value = 0.f;
	Type type = Type::Undefined;
};

static DimensionInfo parse_dimension(const pragma::gui::StyleProperty &prop)
{
	if (auto str = prop.GetValue<udm::String>(); str)
	{
		if (*str == "auto")
			return {{},DimensionInfo::Type::Auto};
		if (!str->empty() && str->front() == '%')
		{
			auto pc = str->substr(1);
			return {pragma::string::to_int(pc) /100.f, DimensionInfo::Type::Percentage};
		}
		return {pragma::string::to_int(*str), DimensionInfo::Type::Value};
	}
	if (auto val = prop.GetValue<udm::Int32>(); val)
		return {*val, DimensionInfo::Type::Value};
	return {0, DimensionInfo::Type::Undefined};
}

static void apply_dimensions(pragma::gui::types::WIBase &el, const pragma::gui::JsonSkinBase::Style &style)
{
	DimensionInfo x {};
	if (auto xprop = style.FindProperty("x"); xprop)
		x = parse_dimension(*xprop);
	DimensionInfo y {};
	if (auto yprop = style.FindProperty("y"); yprop)
		y = parse_dimension(*yprop);
	DimensionInfo w {};
	if (auto wprop = style.FindProperty("width"); wprop)
		w = parse_dimension(*wprop);
	DimensionInfo h {};
	if (auto hprop = style.FindProperty("height"); hprop)
		h = parse_dimension(*hprop);

	auto origSize = el.GetSize();
	auto applyDimension = [&el](pragma::gui::Axis axis, const DimensionInfo &pos, const DimensionInfo &size, pragma::gui::Anchor::Edge edge0, pragma::gui::Anchor::Edge edge1)
	{
		if (pos.type == DimensionInfo::Type::Percentage)
		{
			el.ApplyDimensionPos(axis, 0);
			el.SetAnchor(edge0, pos.value);
			el.SetAnchorEdgeEnabled(edge0, true);
		}
		else if (pos.type == DimensionInfo::Type::Value)
			el.ApplyDimensionPos(axis, pos.value);

		std::optional<std::array<float, 4>> anchor {}; // TODO
		if (size.type == DimensionInfo::Type::Percentage)
		{
			// To make % width work dynamically, we need a left anchor to base the right anchor on
			float baseLeft = 0.f;
			if (pos.type == DimensionInfo::Type::Percentage)
				baseLeft = pos.value;
			else if (anchor)
				baseLeft = (*anchor)[0];

			auto *parent = el.GetParent();
			if (parent)
				el.ApplyDimensionSize(axis, pragma::math::round(parent->GetWidth() *size.value));

			if (pos.type != DimensionInfo::Type::Percentage && !anchor) {
				el.SetAnchor(edge0, baseLeft);
				el.SetAnchorEdgeEnabled(edge0, true);
			}
			// Set Right anchor dynamically
			el.SetAnchor(edge1, baseLeft +size.value);
			el.SetAnchorEdgeEnabled(edge1, true);
		}
		else if (size.type == DimensionInfo::Type::Value)
			el.ApplyDimensionSize(axis, size.value);
	};
	applyDimension(pragma::gui::Axis::X, x, w, pragma::gui::Anchor::Edge::Left, pragma::gui::Anchor::Edge::Right);
	applyDimension(pragma::gui::Axis::Y, y, h, pragma::gui::Anchor::Edge::Top, pragma::gui::Anchor::Edge::Bottom);

	auto wIsAuto = (w.type == DimensionInfo::Type::Auto);
	auto hIsAuto = (h.type == DimensionInfo::Type::Auto);
	if (wIsAuto != hIsAuto)
	{
		auto aspectRatio = (origSize.y > 0) ? (origSize.x /static_cast<float>(origSize.y)) : 0.f;
		if (wIsAuto)
		{
			auto w = el.GetHeight() *aspectRatio;
			el.ApplyWidth(w);
		}
		else
		{
			auto h = (aspectRatio > 0.0001f) ? (el.GetWidth() /aspectRatio) : 0.f;
			el.ApplyHeight(h);
		}
	}
}

static void apply_style(pragma::gui::types::WIText &el, const pragma::gui::JsonSkinBase::Style &style)
{
	if (auto font = style.GetPropertyValue<udm::String>("font"); font)
		el.SetFont(*font);
	if (auto text = style.GetPropertyValue<udm::String>("text"); text)
		el.SetText(*text);
	if (auto textTransform = style.GetPropertyValue<udm::String>("textTransform"); textTransform)
	{
		if (*textTransform == "uppercase")
		{
			auto text = el.GetText();
			text.toUpper();
			el.SetText(text);
		}
		else if (*textTransform == "lowercase")
		{
			auto text = el.GetText();
			text.toLower();
			el.SetText(text);
		}
		else if (*textTransform == "capitalize")
		{
			LOGGER.warn("'capitalize' text transform currently not implemented!");
			/*auto text = el.GetText();
			auto capitalizeNext = true;
			for (size_t i=0;i<text.size();++i) {
				auto &c = text[i];
				if (pragma::string::WHITESPACE.find(c) != std::string::npos)
					capitalizeNext = true;
				else if (capitalizeNext)
				{
					capitalizeNext = false;
					// TODO: This is not a pretty way of handling it
					pragma::string::Utf8String cstr {c};
					cstr.toUpper();
					if (!cstr.empty())
						c = cstr.front();
				}
			}*/
		}
		else
			style.LogInvalidProperty("textTransform");
	}

	if (auto shadowOffset = style.GetArrayPropertyValues<udm::Int32>("shadowOffset"); shadowOffset)
	{
		if (shadowOffset->size() == 2)
		{
			if (auto shadowColor = style.FindProperty("shadowColor"); shadowColor)
			{
				auto color = get_property_color(*shadowColor);
				if (color)
					el.SetShadowColor(*color);
			}

			el.SetShadowOffset({(*shadowOffset)[0], (*shadowOffset)[1]});
			el.EnableShadow(true);
		}
		else
			style.LogInvalidProperty("shadowOffset");
	}
}

static void apply_style(pragma::gui::types::WITexturedRect &el, const pragma::gui::JsonSkinBase::Style &style)
{
	if (auto material = style.GetPropertyValue<udm::String>("material"); material)
	{
		el.SetMaterial(*material);
		el.SizeToTexture();
	}
}

static void apply_style(pragma::gui::types::FlexBox &el, const pragma::gui::JsonSkinBase::Style &style)
{
	if (auto alignItems = style.GetPropertyValue<udm::String>("alignItems"); alignItems)
	{
		if (auto flexAlignEnum = to_flex_align_enum(*alignItems); flexAlignEnum)
			el.SetAlignItems(*flexAlignEnum);
		else
			style.LogInvalidProperty("alignItems");
	}
	if (auto justifyContent = style.GetPropertyValue<udm::String>("justifyContent"); justifyContent)
	{
		if (auto justifyContentEnum = to_flex_justify_enum(*justifyContent); justifyContentEnum)
			el.SetJustifyContent(*justifyContentEnum);
		else
			style.LogInvalidProperty("justifyContent");
	}

	if (auto paddingProp = style.FindProperty("padding"); paddingProp)
	{
		auto padding = unpack_spacing(*paddingProp);
		el.SetPadding(padding[0], padding[1], padding[2], padding[3]);
	}

	if (auto spacing = style.GetPropertyValue<udm::Int32>("spacing"); spacing)
		el.SetSpacing(*spacing);
}

struct AlignInfo
{
	struct EdgeInfo
	{
		std::string_view identifier;
		std::string_view marginIdentifier;
		pragma::gui::Anchor::Edge edge;
	};
	EdgeInfo start;
	EdgeInfo end;
	std::string_view offsetIdentifier;
	bool horizontal;
};

static void apply_anchor_align(pragma::gui::types::WIBase &el, const pragma::gui::JsonSkinBase::Style &style, const AlignInfo &alignInfo, const std::string &alignValue, bool marginHandled)
{
	auto &start = alignInfo.start;
	auto &end = alignInfo.end;

	std::optional<int32_t> marginStart {};
	std::optional<int32_t> marginEnd {};
	if (marginHandled)
	{
		marginStart = 0;
		marginEnd = 0;
	}
	else
	{
		marginStart = style.GetPropertyValue<udm::Int32>(start.marginIdentifier);
		marginEnd = style.GetPropertyValue<udm::Int32>(end.marginIdentifier);
	}

	if (alignValue == start.identifier)
	{
		el.SetAnchor(start.edge, 0.f);
		el.SetAnchor(end.edge, 0.f);
		el.SetAnchorEdgeEnabled(start.edge, true);
		el.SetAnchorOffset(start.edge, marginStart ? *marginStart : 0);
	}
	else if (alignValue == end.identifier)
	{
		el.SetAnchor(start.edge, 1.f);
		el.SetAnchor(end.edge, 1.f);
		el.SetAnchorEdgeEnabled(end.edge, true);
		el.SetAnchorOffset(end.edge, marginEnd ? -*marginEnd : 0);
	}
	else if (alignValue == "center")
	{
		auto offset = style.GetPropertyValue<udm::Int32>(alignInfo.offsetIdentifier);
		if (alignInfo.horizontal)
			el.SetAnchorHorizontalCenter(offset ? *offset : 0);
		else
			el.SetAnchorVerticalCenter(offset ? *offset : 0);
	}
	else if (alignValue == "stretch")
	{
		el.SetAnchor(start.edge, 0.f);
		el.SetAnchor(end.edge, 1.f);
		el.SetAnchorEdgeEnabled(start.edge, true);
		el.SetAnchorEdgeEnabled(end.edge, true);
		el.SetAnchorOffset(start.edge, marginStart ? *marginStart : 0);
		el.SetAnchorOffset(end.edge, marginEnd ? -*marginEnd : 0);
	}
}

static void apply_background(pragma::gui::types::WIBase &el, const pragma::gui::JsonSkinBase::Style &style)
{
	struct BackgroundInfo
	{
		std::optional<Color> color {};
		std::optional<udm::Int32> borderThickness {};
		std::optional<Color> borderColor {};
		std::optional<std::array<udm::Int32, 4>> cornerRadii {};
		std::optional<pragma::gui::StyleProperty> gradient {};

		operator bool() const
		{
			return color || borderThickness || borderColor || cornerRadii || gradient;
		}
	};
	BackgroundInfo bgInfo {};
	if (auto bgColorProp = style.FindProperty("backgroundColor"); bgColorProp)
		bgInfo.color = get_property_color(*bgColorProp);

	if (auto bg = style.GetPropertyValue<udm::String>("background"); bg)
	{
		auto color = string_to_color(*bg);
		if (color)
			bgInfo.color = color;
		else
			style.LogInvalidProperty("background");
	}

	if (auto background = style.FindProperty("background"); background)
	{
		bgInfo.borderThickness = background->GetValue<udm::Int32>("borderThickness");
		if (bgInfo.borderThickness)
		{
			if (auto borderColorProp = background->FindProperty("borderColor"); borderColorProp)
				bgInfo.borderColor = get_property_color(*borderColorProp);
		}
		if (auto cornerRadiiProp = background->FindProperty("cornerRadii"); cornerRadiiProp)
			bgInfo.cornerRadii = unpack_spacing(*cornerRadiiProp);
		if (auto colorProp = background->FindProperty("color"); colorProp)
			bgInfo.color = get_property_color(*colorProp);
		bgInfo.gradient = background->FindProperty("gradient");
	}

	auto *bg = el.GetFirstChildByName("skin_background");
	if (bgInfo)
	{
		auto simpleBackground = !bgInfo.borderThickness && !bgInfo.cornerRadii && !bgInfo.gradient;
		std::string_view bgClass = simpleBackground ? "wirect" : "styled_rect";
		if (bg && pragma::string::compare(std::string_view{bg->GetClass()}, bgClass, false))
		{
			bg->Remove();
			bg = nullptr;
		}

		if (!bg)
		{
			bg = pragma::gui::WGUI::GetInstance().Create(std::string{bgClass}, &el);
			if (bg)
			{
				bg->SetName("skin_background");
				bg->SetBackgroundElement(true);
				bg->SetZPos(-100);
				bg->SetAlignment(pragma::gui::Alignment::Fill);
			}
		}

		if (bg)
		{
			bg->SetVisible(true);
			if (bgInfo.color)
				bg->SetColor(*bgInfo.color);

			if (auto *bgStyled = dynamic_cast<pragma::gui::types::StyledRect*>(bg); !simpleBackground && bgStyled)
			{
				if (bgInfo.borderThickness)
				{
					bgStyled->SetBorderThickness(*bgInfo.borderThickness);
					if (bgInfo.borderColor)
						bgStyled->SetBorderColor(*bgInfo.borderColor);
				}

				if (bgInfo.cornerRadii)
				{
					auto &radii = *bgInfo.cornerRadii;
					bgStyled->SetCornerRadii(Vector4{radii[0], radii[1], radii[2], radii[3]});
				}

				bgStyled->ClearGradient();
				if (bgInfo.gradient)
				{
					auto gradientType = pragma::gui::types::StyledRect::GradientType::Linear;
					if (auto gradientTypeStr = bgInfo.gradient->GetValue<udm::String>("type"); gradientTypeStr)
					{
						if (gradientTypeStr == "linear")
							gradientType = pragma::gui::types::StyledRect::GradientType::Linear;
						else if (gradientTypeStr == "radial")
								gradientType = pragma::gui::types::StyledRect::GradientType::Radial;
						else
							bgInfo.gradient->LogInvalid();
					}
					bgStyled->SetGradientType(gradientType);

					if (auto start = bgInfo.gradient->GetArrayValues<udm::Float>("start"); start)
					{
						if (start->size() == 2)
							bgStyled->SetGradientStart(::Vector2{(*start)[0], (*start)[1]});
						else
							bgInfo.gradient->LogInvalid();
					}
					if (auto end = bgInfo.gradient->GetArrayValues<udm::Float>("end"); end)
					{
						if (end->size() == 2)
							bgStyled->SetGradientEnd(::Vector2{(*end)[0], (*end)[1]});
						else
							bgInfo.gradient->LogInvalid();
					}
					if (auto stops = bgInfo.gradient->GetArrayProperties("stops"); stops)
					{
						std::vector<pragma::gui::types::StyledRect::ColorStop> colorStops;
						colorStops.reserve(stops->size());
						for (auto &stop : *stops)
						{
							auto pos = stop.GetValue<udm::Float>("pos");
							auto colorStr = stop.GetValue<udm::String>("color");
							auto color = colorStr ? string_to_color(*colorStr) : std::optional<Color>{};
							if (!pos || !color)
							{
								stop.LogInvalid();
								continue;
							}
							colorStops.push_back(pragma::gui::types::StyledRect::ColorStop {*color, *pos});
						}
						bgStyled->SetGradientStops(colorStops);
					}
				}
			}
		}
	}
	else if (bg)
		bg->SetVisible(false);
}

static void apply_outline(pragma::gui::types::WIBase &el, const pragma::gui::JsonSkinBase::Style &style)
{
	auto *ol = el.GetFirstChildByName("skin_outline");
	if (auto outlineWidth = style.GetPropertyValue<udm::Int32>("outlineWidth"); outlineWidth && *outlineWidth > 0)
	{
		if (!ol)
		{
			ol = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WIOutlinedRect>(&el);
			ol->SetName("skin_outline");
			ol->SetBackgroundElement(true);
			ol->SetZPos(10);
			ol->SetAlignment(pragma::gui::Alignment::Fill);
		}
		static_cast<pragma::gui::types::WIOutlinedRect*>(ol)->SetOutlineWidth(*outlineWidth);
		if (auto outlineColorProp = style.FindProperty("outlineColor"); outlineColorProp)
		{
			auto color = get_property_color(*outlineColorProp);
			if (color)
				ol->SetColor(*color);
		}
		ol->SetVisible(true);
	}
	else if (ol)
		ol->SetVisible(false);
}

void pragma::gui::JsonSkinBase::ApplyStyle(types::WIBase &el, const Style &style) const
{
	if (auto colorProp = style.FindProperty("color"); colorProp)
	{
		auto color = get_property_color(*colorProp);
		if (color)
			el.SetColor(*color);
	}
	// Are these still needed?
	// if (style.fixedWidth or style.width) and element.SetFixedWidth then element:SetFixedWidth(true) end
	// if (style.fixedHeight or style.height) and element.SetFixedHeight then element:SetFixedHeight(true) end
	if (auto visible = style.GetPropertyValue<udm::Boolean>("visible"); visible)
		el.SetVisible(*visible);
	if (auto zIndex = style.GetPropertyValue<udm::Int32>("zIndex"); zIndex)
		el.SetZPos(*zIndex);
	if (auto cursor = style.GetPropertyValue<udm::String>("cursor"); cursor)
	{
		if (auto cursorEnum = to_cursor_enum(*cursor); cursorEnum)
			el.SetCursor(*cursorEnum);
		else
			style.LogInvalidProperty("cursor");
	}

	// TODO: Make these virtual methods in the GUI library
	if (auto *p = dynamic_cast<types::WIText*>(&el); p)
		apply_style(*p, style);
	if (auto *p = dynamic_cast<types::WITexturedRect*>(&el); p)
		apply_style(*p, style);
	if (auto *p = dynamic_cast<types::FlexBox*>(&el); p)
		apply_style(*p, style);

	/*if (auto alignment = style.GetPropertyValue<udm::String>("alignment"); alignment) {
		auto alignmentEnum = to_alignment_enum(*alignment);
		if (alignmentEnum)
			el.SetAlignment(*alignmentEnum);
		else
			log_invalid_property(*property, name);
	}*/
	/*
	 *if style.alignment then element:SetAlignment(to_alignment_enum(style.alignment)) end
if style.alignmentX then element:SetHorizontalAlignment(to_alignment_enum(style.alignmentX)) end
if style.alignmentY then element:SetVerticalAlignment(to_alignment_enum(style.alignmentY)) end
if style.horizontalAlignment then element:SetHorizontalAlignment(to_alignment_enum(style.horizontalAlignment)) end
if style.verticalAlignment then element:SetVerticalAlignment(to_alignment_enum(style.verticalAlignment)) end
*/

	if (auto minWidth = style.GetPropertyValue<udm::Int32>("minWidth"); minWidth)
		el.SetMinWidth(*minWidth);
	if (auto maxWidth = style.GetPropertyValue<udm::Int32>("maxWidth"); maxWidth)
		el.SetMaxWidth(*maxWidth);
	if (auto minHeight = style.GetPropertyValue<udm::Int32>("minHeight"); minHeight)
		el.SetMinHeight(*minHeight);
	if (auto maxHeight = style.GetPropertyValue<udm::Int32>("maxHeight"); maxHeight)
		el.SetMinHeight(*maxHeight);

	apply_dimensions(el, style);

	auto marginHandled = false;
	if (auto *parent = dynamic_cast<types::FlexBox*>(el.GetParent()); parent)
	{
		std::array<int32_t, 4> margin {0, 0, 0, 0};
		auto marginProp = style.FindProperty("margin");
		if (marginProp)
			margin = unpack_spacing(*marginProp);

		if (auto marginLeft = style.GetPropertyValue<udm::Int32>("marginLeft"); marginLeft)
			margin[0] = *marginLeft;
		if (auto marginRight = style.GetPropertyValue<udm::Int32>("marginRight"); marginRight)
			margin[1] = *marginRight;
		if (auto marginTop = style.GetPropertyValue<udm::Int32>("marginTop"); marginTop)
			margin[2] = *marginTop;
		if (auto marginBottom = style.GetPropertyValue<udm::Int32>("marginBottom"); marginBottom)
			margin[3] = *marginBottom;
		parent->SetChildMargin(el, margin[0], margin[1], margin[2], margin[3]);
		marginHandled = true;
	}

	auto alignX = style.GetPropertyValue<udm::String>("alignX");
	auto alignY = style.GetPropertyValue<udm::String>("alignY");
	if (alignX || alignY)
	{
		el.ClearAnchor();

		if (alignX)
		{
			constexpr AlignInfo alignInfoX {
				{"left", "marginLeft", Anchor::Edge::Left},
				{"right", "marginRight", Anchor::Edge::Right},
				"offsetX", true
			};
			apply_anchor_align(el, style, alignInfoX, *alignX, marginHandled);
		}

		if (alignY)
		{
			constexpr AlignInfo alignInfoY {
				{"top", "marginTop", Anchor::Edge::Top},
				{"bottom", "marginBottom", Anchor::Edge::Bottom},
				"offsetY", false
			};
			apply_anchor_align(el, style, alignInfoY, *alignY, marginHandled);
		}

		el.UpdateAnchorTransform();
	}

	if (auto flex = style.GetPropertyValue<udm::Float>("flex"); flex)
	{
		auto *parent = dynamic_cast<types::FlexBox*>(el.GetParent());
		if (parent)
		{
			parent->SetChildFlex(el, *flex);
			if (parent->IsHorizontalBox())
				parent->SetFixedWidth(true);
			if (parent->IsVerticalBox())
				parent->SetFixedHeight(true);
		}
	}

	apply_background(el, style);
	apply_outline(el, style);

	if (auto sound = style.GetPropertyValue<udm::String>("sound"); sound)
	{
		// TODO
		// -- Ensure we only play the sound once
		// if element.__lastSkinSound ~= style.sound then
		if (auto *cl = get_client_state(); cl)
		{
			if (auto snd = cl->CreateSound(*sound, audio::ALSoundType::GUI, audio::ALCreateFlags::Mono); snd)
			{
				snd->SetRelative(true);
				snd->SetPitch(0.8f); // TODO: Let user define via skin
				snd->Play();
			}
		}
	}

	for (auto &[selector, childStyle] : style.children)
	{
		auto matchingChildren = find_matching_children(el, selector);
		for (auto &child : matchingChildren)
		{
			// TODO: Do we need this here?
			/*if childStyle.margin then
				local immediateParent = child:GetParent()
				if immediateParent and immediateParent.SetChildMargin then
					local l, t, r, b = unpack_spacing(childStyle.margin)
					immediateParent:SetChildMargin(child, l, t, r, b)
				end
			end*/
			ApplyStyle(*child, childStyle);
		}
	}

	for (auto &[decName, decStyle] : style.decorators)
	{
		std::string internalName = "skin_dec_" +decName;
		auto *decEl = el.GetFirstChildByName(internalName);
		if (!decEl)
		{
			auto type = decStyle.GetPropertyValue<udm::String>("type");
			if (type)
			{
				decEl = WGUI::GetInstance().Create(*type, &el);
				if (decEl)
				{
					decEl->SetName(internalName);
					decEl->SetBackgroundElement(true, false);
					decEl->SetZPos(-80);
				}
				else
					LOGGER.warn("Failed to create decorator of type '{}'.", *type);
			}
			else
				LOGGER.warn("Failed to create decorator: No decorator type specified!");
		}

		if (decEl)
		{
			decEl->SetVisible(true);
			ApplyStyle(*decEl, decStyle);
		}
	}
}

void pragma::gui::JsonSkinBase::Initialize(types::WIBase *el)
{
	auto style = ComputeStyle(*el);
	if(!style)
		return;

	auto *l = get_client_state()->GetGUILuaState();
	auto apply_style_class = luabind::object {l, luabind::globals(l)["gui"]};
	if(apply_style_class)
		apply_style_class = apply_style_class["apply_style_class"];
	if(!apply_style_class)
		throw std::runtime_error {"Unable to apply style classes: Could not find Lua function 'apply_style_class'."};

	// To Lua table
	auto applyPropsToTable = [&](this auto &self, const auto &propsMap, luabind::object &targetTable) -> void {
		for(const auto &[name, prop] : propsMap) {
			if(prop.is_string())
				targetTable[name] = prop.template get<std::string>();
			else if(prop.is_number())
				targetTable[name] = prop.template as<double>();
			else if(prop.is_boolean())
				targetTable[name] = prop.template get<bool>() ? true : false;
			else if(prop.is_object()) {
				auto subTable = luabind::newtable(l);
				self(prop.get_object(), subTable);
				targetTable[name] = subTable;
			}
			else if(prop.is_array()) {
				auto tProp = luabind::newtable(l);
				size_t tableIdx = 1;
				for(auto [idx, val] : std::views::enumerate(prop.get_array())) {
					if(val.is_string())
						tProp[tableIdx++] = val.template get<std::string>();
					else if(val.is_number())
						tProp[tableIdx++] = val.template as<double>();
					else if(val.is_boolean())
						tProp[tableIdx++] = val.template get<bool>() ? true : false;
					else if(val.is_object()) {
						auto subTable = luabind::newtable(l);
						self(val.get_object(), subTable);
						tProp[tableIdx++] = subTable;
					}
					else if(val.is_array())
						; // TODO?
					else if(val.is_null())
						tProp[tableIdx++] = luabind::nil;
				}
				targetTable[name] = tProp;
			}
			else if(prop.is_null())
				targetTable[name] = luabind::nil;
		}
	};

	auto buildLuaTable = [&](this auto &self, const Style &style) -> luabind::object {
		auto targetTable = luabind::newtable(l);

		applyPropsToTable(style.properties, targetTable);

		if(!style.children.empty()) {
			auto tChildren = luabind::newtable(l);
			for(const auto &[name, child] : style.children)
				tChildren[name] = self(child);
			targetTable["children"] = tChildren;
		}

		if(!style.decorators.empty()) {
			auto tDecorators = luabind::newtable(l);
			for(const auto &[name, decorator] : style.decorators)
				tDecorators[name] = self(decorator);
			targetTable["decorators"] = tDecorators;
		}

		return targetTable;
	};

	ApplyStyle(*el, *style);

	auto t = buildLuaTable(*style);
	apply_style_class(WGUILuaInterface::GetLuaObject(l, *el), t);
}

namespace pragma::debug {
	std::string get_element_info(pragma::gui::types::WIBase &el);
	gui::types::WIBase *find_gui_element(std::vector<std::string> &argv);
}

static void debug_print_style(pragma::gui::types::WIBase &el)
{
	auto *skin = dynamic_cast<pragma::gui::JsonSkin *>(el.GetSkin());
	if(!skin)
	{
		Con::CWAR<<"Element has no valid skin."<<Con::endl;
		return;
	}
	auto style = skin->GetBaseJsonSkin()->ComputeStyle(el);
	if(!style)
	{
		Con::CWAR<<"Failed to compute style for element!"<<Con::endl;
		return;
	}

	auto print_style = [](this auto &self, const auto &s, int indent, std::ostream &os) -> void {
		std::string ind(indent * 4, ' ');
		std::string child_ind((indent + 1) * 4, ' ');

		for(const auto &[name, prop] : s.properties) {
			std::string prop_str;
			glz::write_json(prop, prop_str);
			os << ind << name << ": " << prop_str << "\n";
		}

		if(!s.children.empty()) {
			os << ind << "children: {\n";
			for(const auto &[name, child] : s.children) {
				os << child_ind << name << ": {\n";
				self(child, indent + 2, os);
				os << child_ind << "}\n";
			}
			os << ind << "}\n";
		}

		if(!s.decorators.empty()) {
			os << ind << "decorators: {\n";
			for(const auto &[name, decorator] : s.decorators) {
				os << child_ind << name << ": {\n";
				self(decorator, indent + 2, os);
				os << child_ind << "}\n";
			}
			os << ind << "}\n";
		}
	};

	std::stringstream ss;
	ss << "Computed Style for element \n" << pragma::debug::get_element_info(el) << "\n";
	ss << "Style classes:\n";
	auto className = el.GetClass();
	ss << className << "\n";
	for(auto &styleClass : el.GetStyleClasses())
		ss << "." << styleClass << "\n";

	auto &name = el.GetName();
	if(!name.empty())
		ss << "#" << name << "\n";

	ss<<"\nComputed:\n";
	ss << "{\n";
	print_style(*style, 1, ss);
	ss << "}\n";

	Con::COUT << ss.str() << Con::endl;
}

static void debug_gui_style(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);

namespace {
	auto UVN = pragma::console::client::register_command("debug_gui_style", &debug_gui_style, pragma::console::ConVarFlags::None, "Prints the effective style data for the specified GUI element.");
}

static void debug_gui_style(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto *el = pragma::debug::find_gui_element(argv);
	if(!el)
		return;
	debug_print_style(*el);
}
