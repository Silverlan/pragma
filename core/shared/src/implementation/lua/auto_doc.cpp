// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.auto_doc;

import pragma.doc;

static void print_function_documentation(const pragma::doc::Function &function, std::stringstream &ss);
static void print_member_documentation(const pragma::doc::Member &member, std::stringstream &ss);
static void print_enum_documentation(const pragma::doc::Enum &e, std::stringstream &ss);
static void print_enum_set_documentation(const pragma::doc::EnumSet &enumSet, std::stringstream &ss);
static void print_collection(const pragma::doc::Collection &collection, std::stringstream &ss);

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
	if(pragma::Engine::Get()->GetConsoleType() == pragma::Engine::ConsoleType::Terminal)
		return arg;
	return "{[l:lua_help \"" + arg + "\"]}" + arg + "{[/l]}";
}
inline std::string wrap_web_link(const std::string &arg)
{
	if(pragma::Engine::Get()->GetConsoleType() == pragma::Engine::ConsoleType::Terminal)
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
	pragma::fs::find_files(Lua::doc::FILE_LOCATION + std::string {"*."} + Lua::doc::FILE_EXTENSION_ASCII, &files, nullptr);
	pragma::fs::find_files(Lua::doc::FILE_LOCATION + std::string {"*."} + Lua::doc::FILE_EXTENSION_BINARY, &files, nullptr);
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
	auto udmData = pragma::util::load_udm_asset(fileName, &err);
	if(udmData == nullptr) {
		Con::CWAR << "Unable to load Lua documentation '" << fileName << "': " << err << Con::endl;
		return false;
	}
	auto col = pragma::doc::Collection::Load(udmData->GetAssetData(), err);
	if(!col) {
		Con::CWAR << "Unable to load Lua documentation '" << fileName << "': " << err << Con::endl;
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
			if(pragma::math::is_flag_set(col.GetFlags(), pragma::doc::Collection::Flags::Library))
				++numLibs;
			if(pragma::math::is_flag_set(col.GetFlags(), pragma::doc::Collection::Flags::Class))
				++numClasses;

			for(auto &child : col.GetChildren())
				fCollectStats(*child);
		};
		for(auto &col : s_docInfo.collections)
			fCollectStats(*col);
		Con::COUT << "Functions: " << numFunctions << Con::endl;
		Con::COUT << "Libraries: " << numLibs << Con::endl;
		Con::COUT << "Classes: " << numClasses << Con::endl;
		Con::COUT << "Members: " << numMembers << Con::endl;
		Con::COUT << "Enums: " << numEnums << Con::endl;
	}
	return true;
}
void Lua::doc::find_candidates(const std::string &name, std::vector<const pragma::doc::BaseCollectionObject *> &outCandidates, uint32_t candidateLimit)
{
	initialize_pragma_documentation();
	std::vector<std::pair<const pragma::doc::BaseCollectionObject *, float>> similarCandidates {};
	for(auto &pair : s_docInfo.lookupTable) {
		auto percentage = pragma::string::calc_similarity(name, pair.first);
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

	pragma::fs::create_path("doc/ZeroBrane/api/lua");
	auto autocompleteScriptZb = pragma::doc::zerobrane::generate_autocomplete_script(s_docInfo.collections);
	pragma::fs::write_file("doc/ZeroBrane/api/lua/pragma.lua", autocompleteScriptZb);

	auto lsDoc = pragma::doc::luals::generate_doc(s_docInfo.collections);
	for(auto &pair : lsDoc.streams) {
		std::string path = "doc/LuaLS/meta/";
		pragma::fs::create_path(path);
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
		pragma::fs::write_file(path, ss.str());
	}

	pragma::fs::create_path("doc/ZeroBrane/cfg");
	pragma::fs::write_file("doc/ZeroBrane/cfg/pragma.lua",
	  R"(debugger.allowediting = true
debugger.ignorecase = true
editor.autotabs = true

-- Autocomplete options
autocomplete = true
acandtip.droprest = false
acandtip.nodynwords = false
acandtip.shorttip = false
acandtip.width = 120)");

	pragma::fs::create_path("doc/ZeroBrane/interpreters");
	pragma::fs::write_file("doc/ZeroBrane/interpreters/pragma.lua",
	  R"(dofile 'interpreters/luabase.lua'
local interpreter = MakeLuaInterpreter()
interpreter.name = "Pragma"
interpreter.description = "Lua-implementation for the Pragma game engine"
-- table.insert(interpreter.api,"pragma")
interpreter.api = {"pragma"}
return interpreter)");

	pragma::fs::create_path("doc/ZeroBrane/");
	pragma::fs::write_file("doc/ZeroBrane/readme.txt", R"(See https://wiki.pragma-engine.com/books/lua-api/page/zerobrane-ide for more information.)");
}
void Lua::doc::print_documentation(const std::string &name, std::stringstream &ss)
{
	const auto MAX_SIMILAR_CANDIDATES = 20u;
	std::vector<const pragma::doc::BaseCollectionObject *> similarCandidates {};
	find_candidates(name, similarCandidates, MAX_SIMILAR_CANDIDATES);
	if(similarCandidates.empty() == false) {
		ss << "\n";
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Yellow | pragma::console::ConsoleColorFlags::Intensity);
		ss << "Were you looking for the following";
		auto *pFunction = dynamic_cast<const pragma::doc::Function *>(similarCandidates.front());
		auto *pMember = dynamic_cast<const pragma::doc::Member *>(similarCandidates.front());
		auto *pEnum = dynamic_cast<const pragma::doc::Enum *>(similarCandidates.front());
		auto *pEnumSet = dynamic_cast<const pragma::doc::EnumSet *>(similarCandidates.front());
		auto *pCollection = dynamic_cast<const pragma::doc::Collection *>(similarCandidates.front());
		if(pFunction != nullptr)
			ss << " function";
		else if(pMember != nullptr)
			ss << " member";
		else if(pEnum != nullptr)
			ss << " enum";
		else if(pEnumSet != nullptr)
			ss << " enum set";
		else if(pCollection != nullptr) {
			auto flags = pCollection->GetFlags();
			auto bClass = (flags & pragma::doc::Collection::Flags::Class) != pragma::doc::Collection::Flags::None;
			ss << " " << (bClass ? "class" : "library");
		}
		ss << "?\n";
		// TODO: Print warning if game state flags don't match with lua state's!
		if(pFunction != nullptr)
			print_function_documentation(*pFunction, ss);
		else if(pMember != nullptr)
			print_member_documentation(*pMember, ss);
		else if(pEnum != nullptr)
			print_enum_documentation(*pEnum, ss);
		else if(pEnumSet != nullptr)
			print_enum_set_documentation(*pEnumSet, ss);
		else if(pCollection != nullptr)
			print_collection(*pCollection, ss);

		if(similarCandidates.size() > 1u) {
			ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
			ss << "Other similar items:\n";
			ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
			for(auto it = similarCandidates.begin() + 1u; it < similarCandidates.end(); ++it)
				ss << " - " << wrap_link((*it)->GetFullName()) << "\n";
		}
	}
}

