/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */
#ifdef _MSC_VER
#include "stdafx_shared.h"

#ifdef ENABLE_PDB_MANAGER

#include "pragma/lua/lad/pdb_manager.hpp"
#include <sharedutils/scope_guard.h>
#include <dia2.h>

#undef max

using namespace pragma::lua;

static std::string ConvertWCSToMBS(const wchar_t *pstr, long wslen)
{
	int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

	std::string dblstr(len, '\0');
	len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */, pstr, wslen /* not necessary NULL-terminated */, &dblstr[0], len, NULL, NULL /* no default char */
	);

	return dblstr;
}

static std::string ConvertBSTRToMBS(BSTR bstr)
{
	int wslen = ::SysStringLen(bstr);
	return ConvertWCSToMBS((wchar_t *)bstr, wslen);
}

bool Symbol::operator==(const Symbol &other) const { return m_symbol == other.m_symbol; }

std::optional<std::string> Symbol::GetSourceFile(uint32_t &outOptLine) const
{
	BSTR fileName;
	if(m_symbol->get_sourceFileName(&fileName) != S_OK)
		return {};
	auto strFileName = ConvertBSTRToMBS(fileName);
	SysFreeString(fileName);
	outOptLine = 0;
	return strFileName;
}

uint64_t Symbol::GetAddress() const
{
	DWORD rva = 0;
	if(m_symbol->get_relativeVirtualAddress(&rva) != S_OK)
		rva = 0;
	return rva;
}

SymTag Symbol::GetTag() const
{
	DWORD tag = SymTagNull;
	m_symbol->get_symTag(&tag);
	return static_cast<SymTag>(tag);
}

std::optional<std::string> Symbol::GetName() const
{
	BSTR name;
	if(m_symbol->get_name(&name) != S_OK)
		return {};
	auto strName = ConvertBSTRToMBS(name);
	auto p = strName.find('(');
	if(p != std::string::npos) {
		strName = strName.substr(p + 1);
		p = strName.find(')');
		if(p != std::string::npos)
			strName = strName.substr(0, p);
	}
	return strName;
}

Symbol::~Symbol() { Release(); }

void Symbol::Release()
{
	if(!m_symbol)
		return;
	m_symbol->Release();
	m_symbol = nullptr;
}

SymbolIterator::SymbolIterator(PdbManager &manager, PdbSession *session, IDiaSymbol *symbol) : m_manager {&manager}, m_session {session}, m_celt {symbol ? 0 : std::numeric_limits<decltype(m_celt)>::max()}, m_symbol {symbol ? std::make_shared<Symbol>(symbol) : nullptr} {}

bool SymbolIterator::operator==(const SymbolIterator &other) const
{
	return m_celt == other.m_celt; //m_symbol == other.m_symbol && m_celt == other.m_celt;
}

SymbolIterator &SymbolIterator::operator++()
{
	if(m_celt == std::numeric_limits<decltype(m_celt)>::max())
		return *this;
	m_symbol->Release();
	if(FAILED(m_session->enumSymbolsByAddr->Next(1, &m_symbol->m_symbol, &m_celt)) || m_symbol->m_symbol == nullptr) {
		m_celt = std::numeric_limits<decltype(m_celt)>::max();
		m_symbol = nullptr;
	}
	return *this;
}

/////////////////

PdbSession::~PdbSession()
{
	if(enumSymbolsByAddr)
		enumSymbolsByAddr->Release();
	if(enumTables)
		enumTables->Release();
	if(globalSymbol)
		globalSymbol->Release();
	if(session)
		session->Release();
	if(source)
		source->Release();
}
PdbManager::~PdbManager()
{
	m_pdbSessions.clear();
	CoUninitialize();
}
bool PdbManager::Initialize()
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED); // If failed, was probably already initialized, so we're ignoring the return value (we still have to call CoUnInitialize either way)
	return true;
}
SymbolIterator PdbManager::begin(const std::string &moduleName) const
{
	auto it = m_pdbSessions.find(moduleName);
	if(it == m_pdbSessions.end())
		return end();
	IDiaSymbol *symbol = nullptr;
	if(FAILED(it->second->enumSymbolsByAddr->symbolByAddr(1, 0, &symbol)))
		return end();
	return SymbolIterator {const_cast<PdbManager &>(*this), &*it->second, symbol};
}
SymbolIterator PdbManager::end() const { return SymbolIterator {const_cast<PdbManager &>(*this), nullptr, nullptr}; }

