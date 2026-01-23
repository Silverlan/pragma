// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.point_at_target;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SPointAtTargetComponent final : public BasePointAtTargetComponent, public SBaseNetComponent {
	  public:
		SPointAtTargetComponent(ecs::BaseEntity &ent) : BasePointAtTargetComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;

		using BasePointAtTargetComponent::SetPointAtTarget;
	  protected:
		virtual void SetPointAtTarget(ecs::BaseEntity *ent) override;
	};
};
