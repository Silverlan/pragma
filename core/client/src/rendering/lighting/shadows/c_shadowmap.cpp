#include "stdafx_client.h"
#include <pragma/engine.h>
#include "pragma/clientstate/clientstate.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/console/convars.h>
#include "pragma/console/c_cvar.h"
#include "pragma/rendering/shaders/c_shader_depth_combine.h"
#include <sharedutils/scope_guard.h>
#include <image/prosper_render_target.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

IndexMap<ShadowMap> ShadowMap::m_shadowMaps;

decltype(ShadowMap::s_shadowBufferManager) ShadowMap::s_shadowBufferManager = {};

#pragma optimize("",off)
static void reload_all_shadow_maps()
{
	auto &shadowMaps = ShadowMap::GetAll();
	for(auto i=decltype(shadowMaps.Size()){0};i<shadowMaps.Size();++i)
	{
		auto *sm = shadowMaps[i];
		if(sm != nullptr)
			sm->ReloadDepthTextures();
	}
}

REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_quality,[](NetworkState*,ConVar*,int,int) {
	reload_all_shadow_maps();
});

/*REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_resolution,[](NetworkState*,ConVar*,int,int val) {
	static auto bSkipChecks = false;
	auto &context = c_engine->GetRenderContext();
	auto &props = context.GetPhysicalDeviceProperties();
	auto &limits = props.limits;
	auto &maxDim = limits.maxImageDimension2D;
	const uint32_t maxRes = 2048;
	if(val <= 0.f)
		Con::cwar<<"WARNING: Negative or zero value used for shadow mapping size. No shadows will be rendered."<<Con::endl;
	else
	{
		if(static_cast<uint32_t>(val) > maxDim)
		{
			if(bSkipChecks == false)
			{
				Con::cwar<<"WARNING: Your hardware doesn't support "<<val<<"x"<<val<<" for shadow mapping. The max supported size is "<<maxDim<<"x"<<maxDim<<". Clamping..."<<Con::endl;
				bSkipChecks = true;
				client->SetConVar("cl_render_shadow_resolution",std::to_string(maxDim));
				bSkipChecks = false;
			}
			return;
		}
		if(static_cast<uint32_t>(val) > maxRes)
		{
			if(bSkipChecks == false)
			{
				Con::cwar<<"WARNING: Shadow resolutions higher than "<<maxRes<<"x"<<maxRes<<" can be unstable and result in system crashes! Clamping to "<<maxRes<<"x"<<maxRes<<"..."<<Con::endl;
				bSkipChecks = true;
				client->SetConVar("cl_render_shadow_resolution",std::to_string(maxRes));
				bSkipChecks = false;
			}
			return;
		}
	}
	reload_all_shadow_maps();
});*/ // prosper TODO

REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_dynamic,[](NetworkState*,ConVar*,bool,bool) {
	reload_all_shadow_maps();
});

IndexMap<ShadowMap> &ShadowMap::GetAll() {return m_shadowMaps;}
void ShadowMap::ClearShadowMapDepthBuffers()
{
	s_shadowBufferManager.Clear();
}

void ShadowMap::InitializeDescriptorSet() {s_shadowBufferManager.Initialize();}
Anvil::DescriptorSet *ShadowMap::GetDescriptorSet() {return s_shadowBufferManager.GetDescriptorSet();}

ShadowMap::ShadowMap(uint32_t layerCount,bool bUseDualTextureSet)
	: m_bValid(true),m_layerCount(layerCount),
	m_onTexturesReloaded(nullptr),m_bUseDualTextureSet(bUseDualTextureSet)
{
	m_index = m_shadowMaps.Insert(this);
}

ShadowMap::~ShadowMap()
{
	m_shadowMaps.Remove(m_index);
	DestroyTextures();
}

void ShadowMap::SetTextureReloadCallback(const std::function<void(void)> &f) {m_onTexturesReloaded = f;}

uint32_t ShadowMap::GetLayerCount() const {return m_layerCount;}

void ShadowMap::Initialize()
{
	ReloadDepthTextures();
}

void ShadowMap::DestroyTextures()
{
	if(m_shadowRt.expired() == true)
		return;
	s_shadowBufferManager.FreeRenderTarget(*m_shadowRt.lock());
	m_shadowRt = {};
}

ShadowMap::Type ShadowMap::GetType() {return ShadowMap::Type::Generic;}

