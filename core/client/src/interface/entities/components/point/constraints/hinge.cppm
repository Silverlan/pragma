// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.client:entities.components.point_constraint_hinge;

import :entities.base_entity;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CPointConstraintHingeComponent final : public BasePointConstraintHingeComponent, public CBaseNetComponent {
		public:
			CPointConstraintHingeComponent(BaseEntity &ent) : BasePointConstraintHingeComponent(ent) {}
			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua_State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		};
	};

	class DLLCLIENT CPointConstraintHinge : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
