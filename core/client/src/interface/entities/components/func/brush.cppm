// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:entities.components.func_brush;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CBrushComponent final : public BaseFuncBrushComponent, public CBaseNetComponent {
		  public:
			CBrushComponent(pragma::ecs::BaseEntity &ent) : BaseFuncBrushComponent(ent) {}
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
