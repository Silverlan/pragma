// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:physics.ragdoll;

export import :physics.object;

export namespace pragma::physics {
	class DLLNETWORK Ragdoll : public PhysObj {
	public:
		Ragdoll();
	};
}
