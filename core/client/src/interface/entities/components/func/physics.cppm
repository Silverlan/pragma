// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.func_physics;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CFuncPhysicsComponent final : public BaseFuncPhysicsComponent, public CBaseNetComponent {
		  public:
			CFuncPhysicsComponent(ecs::BaseEntity &ent) : BaseFuncPhysicsComponent(ent) {}
			virtual void Initialize() override;
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		  protected:
			virtual void OnEntitySpawn() override;
		};
	};

	class DLLCLIENT CFuncPhysics : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
