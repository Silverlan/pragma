// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENTITYFACTORIES_H__
#define __ENTITYFACTORIES_H__
#include "pragma/networkdefinitions.h"
#include <pragma/engine.h>

template<class T>
class EntityClassMap {
  private:
	std::unordered_map<std::string, T *(*)(void)> m_factories;
	std::unordered_map<size_t, std::string> m_classNames;
  public:
	void AddClass(std::string name, const std::type_info &info, T *(*fc)(void));
	void GetFactories(std::unordered_map<std::string, T *(*)(void)> **factories);
	bool GetClassName(const std::type_info &info, std::string *classname);
	T *(*FindFactory(std::string classname))();
};

template<class T>
void EntityClassMap<T>::AddClass(std::string name, const std::type_info &info, T *(*fc)(void))
{
	m_factories.insert(typename std::unordered_map<std::string, T *(*)(void)>::value_type(name, fc));
	m_classNames.insert(typename std::unordered_map<size_t, std::string>::value_type(info.hash_code(), name));
}

template<class T>
void EntityClassMap<T>::GetFactories(std::unordered_map<std::string, T *(*)(void)> **factories)
{
	*factories = &m_factories;
}

template<class T>
bool EntityClassMap<T>::GetClassName(const std::type_info &info, std::string *classname)
{
	std::unordered_map<size_t, std::string>::iterator i = m_classNames.find(info.hash_code());
	if(i == m_classNames.end())
		return false;
	*classname = i->second;
	return true;
}

template<class T>
T *(*EntityClassMap<T>::FindFactory(std::string classname))()
{
	typename std::unordered_map<std::string, T *(*)()>::iterator i = m_factories.find(classname);
	if(i == m_factories.end())
		return NULL;
	return i->second;
}

#define entfactory_newglobal_dec(dlldec, glname, baseclass)                                                                                                                                                                                                                                      \
	dlldec void Link##glname##EntityToFactory(std::string name, const std::type_info &info, baseclass *(*fc)(void));                                                                                                                                                                             \
	dlldec EntityClassMap<baseclass> *Get##glname##EntityClassMap();                                                                                                                                                                                                                             \
	class dlldec __reg_ent_##glname {                                                                                                                                                                                                                                                            \
	  public:                                                                                                                                                                                                                                                                                    \
		__reg_ent_##glname(std::string name, const std::type_info &info, baseclass *(*fc)(void))                                                                                                                                                                                                 \
		{                                                                                                                                                                                                                                                                                        \
			Link##glname##EntityToFactory(name, info, fc);                                                                                                                                                                                                                                       \
			delete this;                                                                                                                                                                                                                                                                         \
		}                                                                                                                                                                                                                                                                                        \
	};

#define entfactory_newglobal_def(dlldec, glname, baseclass)                                                                                                                                                                                                                                      \
	dlldec EntityClassMap<baseclass> *g_##glname##EntityFactories = NULL;                                                                                                                                                                                                                        \
	dlldec void Link##glname##EntityToFactory(std::string name, const std::type_info &info, baseclass *(*fc)(void))                                                                                                                                                                              \
	{                                                                                                                                                                                                                                                                                            \
		if(g_##glname##EntityFactories == NULL) {                                                                                                                                                                                                                                                \
			static EntityClassMap<baseclass> map;                                                                                                                                                                                                                                                \
			g_##glname##EntityFactories = &map;                                                                                                                                                                                                                                                  \
		}                                                                                                                                                                                                                                                                                        \
		g_##glname##EntityFactories->AddClass(name, info, fc);                                                                                                                                                                                                                                   \
	}                                                                                                                                                                                                                                                                                            \
	dlldec EntityClassMap<baseclass> *Get##glname##EntityClassMap()                                                                                                                                                                                                                              \
	{                                                                                                                                                                                                                                                                                            \
		return g_##glname##EntityFactories;                                                                                                                                                                                                                                                      \
	}

#endif
