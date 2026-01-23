// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.flashlight;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SFlashlightComponent final : public BaseFlashlightComponent {
		  public:
			SFlashlightComponent(ecs::BaseEntity &ent) : BaseFlashlightComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER Flashlight : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
