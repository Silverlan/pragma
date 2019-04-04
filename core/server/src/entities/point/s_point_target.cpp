#include "stdafx_server.h"
#include "pragma/entities/point/s_point_target.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_target,PointTarget);

luabind::object SPointTargetComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SPointTargetComponentHandleWrapper>(l);}

void PointTarget::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPointTargetComponent>();
}
