#include "stdafx_client.h"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/rendering/lighting/c_light_ranged.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcasc.h"
#include "pragma/rendering/shaders/debug/c_shader_debugdepthbuffer.h"
#include <wgui/types/wirect.h>
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/game/c_game_createguielement.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/rendering/c_cubemapside.h"
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

static bool GetShadowMap(std::vector<std::string> &argv,pragma::CLightComponent **light,ShadowMap **shadow,CBaseEntity **entity=nullptr,int *index=nullptr)
{
	if(argv.empty())
		return false;
	auto idx = atoi(argv[0].c_str());
	if(index != nullptr)
		*index = idx;
	auto *ent = c_game->GetEntity(idx);
	if(entity != nullptr)
		*entity = ent;
	if(ent == nullptr)
	{
		Con::cwar<<"No entity with index "<<idx<<" found!"<<Con::endl;
		return false;
	}
	auto *pLightComponent = static_cast<pragma::CLightComponent*>(ent->FindComponent("light").get());
	if(pLightComponent == nullptr)
	{
		Con::cwar<<"Entity '"<<ent->GetClass()<<"'("<<idx<<") is not a light!"<<Con::endl;
		return false;
	}
	*light = pLightComponent;
	if(pLightComponent->GetLight() == nullptr)
	{
		Con::cwar<<"Entity '"<<ent->GetClass()<<"'("<<idx<<") has no light attached!"<<Con::endl;
		return false;
	}
	*shadow = (*light)->GetShadowMap();
	if(*shadow == nullptr)
	{
		Con::cwar<<"Invalid shadowmap for this entity!"<<Con::endl;
		return false;
	}
	return true;
}

static void initialize_debug_shadow_gui_elements(WIBase *base,pragma::CLightComponent *lightBase,ShadowMap *shadow,pragma::CLightComponent::RenderPass rp,uint32_t size)
{
	auto type = LightType::Invalid;
	auto *pLight = lightBase->GetLight(type);
	if(pLight == nullptr)
		return;
	auto &depthTexture = (type != LightType::Directional) ? shadow->GetDepthTexture() : static_cast<ShadowMapCasc*>(shadow)->GetDepthTexture(rp);
	if(depthTexture == nullptr)
		return;
	auto &depthImage = depthTexture->GetImage();
	auto numLayers = depthImage->GetLayerCount();
	auto wLayer = size;
	auto hLayer = size;
	prosper::util::BarrierImageLayout barrierImageLayout {
		Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,Anvil::AccessFlagBits::SHADER_READ_BIT
	};
	auto pRadiusComponent = lightBase->GetEntity().GetComponent<pragma::CRadiusComponent>();
	auto &wgui = WGUI::GetInstance();
	switch(type)
	{
		case LightType::Point:
		{
			for(auto i=decltype(numLayers){0};i<numLayers;++i)
			{
				auto *dt = wgui.Create<WIDebugDepthTexture>(base);
				dt->SetTexture(*depthTexture,barrierImageLayout,barrierImageLayout,i);
				dt->SetSize(wLayer,hLayer);
				if(i == static_cast<uint32_t>(CubeMapSide::Left))
					dt->SetPos(0,hLayer); // Left
				else if(i == static_cast<uint32_t>(CubeMapSide::Right))
					dt->SetPos(wLayer *2,hLayer); // Right
				else if(i == static_cast<uint32_t>(CubeMapSide::Top))
					dt->SetPos(wLayer,0); // Up
				else if(i == static_cast<uint32_t>(CubeMapSide::Bottom))
					dt->SetPos(wLayer,hLayer *2); // Down
				else if(i == static_cast<uint32_t>(CubeMapSide::Front))
					dt->SetPos(wLayer,hLayer); // Front
				else
					dt->SetPos(wLayer *3,hLayer); // Back
				dt->Update(1.f,pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f);
			}
			break;
		}
		case LightType::Spot:
		{
			auto *dt = wgui.Create<WIDebugDepthTexture>(base);
			dt->SetTexture(*depthTexture,barrierImageLayout,barrierImageLayout);
			dt->SetSize(wLayer,hLayer);
			dt->Update(1.f,pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f);
			break;
		}
		case LightType::Directional:
		{
			wLayer *= 0.5f;
			hLayer *= 0.5f;
			for(auto i=decltype(numLayers){0};i<numLayers;++i)
			{
				auto *dt = wgui.Create<WIDebugDepthTexture>(base);
				dt->SetTexture(*depthTexture,barrierImageLayout,barrierImageLayout,i);
				dt->SetSize(wLayer,hLayer);
				dt->SetPos(i *wLayer,0);
				dt->Update(1.f,pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f);
			}
			auto csmMap = static_cast<ShadowMapCasc*>(shadow);
			auto &staticDepthTex = csmMap->GetStaticPendingRenderTarget()->GetTexture();
			if(staticDepthTex != nullptr)
			{
				auto &staticDepthImg = staticDepthTex->GetImage();
				for(auto i=decltype(numLayers){0};i<numLayers;++i)
				{
					auto *dt = wgui.Create<WIDebugDepthTexture>(base);
					dt->SetTexture(*staticDepthTex,barrierImageLayout,barrierImageLayout,i);
					dt->SetSize(wLayer,hLayer);
					dt->SetPos(i *wLayer,hLayer);
					dt->Update(1.f,pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f);
				}
			}
			hLayer *= 2.0;
			break;
		}
		default:
			break;
	}
	auto width = ((type == LightType::Point) ? 4 : numLayers) *wLayer;
	auto height = ((type == LightType::Point) ? 3 : 1) * hLayer;
	base->SetSize(width,height);
}

