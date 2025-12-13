// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :physics.surface_type_manager;

pragma::physics::SurfaceType::SurfaceType(util::TypeId id, const std::string &name) : BaseNamedType {id, name} {}
