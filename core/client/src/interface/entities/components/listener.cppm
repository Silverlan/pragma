// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.listener;

export import :entities.base_entity;
export import pragma.soundsystem;

export namespace pragma {
	class DLLCLIENT CListenerComponent final : public BaseEntityComponent {
	  public:
		CListenerComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		CListenerComponent();
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
		float GetGain();
		void SetGain(float gain);
		virtual void InitializeLuaObject(lua::State *l) override;
	  private:
		audio::IListener *m_listener = nullptr;
	};
};

export class DLLCLIENT CListener : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
