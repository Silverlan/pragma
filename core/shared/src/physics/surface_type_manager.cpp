/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/physics/surface_type_manager.hpp"

pragma::physics::SurfaceType::SurfaceType(TypeId id, const std::string &name) : BaseNamedType {id, name} {}
