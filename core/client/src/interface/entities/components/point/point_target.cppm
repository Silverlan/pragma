// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.point_target;

export import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CPointTargetComponent final : public BasePointTargetComponent {
		  public:
			CPointTargetComponent(ecs::BaseEntity &ent) : BasePointTargetComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLCLIENT CPointTarget : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
