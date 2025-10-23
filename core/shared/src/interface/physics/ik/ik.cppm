// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include "pragma/buss_ik/Node.h"

export module pragma.shared:physics.ik;

export import pragma.math;

export namespace util {
	namespace ik {
		DLLNETWORK void get_local_transform(const Node &node, umath::Transform &t);
		DLLNETWORK Quat get_rotation(const Node &node);
	};
};
