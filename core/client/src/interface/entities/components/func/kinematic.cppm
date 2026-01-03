// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.func_kinematic;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CKinematicComponent final : public BaseFuncKinematicComponent, public CBaseNetComponent {
		  public:
			CKinematicComponent(ecs::BaseEntity &ent) : BaseFuncKinematicComponent(ent) {}
			virtual void Initialize() override;
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
			virtual void OnEntitySpawn() override;
		  protected:
			bool m_bInitiallyMoving = false;
		};
	};

	class DLLCLIENT CFuncKinematic : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
