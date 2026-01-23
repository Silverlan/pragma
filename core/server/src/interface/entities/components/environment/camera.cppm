// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.camera;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SCameraComponent final : public BaseEnvCameraComponent {
		  public:
			SCameraComponent(ecs::BaseEntity &ent) : BaseEnvCameraComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		};
	};

	class DLLSERVER EnvCamera : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
