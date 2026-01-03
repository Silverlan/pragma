// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.libraries.locale;

export import pragma.shared;

export namespace Lua {
	namespace Locale {
		DLLCLIENT void set_localization(const std::string &id, const std::string &text);
		DLLCLIENT void change_language(const std::string &lan);
		DLLCLIENT int get_text(lua::State *l);
		DLLCLIENT bool load(const std::string &fileName);
		DLLCLIENT const std::string &get_language();
		DLLCLIENT int get_languages(lua::State *l);
		DLLCLIENT bool localize(const std::string &identifier, const std::string &lan, const std::string &category, const std::string &text);
		DLLCLIENT bool relocalize(const std::string &identifier, const std::string &newIdentifier, const std::string &oldCategory, const std::string &newCategory);
		DLLCLIENT void clear();
		DLLCLIENT map<std::string, std::string> get_texts(lua::State *l);
		DLLCLIENT opt<map<std::string, std::string>> parse(lua::State *l, const std::string &fileName, const std::string &lan);
		DLLCLIENT opt<map<std::string, std::string>> parse(lua::State *l, const std::string &fileName);
	};
};
