// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include "pragma/entities/components/s_entity_component.hpp"

export module pragma.server.entities.components.lights.directional;

export {
	namespace pragma {
		class DLLSERVER SLightDirectionalComponent final : public BaseEnvLightDirectionalComponent, public SBaseNetComponent {
		public:
			SLightDirectionalComponent(BaseEntity &ent) : BaseEnvLightDirectionalComponent(ent) {};
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void SetAmbientColor(const Color &color) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		};
	};

	class DLLSERVER EnvLightDirectional : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
