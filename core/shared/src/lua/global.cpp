#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include <pragma/console/conout.h>
#include "pragma/lua/libraries/lprint.h"
#include "pragma/lua/libraries/lutil.h"
#include "pragma/lua/libraries/lmath.h"
#include "pragma/lua/libraries/ldebug.h"
#include <pragma/console/fcvar.h>
#include "pragma/physics/raytraces.h"
#include "pragma/lua/ldefinitions.h"
#include <pragma/physics/movetypes.h>
#include "pragma/audio/e_alstate.h"
#include <pragma/physics/physapi.h>
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_skeleton.hpp"
#include "pragma/lua/classes/ldatastream.h"
#include "luasystem.h"
#include "pragma/entities/parentmode.h"
#include "pragma/model/animation/activities.h"
#include <pragma/math/intersection.h>
#include "pragma/input/inkeys.h"
#include "pragma/entities/observermode.h"
#include "pragma/physics/collisionmasks.h"
#include <fsys/filesystem.h>
#include "pragma/audio/alsound_type.h"
#include "pragma/game/damagetype.h"
#include "noise/noise.h"
#include "pragma/model/animation/fanim.h"
#include "pragma/addonsystem/addonsystem.h"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/util/util_duration_type.hpp"
#include <sharedutils/util_enum.h>
#include <pragma/emessage.h>
#include <luainterface.hpp>

#ifdef __linux__
	#define FILE_ATTRIBUTE_ARCHIVE 0x20
	#define FILE_ATTRIBUTE_COMPRESSED 0x800
	#define FILE_ATTRIBUTE_DEVICE 0x40
	#define FILE_ATTRIBUTE_DIRECTORY 0x10
	#define FILE_ATTRIBUTE_ENCRYPTED 0x4000
	#define FILE_ATTRIBUTE_HIDDEN 0x2
	#define FILE_ATTRIBUTE_NORMAL 0x80
	#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 0x2000 
	#define FILE_ATTRIBUTE_OFFLINE 0x1000
	#define FILE_ATTRIBUTE_READONLY 0x1
	#define FILE_ATTRIBUTE_REPARSE_POINT 0x400
	#define FILE_ATTRIBUTE_SPARSE_FILE 0x200
	#define FILE_ATTRIBUTE_SYSTEM 0x4
	#define FILE_ATTRIBUTE_TEMPORARY 0x100
	#define FILE_ATTRIBUTE_VIRTUAL 0x10000
#endif