static void print_game_state_flags(pragma::doc::GameStateFlags gameStateFlags, std::stringstream &ss)
{
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "State: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	if(gameStateFlags == pragma::doc::GameStateFlags::None)
		ss << "None" << "\n";
	else {
		const std::unordered_map<pragma::doc::GameStateFlags, std::string> flagNames {{pragma::doc::GameStateFlags::Client, "Client"}, {pragma::doc::GameStateFlags::Server, "Server"}, {pragma::doc::GameStateFlags::GUI, "GUI"}};
		auto bFirstFlag = true;
		for(auto &pair : flagNames) {
			if((gameStateFlags & pair.first) == pragma::doc::GameStateFlags::None)
				continue;
			if(bFirstFlag == false)
				ss << ", ";
			else
				bFirstFlag = false;
			ss << pair.second;
		}
		ss << "\n";
	}
}

void print_member_documentation(const pragma::doc::Member &member, std::stringstream &ss)
{
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Name: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << member.GetFullName() << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Wiki URL: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << wrap_web_link(member.GetWikiURL()) << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Type: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Green | pragma::console::ConsoleColorFlags::Intensity);
	auto typeName = member.GetType().GetFormattedType(pragma::doc::ParameterFormatType::Generic, [](const pragma::doc::Variant &var, std::string &inOutName) { inOutName = wrap_link(inOutName); });
	ss << typeName << "\n";

	auto &def = member.GetDefault();
	if(def.has_value()) {
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
		ss << "Default: " << *def << "\n";
	}
	print_game_state_flags(member.GetGameStateFlags(), ss);

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Description:" << "\n";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << member.GetDescription() << "\n";
	ss << "\n";
}

