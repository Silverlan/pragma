// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;



module pragma.shared;

import :entities.property;

using namespace pragma;

EntityProperty::EntityProperty() : util::SimpleProperty<EntityProperty, EntityHandle>() {}
EntityProperty::EntityProperty(const EntityHandle &hEnt) : util::SimpleProperty<EntityProperty, EntityHandle>(hEnt) {}
