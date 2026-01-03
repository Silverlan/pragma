// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.flashlight;

export import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CFlashlightComponent final : public BaseFlashlightComponent {
		  public:
			CFlashlightComponent(ecs::BaseEntity &ent) : BaseFlashlightComponent(ent) {}
			virtual void Initialize() override;
			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLCLIENT CFlashlight : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
