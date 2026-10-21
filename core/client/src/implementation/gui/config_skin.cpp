// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.config_skin;

import :client_state;

static auto cvDebugGuiStyleLua = pragma::console::get_client_con_var("debug_gui_style_lua");
pragma::gui::ConfigSkinWithLua::ConfigSkinWithLua() : ConfigSkin {} {}
void pragma::gui::ConfigSkinWithLua::ApplyStyle(types::WIBase &el, Style &style) const
{
	auto debugMode = cvDebugGuiStyleLua->GetInt();
	if (debugMode != 2)
	{
		if (auto debugPrint = style.GetPropertyValue<udm::String>("debugPrint"); debugPrint)
		{
			console::set_console_color(pragma::console::ConsoleColorFlags::BackgroundMagenta);

			std::stringstream elStr;
			elStr << el.GetIndex();

			auto msg = *debugPrint;
			string::replace(msg, "{self}", elStr.str());
			Con::COUT << "GUI element debug print: " << msg << Con::endl;
		}
		ConfigSkin::ApplyStyle(el, style);

		constexpr std::string_view soundCacheKey = "lastSkinSound";
		if (auto sound = style.GetPropertyValue<udm::String>("sound"); sound)
		{
			auto &cache = *GetCacheData(el, true);
			std::string lastSkinSound;
			cache[soundCacheKey] >> lastSkinSound;
			if (lastSkinSound != *sound)
			{
				if (auto *cl = get_client_state(); cl)
				{
					if (auto snd = cl->CreateSound(*sound, audio::ALSoundType::GUI, audio::ALCreateFlags::Mono); snd)
					{
						snd->SetRelative(true);
						snd->SetPitch(0.8f); // TODO: Let user define via style
						snd->Play();
					}
				}
				cache[soundCacheKey] << *sound;
			}
		}
		else
		{
			auto *cache = GetCacheData(el, true);
			if (cache)
			{
				auto it = cache->children.find(soundCacheKey);
				if (it != cache->children.end())
					cache->children.erase(it);
			}
		}
	}

	if (debugMode > 0)
		ApplyStyleLua(el, style);
}

void pragma::gui::ConfigSkinWithLua::Initialize(StyleProperty &rootStyle, const udm::PropertyWrapper &data)
{
	if (auto fonts = data["theme"]["fonts"]; fonts)
	{
		for(auto &pair : fonts.ElIt())
		{
			auto &fontName = pair.key;
			auto &udmFont = pair.property;
			std::string fontSet = "default";
			udmFont["fontSet"] >> fontSet;
			ResolveConstant(fontSet);

			if(fontSet == "default")
				fontSet = get_cengine()->GetDefaultFontSetName();

			int32_t size = 16;
			udmFont["size"] >> size;

			auto featureFlags = FontSetFlag::None;
			for (auto &udmFlag : udmFont["flags"])
			{
				std::string flagVal;
				if (!(udmFlag >> flagVal))
					continue;
				using namespace pragma::string::string_switch;
				switch(hash(flagVal))
				{
				case "sans"_:
					featureFlags |= FontSetFlag::Sans;
					break;
				case "mono"_:
					featureFlags |= FontSetFlag::Mono;
					break;
				case "bold"_:
					featureFlags |= FontSetFlag::Bold;
					break;
				case "italic"_:
					featureFlags |= FontSetFlag::Italic;
					break;
				case "serif"_:
					featureFlags |= FontSetFlag::Serif;
					break;
				}
			}

			Lua::engine::create_font(nullptr, std::string {fontName}, fontSet, featureFlags, size, false);
		}
	}
}

void pragma::gui::ConfigSkinWithLua::ApplyStyleLua(types::WIBase &el, Style &style) const
{
#ifdef ENABLE_GUI_LUA_STATE
	auto *l = get_client_state()->GetGUILuaState();
	auto apply_style_class = luabind::object {l, luabind::globals(l)["gui"]};
	if(apply_style_class)
		apply_style_class = apply_style_class["apply_style_class"];
	if(!apply_style_class)
		throw std::runtime_error {"Unable to apply style classes: Could not find Lua function 'apply_style_class'."};

	auto prop = udm::Property::Create<udm::Element>();
	udm::LinkedPropertyWrapper root {*prop};
	style.ToUdmData(root);

	auto t = Lua::udm::udm_to_value(l, root);
	apply_style_class(WGUILuaInterface::GetLuaObject(l, el), t);
#endif
}

namespace pragma::debug {
	std::string get_element_info(pragma::gui::types::WIBase &el);
	gui::types::WIBase *find_gui_element(std::vector<std::string> &argv);
}

static void debug_print_style(pragma::gui::types::WIBase &el)
{
	auto *skin = dynamic_cast<pragma::gui::ConfigSkin *>(el.GetSkin());
	if(!skin)
	{
		Con::CWAR<<"Element has no valid skin."<<Con::endl;
		return;
	}
	auto style = skin->ComputeStyle(el);
	if(!style)
	{
		Con::CWAR<<"Failed to compute style for element!"<<Con::endl;
		return;
	}

	auto print_style = [](this auto &self, const auto &s, int indent, std::ostream &os) -> void {
		std::string ind(indent * 4, ' ');
		std::string child_ind((indent + 1) * 4, ' ');

		for(const auto &[name, prop] : s.GetProperties()) {
			auto propStr = prop.DumpToString();
			os << ind << name << ": " << propStr << "\n";
		}

		if(!s.descendants.empty()) {
			os << ind << "descendants: {\n";
			for(const auto &[name, child] : s.descendants) {
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
