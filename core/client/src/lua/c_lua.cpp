#include "stdafx_client.h"
#include "pragma/rendering/scene/e_frustum.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/libraries/c_lengine.h"
#include "pragma/lua/libraries/c_lgame.h"
#include "pragma/lua/libraries/lents.h"
#include "pragma/lua/libraries/c_lnetmessages.h"
#include "pragma/lua/libraries/c_lents.h"
#include "pragma/lua/libraries/c_lrender.h"
#include "pragma/lua/libraries/lengine.h"
#include "pragma/rendering/c_rendermode.h"
#include <mathutil/glmutil.h>
#include "luasystem.h"
#include "pragma/rendering/shaders/c_shaderlua.h"
#include "cmaterialmanager.h"
#include <wgui/wgui.h>
#include <wgui/wibase.h>
#include "pragma/lua/libraries/lray.h"
#include "pragma/audio/c_alsound.h"
#include "pragma/lua/libraries/c_lglobal.h"
#include "pragma/lua/libraries/c_llocale.h"
#include "pragma/lua/libraries/lgame.h"
#include <sharedutils/util_string.h>
#include "pragma/lua/classes/c_lnetpacket.h"
#include "pragma/lua/classes/c_lcamera.h"
#include "pragma/entities/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_lua_component.hpp"
#include "pragma/lua/classes/c_lentity.h"
#include "pragma/lua/classes/c_lua_entity.h"
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/sh_lua_component_wrapper.hpp>
#include <pragma/lua/lentity_component_lua.hpp>
#include <pragma/lua/sh_lua_component.hpp>
#include <pragma/lua/sh_lua_component_t.hpp>
#include <pragma/networking/resources.h>
#include <pragma/input/inputhelper.h>
#include <pragma/lua/libraries/lnet.hpp>
#include <luainterface.hpp>
#include <pragma/lua/lua_component_event.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_render_target.hpp>
#ifdef PHYS_ENGINE_PHYSX
#include <PxVisualizationParameter.h>
#endif
#include "pragma/debug/c_debug_vehicle.hpp"

#undef LEFT
#undef RIGHT
#undef TOP
#undef BOTTOM
#undef NEAR
#undef FAR

