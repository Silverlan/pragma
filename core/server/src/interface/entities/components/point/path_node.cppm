// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.point.path_node;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SPathNodeComponent final : public BasePointPathNodeComponent {
		  public:
			SPathNodeComponent(ecs::BaseEntity &ent) : BasePointPathNodeComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER PointPathNode : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
