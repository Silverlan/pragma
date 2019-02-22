#include "stdafx_client.h"

// TODO: Remove this file
#if 0
#include "pragma/c_engine.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/lighting/c_light_directional.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcasc.h"
#include <mathutil/umath.h>
#include "pragma/rendering/scene/scene.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcasc.h"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/entities/components/c_render_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/math/intersection.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

CLightDirectional::CLightDirectional(const Vector3 &pos,const Color &col,const Vector3 &dir,ShadowType shadowType)
	: CLight(LightType::Directional,pos,col,shadowType),CLightRanged(),m_bShadowBufferUpdateScheduled(false)
{
	m_bUseDualTextureSet = false;
	SetDirection(dir);
}

void CLightDirectional::TurnOn()
{
	if(IsTurnedOn())
		return;
	CLight::TurnOn();
	c_game->UpdateEnvironmentLightSource();
}
void CLightDirectional::TurnOff()
{
	if(!IsTurnedOn())
		return;
	CLight::TurnOff();
	c_game->UpdateEnvironmentLightSource();
}

void CLightDirectional::RenderStaticWorldGeometry()
{
	 // prosper TODO
#if 0
	m_cmdShadowBuffers.clear();
	auto *entWorld = c_game->GetWorld();
	if(entWorld == nullptr)
		return;
	auto hShaderShadow = c_engine->GetShader("shadowcsmstatic");
	if(!hShaderShadow.IsValid())
		return;
	auto hMdl = entWorld->GetModel();
	if(hMdl == nullptr)
		return;
	auto &model = *hMdl.get();
	auto *shadow = GetShadowMap();
	if(shadow == nullptr)
		return;
	auto *tex = shadow->GetDepthTexture(); // TODO
	if(tex == nullptr)
		return;
	auto &shaderShadow = static_cast<Shader::ShadowCSMStatic&>(*hShaderShadow.get());
	//auto &context = c_engine->GetRenderContext();

	//auto &renderPass = shadow->GetRenderPass(CLightBase::RenderPass::Dynamic);
	//auto &img = tex->GetImage();

	CLightBase::EntityInfo info(this,entWorld);
	info.meshes.reserve(model.GetSubMeshCount());
	//auto &materials = model.GetMaterials();
	for(auto &group : model.GetMeshGroups())
	{
		for(auto &mesh : group->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				if(CLightBase::ShouldPass(model,*static_cast<CModelSubMesh*>(subMesh.get())) == false)
					continue;
				info.meshes.push_back(static_cast<CModelSubMesh*>(subMesh.get()));
			}
		}
	}
	shaderShadow.Render(*this,m_cmdShadowBuffers,info);
#endif
}
void CLightDirectional::ReloadShadowCommandBuffers()
{
	m_bShadowBufferUpdateScheduled = true;
}
/*const std::vector<Vulkan::SwapCommandBuffer> &CLightDirectional::GetShadowCommandBuffers() const {return m_cmdShadowBuffers;}
const Vulkan::SwapCommandBuffer *CLightDirectional::GetShadowCommandBuffer(uint32_t layer) const
{
	if(layer >= m_cmdShadowBuffers.size())
		return nullptr;
	return &m_cmdShadowBuffers[layer];
}*/ // prosper TODO

void CLightDirectional::ScheduleRender() {CLightRanged::ScheduleRender();}
bool CLightDirectional::ShouldRender() {return CLight::ShouldRender();}
float CLightDirectional::GetDistance() const {return CLightBase::GetDistance();}
void CLightDirectional::SetDistance(float distance) {CLightBase::SetDistance(distance);}
const Vector3 &CLightDirectional::GetPosition() const {return CLightBase::GetPosition();}

void CLightDirectional::InitializeShadowMap()
{
	if(m_shadow != nullptr || m_shadowTypes == ShadowType::None)
		return;
	m_shadow = std::make_unique<ShadowMapCasc>();
	static_cast<ShadowMapCasc&>(*m_shadow).SetFrustumUpdateCallback([this]() {
		//m_bFullUpdateRequired = true;
	});
	CLightBase::InitializeShadowMap(*m_shadow);
}

bool CLightDirectional::ShouldPass(uint32_t layer,const Vector3 &min,const Vector3 &max)
{
	if(m_shadow == nullptr)
		return false;
	auto &shadowMap = static_cast<ShadowMapCasc&>(*m_shadow);
	auto numLayers = m_shadow->GetLayerCount();
	auto &frustum = *shadowMap.GetFrustumSplit(layer);
	return (Intersection::AABBAABB(min,max,frustum.aabb.min +frustum.obbCenter,frustum.aabb.max +frustum.obbCenter) != INTERSECT_OUTSIDE) ? true : false;
}

