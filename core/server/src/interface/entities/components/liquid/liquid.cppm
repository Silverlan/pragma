// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.liquid;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SLiquidComponent final : public BaseFuncLiquidComponent, public SBaseNetComponent {
		  public:
			SLiquidComponent(ecs::BaseEntity &ent);
			virtual ~SLiquidComponent() override;
			virtual void Initialize() override;
			virtual void OnEntitySpawn() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			CallbackHandle m_cbGameInitialized = {};
		};
	};

	class DLLSERVER FuncWater : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
