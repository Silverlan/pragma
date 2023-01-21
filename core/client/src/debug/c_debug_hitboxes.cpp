/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debugoverlay.h"
#include <pragma/model/model.h>
#include <pragma/console/sh_cmd.h>

extern DLLCLIENT CGame *c_game;

struct DebugHitboxes
{
	DebugHitboxes()=default;
	~DebugHitboxes();
	DebugHitboxes(const DebugHitboxes&)=delete;
	DebugHitboxes &operator=(const DebugHitboxes&)=delete;
	struct EntityObject
	{
		EntityHandle hEntity;
		std::vector<std::shared_ptr<DebugRenderer::BaseObject>> debugObjects;
	};
	std::vector<EntityObject> objects;
	CallbackHandle cbThink;
	CallbackHandle cbGameEnd;
};

DebugHitboxes::~DebugHitboxes()
{
	if(cbGameEnd.IsValid())
		cbGameEnd.Remove();
	if(cbThink.IsValid())
		cbThink.Remove();
	for(auto &o : objects)
	{
		for(auto &dbObj : o.debugObjects)
		{
			if(dbObj->IsValid())
				dbObj->Remove();
		}
	}
}

void Console::commands::debug_hitboxes(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	static std::unique_ptr<DebugHitboxes> debugHitboxes = nullptr;
	auto bWasValid = (debugHitboxes != nullptr) ? true : false;
	debugHitboxes = nullptr;
	if(c_game == nullptr || pl == nullptr)
		return;
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	if(charComponent.expired())
		return;
	auto ents = command::find_target_entity(state,*charComponent,argv);
	if(ents.empty())
	{
		if(bWasValid == true)
			return;
		Con::cwar<<"No entity targets found!"<<Con::endl;
		return;
	}
	debugHitboxes = std::make_unique<DebugHitboxes>();
	for(auto *ent : ents)
	{
		auto mdlComponent = ent->GetModelComponent();
		auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
		if(mdl == nullptr)
			return;
		auto boneIds = mdl->GetHitboxBones();
		if(!boneIds.empty())
		{
			debugHitboxes->objects.push_back(DebugHitboxes::EntityObject{});
			auto &entObj = debugHitboxes->objects.back();
			entObj.hEntity = ent->GetHandle();
			for(auto boneId : boneIds)
			{
				auto &hb = *mdl->GetHitbox(boneId);
				Vector3 origin,min,max;
				Quat rot;
				if(mdlComponent->GetHitboxBounds(boneId,min,max,origin,rot) == true)
				{
					Color col{255,255,255,255};
					switch(hb.group)
					{
						case HitGroup::Head:
							col = Color::Red;
							break;
						case HitGroup::Chest:
							col = Color::Lime;
							break;
						case HitGroup::Stomach:
							col = Color::Blue;
							break;
						case HitGroup::LeftArm:
							col = Color::Yellow;
							break;
						case HitGroup::RightArm:
							col = Color::Cyan;
							break;
						case HitGroup::LeftLeg:
							col = Color::Magenta;
							break;
						case HitGroup::RightLeg:
							col = Color::OrangeRed;
							break;
						case HitGroup::Gear:
							col = Color::SpringGreen;
							break;
						case HitGroup::Tail:
							col = Color::Violet;
							break;
					}
					entObj.debugObjects.push_back(::DebugRenderer::DrawBox(origin,min,max,rot,col));
				}
			}
		}
	}
	if(debugHitboxes->objects.empty())
	{
		debugHitboxes = nullptr;
		return;
	}
	debugHitboxes->cbThink = c_game->AddCallback("Think",FunctionCallback<>::Create([]() {
		if(debugHitboxes == nullptr)
			return;
		for(auto &o : debugHitboxes->objects)
		{
			auto &hEnt = o.hEntity;
			auto mdlComponent = hEnt.get()->GetModelComponent();
			auto mdl = mdlComponent ? mdlComponent->GetModel() : nullptr;
			if(mdl == nullptr)
				return;
			auto boneIds = mdl->GetHitboxBones();
			uint32_t objId = 0;
			auto numObjs = o.debugObjects.size();
			for(auto boneId : boneIds)
			{
				if(objId >= numObjs)
					break;
				auto &dbgObj = o.debugObjects[objId];
				Vector3 origin,min,max;
				Quat rot;
				if(mdlComponent->GetHitboxBounds(boneId,min,max,origin,rot) == true)
				{
					dbgObj->SetPos(origin);
					dbgObj->SetRotation(rot);
				}
				++objId;
			}
		}
	}));
	debugHitboxes->cbGameEnd = c_game->AddCallback("OnGameEnd",FunctionCallback<void,CGame*>::Create([](CGame*) {
		debugHitboxes = nullptr;
	}));
}
