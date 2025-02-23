/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

module;

#include <pragma/networkdefinitions.h>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

export module pragma.locale;

import pragma.string.unicode;

export namespace pragma::locale {
	struct DLLNETWORK LanguageInfo {
		std::string displayName;
		udm::PProperty configData = nullptr;
	};

	struct DLLNETWORK Localization {
		Localization();
		std::unordered_map<std::string, pragma::string::Utf8String> texts;
	};

	enum class LoadResult : uint8_t { Success = 0, Failed, AlreadyLoaded };

	DLLNETWORK void init();
	DLLNETWORK void clear();

	DLLNETWORK LoadResult load(const std::string &file, bool bReload = false);
	DLLNETWORK void set_language(std::string lan);
	DLLNETWORK void reload_files();
	DLLNETWORK bool get_raw_text(const std::string &id, std::string &outText);
	DLLNETWORK bool get_raw_text(const std::string &id, pragma::string::Utf8String &outText);
	DLLNETWORK bool get_text(const std::string &id, std::string &outText);
	DLLNETWORK bool get_text(const std::string &id, pragma::string::Utf8String &outText);
	DLLNETWORK bool get_text(const std::string &id, const std::vector<std::string> &args, std::string &outText);
	DLLNETWORK bool get_text(const std::string &id, const std::vector<pragma::string::Utf8String> &args, pragma::string::Utf8String &outText);
	DLLNETWORK std::string get_text(const std::string &id, const std::vector<std::string> &args = {});
	DLLNETWORK pragma::string::Utf8String get_text_utf8(const std::string &id, const std::vector<pragma::string::Utf8String> &args = {});
	DLLNETWORK bool set_localization(const std::string &id, const pragma::string::Utf8String &text, bool overwriteIfExists = true);
	DLLNETWORK const std::string &get_language();
	DLLNETWORK const LanguageInfo *get_language_info();
	DLLNETWORK const std::unordered_map<std::string, LanguageInfo> &get_languages();
	DLLNETWORK std::unordered_map<std::string, pragma::string::Utf8String> &get_texts();
	DLLNETWORK void poll();
	DLLNETWORK std::string determine_system_language();
	DLLNETWORK LoadResult load_file(const std::string &file, const std::string &lan, Localization &outLoc);
	DLLNETWORK LoadResult parse_file(const std::string &file, const std::string &lan, std::unordered_map<std::string, pragma::string::Utf8String> &outTexts);
	DLLNETWORK std::string get_file_location(const std::string &file, const std::string &lan);
	DLLNETWORK bool localize(const std::string &identifier, const std::string &lan, const std::string &category, const pragma::string::Utf8String &text);
	DLLNETWORK bool relocalize(const std::string &identifier, const std::string &newIdentifier, const std::string &oldCategory, const std::string &newCategory);
	DLLNETWORK pragma::string::Utf8String get_used_characters();

	// Loads all available scripts for the current language. For debugging purposes only.
	DLLNETWORK void load_all();
};
