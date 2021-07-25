/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_AUTO_DOC_SYMBAL_HANDLER_HPP__
#define __LUA_AUTO_DOC_SYMBAL_HANDLER_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <optional>
#include <luasystem.h>

namespace pragma::os::windows
{
	using RVA = uint64_t; // TODO: RVAs should be uint32_t
	std::string get_last_error_as_string();
};
namespace pragma::lua
{
	class SymbolHandler
	{
	public:
		~SymbolHandler();
		bool Initialize();
		std::optional<std::string> FindModule(DWORD64 rva) const;
		std::optional<std::string> FindAddress(DWORD64 rva) const;
		std::optional<pragma::os::windows::RVA> FindSymbol(const std::string &symbol) const;
		std::optional<std::string> FindSource(DWORD64 rva,uint32_t &outLine) const;

		operator bool() const {return m_valid;}
	private:
		HANDLE m_hProcess = nullptr;
		bool m_valid = false;
	};
};

#endif
