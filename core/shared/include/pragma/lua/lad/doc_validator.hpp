/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_AUTO_DOC_VALIDATOR_HPP__
#define __LUA_AUTO_DOC_VALIDATOR_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/lad/lad.hpp"

namespace pragma::lua
{
	class DocValidator
	{
	public:
	private:
		template<class TItem>
			static const TItem *FindItem(const std::string &name,const std::vector<TItem>&(pragma::doc::Collection::*fPt)() const)
		{
			return nullptr;
		}
		template<class TItem>
			static const TItem *FindItem(const pragma::doc::Collection &collection,const std::string &name,const std::vector<TItem>&(pragma::doc::Collection::*fPt)() const);
		void GetClassInfo(lua_State *L,luabind::detail::class_rep *crep);
		void FindFunction(const std::string &name) {}
		void WriteToLog(const std::stringstream &log) {}
		void ValidateCollection(const pragma::doc::Collection &collection);
		std::vector<LuaClassInfo> m_classes;
	};
};

template<class TItem>
	static const TItem *pragma::lua::DocValidator::FindItem(const pragma::doc::Collection &collection,const std::string &name,const std::vector<TItem>&(pragma::doc::Collection::*fPt)() const)
{
	auto &items = (collection.*fPt)();
	auto it = std::find_if(items.begin(),items.end(),[&name](const TItem &el) {
		return el.GetName() == name;
	});
	if(it != items.end())
		return &*it;
	for(auto &child : collection.GetChildren())
	{
		auto *pItem = find_item(*child,name,fPt);
		if(pItem)
			return pItem;
	}
	return nullptr;
}

#endif
