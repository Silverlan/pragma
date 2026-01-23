// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.effects.sprite;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SSpriteComponent final : public BaseEnvSpriteComponent, public SBaseNetComponent {
		  public:
			SSpriteComponent(ecs::BaseEntity &ent) : BaseEnvSpriteComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvSprite : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
