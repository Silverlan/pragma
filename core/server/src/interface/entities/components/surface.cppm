// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.surface;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SSurfaceComponent final : public BaseSurfaceComponent, public SBaseNetComponent {
	  public:
		SSurfaceComponent(pragma::ecs::BaseEntity &ent) : BaseSurfaceComponent(ent) {}
		virtual void SetPlane(const umath::Plane &plane) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