bool PdbManager::LoadPdb(const std::string &moduleName, const std::string &pdbPath)
{
	auto sessionInfo = std::unique_ptr<PdbSession> {new PdbSession {}};
	auto hr = CoCreateInstance(CLSID_DiaSource, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IDiaDataSource), (void **)&sessionInfo->source);
	if(FAILED(hr))
		return false;

	wchar_t wszFilename[_MAX_PATH];
	mbstowcs(wszFilename, pdbPath.data(), sizeof(wszFilename) / sizeof(wszFilename[0]));
	if(FAILED(sessionInfo->source->loadDataFromPdb(wszFilename))) {
		if(FAILED(sessionInfo->source->loadDataForExe(wszFilename, nullptr, nullptr)))
			return false;
	}

	if(FAILED(sessionInfo->source->openSession(&sessionInfo->session)))
		return false;
	if(FAILED(sessionInfo->session->get_globalScope(&sessionInfo->globalSymbol)))
		return false;
	if(FAILED(sessionInfo->session->getEnumTables(&sessionInfo->enumTables)))
		return false;
	if(FAILED(sessionInfo->session->getSymbolsByAddr(&sessionInfo->enumSymbolsByAddr)))
		return false;
	sessionInfo->SetValid();
	m_pdbSessions[moduleName] = std::move(sessionInfo);
	m_moduleNames.push_back(moduleName);
	return true;
}

std::optional<PdbManager::SymbolInfo> PdbManager::FindSymbolByRva(const std::string &moduleName, DWORD64 rva)
{
	auto it = m_pdbSessions.find(moduleName);
	if(it == m_pdbSessions.end() || !*it->second)
		return {};
	IDiaSymbol *symbol;
	auto &sessionInfo = *it->second;
	if(sessionInfo.session->findSymbolByRVA(rva, SymTagEnum::SymTagFunction, &symbol) != S_OK) {
		if(sessionInfo.session->findSymbolByRVA(rva, SymTagEnum::SymTagPublicSymbol, &symbol) != S_OK)
			return {};
		// Found as a public symbol; Try to find private symbol
		DWORD targetRva;
		if(symbol->get_targetRelativeVirtualAddress(&targetRva) != S_OK || sessionInfo.session->findSymbolByRVA(targetRva, SymTagEnum::SymTagFunction, &symbol) != S_OK)
			return {};
		rva = targetRva;
	}
	DWORD symTag;
	if(symbol->get_symTag(&symTag) != S_OK || symTag != SymTagFunction)
		return {};
	SymbolInfo symbolInfo {};

	BSTR name;
	if(symbol->get_name(&name) == S_OK)
		symbolInfo.name = ConvertBSTRToMBS(name);
	symbolInfo.rva = rva;

	ULONGLONG length = 0;
	if(symbol->get_length(&length) == S_OK) {
		IDiaEnumLineNumbers *lineNums[100];
		if(sessionInfo.session->findLinesByRVA(rva, length, lineNums) == S_OK) {
			auto &l = lineNums[0];
			IDiaLineNumber *lineNum;
			ULONG fetched = 0;
			for(uint8_t i = 0; i < 5; ++i) {
				if(l->Next(i, &lineNum, &fetched) == S_OK && fetched == 1) {
					DWORD l;
					IDiaSourceFile *srcFile;
					if(lineNum->get_sourceFile(&srcFile) == S_OK) {
						BSTR fileName;
						srcFile->get_fileName(&fileName);
						if(lineNum->get_lineNumber(&l) == S_OK) {
							symbolInfo.source = SymbolInfo::Source {};
							symbolInfo.source->fileName = ConvertBSTRToMBS(fileName);
							symbolInfo.source->line = l;
							break;
						}
					}
				}
			}
		}
	}
	symbolInfo.parameters = GetFunctionParameters(symbol, symbolInfo.returnValue);

	symbol->Release();
	return symbolInfo;
}