void NetworkState::RegisterSharedLuaGlobals(Lua::Interface &lua)
{
	lua_register(lua.GetState(),"include",Lua::global::include);
	lua_register(lua.GetState(),"get_script_path",Lua::global::get_script_path);
	lua_register(lua.GetState(),"toboolean",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
		std::string v = Lua::CheckString(l,1);
		auto b = util::to_boolean(v);
		Lua::PushBool(l,b);
		return 1;
	}));

	Lua::RegisterLibraryEnums(lua.GetState(),"console",{
		{"FOREGROUND_BLUE",FOREGROUND_BLUE},
		{"FOREGROUND_GREEN",FOREGROUND_GREEN},
		{"FOREGROUND_RED",FOREGROUND_RED},
		{"FOREGROUND_INTENSITY",FOREGROUND_INTENSITY},
		{"BACKGROUND_BLUE",BACKGROUND_BLUE},
		{"BACKGROUND_GREEN",BACKGROUND_GREEN},
		{"BACKGROUND_RED",BACKGROUND_RED},
		{"BACKGROUND_INTENSITY",BACKGROUND_INTENSITY}
	});

	Lua::RegisterLibraryEnums(lua.GetState(),"console",{
		{"FLAG_NONE",umath::to_integral(ConVarFlags::None)},
		{"FLAG_BIT_CHEAT",umath::to_integral(ConVarFlags::Cheat)},
		{"FLAG_BIT_SINGLEPLAYER",umath::to_integral(ConVarFlags::Singleplayer)},
		{"FLAG_BIT_USERINFO",umath::to_integral(ConVarFlags::Userinfo)},
		{"FLAG_BIT_REPLICATED",umath::to_integral(ConVarFlags::Replicated)},
		{"FLAG_BIT_ARCHIVE",umath::to_integral(ConVarFlags::Archive)},
		{"FLAG_BIT_NOTIFY",umath::to_integral(ConVarFlags::Notify)},
		{"FLAG_BIT_JOYSTICK_AXIS_CONTINUOUS",umath::to_integral(ConVarFlags::JoystickAxisContinuous)},
		{"FLAG_BIT_JOYSTICK_AXIS_SINGLE",umath::to_integral(ConVarFlags::JoystickAxisSingle)}
	});

	Lua::RegisterLibraryEnums(lua.GetState(),"sound",{
		{"TYPE_GENERIC",umath::to_integral(ALSoundType::Generic)},
		{"TYPE_EFFECT",umath::to_integral(ALSoundType::Effect)},
		{"TYPE_MUSIC",umath::to_integral(ALSoundType::Music)},
		{"TYPE_VOICE",umath::to_integral(ALSoundType::Voice)},
		{"TYPE_WEAPON",umath::to_integral(ALSoundType::Weapon)},
		{"TYPE_NPC",umath::to_integral(ALSoundType::NPC)},
		{"TYPE_PLAYER",umath::to_integral(ALSoundType::Player)},
		{"TYPE_VEHICLE",umath::to_integral(ALSoundType::Vehicle)},
		{"TYPE_PHYSICS",umath::to_integral(ALSoundType::Physics)},
		{"TYPE_ENVIRONMENT",umath::to_integral(ALSoundType::Environment)},
		{"TYPE_GUI",umath::to_integral(ALSoundType::GUI)},

		{"STATE_NO_ERROR",umath::to_integral(ALState::NoError)},
		{"STATE_INITIAL",umath::to_integral(ALState::Initial)},
		{"STATE_PLAYING",umath::to_integral(ALState::Playing)},
		{"STATE_PAUSED",umath::to_integral(ALState::Paused)},
		{"STATE_STOPPED",umath::to_integral(ALState::Stopped)}
	});
}