void CMD_debug_light_shadowmap(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string> &argv)
{
	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	dbg = nullptr;
	pragma::CLightComponent *light;
	ShadowMap *shadow;
	CBaseEntity *ent;
	int idx;
	if(GetShadowMap(argv,&light,&shadow,&ent,&idx) == false)
		return;
	auto size = 256u;
	if(argv.size() > 1)
		size = util::to_int(argv.at(1));
	//if(shadow->GetDepthTexture(rp) == nullptr || (argv.size() > 1 && util::to_int(argv[1]) != 0))
	//	rp = pragma::CLightComponent::RenderPass::Static;
	dbg = std::make_unique<DebugGameGUI>([shadow,light,size]() {
		auto &wgui = WGUI::GetInstance();
		auto *r = wgui.Create<WIBase>();
		initialize_debug_shadow_gui_elements(r,light,shadow,pragma::CLightComponent::RenderPass::Dynamic,size);
		return r->GetHandle();
	});
	auto *d = dbg.get();
	dbg->AddCallback("PostRenderScene",FunctionCallback<>::Create([d,light]() {
		//auto &scene = c_game->GetScene();
		//auto &depthTex = scene->GetDepthTexture();
		auto *el = d->GetGUIElement();
		if(el == nullptr)
			return;
		/*auto *ranged = dynamic_cast<CLightRanged*>(light); // prosper TODO
		auto &children = *el->GetChildren();
		for(auto &hChild : children)
		{
			//auto *ddt = dynamic_cast<WIDebugDepthTexture*>(hChild.get()); // prosper TODO
			//if(ddt != nullptr) // prosper TODO
			//	ddt->Update(1.f,ranged->GetDistance()); // prosper TODO
		}*/
	}));
	////if(c_game == NULL)
	////	return;
	////if(hGUIShadowmap.IsValid())
	////	hGUIShadowmap->Remove();
	////if(cbRenderShadowMap.IsValid())
	////	cbRenderShadowMap.Remove();
	////if(cbReleaseShadowMap.IsValid())
	////	cbReleaseShadowMap.Remove();
	////if(shadowmapFBOs != NULL)
	////{
	////	for(int i=0;i<numShadowmapTargets;i++)
	////		delete shadowmapFBOs[i];
	////	delete[] shadowmapFBOs;
	////	shadowmapFBOs = NULL;
	////}
	////if(shadowmapTextures != NULL)
	////{
	////	for(int i=0;i<numShadowmapTargets;i++) delete shadowmapTextures[i];
	////	delete[] shadowmapTextures;
	////	shadowmapTextures = NULL;
	////}
	////numShadowmapTargets = 0;
	////if(argv.empty())
	////{
	////	shadowmapTargetIdx = -1;
	////	return;
	////}
	////pragma::CLightComponent *light;
	////ShadowMap *shadow;
	////CBaseEntity *ent;
	////int idx;
	////if(GetShadowMap(argv,&light,&shadow,&ent,&idx) == false)
	////	return;
	////shadowmapTargetIdx = idx;
	////int vpWidth,vpHeight;
	////OpenGL::GetViewportSize(&vpWidth,&vpHeight);
	////int w = 256;
	////int h = w;
	////auto argc = argv.size();
	////if(argc > 1)
	////{
	////	w = atoi(argv[1].c_str());
	////	if(argc > 2)
	////		h = atoi(argv[2].c_str());
	////	else
	////		h = w;
	////}
	////shadowmapWidth = w;
	////shadowmapHeight = h;
	////unsigned int size = c_game->GetConVarInt("cl_render_shadow_resolution");
	////switch(light->GetType())
	////{
	////case LightType::Directional:
	////	{
	////		ShadowMapCasc *csm = static_cast<ShadowMapCasc*>(shadow);
	////		numShadowmapTargets = csm->GetSplitCount();
	////		break;
	////	}
	////default:
	////	{
	////		numShadowmapTargets = 1;
	////		break;
	////	}
	////}
	////shadowmapFBOs = new GLFrameBufferPtr*[numShadowmapTargets];
	////for(int i=0;i<numShadowmapTargets;i++)
	////	shadowmapFBOs[i] = new GLFrameBufferPtr(GLFrameBuffer::Create());
	////shadowmapTextures = new GLTexturePtr*[numShadowmapTargets];
	////auto bufFramePrev = OpenGL::GetInt(GL_FRAMEBUFFER_BINDING);
	////auto type = light->GetType();
	////if(type != LightType::Point)
	////{
	////	for(int i=0;i<numShadowmapTargets;i++)
	////	{
	////		(*shadowmapFBOs[i])->Bind();
	////		shadowmapTextures[i] = new GLTexturePtr(GLTexture::Create(size,size));
	////		(*shadowmapTextures[i])->Bind();
	////		(*shadowmapTextures[i])->SetParameter(GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	////		(*shadowmapTextures[i])->SetParameter(GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	////		(*shadowmapTextures[i])->SetParameter(GL_TEXTURE_MAX_LEVEL,0);
	////		(*shadowmapFBOs[i])->AttachTexture((*shadowmapTextures[i]).get());
	////	}
	////	OpenGL::BindTexture(0,GL_TEXTURE_2D);
	////}
	////else
	////{
	////	(*shadowmapFBOs[0])->Bind();
	////	shadowmapTextures[0] = new GLTexturePtr(GLTexture::CreateCubemap(size,size));
	////	(*shadowmapTextures[0])->Bind();
	////	(*shadowmapFBOs[0])->AttachTexture((*shadowmapTextures[0]).get());
	////	OpenGL::BindTexture(0,GL_TEXTURE_CUBE_MAP);
	////}
	////OpenGL::BindFrameBuffer(bufFramePrev);
	////switch(type)
	////{
	////case LightType::Point:
	////	{
	////		WICubemap *rect = c_game->CreateGUIElement<WICubemap>();
	////		rect->SetSize(Vector2i(w *4,h *3));
	////		rect->SetTexture((*shadowmapTextures[0])->GetTextureID());
	////		hGUIShadowmap = rect->GetHandle();
	////		break;
	////	}
	////case LightType::Directional:
	////	{
	////	/*	ShadowMapCasc *csm = static_cast<ShadowMapCasc*>(shadow);
	////		unsigned int numCascades = csm->GetSplitCount();
	////		WIRect *rect = c_game->CreateGUIElement<WIRect>();
	////		rect->SetSize(Vector2i(w *numCascades,h));
	////		int x = 0;
	////		for(unsigned int i=0;i<numCascades;i++)
	////		{
	////			WITexturedRect *map = c_game->CreateGUIElement<WITexturedRect>(rect);
	////			map->SetTexture(shadowmapTextures[i]->get());
	////			map->SetSize(Vector2i(w,h));
	////			map->SetPos(Vector2i(x,0));
	////			x += w;
	////		}
	////		hGUIShadowmap = rect->GetHandle();*/ // Vulkan TODO
	////		break;
	////	}
	////default:
	////	{
	////		/*WITexturedRect *rect = c_game->CreateGUIElement<WITexturedRect>();
	////		rect->SetSize(Vector2i(w,h));
	////		rect->SetPos(Vector2i(0,0));
	////		rect->SetTexture(shadowmapTextures[0]->get());
	////		hGUIShadowmap = rect->GetHandle();*/ // Vulkan TODO
	////		break;
	////	}
	////}
	////cbRenderShadowMap = c_game->AddCallback("RenderScreen",FunctionCallback<>::Create(
	////	std::bind(&OnRender,ent->GetHandle())
	////));
	////cbReleaseShadowMap = c_game->AddCallback("EndGame",FunctionCallback<>::Create(
	////	std::bind(&OnGameEnd,ent->GetHandle())
	////));
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
