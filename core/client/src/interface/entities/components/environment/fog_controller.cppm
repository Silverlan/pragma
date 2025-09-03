// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_fog_controller.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/entities/components/c_entity_component.hpp"
#include <memory>

export module pragma.client.entities.components.env_fog_controller;

export {
	namespace pragma {
		class DLLCLIENT CFogControllerComponent final : public BaseEnvFogControllerComponent, public CBaseNetComponent {
		public:
			CFogControllerComponent(BaseEntity &ent) : BaseEnvFogControllerComponent(ent) {}
			virtual void Initialize() override;
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void OnRemove() override;

			virtual void SetFogStart(float start) override;
			virtual void SetFogEnd(float end) override;
			virtual void SetMaxDensity(float density) override;
			virtual void SetFogType(util::FogType type) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void OnEntitySpawn() override;
		protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
			WorldEnvironment::Fog &GetFog();
		};
	};

	class DLLCLIENT CEnvFogController : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
