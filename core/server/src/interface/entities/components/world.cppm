// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.world;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SWorldComponent final : public BaseWorldComponent {
		  public:
			SWorldComponent(ecs::BaseEntity &ent) : BaseWorldComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER World : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