void ShadowMap::GenerateTextures() // Obsolete?
{
	//if(m_depthTexture != 0)
	//	OpenGL::DeleteTexture(m_depthTexture);
	//OpenGL::GenerateTextures(1,&m_depthTexture); // Vulkan TODO
}

void ShadowMap::ApplyTextureParameters()
{
	/*OpenGL::SetTextureParameter(GL_TEXTURE_MAG_FILTER,GL_NEAREST,GL_TEXTURE_2D);
	OpenGL::SetTextureParameter(GL_TEXTURE_MIN_FILTER,GL_NEAREST,GL_TEXTURE_2D);
	OpenGL::SetTextureParameter(GL_TEXTURE_WRAP_R,GL_CLAMP_TO_BORDER,GL_TEXTURE_2D);
	OpenGL::SetTextureParameter(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER,GL_TEXTURE_2D);
	OpenGL::SetTextureParameter(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER,GL_TEXTURE_2D);
	OpenGL::SetTextureParameter(GL_TEXTURE_COMPARE_FUNC,GL_LEQUAL,GL_TEXTURE_2D);

	// Needed when shadow2DProj is used
	OpenGL::SetTextureParameter(GL_TEXTURE_COMPARE_MODE,GL_COMPARE_R_TO_TEXTURE,GL_TEXTURE_2D);
	OpenGL::SetTextureParameter(GL_TEXTURE_COMPARE_FUNC,GL_LEQUAL,GL_TEXTURE_2D);
	OpenGL::SetTextureParameter(GL_DEPTH_TEXTURE_MODE,GL_INTENSITY,GL_TEXTURE_2D);
	//*/ // Vulkan TODO
}

static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
static CVar cvShadowQuality = GetClientConVar("cl_render_shadow_quality");
void ShadowMap::ReloadDepthTextures()
{
	//Scene::ClearLightCache();
	volatile ScopeGuard sg {[this]() {
		if(m_onTexturesReloaded == nullptr)
			return;
		m_onTexturesReloaded();
	}};
	m_bValid = false;
	if(cvShadowQuality->GetInt() <= 0)
		return;
	auto size = cvShadowmapSize->GetInt();
	if(size <= 0)
		return;
	m_bValid = true;
	InitializeDepthTextures(size);
}

void ShadowMap::InitializeDepthTextures(uint32_t size) {}

std::weak_ptr<ShadowMapDepthBufferManager::RenderTarget> ShadowMap::RequestRenderTarget()
{
	FreeRenderTarget();
	m_shadowRt = s_shadowBufferManager.RequestRenderTarget((GetType() == ShadowMap::Type::Cube) ? ShadowMapDepthBufferManager::Type::Cube : ShadowMapDepthBufferManager::Type::Generic,cvShadowmapSize->GetInt());
	return m_shadowRt;
}
void ShadowMap::FreeRenderTarget() {DestroyTextures();}

const std::shared_ptr<prosper::RenderTarget> &ShadowMap::GetDepthRenderTarget() const
{
	static std::shared_ptr<prosper::RenderTarget> nptr = nullptr;
	if(m_shadowRt.expired())
		return nptr;
	return m_shadowRt.lock()->renderTarget;
}
const std::shared_ptr<prosper::Texture> &ShadowMap::GetDepthTexture() const
{
	static std::shared_ptr<prosper::Texture> nptr = nullptr;
	if(m_shadowRt.expired())
		return nptr;
	return m_shadowRt.lock()->renderTarget->GetTexture();
}
const std::shared_ptr<prosper::RenderPass> &ShadowMap::GetRenderPass() const
{
	static std::shared_ptr<prosper::RenderPass> nptr = nullptr;
	if(m_shadowRt.expired())
		return nptr;
	return m_shadowRt.lock()->renderTarget->GetRenderPass();
}
const std::shared_ptr<prosper::Framebuffer> &ShadowMap::GetFramebuffer(uint32_t layerId)
{
	static std::shared_ptr<prosper::Framebuffer> nptr = nullptr;
	if(m_shadowRt.expired())
		return nptr;
	return m_shadowRt.lock()->renderTarget->GetFramebuffer(layerId);
}
bool ShadowMap::HasRenderTarget() const {return !m_shadowRt.expired();}

bool ShadowMap::IsValid() const {return m_bValid;}

bool ShadowMap::ShouldUpdateLayer(uint32_t) const {return true;}
#pragma optimize("",on)
