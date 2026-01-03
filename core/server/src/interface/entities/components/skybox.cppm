// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.skybox;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SSkyboxComponent final : public BaseSkyboxComponent, public SBaseNetComponent {
		  public:
			SSkyboxComponent(ecs::BaseEntity &ent) : BaseSkyboxComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual void SetSkyAngles(const EulerAngles &ang) override;

			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLSERVER Skybox : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
