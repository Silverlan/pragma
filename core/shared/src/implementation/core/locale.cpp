/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

module;

#include "stdafx_shared.h"
#include "pragma/logging.hpp"
#include <fsys/directory_watcher.h>
#include <fsys/filesystem.h>
#include <sharedutils/util_string.h>
#include <sharedutils/util_path.hpp>
#include <sharedutils/magic_enum.hpp>

#undef CreateFile

module pragma.locale;

static std::vector<std::string> g_loadedFiles;
static std::string g_language;
static std::unordered_map<std::string, pragma::locale::LanguageInfo> g_languages;
static pragma::locale::Localization g_localization;

static spdlog::logger &LOGGER = pragma::register_logger("locale");

static constexpr auto LOCALIZATION_ROOT_PATH = "scripts/localization/";

static std::unique_ptr<DirectoryWatcherCallback> g_locFileWatcher = nullptr;

namespace pragma::locale {
	static LoadResult load(const std::string &file, const std::string &lan, bool bReload);
	static LoadResult load_file(const std::string &file, const std::string &lan);
};

pragma::locale::Localization::Localization() {}

void pragma::locale::init() {}
void pragma::locale::clear()
{
	g_locFileWatcher = nullptr;
	g_loadedFiles.clear();
	g_localization.texts.clear();
	g_language.clear();
}
pragma::locale::LoadResult pragma::locale::load(const std::string &file, const std::string &lan, bool bReload)
{
	auto filePath = util::Path::CreateFile(file);
	auto it = std::find(g_loadedFiles.begin(), g_loadedFiles.end(), filePath.GetString());
	if(it != g_loadedFiles.end()) {
		if(bReload == false)
			return pragma::locale::LoadResult::AlreadyLoaded;
		g_loadedFiles.erase(it);
	}
	return load_file(filePath.GetString(), lan);
}

std::string pragma::locale::get_file_location(const std::string &file, const std::string &lan) { return LOCALIZATION_ROOT_PATH + lan + "/texts/" + file; }

