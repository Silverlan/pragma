#include "stdafx_server.h"
#include "pragma/entities/environment/lights/s_env_light_point.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_light_point,EnvLightPoint);

luabind::object SLightPointComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SLightPointComponentHandleWrapper>(l);}

void SLightPointComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(SLightComponent))
		static_cast<SLightComponent&>(component).SetLight(*this);
}

void EnvLightPoint::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SLightComponent>();
	AddComponent<SLightPointComponent>();
}
