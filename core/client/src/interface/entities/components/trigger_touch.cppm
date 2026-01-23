// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.trigger_touch;

export import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CTouchComponent final : public BaseTouchComponent {
	  public:
		CTouchComponent(ecs::BaseEntity &ent) : BaseTouchComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntitySpawn() override;
	};
};

export class DLLCLIENT CTriggerTouch : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