pragma::locale::LoadResult pragma::locale::parse_file(const std::string &file, const std::string &lan, std::unordered_map<std::string, pragma::string::Utf8String> &outTexts)
{
	auto filePath = get_file_location(file, lan);
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
pragma::locale::LoadResult pragma::locale::load_file(const std::string &file, const std::string &lan, Localization &outLoc) { return parse_file(file, lan, outLoc.texts); }

pragma::locale::LoadResult pragma::locale::load_file(const std::string &file, const std::string &lan)
{
	auto res = load_file(file, lan, g_localization);
	if(res == LoadResult::Success) {
		g_loadedFiles.push_back(file);
		LOGGER.debug("Loaded localization file '{}' for language '{}'.", file, lan);
	}
	else
		LOGGER.warn("Failed to load localization file '{}' for language '{}': {}", file, lan, magic_enum::enum_name(res));
	return res;
}

pragma::locale::LoadResult pragma::locale::load(const std::string &file, bool bReload)
{
	auto r = load(file, "en", bReload);
	if(g_language == "en")
		return r;
	r = load(file, g_language, bReload || (r == LoadResult::Success));
	return r;
}

void pragma::locale::poll()
{
	if(g_locFileWatcher)
		g_locFileWatcher->Poll();
}

void pragma::locale::reload_files()
{
	auto files = g_loadedFiles;
	for(auto &f : files)
		load(f, true);
}

void pragma::locale::set_language(std::string lan)
{
	LOGGER.debug("Changing global language to '{}'...", lan);
	ustring::to_lower(lan);
	g_language = lan;

	auto loadedFiles = g_loadedFiles;
	g_loadedFiles.clear();
	for(auto &fpath : loadedFiles)
		load_file(fpath, lan);

	try {
		g_locFileWatcher = std::make_unique<DirectoryWatcherCallback>(LOCALIZATION_ROOT_PATH + lan + '/', [](const std::string &str) {
			auto filePath = util::Path::CreateFile(str);
			auto it = std::find(g_loadedFiles.begin(), g_loadedFiles.end(), filePath.GetString());
			if(it == g_loadedFiles.end())
				return;
			Con::cout << "Reloading localization file '" << str << "'..." << Con::endl;
			load(str, true);
		});
	}
	catch(const std::runtime_error &err) {
	}
}
const std::string &pragma::locale::get_language() { return g_language; }
const pragma::locale::LanguageInfo *pragma::locale::get_language_info()
{
	auto &languages = get_languages();
	auto it = languages.find(g_language);
	if(it == languages.end())
		return nullptr;
	return &it->second;
}
std::unordered_map<std::string, pragma::string::Utf8String> &pragma::locale::get_texts() { return g_localization.texts; }
const std::unordered_map<std::string, pragma::locale::LanguageInfo> &pragma::locale::get_languages()
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
bool pragma::locale::set_localization(const std::string &id, const pragma::string::Utf8String &text, bool overwriteIfExists)
{
	if(!overwriteIfExists && g_localization.texts.find(id) != g_localization.texts.end())
		return false;
	g_localization.texts[id] = text;
	return true;
}
bool pragma::locale::get_text(const std::string &id, pragma::string::Utf8String &outText) { return get_text(id, {}, outText); }
bool pragma::locale::get_text(const std::string &id, std::string &outText) { return get_text(id, {}, outText); }
bool pragma::locale::get_raw_text(const std::string &id, std::string &outText)
{
	auto it = g_localization.texts.find(id);
	if(it == g_localization.texts.end())
		return false;
	outText = it->second.cpp_str();
	return true;
}
bool pragma::locale::get_raw_text(const std::string &id, pragma::string::Utf8String &outText)
{
	auto it = g_localization.texts.find(id);
	if(it == g_localization.texts.end())
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
			using TSubStr = std::conditional_t<std::is_same_v<TString, pragma::string::Utf8String>, std::string, std::string_view>;
			auto innerStartPos = startPos + 1;
			auto innerLen = (endPos - startPos) - 1;
			TSubStr inner;
			if constexpr(std::is_same_v<TString, pragma::string::Utf8String>)
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
				auto innerText = pragma::locale::get_text(std::string {inner});
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
bool pragma::locale::get_text(const std::string &id, const std::vector<pragma::string::Utf8String> &args, pragma::string::Utf8String &outText)
{
	auto it = g_localization.texts.find(id);
	if(it == g_localization.texts.end())
		return false;
	outText = it->second;
	insert_arguments<pragma::string::Utf8String>(args, outText);
	return true;
}
bool pragma::locale::get_text(const std::string &id, const std::vector<std::string> &args, std::string &outText)
{
	auto it = g_localization.texts.find(id);
	if(it == g_localization.texts.end())
		return false;
	outText = it->second.cpp_str();
	insert_arguments<std::string>(args, outText);
	return true;
}
std::string pragma::locale::get_text(const std::string &id, const std::vector<std::string> &args)
{
	auto it = g_localization.texts.find(id);
	if(it == g_localization.texts.end()) {
		LOGGER.warn("Missing localization for '{}'!", id);
		return std::string("<MISSING LOCALIZATION: ") + id + std::string(">");
	}
	auto r = it->second.cpp_str();
	insert_arguments<std::string>(args, r);
	return r;
}
pragma::string::Utf8String pragma::locale::get_text_utf8(const std::string &id, const std::vector<pragma::string::Utf8String> &args)
{
	auto it = g_localization.texts.find(id);
	if(it == g_localization.texts.end()) {
		LOGGER.warn("Missing localization for '{}'!", id);
		return std::string("<MISSING LOCALIZATION: ") + id + std::string(">");
	}
	auto r = it->second;
	insert_arguments<pragma::string::Utf8String>(args, r);
	return r;
}
std::string pragma::locale::determine_system_language()
{
	auto lan = util::get_system_language();
	if(!lan)
		lan = "en";
	return *lan;
}
void pragma::locale::load_all()
{
	auto &lan = get_language();
	std::vector<std::string> files;
	filemanager::find_files(LOCALIZATION_ROOT_PATH + lan + "/texts/*.txt", &files, nullptr);
	for(auto &f : files)
		load_file(f, lan);
}
pragma::string::Utf8String pragma::locale::get_used_characters()
{
	std::unordered_set<uint32_t> usedCharacters;
	for(auto &pair : g_localization.texts) {
		for(auto c : pair.second)
			usedCharacters.insert(c);
	}
	std::vector<uint32_t> vUsedCharacters;
	vUsedCharacters.reserve(usedCharacters.size());
	for(auto c : usedCharacters)
		vUsedCharacters.push_back(c);
	std::sort(vUsedCharacters.begin(), vUsedCharacters.end());
	pragma::string::Utf8String usedCharsStr;
	for(auto c : vUsedCharacters)
		usedCharsStr += static_cast<char16_t>(c);
	return usedCharsStr;
}
static bool save_localization(const pragma::locale::Localization &loc, const std::string &fileName, const std::string &lan)
{
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

		auto val = loc.texts.find(key)->second;
		ustring::replace<pragma::string::Utf8String>(val, "\"", "\\\"");
		ustring::replace<pragma::string::Utf8String>(val, "\n", "\\n");
		out << key << " = \"" << val << "\"";
	}

	auto fullFileName = pragma::locale::get_file_location(fileName, lan);
	if(!FileManager::FindLocalPath(fullFileName, fullFileName))
		return false;
	auto f = filemanager::open_file<VFilePtrReal>(fullFileName, filemanager::FileMode::Write);
	if(!f)
		return false;
	f->WriteString(out.str());
	return true;
}
bool pragma::locale::relocalize(const std::string &identifier, const std::string &newIdentifier, const std::string &oldCategory, const std::string &newCategory)
{
	auto fileName = oldCategory + ".txt";
	for(auto &[lan, lanInfo] : pragma::locale::get_languages()) {
		Localization loc {};
		if(pragma::locale::load_file(fileName, lan, loc) == LoadResult::Failed)
			continue;
		auto it = loc.texts.find(identifier);
		if(it == loc.texts.end())
			continue;
		auto text = std::move(it->second);
		loc.texts.erase(it);

		if(localize(newIdentifier, lan, newCategory, text))
			;
		save_localization(loc, fileName, lan);
	}
	return true;
}
bool pragma::locale::localize(const std::string &identifier, const std::string &lan, const std::string &category, const pragma::string::Utf8String &text)
{
	auto fileName = category + ".txt";
	Localization loc {};
	if(pragma::locale::load_file(fileName, lan, loc) == LoadResult::Failed) {
		auto success = false;
		auto filePath = get_file_location(fileName, lan);
		if(!filemanager::exists(filePath) && lan != "en") {
			auto filePathEn = get_file_location(fileName, "en");
			std::string absPath;
			if(FileManager::FindLocalPath(filePathEn, absPath)) {
				ustring::replace(absPath, "\\", "/");
				auto newPath = absPath;
				ustring::replace(newPath, "/en/", "/" + lan + "/");
				if(filemanager::create_path(ufile::get_path_from_filename(newPath)) && filemanager::write_file(newPath, ""))
					success = (pragma::locale::load_file(fileName, lan, loc) != LoadResult::Failed); // Try again
			}
		}
		if(!success)
			return false;
	}
	loc.texts[identifier] = text;
	return save_localization(loc, fileName, lan);
}