bool CLightDirectional::ShouldPass(const CBaseEntity &ent,uint32_t &renderFlags)
{
	if(CLight::ShouldPass(ent,renderFlags) == false || m_shadow == nullptr)
		return false;
	auto pTrComponent = ent.GetTransformComponent();
	auto pRenderComponent = ent.GetRenderComponent();
	if(pTrComponent.expired() || pRenderComponent.expired())
		return false;
	auto &pos = pTrComponent->GetPosition();
	Vector3 min;
	Vector3 max;
	pRenderComponent->GetRenderBounds(&min,&max);
	min += pos;
	max += pos;

	auto &shadowMap = static_cast<ShadowMapCasc&>(*m_shadow);
	auto numLayers = m_shadow->GetLayerCount();
	for(auto i=decltype(numLayers){0};i<numLayers;++i)
	{
		auto &frustum = *shadowMap.GetFrustumSplit(i);
		if(Intersection::AABBAABB(min,max,frustum.aabb.min +frustum.obbCenter,frustum.aabb.max +frustum.obbCenter) != INTERSECT_OUTSIDE)
			renderFlags |= 1<<i;
	}
	return (renderFlags != 0) ? true : false;
}
bool CLightDirectional::ShouldPass(const CBaseEntity &ent,const CModelMesh &mesh,uint32_t &renderFlags)
{
	if(m_shadow != nullptr)
	{
		auto pTrComponent = ent.GetTransformComponent();
		if(pTrComponent.expired())
			return false;
		auto &pos = pTrComponent->GetPosition();
		Vector3 min;
		Vector3 max;
		mesh.GetBounds(min,max);
		min += pos;
		max += pos;

		auto &shadowMap = static_cast<ShadowMapCasc&>(*m_shadow);
		auto numLayers = m_shadow->GetLayerCount();
		for(auto i=decltype(numLayers){0};i<numLayers;++i)
		{
			auto &frustum = *shadowMap.GetFrustumSplit(i);
			if(Intersection::AABBAABB(min,max,frustum.aabb.min +frustum.obbCenter,frustum.aabb.max +frustum.obbCenter) != INTERSECT_OUTSIDE)
				renderFlags |= 1<<i;
		}
	}
	return (renderFlags != 0) ? CLight::ShouldPass(ent,mesh,renderFlags) : false;
}

bool CLightDirectional::ShouldUpdateRenderPass(RenderPass) const {return true;} // CSM Update requirements are determined through ShadowMapCasc::ShouldUpdateLayer

void CLightDirectional::Initialize()
{
	CLight::Initialize();
	Vector3 pos = GetPosition();
	auto &dir = GetDirection();
	SetViewMatrix(glm::lookAtRH(pos,pos +dir,uvec::get_perpendicular(dir)));
}

void CLightDirectional::UpdateEntity(CBaseEntity *ent)
{
	//if(ent->IsWorld()) // World geometry is handled separately (See CLightDirectional::ReloadShadowCommandBuffers)
	//	return;
	CLight::UpdateEntity(ent);
}

LightType CLightDirectional::GetType() const {return LightType::Directional;}

void CLightDirectional::SetDirection(const Vector3 &dir)
{
	if(uvec::cmp(dir,GetDirection()) == true)
		return;
	CLight::SetDirection(dir);
	SetViewMatrix(glm::lookAtRH(GetPosition(),GetPosition() +dir,uvec::get_perpendicular(dir)));
	SetStaticResolved(false);
}

void CLightDirectional::UpdateFrustum(uint32_t frustumId)
{
	if(IsTurnedOn() == false)
		return;
	auto *csm = static_cast<ShadowMapCasc*>(m_shadow.get());
	if(csm->IsValid() == false)
		return;
	auto &scene = c_game->GetScene();
	auto &cam = scene->camera;
	csm->UpdateFrustum(frustumId,*cam.get(),GetPosition(),GetViewMatrix(),GetDirection());
}

void CLightDirectional::UpdateFrustum()
{
	if(IsTurnedOn() == false)
		return;
	auto *csm = static_cast<ShadowMapCasc*>(m_shadow.get());
	if(csm->IsValid() == false)
		return;
	auto &scene = c_game->GetScene();
	auto &cam = scene->camera;
	csm->UpdateFrustum(*cam.get(),GetPosition(),GetViewMatrix(),GetDirection());
}

void CLightDirectional::Think()
{
	CLight::Think();
	/*if(!IsTurnedOn())
		return;
	auto &scene = c_game->GetScene();
	auto &cam = scene->camera;
	auto *csm = static_cast<ShadowMapCasc*>(m_shadow.get());
	if(csm->IsValid() == false)
		return;
	csm->UpdateFrustum(*cam.get(),GetPosition(),GetViewMatrix(),GetDirection());
	if(m_bShadowBufferUpdateScheduled == true)
	{
		m_bShadowBufferUpdateScheduled = false;
		RenderStaticWorldGeometry();
	}*/
}
#endif