void CGame::RegisterLua()
{
	GetLuaInterface().SetIdentifier("cl");

	auto &modEngine = GetLuaInterface().RegisterLibrary("engine",{
		{"bind_key",Lua::engine::bind_key},
		{"unbind_key",Lua::engine::unbind_key},
		{"load_library",&Lua_engine_LoadLibrary},
		{"create_font",&Lua::engine::create_font},
		{"get_font",&Lua::engine::get_font},
		{"get_text_size",&Lua::engine::get_text_size},
		//{"save_frame_buffer_as_tga",&Lua::engine::save_frame_buffer_as_tga},
		//{"save_texture_as_tga",&Lua::engine::save_texture_as_tga},
		{"get_tick_count",&Lua_engine_GetTickCount}
	});

	Lua::RegisterLibrary(GetLuaState(),"game",{
		LUA_LIB_GAME_SHARED
		//{"create_light",Lua_engine_CreateLight},
		//{"remove_lights",Lua_engine_RemoveLights},
		//{"create_sprite",Lua_engine_CreateSprite},
		{"precache_model",Lua::engine::precache_model},
		{"precache_material",Lua::engine::precache_material},
		{"load_sound_scripts",Lua_engine_LoadSoundScripts},
		{"load_material",Lua::engine::load_material},
		//{"create_texture",&Lua::engine::create_texture},
		{"create_material",Lua::engine::create_material},
		{"create_particle_system",&Lua::engine::create_particle_system},
		{"precache_particle_system",&Lua::engine::precache_particle_system},
		{"save_particle_system",&Lua::engine::save_particle_system},

		{"test",Lua::game::Client::test},
		{"open_dropped_file",Lua::game::Client::open_dropped_file},
		{"set_gravity",Lua::game::Client::set_gravity},
		{"get_gravity",Lua::game::Client::get_gravity},
		{"load_model",Lua::game::Client::load_model},
		{"create_model",Lua::game::Client::create_model},
#ifdef PHYS_ENGINE_PHYSX
		{"get_physx_scene",Lua::game::physx_scene},
#endif
		{"get_action_input",Lua::game::Client::get_action_input},
		{"set_action_input",Lua::game::Client::set_action_input},
		{"draw_scene",Lua::game::Client::draw_scene},
		{"create_scene",Lua::game::Client::create_scene},
		{"get_render_scene",Lua::game::Client::get_render_scene},
		{"get_render_scene_camera",Lua::game::Client::get_render_scene_camera},
		{"get_scene",Lua::game::Client::get_scene},
		{"get_scene_camera",Lua::game::Client::get_scene_camera},
		{"get_draw_command_buffer",Lua::game::Client::get_draw_command_buffer},
		{"get_setup_command_buffer",Lua::game::Client::get_setup_command_buffer},
		{"flush_setup_command_buffer",Lua::game::Client::flush_setup_command_buffer},
		{"get_camera_transform",Lua::game::Client::get_camera_position},
		{"get_render_clip_plane",Lua::game::Client::get_render_clip_plane},
		{"set_render_clip_plane",Lua::game::Client::set_render_clip_plane},

		{"get_debug_buffer",Lua::game::Client::get_debug_buffer},
		{"get_time_buffer",Lua::game::Client::get_time_buffer},
		{"get_csm_buffer",Lua::game::Client::get_csm_buffer},
		{"get_render_settings_descriptor_set",Lua::game::Client::get_render_settings_descriptor_set}
		
		/*{"debug_vehicle",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			Con::cout<<"Creating vehicle..."<<Con::endl;
			static auto vhc = std::make_shared<debug::Vehicle>();
			return 0;
		})}*/
	});

	Lua::RegisterLibrary(GetLuaState(),"render",{
		{"set_material_override",Lua_render_SetMaterialOverride},
		{"set_color_scale",Lua_render_SetColorScale},
		{"set_alpha_scale",Lua_render_SetAlphaScale},
	});

	auto &modEnts = GetLuaInterface().RegisterLibrary("ents",{
		LUA_LIB_ENTS_SHARED
		{"get_local_player",Lua::ents::Client::get_local_player},
		{"get_listener",Lua::ents::Client::get_listener},
		{"get_view_body",Lua::ents::Client::get_view_body},
		{"get_view_model",Lua::ents::Client::get_view_model},
		{"get_instance_buffer",Lua::ents::Client::get_instance_buffer},
		{"get_instance_bone_buffer",Lua::ents::Client::get_instance_bone_buffer},
		{"register_component",Lua::ents::register_component<pragma::CLuaBaseEntityComponent>},
		{"register_component_event",Lua::ents::register_component_event}
	});

	auto entityClassDef = luabind::class_<EntityHandle>("Entity");
	Lua::Entity::Client::register_class(entityClassDef);
	modEnts[entityClassDef];

	auto classDefBase = luabind::class_<CLuaEntityHandle,luabind::bases<EntityHandle>,luabind::default_holder,CLuaEntityWrapper>("BaseEntity");
	classDefBase.def(luabind::tostring(luabind::self));
	classDefBase.def(luabind::constructor<>());
	classDefBase.def("Initialize",&CLuaEntityWrapper::Initialize,&CLuaEntityWrapper::default_Initialize);
	//classDefBase.def("ReceiveNetEvent",&SLuaEntityWrapper::ReceiveNetEvent,&SLuaBaseEntityWrapper::default_ReceiveNetEvent);
	modEnts[classDefBase];

	auto &modNet = GetLuaInterface().RegisterLibrary("net",{
		{"send",Lua_cl_net_Send},
		{"receive",Lua_cl_net_Receive},
		{"register_event",Lua::net::register_event}
	});
	auto netPacketClassDef = luabind::class_<NetPacket>("Packet");
	Lua::NetPacket::Client::register_class(netPacketClassDef);
	netPacketClassDef.def("WritePlayer",static_cast<void(*)(lua_State*,::NetPacket&,util::WeakHandle<pragma::CPlayerComponent>&)>([](lua_State *l,::NetPacket &packet,util::WeakHandle<pragma::CPlayerComponent> &pl) {
		nwm::write_player(packet,pl.get());
	}));
	netPacketClassDef.def("WritePlayer",static_cast<void(*)(lua_State*,::NetPacket&,EntityHandle&)>([](lua_State *l,::NetPacket &packet,EntityHandle &hEnt) {
		nwm::write_player(packet,hEnt.get());
	}));
	netPacketClassDef.def("ReadPlayer",static_cast<void(*)(lua_State*,::NetPacket&)>([](lua_State *l,::NetPacket &packet) {
		auto *pl = static_cast<pragma::CPlayerComponent*>(nwm::read_player(packet));
		if(pl == nullptr)
			return;
		pl->PushLuaObject(l);
	}));
	modNet[netPacketClassDef];
	Lua::net::RegisterLibraryEnums(GetLuaState());

	Lua::RegisterLibrary(GetLuaState(),"locale",{
		{"get_text",Lua::Locale::get_text},
		{"load",Lua::Locale::load},
		{"get_language",Lua::Locale::get_language}
	});

	Game::RegisterLua();
	/*lua_bind(
		luabind::class_<ListenerHandle,EntityHandle>("Listener")
		.def("SetGain",&Lua_Listener_SetGain)
		.def("GetGain",&Lua_Listener_GetGain)
	);*/

	lua_pushboolean(GetLuaState(),1);
	lua_setglobal(GetLuaState(),"CLIENT");

	lua_pushboolean(GetLuaState(),0);
	lua_setglobal(GetLuaState(),"SERVER");

	lua_pushtablecfunction(GetLuaState(),"time","server_time",Lua_ServerTime);
	lua_pushtablecfunction(GetLuaState(),"time","frame_time",Lua_FrameTime);

	Lua::RegisterLibraryEnums(GetLuaState(),"sound",{
		{"CHANNEL_CONFIG_MONO",umath::to_integral(al::ChannelConfig::Mono)},
		{"CHANNEL_CONFIG_STEREO",umath::to_integral(al::ChannelConfig::Stereo)},
		{"CHANNEL_CONFIG_REAR",umath::to_integral(al::ChannelConfig::Rear)},
		{"CHANNEL_CONFIG_QUAD",umath::to_integral(al::ChannelConfig::Quad)},
		{"CHANNEL_CONFIG_X51",umath::to_integral(al::ChannelConfig::X51)},
		{"CHANNEL_CONFIG_X61",umath::to_integral(al::ChannelConfig::X61)},
		{"CHANNEL_CONFIG_X71",umath::to_integral(al::ChannelConfig::X71)},
		{"CHANNEL_CONFIG_BFORMAT_2D",umath::to_integral(al::ChannelConfig::BFormat2D)},
		{"CHANNEL_CONFIG_BFORMAT_3D",umath::to_integral(al::ChannelConfig::BFormat3D)},

		{"SAMPLE_TYPE_UINT8",umath::to_integral(al::SampleType::UInt8)},
		{"SAMPLE_TYPE_INT16",umath::to_integral(al::SampleType::Int16)},
		{"SAMPLE_TYPE_FLOAT32",umath::to_integral(al::SampleType::Float32)},
		{"SAMPLE_TYPE_MULAW",umath::to_integral(al::SampleType::Mulaw)}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"time",{
		{"TIMER_TYPE_SERVERTIME",umath::to_integral(TimerType::ServerTime)}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"file",{
		{"SEARCH_RESOURCES",FSYS_SEARCH_RESOURCES}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"geometry",{
		{"FRUSTUM_PLANE_LEFT",umath::to_integral(FRUSTUM_PLANE::LEFT)},
		{"FRUSTUM_PLANE_RIGHT",umath::to_integral(FRUSTUM_PLANE::RIGHT)},
		{"FRUSTUM_PLANE_TOP",umath::to_integral(FRUSTUM_PLANE::TOP)},
		{"FRUSTUM_PLANE_BOTTOM",umath::to_integral(FRUSTUM_PLANE::BOTTOM)},
		{"FRUSTUM_PLANE_NEAR",umath::to_integral(FRUSTUM_PLANE::NEAR)},
		{"FRUSTUM_PLANE_FAR",umath::to_integral(FRUSTUM_PLANE::FAR)},
		{"FRUSTUM_PLANE_COUNT",umath::to_integral(FRUSTUM_PLANE::COUNT)},

		{"FRUSTUM_POINT_FAR_BOTTOM_LEFT",umath::to_integral(FRUSTUM_POINT::FAR_BOTTOM_LEFT)},
		{"FRUSTUM_POINT_FAR_TOP_LEFT",umath::to_integral(FRUSTUM_POINT::FAR_TOP_LEFT)},
		{"FRUSTUM_POINT_FAR_TOP_RIGHT",umath::to_integral(FRUSTUM_POINT::FAR_TOP_RIGHT)},
		{"FRUSTUM_POINT_FAR_BOTTOM_RIGHT",umath::to_integral(FRUSTUM_POINT::FAR_BOTTOM_RIGHT)},
		{"FRUSTUM_POINT_NEAR_BOTTOM_LEFT",umath::to_integral(FRUSTUM_POINT::NEAR_BOTTOM_LEFT)},
		{"FRUSTUM_POINT_NEAR_TOP_LEFT",umath::to_integral(FRUSTUM_POINT::NEAR_TOP_LEFT)},
		{"FRUSTUM_POINT_NEAR_TOP_RIGHT",umath::to_integral(FRUSTUM_POINT::NEAR_TOP_RIGHT)},
		{"FRUSTUM_POINT_NEAR_BOTTOM_RIGHT",umath::to_integral(FRUSTUM_POINT::NEAR_BOTTOM_RIGHT)}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"gui",{
		{"CURSOR_SHAPE_DEFAULT",umath::to_integral(GLFW::Cursor::Shape::Default)},
		{"CURSOR_SHAPE_HIDDEN",umath::to_integral(GLFW::Cursor::Shape::Hidden)},
		{"CURSOR_SHAPE_ARROW",umath::to_integral(GLFW::Cursor::Shape::Arrow)},
		{"CURSOR_SHAPE_IBEAM",umath::to_integral(GLFW::Cursor::Shape::IBeam)},
		{"CURSOR_SHAPE_CROSSHAIR",umath::to_integral(GLFW::Cursor::Shape::Crosshair)},
		{"CURSOR_SHAPE_HAND",umath::to_integral(GLFW::Cursor::Shape::Hand)},
		{"CURSOR_SHAPE_HRESIZE",umath::to_integral(GLFW::Cursor::Shape::HResize)},
		{"CURSOR_SHAPE_VRESIZE",umath::to_integral(GLFW::Cursor::Shape::VResize)},

		{"CURSOR_MODE_DISABLED",umath::to_integral(GLFW::CursorMode::Disabled)},
		{"CURSOR_MODE_HIDDEN",umath::to_integral(GLFW::CursorMode::Hidden)},
		{"CURSOR_MODE_NORMAL",umath::to_integral(GLFW::CursorMode::Normal)}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"game",{
		{"RENDER_FLAG_NONE",0},
		{"RENDER_FLAG_BIT_WORLD",umath::to_integral(FRender::World)},
		{"RENDER_FLAG_BIT_VIEW",umath::to_integral(FRender::View)},
		{"RENDER_FLAG_BIT_SKYBOX",umath::to_integral(FRender::Skybox)},
		{"RENDER_FLAG_BIT_SHADOWS",umath::to_integral(FRender::Shadows)},
		{"RENDER_FLAG_BIT_PARTICLES",umath::to_integral(FRender::Particles)},
		{"RENDER_FLAG_BIT_GLOW",umath::to_integral(FRender::Glow)},
		{"RENDER_FLAG_BIT_DEBUG",umath::to_integral(FRender::Debug)},
		{"RENDER_FLAG_ALL",umath::to_integral(FRender::All)},
		{"RENDER_FLAG_REFLECTION_BIT",umath::to_integral(FRender::Reflection)},
		{"RENDER_FLAG_WATER_BIT",umath::to_integral(FRender::Water)}
	});

	auto gameMod = luabind::module(GetLuaState(),"game");
	auto classDefCamera = luabind::class_<std::shared_ptr<Camera>>("Camera");
	classDefCamera.scope[luabind::def("Create",&Lua::Camera::Create)];
	classDefCamera.def("Copy",&Lua::Camera::Copy);
	classDefCamera.def("GetProjectionMatrix",&Lua::Camera::GetProjectionMatrix);
	classDefCamera.def("GetViewMatrix",&Lua::Camera::GetViewMatrix);
	classDefCamera.def("GetRight",&Lua::Camera::GetRight);
	classDefCamera.def("GetUp",&Lua::Camera::GetUp);
	classDefCamera.def("SetUp",&Lua::Camera::SetUp);
	classDefCamera.def("GetPos",&Lua::Camera::GetPos);
	classDefCamera.def("SetPos",&Lua::Camera::SetPos);
	classDefCamera.def("LookAt",&Lua::Camera::LookAt);
	classDefCamera.def("UpdateMatrices",&Lua::Camera::UpdateMatrices);
	classDefCamera.def("UpdateViewMatrix",&Lua::Camera::UpdateViewMatrix);
	classDefCamera.def("UpdateProjectionMatrix",&Lua::Camera::UpdateProjectionMatrix);
	classDefCamera.def("UpdateViewProjectionMatrix",&Lua::Camera::UpdateViewProjectionMatrix);
	classDefCamera.def("UpdateProjectionMatrices",&Lua::Camera::UpdateProjectionMatrices);
	classDefCamera.def("SetFOV",&Lua::Camera::SetFOV);
	classDefCamera.def("SetViewFOV",&Lua::Camera::SetViewFOV);
	classDefCamera.def("SetAspectRatio",&Lua::Camera::SetAspectRatio);
	classDefCamera.def("SetNearZ",&Lua::Camera::SetZNear);
	classDefCamera.def("SetFarZ",&Lua::Camera::SetZFar);
	classDefCamera.def("SetForward",&Lua::Camera::SetForward);
	classDefCamera.def("GetViewProjectionMatrix",&Lua::Camera::GetViewProjectionMatrix);
	classDefCamera.def("GetForward",&Lua::Camera::GetForward);
	classDefCamera.def("GetFOV",&Lua::Camera::GetFOV);
	classDefCamera.def("GetViewFOV",&Lua::Camera::GetViewFOV);
	classDefCamera.def("GetFOVRad",&Lua::Camera::GetFOVRad);
	classDefCamera.def("GetViewFOVRad",&Lua::Camera::GetViewFOVRad);
	classDefCamera.def("GetAspectRatio",&Lua::Camera::GetAspectRatio);
	classDefCamera.def("GetNearZ",&Lua::Camera::GetZNear);
	classDefCamera.def("GetFarZ",&Lua::Camera::GetZFar);
	classDefCamera.def("GetFrustumPlanes",&Lua::Camera::GetFrustumPlanes);
	classDefCamera.def("GetFarPlaneCenter",&Lua::Camera::GetFarPlaneCenter);
	classDefCamera.def("GetNearPlaneCenter",&Lua::Camera::GetNearPlaneCenter);
	classDefCamera.def("GetFarPlanePoints",&Lua::Camera::GetFarPlaneBoundaries);
	classDefCamera.def("GetNearPlanePoints",&Lua::Camera::GetNearPlaneBoundaries);
	classDefCamera.def("GetPlanePoints",&Lua::Camera::GetPlaneBoundaries);
	classDefCamera.def("SetProjectionMatrix",&Lua::Camera::SetProjectionMatrix);
	classDefCamera.def("SetViewMatrix",&Lua::Camera::SetViewMatrix);
	classDefCamera.def("SetViewProjectionMatrix",&Lua::Camera::SetViewProjectionMatrix);
	classDefCamera.def("GetFarPlaneBounds",&Lua::Camera::GetFarPlaneBounds);
	classDefCamera.def("GetNearPlaneBounds",&Lua::Camera::GetNearPlaneBounds);
	classDefCamera.def("GetFrustumNeighbors",&Lua::Camera::GetFrustumNeighbors);
	classDefCamera.def("GetFrustumPlaneCornerPoints",&Lua::Camera::GetFrustumPlaneCornerPoints);
	classDefCamera.def("CreateFrustumKDop",static_cast<void(*)(lua_State*,std::shared_ptr<::Camera>&,luabind::object,luabind::object,Vector3)>(&Lua::Camera::CreateFrustumKDop));
	classDefCamera.def("CreateFrustumKDop",static_cast<void(*)(lua_State*,std::shared_ptr<::Camera>&,const Vector2&,const Vector2&)>(&Lua::Camera::CreateFrustumKDop));
	classDefCamera.def("GetFrustumPoints",&Lua::Camera::GetFrustumPoints);
	classDefCamera.def("GetNearPlanePoint",&Lua::Camera::GetNearPlanePoint);
	classDefCamera.def("GetFarPlanePoint",&Lua::Camera::GetFarPlanePoint);
	classDefCamera.def("GetRotation",&Lua::Camera::GetRotation);
	classDefCamera.def("CreateFrustumMesh",&Lua::Camera::CreateFrustumMesh);
	classDefCamera.def("GetProjectionMatrixProperty",static_cast<void(*)(lua_State*,std::shared_ptr<Camera>&)>([](lua_State *l,std::shared_ptr<Camera> &pCam) {
		Lua::Property::push(l,*pCam->GetProjectionMatrixProperty());
	}));
	classDefCamera.def("GetViewProjectionMatrixProperty",static_cast<void(*)(lua_State*,std::shared_ptr<Camera>&)>([](lua_State *l,std::shared_ptr<Camera> &pCam) {
		Lua::Property::push(l,*pCam->GetViewProjectionMatrixProperty());
	}));
	classDefCamera.def("GetViewMatrixProperty",static_cast<void(*)(lua_State*,std::shared_ptr<Camera>&)>([](lua_State *l,std::shared_ptr<Camera> &pCam) {
		Lua::Property::push(l,*pCam->GetViewMatrixProperty());
	}));
	classDefCamera.def("GetUpProperty",static_cast<void(*)(lua_State*,std::shared_ptr<Camera>&)>([](lua_State *l,std::shared_ptr<Camera> &pCam) {
		Lua::Property::push(l,*pCam->GetUpProperty());
	}));
	classDefCamera.def("GetForwardProperty",static_cast<void(*)(lua_State*,std::shared_ptr<Camera>&)>([](lua_State *l,std::shared_ptr<Camera> &pCam) {
		Lua::Property::push(l,*pCam->GetForwardProperty());
	}));
	classDefCamera.def("GetPosProperty",static_cast<void(*)(lua_State*,std::shared_ptr<Camera>&)>([](lua_State *l,std::shared_ptr<Camera> &pCam) {
		Lua::Property::push(l,*pCam->GetPosProperty());
	}));
	classDefCamera.def("GetNearZProperty",static_cast<void(*)(lua_State*,std::shared_ptr<Camera>&)>([](lua_State *l,std::shared_ptr<Camera> &pCam) {
		Lua::Property::push(l,*pCam->GetNearZProperty());
	}));
	classDefCamera.def("GetFarZProperty",static_cast<void(*)(lua_State*,std::shared_ptr<Camera>&)>([](lua_State *l,std::shared_ptr<Camera> &pCam) {
		Lua::Property::push(l,*pCam->GetFarZProperty());
	}));
	classDefCamera.def("GetFOVProperty",static_cast<void(*)(lua_State*,std::shared_ptr<Camera>&)>([](lua_State *l,std::shared_ptr<Camera> &pCam) {
		Lua::Property::push(l,*pCam->GetFOVProperty());
	}));
	classDefCamera.def("GetViewFOVProperty",static_cast<void(*)(lua_State*,std::shared_ptr<Camera>&)>([](lua_State *l,std::shared_ptr<Camera> &pCam) {
		Lua::Property::push(l,*pCam->GetViewFOVProperty());
	}));
	gameMod[classDefCamera];

	auto classDefScene = luabind::class_<std::shared_ptr<Scene>>("Scene");
	classDefScene.def("GetCamera",&Lua::Scene::GetCamera);
	classDefScene.def("GetWidth",&Lua::Scene::GetWidth);
	classDefScene.def("GetHeight",&Lua::Scene::GetHeight);
	classDefScene.def("GetSize",&Lua::Scene::GetSize);
	classDefScene.def("Resize",&Lua::Scene::Resize);
	classDefScene.def("BeginDraw",&Lua::Scene::BeginDraw);
	classDefScene.def("UpdateBuffers",&Lua::Scene::UpdateBuffers);
	classDefScene.def("GetWorldEnvironment",&Lua::Scene::GetWorldEnvironment);
	classDefScene.def("SetWorldEnvironment",&Lua::Scene::SetWorldEnvironment);
	classDefScene.def("ClearWorldEnvironment",&Lua::Scene::ClearWorldEnvironment);
	classDefScene.def("InitializeRenderTarget",&Lua::Scene::InitializeRenderTarget);
	classDefScene.def("GetPrepassDepthTexture",&Lua::Scene::GetPrepassDepthTexture);
	classDefScene.def("GetPostPrepassDepthTexture", static_cast<void(*)(lua_State*, std::shared_ptr<::Scene>&)>([](lua_State *l, std::shared_ptr<::Scene> &scene) {
		auto &depthTex = scene->GetPrepass().textureDepthSampled;
		if (depthTex == nullptr)
			return;
		Lua::Push(l, depthTex);
	}));
	classDefScene.def("GetPrepassNormalTexture",&Lua::Scene::GetPrepassNormalTexture);
	classDefScene.def("GetRenderTarget",&Lua::Scene::GetRenderTarget);
	classDefScene.def("GetStagingRenderTarget",static_cast<void(*)(lua_State*,std::shared_ptr<Scene>&)>([](lua_State *l,std::shared_ptr<Scene> &scene) {
		auto &rt = scene->GetHDRInfo().hdrStagingRenderTarget;
		if(rt == nullptr)
			return;
		Lua::Push(l,rt);
	}));
	classDefScene.def("GetRenderTargetTextureDescriptorSet",static_cast<void(*)(lua_State*,std::shared_ptr<Scene>&)>([](lua_State *l,std::shared_ptr<Scene> &scene) {
		auto &dsg = scene->GetHDRInfo().descSetGroupHdr;
		if(dsg == nullptr)
			return;
		Lua::Push(l,dsg);
	}));

	classDefScene.def("BeginRenderPass",static_cast<void(*)(lua_State*,std::shared_ptr<::Scene>&,std::shared_ptr<prosper::CommandBuffer>&,std::shared_ptr<prosper::RenderPass>&)>(&Lua::Scene::BeginRenderPass));
	classDefScene.def("BeginRenderPass",static_cast<void(*)(lua_State*,std::shared_ptr<::Scene>&,std::shared_ptr<prosper::CommandBuffer>&)>(&Lua::Scene::BeginRenderPass));
	classDefScene.def("EndRenderPass",&Lua::Scene::EndRenderPass);
	classDefScene.def("SetLightSources",&Lua::Scene::SetLightSources);
	classDefScene.def("GetLightSources",&Lua::Scene::GetLightSources);
	classDefScene.def("LinkLightSources",&Lua::Scene::LinkLightSources);
	classDefScene.def("SetEntities",&Lua::Scene::SetEntities);
	classDefScene.def("GetEntities",&Lua::Scene::GetEntities);
	classDefScene.def("LinkEntities",&Lua::Scene::LinkEntities);
	classDefScene.def("GetCameraDescriptorSet",static_cast<void(*)(lua_State*,std::shared_ptr<::Scene>&,uint32_t)>(&Lua::Scene::GetCameraDescriptorSet));
	classDefScene.def("GetCameraDescriptorSet",static_cast<void(*)(lua_State*,std::shared_ptr<::Scene>&)>(&Lua::Scene::GetCameraDescriptorSet));
	classDefScene.def("GetViewCameraDescriptorSet",&Lua::Scene::GetViewCameraDescriptorSet);
	classDefScene.def("SetShaderOverride",&Lua::Scene::SetShaderOverride);
	classDefScene.def("ClearShaderOverride",&Lua::Scene::ClearShaderOverride);
	classDefScene.def("GetPrepassShader",&Lua::Scene::GetPrepassShader);
	classDefScene.def("SetPrepassMode",&Lua::Scene::SetPrepassMode);
	classDefScene.def("GetPrepassMode",&Lua::Scene::GetPrepassMode);
	classDefScene.def("AddLightSource",&Lua::Scene::AddLightSource);
	classDefScene.def("RemoveLightSource",&Lua::Scene::RemoveLightSource);
	classDefScene.def("AddEntity",&Lua::Scene::AddEntity);
	classDefScene.def("RemoveEntity",&Lua::Scene::RemoveEntity);
	classDefScene.add_static_constant("PREPASS_MODE_DISABLED",umath::to_integral(::Scene::PrepassMode::NoPrepass));
	classDefScene.add_static_constant("PREPASS_MODE_DEPTH_ONLY",umath::to_integral(::Scene::PrepassMode::DepthOnly));
	classDefScene.add_static_constant("PREPASS_MODE_EXTENDED",umath::to_integral(::Scene::PrepassMode::Extended));

	// Texture indices for scene render target
	classDefScene.add_static_constant("RENDER_TARGET_TEXTURE_COLOR",0u);
	classDefScene.add_static_constant("RENDER_TARGET_TEXTURE_BLOOM",1u);
	classDefScene.add_static_constant("RENDER_TARGET_TEXTURE_DEPTH",2u);

	gameMod[classDefScene];
	RegisterLuaGameClasses(gameMod);

	// Needs to be registered AFTER RegisterLuaGameClasses has been called!
	auto defEntCmp = luabind::class_<BaseLuaBaseEntityHandle,luabind::bases<BaseEntityComponentHandle>,luabind::default_holder,LuaBaseEntityComponentWrapper>("BaseEntityComponent");
	Lua::register_base_entity_component(defEntCmp);
	defEntCmp.def("ReceiveData",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,NetPacket)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,NetPacket packet) {

	}));
	defEntCmp.def("ReceiveNetEvent",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,uint32_t,NetPacket)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,uint32_t evId,NetPacket packet) {
		
	}));
	defEntCmp.def("ReceiveSnapshotData",static_cast<void(*)(lua_State*,BaseLuaBaseEntityHandle&,NetPacket)>([](lua_State *l,BaseLuaBaseEntityHandle &hComponent,NetPacket packet) {
		
	}));
	modEnts[defEntCmp];

	auto _G = luabind::globals(GetLuaState());
	_G["BaseEntityComponent"] = _G["ents"]["BaseEntityComponent"];
	
	Lua::RegisterLibraryEnums(GetLuaState(),"input",{
		{"STATE_PRESS",GLFW_PRESS},
		{"STATE_RELEASE",GLFW_RELEASE},
		{"STATE_REPEAT",GLFW_REPEAT},

		{"MOUSE_BUTTON_1",GLFW_MOUSE_BUTTON_1},
		{"MOUSE_BUTTON_2",GLFW_MOUSE_BUTTON_2},
		{"MOUSE_BUTTON_3",GLFW_MOUSE_BUTTON_3},
		{"MOUSE_BUTTON_4",GLFW_MOUSE_BUTTON_4},
		{"MOUSE_BUTTON_5",GLFW_MOUSE_BUTTON_5},
		{"MOUSE_BUTTON_6",GLFW_MOUSE_BUTTON_6},
		{"MOUSE_BUTTON_7",GLFW_MOUSE_BUTTON_7},
		{"MOUSE_BUTTON_8",GLFW_MOUSE_BUTTON_8},
		{"MOUSE_BUTTON_LEFT",GLFW_MOUSE_BUTTON_LEFT},
		{"MOUSE_BUTTON_RIGHT",GLFW_MOUSE_BUTTON_RIGHT},
		{"MOUSE_BUTTON_MIDDLE",GLFW_MOUSE_BUTTON_MIDDLE},
		
		{"MOD_NONE",0},
		{"MOD_SHIFT",GLFW_MOD_SHIFT},
		{"MOD_CONTROL",GLFW_MOD_CONTROL},
		{"MOD_ALT",GLFW_MOD_ALT},
		{"MOD_SUPER",GLFW_MOD_SUPER},

		{"KEY_UNKNOWN",GLFW_KEY_UNKNOWN},
		{"KEY_SPACE",GLFW_KEY_SPACE},
		{"KEY_APOSTROPHE",GLFW_KEY_APOSTROPHE},
		{"KEY_COMMA",GLFW_KEY_COMMA},
		{"KEY_MINUS",GLFW_KEY_MINUS},
		{"KEY_PERIOD",GLFW_KEY_PERIOD},
		{"KEY_SLASH",GLFW_KEY_SLASH},
		{"KEY_0",GLFW_KEY_0},
		{"KEY_1",GLFW_KEY_1},
		{"KEY_2",GLFW_KEY_2},
		{"KEY_3",GLFW_KEY_3},
		{"KEY_4",GLFW_KEY_4},
		{"KEY_5",GLFW_KEY_5},
		{"KEY_6",GLFW_KEY_6},
		{"KEY_7",GLFW_KEY_7},
		{"KEY_8",GLFW_KEY_8},
		{"KEY_9",GLFW_KEY_9},
		{"KEY_SEMICOLON",GLFW_KEY_SEMICOLON},
		{"KEY_EQUAL",GLFW_KEY_EQUAL},
		{"KEY_A",GLFW_KEY_A},
		{"KEY_B",GLFW_KEY_B},
		{"KEY_C",GLFW_KEY_C},
		{"KEY_D",GLFW_KEY_D},
		{"KEY_E",GLFW_KEY_E},
		{"KEY_F",GLFW_KEY_F},
		{"KEY_G",GLFW_KEY_G},
		{"KEY_H",GLFW_KEY_H},
		{"KEY_I",GLFW_KEY_I},
		{"KEY_J",GLFW_KEY_J},
		{"KEY_K",GLFW_KEY_K},
		{"KEY_L",GLFW_KEY_L},
		{"KEY_M",GLFW_KEY_M},
		{"KEY_N",GLFW_KEY_N},
		{"KEY_O",GLFW_KEY_O},
		{"KEY_P",GLFW_KEY_P},
		{"KEY_Q",GLFW_KEY_Q},
		{"KEY_R",GLFW_KEY_R},
		{"KEY_S",GLFW_KEY_S},
		{"KEY_T",GLFW_KEY_T},
		{"KEY_U",GLFW_KEY_U},
		{"KEY_V",GLFW_KEY_V},
		{"KEY_W",GLFW_KEY_W},
		{"KEY_X",GLFW_KEY_X},
		{"KEY_Y",GLFW_KEY_Y},
		{"KEY_Z",GLFW_KEY_Z},
		{"KEY_LEFT_BRACKET",GLFW_KEY_LEFT_BRACKET},
		{"KEY_BACKSLASH",GLFW_KEY_BACKSLASH},
		{"KEY_RIGHT_BRACKET",GLFW_KEY_RIGHT_BRACKET},
		{"KEY_GRAVE_ACCENT",GLFW_KEY_GRAVE_ACCENT},
		{"KEY_WORLD_1",GLFW_KEY_WORLD_1},
		{"KEY_WORLD_2",GLFW_KEY_WORLD_2},
		{"KEY_ESCAPE",GLFW_KEY_ESCAPE},
		{"KEY_ENTER",GLFW_KEY_ENTER},
		{"KEY_TAB",GLFW_KEY_TAB},
		{"KEY_BACKSPACE",GLFW_KEY_BACKSPACE},
		{"KEY_INSERT",GLFW_KEY_INSERT},
		{"KEY_DELETE",GLFW_KEY_DELETE},
		{"KEY_RIGHT",GLFW_KEY_RIGHT},
		{"KEY_LEFT",GLFW_KEY_LEFT},
		{"KEY_DOWN",GLFW_KEY_DOWN},
		{"KEY_UP",GLFW_KEY_UP},
		{"KEY_PAGE_UP",GLFW_KEY_PAGE_UP},
		{"KEY_PAGE_DOWN",GLFW_KEY_PAGE_DOWN},
		{"KEY_HOME",GLFW_KEY_HOME},
		{"KEY_END",GLFW_KEY_END},
		{"KEY_CAPS_LOCK",GLFW_KEY_CAPS_LOCK},
		{"KEY_SCROLL_LOCK",GLFW_KEY_SCROLL_LOCK},
		{"KEY_NUM_LOCK",GLFW_KEY_NUM_LOCK},
		{"KEY_PRINT_SCREEN",GLFW_KEY_PRINT_SCREEN},
		{"KEY_PAUSE",GLFW_KEY_PAUSE},
		{"KEY_F1",GLFW_KEY_F1},
		{"KEY_F2",GLFW_KEY_F2},
		{"KEY_F3",GLFW_KEY_F3},
		{"KEY_F4",GLFW_KEY_F4},
		{"KEY_F5",GLFW_KEY_F5},
		{"KEY_F6",GLFW_KEY_F6},
		{"KEY_F7",GLFW_KEY_F7},
		{"KEY_F8",GLFW_KEY_F8},
		{"KEY_F9",GLFW_KEY_F9},
		{"KEY_F10",GLFW_KEY_F10},
		{"KEY_F11",GLFW_KEY_F11},
		{"KEY_F12",GLFW_KEY_F12},
		{"KEY_F13",GLFW_KEY_F13},
		{"KEY_F14",GLFW_KEY_F14},
		{"KEY_F15",GLFW_KEY_F15},
		{"KEY_F16",GLFW_KEY_F16},
		{"KEY_F17",GLFW_KEY_F17},
		{"KEY_F18",GLFW_KEY_F18},
		{"KEY_F19",GLFW_KEY_F19},
		{"KEY_F20",GLFW_KEY_F20},
		{"KEY_F21",GLFW_KEY_F21},
		{"KEY_F22",GLFW_KEY_F22},
		{"KEY_F23",GLFW_KEY_F23},
		{"KEY_F24",GLFW_KEY_F24},
		{"KEY_F25",GLFW_KEY_F25},
		{"KEY_KP_0",GLFW_KEY_KP_0},
		{"KEY_KP_1",GLFW_KEY_KP_1},
		{"KEY_KP_2",GLFW_KEY_KP_2},
		{"KEY_KP_3",GLFW_KEY_KP_3},
		{"KEY_KP_4",GLFW_KEY_KP_4},
		{"KEY_KP_5",GLFW_KEY_KP_5},
		{"KEY_KP_6",GLFW_KEY_KP_6},
		{"KEY_KP_7",GLFW_KEY_KP_7},
		{"KEY_KP_8",GLFW_KEY_KP_8},
		{"KEY_KP_9",GLFW_KEY_KP_9},
		{"KEY_KP_DECIMAL",GLFW_KEY_KP_DECIMAL},
		{"KEY_KP_DIVIDE",GLFW_KEY_KP_DIVIDE},
		{"KEY_KP_MULTIPLY",GLFW_KEY_KP_MULTIPLY},
		{"KEY_KP_SUBTRACT",GLFW_KEY_KP_SUBTRACT},
		{"KEY_KP_ADD",GLFW_KEY_KP_ADD},
		{"KEY_KP_ENTER",GLFW_KEY_KP_ENTER},
		{"KEY_KP_EQUAL",GLFW_KEY_KP_EQUAL},
		{"KEY_LEFT_SHIFT",GLFW_KEY_LEFT_SHIFT},
		{"KEY_LEFT_CONTROL",GLFW_KEY_LEFT_CONTROL},
		{"KEY_LEFT_ALT",GLFW_KEY_LEFT_ALT},
		{"KEY_LEFT_SUPER",GLFW_KEY_LEFT_SUPER},
		{"KEY_RIGHT_SHIFT",GLFW_KEY_RIGHT_SHIFT},
		{"KEY_RIGHT_CONTROL",GLFW_KEY_RIGHT_CONTROL},
		{"KEY_RIGHT_ALT",GLFW_KEY_RIGHT_ALT},
		{"KEY_RIGHT_SUPER",GLFW_KEY_RIGHT_SUPER},
		{"KEY_MENU",GLFW_KEY_MENU},

		{"KEY_SCRL_UP",GLFW_CUSTOM_KEY_SCRL_UP},
		{"KEY_SCRL_DOWN",GLFW_CUSTOM_KEY_SCRL_DOWN},
		{"KEY_SCRL_DOWN",GLFW_CUSTOM_KEY_SCRL_DOWN},

		{"KEY_MOUSE_BUTTON_1",GLFW_KEY_SPECIAL_MOUSE_BUTTON_1},
		{"KEY_MOUSE_BUTTON_2",GLFW_KEY_SPECIAL_MOUSE_BUTTON_2},
		{"KEY_MOUSE_BUTTON_3",GLFW_KEY_SPECIAL_MOUSE_BUTTON_3},
		{"KEY_MOUSE_BUTTON_4",GLFW_KEY_SPECIAL_MOUSE_BUTTON_4},
		{"KEY_MOUSE_BUTTON_5",GLFW_KEY_SPECIAL_MOUSE_BUTTON_5},
		{"KEY_MOUSE_BUTTON_6",GLFW_KEY_SPECIAL_MOUSE_BUTTON_6},
		{"KEY_MOUSE_BUTTON_7",GLFW_KEY_SPECIAL_MOUSE_BUTTON_7},
		{"KEY_MOUSE_BUTTON_8",GLFW_KEY_SPECIAL_MOUSE_BUTTON_8},
		{"KEY_MOUSE_BUTTON_9",GLFW_KEY_SPECIAL_MOUSE_BUTTON_9},

		{"CONTROLLER_0_KEY_START",GLFW_CUSTOM_KEY_JOYSTICK_0_KEY_START},
		{"CONTROLLER_0_AXIS_START",GLFW_CUSTOM_KEY_JOYSTICK_0_AXIS_START},
		{"CONTROLLER_1_KEY_START",GLFW_CUSTOM_KEY_JOYSTICK_1_KEY_START},
		{"CONTROLLER_1_AXIS_START",GLFW_CUSTOM_KEY_JOYSTICK_1_AXIS_START},
		{"CONTROLLER_2_KEY_START",GLFW_CUSTOM_KEY_JOYSTICK_2_KEY_START},
		{"CONTROLLER_2_AXIS_START",GLFW_CUSTOM_KEY_JOYSTICK_2_AXIS_START},
		{"CONTROLLER_3_KEY_START",GLFW_CUSTOM_KEY_JOYSTICK_3_KEY_START},
		{"CONTROLLER_3_AXIS_START",GLFW_CUSTOM_KEY_JOYSTICK_3_AXIS_START},
		{"CONTROLLER_4_KEY_START",GLFW_CUSTOM_KEY_JOYSTICK_4_KEY_START},
		{"CONTROLLER_4_AXIS_START",GLFW_CUSTOM_KEY_JOYSTICK_4_AXIS_START},
		{"CONTROLLER_5_KEY_START",GLFW_CUSTOM_KEY_JOYSTICK_5_KEY_START},
		{"CONTROLLER_5_AXIS_START",GLFW_CUSTOM_KEY_JOYSTICK_5_AXIS_START},
		{"CONTROLLER_6_KEY_START",GLFW_CUSTOM_KEY_JOYSTICK_6_KEY_START},
		{"CONTROLLER_6_AXIS_START",GLFW_CUSTOM_KEY_JOYSTICK_6_AXIS_START},
		{"CONTROLLER_7_KEY_START",GLFW_CUSTOM_KEY_JOYSTICK_7_KEY_START},
		{"CONTROLLER_7_AXIS_START",GLFW_CUSTOM_KEY_JOYSTICK_7_AXIS_START},
		{"CONTROLLER_8_KEY_START",GLFW_CUSTOM_KEY_JOYSTICK_8_KEY_START},
		{"CONTROLLER_8_AXIS_START",GLFW_CUSTOM_KEY_JOYSTICK_8_AXIS_START},
		{"CONTROLLER_9_KEY_START",GLFW_CUSTOM_KEY_JOYSTICK_9_KEY_START},
		{"CONTROLLER_9_AXIS_START",GLFW_CUSTOM_KEY_JOYSTICK_9_AXIS_START}
	});

	/*lua_registerglobalint(SHADER_VERTEX_BUFFER_LOCATION);
	lua_registerglobalint(SHADER_UV_BUFFER_LOCATION);
	lua_registerglobalint(SHADER_NORMAL_BUFFER_LOCATION);*/ // Vulkan TODO

