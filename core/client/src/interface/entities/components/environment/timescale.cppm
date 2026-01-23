// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.env_timescale;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CEnvTimescaleComponent final : public BaseEnvTimescaleComponent, public CBaseNetComponent {
		  public:
			CEnvTimescaleComponent(ecs::BaseEntity &ent) : BaseEnvTimescaleComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CEnvTimescale : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
