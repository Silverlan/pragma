// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.wheel;

import :entities.components.entity;
import :entities;

export {
	namespace pragma {
		class DLLSERVER SWheelComponent final : public BaseWheelComponent, public SBaseSnapshotComponent {
		  public:
			SWheelComponent(ecs::BaseEntity &ent) : BaseWheelComponent(ent) {}
			virtual ~SWheelComponent() override;
			virtual void Initialize() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void SendSnapshotData(NetPacket &packet, BasePlayerComponent &pl) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual bool ShouldTransmitSnapshotData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};
	class DLLSERVER SWheel : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
