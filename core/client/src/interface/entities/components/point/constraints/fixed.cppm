// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.point_constraint_fixed;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPointConstraintFixedComponent final : public BasePointConstraintFixedComponent, public CBaseNetComponent {
		  public:
			CPointConstraintFixedComponent(ecs::BaseEntity &ent) : BasePointConstraintFixedComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CPointConstraintFixed : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
