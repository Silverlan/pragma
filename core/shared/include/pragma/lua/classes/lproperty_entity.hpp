/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LPROPERTY_ENTITY_HPP__
#define __LPROPERTY_ENTITY_HPP__

#include "pragma/lua/classes/lproperty_generic.hpp"
#include "pragma/entities/entity_property.hpp"
// Entity
class LEntityPropertyWrapper
	: public LSimplePropertyWrapper<pragma::EntityProperty,EntityHandle>
{
public:
	using LSimplePropertyWrapper<pragma::EntityProperty,EntityHandle>::LSimplePropertyWrapper;
	using LSimplePropertyWrapper<pragma::EntityProperty,EntityHandle>::operator*;
	using LSimplePropertyWrapper<pragma::EntityProperty,EntityHandle>::operator->;
	virtual pragma::EntityProperty &GetProperty() const override {return *static_cast<pragma::EntityProperty*>(prop.get());}

	LEntityPropertyWrapper()
		: LSimplePropertyWrapper()
	{}
	LEntityPropertyWrapper(const EntityHandle &v)
		: LSimplePropertyWrapper(v)
	{}
};
using LEntityProperty = LEntityPropertyWrapper;

#endif
