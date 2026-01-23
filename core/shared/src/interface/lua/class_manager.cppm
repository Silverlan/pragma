// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

#undef RegisterClass

export module pragma.shared:scripting.lua.class_manager;

export import pragma.lua;
export import std;

export namespace pragma::LuaCore {
	class DLLNETWORK ClassManager {
	  public:
		struct DLLNETWORK ClassRef {
			ClassRef(const std::string &className) : className {className} {}
			ClassRef(const luabind::object &classObject) : classObject {classObject} {}
			std::optional<std::string> className;
			std::optional<luabind::object> classObject;
		};
		struct DLLNETWORK ClassInfo {
			luabind::object classObject;
			luabind::object regFunc;
			std::string className;
		};

		ClassManager(lua::State &l);
		void RegisterClass(const std::string &className, luabind::object oClass, luabind::object regFc);
		bool IsClassRegistered(const ClassRef &classRef) const;
		bool IsClassMethodDefined(const ClassRef &classRef, const std::string &methodName) const;
		ClassInfo *FindClassInfo(const ClassRef &classRef);
		const ClassInfo *FindClassInfo(const ClassRef &classRef) const;
	  private:
		lua::State &m_luaState;
		std::vector<ClassInfo> m_classes = {};
		std::unordered_map<std::string, size_t> m_classNameToClassIndex = {};
	};
};
