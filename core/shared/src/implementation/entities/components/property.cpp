// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.property;

using namespace pragma;

EntityProperty::EntityProperty() : SimpleProperty<EntityProperty, EntityHandle>() {}
EntityProperty::EntityProperty(const EntityHandle &hEnt) : SimpleProperty<EntityProperty, EntityHandle>(hEnt) {}
