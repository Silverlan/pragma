// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.gamemode;

export import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CGamemodeComponent final : public BaseGamemodeComponent {
	  public:
		CGamemodeComponent(ecs::BaseEntity &ent) : BaseGamemodeComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};

export class DLLCLIENT CGamemode : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
