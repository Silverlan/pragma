/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_AUTO_DOC_PDB_MANAGER_HPP__
#define __LUA_AUTO_DOC_PDB_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <optional>
#include <unordered_map>
#include <luasystem.h>

struct IDiaDataSource;
struct IDiaSession;
struct IDiaSymbol;
struct IDiaEnumTables;
struct IDiaEnumSymbolsByAddr;

namespace pragma::lua
{
	struct ParameterInfo
	{
		enum class Flags : uint16_t
		{
			None = 0u,
			Const = 1u,
			Reference = Const<<1u,
			Pointer = Reference<<1u,
			Volatile = Pointer<<1u,
			Unaligned = Volatile<<1u,

			Struct = Unaligned<<1u,
			Class = Struct<<1u,
			Union = Class<<1u,
			Interface = Union<<1u,
			Enum = Interface<<1u
		};
		std::string typeName;
		Flags flags = Flags::None;
	};

	enum SymTag
	{
		Null,
		Exe,
		Compiland,
		CompilandDetails,
		CompilandEnv,
		Function,
		Block,
		Data,
		Annotation,
		Label,
		PublicSymbol,
		UDT,
		Enum,
		FunctionType,
		PointerType,
		ArrayType,
		BaseType,
		Typedef,
		BaseClass,
		Friend,
		FunctionArgType,
		FuncDebugStart,
		FuncDebugEnd,
		UsingNamespace,
		VTableShape,
		VTable,
		Custom,
		Thunk,
		CustomType,
		ManagedType,
		Dimension,
		CallSite,
		InlineSite,
		BaseInterface,
		VectorType,
		MatrixType,
		HLSLType,
		Caller,
		Callee,
		Export,
		HeapAllocationSite,
		CoffGroup,
		Max
	};

	struct SymbolIterator;
	class Symbol
	{
	public:
		Symbol(IDiaSymbol *symbol=nullptr)
			: m_symbol{symbol}
		{}
		~Symbol();
		Symbol(const Symbol&)=delete;
		Symbol &operator=(const Symbol&)=delete;
		IDiaSymbol &GetSymbol() {return *m_symbol;}
		uint64_t GetAddress() const;
		SymTag GetTag() const;

		std::optional<std::string> GetSourceFile(uint32_t &outOptLine) const;
		std::optional<std::string> GetName() const;
		operator bool() const {return m_symbol != nullptr;}
		bool operator==(const Symbol &other) const;
		bool operator!=(const Symbol &other) const {return !operator==(other);}
	private:
		void Release();
		friend SymbolIterator;
		IDiaSymbol *m_symbol = nullptr;
	};

	class PdbManager;
	struct PdbSession;
	struct SymbolIterator
	{
		SymbolIterator(PdbManager &manager,PdbSession *session,IDiaSymbol *symbol=nullptr);
		SymbolIterator(const SymbolIterator&)=default;
		SymbolIterator &operator=(const SymbolIterator&)=default;
		using difference_type = long;
		using pointer = Symbol*;
		using reference = Symbol&;
		using iterator_category = std::forward_iterator_tag;

		reference operator*() {return *m_symbol;}
		pointer operator->() {return m_symbol.get();}

		// Prefix increment
		SymbolIterator& operator++();

		// Postfix increment
		SymbolIterator operator++(int) {SymbolIterator tmp = *this; ++(*this); return tmp;}

		bool operator==(const SymbolIterator &other) const;
		bool operator!=(const SymbolIterator &other) const {return !operator==(other);};
	private:
		PdbManager *m_manager = nullptr;
		PdbSession *m_session = nullptr;
		std::shared_ptr<Symbol> m_symbol = nullptr;
		ULONG m_celt = 0;
	};

	struct PdbSession
	{
		~PdbSession();
		std::string pdbPath;
		IDiaDataSource *source = nullptr;
		IDiaSession *session = nullptr;
		IDiaSymbol *globalSymbol = nullptr;
		IDiaEnumTables *enumTables = nullptr;
		IDiaEnumSymbolsByAddr *enumSymbolsByAddr = nullptr;
		operator bool() const {return m_valid;}
		void SetValid() {m_valid = true;}
	private:
		bool m_valid = false;
	};

	class PdbManager
	{
	public:
		struct SymbolInfo
		{
			struct Source
			{
				std::string fileName;
				uint32_t line;
			};
			std::string name;
			DWORD64 rva = 0;
			std::optional<Source> source {};
			std::optional<std::vector<ParameterInfo>> parameters {};
			std::optional<ParameterInfo> returnValue {};
		};

		~PdbManager();
		bool Initialize();
		bool LoadPdb(const std::string &moduleName,const std::string &pdbPath);
		const std::vector<std::string> &GetModuleNames() const {return m_moduleNames;}

        SymbolIterator begin(const std::string &moduleName) const;
        SymbolIterator end() const;

		std::optional<SymbolInfo> FindSymbolByRva(const std::string &moduleName,DWORD64 rva);
	private:
		static std::optional<ParameterInfo> SymbolToParameterInfo(IDiaSymbol *pSymbol);
		static std::optional<std::vector<ParameterInfo>> GetFunctionParameters(IDiaSymbol *pSymbol,std::optional<ParameterInfo> &outReturnValue);

		std::unordered_map<std::string,std::unique_ptr<PdbSession>> m_pdbSessions {};
		std::vector<std::string> m_moduleNames;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::lua::ParameterInfo::Flags)

#endif
