/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __LOCALIZATION_H__
#define __LOCALIZATION_H__

#include <pragma/networkdefinitions.h>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

class Locale;
struct DLLNETWORK Localization
{
public:
	friend Locale;
protected:
	Localization();
public:
	std::unordered_map<std::string,std::string> texts;
};

class DLLNETWORK Locale
{
public:
	static bool Load(const std::string &file,bool bReload=false);
	static void SetLanguage(std::string lan);
	static bool GetText(const std::string &id,std::string &outText);
	static bool GetText(const std::string &id,const std::vector<std::string> &args,std::string &outText);
	static std::string GetText(const std::string &id,const std::vector<std::string> &args={});
	static const std::string &GetLanguage();
	static std::unordered_map<std::string,std::string> GetLanguages();
private:
	static Localization m_localization;
	static std::vector<std::string> m_loadedFiles;
	static std::string m_language;
	static bool Load(const std::string &file,const std::string &lan,bool bReload);
	static bool LoadFile(const std::string &file,const std::string &lan);
};

#endif