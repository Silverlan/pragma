// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/buss_ik/Node.h"
#include "mathutil/transform.hpp"

export module pragma.shared:physics.ik;

import :physics.ik.controller;
import :physics.ik.method;

export namespace util {
	namespace ik {
		DLLNETWORK void get_local_transform(const Node &node, umath::Transform &t);
		DLLNETWORK Quat get_rotation(const Node &node);
	};
};
