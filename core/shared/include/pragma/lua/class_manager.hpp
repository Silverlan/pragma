// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LUA_CLASS_MANAGER_HPP__
#define __LUA_CLASS_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <optional>
#include <unordered_map>
#include <luasystem.h>

#undef RegisterClass

struct lua_State;
namespace pragma::lua {
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

		ClassManager(lua_State &l);
		void RegisterClass(const std::string &className, luabind::object oClass, luabind::object regFc);
		bool IsClassRegistered(const ClassRef &classRef) const;
		bool IsClassMethodDefined(const ClassRef &classRef, const std::string &methodName) const;
		ClassInfo *FindClassInfo(const ClassRef &classRef);
		const ClassInfo *FindClassInfo(const ClassRef &classRef) const;
	  private:
		lua_State &m_luaState;
		std::vector<ClassInfo> m_classes = {};
		std::unordered_map<std::string, size_t> m_classNameToClassIndex = {};
	};
};

#endif
