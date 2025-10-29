// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <ostream>

#include <string>

module pragma.shared;

import :physics.surface_type_manager;

pragma::physics::SurfaceType::SurfaceType(TypeId id, const std::string &name) : BaseNamedType {id, name} {}
