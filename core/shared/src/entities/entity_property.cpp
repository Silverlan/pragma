#include "stdafx_shared.h"
#include "pragma/entities/entity_property.hpp"

using namespace pragma;

EntityProperty::EntityProperty()
	: util::SimpleProperty<EntityProperty,EntityHandle>()
{}
EntityProperty::EntityProperty(const EntityHandle &hEnt)
	: util::SimpleProperty<EntityProperty,EntityHandle>(hEnt)
{}
