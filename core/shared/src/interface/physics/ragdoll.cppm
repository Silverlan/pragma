// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:physics.ragdoll;

export import :physics.object;

export class DLLNETWORK Ragdoll : public pragma::physics::PhysObj {
  public:
	Ragdoll();
};