static std::string get_symbol_name(IDiaSymbol *pSymbol)
{
	BSTR bstrName = NULL;
	BSTR bstrUndName = NULL;
	if(pSymbol->get_name(&bstrName) != S_OK)
		return "";
	util::ScopeGuard sg {[&bstrName, &bstrUndName]() {
		if(bstrName)
			SysFreeString(bstrName);
		if(bstrUndName)
			SysFreeString(bstrUndName);
	}};
	if(pSymbol->get_undecoratedName(&bstrUndName) != S_OK)
		return ConvertBSTRToMBS(bstrName);
	if(wcscmp(bstrName, bstrUndName) == 0)
		return ConvertBSTRToMBS(bstrName);
	return ConvertBSTRToMBS(bstrUndName) + "(" + ConvertBSTRToMBS(bstrName) + ")";
}

std::optional<std::vector<ParameterInfo>> PdbManager::GetFunctionParameters(IDiaSymbol *symbol, std::optional<ParameterInfo> &outReturnValue)
{
	IDiaSymbol *typeSymbol;
	if(symbol->get_type(&typeSymbol) != S_OK)
		return {};
	util::ScopeGuard sg {[&typeSymbol]() { typeSymbol->Release(); }};

	DWORD argCount = 0;
	if(typeSymbol->get_count(&argCount) != S_OK)
		return {};
	std::vector<ParameterInfo> params {};
	params.reserve(argCount);
	IDiaEnumSymbols *children;
	if(typeSymbol->findChildren(SymTagEnum::SymTagFunctionArgType, NULL, nsNone, &children) != S_OK)
		return {};
	util::ScopeGuard sgChildren {[&children]() { children->Release(); }};

	IDiaSymbol *argSymbol;
	ULONG num = 0;
	while(children->Next(1, &argSymbol, &num) == S_OK) {
		IDiaSymbol *typeSym;
		if(argSymbol->get_type(&typeSym) == S_OK) {
			auto paramInfo = SymbolToParameterInfo(typeSym);
			if(paramInfo.has_value())
				params.push_back(std::move(*paramInfo));
			else
				return {};
			typeSym->Release();
		}
		argSymbol->Release();
	}

	IDiaSymbol *retSymbol;
	if(typeSymbol->get_type(&retSymbol) == S_OK) {
		auto paramInfo = SymbolToParameterInfo(retSymbol);
		if(paramInfo.has_value())
			outReturnValue = std::move(*paramInfo);
		else
			return {};
		retSymbol->Release();
	}

	return params;
}

