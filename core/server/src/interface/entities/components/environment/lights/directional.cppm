// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.lights.directional;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SLightDirectionalComponent final : public BaseEnvLightDirectionalComponent, public SBaseNetComponent {
		  public:
			SLightDirectionalComponent(ecs::BaseEntity &ent) : BaseEnvLightDirectionalComponent(ent) {};
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void SetAmbientColor(const Color &color) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		};
	};

	class DLLSERVER EnvLightDirectional : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
