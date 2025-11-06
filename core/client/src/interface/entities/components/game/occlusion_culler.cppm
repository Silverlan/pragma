// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:entities.components.game_occlusion_culler;

export import :entities.base_entity;
export import :rendering.occlusion_culling.octree;

export namespace pragma {
	class DLLCLIENT COcclusionCullerComponent final : public BaseEntityComponent {
	  public:
		COcclusionCullerComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void Initialize() override;
		virtual void OnRemove() override;
		virtual void OnEntitySpawn() override;

		OcclusionOctree<CBaseEntity *> &GetOcclusionOctree();
		const OcclusionOctree<CBaseEntity *> &GetOcclusionOctree() const;

		void AddEntity(CBaseEntity &ent);
	  private:
		std::shared_ptr<OcclusionOctree<CBaseEntity *>> m_occlusionOctree = nullptr;
		std::unordered_map<CBaseEntity *, std::vector<CallbackHandle>> m_callbacks {};
	};
};

export class DLLCLIENT COcclusionCuller : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
