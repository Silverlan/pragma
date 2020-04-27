/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __LUA_CLASS_MANAGER_HPP__
#define __LUA_CLASS_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <optional>
#include <unordered_map>
#include <luasystem.h>

#undef RegisterClass

struct lua_State;
namespace pragma::lua
{
	class DLLNETWORK ClassManager
	{
	public:
		struct DLLNETWORK ClassRef
		{
			ClassRef(const std::string &className)
				: className{className}
			{}
			ClassRef(const luabind::object &classObject)
				: classObject{classObject}
			{}
			std::optional<std::string> className;
			std::optional<luabind::object> classObject;
		};

		ClassManager(lua_State &l);
		void RegisterClass(const std::string &className,luabind::object oClass);
		bool IsClassRegistered(const ClassRef &classRef) const;
		bool IsClassMethodDefined(const ClassRef &classRef,const std::string &methodName) const;
	private:
		lua_State &m_luaState;
		struct ClassInfo
		{
			luabind::object classObject;
			luabind::detail::class_rep *class_rep;
		};
		ClassInfo *FindClassInfo(const ClassRef &classRef);
		const ClassInfo *FindClassInfo(const ClassRef &classRef) const;
		std::vector<ClassInfo> m_classes = {};
		std::unordered_map<std::string,size_t> m_classNameToClassIndex = {};
	};
};

#endif
