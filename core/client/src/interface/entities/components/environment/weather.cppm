// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.env_weather;

export import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CWeatherComponent final : public BaseEnvWeatherComponent {
		  public:
			CWeatherComponent(ecs::BaseEntity &ent) : BaseEnvWeatherComponent(ent) {}
			virtual ~CWeatherComponent() override;
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual void OnEntitySpawn() override;
		  protected:
			EntityHandle m_hParticle = {};
		};
	};

	class DLLCLIENT CEnvWeather : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
