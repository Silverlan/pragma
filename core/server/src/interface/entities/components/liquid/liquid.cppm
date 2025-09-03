// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/liquid/base_liquid_component.hpp>

export module pragma.server.entities.components.liquid;

export {
	namespace pragma {
		class DLLSERVER SLiquidComponent final : public BaseFuncLiquidComponent, public SBaseNetComponent {
		public:
			SLiquidComponent(BaseEntity &ent);
			virtual ~SLiquidComponent() override;
			virtual void Initialize() override;
			virtual void OnEntitySpawn() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		protected:
			CallbackHandle m_cbGameInitialized = {};
		};
	};

	class DLLSERVER FuncWater : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
