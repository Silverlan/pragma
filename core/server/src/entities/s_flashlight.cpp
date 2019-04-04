#include "stdafx_server.h"
#include "pragma/entities/s_flashlight.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(flashlight,Flashlight);

luabind::object SFlashlightComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SFlashlightComponentHandleWrapper>(l);}

void Flashlight::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFlashlightComponent>();
}
