/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LUA_ENTITY_ITERATOR_HPP__
#define __LUA_ENTITY_ITERATOR_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/entities/entity_iterator.hpp"

class LuaBaseEntityIterator
{
public:
	LuaBaseEntityIterator(const BaseEntityIterator &iterator);
	LuaBaseEntityIterator(const LuaBaseEntityIterator &other);
	LuaBaseEntityIterator &operator=(const LuaBaseEntityIterator &other)=delete;
	LuaBaseEntityIterator &operator++();
	LuaBaseEntityIterator operator++(int);
	luabind::object operator*();
	luabind::object operator->();
	bool operator==(const LuaBaseEntityIterator &other);
	bool operator!=(const LuaBaseEntityIterator &other);
private:
	BaseEntityIterator m_iterator;
};

struct LuaEntityIteratorFilterBase
{
	virtual void Attach(EntityIterator &iterator)=0;
};
lua_registercheck(EntityIteratorFilter,LuaEntityIteratorFilterBase)

class LuaEntityIterator
{
public:
	LuaEntityIterator(lua_State *l,EntityIterator::FilterFlags filterFlags=EntityIterator::FilterFlags::Default);
	LuaEntityIterator(lua_State *l,pragma::ComponentId componentId,EntityIterator::FilterFlags filterFlags=EntityIterator::FilterFlags::Default);
	LuaEntityIterator(lua_State *l,const std::string &componentName,EntityIterator::FilterFlags filterFlags=EntityIterator::FilterFlags::Default);
	LuaBaseEntityIterator begin() const;
	LuaBaseEntityIterator end() const;
	void AttachFilter(LuaEntityIteratorFilterBase &filter);

	EntityIterator &GetIterator();
private:
	std::shared_ptr<EntityIterator> m_iterator = nullptr;
};

struct LuaEntityIteratorFilterName
	: public LuaEntityIteratorFilterBase
{
	LuaEntityIteratorFilterName(const std::string &name,bool caseSensitive=false,bool exactMatch=true);
	virtual void Attach(EntityIterator &iterator) override;
private:
	std::string m_name;
	bool m_bCaseSensitive = false;
	bool m_bExactMatch = true;
};

struct LuaEntityIteratorFilterModel
	: public LuaEntityIteratorFilterBase
{
	LuaEntityIteratorFilterModel(const std::string &mdlName);
	virtual void Attach(EntityIterator &iterator) override;
private:
	std::string m_modelName;
};

struct LuaEntityIteratorFilterUuid
	: public LuaEntityIteratorFilterBase
{
	LuaEntityIteratorFilterUuid(const std::string &uuid);
	virtual void Attach(EntityIterator &iterator) override;
private:
	std::string m_uuid;
};

struct LuaEntityIteratorFilterClass
	: public LuaEntityIteratorFilterBase
{
	LuaEntityIteratorFilterClass(const std::string &className,bool caseSensitive=false,bool exactMatch=true);
	virtual void Attach(EntityIterator &iterator) override;
private:
	std::string m_className;
	bool m_bCaseSensitive = false;
	bool m_bExactMatch = true;
};

struct LuaEntityIteratorFilterNameOrClass
	: public LuaEntityIteratorFilterBase
{
	LuaEntityIteratorFilterNameOrClass(const std::string &name,bool caseSensitive=false,bool exactMatch=true);
	virtual void Attach(EntityIterator &iterator) override;
private:
	std::string m_name;
	bool m_bCaseSensitive = false;
	bool m_bExactMatch = true;
};

struct LuaEntityIteratorFilterEntity
	: public LuaEntityIteratorFilterBase
{
	LuaEntityIteratorFilterEntity(const std::string &name);
	virtual void Attach(EntityIterator &iterator) override;
private:
	std::string m_name;
};

struct LuaEntityIteratorFilterSphere
	: public LuaEntityIteratorFilterBase
{
	LuaEntityIteratorFilterSphere(const Vector3 &origin,float radius);
	virtual void Attach(EntityIterator &iterator) override;
private:
	Vector3 m_origin;
	float m_radius = 0.f;
};

struct LuaEntityIteratorFilterBox
	: public LuaEntityIteratorFilterBase
{
	LuaEntityIteratorFilterBox(const Vector3 &min,const Vector3 &max);
	virtual void Attach(EntityIterator &iterator) override;
private:
	Vector3 m_min;
	Vector3 m_max;
};

struct LuaEntityIteratorFilterCone
	: public LuaEntityIteratorFilterBase
{
	LuaEntityIteratorFilterCone(const Vector3 &origin,const Vector3 &dir,float radius,float angle);
	virtual void Attach(EntityIterator &iterator) override;
private:
	Vector3 m_origin;
	Vector3 m_direction;
	float m_radius = 0.f;
	float m_angle = 0.f;
};

struct LuaEntityIteratorFilterComponent
	: public LuaEntityIteratorFilterBase
{
	LuaEntityIteratorFilterComponent(pragma::ComponentId componentId);
	LuaEntityIteratorFilterComponent(lua_State *l,const std::string &componentName);
	virtual void Attach(EntityIterator &iterator) override;
private:
	pragma::ComponentId m_componentId = pragma::INVALID_COMPONENT_ID;
};

#endif