void Game::RegisterLuaGlobals()
{
	NetworkState::RegisterSharedLuaGlobals(GetLuaInterface());

	Lua::RegisterLibraryEnums(GetLuaState(),"game",{
		{"DAMAGETYPE_GENERIC",umath::to_integral(DAMAGETYPE::GENERIC)},
		{"DAMAGETYPE_BULLET",umath::to_integral(DAMAGETYPE::BULLET)},
		{"DAMAGETYPE_EXPLOSION",umath::to_integral(DAMAGETYPE::EXPLOSION)},
		{"DAMAGETYPE_FIRE",umath::to_integral(DAMAGETYPE::FIRE)},
		{"DAMAGETYPE_PLASMA",umath::to_integral(DAMAGETYPE::PLASMA)},
		{"DAMAGETYPE_BASH",umath::to_integral(DAMAGETYPE::BASH)},
		{"DAMAGETYPE_CRUSH",umath::to_integral(DAMAGETYPE::CRUSH)},
		{"DAMAGETYPE_SLASH",umath::to_integral(DAMAGETYPE::SLASH)},
		{"DAMAGETYPE_ELECTRICITY",umath::to_integral(DAMAGETYPE::ELECTRICITY)},
		{"DAMAGETYPE_ENERGY",umath::to_integral(DAMAGETYPE::ENERGY)},
		
		{"STATE_FLAG_NONE",umath::to_integral(Game::GameFlags::None)},
		{"STATE_FLAG_BIT_MAP_INITIALIZED",umath::to_integral(Game::GameFlags::MapInitialized)},
		{"STATE_FLAG_BIT_GAME_INITIALIZED",umath::to_integral(Game::GameFlags::GameInitialized)},
		{"STATE_FLAG_BIT_MAP_LOADED",umath::to_integral(Game::GameFlags::MapLoaded)},
		{"STATE_FLAG_BIT_INITIAL_TICK",umath::to_integral(Game::GameFlags::InitialTick)},
		{"STATE_FLAG_BIT_LEVEL_TRANSITION",umath::to_integral(Game::GameFlags::LevelTransition)}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"sound",{
		{"CHANNEL_AUTO",umath::to_integral(ALChannel::Auto)},
		{"CHANNEL_MONO",umath::to_integral(ALChannel::Mono)},
		{"CHANNEL_BOTH",umath::to_integral(ALChannel::Both)}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"intersect",{
		{"RESULT_OUTSIDE",INTERSECT_OUTSIDE},
		{"RESULT_INSIDE",INTERSECT_INSIDE},
		{"RESULT_OVERLAP",INTERSECT_OVERLAP}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"time",{
		{"TIMER_TYPE_CURTIME",umath::to_integral(TimerType::CurTime)},
		{"TIMER_TYPE_REALTIME",umath::to_integral(TimerType::RealTime)},
		
		{"DURATION_TYPE_NANO_SECONDS",umath::to_integral(util::DurationType::NanoSeconds)},
		{"DURATION_TYPE_MICRO_SECONDS",umath::to_integral(util::DurationType::MicroSeconds)},
		{"DURATION_TYPE_MILLI_SECONDS",umath::to_integral(util::DurationType::MilliSeconds)},
		{"DURATION_TYPE_SECONDS",umath::to_integral(util::DurationType::Seconds)},
		{"DURATION_TYPE_MINUTES",umath::to_integral(util::DurationType::Minutes)},
		{"DURATION_TYPE_HOURS",umath::to_integral(util::DurationType::Hours)}
	});

	Lua::RegisterLibraryValue<bool>(GetLuaState(),"os","SYSTEM_WINDOWS",util::is_windows_system());
	Lua::RegisterLibraryValue<bool>(GetLuaState(),"os","SYSTEM_LINUX",util::is_linux_system());
	Lua::RegisterLibraryValue<bool>(GetLuaState(),"os","SYSTEM_X86",util::is_x86_system());
	Lua::RegisterLibraryValue<bool>(GetLuaState(),"os","SYSTEM_X64",util::is_x64_system());

	Lua::RegisterLibraryEnums(GetLuaState(),"input",{
		{"ACTION_MOVEFORWARD",umath::to_integral(Action::MoveForward)},
		{"ACTION_MOVEBACKWARD",umath::to_integral(Action::MoveBackward)},
		{"ACTION_MOVELEFT",umath::to_integral(Action::MoveLeft)},
		{"ACTION_MOVERIGHT",umath::to_integral(Action::MoveRight)},
		{"ACTION_SPRINT",umath::to_integral(Action::Sprint)},
		{"ACTION_WALK",umath::to_integral(Action::Walk)},
		{"ACTION_JUMP",umath::to_integral(Action::Jump)},
		{"ACTION_CROUCH",umath::to_integral(Action::Crouch)},
		{"ACTION_ATTACK",umath::to_integral(Action::Attack)},
		{"ACTION_ATTACK2",umath::to_integral(Action::Attack2)},
		{"ACTION_ATTACK3",umath::to_integral(Action::Attack3)},
		{"ACTION_ATTACK4",umath::to_integral(Action::Attack4)},
		{"ACTION_RELOAD",umath::to_integral(Action::Reload)},
		{"ACTION_USE",umath::to_integral(Action::Use)}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"game",{
		{"HITGROUP_INVALID",umath::to_integral(HitGroup::Invalid)},
		{"HITGROUP_GENERIC",umath::to_integral(HitGroup::Generic)},
		{"HITGROUP_HEAD",umath::to_integral(HitGroup::Head)},
		{"HITGROUP_CHEST",umath::to_integral(HitGroup::Chest)},
		{"HITGROUP_STOMACH",umath::to_integral(HitGroup::Stomach)},
		{"HITGROUP_LEFT_ARM",umath::to_integral(HitGroup::LeftArm)},
		{"HITGROUP_RIGHT_ARM",umath::to_integral(HitGroup::RightArm)},
		{"HITGROUP_LEFT_LEG",umath::to_integral(HitGroup::LeftLeg)},
		{"HITGROUP_RIGHT_LEG",umath::to_integral(HitGroup::RightLeg)},
		{"HITGROUP_GEAR",umath::to_integral(HitGroup::Gear)},
		{"HITGROUP_TAIL",umath::to_integral(HitGroup::Tail)}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"file",{
		{"FLAG_INVALID",FVFILE_INVALID},
		{"FLAG_PACKAGE",FVFILE_PACKAGE},
		{"FLAG_COMPRESSED",FVFILE_COMPRESSED},
		{"FLAG_DIRECTORY",FVFILE_DIRECTORY},
		{"FLAG_ENCRYPTED",FVFILE_ENCRYPTED},
		{"FLAG_VIRTUAL",FVFILE_VIRTUAL},
		{"FLAG_READONLY",FVFILE_READONLY},

		{"OPEN_MODE_READ",umath::to_integral(FileOpenMode::Read)},
		{"OPEN_MODE_WRITE",umath::to_integral(FileOpenMode::Write)},
		{"OPEN_MODE_APPEND",umath::to_integral(FileOpenMode::Append)},
		{"OPEN_MODE_UPDATE",umath::to_integral(FileOpenMode::Update)},
		{"OPEN_MODE_BINARY",umath::to_integral(FileOpenMode::Binary)}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"file",{
		{"TYPE_VIRTUAL",VFILE_VIRTUAL},
		{"TYPE_LOCAL",VFILE_LOCAL},
		{"TYPE_PACKAGE",VFILE_PACKAGE}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"file",{
		{"ATTRIBUTE_ARCHIVE",FILE_ATTRIBUTE_ARCHIVE},
		{"ATTRIBUTE_COMPRESSED",FILE_ATTRIBUTE_COMPRESSED},
		{"ATTRIBUTE_DEVICE",FILE_ATTRIBUTE_DEVICE},
		{"ATTRIBUTE_DIRECTORY",FILE_ATTRIBUTE_DIRECTORY},
		{"ATTRIBUTE_ENCRYPTED",FILE_ATTRIBUTE_ENCRYPTED},
		{"ATTRIBUTE_HIDDEN",FILE_ATTRIBUTE_HIDDEN},
		{"ATTRIBUTE_NORMAL",FILE_ATTRIBUTE_NORMAL},
		{"ATTRIBUTE_NOT_CONTENT_INDEXED",FILE_ATTRIBUTE_NOT_CONTENT_INDEXED},
		{"ATTRIBUTE_OFFLINE",FILE_ATTRIBUTE_OFFLINE},
		{"ATTRIBUTE_READONLY",FILE_ATTRIBUTE_READONLY},
		{"ATTRIBUTE_REPARSE_POINT",FILE_ATTRIBUTE_REPARSE_POINT},
		{"ATTRIBUTE_SPARSE_FILE",FILE_ATTRIBUTE_SPARSE_FILE},
		{"ATTRIBUTE_SYSTEM",FILE_ATTRIBUTE_SYSTEM},
		{"ATTRIBUTE_TEMPORARY",FILE_ATTRIBUTE_TEMPORARY},
		{"ATTRIBUTE_VIRTUAL",FILE_ATTRIBUTE_VIRTUAL},
		{"ATTRIBUTE_INVALID",INVALID_FILE_ATTRIBUTES}
	});

	Lua::RegisterLibraryEnums(GetLuaState(),"file",{
		{"SEARCH_NONE",umath::to_integral(fsys::SearchFlags::None)},
		{"SEARCH_VIRTUAL",umath::to_integral(fsys::SearchFlags::Virtual)},
		{"SEARCH_PACKAGE",umath::to_integral(fsys::SearchFlags::Package)},
		{"SEARCH_LOCAL",umath::to_integral(fsys::SearchFlags::Local)},
		{"SEARCH_NO_MOUNTS",umath::to_integral(fsys::SearchFlags::NoMounts)},
		{"SEARCH_LOCAL_ROOT",umath::to_integral(fsys::SearchFlags::LocalRoot)},
		{"SEARCH_ALL",umath::to_integral(fsys::SearchFlags::All)},
		{"SEARCH_ADDON",FSYS_SEARCH_ADDON}
	});

#ifdef PHYS_ENGINE_PHYSX
	lua_pushinteger(GetLuaState(),physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING);
	lua_setglobal(GetLuaState(),"FPHYSX_NONWALKABLEMODE_PREVENT_CLIMBING");

	lua_pushinteger(GetLuaState(),physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING);
	lua_setglobal(GetLuaState(),"FPHYSX_NONWALKABLEMODE_PREVENT_CLIMBING_AND_FORCE_SLIDING");

	lua_pushinteger(GetLuaState(),physx::PxCapsuleClimbingMode::eCONSTRAINED);
	lua_setglobal(GetLuaState(),"FPHYSX_CAPSULE_CLIMBINGMODE_CONSTRAINED");

	lua_pushinteger(GetLuaState(),physx::PxCapsuleClimbingMode::eEASY);
	lua_setglobal(GetLuaState(),"FPHYSX_CAPSULE_CLIMBINGMODE_EASY");

	lua_pushinteger(GetLuaState(),physx::PxActorFlag::eDISABLE_GRAVITY);
	lua_setglobal(GetLuaState(),"FPHYSX_ACTOR_DISABLE_GRAVITY");

	lua_pushinteger(GetLuaState(),physx::PxActorFlag::eDISABLE_SIMULATION);
	lua_setglobal(GetLuaState(),"FPHYSX_ACTOR_DISABLE_SIMULATION");

	lua_pushinteger(GetLuaState(),physx::PxActorFlag::eSEND_SLEEP_NOTIFIES);
	lua_setglobal(GetLuaState(),"FPHYSX_ACTOR_SEND_SLEEP_NOTIFIES");

	lua_pushinteger(GetLuaState(),physx::PxActorFlag::eVISUALIZATION);
	lua_setglobal(GetLuaState(),"FPHYSX_ACTOR_VISUALIZATION");

	lua_pushinteger(GetLuaState(),physx::PxForceMode::Enum::eACCELERATION);
	lua_setglobal(GetLuaState(),"FPHYSX_FORCEMODE_ACCELERATION");

	lua_pushinteger(GetLuaState(),physx::PxForceMode::Enum::eFORCE);
	lua_setglobal(GetLuaState(),"FPHYSX_FORCEMODE_FORCE");

	lua_pushinteger(GetLuaState(),physx::PxForceMode::Enum::eIMPULSE);
	lua_setglobal(GetLuaState(),"FPHYSX_FORCEMODE_IMPULSE");

	lua_pushinteger(GetLuaState(),physx::PxForceMode::Enum::eVELOCITY_CHANGE);
	lua_setglobal(GetLuaState(),"FPHYSX_FORCEMODE_VELOCITY_CHANGE");

	lua_pushinteger(GetLuaState(),physx::PxActorTypeFlag::eCLOTH);
	lua_setglobal(GetLuaState(),"FPHYSX_ACTOR_TYPE_CLOTH");

	lua_pushinteger(GetLuaState(),physx::PxActorTypeFlag::ePARTICLE_FLUID);
	lua_setglobal(GetLuaState(),"FPHYSX_ACTOR_TYPE_PARTICLE_FLUID");

	lua_pushinteger(GetLuaState(),physx::PxActorTypeFlag::ePARTICLE_SYSTEM);
	lua_setglobal(GetLuaState(),"FPHYSX_ACTOR_TYPE_PARTICLE_SYSTEM");

	lua_pushinteger(GetLuaState(),physx::PxActorTypeFlag::eRIGID_DYNAMIC);
	lua_setglobal(GetLuaState(),"FPHYSX_ACTOR_TYPE_RIGID_DYNAMIC");

	lua_pushinteger(GetLuaState(),physx::PxActorTypeFlag::eRIGID_STATIC);
	lua_setglobal(GetLuaState(),"FPHYSX_ACTOR_TYPE_RIGID_STATIC");

	lua_pushinteger(GetLuaState(),16384);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_ENABLE_STABILIZATION");

	lua_pushinteger(GetLuaState(),32768);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_ENABLE_AVERAGE_POINT");

	lua_pushinteger(GetLuaState(),(1 << 14));
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_ENABLE_STABILIZATION");

	lua_pushinteger(GetLuaState(),(1 << 15));
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_ENABLE_AVERAGE_POINT");

	lua_pushinteger(GetLuaState(),physx::PxSceneFlag::eADAPTIVE_FORCE);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_ADAPTIVE_FORCE");

	lua_pushinteger(GetLuaState(),physx::PxSceneFlag::eDISABLE_CCD_RESWEEP);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_DISABLE_CCD_RESWEEP");

	lua_pushinteger(GetLuaState(),physx::PxSceneFlag::eDISABLE_CONTACT_CACHE);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_DISABLE_CONTACT_CACHE");

	lua_pushinteger(GetLuaState(),physx::PxSceneFlag::eDISABLE_CONTACT_REPORT_BUFFER_RESIZE);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_DISABLE_CONTACT_REPORT_BUFFER_RESIZE");

	lua_pushinteger(GetLuaState(),physx::PxSceneFlag::eENABLE_ACTIVETRANSFORMS);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_ENABLE_ACTIVETRANSFORMS");

	lua_pushinteger(GetLuaState(),physx::PxSceneFlag::eENABLE_CCD);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_ENABLE_CCD");

	lua_pushinteger(GetLuaState(),physx::PxSceneFlag::eENABLE_KINEMATIC_PAIRS);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_ENABLE_KINEMATIC_PAIRS");

	lua_pushinteger(GetLuaState(),physx::PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_ENABLE_KINEMATIC_STATIC_PAIRS");

	lua_pushinteger(GetLuaState(),physx::PxSceneFlag::eENABLE_PCM);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_ENABLE_PCM");

	lua_pushinteger(GetLuaState(),physx::PxSceneFlag::eREQUIRE_RW_LOCK);
	lua_setglobal(GetLuaState(),"FPHYSX_SCENE_REQUIRE_RW_LOCK");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::eASSUME_NO_INITIAL_OVERLAP);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_ASSUME_NO_INITIAL_OVERLAP");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::eDEFAULT);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_DEFAULT");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::eDISTANCE);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_DISTANCE");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::eMESH_ANY);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_MESH_ANY");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::eMESH_BOTH_SIDES);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_MESH_BOTH_SIDES");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::eMESH_MULTIPLE);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_MESH_MULTIPLE");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::eMODIFIABLE_FLAGS);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_MODIFIABLE_FLAGS");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::eNORMAL);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_NORMAL");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::ePOSITION);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_POSITION");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::ePRECISE_SWEEP);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_PRECISE_SWEEP");

	lua_pushinteger(GetLuaState(),physx::PxHitFlag::eUV);
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_UV");

	lua_pushinteger(GetLuaState(),(1 << 9));
	lua_setglobal(GetLuaState(),"FPHYSX_HIT_MTD");

	lua_pushinteger(GetLuaState(),physx::PxQueryFlag::eANY_HIT);
	lua_setglobal(GetLuaState(),"FPHYSX_QUERY_ANY_HIT");
	
	lua_pushinteger(GetLuaState(),physx::PxQueryFlag::eDYNAMIC);
	lua_setglobal(GetLuaState(),"FPHYSX_QUERY_DYNAMIC");
	
	lua_pushinteger(GetLuaState(),physx::PxQueryFlag::eNO_BLOCK);
	lua_setglobal(GetLuaState(),"FPHYSX_QUERY_NO_BLOCK");
	
	lua_pushinteger(GetLuaState(),physx::PxQueryFlag::ePOSTFILTER);
	lua_setglobal(GetLuaState(),"FPHYSX_QUERY_POSTFILTER");

	lua_pushinteger(GetLuaState(),physx::PxQueryFlag::ePREFILTER);
	lua_setglobal(GetLuaState(),"FPHYSX_QUERY_PREFILTER");

	lua_pushinteger(GetLuaState(),physx::PxQueryFlag::eRESERVED);
	lua_setglobal(GetLuaState(),"FPHYSX_QUERY_RESERVED");

	lua_pushinteger(GetLuaState(),physx::PxQueryFlag::eSTATIC);
	lua_setglobal(GetLuaState(),"FPHYSX_QUERY_STATIC");

	lua_pushinteger(GetLuaState(),physx::PxControllerCollisionFlag::eCOLLISION_DOWN);
	lua_setglobal(GetLuaState(),"FPHYSX_CONTROLLER_COLLISION_DOWN");

	lua_pushinteger(GetLuaState(),physx::PxControllerCollisionFlag::eCOLLISION_SIDES);
	lua_setglobal(GetLuaState(),"FPHYSX_CONTROLLER_COLLISION_SIDES");

	lua_pushinteger(GetLuaState(),physx::PxControllerCollisionFlag::eCOLLISION_UP);
	lua_setglobal(GetLuaState(),"FPHYSX_CONTROLLER_COLLISION_UP");
#endif
}