void print_collection(const pragma::doc::Collection &collection, std::stringstream &ss)
{
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Name: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << collection.GetFullName() << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Wiki URL: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << wrap_web_link(collection.GetWikiURL()) << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Type: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	auto flags = collection.GetFlags();
	if((flags & pragma::doc::Collection::Flags::Library) != pragma::doc::Collection::Flags::None)
		ss << "Library";
	else if((flags & pragma::doc::Collection::Flags::Base) != pragma::doc::Collection::Flags::None)
		ss << "Base-Class";
	else if((flags & pragma::doc::Collection::Flags::Class) != pragma::doc::Collection::Flags::None)
		ss << "Class";
	ss << "\n";

	auto &derivedFrom = collection.GetDerivedFrom();
	if(derivedFrom.empty() == false) {
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
		ss << "Derived from:" << "\n";
		auto bFirst = true;
		for(auto &df : derivedFrom) {
			if(bFirst == false)
				ss << " -> ";
			else {
				ss << "- ";
				bFirst = false;
			}
			ss << wrap_link(df->GetName());
		}
		ss << "\n" << "\n";
	}

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Description:" << "\n";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << collection.GetDescription() << "\n";
	ss << "\n";

	auto &members = collection.GetMembers();
	if(members.empty() == false) {
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
		ss << "Members:" << "\n";
		std::vector<std::size_t> sortedIndices;
		sortedIndices.reserve(members.size());
		for(auto i = decltype(members.size()) {0u}; i < members.size(); ++i)
			sortedIndices.push_back(i);
		std::sort(sortedIndices.begin(), sortedIndices.end(), [&members](std::size_t idx0, std::size_t idx1) { return members.at(idx0).GetName() < members.at(idx1).GetName(); });
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
		for(auto idx : sortedIndices) {
			auto &member = members.at(idx);
			ss << "- ";
			ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Green | pragma::console::ConsoleColorFlags::Intensity);

			auto typeName = member.GetType().GetFormattedType(pragma::doc::ParameterFormatType::Generic, [](const pragma::doc::Variant &var, std::string &inOutName) { inOutName = wrap_link(inOutName); });

			ss << "[" << wrap_link(typeName) << "] ";
			ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
			ss << member.GetFullName() << "\n";
		}
		ss << "\n";
	}

	auto &enumSets = collection.GetEnumSets();
	if(enumSets.empty() == false) {
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
		ss << "Enum Sets:" << "\n";
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
		for(auto &enumSet : enumSets) {
			ss << "- ";
			ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Green | pragma::console::ConsoleColorFlags::Intensity);
			ss << "[" << wrap_link(enumSet->GetUnderlyingType()) << "] ";
			ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
			ss << enumSet->GetFullName() << "\n";
		}
		ss << "\n";
	}

	auto fPrintFunctions = [&collection, &ss](pragma::doc::Function::Type type, const std::string &label) {
		auto &functions = collection.GetFunctions();
		std::vector<const pragma::doc::Function *> typeFunctions;
		typeFunctions.reserve(functions.size());
		for(auto &fc : functions) {
			if(fc.GetType() == type)
				typeFunctions.push_back(&fc);
		}
		if(typeFunctions.empty())
			return;
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
		ss << label << ":" << "\n";
		std::vector<std::size_t> sortedIndices;
		sortedIndices.reserve(typeFunctions.size());
		for(auto i = decltype(typeFunctions.size()) {0u}; i < typeFunctions.size(); ++i)
			sortedIndices.push_back(i);
		std::sort(sortedIndices.begin(), sortedIndices.end(), [&typeFunctions](std::size_t idx0, std::size_t idx1) { return typeFunctions.at(idx0)->GetName() < typeFunctions.at(idx1)->GetName(); });
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
		for(auto idx : sortedIndices) {
			auto &fc = typeFunctions.at(idx);
			ss << "- ";
			ss << wrap_link(fc->GetName()) << "\n";
			;
		}
		ss << "\n";
	};
	fPrintFunctions(pragma::doc::Function::Type::Function, "Static Functions");
	fPrintFunctions(pragma::doc::Function::Type::Method, "Methods");
	fPrintFunctions(pragma::doc::Function::Type::Hook, "Hooks");

	auto &children = collection.GetChildren();
	if(children.empty() == false) {
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
		ss << "Children:" << "\n";
		std::vector<std::size_t> sortedIndices;
		sortedIndices.reserve(children.size());
		for(auto i = decltype(children.size()) {0u}; i < children.size(); ++i)
			sortedIndices.push_back(i);
		std::sort(sortedIndices.begin(), sortedIndices.end(), [&children](std::size_t idx0, std::size_t idx1) { return children.at(idx0)->GetName() < children.at(idx1)->GetName(); });
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
		for(auto idx : sortedIndices) {
			auto &child = children.at(idx);
			ss << "- ";
			ss << wrap_link(child->GetName()) << "\n";
		}
		ss << "\n";
	}
}

