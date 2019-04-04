#include "stdafx_client.h"
#include "pragma/entities/c_skybox.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(skybox,CSkybox);

void CSkyboxComponent::Initialize()
{
	BaseSkyboxComponent::Initialize();
	auto &ent = static_cast<CBaseEntity&>(GetEntity());
	auto pRenderComponent = ent.GetComponent<pragma::CRenderComponent>();
	if(pRenderComponent.valid())
	{
		pRenderComponent->SetRenderMode(RenderMode::Skybox);
		pRenderComponent->SetCastShadows(false);
	}
}
luabind::object CSkyboxComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CSkyboxComponentHandleWrapper>(l);}

void CSkybox::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSkyboxComponent>();
}
