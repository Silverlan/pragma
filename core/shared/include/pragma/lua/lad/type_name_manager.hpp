/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifdef _MSC_VER
#ifndef __LUA_AUTO_DOC_TYPE_NAME_MANAGER_HPP__
#define __LUA_AUTO_DOC_TYPE_NAME_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include <luasystem.h>

namespace pragma::lua {
	class TypeNameManager {
	  public:
		struct TypeInfo {
			size_t hash = 0;
			std::string name;
			std::string simplifiedName;
			std::string decoratedName;
			std::string luaName;
			const luabind::detail::class_rep *luaClassDef = nullptr;

			std::optional<std::string> bestMatch {};
			double bestMatchScore = std::numeric_limits<double>::lowest();
		};
		void RegisterType(const std::type_info &typeInfo, const luabind::detail::class_rep &luaClassDef);
		void AssignType(const std::string &name);
		const TypeInfo *GetTypeInfo(const std::type_info *ti) const;
		const std::unordered_map<const std::type_info *, TypeInfo *> &GetTypes() const { return m_types; }
		const std::unordered_map<std::string, const std::type_info *> &GetAssignedTypes() const { return m_assignedTypes; }
		std::optional<const std::string_view> TranslateType(const std::string &type) const;
	  private:
		std::unordered_map<const std::type_info *, TypeInfo *> m_types;
		std::unordered_map<std::string, const std::type_info *> m_assignedTypes;
	};
};

#endif
#endif
