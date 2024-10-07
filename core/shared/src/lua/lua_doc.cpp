/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/lua_doc.hpp"
#include "pragma/console/debugconsole.h"
#include "pragma/console/util_console_color.hpp"
#include "pragma/console/conout.h"
#include <pragma/util/util_game.hpp>
#include <pragma/engine.h>
#include <sharedutils/util_string.h>
#include <udm.hpp>

import pragma.doc;

extern DLLNETWORK Engine *engine;

static void print_function_documentation(const pragma::doc::Function &function);
static void print_member_documentation(const pragma::doc::Member &member);
static void print_enum_documentation(const pragma::doc::Enum &e);
static void print_enum_set_documentation(const pragma::doc::EnumSet &enumSet);
static void print_collection(const pragma::doc::Collection &collection);

struct DocInfo {
	std::vector<pragma::doc::PCollection> collections {};
	std::unordered_map<std::string, const pragma::doc::BaseCollectionObject *> lookupTable;
	void Clear()
	{
		lookupTable.clear();
		collections.clear();
	}
	void AddCollections(const std::vector<pragma::doc::PCollection> &newCollections)
	{
		collections.reserve(collections.size() + newCollections.size());
		for(auto &c : newCollections)
			collections.push_back(c);
		std::function<void(const std::vector<pragma::doc::PCollection> &)> fPopulateFunctions = nullptr;
		fPopulateFunctions = [&fPopulateFunctions, this](const std::vector<pragma::doc::PCollection> &collections) {
			for(auto &collection : collections) {
				lookupTable[collection->GetFullName()] = collection.get();
				for(auto &fc : collection->GetFunctions())
					lookupTable[fc.GetFullName()] = &fc;
				for(auto &member : collection->GetMembers())
					lookupTable[member.GetFullName()] = &member;
				for(auto &enumSet : collection->GetEnumSets()) {
					for(auto &e : enumSet->GetEnums())
						lookupTable[e.GetFullName()] = &e;
					lookupTable[enumSet->GetFullName()] = enumSet.get();
				}
				fPopulateFunctions(collection->GetChildren());
			}
		};
		fPopulateFunctions(newCollections);
	}
};
static DocInfo s_docInfo {};
inline std::string wrap_link(const std::string &arg)
{
	if(engine->GetConsoleType() == Engine::ConsoleType::Terminal)
		return arg;
	return "{[l:lua_help \"" + arg + "\"]}" + arg + "{[/l]}";
}
inline std::string wrap_web_link(const std::string &arg)
{
	if(engine->GetConsoleType() == Engine::ConsoleType::Terminal)
		return arg;
	return "{[l:url \"" + arg + "\"]}" + arg + "{[/l]}";
}
static auto doc_initialized = false;
static void initialize_pragma_documentation()
{
	if(doc_initialized == true)
		return;
	doc_initialized = true;
	s_docInfo.Clear();
	std::vector<std::string> files;
	filemanager::find_files(Lua::doc::FILE_LOCATION + std::string {"*."} + Lua::doc::FILE_EXTENSION_ASCII, &files, nullptr);
	filemanager::find_files(Lua::doc::FILE_LOCATION + std::string {"*."} + Lua::doc::FILE_EXTENSION_BINARY, &files, nullptr);
	for(auto &f : files)
		Lua::doc::load_documentation_file(Lua::doc::FILE_LOCATION + f);
}
void Lua::doc::clear()
{
	doc_initialized = false;
	s_docInfo.Clear();
}
void Lua::doc::reset()
{
	if(!doc_initialized)
		return;
	clear();
	initialize_pragma_documentation();
}
bool Lua::doc::load_documentation_file(const std::string &fileName)
{
	initialize_pragma_documentation();

	std::string err;
	auto udmData = util::load_udm_asset(fileName, &err);
	if(udmData == nullptr) {
		Con::cwar << "Unable to load Lua documentation '" << fileName << "': " << err << Con::endl;
		return false;
	}
	auto col = pragma::doc::Collection::Load(udmData->GetAssetData(), err);
	if(!col) {
		Con::cwar << "Unable to load Lua documentation '" << fileName << "': " << err << Con::endl;
		return false;
	}
	s_docInfo.AddCollections(col->GetChildren());

	static auto printStats = false;
	if(printStats) {
		uint32_t numFunctions = 0;
		uint32_t numLibs = 0;
		uint32_t numClasses = 0;
		uint32_t numMembers = 0;
		uint32_t numEnums = 0;
		std::function<void(const pragma::doc::Collection &)> fCollectStats = nullptr;
		fCollectStats = [&](const pragma::doc::Collection &col) {
			numFunctions += col.GetFunctions().size();
			numEnums += col.GetEnumSets().size();
			numMembers += col.GetMembers().size();
			if(umath::is_flag_set(col.GetFlags(), pragma::doc::Collection::Flags::Library))
				++numLibs;
			if(umath::is_flag_set(col.GetFlags(), pragma::doc::Collection::Flags::Class))
				++numClasses;

			for(auto &child : col.GetChildren())
				fCollectStats(*child);
		};
		for(auto &col : s_docInfo.collections)
			fCollectStats(*col);
		Con::cout << "Functions: " << numFunctions << Con::endl;
		Con::cout << "Libraries: " << numLibs << Con::endl;
		Con::cout << "Classes: " << numClasses << Con::endl;
		Con::cout << "Members: " << numMembers << Con::endl;
		Con::cout << "Enums: " << numEnums << Con::endl;
	}
	return true;
}
void Lua::doc::find_candidates(const std::string &name, std::vector<const pragma::doc::BaseCollectionObject *> &outCandidates, uint32_t candidateLimit)
{
	initialize_pragma_documentation();
	std::vector<std::pair<const pragma::doc::BaseCollectionObject *, float>> similarCandidates {};
	for(auto &pair : s_docInfo.lookupTable) {
		auto percentage = ustring::calc_similarity(name, pair.first);
		auto bInserted = false;
		for(auto it = similarCandidates.begin(); it != similarCandidates.end(); ++it) {
			auto &sc = *it;
			if(percentage >= sc.second)
				continue;
			similarCandidates.insert(it, std::pair<const pragma::doc::BaseCollectionObject *, float> {pair.second, percentage});
			bInserted = true;
			break;
		}
		if(similarCandidates.size() > candidateLimit)
			similarCandidates.resize(candidateLimit);
		if(bInserted == false && similarCandidates.size() < candidateLimit)
			similarCandidates.push_back(std::pair<const pragma::doc::BaseCollectionObject *, float> {pair.second, percentage});
	}
	outCandidates.reserve(similarCandidates.size());
	for(auto &pair : similarCandidates)
		outCandidates.push_back(pair.first);
}
void Lua::doc::generate_autocomplete_script()
{
	initialize_pragma_documentation();

	filemanager::create_path("doc/ZeroBrane/api/lua");
	auto autocompleteScriptZb = pragma::doc::zerobrane::generate_autocomplete_script(s_docInfo.collections);
	filemanager::write_file("doc/ZeroBrane/api/lua/pragma.lua", autocompleteScriptZb);

	auto lsDoc = pragma::doc::luals::generate_doc(s_docInfo.collections);
	for(auto &pair : lsDoc.streams) {
		std::string path = "doc/LuaLS/meta/";
		filemanager::create_path(path);
		path += pair.first + ".lua";
		auto &ss = pair.second;
		if(pair.first == "_G") {
			// Some types have global aliases which can't be detected automatically, so we define them here.
			const std::unordered_map<std::string, std::string> globalAliases {
			  {"Vector2i", "math.Vector2i"},
			  {"Vector", "math.Vector"},
			  {"Vector2", "math.Vector2"},
			  {"Vector4", "math.Vector4"},
			  {"EulerAngles", "math.EulerAngles"},
			  {"Quaternion", "math.Quaternion"},
			  {"Color", "util.Color"},
			};
			ss << "\n";
			for(auto &pair : globalAliases)
				ss << pair.first << " = " << pair.second << "\n";
		}
		filemanager::write_file(path, ss.str());
	}

	filemanager::create_path("doc/ZeroBrane/cfg");
	filemanager::write_file("doc/ZeroBrane/cfg/pragma.lua",
	  R"(debugger.allowediting = true
debugger.ignorecase = true
editor.autotabs = true

-- Autocomplete options
autocomplete = true
acandtip.droprest = false
acandtip.nodynwords = false
acandtip.shorttip = false
acandtip.width = 120)");

	filemanager::create_path("doc/ZeroBrane/interpreters");
	filemanager::write_file("doc/ZeroBrane/interpreters/pragma.lua",
	  R"(dofile 'interpreters/luabase.lua'
local interpreter = MakeLuaInterpreter()
interpreter.name = "Pragma"
interpreter.description = "Lua-implementation for the Pragma game engine"
-- table.insert(interpreter.api,"pragma")
interpreter.api = {"pragma"}
return interpreter)");

	filemanager::create_path("doc/ZeroBrane/");
	filemanager::write_file("doc/ZeroBrane/readme.txt", R"(See https://wiki.pragma-engine.com/books/lua-api/page/zerobrane-ide for more information.)");
}
void Lua::doc::print_documentation(const std::string &name)
{
	const auto MAX_SIMILAR_CANDIDATES = 20u;
	std::vector<const pragma::doc::BaseCollectionObject *> similarCandidates {};
	Lua::doc::find_candidates(name, similarCandidates, MAX_SIMILAR_CANDIDATES);
	if(similarCandidates.empty() == false) {
		Con::cout << Con::endl;
		util::set_console_color(util::ConsoleColorFlags::Yellow | util::ConsoleColorFlags::Intensity);
		Con::cout << "Were you looking for the following";
		auto *pFunction = dynamic_cast<const pragma::doc::Function *>(similarCandidates.front());
		auto *pMember = dynamic_cast<const pragma::doc::Member *>(similarCandidates.front());
		auto *pEnum = dynamic_cast<const pragma::doc::Enum *>(similarCandidates.front());
		auto *pEnumSet = dynamic_cast<const pragma::doc::EnumSet *>(similarCandidates.front());
		auto *pCollection = dynamic_cast<const pragma::doc::Collection *>(similarCandidates.front());
		if(pFunction != nullptr)
			Con::cout << " function";
		else if(pMember != nullptr)
			Con::cout << " member";
		else if(pEnum != nullptr)
			Con::cout << " enum";
		else if(pEnumSet != nullptr)
			Con::cout << " enum set";
		else if(pCollection != nullptr) {
			auto flags = pCollection->GetFlags();
			auto bClass = (flags & pragma::doc::Collection::Flags::Class) != pragma::doc::Collection::Flags::None;
			Con::cout << " " << (bClass ? "class" : "library");
		}
		Con::cout << "?" << Con::endl;
		// TODO: Print warning if game state flags don't match with lua state's!
		if(pFunction != nullptr)
			print_function_documentation(*pFunction);
		else if(pMember != nullptr)
			print_member_documentation(*pMember);
		else if(pEnum != nullptr)
			print_enum_documentation(*pEnum);
		else if(pEnumSet != nullptr)
			print_enum_set_documentation(*pEnumSet);
		else if(pCollection != nullptr)
			print_collection(*pCollection);

		if(similarCandidates.size() > 1u) {
			util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
			Con::cout << "Other similar items:" << Con::endl;
			util::reset_console_color();
			for(auto it = similarCandidates.begin() + 1u; it < similarCandidates.end(); ++it)
				Con::cout << " - " << wrap_link((*it)->GetFullName()) << Con::endl;
		}
	}
}

