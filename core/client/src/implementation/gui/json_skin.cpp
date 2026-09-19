// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <glaze/glaze.hpp>

module pragma.client;

import :gui.json_skin;

import :client_state;

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

pragma::gui::JsonSkin::JsonSkin() : WISkin {} {}

void pragma::gui::JsonSkin::Load(const Settings &settings)
{
	const auto &j = settings.jsonData;

	ParseConstants(j);
	ParseFonts(j);

	ParseClass(j, m_rootClass);
	ResolveVariables(m_rootClass);
	ResolveMixins(m_rootClass, m_rootClass);

	if(settings.base != nullptr) {
		for(const auto &[k, v] : settings.base->m_constants)
			m_constants.insert({k, v});
		MergeBaseSkin(settings.base->m_rootClass, m_rootClass);
	}
}

void pragma::gui::JsonSkin::ParseConstants(const glz::json_t &j)
{
	if(j.contains("constants") && j["constants"].is_object()) {
		for(const auto &[key, val] : j["constants"].get_object()) {
			if(val.is_string())
				m_constants[key] = val.get<std::string>();
		}
	}
}

void pragma::gui::JsonSkin::ParseFonts(const glz::json_t &j)
{
	if(!j.contains("fonts") || !j["fonts"].is_object())
		return;

	for(const auto &[fontName, fontData] : j["fonts"].get_object()) {
		std::string fontSet = fontData.contains("fontSet") ? fontData["fontSet"].get<std::string>() : "default";

		if(!fontSet.empty() && fontSet[0] == '$') {
			auto it = m_constants.find(fontSet.substr(1));
			if(it != m_constants.end())
				fontSet = it->second;
		}

		if(fontSet == "default")
			fontSet = get_cengine()->GetDefaultFontSetName();

		int32_t size = fontData.contains("size") ? fontData["size"].get<double>() : 16;

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

void pragma::gui::JsonSkin::ParseClass(const glz::json_t &j, JsonSkinClass &outClass)
{
	if(!j.is_object())
		return;

	for(const auto &[key, value] : j.get_object()) {
		if(key == "constants")
			continue; // Already handled
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
		else if(value.is_object()) {
			outClass.children[key] = std::make_unique<JsonSkinClass>();
			ParseClass(value, *outClass.children[key]);
		}
		else
			outClass.properties[key] = value;
	}
}

void pragma::gui::JsonSkin::ResolveVariables(JsonSkinClass &cl)
{
	for(auto &[key, val] : cl.properties) {
		if(val.is_string()) {
			auto strVal = val.get<std::string>();
			if(!strVal.empty() && strVal[0] == '$') {
				auto varName = strVal.substr(1);
				auto it = m_constants.find(varName);
				if(it != m_constants.end())
					val = it->second;
			}
		}
	}

	for(auto &[key, child] : cl.children)
		ResolveVariables(*child);
	for(auto &[key, state] : cl.states)
		ResolveVariables(*state);
	for(auto &[key, decorator] : cl.decorators)
		ResolveVariables(*decorator);
}

void pragma::gui::JsonSkin::ResolveMixins(JsonSkinClass &target, const JsonSkinClass &root)
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

void pragma::gui::JsonSkin::MergeBaseSkin(const JsonSkinClass &baseRoot, JsonSkinClass &cl) { cl.MergeFrom(baseRoot); }

namespace pragma::gui {
	JsonSkinClass *find_skin_class(const std::string &className, const string::StringMap<std::unique_ptr<JsonSkinClass>> &classes)
	{
		auto it = classes.find(className);
		return (it != classes.end()) ? it->second.get() : nullptr;
	}
	void find_skin_classes(types::WIBase *el, const string::StringMap<std::unique_ptr<JsonSkinClass>> &classes, std::vector<JsonSkinClass *> &outClasses)
	{
		auto &styleClasses = el->GetStyleClasses();
		for(auto &styleClass : styleClasses) {
			auto *cl = find_skin_class(styleClass, classes);
			if(cl != nullptr)
				outClasses.push_back(cl);
		}
	}
}

static std::string get_state_name(pragma::gui::InputState state)
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

void pragma::gui::JsonSkin::Initialize(types::WIBase *el)
{
	WISkin::Initialize(el);

	std::vector<types::WIBase *> els;
	auto *parent = el;
	while(parent != nullptr) {
		els.push_back(parent);
		parent = parent->GetParent();
	}
	std::vector classes = {&m_rootClass};
	for(auto &el : els | std::views::reverse) {
		auto numClasses = classes.size();
		for(size_t i = 0; i < numClasses; ++i) // Note: Loop modifies 'classes', so we need to iterate using indices
			find_skin_classes(el, classes[i]->children, classes);
	}

	std::vector<JsonSkinClass *> elClasses;
	for(auto &c : classes)
		find_skin_classes(el, c->children, elClasses);
	auto *l = get_client_state()->GetGUILuaState();
	auto apply_style_class = luabind::object {l, luabind::globals(l)["gui"]};
	if(apply_style_class)
		apply_style_class = apply_style_class["apply_style_class"];
	if(!apply_style_class)
		throw std::runtime_error {"Unable to apply style classes: Could not find Lua function 'apply_style_class'."};

	auto currentStateStr = get_state_name(el->GetInputState());
	for(auto *cl : elClasses) {
		auto t = luabind::newtable(l);

		// Convert json properties to Lua table
		auto applyPropsToTable = [&](const auto &propsMap, luabind::object &targetTable) {
			for(const auto &[name, prop] : propsMap) {
				if(prop.is_string())
					targetTable[name] = prop.template get<std::string>();
				else if(prop.is_number())
					targetTable[name] = prop.template get<double>();
				else if(prop.is_boolean())
					targetTable[name] = prop.template get<bool>() ? true : false;
				else if(prop.is_object())
					; // TODO: nested object
				else if(prop.is_array()) {
					auto tProp = luabind::newtable(l);
					for(auto [idx, val] : std::views::enumerate(prop.get_array())) {
						if(val.is_string())
							tProp[idx + 1] = val.template get<std::string>();
						else if(val.is_number())
							tProp[idx + 1] = val.template get<double>();
						else if(val.is_boolean())
							tProp[idx + 1] = val.template get<bool>() ? true : false;
						else if(val.is_object())
							; // TODO?
						else if(val.is_array())
							; // TODO?
						else if(val.is_null())
							tProp[idx + 1] = luabind::nil;
					}
					targetTable[name] = tProp;
				}
				else if(prop.is_null())
					targetTable[name] = luabind::nil;
			}
		};

		// Base properties
		applyPropsToTable(cl->properties, t);

		// Property overrides from active state (e.g. hover)
		JsonSkinClass *activeState = nullptr;
		if(!currentStateStr.empty()) {
			auto stateIt = cl->states.find(currentStateStr);
			if(stateIt != cl->states.end()) {
				activeState = stateIt->second.get();
				applyPropsToTable(activeState->properties, t);
			}
		}

		auto mergeProperties = [&](const auto &baseMap, const auto *stateMap, const char *tableName) {
			if(baseMap.empty() && (!stateMap || stateMap->empty()))
				return;

			auto tGroup = luabind::newtable(l);

			// Base properties
			for(const auto &[name, baseItem] : baseMap) {
				auto tItem = luabind::newtable(l);

				applyPropsToTable(baseItem->properties, tItem);
				if(stateMap) {
					auto stateIt = stateMap->find(name);
					if(stateIt != stateMap->end())
						applyPropsToTable(stateIt->second->properties, tItem);
				}
				tGroup[name] = tItem;
			}

			// Apply states
			if(stateMap) {
				for(const auto &[name, stateItem] : *stateMap) {
					if(baseMap.find(name) == baseMap.end()) {
						auto tItem = luabind::newtable(l);
						applyPropsToTable(stateItem->properties, tItem);
						tGroup[name] = tItem;
					}
				}
			}
			t[tableName] = tGroup;
		};

		mergeProperties(cl->children, activeState ? &activeState->children : nullptr, "children");
		mergeProperties(cl->decorators, activeState ? &activeState->decorators : nullptr, "decorators");

		apply_style_class(WGUILuaInterface::GetLuaObject(l, *el), t);
	}
}

void pragma::gui::JsonSkin::Release(types::WIBase *el) { WISkin::Release(el); }
