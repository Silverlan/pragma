/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include <sharedutils/datastream.h>

using namespace pragma;

ComponentEventId LogicComponent::EVENT_ON_TICK = INVALID_COMPONENT_ID;
void LogicComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_TICK = componentManager.RegisterEvent("ON_TICK",std::type_index(typeid(LogicComponent)));
}
LogicComponent::LogicComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_lastThink(0)
{}

void LogicComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}

void LogicComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	m_lastThink = GetEntity().GetNetworkState()->GetGameState()->CurTime();
}

luabind::object LogicComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<LogicComponentHandleWrapper>(l);}

double LogicComponent::GetNextThink() const {return m_tNextThink;}
void LogicComponent::SetNextThink(double t) {m_tNextThink = t;}

double LogicComponent::LastThink() const {return m_lastThink;}

double LogicComponent::DeltaTime() const
{
	Game *game = GetEntity().GetNetworkState()->GetGameState();
	//auto r = game->CurTime() -m_lastThink; // This would be more accurate, but can be 0 if the engine had to catch up on the tick rate
	auto r = game->DeltaTickTime();
	//assert(r != 0.0); // Delta time mustn't ever be 0, otherwise there can be problems with animation events repeating (among other things)
	return r;
}

void LogicComponent::PostThink() {}

void LogicComponent::Think(double dt)
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
	InvokeEventCallbacks(EVENT_ON_TICK,CEOnTick{dt});
	if(hThis.expired())
		return; // This component isn't valid anymore; Return immediately
	Game *game = ent.GetNetworkState()->GetGameState();
	m_lastThink = game->CurTime();
}

void LogicComponent::Save(DataStream &ds)
{
	BaseEntityComponent::Save(ds);
	auto tCur = GetEntity().GetNetworkState()->GetGameState()->CurTime();
	ds->Write<float>(m_lastThink -tCur);
	ds->Write<float>(m_tNextThink -tCur);
}
void LogicComponent::Load(DataStream &ds,uint32_t version)
{
	BaseEntityComponent::Load(ds,version);
	auto tCur = GetEntity().GetNetworkState()->GetGameState()->CurTime();
	auto lastThink = ds->Read<float>();
	auto nextThink = ds->Read<float>();
	m_lastThink = tCur +lastThink;
	m_tNextThink = tCur +nextThink;
}

//////////

CEOnTick::CEOnTick(double dt)
	: deltaTime(dt)
{}
void CEOnTick::PushArguments(lua_State *l)
{
	Lua::PushNumber(l,deltaTime);
}
