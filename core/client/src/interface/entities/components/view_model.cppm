// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.view_model;

export import :entities.base_entity;

export namespace pragma {
	class DLLCLIENT CViewModelComponent final : public BaseEntityComponent {
	  public:
		CViewModelComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		void SetViewModelOffset(const Vector3 &offset);
		const Vector3 &GetViewModelOffset() const;
		void SetViewFOV(float fov);
		float GetViewFOV() const;
		virtual void InitializeLuaObject(lua::State *l) override;

		BasePlayerComponent *GetPlayer();
		BaseWeaponComponent *GetWeapon();
	  protected:
		Vector3 m_viewModelOffset;
		float m_viewFov = std::numeric_limits<float>::quiet_NaN();
	};
};

export class DLLCLIENT CViewModel : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
