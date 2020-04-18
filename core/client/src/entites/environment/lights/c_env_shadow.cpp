#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/console/c_cvar.h>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <image/prosper_render_target.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

CShadowComponent::CShadowComponent(BaseEntity &ent)
	: BaseEntityComponent{ent}
{}

void CShadowComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	ReloadDepthTextures();
}

static void reload_all_shadow_maps()
{
	if(c_game == nullptr)
		return;
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CShadowComponent>>();
	for(auto *ent : entIt)
	{
		auto shadowC = ent->GetComponent<pragma::CShadowComponent>();
		shadowC->ReloadDepthTextures();
	}
}

REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_quality,[](NetworkState*,ConVar*,int,int) {
	reload_all_shadow_maps();
});

REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_dynamic,[](NetworkState*,ConVar*,bool,bool) {
	reload_all_shadow_maps();
});

Anvil::DescriptorSet *CShadowComponent::GetDescriptorSet()
{
	auto *shadowManager = CShadowManagerComponent::GetShadowManager();
	return shadowManager ? shadowManager->GetDescriptorSet() : nullptr;
}

void CShadowComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	DestroyTextures();
}

void CShadowComponent::SetTextureReloadCallback(const std::function<void(void)> &f) {m_onTexturesReloaded = f;}

void CShadowComponent::DestroyTextures()
{
	if(m_hRt.expired())
		return;
	auto *shadowManager = CShadowManagerComponent::GetShadowManager();
	if(shadowManager)
		shadowManager->FreeRenderTarget(*m_hRt->lock());
}

CShadowComponent::Type CShadowComponent::GetType() const {return m_type;}
void CShadowComponent::SetType(Type type) {m_type = type;}
uint32_t CShadowComponent::GetLayerCount() const
{
	switch(m_type)
	{
	case Type::Generic:
		return 1;
	case Type::Cube:
		return 6;
	}
	return 0;
}

luabind::object CShadowComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CShadowComponentHandleWrapper>(l);}

static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
static CVar cvShadowQuality = GetClientConVar("cl_render_shadow_quality");
void CShadowComponent::ReloadDepthTextures()
{
	//Scene::ClearLightCache();
	volatile ScopeGuard sg {[this]() {
		if(m_onTexturesReloaded == nullptr)
			return;
		m_onTexturesReloaded();
	}};
	if(cvShadowQuality->GetInt() <= 0)
		return;
	auto size = cvShadowmapSize->GetInt();
	if(size <= 0)
		return;
	InitializeDepthTextures(size);
}

void CShadowComponent::InitializeDepthTextures(uint32_t size) {}

std::weak_ptr<CShadowManagerComponent::RenderTarget> CShadowComponent::RequestRenderTarget()
{
	// Note: Render target are assigned by priority, which is simply the frame id it was last used.
	// That means the render targets for shadow maps that haven't been used in a while have a lower priority
	// and are more likely to be re-assigned if a new one is requested.

	auto *shadowManager = CShadowManagerComponent::GetShadowManager();
	if(shadowManager == nullptr)
		return std::weak_ptr<CShadowManagerComponent::RenderTarget>{};
	auto priority = c_engine->GetLastFrameId();
	if(m_hRt.valid())
	{
		// We'll keep our current render target; Just update the priority
		shadowManager->UpdatePriority(*m_hRt->lock(),priority);
		return *m_hRt;
	}
	m_lastFrameRendered = -1; // Shadows will have to be re-rendered
	SetDirty(true);
	m_hRt = shadowManager->RequestRenderTarget(
		(GetType() == CShadowComponent::Type::Cube) ? CShadowManagerComponent::Type::Cube : CShadowManagerComponent::Type::Generic,
		cvShadowmapSize->GetInt(),
		priority
	);
	return m_hRt.valid() ? *m_hRt : std::weak_ptr<CShadowManagerComponent::RenderTarget>{};
}
void CShadowComponent::FreeRenderTarget() {DestroyTextures();}

int64_t CShadowComponent::GetLastFrameRendered() const {return m_lastFrameRendered;}
void CShadowComponent::SetLastFrameRendered(int64_t frameId) {m_lastFrameRendered = frameId;}

bool CShadowComponent::IsDirty() const {return m_bDirty;}
void CShadowComponent::SetDirty(bool dirty) {m_bDirty = dirty;}

const std::shared_ptr<prosper::RenderTarget> &CShadowComponent::GetDepthRenderTarget() const
{
	static std::shared_ptr<prosper::RenderTarget> nptr = nullptr;
	if(m_hRt.expired())
		return nptr;
	return m_hRt->lock()->renderTarget;
}
const std::shared_ptr<prosper::Texture> &CShadowComponent::GetDepthTexture() const
{
	static std::shared_ptr<prosper::Texture> nptr = nullptr;
	auto rt = GetDepthRenderTarget();
	return rt ? rt->GetTexture() : nptr;
}
const std::shared_ptr<prosper::RenderPass> &CShadowComponent::GetRenderPass() const
{
	static std::shared_ptr<prosper::RenderPass> nptr = nullptr;
	auto rt = GetDepthRenderTarget();
	return rt ? rt->GetRenderPass() : nptr;
}
const std::shared_ptr<prosper::Framebuffer> &CShadowComponent::GetFramebuffer(uint32_t layerId)
{
	static std::shared_ptr<prosper::Framebuffer> nptr = nullptr;
	auto rt = GetDepthRenderTarget();
	return rt ? rt->GetFramebuffer(layerId) : nptr;
}
bool CShadowComponent::HasRenderTarget() const {return m_hRt.valid();}

bool CShadowComponent::ShouldUpdateLayer(uint32_t) const {return true;}