static void print_game_state_flags(pragma::doc::GameStateFlags gameStateFlags)
{
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "State: ";
	util::reset_console_color();
	if(gameStateFlags == pragma::doc::GameStateFlags::None)
		Con::cout << "None" << Con::endl;
	else {
		const std::unordered_map<pragma::doc::GameStateFlags, std::string> flagNames {{pragma::doc::GameStateFlags::Client, "Client"}, {pragma::doc::GameStateFlags::Server, "Server"}, {pragma::doc::GameStateFlags::GUI, "GUI"}};
		auto bFirstFlag = true;
		for(auto &pair : flagNames) {
			if((gameStateFlags & pair.first) == pragma::doc::GameStateFlags::None)
				continue;
			if(bFirstFlag == false)
				Con::cout << ", ";
			else
				bFirstFlag = false;
			Con::cout << pair.second;
		}
		Con::cout << Con::endl;
	}
}

void print_member_documentation(const pragma::doc::Member &member)
{
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Name: ";
	util::reset_console_color();
	Con::cout << member.GetFullName() << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Wiki URL: ";
	util::reset_console_color();
	Con::cout << wrap_web_link(member.GetWikiURL()) << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Type: ";
	util::set_console_color(util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Intensity);
	auto typeName = member.GetType().GetFormattedType(pragma::doc::ParameterFormatType::Generic, [](const pragma::doc::Variant &var, std::string &inOutName) { inOutName = wrap_link(inOutName); });
	Con::cout << typeName << Con::endl;

	auto &def = member.GetDefault();
	if(def.has_value()) {
		util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		Con::cout << "Default: " << *def << Con::endl;
	}
	print_game_state_flags(member.GetGameStateFlags());

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Description:" << Con::endl;
	util::reset_console_color();
	Con::cout << member.GetDescription() << Con::endl;
	Con::cout << Con::endl;
}

