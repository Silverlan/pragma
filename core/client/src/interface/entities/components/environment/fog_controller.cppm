// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.env_fog_controller;

export import :entities.base_entity;
export import :entities.components.entity;
import :rendering.world_environment;

export {
	namespace pragma {
		class DLLCLIENT CFogControllerComponent final : public BaseEnvFogControllerComponent, public CBaseNetComponent {
		  public:
			CFogControllerComponent(ecs::BaseEntity &ent) : BaseEnvFogControllerComponent(ent) {}
			virtual void Initialize() override;
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void OnRemove() override;

			virtual void SetFogStart(float start) override;
			virtual void SetFogEnd(float end) override;
			virtual void SetMaxDensity(float density) override;
			virtual void SetFogType(util::FogType type) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void OnEntitySpawn() override;
		  protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
			rendering::WorldEnvironment::Fog &GetFog();
		};
	};

	class DLLCLIENT CEnvFogController : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
