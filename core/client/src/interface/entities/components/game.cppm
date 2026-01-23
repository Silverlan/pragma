// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.game;

export import :entities.components.camera;

export namespace pragma {
	class DLLCLIENT CGameComponent final : public BaseGameComponent {
	  public:
		CGameComponent(ecs::BaseEntity &ent) : BaseGameComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
		void UpdateFrame(CCameraComponent *cam);
		void UpdateCamera(CCameraComponent *cam);
	  private:
	};
};

export class DLLCLIENT CGameEntity : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
