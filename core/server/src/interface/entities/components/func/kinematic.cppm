// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.func.kinematic;

import :entities;

import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SKinematicComponent final : public BaseFuncKinematicComponent, public SBaseNetComponent {
		  public:
			SKinematicComponent(ecs::BaseEntity &ent) : BaseFuncKinematicComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void StartForward() override;
			virtual void StartBackward() override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER FuncKinematic : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
