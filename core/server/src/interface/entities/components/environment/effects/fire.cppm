// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.effects.fire;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SFireComponent final : public BaseEnvFireComponent {
		  public:
			SFireComponent(ecs::BaseEntity &ent) : BaseEnvFireComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvFire : public SBaseEntity {
	  protected:
	  public:
		virtual void Initialize() override;
	};
};
