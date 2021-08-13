/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_AUTO_DOC_GENERATOR_HPP__
#define __LUA_AUTO_DOC_GENERATOR_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/lad/lad.hpp"
#include <luasystem.h>

#undef GetClassInfo

namespace pragma::doc
{
	class Collection;
	struct Source;
};
namespace pragma::lua
{
	struct LuaMethodInfo;
	struct LuaClassInfo;
	struct ClassRegInfo;
	class TypeNameManager;
	class PdbManager;
	class SymbolHandler;
	struct LuaDocGenerator
	{
	public:
		LuaDocGenerator(lua_State *l);
		void IterateLibraries(luabind::object o,pragma::doc::Collection &collection,pragma::doc::Collection &colTarget);
		void SetPdbManager(PdbManager *manager) {m_pdbManager = manager;}
		void SetSymbolHandler(SymbolHandler *symHandler) {m_symbolHandler = symHandler;}
		void SetTypeManager(TypeNameManager *typeManager){m_typeNameManager = typeManager;}
		void PopulateMethods(lua_State *l);

		const std::unordered_map<luabind::detail::class_rep*,pragma::doc::Collection*> &GetClassMap() const {return m_iteratedClasses;}
		const std::unordered_map<pragma::doc::Collection*,LuaClassInfo> &GetCollectionToLuaClassInfoMap() const {return m_collectionClassInfo;}
		const std::unordered_map<const luabind::detail::class_rep*,pragma::doc::Collection*> &GetClassRepToCollectionMap() const {return m_classRepToCollection;}
	private:
		std::string *LoadFileContents(const std::string &fileName);
		void IterateLibraries(
			luabind::object o,const std::string &path,pragma::doc::Collection &collection,
			pragma::doc::Collection &colTarget
		);
		void ParseLuaProperty(const std::string &name,const luabind::object &o,LuaClassInfo &result,bool isMethod);
		LuaClassInfo GetClassInfo(luabind::detail::class_rep *crep);
		void AddClass(luabind::detail::class_rep *crep,const std::string &path);
		void InitializeRepositoryUrls();
		void GenerateDocParameters(const luabind::detail::TypeInfo &input,std::vector<pragma::doc::Parameter> &outputs,const std::optional<std::string> &argName={},bool keepTuples=false);
		static std::optional<std::string> GetCommitId(const std::string &repositoryUrl);
		static std::string BuildRepositoryUrl(const std::string &baseUrl,const std::string &commitUrl);
		std::optional<std::string> SourceToUrl(const pragma::doc::Source &source);
		std::shared_ptr<pragma::doc::Collection> ClassInfoToCollection(lua_State *l,const std::string &path,const LuaClassInfo &classInfo);
		void AddFunction(lua_State *l,pragma::doc::Collection &collection,const LuaMethodInfo &method,bool isMethod=false);
	
		void TranslateFunctionLineDefinition(const std::string &fileName,uint32_t &inOutLine);
		std::optional<std::vector<std::string>> FindFunctionArguments(const std::string &fileName,uint32_t line);

		lua_State *m_luaState = nullptr;
		std::unordered_map<std::string,std::string> m_cachedFileContents {};
		std::vector<luabind::object> m_traversed {};
		std::unordered_map<luabind::detail::class_rep*,ClassRegInfo> m_classRegs;
		std::unordered_map<luabind::detail::class_rep*,pragma::doc::Collection*> m_iteratedClasses;
		std::unordered_map<std::string,std::string> m_moduleToRepositoryMap;

		std::unordered_map<pragma::doc::Collection*,LuaClassInfo> m_collectionClassInfo;
		std::unordered_map<const luabind::detail::class_rep*,pragma::doc::Collection*> m_classRepToCollection;

		PdbManager *m_pdbManager = nullptr;
		SymbolHandler *m_symbolHandler = nullptr;
		TypeNameManager *m_typeNameManager = nullptr;
	};
};

#endif
