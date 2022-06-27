/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_bvh_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/model/c_modelmesh.h"

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

void CBvhComponent::InitializeLuaObject(lua_State *l) {return BaseBvhComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

void CBvhComponent::Initialize()
{
	BaseBvhComponent::Initialize();

	BindEventUnhandled(CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		RebuildBvh();
	});
	BindEventUnhandled(CAnimatedComponent::EVENT_ON_ANIMATIONS_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		m_bvhDirty = true;
	});
	if(GetEntity().IsSpawned())
		RebuildBvh();
}

bool CBvhComponent::IntersectionTest(
	const Vector3 &origin,const Vector3 &dir,float minDist,float maxDist,
	BvhHitInfo &outHitInfo
) const
{
	if(m_bvhDirty)
	{
		const_cast<CBvhComponent*>(this)->RebuildAnimatedBvh();
		const_cast<CBvhComponent*>(this)->m_bvhDirty = false;
	}
	return BaseBvhComponent::IntersectionTest(origin,dir,minDist,maxDist,outHitInfo);
}

void CBvhComponent::RebuildBvh()
{
	ClearBvh();
	auto *mdlC = static_cast<CModelComponent*>(GetEntity().GetModelComponent());
	if(!mdlC)
		return;
	auto &renderMeshes = mdlC->GetRenderMeshes();
	BaseBvhComponent::RebuildBvh(renderMeshes);
	RebuildAnimatedBvh();
}
