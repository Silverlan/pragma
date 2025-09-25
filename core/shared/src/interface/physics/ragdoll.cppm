// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/physics/physobj.h"

export module pragma.shared:physics.ragdoll;

export class DLLNETWORK Ragdoll : public PhysObj {
  public:
	Ragdoll();
};
