/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LOCALIZATION_H__
#define __LOCALIZATION_H__

#include <pragma/networkdefinitions.h>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <sharedutils/util_utf8.hpp>

class Locale;
struct DLLNETWORK Localization {
	Localization();
	std::unordered_map<std::string, util::Utf8String> texts;
};

class DLLNETWORK Locale {
  public:
	struct DLLNETWORK LanguageInfo {
		std::string displayName;
		udm::PProperty configData = nullptr;
	};

	enum class LoadResult : uint8_t { Success = 0, Failed, AlreadyLoaded };

	static void Init();
	static void Clear();

	static LoadResult Load(const std::string &file, bool bReload = false);
	static void SetLanguage(std::string lan);
	static void ReloadFiles();
	static bool GetText(const std::string &id, std::string &outText);
	static bool GetText(const std::string &id, util::Utf8String &outText);
	static bool GetText(const std::string &id, const std::vector<std::string> &args, std::string &outText);
	static bool GetText(const std::string &id, const std::vector<util::Utf8String> &args, util::Utf8String &outText);
	static std::string GetText(const std::string &id, const std::vector<std::string> &args = {});
	static util::Utf8String GetTextUtf8(const std::string &id, const std::vector<util::Utf8String> &args = {});
	static bool SetLocalization(const std::string &id, const util::Utf8String &text, bool overwriteIfExists = true);
	static const std::string &GetLanguage();
	static const LanguageInfo *GetLanguageInfo();
	static const std::unordered_map<std::string, LanguageInfo> &GetLanguages();
	static std::unordered_map<std::string, util::Utf8String> &GetTexts();
	static void Poll();
	static std::string DetermineSystemLanguage();
	static LoadResult LoadFile(const std::string &file, const std::string &lan, Localization &outLoc);
	static std::string GetFileLocation(const std::string &file, const std::string &lan);
	static bool Localize(const std::string &identifier, const std::string &lan, const std::string &category, const util::Utf8String &text);
  private:
	static Localization m_localization;
	static std::vector<std::string> m_loadedFiles;
	static std::string m_language;
	static LoadResult Load(const std::string &file, const std::string &lan, bool bReload);
	static LoadResult LoadFile(const std::string &file, const std::string &lan);
};

#endif
