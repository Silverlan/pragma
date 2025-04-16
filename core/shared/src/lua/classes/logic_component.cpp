/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include <sharedutils/datastream.h>

using namespace pragma;

ComponentEventId LogicComponent::EVENT_ON_TICK = INVALID_COMPONENT_ID;
void LogicComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_TICK = registerEvent("ON_TICK", ComponentEventInfo::Type::Explicit); }

LogicComponent::LogicComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}

void LogicComponent::Initialize() { BaseEntityComponent::Initialize(); }
void LogicComponent::OnRemove() { BaseEntityComponent::OnRemove(); }
void LogicComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }
void LogicComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void LogicComponent::OnTick(double dt)
{
	//if(contact != NULL)
	//{
	/*if(contact->actor->isRigidDynamic())
		{
			physx::PxRigidDynamic *rigid = static_cast<physx::PxRigidDynamic*>(contact->actor);
			Vector3 pos = *GetPosition();
			for(unsigned int i=0;i<contact->contactPoints.size();i++)
			{
				Vector3 vDir = pos -(*contact->entity->GetPosition() +contact->contactPoints[i]);
				Vector3::normalize(&vDir);
				physx::PxVec3 dir(vDir.x,vDir.y,vDir.z);
				dir = dir *250.f;
				rigid->addForce(dir);
				rigid->addTorque(dir);
				rigid->setLinearVelocity(dir);
			}
			std::cout<<"CONTACT!"<<std::endl;
		}
		delete contact;
		contact = NULL;*/
	//}

	auto hThis = GetHandle();
	auto &ent = GetEntity();
	CEOnTick evData {dt};
	InvokeEventCallbacks(EVENT_ON_TICK, evData);
}

//////////

CEOnTick::CEOnTick(double dt) : deltaTime(dt) {}
void CEOnTick::PushArguments(lua_State *l) { Lua::PushNumber(l, deltaTime); }