#ifdef PHYS_ENGINE_PHYSX
	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eSCALE);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_SCALE");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eWORLD_AXES);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_WORLD_AXES");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eBODY_AXES);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_BODY_AXES");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eBODY_MASS_AXES);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_MASS_AXES");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eBODY_LIN_VELOCITY);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_LIN_VELOCITY");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eBODY_ANG_VELOCITY);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_ANG_VELOCITY");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eBODY_JOINT_GROUPS);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_JOINT_GROUPS");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCONTACT_POINT);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_CONTACT_POINT");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCONTACT_NORMAL);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_CONTACT_NORMAL");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCONTACT_ERROR);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_CONTACT_ERROR");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCONTACT_FORCE);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_CONTACT_FORCE");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eACTOR_AXES);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_ACTOR_AXES");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCOLLISION_AABBS);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_COLLISION_AABBS");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCOLLISION_SHAPES);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_COLLISION_SHAPES");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCOLLISION_AXES);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_COLLISION_AXES");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCOLLISION_COMPOUNDS);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_COLLISION_COMPOUNDS");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCOLLISION_FNORMALS);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_COLLISION_FNORMALS");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCOLLISION_EDGES);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_COLLISION_EDGES");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCOLLISION_STATIC);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_COLLISION_STATIC");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCOLLISION_DYNAMIC);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_COLLISION_DYNAMIC");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCOLLISION_PAIRS);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_COLLISION_PAIRS");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_JOINT_LOCAL_FRAMES");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eJOINT_LIMITS);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_JOINT_LIMITS");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::ePARTICLE_SYSTEM_POSITION);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_PARTICLE_SYSTEM_POSITION");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::ePARTICLE_SYSTEM_VELOCITY);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_PARTICLE_SYSTEM_VELOCITY");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::ePARTICLE_SYSTEM_COLLISION_NORMAL);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_PARTICLE_SYSTEM_COLLISION_NORMAL");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::ePARTICLE_SYSTEM_BOUNDS);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_PARTICLE_SYSTEM_BOUNDS");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::ePARTICLE_SYSTEM_GRID);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_PARTICLE_SYSTEM_GRID");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::ePARTICLE_SYSTEM_BROADPHASE_BOUNDS);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_PARTICLE_SYSTEM_BROADPHASE_BOUNDS");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::ePARTICLE_SYSTEM_MAX_MOTION_DISTANCE);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_PARTICLE_SYSTEM_MAX_MOTION_DISTANCE");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCULL_BOX);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_CULL_BOX");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCLOTH_VERTICAL);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_CLOTH_VERTICAL");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCLOTH_HORIZONTAL);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_CLOTH_HORIZONTAL");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCLOTH_BENDING);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_CLOTH_BENDING");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCLOTH_SHEARING);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_CLOTH_SHEARING");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eCLOTH_VIRTUAL_PARTICLES);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_CLOTH_VIRTUAL_PARTICLES");

	lua_pushinteger(m_lua,physx::PxVisualizationParameter::eMBP_REGIONS);
	lua_setglobal(m_lua,"FPHYSX_VISUALIZE_MBP_REGIONS");
