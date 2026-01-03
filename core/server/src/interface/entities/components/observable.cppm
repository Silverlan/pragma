// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.observable;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SObservableComponent final : public BaseObservableComponent, public SBaseNetComponent {
	  public:
		SObservableComponent(ecs::BaseEntity &ent) : BaseObservableComponent(ent) {}

		virtual void SetLocalCameraOrigin(CameraType type, const Vector3 &origin) override;
		virtual void SetLocalCameraOffset(CameraType type, const Vector3 &offset) override;
		virtual void SetViewOffset(const Vector3 &offset) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
