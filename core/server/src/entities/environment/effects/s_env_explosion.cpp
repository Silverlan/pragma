#include "stdafx_server.h"
#include "pragma/entities/environment/effects/s_env_explosion.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/game/damageinfo.h>
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_explosion,EnvExplosion);

extern ServerState *server;

void SExplosionComponent::Explode()
{
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		server->BroadcastTCP("envexplosion_explode",p);
	}
	BaseEnvExplosionComponent::Explode();
}

luabind::object SExplosionComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SExplosionComponentHandleWrapper>(l);}

void EnvExplosion::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SExplosionComponent>();
}
