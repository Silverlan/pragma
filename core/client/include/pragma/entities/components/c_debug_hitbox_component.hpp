/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __C_DEBUG_HITBOX_COMPONENT_HPP__
#define __C_DEBUG_HITBOX_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_entity_component.hpp>

namespace DebugRenderer {
	class BaseObject;
};
namespace pragma {
	class DLLCLIENT CDebugHitboxComponent final : public BaseEntityComponent {
	  public:
		CDebugHitboxComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual ~CDebugHitboxComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		virtual void OnTick(double tDelta) override;

		void SetHitboxColor(BoneId boneId, const std::optional<Color> &color);
	  protected:
		void InitializeDebugObjects();
		void ClearDebugObjects();
		std::vector<std::shared_ptr<DebugRenderer::BaseObject>> m_debugObjects;
		std::unordered_map<BoneId, Color> m_hitboxColors;
		bool m_dirty = false;
	};
};

#endif
