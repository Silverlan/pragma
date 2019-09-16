#include "stdafx_client.h"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/rendering/shaders/debug/c_shader_debugdepthbuffer.h"
#include <wgui/types/wirect.h>
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/game/c_game_createguielement.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/rendering/c_cubemapside.h"
#include "pragma/gui/debug/widebugshadowmap.hpp"
#include <pragma/console/sh_cmd.h>
#include <image/prosper_render_target.hpp>

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

static WIHandle hGUIShadowmap;
static int numShadowmapTargets = 0;
static int shadowmapTargetIdx = -1;
static int shadowmapWidth;
static int shadowmapHeight;
static CallbackHandle cbRenderShadowMap;
static CallbackHandle cbReleaseShadowMap;
static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
static void OnRender(EntityHandle hEntity)
{
	/*if(!hEntity.IsValid())
		return;
	CEnvLight *entLight = hEntity.get<CEnvLight>();
	pragma::CLightComponent *light = entLight->GetLight();
	if(light == NULL)
		return;
	ShadowMap *shadow = light->GetShadowMap();
	if(shadow == NULL)
		return;
	static ShaderBase *shader = c_game->GetShader("debugdepthbuffer");
	if(shader == NULL)
		return;
	CLightRanged *lightRanged = dynamic_cast<CLightRanged*>(light);
	int w,h;
	OpenGL::GetViewportSize(&w,&h);
	auto bufFramePrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
	ShaderDebugDepthBuffer *ddb = static_cast<ShaderDebugDepthBuffer*>(shader);
	unsigned int size = cvShadowmapSize->GetInt();
	OpenGL::SetViewPort(0,0,size,size);
	switch(light->GetType())
	{
	case LIGHT_TYPE_POINT:
		{
			(*shadowmapFBOs[0])->Bind();
			ddb->Render(GL_TEXTURE_CUBE_MAP,shadow->GetDepthTexture(),c_game->GetScreenVertexBuffer(),2,CUInt32(lightRanged->GetDistance()),-1,(*shadowmapTextures[0])->GetTextureID());
			break;
		}
	case LIGHT_TYPE_DIRECTIONAL:
		{
			CSM *csm = dynamic_cast<CSM*>(shadow);
			unsigned int numCascades = csm->GetSplitCount();
			unsigned int texture = csm->GetDepthTexture();
			for(unsigned int i=0;i<numCascades;i++)
			{
				(*shadowmapFBOs[i])->Bind();
				ddb->Render(GL_TEXTURE_2D_ARRAY,texture,c_game->GetScreenVertexBuffer(),2,CUInt32(lightRanged->GetDistance()),i);
			}
			break;
		}
	default:
		{
			(*shadowmapFBOs[0])->Bind();
			ddb->Render(GL_TEXTURE_2D,shadow->GetDepthTexture(),c_game->GetScreenVertexBuffer(),2,CUInt32(lightRanged->GetDistance()));
			break;
		}
	}
	OpenGL::SetViewPort(0,0,w,h);
	OpenGL::BindFrameBuffer(bufFramePrev);*/ // Vulkan TODO
}

static void OnGameEnd(EntityHandle hEntity)
{
	if(hGUIShadowmap.IsValid())
		hGUIShadowmap->Remove();
	if(cbRenderShadowMap.IsValid())
		cbRenderShadowMap.Remove();
	if(cbReleaseShadowMap.IsValid())
		cbReleaseShadowMap.Remove();
	numShadowmapTargets = 0;
}

static bool get_shadow_map(NetworkState *nw,std::vector<std::string> &argv,pragma::CLightComponent **light,pragma::CLightComponent::ShadowMapType smType)
{
	if(argv.empty())
		return false;
	auto ents = command::find_named_targets(nw,argv.front());
	if(ents.empty())
		return false;
	auto *ent = static_cast<CBaseEntity*>(ents.front());
	auto *pLightComponent = static_cast<pragma::CLightComponent*>(ent->FindComponent("light").get());
	if(pLightComponent == nullptr)
	{
		Con::cwar<<"Entity '"<<ent->GetClass()<<"'("<<argv.front()<<") is not a light!"<<Con::endl;
		return false;
	}
	*light = pLightComponent;
	if(pLightComponent->GetLight() == nullptr)
	{
		Con::cwar<<"Entity '"<<ent->GetClass()<<"'("<<argv.front()<<") has no light attached!"<<Con::endl;
		return false;
	}
	auto hShadowmap = (*light)->GetShadowMap(smType);
	if(hShadowmap.expired() && (*light)->GetEntity().HasComponent<pragma::CShadowCSMComponent>() == false)
	{
		Con::cwar<<"Invalid shadowmap for this entity!"<<Con::endl;
		return false;
	}
	return true;
}

void CMD_debug_light_shadowmap(NetworkState *nw,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	auto &wgui = WGUI::GetInstance();
	auto *pRoot = wgui.GetBaseElement();
	const std::string name = "debug_shadowmap";
	auto *pEl = pRoot->FindDescendantByName(name);
	if(pEl != nullptr)
		pEl->Remove();

	auto smType = pragma::CLightComponent::ShadowMapType::Static;
	if(argv.size() > 1 && util::to_boolean(argv.at(1)))
		smType = pragma::CLightComponent::ShadowMapType::Dynamic;

	pragma::CLightComponent *light;
	if(get_shadow_map(nw,argv,&light,smType) == false)
		return;
	if(c_game == nullptr || argv.empty() || pRoot == nullptr)
		return;
	auto *pElSm = wgui.Create<WIDebugShadowMap>();
	if(pElSm == nullptr)
		return;
	auto size = 256u;
	pElSm->SetLightSource(*light);
	pElSm->SetShadowMapSize(size,size);
	pElSm->SetShadowMapType(smType);
	pElSm->SetName(name);
	pElSm->Update();
}

static void CVAR_CALLBACK_cl_render_shadow_pssm_split_count(NetworkState *state,ConVar*,int,int)
{
	if(shadowmapTargetIdx == -1)
		return;
	std::vector<std::string> argv = {
		std::to_string(shadowmapTargetIdx),
		std::to_string(shadowmapWidth),
		std::to_string(shadowmapHeight)
	};
	CMD_debug_light_shadowmap(state,nullptr,argv);
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_pssm_split_count,CVAR_CALLBACK_cl_render_shadow_pssm_split_count);