void print_collection(const pragma::doc::Collection &collection)
{
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Name: ";
	util::reset_console_color();
	Con::cout << collection.GetFullName() << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Wiki URL: ";
	util::reset_console_color();
	Con::cout << wrap_web_link(collection.GetWikiURL()) << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Type: ";
	util::reset_console_color();
	auto flags = collection.GetFlags();
	if((flags & pragma::doc::Collection::Flags::Library) != pragma::doc::Collection::Flags::None)
		Con::cout << "Library";
	else if((flags & pragma::doc::Collection::Flags::Base) != pragma::doc::Collection::Flags::None)
		Con::cout << "Base-Class";
	else if((flags & pragma::doc::Collection::Flags::Class) != pragma::doc::Collection::Flags::None)
		Con::cout << "Class";
	Con::cout << Con::endl;

	auto &derivedFrom = collection.GetDerivedFrom();
	if(derivedFrom.empty() == false) {
		util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		Con::cout << "Derived from:" << Con::endl;
		auto bFirst = true;
		for(auto &df : derivedFrom) {
			if(bFirst == false)
				Con::cout << " -> ";
			else {
				Con::cout << "- ";
				bFirst = false;
			}
			Con::cout << wrap_link(df->GetName());
		}
		Con::cout << Con::endl << Con::endl;
	}

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Description:" << Con::endl;
	util::reset_console_color();
	Con::cout << collection.GetDescription() << Con::endl;
	Con::cout << Con::endl;

	auto &members = collection.GetMembers();
	if(members.empty() == false) {
		util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		Con::cout << "Members:" << Con::endl;
		std::vector<std::size_t> sortedIndices;
		sortedIndices.reserve(members.size());
		for(auto i = decltype(members.size()) {0u}; i < members.size(); ++i)
			sortedIndices.push_back(i);
		std::sort(sortedIndices.begin(), sortedIndices.end(), [&members](std::size_t idx0, std::size_t idx1) { return members.at(idx0).GetName() < members.at(idx1).GetName(); });
		util::reset_console_color();
		for(auto idx : sortedIndices) {
			auto &member = members.at(idx);
			Con::cout << "- ";
			util::set_console_color(util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Intensity);

			auto typeName = member.GetType().GetFormattedType(pragma::doc::ParameterFormatType::Generic, [](const pragma::doc::Variant &var, std::string &inOutName) { inOutName = wrap_link(inOutName); });

			Con::cout << "[" << wrap_link(typeName) << "] ";
			util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
			Con::cout << member.GetFullName() << Con::endl;
		}
		Con::cout << Con::endl;
	}

	auto &enumSets = collection.GetEnumSets();
	if(enumSets.empty() == false) {
		util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		Con::cout << "Enum Sets:" << Con::endl;
		util::reset_console_color();
		for(auto &enumSet : enumSets) {
			Con::cout << "- ";
			util::set_console_color(util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Intensity);
			Con::cout << "[" << wrap_link(enumSet->GetUnderlyingType()) << "] ";
			util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
			Con::cout << enumSet->GetFullName() << Con::endl;
		}
		Con::cout << Con::endl;
	}

	auto fPrintFunctions = [&collection](pragma::doc::Function::Type type, const std::string &label) {
		auto &functions = collection.GetFunctions();
		std::vector<const pragma::doc::Function *> typeFunctions;
		typeFunctions.reserve(functions.size());
		for(auto &fc : functions) {
			if(fc.GetType() == type)
				typeFunctions.push_back(&fc);
		}
		if(typeFunctions.empty())
			return;
		util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		Con::cout << label << ":" << Con::endl;
		std::vector<std::size_t> sortedIndices;
		sortedIndices.reserve(typeFunctions.size());
		for(auto i = decltype(typeFunctions.size()) {0u}; i < typeFunctions.size(); ++i)
			sortedIndices.push_back(i);
		std::sort(sortedIndices.begin(), sortedIndices.end(), [&typeFunctions](std::size_t idx0, std::size_t idx1) { return typeFunctions.at(idx0)->GetName() < typeFunctions.at(idx1)->GetName(); });
		util::reset_console_color();
		for(auto idx : sortedIndices) {
			auto &fc = typeFunctions.at(idx);
			Con::cout << "- ";
			Con::cout << wrap_link(fc->GetName()) << Con::endl;
			;
		}
		Con::cout << Con::endl;
	};
	fPrintFunctions(pragma::doc::Function::Type::Function, "Static Functions");
	fPrintFunctions(pragma::doc::Function::Type::Method, "Methods");
	fPrintFunctions(pragma::doc::Function::Type::Hook, "Hooks");

	auto &children = collection.GetChildren();
	if(children.empty() == false) {
		util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		Con::cout << "Children:" << Con::endl;
		std::vector<std::size_t> sortedIndices;
		sortedIndices.reserve(children.size());
		for(auto i = decltype(children.size()) {0u}; i < children.size(); ++i)
			sortedIndices.push_back(i);
		std::sort(sortedIndices.begin(), sortedIndices.end(), [&children](std::size_t idx0, std::size_t idx1) { return children.at(idx0)->GetName() < children.at(idx1)->GetName(); });
		util::reset_console_color();
		for(auto idx : sortedIndices) {
			auto &child = children.at(idx);
			Con::cout << "- ";
			Con::cout << wrap_link(child->GetName()) << Con::endl;
		}
		Con::cout << Con::endl;
	}
}

