// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;


#include "pragma/lua/core.hpp"


module pragma.shared;

import :entities.components.logic_component;

using namespace pragma;

ComponentEventId LogicComponent::EVENT_ON_TICK = INVALID_COMPONENT_ID;
void LogicComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_TICK = registerEvent("ON_TICK", ComponentEventInfo::Type::Explicit); }

LogicComponent::LogicComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

void LogicComponent::Initialize() { BaseEntityComponent::Initialize(); }
void LogicComponent::OnRemove() { BaseEntityComponent::OnRemove(); }
void LogicComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }
void LogicComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void LogicComponent::OnTick(double dt)
{
	//if(contact != nullptr)
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
		contact = nullptr;*/
	//}

	auto hThis = GetHandle();
	auto &ent = GetEntity();
	CEOnTick evData {dt};
	InvokeEventCallbacks(EVENT_ON_TICK, evData);
}

//////////

CEOnTick::CEOnTick(double dt) : deltaTime(dt) {}
void CEOnTick::PushArguments(lua_State *l) { Lua::PushNumber(l, deltaTime); }
