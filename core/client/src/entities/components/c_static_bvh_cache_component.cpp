/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_static_bvh_cache_component.hpp"
#include "pragma/entities/components/c_static_bvh_user_component.hpp"
#include "pragma/entities/components/c_bvh_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/model/modelmesh.h"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

void CStaticBvhCacheComponent::InitializeLuaObject(lua_State *l) { return BaseStaticBvhCacheComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CStaticBvhCacheComponent::Initialize() { BaseStaticBvhCacheComponent::Initialize(); }

void CStaticBvhCacheComponent::DoRebuildBvh() {}

void CStaticBvhCacheComponent::TestRebuildBvh()
{
	std::vector<std::shared_ptr<ModelSubMesh>> meshes;
	std::vector<BaseEntity *> meshToEntity;
	std::vector<umath::ScaledTransform> meshPoses;
	for(auto *c : m_entities) {
		auto &ent = c->GetEntity();
		auto *mdlC = static_cast<CModelComponent *>(ent.GetModelComponent());
		if(!mdlC)
			continue;
		auto &renderMeshes = mdlC->GetRenderMeshes();
		if(meshes.size() == meshes.capacity()) {
			meshes.reserve(meshes.size() * 1.5 + renderMeshes.size() + 100);
			meshToEntity.reserve(meshes.size());
			meshPoses.reserve(meshes.size());
		}
		auto &pose = ent.GetPose();
		for(auto &mesh : renderMeshes) {
			if(!ShouldConsiderMesh(*mesh))
				continue;
			meshes.push_back(mesh);
			meshToEntity.push_back(&ent);
			meshPoses.push_back(pose);
		}
	}

	Build(std::move(meshes), std::move(meshToEntity), std::move(meshPoses));
}