void print_enum_set_documentation(const pragma::doc::EnumSet &enumSet)
{
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Name: ";
	util::reset_console_color();
	Con::cout << enumSet.GetFullName() << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Underlying Type: ";
	util::reset_console_color();
	Con::cout << enumSet.GetUnderlyingType() << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Wiki URL: ";
	util::reset_console_color();
	Con::cout << wrap_web_link(enumSet.GetWikiURL()) << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Values:" << Con::endl;
	auto &enums = enumSet.GetEnums();
	std::vector<std::size_t> sortedIndices;
	sortedIndices.reserve(enums.size());
	for(auto i = decltype(enums.size()) {0u}; i < enums.size(); ++i)
		sortedIndices.push_back(i);
	std::sort(sortedIndices.begin(), sortedIndices.end(), [&enums](std::size_t idx0, std::size_t idx1) { return enums.at(idx0).GetName() < enums.at(idx1).GetName(); });
	util::reset_console_color();
	for(auto idx : sortedIndices) {
		auto &en = enums.at(idx);
		auto &desc = en.GetDescription();
		Con::cout << "- " << en.GetName();
		auto &value = en.GetValue();
		if(value.empty() == false)
			Con::cout << " = " << value;
		if(desc.empty() == false)
			Con::cout << ": " << desc;
		Con::cout << Con::endl;
	}
	Con::cout << Con::endl;
}

