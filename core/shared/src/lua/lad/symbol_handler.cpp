/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */
#ifdef _MSC_VER
#include "stdafx_shared.h"
#include "pragma/lua/lad/symbol_handler.hpp"

using namespace pragma::lua;

std::string pragma::os::windows::get_last_error_as_string()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if(errorMessageID == 0) {
		return std::string(); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}

SymbolHandler::~SymbolHandler() {}

bool SymbolHandler::Initialize()
{
	SymSetOptions(/*SYMOPT_UNDNAME | */ SYMOPT_DEFERRED_LOADS);
	m_hProcess = GetCurrentProcess();
	if(!SymInitialize(m_hProcess, nullptr, TRUE)) {
		Con::cwar << "Unable to initialize symbol handler: " << os::windows::get_last_error_as_string() << Con::endl;
		m_valid = false;
		return false;
	}
	m_valid = true;
	return true;
}

std::optional<std::string> SymbolHandler::FindModule(DWORD64 rva) const
{
	if(!*this)
		return {};
	IMAGEHLP_MODULE moduleInfo;
	moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
	if(SymGetModuleInfo(m_hProcess, rva, &moduleInfo))
		return moduleInfo.ModuleName;
	return {};
}
std::optional<std::string> SymbolHandler::FindAddress(DWORD64 rva) const
{
	if(!*this)
		return {};
	DWORD64 dwDisplacement;
	constexpr uint32_t maxNameLen = 4'096;
	auto *ptr = new uint8_t[sizeof(SYMBOL_INFO) + maxNameLen + 1]; // See https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/ns-dbghelp-symbol_info?redirectedfrom=MSDN
	util::ScopeGuard sg {[ptr]() { delete[] ptr; }};
	auto &sym = *reinterpret_cast<SYMBOL_INFO *>(ptr);
	sym.MaxNameLen = maxNameLen;
	sym.SizeOfStruct = sizeof(SYMBOL_INFO);
	if(SymFromAddr(m_hProcess, rva, &dwDisplacement, &sym))
		return sym.Name;
	return {};
}
std::optional<std::string> SymbolHandler::FindSource(DWORD64 rva, uint32_t &outLine) const
{
	if(!*this)
		return {};
	IMAGEHLP_LINE64 line;
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	DWORD dwDisplacement;
	if(SymGetLineFromAddr(m_hProcess, rva, &dwDisplacement, &line) == TRUE) {
		outLine = line.LineNumber;
		return line.FileName;
	}
	else
		Con::cwar << "" << os::windows::get_last_error_as_string() << Con::endl;
	return {};
}
std::optional<pragma::os::windows::RVA> SymbolHandler::FindSymbol(const std::string &symbol) const
{
	return {};
#if 0
	if(!*this)
		return {};
	struct SymbolInfo
	{
		const SymbolHandler &symbolHandler;
		std::optional<pragma::os::windows::RVA> rva {};
	} symbolInfo {*this};
	auto mask = "*!" +symbol; // See https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-symenumsymbols
    SymEnumSymbols(m_hProcess, // Process handle from SymInitialize.
		0, // Base address of module.
		mask.c_str(), // Name of symbols to match.
		[](
			PSYMBOL_INFO pSymInfo,   
			ULONG SymbolSize,      
			PVOID UserContext
		) -> BOOL {
			auto &symbolInfo = *static_cast<SymbolInfo*>(UserContext);
			symbolInfo.rva = pSymInfo->Address -pSymInfo->ModBase;

			IMAGEHLP_LINE64 line;
			DWORD dwDisplacement;
			if(SymGetLineFromAddr(symbolInfo.symbolHandler.m_hProcess,(DWORD64)testt/*pSymInfo->Address*/,&dwDisplacement,&line) == TRUE)
			{
				std::cout<<"LINE: "<<line.FileName<<": "<<line.LineNumber<<std::endl;
			}
			else
				std::cout<<GetLastErrorAsString()<<std::endl;
			return TRUE;
		}, // Symbol handler procedure.
		&symbolInfo
	);
	return symbolInfo.rva;
#endif
}
#endif
