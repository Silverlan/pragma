/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/liquid/c_liquid_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_surface_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_surface_simulation_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/entities/c_water_surface.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/physics/c_phys_water_surface_simulator.hpp"
#include "pragma/gui/widebugmsaatexture.hpp"
#include "pragma/model/vk_mesh.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/console/sh_cmd.h>
#include <image/prosper_render_target.hpp>
#include <wgui/wgui.h>
#include <wgui/types/wirect.h>
#include <pragma/physics/raytraces.h>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;

void Console::commands::debug_water(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{

	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	dbg = nullptr;
	if(c_game == nullptr || pl == nullptr)
		return;
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	auto ents = command::find_target_entity(state, *charComponent, argv, [](TraceData &trData) { trData.SetCollisionFilterMask(trData.GetCollisionFilterGroup() | CollisionMask::Water | CollisionMask::WaterSurface); });
	auto bFoundWater = false;
	if(ents.empty() == false) {
		for(auto *ent : ents) {
			auto *entWater = dynamic_cast<CFuncWater *>(ent);
			if(entWater == nullptr)
				continue;
			static WIHandle hDepthTex = {};
			static EntityHandle hWater = {};
			hWater = entWater->GetHandle();
			dbg = std::make_unique<DebugGameGUI>([entWater]() {
				auto &wgui = WGUI::GetInstance();
				auto *r = wgui.Create<WIBase>();
				const auto size = 256u;
				r->SetSize(size * 4, size);

				auto pWaterComponent = entWater->GetComponent<pragma::CLiquidSurfaceComponent>();
				if(pWaterComponent.valid() == false || pWaterComponent->IsWaterSceneValid() == false)
					return WIHandle {};
				auto &waterScene = pWaterComponent->GetWaterScene();
				auto *renderer = waterScene.sceneReflection.valid() ? dynamic_cast<const pragma::CRasterizationRendererComponent *>(waterScene.sceneReflection->GetRenderer()) : nullptr;
				if(renderer == nullptr)
					return WIHandle {};
				// Debug GUI
				auto &hdrInfo = renderer->GetHDRInfo();
				auto *pReflection = wgui.Create<WITexturedRect>(r);
				pReflection->SetSize(size, size);
				pReflection->SetTexture(hdrInfo.sceneRenderTarget->GetTexture());
				pReflection->SetName("dbg_water_reflection");

				auto *pRefractionDepth = wgui.Create<WIDebugDepthTexture>(r);
				pRefractionDepth->SetSize(size, size);
				pRefractionDepth->SetX(size);
				pRefractionDepth->SetTexture(*hdrInfo.prepass.textureDepth,
				  {//*waterScene.texSceneDepth
				    prosper::PipelineStageFlags::LateFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit},
				  {prosper::PipelineStageFlags::EarlyFragmentTestsBit, prosper::ImageLayout::DepthStencilAttachmentOptimal, prosper::AccessFlags::DepthStencilAttachmentWriteBit});
				pRefractionDepth->SetShouldResolveImage(true);
				pRefractionDepth->SetName("dbg_water_refraction_depth");
				hDepthTex = pRefractionDepth->GetHandle();

				auto *pSceneNoWater = wgui.Create<WIDebugMSAATexture>(r);
				pSceneNoWater->SetSize(size, size);
				pSceneNoWater->SetX(size * 2u);
				pSceneNoWater->SetTexture(*waterScene.texScene);
				pSceneNoWater->SetShouldResolveImage(false);
				pSceneNoWater->SetName("dbg_water_scene");
				return r->GetHandle();
			});
			dbg->AddCallback("PostRenderScenes", FunctionCallback<void>::Create([]() {
				if(hWater.valid() == false)
					return;
				auto *entWater = static_cast<CFuncWater *>(hWater.get());
				auto *cam = c_game->GetRenderCamera();
				// Update debug depth GUI element
				if(hDepthTex.IsValid() && cam != nullptr) {
					auto *pDepthTex = static_cast<WIDebugDepthTexture *>(hDepthTex.get());
					pDepthTex->Setup(cam->GetNearZ(), cam->GetFarZ());
				}
			}));

			// Debug surface points
			auto pWaterComponent = entWater->GetComponent<pragma::CLiquidSurfaceSimulationComponent>();
			auto *entSurface = pWaterComponent.valid() ? pWaterComponent->GetSurfaceEntity() : nullptr;
			if(entSurface != nullptr) {
				auto pWaterSurfComponent = entSurface->GetComponent<pragma::CWaterSurfaceComponent>();
				auto *meshSurface = pWaterSurfComponent.valid() ? pWaterSurfComponent->GetWaterSurfaceMesh() : nullptr;
				if(meshSurface != nullptr) {
					auto &vkMesh = meshSurface->GetSceneMesh();
					auto *sim = static_cast<const CPhysWaterSurfaceSimulator *>(pWaterComponent->GetSurfaceSimulator());
					//auto &buf = sim->GetPositionBuffer();
					auto dbgPoints = DebugRenderer::DrawPoints(vkMesh->GetVertexBuffer(), meshSurface->GetVertexCount(), Color::Yellow);
					if(dbgPoints != nullptr) {
						dbg->CallOnRemove([dbgPoints]() mutable { dbgPoints = nullptr; });
					}
				}
			}
			bFoundWater = true;
			break;
		}
	}
	if(bFoundWater == false)
		Con::cwar << "No water entity found!" << Con::endl;
}
