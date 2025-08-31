// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/func/basefuncbrush.h"
#include "pragma/entities/components/c_entity_component.hpp"

import pragma.entities.components;

export module pragma.client.entities.components:func_brush;

export {
	namespace pragma {
		class DLLCLIENT CBrushComponent final : public BaseFuncBrushComponent, public CBaseNetComponent {
		public:
			CBrushComponent(BaseEntity &ent) : BaseFuncBrushComponent(ent) {}
			virtual void Initialize() override;
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void OnEntitySpawn() override;
		};
	};

	class DLLCLIENT CFuncBrush : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
}
