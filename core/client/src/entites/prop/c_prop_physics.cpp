#include "stdafx_client.h"
#include "pragma/entities/prop/c_prop_physics.h"
#include "pragma/entities/c_entityfactories.h"
#include <pragma/physics/physobj.h>
#include "pragma/physics/movetypes.h"
#include "pragma/entities/components/c_physics_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(prop_physics,CPropPhysics);

void CPropPhysicsComponent::Initialize()
{
	BasePropPhysicsComponent::Initialize();
}

void CPropPhysicsComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BasePropPhysicsComponent::OnEntityComponentAdded(component);
	/*if(typeid(component) == typeid(pragma::CPhysicsComponent))
	{
		auto *pPhysComponent = static_cast<pragma::CPhysicsComponent*>(&component);
		pPhysComponent->SetMoveType(MOVETYPE::PHYSICS);
		// m_propPhysType = PHYSICSTYPE::DYNAMIC;
	}
	else if(typeid(component) == typeid(pragma::CRenderComponent))
	{
		auto *pRenderComponent = static_cast<pragma::CRenderComponent*>(&component);
		pRenderComponent->SetCastShadows(true);
	}*/
}

luabind::object CPropPhysicsComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CPropPhysicsComponentHandleWrapper>(l);}

//////////////////

void CPropPhysics::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CPropPhysicsComponent>();
}

