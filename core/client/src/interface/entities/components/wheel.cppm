// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.wheel;

export import :entities.base_entity;
export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CWheelComponent final : public BaseWheelComponent, public CBaseSnapshotComponent {
	  public:
		CWheelComponent(ecs::BaseEntity &ent) : BaseWheelComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void ReceiveSnapshotData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual bool ShouldTransmitSnapshotData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};

export class DLLCLIENT CWheel : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
