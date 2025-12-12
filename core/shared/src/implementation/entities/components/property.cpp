// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.property;

using namespace pragma;

EntityProperty::EntityProperty() : pragma::util::SimpleProperty<EntityProperty, EntityHandle>() {}
EntityProperty::EntityProperty(const EntityHandle &hEnt) : pragma::util::SimpleProperty<EntityProperty, EntityHandle>(hEnt) {}
