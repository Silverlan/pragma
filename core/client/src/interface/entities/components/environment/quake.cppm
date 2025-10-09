// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "sharedutils/functioncallback.h"
#include "pragma/lua/luaapi.h"




export module pragma.client:entities.components.env_quake;

import :entities.base_entity;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CQuakeComponent final : public BaseEnvQuakeComponent, public CBaseNetComponent {
		public:
			CQuakeComponent(BaseEntity &ent) : BaseEnvQuakeComponent(ent) {}
			virtual ~CQuakeComponent() override;

			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		protected:
			CallbackHandle m_cbScreenShake;
			virtual void StartShake() override;
			void CancelScreenShake();
		};
	};

	class DLLCLIENT CEnvQuake : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
