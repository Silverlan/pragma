// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/entities/entity_property.hpp"

using namespace pragma;

EntityProperty::EntityProperty() : util::SimpleProperty<EntityProperty, EntityHandle>() {}
EntityProperty::EntityProperty(const EntityHandle &hEnt) : util::SimpleProperty<EntityProperty, EntityHandle>(hEnt) {}