void print_enum_set_documentation(const pragma::doc::EnumSet &enumSet, std::stringstream &ss)
{
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Name: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << enumSet.GetFullName() << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Underlying Type: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << enumSet.GetUnderlyingType() << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Wiki URL: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << wrap_web_link(enumSet.GetWikiURL()) << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Values:" << "\n";
	auto &enums = enumSet.GetEnums();
	std::vector<std::size_t> sortedIndices;
	sortedIndices.reserve(enums.size());
	for(auto i = decltype(enums.size()) {0u}; i < enums.size(); ++i)
		sortedIndices.push_back(i);
	std::sort(sortedIndices.begin(), sortedIndices.end(), [&enums](std::size_t idx0, std::size_t idx1) { return enums.at(idx0).GetName() < enums.at(idx1).GetName(); });
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	for(auto idx : sortedIndices) {
		auto &en = enums.at(idx);
		auto &desc = en.GetDescription();
		ss << "- " << en.GetName();
		auto &value = en.GetValue();
		if(value.empty() == false)
			ss << " = " << value;
		if(desc.empty() == false)
			ss << ": " << desc;
		ss << "\n";
	}
	ss << "\n";
}

void print_enum_documentation(const pragma::doc::Enum &e, std::stringstream &ss)
{
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Name: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << e.GetFullName() << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Enum Set: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << e.GetEnumSet()->GetName() << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Wiki URL: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << wrap_web_link(e.GetWikiURL()) << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Type: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Green | pragma::console::ConsoleColorFlags::Intensity);
	switch(e.GetType()) {
	case pragma::doc::Enum::Type::Bit:
		ss << "Bit";
		break;
	default:
		ss << "Value" << "\n";
		break;
	}
	ss << "\n";

	auto &value = e.GetValue();
	if(value.empty() == false) {
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
		ss << "Value: ";
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
		ss << value << "\n";
	}
	print_game_state_flags(e.GetGameStateFlags(), ss);

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Description:" << "\n";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << e.GetDescription() << "\n";
	ss << "\n";
}