#endif
}

void CGame::InitializeLua()
{
	Game::InitializeLua();
	CallCallbacks<void,lua_State*>("OnLuaInitialized",GetLuaState());
}

void CGame::SetupLua()
{
	Game::SetupLua();
	RunLuaFiles("autorun\\");
	RunLuaFiles("autorun\\client\\");
	InitializeGameMode();
	//for(auto &subDir : GetLuaEntityDirectories())
	//	LoadLuaEntities(subDir);

	LoadLuaShaders();
}

void CGame::LoadLuaShaders()
{
	std::vector<std::string> files;
	//FileManager::FindFiles("lua\\shaders\\*.lua",&files,NULL); // Deprecated; Shaders have to be explicitely included now
	for(unsigned int i=0;i<files.size();i++)
		LoadLuaShader(files[i]);
}

void CGame::LoadLuaShader(std::string file)
{
	StringToLower(file);
	std::string identifier = file.substr(0,file.length() -4);
	file = "shaders\\" +file;
	ExecuteLuaFile(file);
}

std::string CGame::GetLuaNetworkDirectoryName() const {return "client";}
std::string CGame::GetLuaNetworkFileName() const {return "cl_init.lua";}

//////////////////////////////////////////////

void ClientState::RegisterSharedLuaGlobals(Lua::Interface &lua)
{

}