#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/entities/components/c_lua_component.hpp"
#include "pragma/entities/components/c_ownable_component.hpp"
#include <pragma/lua/lentity_component_lua.hpp>
#include <pragma/game/game_entity_component.hpp>
#include <pragma/entities/entity_component_manager.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

void CGame::InitializeEntityComponents(pragma::EntityComponentManager &componentManager)
{
	Game::InitializeEntityComponents(componentManager);
	componentManager.RegisterComponentType<pragma::CAIComponent>("ai");
	componentManager.RegisterComponentType<pragma::CCharacterComponent>("character");
	componentManager.RegisterComponentType<pragma::CColorComponent>("color");
	componentManager.RegisterComponentType<pragma::CScoreComponent>("score");
	componentManager.RegisterComponentType<pragma::CFlammableComponent>("flammable");
	componentManager.RegisterComponentType<pragma::CHealthComponent>("health");
	componentManager.RegisterComponentType<pragma::CNameComponent>("name");
	componentManager.RegisterComponentType<pragma::CNetworkedComponent>("networked");
	componentManager.RegisterComponentType<pragma::CObservableComponent>("observable");
	componentManager.RegisterComponentType<pragma::CPhysicsComponent>("physics");
	componentManager.RegisterComponentType<pragma::CPlayerComponent>("player");
	componentManager.RegisterComponentType<pragma::CRadiusComponent>("radius");
	componentManager.RegisterComponentType<pragma::CRenderComponent>("render");
	componentManager.RegisterComponentType<pragma::CSoundEmitterComponent>("sound_emitter");
	componentManager.RegisterComponentType<pragma::CToggleComponent>("toggle");
	componentManager.RegisterComponentType<pragma::CTransformComponent>("transform");
	componentManager.RegisterComponentType<pragma::CVehicleComponent>("vehicle");
	componentManager.RegisterComponentType<pragma::CWeaponComponent>("weapon");
	componentManager.RegisterComponentType<pragma::CWheelComponent>("wheel");
	componentManager.RegisterComponentType<pragma::CSoundDspComponent>("sound_dsp");
	componentManager.RegisterComponentType<pragma::CSoundDspChorusComponent>("sound_dsp_chorus");
	componentManager.RegisterComponentType<pragma::CSoundDspDistortionComponent>("sound_dsp_distortion");
	componentManager.RegisterComponentType<pragma::CSoundDspEAXReverbComponent>("sound_dsp_eax_reverb");
	componentManager.RegisterComponentType<pragma::CSoundDspEchoComponent>("sound_dsp_echo");
	componentManager.RegisterComponentType<pragma::CSoundDspEqualizerComponent>("sound_dsp_equalizer");
	componentManager.RegisterComponentType<pragma::CSoundDspFlangerComponent>("sound_dsp_flanger");
	componentManager.RegisterComponentType<pragma::CCameraComponent>("camera");
	componentManager.RegisterComponentType<pragma::CDecalComponent>("decal");
	componentManager.RegisterComponentType<pragma::CExplosionComponent>("explosion");
	componentManager.RegisterComponentType<pragma::CFireComponent>("fire");
	componentManager.RegisterComponentType<pragma::CFogControllerComponent>("fog_controller");
	componentManager.RegisterComponentType<pragma::CLightComponent>("light");
	componentManager.RegisterComponentType<pragma::CLightDirectionalComponent>("light_directional");
	componentManager.RegisterComponentType<pragma::CLightPointComponent>("light_point");
	componentManager.RegisterComponentType<pragma::CLightSpotComponent>("light_spot");
	componentManager.RegisterComponentType<pragma::CLightSpotVolComponent>("light_spot_volume");
	componentManager.RegisterComponentType<pragma::CMicrophoneComponent>("microphone");
	componentManager.RegisterComponentType<pragma::CParticleSystemComponent>("particle_system");
	componentManager.RegisterComponentType<pragma::CQuakeComponent>("quake");
	componentManager.RegisterComponentType<pragma::CSmokeTrailComponent>("smoke_trail");
	componentManager.RegisterComponentType<pragma::CSoundComponent>("sound");
	componentManager.RegisterComponentType<pragma::CSoundScapeComponent>("sound_scape");
	componentManager.RegisterComponentType<pragma::CSpriteComponent>("sprite");
	componentManager.RegisterComponentType<pragma::CEnvTimescaleComponent>("env_time_scale");
	componentManager.RegisterComponentType<pragma::CWindComponent>("wind");
	componentManager.RegisterComponentType<pragma::CBrushComponent>("brush");
	componentManager.RegisterComponentType<pragma::CKinematicComponent>("kinematic");
	componentManager.RegisterComponentType<pragma::CFuncPhysicsComponent>("func_physics");
	componentManager.RegisterComponentType<pragma::CFuncPortalComponent>("func_portal");
	componentManager.RegisterComponentType<pragma::CWaterComponent>("water");
	componentManager.RegisterComponentType<pragma::CButtonComponent>("button");
	componentManager.RegisterComponentType<pragma::CBotComponent>("bot");
	componentManager.RegisterComponentType<pragma::CPointConstraintBallSocketComponent>("constraint_ball_socket");
	componentManager.RegisterComponentType<pragma::CPointConstraintConeTwistComponent>("constraint_cone_twist");
	componentManager.RegisterComponentType<pragma::CPointConstraintDoFComponent>("constraint_dof");
	componentManager.RegisterComponentType<pragma::CPointConstraintFixedComponent>("constraint_fixed");
	componentManager.RegisterComponentType<pragma::CPointConstraintHingeComponent>("constraint_hinge");
	componentManager.RegisterComponentType<pragma::CPointConstraintSliderComponent>("constraint_slider");
	componentManager.RegisterComponentType<pragma::CPointTargetComponent>("render_target");
	componentManager.RegisterComponentType<pragma::CPropComponent>("prop");
	componentManager.RegisterComponentType<pragma::CPropDynamicComponent>("prop_dynamic");
	componentManager.RegisterComponentType<pragma::CPropPhysicsComponent>("prop_physics");
	componentManager.RegisterComponentType<pragma::CTouchComponent>("touch");
	componentManager.RegisterComponentType<pragma::CSkyboxComponent>("skybox");
	componentManager.RegisterComponentType<pragma::CWorldComponent>("world");
	componentManager.RegisterComponentType<pragma::CShooterComponent>("shooter");
	componentManager.RegisterComponentType<pragma::CModelComponent>("model");
	componentManager.RegisterComponentType<pragma::CAnimatedComponent>("animated");
	componentManager.RegisterComponentType<pragma::CGenericComponent>("entity");
	componentManager.RegisterComponentType<pragma::CIOComponent>("io");
	componentManager.RegisterComponentType<pragma::CTimeScaleComponent>("time_scale");
	componentManager.RegisterComponentType<pragma::CAttachableComponent>("attachable");
	componentManager.RegisterComponentType<pragma::CParentComponent>("parent");
	componentManager.RegisterComponentType<pragma::CReflectionProbeComponent>("reflection_probe");
	componentManager.RegisterComponentType<pragma::CPBRConverterComponent>("pbr_converter");

	componentManager.RegisterComponentType<pragma::CEnvSoundProbeComponent>("sound_probe");
	componentManager.RegisterComponentType<pragma::CWeatherComponent>("weather");
	componentManager.RegisterComponentType<pragma::CFuncSoftPhysicsComponent>("func_soft_physics");
	componentManager.RegisterComponentType<pragma::CWaterSurfaceComponent>("water_surface");
	componentManager.RegisterComponentType<pragma::CFlashlightComponent>("flashlight");
	componentManager.RegisterComponentType<pragma::CListenerComponent>("listener");
	componentManager.RegisterComponentType<pragma::CViewBodyComponent>("view_body");
	componentManager.RegisterComponentType<pragma::CViewModelComponent>("view_model");
	componentManager.RegisterComponentType<pragma::CFlexComponent>("flex");
	componentManager.RegisterComponentType<pragma::CSoftBodyComponent>("softbody");
	componentManager.RegisterComponentType<pragma::CVertexAnimatedComponent>("vertex_animated");
	componentManager.RegisterComponentType<pragma::COwnableComponent>("ownable");
	componentManager.RegisterComponentType<pragma::CDebugTextComponent>("debug_text");
	componentManager.RegisterComponentType<pragma::CDebugPointComponent>("debug_point");
	componentManager.RegisterComponentType<pragma::CDebugLineComponent>("debug_line");
	componentManager.RegisterComponentType<pragma::CDebugBoxComponent>("debug_box");
	componentManager.RegisterComponentType<pragma::CDebugSphereComponent>("debug_sphere");
	componentManager.RegisterComponentType<pragma::CDebugConeComponent>("debug_cone");
	componentManager.RegisterComponentType<pragma::CDebugCylinderComponent>("debug_cylinder");
	componentManager.RegisterComponentType<pragma::CDebugPlaneComponent>("debug_plane");
	componentManager.RegisterComponentType<pragma::CBSPLeafComponent>("bsp_leaf");
	componentManager.RegisterComponentType<pragma::CPointAtTargetComponent>("point_at_target");
	componentManager.RegisterComponentType<pragma::CRaytracingComponent>("raytracing");

	componentManager.RegisterComponentType<pragma::CBSPComponent>("bsp");
	componentManager.RegisterComponentType<pragma::CLightMapComponent>("light_map");

	auto *l = GetLuaState();
	for(auto &evName : {
		"ON_UPDATE_RENDER_DATA",
		"ON_RENDER_BUFFERS_INITIALIZED"
	})
		componentManager.RegisterEvent(evName);
}

pragma::BaseEntityComponent *CGame::CreateLuaEntityComponent(BaseEntity &ent,std::string classname)
{
	return Game::CreateLuaEntityComponent<pragma::CLuaBaseEntityComponent>(ent,classname);
}
