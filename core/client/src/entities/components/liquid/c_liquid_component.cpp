/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/liquid/c_liquid_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include <pragma/physics/physobj.h>
#include "pragma/model/c_modelmesh.h"
#include "pragma/physics/c_phys_water_surface_simulator.hpp"
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/c_water_surface.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_surface_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_surface_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/model/model.h>
#include <pragma/physics/raytraces.h>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_water, CFuncWater);

extern DLLCLIENT CGame *c_game;

CLiquidComponent::CLiquidComponent(BaseEntity &ent) : BaseFuncLiquidComponent(ent) { pragma::CParticleSystemComponent::Precache("water"); }
CLiquidComponent::~CLiquidComponent() {}
void CLiquidComponent::Initialize()
{
	BaseFuncLiquidComponent::Initialize();

	GetEntity().AddComponent<CLiquidSurfaceComponent>();
	/*BindEvent(CRenderComponent::EVENT_SHOULD_DRAW,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		// TODO: Run CRenderComponent::UpdateShouldDrawState when any of these change
		if(!(m_hWaterSurface.valid() == false && (m_waterScene == nullptr || m_waterScene->descSetGroupTexEffects != nullptr)))
		{
			static_cast<CEShouldDraw&>(evData.get()).shouldDraw = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		if(GetEntity().IsSpawned() == false)
			return;
		BaseFuncLiquidComponent::InitializeWaterSurface();
		SetupWater();
		ReloadSurfaceSimulator();
	});*/
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		BaseFuncLiquidComponent::InitializeWaterSurface();
		SetupWater();
	});
}
void CLiquidComponent::OnEntitySpawn()
{
	BaseFuncLiquidComponent::OnEntitySpawn();
	auto &ent = static_cast<CBaseEntity &>(GetEntity());
	//auto pPhysComponent = ent.GetPhysicsComponent();
	//if(pPhysComponent != nullptr)
	//	pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent) {
		// pRenderComponent->SetSceneRenderPass(pragma::rendering::SceneRenderPass::Water);
		// pRenderComponent->GetRenderModeProperty()->SetLocked(true);
	}
}
void CLiquidComponent::ReceiveData(NetPacket &packet) {}

void CLiquidComponent::SetupWater()
{
	auto renderC = GetEntity().GetComponent<CLiquidSurfaceComponent>();
	if(renderC.expired())
		return;
	renderC->ClearWaterScene();
	/*auto &mdl = GetModel();
	if(mdl == nullptr)
		return;
	auto &mats = mdl->GetMaterials();
	std::unordered_map<std::size_t,bool> waterMaterialIds;
	waterMaterialIds.reserve(mats.size());
	for(auto it=mats.begin();it!=mats.end();++it)
	{
		auto &hMat = *it;
		if(hMat.IsValid() == false)
			continue;
		auto *mat = static_cast<CMaterial*>(hMat.get());
		auto hShader = mat->GetShader();
		if(hShader.IsValid() == false || dynamic_cast<Shader::Water*>(hShader.get()) == nullptr)
			continue;
		waterMaterialIds.insert(decltype(waterMaterialIds)::value_type({it -mats.begin(),true}));
	}
	if(waterMaterialIds.empty() == true)
		return;
	std::vector<CModelSubMesh*> waterSurfaces;
	for(auto meshId : mdl->GetBaseMeshes())
	{
		auto meshGroup = mdl->GetMeshGroup(meshId);
		if(meshGroup == nullptr)
			continue;
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto *cSubMesh = static_cast<CModelSubMesh*>(subMesh.get());
				auto texId = cSubMesh->GetTexture();
				auto it = waterMaterialIds.find(texId);
				if(it != waterMaterialIds.end())
					waterSurfaces.push_back(cSubMesh);
			}
		}
	}
	if(waterSurfaces.empty() == true)
		return;
	auto *meshSurface = waterSurfaces.front(); // TODO: All surfaces?
	*/
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto surfC = GetEntity().GetComponent<CSurfaceComponent>();
	auto *mesh = surfC.valid() ? surfC->GetMesh() : nullptr;
	auto &mats = mdl->GetMaterials();
	if(!mesh)
		return;
	auto *meshSurface = static_cast<CModelSubMesh *>(mesh);
	auto matIdx = mdl->GetMaterialIndex(*meshSurface);
	auto *mat = matIdx.has_value() ? static_cast<CMaterial *>(mats.at(*matIdx).get()) : nullptr;
	Vector3 min, max;
	mdl->GetCollisionBounds(min, max);
	renderC->InitializeWaterScene(meshSurface->GetVertexPosition(0), meshSurface->GetVertexNormal(0), min, max);
}

void CLiquidComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////////

void CFuncWater::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CLiquidComponent>();
}
