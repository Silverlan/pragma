/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/localization.h"
#include "pragma/logging.hpp"
#include <fsys/directory_watcher.h>
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util_path.hpp>
#include <sharedutils/magic_enum.hpp>

#undef CreateFile
decltype(Locale::m_localization) Locale::m_localization;
decltype(Locale::m_language) Locale::m_language;
decltype(Locale::m_loadedFiles) Locale::m_loadedFiles;

Localization::Localization() {}

//////////////////////////

static spdlog::logger &LOGGER = pragma::register_logger("locale");

static constexpr auto LOCALIZATION_ROOT_PATH = "scripts/localization/";

static std::unique_ptr<DirectoryWatcherCallback> g_locFileWatcher = nullptr;
void Locale::Init() {}
void Locale::Clear()
{
	g_locFileWatcher = nullptr;
	m_loadedFiles.clear();
	m_localization.texts.clear();
	m_language.clear();
}
Locale::LoadResult Locale::Load(const std::string &file, const std::string &lan, bool bReload)
{
	auto filePath = util::Path::CreateFile(file);
	auto it = std::find(m_loadedFiles.begin(), m_loadedFiles.end(), filePath.GetString());
	if(it != m_loadedFiles.end()) {
		if(bReload == false)
			return Locale::LoadResult::AlreadyLoaded;
		m_loadedFiles.erase(it);
	}
	return LoadFile(filePath.GetString(), lan);
}

std::string Locale::GetFileLocation(const std::string &file, const std::string &lan) { return LOCALIZATION_ROOT_PATH + lan + "/texts/" + file; }

Locale::LoadResult Locale::ParseFile(const std::string &file, const std::string &lan, std::unordered_map<std::string, util::Utf8String> &outTexts)
{
	auto filePath = GetFileLocation(file, lan);
	auto f = FileManager::OpenFile(filePath.c_str(), "r");
	if(f != nullptr) {
		while(!f->Eof()) {
			auto l = f->ReadLine();
			std::string key;
			std::string val;
			if(ustring::get_key_value(l, key, val)) {
				ustring::replace(val, "\\\"", "\"");
				ustring::replace(val, "\\n", "\n");
				outTexts[key] = val;
			}
		}
		return LoadResult::Success;
	}
	return LoadResult::Failed;
}
Locale::LoadResult Locale::LoadFile(const std::string &file, const std::string &lan, Localization &outLoc) { return ParseFile(file, lan, outLoc.texts); }

Locale::LoadResult Locale::LoadFile(const std::string &file, const std::string &lan)
{
	auto res = LoadFile(file, lan, m_localization);
	if(res == LoadResult::Success) {
		m_loadedFiles.push_back(file);
		LOGGER.debug("Loaded localization file '{}' for language '{}'.", file, lan);
	}
	else
		LOGGER.warn("Failed to load localization file '{}' for language '{}': {}", file, lan, magic_enum::enum_name(res));
	return res;
}

Locale::LoadResult Locale::Load(const std::string &file, bool bReload)
{
	auto r = Load(file, "en", bReload);
	if(m_language == "en")
		return r;
	r = Load(file, m_language, bReload || (r == LoadResult::Success));
	return r;
}

void Locale::Poll()
{
	if(g_locFileWatcher)
		g_locFileWatcher->Poll();
}

void Locale::ReloadFiles()
{
	auto files = m_loadedFiles;
	for(auto &f : files)
		Load(f, true);
}

