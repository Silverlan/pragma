// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "mathutil/transform.hpp"

#include "pragma/lua/luaapi.h"

#include "mathutil/umath.h"

#include "stdafx_client.h"

module pragma.client;

import :entities.components.static_bvh_cache;
import :engine;


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
