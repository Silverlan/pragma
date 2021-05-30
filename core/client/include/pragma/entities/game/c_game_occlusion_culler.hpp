/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_GAME_OCCLUSION_CULLER_HPP__
#define __C_GAME_OCCLUSION_CULLER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/rendering/occlusion_culling/c_occlusion_octree.hpp"
#include <pragma/entities/components/base_entity_component.hpp>

namespace pragma
{
	class DLLCLIENT COcclusionCullerComponent final
		: public BaseEntityComponent
	{
	public:
		COcclusionCullerComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;

		OcclusionOctree<CBaseEntity*> &GetOcclusionOctree();
		const OcclusionOctree<CBaseEntity*> &GetOcclusionOctree() const;

		void AddEntity(CBaseEntity &ent);
	private:
		std::shared_ptr<OcclusionOctree<CBaseEntity*>> m_occlusionOctree = nullptr;
		std::unordered_map<CBaseEntity*,std::vector<CallbackHandle>> m_callbacks {};
	};
};

class DLLCLIENT COcclusionCuller
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