void Locale::SetLanguage(std::string lan)
{
	LOGGER.debug("Changing global language to '{}'...", lan);
	ustring::to_lower(lan);
	m_language = lan;

	auto loadedFiles = m_loadedFiles;
	m_loadedFiles.clear();
	for(auto &fpath : loadedFiles)
		LoadFile(fpath, lan);

	try {
		g_locFileWatcher = std::make_unique<DirectoryWatcherCallback>(LOCALIZATION_ROOT_PATH + lan + '/', [](const std::string &str) {
			auto filePath = util::Path::CreateFile(str);
			auto it = std::find(m_loadedFiles.begin(), m_loadedFiles.end(), filePath.GetString());
			if(it == m_loadedFiles.end())
				return;
			Con::cout << "Reloading localization file '" << str << "'..." << Con::endl;
			Load(str, true);
		});
	}
	catch(const std::runtime_error &err) {
	}
}
const std::string &Locale::GetLanguage() { return m_language; }
static std::unordered_map<std::string, Locale::LanguageInfo> g_languages;
const Locale::LanguageInfo *Locale::GetLanguageInfo()
{
	auto &languages = GetLanguages();
	auto it = languages.find(m_language);
	if(it == languages.end())
		return nullptr;
	return &it->second;
}
std::unordered_map<std::string, util::Utf8String> &Locale::GetTexts() { return m_localization.texts; }
const std::unordered_map<std::string, Locale::LanguageInfo> &Locale::GetLanguages()
{
	if(g_languages.empty()) {
		std::vector<std::string> lanDirs;
		std::string baseDir = "scripts/localization/";
		filemanager::find_files(baseDir + "*", nullptr, &lanDirs);
		for(auto &identifier : lanDirs) {
			auto lanPath = baseDir + identifier + "/";
			if(!filemanager::exists(lanPath + "language.udm"))
				continue;
			LanguageInfo lanInfo {};
			try {
				auto udmConfig = udm::Data::Load("scripts/localization/" + identifier + "/language.udm");
				if(udmConfig) {
					auto udmData = udmConfig->GetAssetData().GetData()[identifier];
					udmData["displayName"](lanInfo.displayName);
					lanInfo.configData = udmData.ClaimOwnership();
				}
			}
			catch(const udm::Exception &e) {
			}
			g_languages[identifier] = lanInfo;
		}
	}
	return g_languages;
}
bool Locale::SetLocalization(const std::string &id, const util::Utf8String &text, bool overwriteIfExists)
{
	if(!overwriteIfExists && m_localization.texts.find(id) != m_localization.texts.end())
		return false;
	m_localization.texts[id] = text;
	return true;
}
bool Locale::GetText(const std::string &id, util::Utf8String &outText) { return GetText(id, {}, outText); }
bool Locale::GetText(const std::string &id, std::string &outText) { return GetText(id, {}, outText); }
bool Locale::GetRawText(const std::string &id, std::string &outText)
{
	auto it = m_localization.texts.find(id);
	if(it == m_localization.texts.end())
		return false;
	outText = it->second.cpp_str();
	return true;
}
bool Locale::GetRawText(const std::string &id, util::Utf8String &outText)
{
	auto it = m_localization.texts.find(id);
	if(it == m_localization.texts.end())
		return false;
	outText = it->second;
	return true;
}
template<class TString>
static void insert_arguments(const std::vector<TString> &args, TString &inOutText)
{
	size_t startPos = inOutText.find('{');
	uint32_t argIdx = 0;
	while(startPos != std::string::npos) {
		auto endPos = inOutText.find('}', startPos);
		if(endPos == std::string::npos)
			return;
		size_t numCharsAdded = 0;
		if(endPos == startPos + 1) {
			// Brackets without argument ("{}")
			assert(argIdx < args.size());
			if(argIdx < args.size()) {
				auto &arg = args[argIdx++];
				inOutText = inOutText.replace(startPos, 2, arg);
				numCharsAdded = arg.size();
			}
			else {
				// Missing argument
				std::string arg = "{}";
				std::string str;
				inOutText = inOutText.replace(startPos, endPos - startPos + 1, arg);
				numCharsAdded = arg.size();
			}
		}
		else {
			using TSubStr = std::conditional_t<std::is_same_v<TString, util::Utf8String>, std::string, std::string_view>;
			auto innerStartPos = startPos + 1;
			auto innerLen = (endPos - startPos) - 1;
			TSubStr inner;
			if constexpr(std::is_same_v<TString, util::Utf8String>)
				inner = inOutText.substr(innerStartPos, innerLen).cpp_str();
			else
				inner = {inOutText.c_str() + innerStartPos, innerLen};
			int32_t argIdx = -1;
			auto result = std::from_chars(inner.data(), inner.data() + inner.size(), argIdx);
			if(result.ec != std::errc::invalid_argument) {
				// Brackets with index argument (e.g. "{0}")
				assert(argIdx < args.size());
				if(argIdx < args.size()) {
					auto &arg = args[argIdx];
					inOutText = inOutText.replace(startPos, endPos - startPos + 1, arg);
					numCharsAdded = arg.size();
				}
				else {
					// Missing argument
					std::string arg = "{" + std::string {inner} + "}";
					inOutText = inOutText.replace(startPos, endPos - startPos + 1, arg);
					numCharsAdded = arg.size();
				}
			}
			else {
				// Brackets with locale id (e.g. "{math_unit}")
				auto innerText = Locale::GetText(std::string {inner});
				inOutText = inOutText.replace(startPos, endPos - startPos + 1, innerText);
				numCharsAdded = innerText.size();
			}
		}
		endPos = startPos + numCharsAdded;
		if(endPos >= inOutText.size())
			break;
		startPos = inOutText.find('{', endPos);
	}
}
bool Locale::GetText(const std::string &id, const std::vector<util::Utf8String> &args, util::Utf8String &outText)
{
	auto it = m_localization.texts.find(id);
	if(it == m_localization.texts.end())
		return false;
	outText = it->second;
	insert_arguments<util::Utf8String>(args, outText);
	return true;
}
bool Locale::GetText(const std::string &id, const std::vector<std::string> &args, std::string &outText)
{
	auto it = m_localization.texts.find(id);
	if(it == m_localization.texts.end())
		return false;
	outText = it->second.cpp_str();
	insert_arguments<std::string>(args, outText);
	return true;
}
std::string Locale::GetText(const std::string &id, const std::vector<std::string> &args)
{
	auto it = m_localization.texts.find(id);
	if(it == m_localization.texts.end()) {
		LOGGER.warn("Missing localization for '{}'!", id);
		return std::string("<MISSING LOCALIZATION: ") + id + std::string(">");
	}
	auto r = it->second.cpp_str();
	insert_arguments<std::string>(args, r);
	return r;
}
util::Utf8String Locale::GetTextUtf8(const std::string &id, const std::vector<util::Utf8String> &args)
{
	auto it = m_localization.texts.find(id);
	if(it == m_localization.texts.end()) {
		LOGGER.warn("Missing localization for '{}'!", id);
		return std::string("<MISSING LOCALIZATION: ") + id + std::string(">");
	}
	auto r = it->second;
	insert_arguments<util::Utf8String>(args, r);
	return r;
}
std::string Locale::DetermineSystemLanguage()
{
	auto lan = util::get_system_language();
	if(!lan)
		lan = "en";
	return *lan;
}
void Locale::LoadAll()
{
	auto &lan = GetLanguage();
	std::vector<std::string> files;
	filemanager::find_files(LOCALIZATION_ROOT_PATH + lan + "/texts/*.txt", &files, nullptr);
	for(auto &f : files)
		LoadFile(f, lan);
}
util::Utf8String Locale::GetUsedCharacters()
{
	std::unordered_set<uint32_t> usedCharacters;
	for(auto &pair : m_localization.texts) {
		for(auto c : pair.second)
			usedCharacters.insert(c);
	}
	std::vector<uint32_t> vUsedCharacters;
	vUsedCharacters.reserve(usedCharacters.size());
	for(auto c : usedCharacters)
		vUsedCharacters.push_back(c);
	std::sort(vUsedCharacters.begin(), vUsedCharacters.end());
	util::Utf8String usedCharsStr;
	for(auto c : vUsedCharacters)
		usedCharsStr += static_cast<char16_t>(c);
	return usedCharsStr;
}
bool Locale::Localize(const std::string &identifier, const std::string &lan, const std::string &category, const util::Utf8String &text)
{
	auto fileName = category + ".txt";
	Localization loc {};
	if(Locale::LoadFile(fileName, lan, loc) == LoadResult::Failed) {
		auto success = false;
		auto filePath = GetFileLocation(fileName, lan);
		if(!filemanager::exists(filePath) && lan != "en") {
			auto filePathEn = GetFileLocation(fileName, "en");
			std::string absPath;
			if(FileManager::FindLocalPath(filePathEn, absPath)) {
				ustring::replace(absPath, "\\", "/");
				auto newPath = absPath;
				ustring::replace(newPath, "/en/", "/" + lan + "/");
				if(filemanager::create_path(ufile::get_path_from_filename(newPath)) && filemanager::write_file(newPath, ""))
					success = (Locale::LoadFile(fileName, lan, loc) != LoadResult::Failed); // Try again
			}
		}
		if(!success)
			return false;
	}
	loc.texts[identifier] = text;
	std::vector<std::string> keys;
	keys.reserve(loc.texts.size());
	for(auto &pair : loc.texts)
		keys.push_back(pair.first);
	std::sort(keys.begin(), keys.end());

	std::stringstream out;
	auto first = true;
	for(auto &key : keys) {
		if(!first)
			out << "\n";
		else
			first = false;

		auto val = loc.texts[key];
		ustring::replace<util::Utf8String>(val, "\"", "\\\"");
		ustring::replace<util::Utf8String>(val, "\n", "\\n");
		out << key << " = \"" << val << "\"";
	}

	auto fullFileName = Locale::GetFileLocation(fileName, lan);
	if(!FileManager::FindLocalPath(fullFileName, fullFileName))
		return false;
	auto f = filemanager::open_file<VFilePtrReal>(fullFileName, filemanager::FileMode::Write);
	if(!f)
		return false;
	f->WriteString(out.str());
	return true;
}
