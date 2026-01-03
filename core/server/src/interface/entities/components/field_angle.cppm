// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.field_angle;

import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SFieldAngleComponent final : public BaseFieldAngleComponent, public SBaseNetComponent {
		  public:
			SFieldAngleComponent(ecs::BaseEntity &ent) : BaseFieldAngleComponent(ent) {}
			virtual void Initialize() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};
};
