// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/components/base_entity_component.hpp>
#include <unordered_set>

export module pragma.client.entities.components:raytracing_renderer;

export {
	namespace pragma {
		class DLLCLIENT CRaytracingRendererComponent final : public BaseEntityComponent {
		public:
			CRaytracingRendererComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
			virtual void InitializeLuaObject(lua_State *l) override;
		private:
		};
	};

	class DLLCLIENT CRaytracingRenderer : public CBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
