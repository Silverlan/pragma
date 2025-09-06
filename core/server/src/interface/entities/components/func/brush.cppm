// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/func/basefuncbrush.h"
#include "pragma/entities/components/s_entity_component.hpp"

export module pragma.server.entities.components.func.brush;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER SBrushComponent final : public BaseFuncBrushComponent, public SBaseNetComponent {
		public:
			SBrushComponent(BaseEntity &ent) : BaseFuncBrushComponent(ent) {}
			virtual void Initialize() override;
			virtual void OnEntitySpawn() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER FuncBrush : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
