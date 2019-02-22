#include "stdafx_client.h"
#include "pragma/entities/c_viewbody.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/parentinfo.h"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(viewbody,CViewBody);

extern ClientState *client;
extern CGame *c_game;

void CViewBodyComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(CRenderComponent::EVENT_SHOULD_DRAW,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &shouldDrawData = static_cast<CEShouldDraw&>(evData.get());
		auto *pl = c_game->GetLocalPlayer();
		if(pl == nullptr || pl->GetObserverMode() != OBSERVERMODE::FIRSTPERSON)
		{
			shouldDrawData.shouldDraw = CEShouldDraw::ShouldDraw::No;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});

	auto &ent = GetEntity();
	ent.AddComponent<CTransformComponent>();
	ent.AddComponent<CModelComponent>();
	ent.AddComponent<LogicComponent>(); // Logic component is needed for animations
	auto pRenderComponent = ent.AddComponent<CRenderComponent>();
	if(pRenderComponent.valid())
	{
		pRenderComponent->SetRenderMode(RenderMode::View);
		pRenderComponent->SetCastShadows(false);
	}
	ent.AddComponent<CAnimatedComponent>();
}
luabind::object CViewBodyComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CViewBodyComponentHandleWrapper>(l);}

//////////////

void CViewBody::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CViewBodyComponent>();
}
