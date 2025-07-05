// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __UTIL_TYPED_MANAGER_HPP__
#define __UTIL_TYPED_MANAGER_HPP__

#include "pragma/networkdefinitions.h"
#include <sharedutils/util_shared_handle.hpp>
#include <cinttypes>
#include <type_traits>
#include <string>

namespace pragma {
	using TypeId = uint32_t;
	class BaseNamedType;
	using TypeId = uint32_t;
	template<class TType>
	class TTypeManager {
	  public:
		TTypeManager() = default;
		virtual ~TTypeManager() = default;
		const std::vector<::util::TSharedHandle<TType>> &GetRegisteredTypes() const { return m_registeredTypes; }
		template<typename... TARGS>
		::util::TWeakSharedHandle<TType> RegisterType(const std::string &name, TARGS &&...args);
		::util::TWeakSharedHandle<TType> GetType(TypeId id) const;
		::util::TWeakSharedHandle<TType> FindType(const std::string &name) const;
	  protected:
		TTypeManager(const TTypeManager &) = delete;
		TTypeManager &operator=(const TTypeManager &) = delete;
	  private:
		std::vector<::util::TSharedHandle<TType>> m_registeredTypes = {};
		std::unordered_map<std::string, TypeId> m_typeNameToId = {};
	};

	class DLLNETWORK BaseNamedType {
	  public:
		virtual ~BaseNamedType() = default;
		TypeId GetId() const;
		const std::string &GetName() const;
	  protected:
		BaseNamedType(TypeId id, const std::string &name);
	  private:
		TypeId m_id = 0u;
		std::string m_name = "";
	};
};

template<class TType>
template<typename... TARGS>
::util::TWeakSharedHandle<TType> pragma::TTypeManager<TType>::RegisterType(const std::string &name, TARGS &&...args)
{
	auto hType = FindType(name);
	if(hType.GetRawPtr())
		return hType;
	auto id = m_registeredTypes.size();
	m_typeNameToId[name] = id;
	m_registeredTypes.push_back(::util::TSharedHandle<TType> {new TType {static_cast<TypeId>(id), name, std::forward<TARGS>(args)...}});
	return ::util::TWeakSharedHandle<TType> {m_registeredTypes.back()};
}

template<class TType>
::util::TWeakSharedHandle<TType> pragma::TTypeManager<TType>::GetType(TypeId id) const
{
	return (id < m_registeredTypes.size()) ? m_registeredTypes.at(id) : ::util::TWeakSharedHandle<TType> {};
}

template<class TType>
::util::TWeakSharedHandle<TType> pragma::TTypeManager<TType>::FindType(const std::string &name) const
{
	auto it = m_typeNameToId.find(name);
	return (it != m_typeNameToId.end()) ? ::util::TWeakSharedHandle<TType> {m_registeredTypes.at(it->second)} : ::util::TWeakSharedHandle<TType> {};
}

#endif