std::optional<ParameterInfo> PdbManager::SymbolToParameterInfo(IDiaSymbol *pSymbol)
{
	// See https://github.com/KirillOsenkov/Dia2Dump/blob/master/PrintSymbol.cpp
	DWORD dwTag;
	if(pSymbol->get_symTag(&dwTag) != S_OK)
		return {};

	BSTR bstrName;
	if(pSymbol->get_name(&bstrName) != S_OK)
		bstrName = NULL;
	util::ScopeGuard sg {[&bstrName]() { SysFreeString(bstrName); }};

	ParameterInfo paramInfo {};
	BOOL bSet;
	if(dwTag != SymTagPointerType) {
		if((pSymbol->get_constType(&bSet) == S_OK) && bSet)
			paramInfo.flags |= ParameterInfo::Flags::Const;

		if((pSymbol->get_volatileType(&bSet) == S_OK) && bSet)
			paramInfo.flags |= ParameterInfo::Flags::Volatile;

		if((pSymbol->get_unalignedType(&bSet) == S_OK) && bSet)
			paramInfo.flags |= ParameterInfo::Flags::Unaligned;
	}

	switch(dwTag) {
	case SymTagUDT:
		{
			DWORD dwKind = 0;
			if(pSymbol->get_udtKind(&dwKind) == S_OK) {
				switch(dwKind) {
				case UdtKind::UdtStruct:
					paramInfo.flags |= ParameterInfo::Flags::Struct;
					break;
				case UdtKind::UdtClass:
					paramInfo.flags |= ParameterInfo::Flags::Class;
					break;
				case UdtKind::UdtUnion:
					paramInfo.flags |= ParameterInfo::Flags::Union;
					break;
				case UdtKind::UdtInterface:
					paramInfo.flags |= ParameterInfo::Flags::Interface;
					break;
				}
			}
			paramInfo.typeName = get_symbol_name(pSymbol);
			break;
		}
	case SymTagEnum:
		paramInfo.flags |= ParameterInfo::Flags::Enum;
		paramInfo.typeName = get_symbol_name(pSymbol);
		break;
	case SymTagPointerType:
		{
			paramInfo.flags |= ParameterInfo::Flags::Pointer;
			IDiaSymbol *pBaseType;
			if(pSymbol->get_type(&pBaseType) != S_OK) {
				paramInfo.typeName = get_symbol_name(pSymbol);
				return {};
			}

			auto optParamInfo = SymbolToParameterInfo(pBaseType);
			pBaseType->Release();
			if(!optParamInfo.has_value())
				return {};
			paramInfo = *optParamInfo;

			if((pSymbol->get_reference(&bSet) == S_OK) && bSet)
				paramInfo.flags |= ParameterInfo::Flags::Reference;
			else
				paramInfo.flags |= ParameterInfo::Flags::Pointer;

			if((pSymbol->get_constType(&bSet) == S_OK) && bSet)
				paramInfo.flags |= ParameterInfo::Flags::Const;

			if((pSymbol->get_volatileType(&bSet) == S_OK) && bSet)
				paramInfo.flags |= ParameterInfo::Flags::Volatile;

			if((pSymbol->get_unalignedType(&bSet) == S_OK) && bSet)
				paramInfo.flags |= ParameterInfo::Flags::Unaligned;
			break;
		}
	case SymTagBaseType:
		DWORD dwInfo;
		if(pSymbol->get_baseType(&dwInfo) != S_OK)
			return {};

		switch(dwInfo) {
		case btUInt:
			paramInfo.typeName = "unsigned ";
			// Fall through
		case btInt:
			ULONGLONG ulLen;
			if(pSymbol->get_length(&ulLen) != S_OK)
				return {};
			switch(ulLen) {
			case 1:
				if(dwInfo == btInt)
					paramInfo.typeName += "signed ";
				paramInfo.typeName += "char";
				break;

			case 2:
				paramInfo.typeName += "short";
				break;

			case 4:
				paramInfo.typeName += "int";
				break;

			case 8:
				paramInfo.typeName += "__int64";
				break;
			}
			dwInfo = 0xFFFFFFFF;
			break;
		case btFloat:
			switch(ulLen) {
			case 4:
				paramInfo.typeName += "float";
				break;
			case 8:
				paramInfo.typeName += "double";
				break;
			}
			dwInfo = 0xFFFFFFFF;
			break;
		}

		if(dwInfo == 0xFFFFFFFF)
			break;

		constexpr std::array<const char *, 32> rgBaseType {
		  "<NoType>", // btNoType = 0,
		  "void",     // btVoid = 1,
		  "char",     // btChar = 2,
		  "wchar_t",  // btWChar = 3,
		  "signed char", "unsigned char",
		  "int",          // btInt = 6,
		  "unsigned int", // btUInt = 7,
		  "float",        // btFloat = 8,
		  "<BCD>",        // btBCD = 9,
		  "bool",         // btBool = 10,
		  "short", "unsigned short",
		  "long",          // btLong = 13,
		  "unsigned long", // btULong = 14,
		  "__int8", "__int16", "__int32", "__int64", "__int128", "unsigned __int8", "unsigned __int16", "unsigned __int32", "unsigned __int64", "unsigned __int128",
		  "<currency>", // btCurrency = 25,
		  "<date>",     // btDate = 26,
		  "VARIANT",    // btVariant = 27,
		  "<complex>",  // btComplex = 28,
		  "<bit>",      // btBit = 29,
		  "BSTR",       // btBSTR = 30,
		  "HRESULT"     // btHresult = 31
		};
		paramInfo.typeName += rgBaseType[dwInfo];
		break;
	}
	return paramInfo;
}

#endif
#endif