void print_enum_documentation(const pragma::doc::Enum &e)
{
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Name: ";
	util::reset_console_color();
	Con::cout << e.GetFullName() << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Enum Set: ";
	util::reset_console_color();
	Con::cout << e.GetEnumSet()->GetName() << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Wiki URL: ";
	util::reset_console_color();
	Con::cout << wrap_web_link(e.GetWikiURL()) << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Type: ";
	util::set_console_color(util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Intensity);
	switch(e.GetType()) {
	case pragma::doc::Enum::Type::Bit:
		Con::cout << "Bit";
		break;
	default:
		Con::cout << "Value" << Con::endl;
		break;
	}
	Con::cout << Con::endl;

	auto &value = e.GetValue();
	if(value.empty() == false) {
		util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		Con::cout << "Value: ";
		util::reset_console_color();
		Con::cout << value << Con::endl;
	}
	print_game_state_flags(e.GetGameStateFlags());

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Description:" << Con::endl;
	util::reset_console_color();
	Con::cout << e.GetDescription() << Con::endl;
	Con::cout << Con::endl;
}

void print_function_documentation(const pragma::doc::Function &function)
{
	// TODO: Check game state; Compare with state of called lua state -> If not same, print warning!
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Name: ";
	util::reset_console_color();
	Con::cout << function.GetFullName() << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Wiki URL: ";
	util::reset_console_color();
	Con::cout << wrap_web_link(function.GetWikiURL()) << Con::endl;

	auto type = function.GetType();
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Type: ";
	util::reset_console_color();
	switch(type) {
	case pragma::doc::Function::Type::Function:
		Con::cout << "Function";
		break;
	case pragma::doc::Function::Type::Method:
		Con::cout << "Method";
		break;
	case pragma::doc::Function::Type::Hook:
		Con::cout << "Hook";
		break;
	default:
		Con::cout << "Unknown";
	}
	Con::cout << Con::endl;

	auto flags = function.GetFlags();
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Flags: ";
	util::reset_console_color();
	if(flags == pragma::doc::Function::Flags::None)
		Con::cout << "None" << Con::endl;
	else {
		const std::unordered_map<pragma::doc::Function::Flags, std::string> flagNames {{pragma::doc::Function::Flags::Debug, "Debug"}, {pragma::doc::Function::Flags::Deprecated, "Deprecated"}, {pragma::doc::Function::Flags::Vanilla, "Vanilla"}};
		auto bFirstFlag = true;
		for(auto &pair : flagNames) {
			if((flags & pair.first) == pragma::doc::Function::Flags::None)
				continue;
			if(bFirstFlag == false)
				Con::cout << ", ";
			else
				bFirstFlag = false;
			Con::cout << pair.second;
		}
		Con::cout << Con::endl;
	}

	print_game_state_flags(function.GetGameStateFlags());
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Overloads:" << Con::endl;
	util::reset_console_color();
	for(auto &overload : function.GetOverloads()) {
		auto &returnValues = overload.GetReturnValues();
		auto bFirst = true;
		util::set_console_color(util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Intensity);
		if(returnValues.empty() == false) {
			auto retStr = pragma::doc::Parameter::GetFormattedParameterString(returnValues, pragma::doc::ParameterFormatType::Generic, false, [](const pragma::doc::Variant &var, std::string &inOutName) { inOutName = wrap_link(inOutName); });
			Con::cout << retStr;
			/*for(auto &returnValue : returnValues)
			{
				if(bFirst == false)
				{
					util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
					Con::cout<<", ";
				}
				else
					bFirst = false;
				util::set_console_color(util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Intensity);
				Con::cout<<"["<<wrap_link(returnValue.GetFullType())<<"]";
			}*/
		}
		else
			Con::cout << "[void]";
		util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		Con::cout << " " << function.GetName() << "(";

		auto &parameters = overload.GetParameters();
		auto paramStr = pragma::doc::Parameter::GetFormattedParameterString(parameters, pragma::doc::ParameterFormatType::Generic, true, [](const pragma::doc::Variant &var, std::string &inOutName) { inOutName = wrap_link(inOutName); });
		Con::cout << paramStr;
		/*bFirst = true;
		for(auto &param : parameters)
		{
			if(bFirst == false)
				Con::cout<<", ";
			else
				bFirst = false;

			util::set_console_color(util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Intensity);
			Con::cout<<"["<<wrap_link(param.GetFullType())<<"]";
			util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
			Con::cout<<" "<<param.GetName();
			auto &def = param.GetDefault();
			if(def.has_value())
				Con::cout<<"="<<*def;
		//	param.GetGameStateFlags // TODO
		}*/
		Con::cout << ")" << Con::endl;
		util::reset_console_color();
	}
	Con::cout << Con::endl;

	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << "Description:" << Con::endl;
	util::reset_console_color();
	Con::cout << function.GetDescription() << Con::endl;
	Con::cout << Con::endl;

	auto &codeExample = function.GetExampleCode();
	if(codeExample.has_value()) {
		util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
		Con::cout << "Code Example:" << Con::endl;
		util::reset_console_color();
		if(codeExample->description.empty() == false)
			Con::cout << "-- " << codeExample->description << Con::endl;
		Con::cout << codeExample->code << Con::endl;
		Con::cout << Con::endl;
	}
}
