// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.env_wind;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CWindComponent final : public BaseEnvWindComponent, public CBaseNetComponent {
		  public:
			CWindComponent(ecs::BaseEntity &ent) : BaseEnvWindComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CEnvWind : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
