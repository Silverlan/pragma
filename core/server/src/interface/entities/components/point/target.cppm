// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.point.target;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SPointTargetComponent final : public BasePointTargetComponent {
		  public:
			SPointTargetComponent(ecs::BaseEntity &ent) : BasePointTargetComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER PointTarget : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
