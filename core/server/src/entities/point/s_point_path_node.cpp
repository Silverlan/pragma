#include "stdafx_server.h"
#include "pragma/entities/point/s_point_path_node.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_path_node,PointPathNode);

luabind::object SPathNodeComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SPathNodeComponentHandleWrapper>(l);}

void PointPathNode::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPathNodeComponent>();
}
