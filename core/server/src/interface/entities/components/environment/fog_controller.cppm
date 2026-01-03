// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.fog_controller;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SFogControllerComponent final : public BaseEnvFogControllerComponent, public SBaseNetComponent {
		  public:
			SFogControllerComponent(ecs::BaseEntity &ent) : BaseEnvFogControllerComponent(ent) {}
			virtual void SetFogStart(float start) override;
			virtual void SetFogEnd(float end) override;
			virtual void SetMaxDensity(float density) override;
			virtual void SetFogType(util::FogType type) override;

			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvFogController : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
