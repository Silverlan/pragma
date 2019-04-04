#include "stdafx_client.h"
#include "pragma/entities/func/c_func_water.h"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/entities/c_water_surface.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/physics/c_phys_water_surface_simulator.hpp"
#include "pragma/gui/widebugmsaatexture.hpp"
#include "pragma/model/vk_mesh.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/console/sh_cmd.h>
#include <image/prosper_render_target.hpp>
#include <wgui/wgui.h>
#include <wgui/types/wirect.h>
#include <pragma/physics/raytraces.h>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;

void Console::commands::debug_water(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{

	static std::unique_ptr<DebugGameGUI> dbg = nullptr;
	dbg = nullptr;
	if(c_game == nullptr || pl == nullptr)
		return;
	auto charComponent = pl->GetEntity().GetCharacterComponent();
	auto ents = command::find_target_entity(state,*charComponent,argv,[](TraceData &trData) {
		trData.SetCollisionFilterMask(trData.GetCollisionFilterGroup() | CollisionMask::Water | CollisionMask::WaterSurface);
	});
	auto bFoundWater = false;
	if(ents.empty() == false)
	{
		for(auto *ent : ents)
		{
			auto *entWater = dynamic_cast<CFuncWater*>(ent);
			if(entWater == nullptr)
				continue;
			static WIHandle hDepthTex = {};
			static EntityHandle hWater = {};
			hWater = entWater->GetHandle();
			dbg = std::make_unique<DebugGameGUI>([entWater]() {
				auto &wgui = WGUI::GetInstance();
				auto *r = wgui.Create<WIBase>();
				const auto size = 256u;
				r->SetSize(size *4,size);

				auto pWaterComponent = entWater->GetComponent<pragma::CWaterComponent>();
				if(pWaterComponent.valid() == false || pWaterComponent->IsWaterSceneValid() == false)
					return WIHandle{};
				auto &waterScene = pWaterComponent->GetWaterScene();
				// Debug GUI
				auto &hdrInfo = waterScene.sceneReflection->GetHDRInfo();
				auto *pReflection = wgui.Create<WITexturedRect>(r);
				pReflection->SetSize(size,size);
				pReflection->SetTexture(*hdrInfo.hdrRenderTarget->GetTexture());
				pReflection->SetName("dbg_water_reflection");

				auto *pRefractionDepth = wgui.Create<WIDebugDepthTexture>(r);
				pRefractionDepth->SetSize(size,size);
				pRefractionDepth->SetX(size);
				pRefractionDepth->SetTexture(*hdrInfo.prepass.textureDepth,{//*waterScene.texSceneDepth
					Anvil::PipelineStageFlagBits::LATE_FRAGMENT_TESTS_BIT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AccessFlagBits::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
				},{
					Anvil::PipelineStageFlagBits::EARLY_FRAGMENT_TESTS_BIT,Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL,Anvil::AccessFlagBits::DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
				});
				pRefractionDepth->SetShouldResolveImage(true);
				pRefractionDepth->SetName("dbg_water_refraction_depth");
				hDepthTex = pRefractionDepth->GetHandle();

				auto *pSceneNoWater = wgui.Create<WIDebugMSAATexture>(r);
				pSceneNoWater->SetSize(size,size);
				pSceneNoWater->SetX(size *2u);
				pSceneNoWater->SetTexture(*waterScene.texScene);
				pSceneNoWater->SetShouldResolveImage(false);
				pSceneNoWater->SetName("dbg_water_scene");
				return r->GetHandle();
			});
			dbg->AddCallback("PostRenderScenes",FunctionCallback<void>::Create([]() {
				if(hWater.IsValid() == false)
					return;
				auto *entWater = static_cast<CFuncWater*>(hWater.get());
				auto *cam = c_game->GetRenderCamera();
				// Update debug depth GUI element
				if(hDepthTex.IsValid() && cam != nullptr)
				{
					auto *pDepthTex = static_cast<WIDebugDepthTexture*>(hDepthTex.get());
					pDepthTex->Update(cam->GetZNear(),cam->GetZFar());
				}
			}));

			// Debug surface points
			auto pWaterComponent = entWater->GetComponent<pragma::CWaterComponent>();
			auto *entSurface = pWaterComponent.valid() ? pWaterComponent->GetSurfaceEntity() : nullptr;
			if(entSurface != nullptr)
			{
				auto pWaterSurfComponent = entSurface->GetComponent<pragma::CWaterSurfaceComponent>();
				auto *meshSurface = pWaterSurfComponent.valid() ? pWaterSurfComponent->GetWaterSurfaceMesh() : nullptr;
				if(meshSurface != nullptr)
				{
					auto &vkMesh = meshSurface->GetVKMesh();
					auto *sim = static_cast<const CPhysWaterSurfaceSimulator*>(pWaterComponent->GetSurfaceSimulator());
					//auto &buf = sim->GetPositionBuffer();
					auto dbgPoints = DebugRenderer::DrawPoints(vkMesh->GetVertexBuffer(),meshSurface->GetVertexCount(),Color::Yellow);
					if(dbgPoints != nullptr)
					{
						dbg->CallOnRemove([dbgPoints]() mutable {
							dbgPoints = nullptr;
						});
					}
				}
			}
			bFoundWater = true;
			break;
		}
	}
	if(bFoundWater == false)
		Con::cwar<<"WARNING: No water entity found!"<<Con::endl;
}

