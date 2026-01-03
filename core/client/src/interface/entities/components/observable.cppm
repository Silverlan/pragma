// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.observable;

export import :entities.components.entity;

export namespace pragma {
	namespace cObservableComponent {
		using namespace baseObservableComponent;
	}
	class DLLCLIENT CObservableComponent final : public BaseObservableComponent, public CBaseNetComponent {
	  public:
		CObservableComponent(ecs::BaseEntity &ent);
		virtual void SetLocalCameraOrigin(CameraType type, const Vector3 &offset) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