void print_function_documentation(const pragma::doc::Function &function, std::stringstream &ss)
{
	// TODO: Check game state; Compare with state of called lua state -> If not same, print warning!
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Name: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << function.GetFullName() << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Wiki URL: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << wrap_web_link(function.GetWikiURL()) << "\n";

	auto type = function.GetType();
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Type: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	switch(type) {
	case pragma::doc::Function::Type::Function:
		ss << "Function";
		break;
	case pragma::doc::Function::Type::Method:
		ss << "Method";
		break;
	case pragma::doc::Function::Type::Hook:
		ss << "Hook";
		break;
	default:
		ss << "Unknown";
	}
	ss << "\n";

	auto flags = function.GetFlags();
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Flags: ";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	if(flags == pragma::doc::Function::Flags::None)
		ss << "None" << "\n";
	else {
		const std::unordered_map<pragma::doc::Function::Flags, std::string> flagNames {{pragma::doc::Function::Flags::Debug, "Debug"}, {pragma::doc::Function::Flags::Deprecated, "Deprecated"}, {pragma::doc::Function::Flags::Vanilla, "Vanilla"}};
		auto bFirstFlag = true;
		for(auto &pair : flagNames) {
			if((flags & pair.first) == pragma::doc::Function::Flags::None)
				continue;
			if(bFirstFlag == false)
				ss << ", ";
			else
				bFirstFlag = false;
			ss << pair.second;
		}
		ss << "\n";
	}

	print_game_state_flags(function.GetGameStateFlags(), ss);
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Overloads:" << "\n";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	for(auto &overload : function.GetOverloads()) {
		auto &returnValues = overload.GetReturnValues();
		auto bFirst = true;
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Green | pragma::console::ConsoleColorFlags::Intensity);
		if(returnValues.empty() == false) {
			auto retStr = pragma::doc::Parameter::GetFormattedParameterString(returnValues, pragma::doc::ParameterFormatType::Generic, false, [](const pragma::doc::Variant &var, std::string &inOutName) { inOutName = wrap_link(inOutName); });
			ss << retStr;
			/*for(auto &returnValue : returnValues)
			{
				if(bFirst == false)
				{
					ss<<pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
					ss<<", ";
				}
				else
					bFirst = false;
				ss<<pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Green | pragma::console::ConsoleColorFlags::Intensity);
				ss<<"["<<wrap_link(returnValue.GetFullType())<<"]";
			}*/
		}
		else
			ss << "[void]";
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
		ss << " " << function.GetName() << "(";

		auto &parameters = overload.GetParameters();
		auto paramStr = pragma::doc::Parameter::GetFormattedParameterString(parameters, pragma::doc::ParameterFormatType::Generic, true, [](const pragma::doc::Variant &var, std::string &inOutName) { inOutName = wrap_link(inOutName); });
		ss << paramStr;
		/*bFirst = true;
		for(auto &param : parameters)
		{
			if(bFirst == false)
				ss<<", ";
			else
				bFirst = false;

			ss<<pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Green | pragma::console::ConsoleColorFlags::Intensity);
			ss<<"["<<wrap_link(param.GetFullType())<<"]";
			ss<<pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
			ss<<" "<<param.GetName();
			auto &def = param.GetDefault();
			if(def.has_value())
				ss<<"="<<*def;
		//	param.GetGameStateFlags // TODO
		}*/
		ss << ")" << "\n";
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	}
	ss << "\n";

	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
	ss << "Description:" << "\n";
	ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
	ss << function.GetDescription() << "\n";
	ss << "\n";

	auto &codeExample = function.GetExampleCode();
	if(codeExample.has_value()) {
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
		ss << "Code Example:" << "\n";
		ss << pragma::console::get_ansi_color_code(pragma::console::ConsoleColorFlags::Reset);
		if(codeExample->description.empty() == false)
			ss << "-- " << codeExample->description << "\n";
		ss << codeExample->code << "\n";
		ss << "\n";
	}
}
