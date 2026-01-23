// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:util.type_manager;

export import pragma.util;

export {
	namespace pragma::util {
		using TypeId = uint32_t;
		class BaseNamedType;
		using TypeId = uint32_t;
		template<class TType>
		class TTypeManager {
		  public:
			TTypeManager() = default;
			virtual ~TTypeManager() = default;
			const std::vector<TSharedHandle<TType>> &GetRegisteredTypes() const { return m_registeredTypes; }
			template<typename... TARGS>
			TWeakSharedHandle<TType> RegisterType(const std::string &name, TARGS &&...args);
			TWeakSharedHandle<TType> GetType(TypeId id) const;
			TWeakSharedHandle<TType> FindType(const std::string &name) const;
		  protected:
			TTypeManager(const TTypeManager &) = delete;
			TTypeManager &operator=(const TTypeManager &) = delete;
		  private:
			std::vector<TSharedHandle<TType>> m_registeredTypes = {};
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

	namespace pragma::util {
		template<class TType>
		template<typename... TARGS>
		TWeakSharedHandle<TType> TTypeManager<TType>::RegisterType(const std::string &name, TARGS &&...args)
		{
			auto hType = FindType(name);
			if(hType.GetRawPtr())
				return hType;
			auto id = m_registeredTypes.size();
			m_typeNameToId[name] = id;
			m_registeredTypes.push_back(pragma::util::TSharedHandle<TType> {new TType {static_cast<TypeId>(id), name, std::forward<TARGS>(args)...}});
			return pragma::util::TWeakSharedHandle<TType> {m_registeredTypes.back()};
		}

		template<class TType>
		TWeakSharedHandle<TType> TTypeManager<TType>::GetType(TypeId id) const
		{
			return (id < m_registeredTypes.size()) ? m_registeredTypes.at(id) : pragma::util::TWeakSharedHandle<TType> {};
		}

		template<class TType>
		TWeakSharedHandle<TType> TTypeManager<TType>::FindType(const std::string &name) const
		{
			auto it = m_typeNameToId.find(name);
			return (it != m_typeNameToId.end()) ? pragma::util::TWeakSharedHandle<TType> {m_registeredTypes.at(it->second)} : pragma::util::TWeakSharedHandle<TType> {};
		}
	}
};
