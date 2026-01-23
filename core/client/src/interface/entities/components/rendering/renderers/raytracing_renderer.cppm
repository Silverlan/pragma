// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.raytracing_renderer;

import :entities.base_entity;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CRaytracingRendererComponent final : public BaseEntityComponent {
		  public:
			CRaytracingRendererComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		  private:
		};
	};

	class DLLCLIENT CRaytracingRenderer : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
