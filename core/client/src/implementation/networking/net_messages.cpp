// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :networking.net_messages;
import :networking.model_load_manager;

#undef DrawText

static void NET_cl_RESOURCEINFO(NetPacket packet);
static void NET_cl_RESOURCECOMPLETE(NetPacket packet);
static void NET_cl_RESOURCE_FRAGMENT(NetPacket packet);
static void NET_cl_RESOURCE_MDL_ROUGH(NetPacket packet);

static void NET_cl_SERVERINFO(NetPacket packet);
static void NET_cl_START_RESOURCE_TRANSFER(NetPacket packet);

static void NET_cl_SND_EV(NetPacket packet);
static void NET_cl_SND_CREATE(NetPacket packet);
static void NET_cl_SND_PRECACHE(NetPacket packet);

static void NET_cl_LUANET(NetPacket packet);
static void NET_cl_LUANET_REG(NetPacket packet);
static void NET_cl_REGISTER_NET_EVENT(NetPacket packet);

static void NET_cl_ENT_CREATE(NetPacket packet);
static void NET_cl_ENT_REMOVE(NetPacket packet);
static void NET_cl_ENT_SOUND(NetPacket packet);
static void NET_cl_ENT_MODEL(NetPacket packet);
static void NET_cl_ENT_SKIN(NetPacket packet);
static void NET_cl_PL_LOCAL(NetPacket packet);
static void NET_cl_GAME_START(NetPacket packet);
static void NET_cl_MAP_READY(NetPacket packet);
static void NET_cl_MAP_LOAD(NetPacket packet);
static void NET_cl_ENT_CREATE_LUA(NetPacket packet);
static void NET_cl_ENT_ANIM_PLAY(NetPacket packet);
static void NET_cl_ENT_MOVETYPE(NetPacket packet);
static void NET_cl_ENT_COLLISIONTYPE(NetPacket packet);
static void NET_cl_ENT_EYEOFFSET(NetPacket packet);
static void NET_cl_ENT_SETUNLIT(NetPacket packet);
static void NET_cl_ENT_SETCASTSHADOWS(NetPacket packet);
static void NET_cl_ENT_SETHEALTH(NetPacket packet);
static void NET_cl_ENT_SETNAME(NetPacket packet);
static void NET_cl_ENT_SETPARENT(NetPacket packet);
static void NET_cl_ENT_SETPARENTMODE(NetPacket packet);
static void NET_cl_ENT_PHYS_INIT(NetPacket packet);
static void NET_cl_ENT_PHYS_DESTROY(NetPacket packet);
static void NET_cl_ENT_EVENT(NetPacket packet);
static void NET_cl_ENT_TOGGLE(NetPacket packet);
static void NET_cl_ENT_SETCOLLISIONFILTER(NetPacket packet);
static void NET_cl_ENT_ANIM_GESTURE_PLAY(NetPacket packet);
static void NET_cl_ENT_ANIM_GESTURE_STOP(NetPacket packet);
static void NET_cl_ENT_SETKINEMATIC(NetPacket packet);
static void NET_cl_GAME_TIMESCALE(NetPacket packet);
static void NET_cl_FIRE_BULLET(NetPacket packet);
static void NET_cl_CREATE_GIBLET(NetPacket packet);
static void NET_cl_REGISTER_ENTITY_COMPONENT(NetPacket packet);
static void NET_cl_PL_TOGGLE_NOCLIP(NetPacket packet);

static void NET_cl_GAMEINFO(NetPacket packet);
static void NET_cl_GAME_READY(NetPacket packet);

static void NET_cl_SNAPSHOT(NetPacket packet);

static void NET_cl_CVAR_SET(NetPacket packet);
static void NET_cl_LUACMD_REG(NetPacket packet);

static void NET_cl_PLAYERINPUT(NetPacket packet);

static void NET_cl_PL_SPEED_WALK(NetPacket packet);
static void NET_cl_PL_SPEED_RUN(NetPacket packet);
static void NET_cl_PL_SPEED_SPRINT(NetPacket packet);
static void NET_cl_PL_SPEED_CROUCH_WALK(NetPacket packet);
static void NET_cl_PL_HEIGHT_STAND(NetPacket packet);
static void NET_cl_PL_HEIGHT_CROUCH(NetPacket packet);
static void NET_cl_PL_EYELEVEL_STAND(NetPacket packet);
static void NET_cl_PL_EYELEVEL_CROUCH(NetPacket packet);
static void NET_cl_PL_SLOPELIMIT(NetPacket packet);
static void NET_cl_PL_STEPOFFSET(NetPacket packet);
static void NET_cl_PL_UPDIRECTION(NetPacket packet);
static void NET_cl_PL_CHANGEDNAME(NetPacket packet);

static void NET_cl_WEP_DEPLOY(NetPacket packet);
static void NET_cl_WEP_HOLSTER(NetPacket packet);
static void NET_cl_WEP_PRIMARYATTACK(NetPacket packet);
static void NET_cl_WEP_SECONDARYATTACK(NetPacket packet);
static void NET_cl_WEP_ATTACK3(NetPacket packet);
static void NET_cl_WEP_ATTACK4(NetPacket packet);
static void NET_cl_WEP_RELOAD(NetPacket packet);
static void NET_cl_WEP_PRIM_CLIP_SIZE(NetPacket packet);
static void NET_cl_WEP_SEC_CLIP_SIZE(NetPacket packet);
static void NET_cl_WEP_PRIM_MAX_CLIP_SIZE(NetPacket packet);
static void NET_cl_WEP_SEC_MAX_CLIP_SIZE(NetPacket packet);
static void NET_cl_WEP_PRIM_AMMO_TYPE(NetPacket packet);
static void NET_cl_WEP_SEC_AMMO_TYPE(NetPacket packet);

static void NET_cl_ENV_LIGHT_SPOT_OUTERCUTOFF_ANGLE(NetPacket packet);
static void NET_cl_ENV_LIGHT_SPOT_INNERCUTOFF_ANGLE(NetPacket packet);

static void NET_cl_ENVLIGHT_SETSTATE(NetPacket packet);
static void NET_cl_ENVEXPLOSION_EXPLODE(NetPacket packet);

static void NET_cl_SV_SEND(NetPacket packet);

static void NET_cl_ENV_FOGCON_SETSTARTDIST(NetPacket packet);
static void NET_cl_ENV_FOGCON_SETENDDIST(NetPacket packet);
static void NET_cl_ENV_FOGCON_SETMAXDENSITY(NetPacket packet);

static void NET_cl_ENV_PRTSYS_SETCONTINUOUS(NetPacket packet);

static void NET_cl_CLIENT_DROPPED(NetPacket packet);
static void NET_cl_CLIENT_READY(NetPacket packet);
static void NET_cl_CLIENT_JOINED(NetPacket packet);

static void NET_cl_CREATE_EXPLOSION(NetPacket packet);
static void NET_cl_DEBUG_AI_NAVIGATION(NetPacket packet);
static void NET_cl_DEBUG_AI_SCHEDULE_PRINT(NetPacket packet);
static void NET_cl_DEBUG_AI_SCHEDULE_TREE(NetPacket packet);
static void NET_cl_CMD_CALL_RESPONSE(NetPacket packet);

static void NET_cl_ENT_TRIGGER_GRAVITY_ONSTARTTOUCH(NetPacket packet);

static void NET_cl_ADD_SHARED_COMPONENT(NetPacket packet);

static void NET_cl_DEBUG_DRAWPOINT(NetPacket packet);
static void NET_cl_DEBUG_DRAWLINE(NetPacket packet);
static void NET_cl_DEBUG_DRAWBOX(NetPacket packet);
static void NET_cl_DEBUG_DRAWTEXT(NetPacket packet);
static void NET_cl_DEBUG_DRAWSPHERE(NetPacket packet);
static void NET_cl_DEBUG_DRAWCONE(NetPacket packet);
static void NET_cl_DEBUG_DRAWAXIS(NetPacket packet);
static void NET_cl_DEBUG_DRAWPATH(NetPacket packet);
static void NET_cl_DEBUG_DRAWSPLINE(NetPacket packet);
static void NET_cl_DEBUG_DRAWPLANE(NetPacket packet);
static void NET_cl_DEBUG_DRAW_MESH(NetPacket packet);
static void NET_cl_DEBUG_DRAWTRUNCATEDCONE(NetPacket packet);
static void NET_cl_DEBUG_DRAWCYLINDER(NetPacket packet);

#define REGISTER_NET_MSG(NAME) netMessageMap.RegisterNetMessage(client::NAME, &NET_cl_##NAME)

static void register_net_messages(pragma::networking::ClientMessageMap &netMessageMap)
{
	using namespace pragma::networking::net_messages;
	REGISTER_NET_MSG(RESOURCEINFO);
	REGISTER_NET_MSG(RESOURCECOMPLETE);
	REGISTER_NET_MSG(RESOURCE_FRAGMENT);
	REGISTER_NET_MSG(RESOURCE_MDL_ROUGH);

	REGISTER_NET_MSG(SERVERINFO);
	REGISTER_NET_MSG(START_RESOURCE_TRANSFER);

	REGISTER_NET_MSG(SND_EV);
	REGISTER_NET_MSG(SND_CREATE);
	REGISTER_NET_MSG(SND_PRECACHE);

	REGISTER_NET_MSG(LUANET);
	REGISTER_NET_MSG(LUANET_REG);
	REGISTER_NET_MSG(REGISTER_NET_EVENT);

	REGISTER_NET_MSG(ENT_CREATE);
	REGISTER_NET_MSG(ENT_REMOVE);
	REGISTER_NET_MSG(ENT_SOUND);
	REGISTER_NET_MSG(ENT_MODEL);
	REGISTER_NET_MSG(ENT_SKIN);
	REGISTER_NET_MSG(PL_LOCAL);
	REGISTER_NET_MSG(GAME_START);
	REGISTER_NET_MSG(MAP_READY);
	REGISTER_NET_MSG(MAP_LOAD);
	REGISTER_NET_MSG(ENT_CREATE_LUA);
	REGISTER_NET_MSG(ENT_ANIM_PLAY);
	REGISTER_NET_MSG(ENT_MOVETYPE);
	REGISTER_NET_MSG(ENT_COLLISIONTYPE);
	REGISTER_NET_MSG(ENT_EYEOFFSET);
	REGISTER_NET_MSG(ENT_SETUNLIT);
	REGISTER_NET_MSG(ENT_SETCASTSHADOWS);
	REGISTER_NET_MSG(ENT_SETHEALTH);
	REGISTER_NET_MSG(ENT_SETNAME);
	REGISTER_NET_MSG(ENT_SETPARENT);
	REGISTER_NET_MSG(ENT_SETPARENTMODE);
	REGISTER_NET_MSG(ENT_PHYS_INIT);
	REGISTER_NET_MSG(ENT_PHYS_DESTROY);
	REGISTER_NET_MSG(ENT_EVENT);
	REGISTER_NET_MSG(ENT_TOGGLE);
	REGISTER_NET_MSG(ENT_SETCOLLISIONFILTER);
	REGISTER_NET_MSG(ENT_ANIM_GESTURE_PLAY);
	REGISTER_NET_MSG(ENT_ANIM_GESTURE_STOP);
	REGISTER_NET_MSG(ENT_SETKINEMATIC);
	REGISTER_NET_MSG(GAME_TIMESCALE);
	REGISTER_NET_MSG(FIRE_BULLET);
	REGISTER_NET_MSG(CREATE_GIBLET);
	REGISTER_NET_MSG(REGISTER_ENTITY_COMPONENT);
	REGISTER_NET_MSG(PL_TOGGLE_NOCLIP);

	REGISTER_NET_MSG(GAMEINFO);
	REGISTER_NET_MSG(GAME_READY);

	REGISTER_NET_MSG(SNAPSHOT);

	REGISTER_NET_MSG(CVAR_SET);
	REGISTER_NET_MSG(LUACMD_REG);

	REGISTER_NET_MSG(PLAYERINPUT);

	REGISTER_NET_MSG(PL_SPEED_WALK);
	REGISTER_NET_MSG(PL_SPEED_RUN);
	REGISTER_NET_MSG(PL_SPEED_SPRINT);
	REGISTER_NET_MSG(PL_SPEED_CROUCH_WALK);
	REGISTER_NET_MSG(PL_HEIGHT_STAND);
	REGISTER_NET_MSG(PL_HEIGHT_CROUCH);
	REGISTER_NET_MSG(PL_EYELEVEL_STAND);
	REGISTER_NET_MSG(PL_EYELEVEL_CROUCH);
	REGISTER_NET_MSG(PL_SLOPELIMIT);
	REGISTER_NET_MSG(PL_STEPOFFSET);
	REGISTER_NET_MSG(PL_UPDIRECTION);
	REGISTER_NET_MSG(PL_CHANGEDNAME);

	REGISTER_NET_MSG(WEP_DEPLOY);
	REGISTER_NET_MSG(WEP_HOLSTER);
	REGISTER_NET_MSG(WEP_PRIMARYATTACK);
	REGISTER_NET_MSG(WEP_SECONDARYATTACK);
	REGISTER_NET_MSG(WEP_ATTACK3);
	REGISTER_NET_MSG(WEP_ATTACK4);
	REGISTER_NET_MSG(WEP_RELOAD);
	REGISTER_NET_MSG(WEP_PRIM_CLIP_SIZE);
	REGISTER_NET_MSG(WEP_SEC_CLIP_SIZE);
	REGISTER_NET_MSG(WEP_PRIM_MAX_CLIP_SIZE);
	REGISTER_NET_MSG(WEP_SEC_MAX_CLIP_SIZE);
	REGISTER_NET_MSG(WEP_PRIM_AMMO_TYPE);
	REGISTER_NET_MSG(WEP_SEC_AMMO_TYPE);

	REGISTER_NET_MSG(ENV_LIGHT_SPOT_OUTERCUTOFF_ANGLE);
	REGISTER_NET_MSG(ENV_LIGHT_SPOT_INNERCUTOFF_ANGLE);

	REGISTER_NET_MSG(ENVLIGHT_SETSTATE);
	REGISTER_NET_MSG(ENVEXPLOSION_EXPLODE);

	REGISTER_NET_MSG(SV_SEND);

	REGISTER_NET_MSG(ENV_FOGCON_SETSTARTDIST);
	REGISTER_NET_MSG(ENV_FOGCON_SETENDDIST);
	REGISTER_NET_MSG(ENV_FOGCON_SETMAXDENSITY);

	REGISTER_NET_MSG(ENV_PRTSYS_SETCONTINUOUS);

	REGISTER_NET_MSG(CLIENT_DROPPED);
	REGISTER_NET_MSG(CLIENT_READY);
	REGISTER_NET_MSG(CLIENT_JOINED);

	REGISTER_NET_MSG(CREATE_EXPLOSION);
	REGISTER_NET_MSG(DEBUG_AI_NAVIGATION);
	REGISTER_NET_MSG(DEBUG_AI_SCHEDULE_PRINT);
	REGISTER_NET_MSG(DEBUG_AI_SCHEDULE_TREE);
	REGISTER_NET_MSG(CMD_CALL_RESPONSE);

	REGISTER_NET_MSG(ENT_TRIGGER_GRAVITY_ONSTARTTOUCH);

	REGISTER_NET_MSG(ADD_SHARED_COMPONENT);

	REGISTER_NET_MSG(DEBUG_DRAWPOINT);
	REGISTER_NET_MSG(DEBUG_DRAWLINE);
	REGISTER_NET_MSG(DEBUG_DRAWBOX);
	REGISTER_NET_MSG(DEBUG_DRAWTEXT);
	REGISTER_NET_MSG(DEBUG_DRAWSPHERE);
	REGISTER_NET_MSG(DEBUG_DRAWCONE);
	REGISTER_NET_MSG(DEBUG_DRAWAXIS);
	REGISTER_NET_MSG(DEBUG_DRAWPATH);
	REGISTER_NET_MSG(DEBUG_DRAWSPLINE);
	REGISTER_NET_MSG(DEBUG_DRAWPLANE);
	REGISTER_NET_MSG(DEBUG_DRAW_MESH);
	REGISTER_NET_MSG(DEBUG_DRAWTRUNCATEDCONE);
	REGISTER_NET_MSG(DEBUG_DRAWCYLINDER);
}
void pragma::networking::register_client_net_messages()
{
	static auto netMessagesRegistered = false;
	if(netMessagesRegistered)
		return;
	netMessagesRegistered = true;

	::register_net_messages(*get_client_message_map());
}

void NET_cl_RESOURCEINFO(NetPacket packet) { pragma::get_client_state()->HandleClientResource(packet); }
void NET_cl_RESOURCECOMPLETE(NetPacket packet)
{
	Con::CCL << "All resources have been received!" << Con::endl;

	auto *client = pragma::get_client_state();
	auto *cl = client->GetClient();
	if(cl != nullptr)
		cl->SetTimeoutDuration(client->GetConVarFloat("sv_timeout_duration")); // Resource transfer complete; Reset timeout

	Con::CCL << "Requesting Game Info..." << Con::endl;
	client->SendUserInfo();
}

void NET_cl_RESOURCE_FRAGMENT(NetPacket packet) { pragma::get_client_state()->HandleClientResourceFragment(packet); }

void NET_cl_RESOURCE_MDL_ROUGH(NetPacket packet)
{
	if(pragma::get_client_state() == nullptr)
		return;
	auto fileName = packet->ReadString();
	auto *client = pragma::get_client_state();
	auto mdl = client->GetModelManager().CreateModel(fileName);
	if(mdl == nullptr)
		return;
	mdl->AddMaterial(0, client->LoadMaterial("loading"));

	auto &manager = ModelLoadManager::Initialize();
#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::CCL << "[ResourceManager] Adding query: " << fileName << Con::endl;
#endif
	manager.AddQuery(packet, mdl, fileName);
}

void NET_cl_SERVERINFO(NetPacket packet) { pragma::get_client_state()->HandleClientReceiveServerInfo(packet); }
void NET_cl_START_RESOURCE_TRANSFER(NetPacket packet) { pragma::get_client_state()->HandleClientStartResourceTransfer(packet); }

void NET_cl_LUANET(NetPacket packet) { pragma::get_client_state()->HandleLuaNetPacket(packet); }

void NET_cl_LUANET_REG(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	std::string name = packet->ReadString();
	pragma::Game *game = client->GetGameState();
	game->RegisterNetMessage(name);
}

void NET_cl_REGISTER_NET_EVENT(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	static_cast<pragma::CGame *>(client->GetGameState())->OnReceivedRegisterNetEvent(packet);
}

void NET_cl_SND_PRECACHE(NetPacket packet)
{
	std::string snd = packet->ReadString();
	auto mode = packet->Read<uint8_t>();
	pragma::get_client_state()->PrecacheSound(snd, static_cast<pragma::audio::ALChannel>(mode));
}

void NET_cl_SND_CREATE(NetPacket packet)
{
	std::string snd = packet->ReadString();
	auto type = packet->Read<pragma::audio::ALSoundType>();
	unsigned int idx = packet->Read<unsigned int>();
	auto createFlags = packet->Read<pragma::audio::ALCreateFlags>();
	auto *client = pragma::get_client_state();
	auto as = client->CreateSound(snd, pragma::audio::ALSoundType::Generic, createFlags);
	if(as == nullptr)
		return;
	client->IndexSound(as, idx);

	auto fullUpdate = packet->Read<bool>();
	if(fullUpdate == false)
		return;
	auto state = packet->Read<pragma::audio::ALState>();

	as->SetOffset(packet->Read<float>());
	as->SetPitch(packet->Read<float>());
	as->SetLooping(packet->Read<bool>());
	as->SetGain(packet->Read<float>());
	as->SetPosition(packet->Read<Vector3>());
	as->SetVelocity(packet->Read<Vector3>());
	as->SetDirection(packet->Read<Vector3>());
	as->SetRelative(packet->Read<bool>());
	as->SetReferenceDistance(packet->Read<float>());
	as->SetRolloffFactor(packet->Read<float>());
	as->SetRoomRolloffFactor(packet->Read<float>());
	as->SetMaxDistance(packet->Read<float>());
	as->SetMinGain(packet->Read<float>());
	as->SetMaxGain(packet->Read<float>());
	as->SetInnerConeAngle(packet->Read<float>());
	as->SetOuterConeAngle(packet->Read<float>());
	as->SetOuterConeGain(packet->Read<float>());
	as->SetOuterConeGainHF(packet->Read<float>());
	as->SetFlags(packet->Read<uint32_t>());

	auto hasRange = packet->Read<bool>();
	if(hasRange) {
		auto start = packet->Read<float>();
		auto end = packet->Read<float>();
		as->SetRange(start, end);
	}

	as->SetFadeInDuration(packet->Read<float>());
	as->SetFadeOutDuration(packet->Read<float>());
	as->SetPriority(packet->Read<uint32_t>());

	auto at = packet->Read<Vector3>();
	auto up = packet->Read<Vector3>();
	as->SetOrientation(at, up);

	as->SetDopplerFactor(packet->Read<float>());
	as->SetLeftStereoAngle(packet->Read<float>());
	as->SetRightStereoAngle(packet->Read<float>());

	as->SetAirAbsorptionFactor(packet->Read<float>());

	auto directHF = packet->Read<bool>();
	auto send = packet->Read<bool>();
	auto sendHF = packet->Read<bool>();
	as->SetGainAuto(directHF, send, sendHF);

	auto gain = packet->Read<float>();
	auto gainHF = packet->Read<float>();
	auto gainLF = packet->Read<float>();
	as->SetDirectFilter({gain, gainHF, gainLF});

	std::weak_ptr<pragma::audio::ALSound> wpSnd = as;
	pragma::networking::read_unique_entity(packet, [wpSnd](pragma::ecs::BaseEntity *ent) {
		if(ent == nullptr || wpSnd.expired())
			return;
		wpSnd.lock()->SetSource(ent);
	});

	switch(state) {
	case pragma::audio::ALState::Paused:
		as->Pause();
		break;
	case pragma::audio::ALState::Playing:
		as->Play();
		break;
	case pragma::audio::ALState::Stopped:
		as->Stop();
		break;
	case pragma::audio::ALState::Initial:
		break;
	}
}

void NET_cl_SND_EV(NetPacket packet)
{
	unsigned char ev = packet->Read<unsigned char>();
	unsigned int idx = packet->Read<unsigned int>();
	std::shared_ptr<pragma::audio::ALSound> as = pragma::get_client_state()->GetSoundByIndex(idx);
	if(as == nullptr)
		return;
	switch(static_cast<pragma::audio::ALSound::NetEvent>(ev)) {
	case pragma::audio::ALSound::NetEvent::Play:
		as->Play();
		break;
	case pragma::audio::ALSound::NetEvent::Stop:
		as->Stop();
		break;
	case pragma::audio::ALSound::NetEvent::Pause:
		as->Pause();
		break;
	case pragma::audio::ALSound::NetEvent::Rewind:
		as->Rewind();
		break;
	case pragma::audio::ALSound::NetEvent::SetOffset:
		{
			float offset = packet->Read<float>();
			as->SetOffset(offset);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetPitch:
		{
			float pitch = packet->Read<float>();
			as->SetPitch(pitch);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetLooping:
		{
			bool loop = packet->Read<bool>();
			as->SetLooping(loop);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetGain:
		{
			float gain = packet->Read<float>();
			as->SetGain(gain);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetPos:
		{
			Vector3 pos = pragma::networking::read_vector(packet);
			as->SetPosition(pos);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetVelocity:
		{
			Vector3 vel = pragma::networking::read_vector(packet);
			as->SetVelocity(vel);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetDirection:
		{
			Vector3 dir = pragma::networking::read_vector(packet);
			as->SetDirection(dir);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetRelative:
		{
			bool relative = packet->Read<bool>();
			as->SetRelative(relative);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetReferenceDistance:
		{
			float distRef = packet->Read<float>();
			as->SetReferenceDistance(distRef);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetRolloffFactor:
		{
			float rolloff = packet->Read<float>();
			as->SetRolloffFactor(rolloff);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetRoomRolloffFactor:
		{
			auto roomRolloff = packet->Read<float>();
			as->SetRoomRolloffFactor(roomRolloff);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetMaxDistance:
		{
			float dist = packet->Read<float>();
			as->SetMaxDistance(dist);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetMinGain:
		{
			float gain = packet->Read<float>();
			as->SetMinGain(gain);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetMaxGain:
		{
			float gain = packet->Read<float>();
			as->SetMaxGain(gain);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetConeInnerAngle:
		{
			float coneInnerAngle = packet->Read<float>();
			as->SetInnerConeAngle(coneInnerAngle);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetConeOuterAngle:
		{
			float coneOuterAngle = packet->Read<float>();
			as->SetOuterConeAngle(coneOuterAngle);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetConeOuterGain:
		{
			float coneOuterGain = packet->Read<float>();
			as->SetOuterConeGain(coneOuterGain);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetConeOuterGainHF:
		{
			float coneOuterGainHF = packet->Read<float>();
			as->SetOuterConeGainHF(coneOuterGainHF);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetFlags:
		{
			unsigned int flags = packet->Read<unsigned int>();
			as->SetFlags(flags);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetType:
		{
			auto type = packet->Read<pragma::audio::ALSoundType>();
			as->SetType(type);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetSource:
		{
			auto *ent = pragma::networking::read_entity(packet);
			as->SetSource(ent);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetRange:
		{
			auto start = packet->Read<float>();
			auto end = packet->Read<float>();
			as->SetRange(start, end);
			break;
		}
	case pragma::audio::ALSound::NetEvent::ClearRange:
		{
			as->ClearRange();
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetFadeInDuration:
		{
			auto t = packet->Read<float>();
			as->SetFadeInDuration(t);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetFadeOutDuration:
		{
			auto t = packet->Read<float>();
			as->SetFadeOutDuration(t);
			break;
		}
	case pragma::audio::ALSound::NetEvent::FadeIn:
		{
			auto t = packet->Read<float>();
			as->FadeIn(t);
			break;
		}
	case pragma::audio::ALSound::NetEvent::FadeOut:
		{
			auto t = packet->Read<float>();
			as->FadeOut(t);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetIndex:
		{
			auto idx = packet->Read<uint32_t>();
			pragma::audio::CALSound::SetIndex(as.get(), idx);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetPriority:
		{
			auto priority = packet->Read<uint32_t>();
			as->SetPriority(priority);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetOrientation:
		{
			auto at = packet->Read<Vector3>();
			auto up = packet->Read<Vector3>();
			as->SetOrientation(at, up);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetDopplerFactor:
		{
			auto factor = packet->Read<float>();
			as->SetDopplerFactor(factor);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetLeftStereoAngle:
		{
			auto ang = packet->Read<float>();
			as->SetLeftStereoAngle(ang);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetRightStereoAngle:
		{
			auto ang = packet->Read<float>();
			as->SetRightStereoAngle(ang);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetAirAbsorptionFactor:
		{
			auto factor = packet->Read<float>();
			as->SetAirAbsorptionFactor(factor);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetGainAuto:
		{
			auto directHF = packet->Read<float>();
			auto send = packet->Read<float>();
			auto sendHF = packet->Read<float>();
			as->SetGainAuto(directHF, send, sendHF);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetDirectFilter:
		{
			auto gain = packet->Read<float>();
			auto gainHF = packet->Read<float>();
			auto gainLF = packet->Read<float>();
			as->SetDirectFilter({gain, gainHF, gainLF});
			break;
		}
	case pragma::audio::ALSound::NetEvent::AddEffect:
		{
			auto effectName = packet->ReadString();
			auto gain = packet->Read<float>();
			auto gainHF = packet->Read<float>();
			auto gainLF = packet->Read<float>();
			as->AddEffect(effectName, {gain, gainHF, gainLF});
			break;
		}
	case pragma::audio::ALSound::NetEvent::RemoveEffect:
		{
			auto effectName = packet->ReadString();
			as->RemoveEffect(effectName);
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetEffectParameters:
		{
			auto effectName = packet->ReadString();
			auto gain = packet->Read<float>();
			auto gainHF = packet->Read<float>();
			auto gainLF = packet->Read<float>();
			as->SetEffectParameters(effectName, {gain, gainHF, gainLF});
			break;
		}
	case pragma::audio::ALSound::NetEvent::SetEntityMapIndex:
		{
			auto idx = packet->Read<uint32_t>();
			//as->SetIdentifier("world_sound" +std::to_string(idx)); // Has to correspond to identifier in c_game_audio.cpp
			break;
		}
	default:
		{
			spdlog::warn("Unhandled sound net event {}!", ev);
			break;
		}
	}
}

pragma::ecs::CBaseEntity *NET_cl_ENT_CREATE(NetPacket &packet, bool bSpawn, bool bIgnoreMapInit = false)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return nullptr;
	pragma::CGame *game = client->GetGameState();
	unsigned int factoryID = packet->Read<unsigned int>();
	auto *factory = client_entities::ClientEntityRegistry::Instance().GetNetworkedFactory(factoryID);
	if(factory == nullptr) {
		Con::CWAR << "Unable to create entity with factory ID '" << factoryID << "': Factory not found!" << Con::endl;
		return nullptr;
	}
	unsigned int idx = packet->Read<unsigned int>();
	unsigned int mapIdx = packet->Read<unsigned int>();
	pragma::ecs::CBaseEntity *ent = (*factory)(client, idx);
	ent->ReceiveData(packet);
	if(mapIdx == 0) {
		if(bSpawn)
			ent->Spawn();
	}
	else if(bIgnoreMapInit == false && game->IsMapInitialized()) {
		Con::CWAR << "Map-entity created after map initialization. Removing..." << Con::endl;
		ent->RemoveSafely();
		return nullptr;
	}
	else {
		auto pMapComponent = ent->AddComponent<pragma::MapComponent>();
		if(pMapComponent.valid())
			pMapComponent->SetMapIndex(mapIdx);
	}
	return ent;
}

void NET_cl_ENT_CREATE(NetPacket packet) { NET_cl_ENT_CREATE(packet, true); }

void NET_cl_ENT_REMOVE(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	ent->Remove();
}

pragma::ecs::CBaseEntity *NET_cl_ENT_CREATE_LUA(NetPacket &packet, bool bSpawn, bool bIgnoreMapInit = false)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return nullptr;

	pragma::CGame *game = client->GetGameState();
	std::string classname = packet->ReadString();
	unsigned int idx = packet->Read<unsigned int>();
	unsigned int mapIdx = packet->Read<unsigned int>();
	pragma::ecs::CBaseEntity *ent = game->CreateLuaEntity(classname, idx, true);
	if(ent == nullptr) {
		Con::CWAR << "Attempted to create unregistered entity '" << classname << "'!" << Con::endl;
		return nullptr;
	}
	ent->ReceiveData(packet);

	if(mapIdx == 0) {
		if(bSpawn)
			ent->Spawn();
	}
	else if(bIgnoreMapInit == false && game->IsMapInitialized()) {
		Con::CWAR << "Map-entity created after map initialization. Removing..." << Con::endl;
		ent->RemoveSafely();
		return nullptr;
	}
	else {
		auto pMapComponent = ent->AddComponent<pragma::MapComponent>();
		if(pMapComponent.valid())
			pMapComponent->SetMapIndex(mapIdx);
	}
	return ent;
}

void NET_cl_ENT_CREATE_LUA(NetPacket packet) { NET_cl_ENT_CREATE_LUA(packet, true); }

void NET_game_timescale(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	float timeScale = packet->Read<float>();
	pragma::CGame *game = client->GetGameState();
	game->SetTimeScale(timeScale);
}

void NET_cl_CREATE_GIBLET(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto info = packet->Read<GibletCreateInfo>();
	pragma::get_cgame()->CreateGiblet(info);
}

void NET_cl_REGISTER_ENTITY_COMPONENT(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto svComponentId = packet->Read<pragma::ComponentId>();
	auto name = packet->ReadString();
	auto &componentManager = static_cast<pragma::CEntityComponentManager &>(pragma::get_cgame()->GetEntityComponentManager());
	auto &svComponentToClComponent = componentManager.GetServerComponentIdToClientComponentIdTable();
	auto clComponentId = componentManager.PreRegisterComponentType(name);
	if(svComponentId >= svComponentToClComponent.size())
		svComponentToClComponent.resize(svComponentId + 1u, pragma::CEntityComponentManager::INVALID_COMPONENT);
	svComponentToClComponent.at(svComponentId) = clComponentId;
}

void NET_cl_FIRE_BULLET(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	auto numTracer = packet->Read<uint8_t>();
	auto tracerSettings = packet->Read<uint8_t>();
	auto trRadius = (tracerSettings & 1) ? packet->Read<float>() : 1.f;
	Color trColor {255, 255, 90, 255};
	if(tracerSettings & 2) {
		trColor.r = packet->Read<uint8_t>();
		trColor.g = packet->Read<uint8_t>();
		trColor.b = packet->Read<uint8_t>();
		trColor.a = packet->Read<uint8_t>();
	}
	auto trLength = (tracerSettings & 4) ? packet->Read<float>() : 200.f;
	auto trSpeed = (tracerSettings & 8) ? packet->Read<float>() : 6'000.f;
	auto trMat = (tracerSettings & 16) ? packet->ReadString() : "particles/beam_tracer";
	auto trBloom = (tracerSettings & 32) ? packet->Read<float>() : 0.25f;

	auto start = packet->Read<Vector3>();
	auto numHits = packet->Read<uint8_t>();
	for(auto i = decltype(numHits) {0}; i < numHits; ++i) {
		auto p = packet->Read<Vector3>();
		auto n = packet->Read<Vector3>();
		auto surfaceMaterialId = packet->Read<int32_t>();
		if(numTracer > 0 && (i % numTracer) == 0)
			pragma::get_cgame()->CreateParticleTracer<pragma::ecs::CParticleSystemComponent>(start, p, trRadius, trColor, trLength, trSpeed, trMat, trBloom);

		auto *surfaceMaterial = (surfaceMaterialId != -1) ? pragma::get_cgame()->GetSurfaceMaterial(surfaceMaterialId) : nullptr;
		auto *surfaceMaterialGeneric = pragma::get_cgame()->GetSurfaceMaterial(0);
		if(surfaceMaterial != nullptr || surfaceMaterialGeneric != nullptr) {
			auto particleEffect = (surfaceMaterial != nullptr) ? surfaceMaterial->GetImpactParticleEffect() : "";
			if(particleEffect.empty() && surfaceMaterialGeneric != nullptr)
				particleEffect = surfaceMaterialGeneric->GetImpactParticleEffect();
			if(!particleEffect.empty()) {
				auto *pt = pragma::ecs::CParticleSystemComponent::Create(particleEffect);
				if(pt != nullptr) {
					auto pTrComponent = pt->GetEntity().GetTransformComponent();
					if(pTrComponent != nullptr) {
						pTrComponent->SetPosition(p);

						auto ang = uvec::to_angle(n);
						auto rot = uquat::create(ang);
						pTrComponent->SetRotation(rot);
					}
					pt->SetRemoveOnComplete(true);
					pt->Start();
				}
			}

			auto sndEffect = (surfaceMaterial != nullptr) ? surfaceMaterial->GetBulletImpactSound() : "";
			if(sndEffect.empty() && surfaceMaterialGeneric != nullptr)
				sndEffect = surfaceMaterialGeneric->GetBulletImpactSound();
			if(!sndEffect.empty()) {
				auto snd = client->CreateSound(sndEffect, pragma::audio::ALSoundType::Effect | pragma::audio::ALSoundType::Physics, pragma::audio::ALCreateFlags::Mono);
				if(snd != nullptr) {
					snd->SetPosition(p);
					snd->Play();
				}
			}
		}
	}
}

void NET_cl_ENT_SOUND(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	unsigned int sndID = packet->Read<unsigned int>();
	std::shared_ptr<pragma::audio::ALSound> snd = client->GetSoundByIndex(sndID);
	if(snd == nullptr)
		return;
	pragma::ecs::CBaseEntity *cent = static_cast<pragma::ecs::CBaseEntity *>(ent);
	auto pSoundEmitterComponent = cent->GetComponent<pragma::CSoundEmitterComponent>();
	if(pSoundEmitterComponent.valid())
		pSoundEmitterComponent->AddSound(snd);
}

void NET_cl_ENT_SETUNLIT(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto pRenderComponent = ent->GetRenderComponent();
	if(!pRenderComponent)
		return;
	bool b = packet->Read<bool>();
	pRenderComponent->SetUnlit(b);
}

void NET_cl_ENT_SETCASTSHADOWS(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto pRenderComponent = ent->GetRenderComponent();
	if(!pRenderComponent)
		return;
	bool b = packet->Read<bool>();
	pRenderComponent->SetCastShadows(b);
}

void NET_cl_ENT_SETHEALTH(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto pHealthComponent = ent->GetComponent<pragma::CHealthComponent>();
	if(pHealthComponent.expired())
		return;
	unsigned short health = packet->Read<unsigned short>();
	pHealthComponent->SetHealth(health);
}

void NET_cl_ENT_SETNAME(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto pNameComponent = ent->GetComponent<pragma::CNameComponent>();
	if(pNameComponent.expired())
		return;
	std::string name = packet->ReadString();
	pNameComponent->SetName(name);
}

void NET_cl_ENT_MODEL(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	std::string mdl = packet->ReadString();
	pragma::ecs::CBaseEntity *cent = static_cast<pragma::ecs::CBaseEntity *>(ent);
	auto mdlComponent = cent->GetModelComponent();
	if(mdlComponent)
		mdlComponent->SetModel(mdl.c_str());
}

void NET_cl_ENT_SKIN(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	unsigned int skin = packet->Read<unsigned int>();
	auto mdlComponent = ent->GetModelComponent();
	if(mdlComponent)
		mdlComponent->SetSkin(skin);
}

void NET_cl_ENT_ANIM_PLAY(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	int anim = packet->Read<int>();
	auto pAnimComponent = ent->GetAnimatedComponent();
	if(pAnimComponent.valid())
		pAnimComponent->PlayAnimation(anim);
}

void NET_cl_ENT_ANIM_GESTURE_PLAY(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	int slot = packet->Read<int>();
	int animation = packet->Read<int>();
	auto pAnimComponent = ent->GetAnimatedComponent();
	if(pAnimComponent.valid())
		pAnimComponent->PlayLayeredAnimation(slot, animation);
}

void NET_cl_ENT_ANIM_GESTURE_STOP(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	int slot = packet->Read<int>();
	auto pAnimComponent = ent->GetAnimatedComponent();
	if(pAnimComponent.valid())
		pAnimComponent->StopLayeredAnimation(slot);
}

void NET_cl_ENT_SETPARENT(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	pragma::ecs::BaseEntity *parent = pragma::networking::read_entity(packet);
	if(parent == nullptr)
		return;
	auto flags = packet->Read<pragma::FAttachmentMode>();
	auto offset = packet->Read<Vector3>();
	auto rot = packet->Read<Quat>();
	auto pAttComponent = ent->GetComponent<pragma::CAttachmentComponent>();
	if(pAttComponent.valid()) {
		AttachmentInfo attInfo {};
		attInfo.flags |= flags;
		attInfo.offset = offset;
		attInfo.rotation = rot;
		pAttComponent->AttachToEntity(parent, attInfo);
	}
}

void NET_cl_ENT_SETPARENTMODE(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto flags = packet->Read<pragma::FAttachmentMode>();
	auto pAttComponent = ent->GetComponent<pragma::CAttachmentComponent>();
	if(pAttComponent.valid())
		pAttComponent->SetAttachmentFlags(flags);
}

void NET_cl_ENT_PHYS_INIT(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	unsigned int type = packet->Read<unsigned int>();
	pPhysComponent->InitializePhysics(pragma::physics::PhysicsType(type));
}

void NET_cl_ENT_PHYS_DESTROY(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	pPhysComponent->DestroyPhysicsObject();
}

void NET_cl_ENT_EVENT(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	packet->SetOffset(packet->GetDataSize() - sizeof(UInt32) - sizeof(unsigned int));
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto eventId = packet->Read<UInt32>();
	auto localId = pragma::get_cgame()->SharedNetEventIdToLocal(eventId);
	if(localId == std::numeric_limits<pragma::NetEventId>::max()) {
		Con::CWAR << "Unknown net event with shared id " << eventId << "!" << Con::endl;
		return;
	}
	packet->SetOffset(0);
	ent->ReceiveNetEvent(localId, packet);
}

void NET_cl_ENT_MOVETYPE(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	pragma::physics::MoveType movetype = pragma::physics::MoveType(packet->Read<unsigned char>());
	pPhysComponent->SetMoveType(movetype);
}

void NET_cl_PL_TOGGLE_NOCLIP(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto bNoclip = packet->Read<bool>();
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	if(bNoclip == false) {
		pPhysComponent->SetMoveType(pragma::physics::MoveType::Walk);
		pPhysComponent->SetCollisionFilterGroup(pragma::physics::CollisionMask::Player);
	}
	else {
		pPhysComponent->SetMoveType(pragma::physics::MoveType::Noclip);
		pPhysComponent->SetCollisionFilterGroup(pragma::physics::CollisionMask::NoCollision);
		//pl->SetCollisionsEnabled(false); // Bugged due to CCD
	}
}

void NET_cl_ENT_COLLISIONTYPE(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	pragma::physics::CollisionType collisiontype = pragma::physics::CollisionType(packet->Read<unsigned char>());
	pPhysComponent->SetCollisionType(collisiontype);
}

void NET_cl_ENT_EYEOFFSET(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *ent = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(ent == nullptr)
		return;
	auto pTrComponent = ent->GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	Vector3 offset = pragma::networking::read_vector(packet);
	pTrComponent->SetEyeOffset(offset);
}

void NET_cl_GAME_START(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	float timeScale = packet->Read<float>();
	auto gameMode = packet->ReadString();
	client->StartNewGame(gameMode);
	if(!client->IsGameActive())
		return;
	pragma::CGame *game = client->GetGameState();
	game->SetTimeScale(timeScale);
}

void NET_cl_GAME_TIMESCALE(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	float timeScale = packet->Read<float>();
	pragma::CGame *game = client->GetGameState();
	game->SetTimeScale(timeScale);
}

// Map has been changed by server
void NET_cl_MAP_READY(NetPacket packet) { pragma::get_client_state()->RequestServerInfo(); }

void NET_cl_MAP_LOAD(NetPacket packet)
{
	if(pragma::get_cgame() == nullptr)
		return;
	auto mapName = packet->ReadString();
	auto origin = packet->Read<Vector3>();
	//auto startIdx = packet->Read<uint32_t>();
	//pragma::get_cgame()->SetEntityMapIndexStart(startIdx);
	pragma::get_client_state()->ReadEntityData(packet);
	auto bNewWorld = packet->Read<bool>();
	auto r = false;
	if(bNewWorld == false)
		r = pragma::get_cgame()->Game::LoadMap(mapName.c_str(), origin);
	else
		r = pragma::get_cgame()->LoadMap(mapName.c_str(), origin);
	if(r == false)
		Con::CWAR << "Unable to load map '" << mapName << "'! Ignoring..." << Con::endl;
}

void NET_cl_PL_LOCAL(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	auto *pl = static_cast<pragma::CPlayerComponent *>(pragma::networking::read_player(packet));
	if(pl == nullptr)
		return;
	pragma::CGame *game = client->GetGameState();
	game->SetLocalPlayer(pl);
}

void NET_cl_GAME_READY(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	client->SetGameReady();
}

void NET_cl_SNAPSHOT(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	pragma::CGame *game = client->GetGameState();
	game->ReceiveSnapshot(packet);
}

void NET_cl_CVAR_SET(NetPacket packet)
{
	std::string cvar = packet->ReadString();
	std::string val = packet->ReadString();
	pragma::get_cengine()->SetReplicatedConVar(cvar, val);
}

void NET_cl_LUACMD_REG(NetPacket packet)
{
	std::string scmd = packet->ReadString();
	unsigned int id = packet->Read<unsigned int>();
	pragma::get_client_state()->RegisterServerConVar(scmd, id);
}

void NET_cl_PLAYERINPUT(NetPacket packet)
{
	if(packet->GetDataSize() == sizeof(uint8_t)) {
		auto id = packet->Read<uint8_t>();
		pragma::get_cgame()->OnReceivedPlayerInputResponse(id);
		return;
	}
	auto *pl = pragma::networking::read_player(packet);
	auto orientation = pragma::networking::read_quat(packet);
	if(pl != nullptr) {
		auto &ent = pl->GetEntity();
		if(ent.IsCharacter())
			ent.GetCharacterComponent()->SetViewOrientation(orientation);
	}
	auto actions = packet->Read<pragma::Action>();
	auto bController = packet->Read<bool>();
	auto *actionInputC = pl ? pl->GetActionInputController() : nullptr;
	if(bController == true) {
		auto actionValues = pragma::math::get_power_of_2_values(pragma::math::to_integral(actions));
		for(auto v : actionValues) {
			auto magnitude = packet->Read<float>();
			if(pl != nullptr)
				actionInputC->SetActionInputAxisMagnitude(static_cast<pragma::Action>(v), magnitude);
		}
	}
	if(pl != nullptr)
		actionInputC->SetActionInputs(actions, true);
}

void NET_cl_PL_SPEED_WALK(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsPlayer() == false)
		return;
	float speed = packet->Read<float>();
	pl->GetPlayerComponent()->SetWalkSpeed(speed);
}

void NET_cl_PL_SLOPELIMIT(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsCharacter() == false)
		return;
	float limit = packet->Read<float>();
	pl->GetCharacterComponent()->SetSlopeLimit(limit);
}

void NET_cl_PL_STEPOFFSET(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsCharacter() == false)
		return;
	float offset = packet->Read<float>();
	pl->GetCharacterComponent()->SetStepOffset(offset);
}

void NET_cl_PL_SPEED_RUN(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsPlayer() == false)
		return;
	float speed = packet->Read<float>();
	pl->GetPlayerComponent()->SetRunSpeed(speed);
}

void NET_cl_PL_SPEED_SPRINT(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsPlayer() == false)
		return;
	float speed = packet->Read<float>();
	pl->GetPlayerComponent()->SetSprintSpeed(speed);
}

void NET_cl_PL_SPEED_CROUCH_WALK(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsPlayer() == false)
		return;
	float speed = packet->Read<float>();
	pl->GetPlayerComponent()->SetCrouchedWalkSpeed(speed);
}

void NET_cl_PL_HEIGHT_STAND(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsPlayer() == false)
		return;
	float height = packet->Read<float>();
	pl->GetPlayerComponent()->SetStandHeight(height);
}

void NET_cl_PL_HEIGHT_CROUCH(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsPlayer() == false)
		return;
	float height = packet->Read<float>();
	pl->GetPlayerComponent()->SetCrouchHeight(height);
}

void NET_cl_PL_EYELEVEL_STAND(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsPlayer() == false)
		return;
	float eyelevel = packet->Read<float>();
	pl->GetPlayerComponent()->SetStandEyeLevel(eyelevel);
}

void NET_cl_PL_EYELEVEL_CROUCH(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsPlayer() == false)
		return;
	float eyelevel = packet->Read<float>();
	pl->GetPlayerComponent()->SetCrouchEyeLevel(eyelevel);
}

void NET_cl_PL_UPDIRECTION(NetPacket packet)
{
	auto *pl = pragma::networking::read_entity(packet);
	if(pl == nullptr || pl->IsCharacter() == false)
		return;
	Vector3 direction = pragma::networking::read_vector(packet);
	auto charC = pl->GetCharacterComponent();
	if(charC.valid()) {
		auto *orientC = charC->GetOrientationComponent();
		if(orientC)
			orientC->SetUpDirection(direction);
	}
}

void NET_cl_PL_CHANGEDNAME(NetPacket packet)
{
	auto *pl = pragma::networking::read_player(packet);
	if(pl == nullptr)
		return;
	auto name = packet->ReadString();
	auto nameC = pl->GetEntity().GetNameComponent();
	if(nameC.valid())
		nameC->SetName(name);
}

void NET_cl_WEP_DEPLOY(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto wepComponent = wep->GetWeaponComponent();
	auto whOwnerComponent = wep->GetComponent<pragma::COwnableComponent>();
	pragma::ecs::BaseEntity *owner = whOwnerComponent.valid() ? whOwnerComponent->GetOwner() : nullptr;
	if(owner != nullptr && owner->IsCharacter())
		owner->GetCharacterComponent()->SetActiveWeapon(wep);
	wepComponent->Deploy();
}

void NET_cl_WEP_HOLSTER(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto wepComponent = wep->GetWeaponComponent();
	auto whOwnerComponent = wep->GetComponent<pragma::COwnableComponent>();
	pragma::ecs::BaseEntity *owner = whOwnerComponent.valid() ? whOwnerComponent->GetOwner() : nullptr;
	if(owner != nullptr && owner->IsCharacter())
		owner->GetCharacterComponent()->SetActiveWeapon(nullptr);
	wepComponent->Holster();
}

void NET_cl_WEP_PRIMARYATTACK(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->PrimaryAttack();
}

void NET_cl_WEP_SECONDARYATTACK(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->SecondaryAttack();
}

void NET_cl_WEP_ATTACK3(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->TertiaryAttack();
}

void NET_cl_WEP_ATTACK4(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->Attack4();
}

void NET_cl_WEP_RELOAD(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	wep->GetWeaponComponent()->Reload();
}

void NET_cl_WEP_PRIM_CLIP_SIZE(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto size = packet->Read<UInt16>();
	wep->GetWeaponComponent()->SetPrimaryClipSize(size);
}

void NET_cl_WEP_SEC_CLIP_SIZE(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto size = packet->Read<UInt16>();
	wep->GetWeaponComponent()->SetSecondaryClipSize(size);
}

void NET_cl_WEP_PRIM_MAX_CLIP_SIZE(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto size = packet->Read<UInt16>();
	wep->GetWeaponComponent()->SetMaxPrimaryClipSize(size);
}

void NET_cl_WEP_SEC_MAX_CLIP_SIZE(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto size = packet->Read<UInt16>();
	wep->GetWeaponComponent()->SetMaxSecondaryClipSize(size);
}

void NET_cl_WEP_PRIM_AMMO_TYPE(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto type = packet->Read<UInt32>();
	wep->GetWeaponComponent()->SetPrimaryAmmoType(type);
}

void NET_cl_WEP_SEC_AMMO_TYPE(NetPacket packet)
{
	auto *wep = pragma::networking::read_entity(packet);
	if(wep == nullptr || wep->IsWeapon() == false)
		return;
	auto type = packet->Read<UInt32>();
	wep->GetWeaponComponent()->SetSecondaryAmmoType(type);
}

void NET_cl_GAMEINFO(NetPacket packet) { pragma::get_client_state()->HandleReceiveGameInfo(packet); }

void NET_cl_SV_SEND(NetPacket packet)
{
	std::string msg = packet->ReadString();
	Con::CCL << "Received message from server: " << msg << Con::endl;
}

void NET_cl_ENV_LIGHT_SPOT_OUTERCUTOFF_ANGLE(NetPacket packet)
{
	auto *light = pragma::networking::read_entity(packet);
	auto pLightSpotComponent = (light != nullptr) ? light->GetComponent<pragma::CLightSpotComponent>() : pragma::ComponentHandle<pragma::CLightSpotComponent>();
	if(pLightSpotComponent.expired())
		return;
	float cutoffAngle = packet->Read<float>();
	pLightSpotComponent->SetOuterConeAngle(cutoffAngle);
}

void NET_cl_ENV_LIGHT_SPOT_INNERCUTOFF_ANGLE(NetPacket packet)
{
	auto *light = pragma::networking::read_entity(packet);
	auto pLightSpotComponent = (light != nullptr) ? light->GetComponent<pragma::CLightSpotComponent>() : pragma::ComponentHandle<pragma::CLightSpotComponent>();
	if(pLightSpotComponent.expired())
		return;
	float blendFraction = packet->Read<float>();
	pLightSpotComponent->SetBlendFraction(blendFraction);
}

void NET_cl_ENVLIGHT_SETSTATE(NetPacket packet)
{
	auto *light = pragma::networking::read_entity(packet);
	auto *pToggleComponent = (light != nullptr) ? static_cast<pragma::BaseToggleComponent *>(light->FindComponent("toggle").get()) : nullptr;
	if(pToggleComponent == nullptr)
		return;
	bool b = packet->Read<bool>();
	if(b)
		pToggleComponent->TurnOn();
	else
		pToggleComponent->TurnOff();
}

void NET_cl_ENVEXPLOSION_EXPLODE(NetPacket packet)
{
	auto *exp = pragma::networking::read_entity(packet);
	auto *pExplosionComponent = (exp != nullptr) ? static_cast<pragma::CExplosionComponent *>(exp->FindComponent("explosion").get()) : nullptr;
	if(pExplosionComponent == nullptr)
		return;
	pExplosionComponent->Explode();
}

void NET_cl_ENT_TOGGLE(NetPacket packet)
{
	auto *ent = pragma::networking::read_entity(packet);
	if(ent == nullptr)
		return;
	bool bOn = packet->Read<bool>();
	auto *pToggleComponent = static_cast<pragma::BaseToggleComponent *>(ent->FindComponent("toggle").get());
	if(pToggleComponent == nullptr)
		return;
	if(bOn == true)
		pToggleComponent->TurnOn();
	else
		pToggleComponent->TurnOff();
}

void NET_cl_ENT_SETCOLLISIONFILTER(NetPacket packet)
{
	auto *ent = pragma::networking::read_entity(packet);
	if(ent == nullptr)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	pragma::physics::CollisionMask filterGroup = static_cast<pragma::physics::CollisionMask>(packet->Read<unsigned int>());
	pragma::physics::CollisionMask filterMask = static_cast<pragma::physics::CollisionMask>(packet->Read<unsigned int>());
	pPhysComponent->SetCollisionFilter(filterGroup, filterMask);
}

void NET_cl_ENT_SETKINEMATIC(NetPacket packet)
{
	auto *ent = pragma::networking::read_entity(packet);
	if(ent == nullptr)
		return;
	auto pPhysComponent = ent->GetPhysicsComponent();
	if(pPhysComponent == nullptr)
		return;
	bool bKinematic = packet->Read<bool>();
	pPhysComponent->SetKinematic(bKinematic);
}

void NET_cl_ENV_FOGCON_SETSTARTDIST(NetPacket packet)
{
	auto *fog = pragma::networking::read_entity(packet);
	auto *pFogComponent = (fog != nullptr) ? static_cast<pragma::BaseEnvFogControllerComponent *>(fog->FindComponent("fog_controller").get()) : nullptr;
	if(pFogComponent == nullptr)
		return;
	pFogComponent->SetFogStart(packet->Read<float>());
}

void NET_cl_ENV_FOGCON_SETENDDIST(NetPacket packet)
{
	auto *fog = pragma::networking::read_entity(packet);
	auto *pFogComponent = (fog != nullptr) ? static_cast<pragma::BaseEnvFogControllerComponent *>(fog->FindComponent("fog_controller").get()) : nullptr;
	if(pFogComponent == nullptr)
		return;
	pFogComponent->SetFogEnd(packet->Read<float>());
}

void NET_cl_ENV_FOGCON_SETMAXDENSITY(NetPacket packet)
{
	auto *fog = pragma::networking::read_entity(packet);
	auto *pFogComponent = (fog != nullptr) ? static_cast<pragma::BaseEnvFogControllerComponent *>(fog->FindComponent("fog_controller").get()) : nullptr;
	if(pFogComponent == nullptr)
		return;
	pFogComponent->SetMaxDensity(packet->Read<float>());
}

void NET_cl_ENV_PRTSYS_SETCONTINUOUS(NetPacket packet)
{
	auto *prt = pragma::networking::read_entity(packet);
	auto *pParticleComponent = (prt != nullptr) ? static_cast<pragma::BaseEnvParticleSystemComponent *>(prt->FindComponent("particle_system").get()) : nullptr;
	if(pParticleComponent == nullptr)
		return;
	auto b = packet->Read<bool>();
	pParticleComponent->SetContinuous(b);
}

void NET_cl_CLIENT_DROPPED(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	auto *pl = pragma::networking::read_player(packet);
	if(pl == nullptr)
		return;
	auto reason = packet->Read<int32_t>();
	client->GetGameState()->OnPlayerDropped(*pl, static_cast<pragma::networking::DropReason>(reason));
}

void NET_cl_CLIENT_READY(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	auto *pl = pragma::networking::read_player(packet);
	if(pl == nullptr)
		return;
	client->GetGameState()->OnPlayerReady(*pl);
}

void NET_cl_CLIENT_JOINED(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	auto *pl = pragma::networking::read_player(packet);
	if(pl == nullptr)
		return;
	client->GetGameState()->OnPlayerJoined(*pl);
}

void NET_cl_CREATE_EXPLOSION(NetPacket packet)
{
	auto *client = pragma::get_client_state();
	if(!client->IsGameActive())
		return;
	auto origin = packet->Read<Vector3>();
	auto radius = packet->Read<float>();

	auto *pt = pragma::ecs::CParticleSystemComponent::Create("explosion");
	if(pt != nullptr) {
		auto pTrComponent = pt->GetEntity().GetTransformComponent();
		if(pTrComponent != nullptr)
			pTrComponent->SetPosition(origin);
		pt->SetRemoveOnComplete(true);
		pt->Start();
	}

	auto snd = client->CreateSound("fx.explosion", pragma::audio::ALSoundType::Effect, pragma::audio::ALCreateFlags::Mono);
	if(snd != nullptr) {
		snd->SetPosition(origin);
		snd->SetType(pragma::audio::ALSoundType::Effect);
		snd->Play();
	}

	auto *entQuake = pragma::get_cgame()->CreateEntity<CEnvQuake>();
	if(entQuake != nullptr) {
		auto *pQuakeComponent = static_cast<pragma::BaseEnvQuakeComponent *>(entQuake->FindComponent("quake").get());
		auto pTrComponent = entQuake->GetTransformComponent();
		if(pTrComponent != nullptr)
			pTrComponent->SetPosition(origin);
		if(pQuakeComponent != nullptr) {
			pQuakeComponent->SetFrequency(50.f);
			pQuakeComponent->SetAmplitude(50.f);
			pQuakeComponent->SetRadius(radius);
		}
		entQuake->SetKeyValue("spawnflags", std::to_string(SF_QUAKE_IN_AIR | SF_QUAKE_REMOVE_ON_COMPLETE));
		entQuake->Spawn();
		auto pInputComponent = entQuake->GetComponent<pragma::CIOComponent>();
		if(pInputComponent.valid())
			pInputComponent->Input("StartShake");
	}
}

void NET_cl_ENT_TRIGGER_GRAVITY_ONSTARTTOUCH(NetPacket packet)
{
	using namespace pragma;
	auto *ent = networking::read_entity(packet);
	if(ent == nullptr)
		return;
	auto netFlags = static_cast<Entity::TriggerGravity::NetFlags>(packet->Read<uint8_t>());
	auto flags = packet->Read<uint32_t>();
	auto gravityDir = packet->Read<Vector3>();
	auto gravityForce = packet->Read<float>();
	auto bUseForce = (netFlags & Entity::TriggerGravity::NetFlags::UseForce) != Entity::TriggerGravity::NetFlags::None;
	if((netFlags & Entity::TriggerGravity::NetFlags::StartTouch) != Entity::TriggerGravity::NetFlags::None)
		Entity::TriggerGravity::apply_gravity(ent, flags, -gravityDir, gravityDir, bUseForce, gravityForce);
	else {
		auto dirMove = packet->Read<Vector3>();
		Entity::TriggerGravity::apply_gravity(ent, flags, gravityDir, dirMove, bUseForce, gravityForce);
	}
}

struct NavDebugInfo {
	NavDebugInfo(const std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> &path = nullptr, const std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> &spline = nullptr, const std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> &node = nullptr) : dbgPath(path), dbgSpline(spline), dbgNode(node) {}
	std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> dbgPath;
	std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> dbgSpline;
	std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> dbgNode;
};
static std::unordered_map<const pragma::ecs::CBaseEntity *, NavDebugInfo> s_aiNavDebugObjects {};
void NET_cl_DEBUG_AI_NAVIGATION(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto *npc = static_cast<pragma::ecs::CBaseEntity *>(pragma::networking::read_entity(packet));
	if(npc == nullptr)
		return;
	std::vector<Vector3> points;
	auto numNodes = packet->Read<uint32_t>();
	auto currentNodeIdx = packet->Read<uint32_t>();
	points.reserve(numNodes * 2 - 2);
	std::shared_ptr<pragma::debug::DebugRenderer::BaseObject> dbgNode = nullptr;
	Con::CCL << "New path for " << npc->GetClass() << ":" << Con::endl;
	for(auto nodeId = decltype(numNodes) {0}; nodeId < numNodes; ++nodeId) {
		auto pos = packet->Read<Vector3>() + Vector3(0.f, 20.f, 0.f);
		points.push_back(pos);
		Con::CCL << "#" << (nodeId + 1) << ": " << pos << Con::endl;

		if(nodeId == currentNodeIdx)
			dbgNode = pragma::debug::DebugRenderer::DrawLine(pos, pos + Vector3 {0.f, 100.f, 0.f}, colors::Lime);
	}
	Con::CCL << Con::endl;

	auto pGenericComponent = npc->GetComponent<pragma::CGenericComponent>();
	if(pGenericComponent.valid()) {
		pGenericComponent->BindEventUnhandled(pragma::ecs::baseEntity::EVENT_ON_REMOVE, [pGenericComponent](std::reference_wrapper<pragma::ComponentEvent> evData) {
			auto it = s_aiNavDebugObjects.find(static_cast<const pragma::ecs::CBaseEntity *>(&pGenericComponent->GetEntity()));
			if(it == s_aiNavDebugObjects.end())
				return;
			s_aiNavDebugObjects.erase(it);
		});
	}
	s_aiNavDebugObjects[npc] = {pragma::debug::DebugRenderer::DrawPath(points, colors::Yellow), pragma::debug::DebugRenderer::DrawSpline(points, 10, 1.f, {colors::OrangeRed}), dbgNode};
}

void NET_cl_DEBUG_AI_SCHEDULE_PRINT(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	auto b = packet->Read<bool>();
	if(b == false)
		Con::COUT << "> NPC has no active schedule!" << Con::endl;
	else {
		auto msg = packet->ReadString();
		Con::COUT << "> Active NPC Schedule:" << Con::endl;
		Con::COUT << msg << Con::endl;
	}
}

static std::unique_ptr<DebugGameGUI> dbgAiSchedule = nullptr;
void CMD_debug_ai_schedule(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	dbgAiSchedule = nullptr;
	if(!check_cheats("debug_ai_schedule", state))
		return;
	if(pragma::get_cgame() == nullptr || pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	auto ents = pragma::console::find_target_entity(state, *charComponent, argv);
	pragma::ecs::BaseEntity *npc = nullptr;
	for(auto *ent : ents) {
		if(ent->IsNPC() == false)
			continue;
		npc = ent;
		break;
	}
	if(npc == nullptr) {
		Con::CWAR << "No valid NPC target found!" << Con::endl;
		return;
	}
	Con::COUT << "Querying schedule data for NPC " << *npc << "..." << Con::endl;
	NetPacket p;
	pragma::networking::write_entity(p, npc);
	pragma::get_client_state()->SendPacket(pragma::networking::net_messages::server::DEBUG_AI_SCHEDULE_TREE, p, pragma::networking::Protocol::SlowReliable);
}

void CMD_debug_draw_line(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		Con::CWAR << "No position has been specified!" << Con::endl;
		return;
	}
	if(!pragma::get_cgame()) {
		Con::CWAR << "No active game!" << Con::endl;
		return;
	}
	auto *cam = pragma::get_cgame()->GetRenderCamera<pragma::CCameraComponent>();
	if(!cam)
		cam = pragma::get_cgame()->GetPrimaryCamera<pragma::CCameraComponent>();
	if(!cam) {
		Con::CWAR << "No active camera found!" << Con::endl;
		return;
	}
	auto srcPos = cam->GetEntity().GetPosition();
	auto tgtPos = uvec::create(argv.front());
	pragma::get_cgame()->DrawLine(srcPos, tgtPos, colors::White, 12.f);
	pragma::get_cgame()->DrawLine(tgtPos, tgtPos + uvec::PRM_RIGHT, colors::Red, 12.f);
	pragma::get_cgame()->DrawLine(tgtPos, tgtPos + uvec::PRM_UP, colors::Lime, 12.f);
	pragma::get_cgame()->DrawLine(tgtPos, tgtPos + uvec::PRM_FORWARD, colors::Blue, 12.f);
}

void CMD_debug_aim_info(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(pl == nullptr)
		return;
	auto &entPl = pl->GetEntity();
	if(entPl.IsCharacter() == false)
		return;
	auto charComponent = entPl.GetCharacterComponent();
	auto ents = pragma::console::find_target_entity(state, *charComponent, argv);
	pragma::CCameraComponent *cam = nullptr;
	if(ents.empty() == false) {
		for(auto *ent : ents) {
			auto sceneC = ent->GetComponent<pragma::CSceneComponent>();
			if(sceneC.expired() || sceneC->GetActiveCamera().expired())
				continue;
			cam = sceneC->GetActiveCamera().get();
			break;
		}
	}
	cam = cam ? cam : pragma::get_cgame()->GetPrimaryCamera<pragma::CCameraComponent>();
	if(cam == nullptr)
		return;
	auto trC = cam->GetEntity().GetComponent<pragma::CTransformComponent>();
	if(trC.expired())
		return;
	auto trData = pragma::util::get_entity_trace_data(*trC);
	trData.SetFlags(pragma::physics::RayCastFlags::InvertFilter);
	trData.SetFilter(entPl);

	pragma::physics::TraceResult res {};
	pragma::ecs::EntityIterator entIt {*pragma::get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CRenderComponent>>();
	std::optional<pragma::math::intersection::LineMeshResult> closestMesh {};
	pragma::ecs::BaseEntity *entClosest = nullptr;
	for(auto *ent : entIt) {
		if(ent == &entPl)
			continue;
		auto renderC = ent->GetComponent<pragma::CRenderComponent>();
		auto lineMeshResult = renderC->CalcRayIntersection(trData.GetSourceOrigin(), trData.GetTargetOrigin(), true);
		if(lineMeshResult.has_value() == false || lineMeshResult->result != pragma::math::intersection::Result::Intersect)
			continue;
		if(closestMesh.has_value() && lineMeshResult->hitValue > closestMesh->hitValue)
			continue;
		closestMesh = lineMeshResult;
		entClosest = ent;
	}
	if(closestMesh.has_value()) {
		res.hitType = pragma::physics::RayCastHitType::Block;
		res.entity = entClosest->GetHandle();
		res.position = closestMesh->hitPos;
		res.normal = {};
		res.distance = uvec::distance(closestMesh->hitPos, trData.GetSourceOrigin());
		if(closestMesh->precise) {
			res.meshInfo = pragma::util::make_shared<pragma::physics::TraceResult::MeshInfo>();
			res.meshInfo->mesh = closestMesh->precise->mesh.get();
			res.meshInfo->subMesh = closestMesh->precise->subMesh.get();
		}
	}
	else
		res = pragma::get_cgame()->RayCast(trData);

	if(res.hitType == pragma::physics::RayCastHitType::None) {
		Con::COUT << "Nothing found in player aim direction!" << Con::endl;
		return;
	}
	Con::COUT << "Hit Entity: ";
	if(res.entity.valid() == false)
		Con::COUT << "NULL";
	else
		res.entity->print(Con::COUT);
	Con::COUT << Con::endl;
	Con::COUT << "Hit Position: (" << res.position.x << "," << res.position.y << "," << res.position.z << ")" << Con::endl;
	Con::COUT << "Hit Normal: (" << res.normal.x << "," << res.normal.y << "," << res.normal.z << ")" << Con::endl;
	Con::COUT << "Hit Distance: " << res.distance << Con::endl;
	std::string mat;
	auto b = res.GetMaterial(mat);
	Con::COUT << "Hit Material: " << (b ? mat : "Nothing") << Con::endl;
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_ai_schedule", &CMD_debug_ai_schedule, pragma::console::ConVarFlags::None, "Prints the current schedule behavior tree for the specified NPC on screen.");
	auto UVN = pragma::console::client::register_command("debug_draw_line", &CMD_debug_draw_line, pragma::console::ConVarFlags::None, "Draws a line from the current camera position to the specified target position");
	auto UVN = pragma::console::client::register_command("debug_aim_info", &CMD_debug_aim_info, pragma::console::ConVarFlags::None, "Prints information about whatever the local player is looking at.");
}

void NET_cl_DEBUG_AI_SCHEDULE_TREE(NetPacket packet)
{
	if(!pragma::get_client_state()->IsGameActive())
		return;
	const auto bUseGraphicVisualization = true;
	auto updateState = packet->Read<uint8_t>();
	std::shared_ptr<pragma::debug::DebugBehaviorTreeNode> dbgTree = nullptr;
	pragma::ecs::BaseEntity *ent = nullptr;

	static const auto fGetStateInfo = [](const pragma::debug::DebugBehaviorTreeNode &node, Color &col, std::string &text) {
		col = colors::White;
		text = node.name + " (";
		switch(node.state) {
		case pragma::debug::DebugBehaviorTreeNode::State::Pending:
			//text += "pending";
			col = colors::Aqua;
			break;
		case pragma::debug::DebugBehaviorTreeNode::State::Failed:
			//text += "failed";
			col = colors::Maroon;
			break;
		case pragma::debug::DebugBehaviorTreeNode::State::Initial:
			//text += "initial";
			col = colors::Gray;
			break;
		default:
			//text += "succeeded";
			col = colors::Lime;
			break;
		}
		//text += ", ";
		switch(node.nodeType) {
		case pragma::debug::DebugBehaviorTreeNode::BehaviorNodeType::Selector:
			text += "selector";
			break;
		case pragma::debug::DebugBehaviorTreeNode::BehaviorNodeType::Sequence:
			text += "sequence";
			break;
		default:
			text = "unknown";
			break;
		}
		text += ", ";
		switch(node.selectorType) {
		case pragma::debug::DebugBehaviorTreeNode::SelectorType::Sequential:
			text += "sequential";
			break;
		case pragma::debug::DebugBehaviorTreeNode::SelectorType::RandomShuffle:
			text += "random shuffle";
			break;
		default:
			text = "unknown";
			break;
		}
		text += ")";
	};

	const auto fUpdateGraphicalGUI = [](pragma::debug::DebugBehaviorTreeNode &dbgTree, pragma::gui::types::WILuaBase &pElLua) {
		auto *l = pragma::get_cgame()->GetLuaState();
		auto t = Lua::CreateTable(l);
		std::function<void(pragma::debug::DebugBehaviorTreeNode &)> fPushNode = nullptr;
		fPushNode = [l, &fPushNode](pragma::debug::DebugBehaviorTreeNode &node) {
			auto tNode = Lua::CreateTable(l);

			Lua::PushString(l, "name");
			Lua::PushString(l, node.name);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "nodeType");
			Lua::PushInt(l, pragma::math::to_integral(node.nodeType));
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "selectorType");
			Lua::PushInt(l, pragma::math::to_integral(node.selectorType));
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "lastUpdate");
			Lua::PushNumber(l, node.lastUpdate);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "lastStartTime");
			Lua::PushNumber(l, node.lastStartTime);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "lastEndTime");
			Lua::PushNumber(l, node.lastEndTime);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "executionIndex");
			Lua::PushInt(l, node.executionIndex);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "active");
			Lua::PushBool(l, node.active);
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "state");
			Lua::PushInt(l, pragma::math::to_integral(node.state));
			Lua::SetTableValue(l, tNode);

			Lua::PushString(l, "children");
			auto tChildren = Lua::CreateTable(l);
			auto idx = 1;
			for(auto &pChild : node.children) {
				Lua::PushInt(l, idx++);
				fPushNode(*pChild);
				Lua::SetTableValue(l, tChildren);
			}
			Lua::SetTableValue(l, tNode);
		};
		auto o = pElLua.GetLuaObject();
		o.push(l); /* 1 */
		auto tIdx = Lua::GetStackTop(l);
		Lua::PushString(l, "BuildTree"); /* 2 */
		Lua::GetTableValue(l, tIdx);     /* 2 */
		if(Lua::IsFunction(l, -1)) {
			o.push(l);          /* 3 */
			fPushNode(dbgTree); /* 4 */
			std::string err;
			Lua::ProtectedCall(l, 2, 0, err); /* 1 */
		}
		else
			Lua::Pop(l, 1); /* 1 */
		Lua::Pop(l, 1);     /* 0 */
	};

	if(dbgAiSchedule != nullptr && bUseGraphicVisualization == true && (updateState == 0 || updateState == 1)) {
		auto *pEl = dbgAiSchedule->GetGUIElement();
		if(pEl != nullptr) {
			auto *pElLua = static_cast<pragma::gui::types::WILuaBase *>(pEl);
			auto *l = pragma::get_cgame()->GetLuaState();
			auto o = pElLua->GetLuaObject();
			o.push(l); /* 1 */
			auto tIdx = Lua::GetStackTop(l);
			Lua::PushString(l, "ClearNodes"); /* 2 */
			Lua::GetTableValue(l, tIdx);      /* 2 */
			if(Lua::IsFunction(l, -1)) {
				o.push(l); /* 3 */
				std::string err;
				Lua::ProtectedCall(l, 1, 0, err); /* 1 */
			}
			else
				Lua::Pop(l, 1); /* 1 */
			Lua::Pop(l, 1);     /* 0 */
		}
	}

	if(updateState == 0) {
		dbgAiSchedule = nullptr;
		return;
	}
	else if(updateState == 1) // Full update
	{
		dbgAiSchedule = nullptr;
		ent = pragma::networking::read_entity(packet);
		if(ent == nullptr)
			return;
		dbgTree = pragma::util::make_shared<pragma::debug::DebugBehaviorTreeNode>();
		std::function<void(NetPacket &, pragma::debug::DebugBehaviorTreeNode &)> fReadTree = nullptr;
		fReadTree = [&fReadTree](NetPacket &p, pragma::debug::DebugBehaviorTreeNode &node) {
			node.name = p->ReadString();
			node.nodeType = static_cast<pragma::debug::DebugBehaviorTreeNode::BehaviorNodeType>(p->Read<uint32_t>());
			node.selectorType = static_cast<pragma::debug::DebugBehaviorTreeNode::SelectorType>(p->Read<uint32_t>());
			node.lastStartTime = p->Read<float>();
			node.lastEndTime = p->Read<float>();
			node.executionIndex = p->Read<uint64_t>();
			node.active = p->Read<bool>();
			node.state = p->Read<pragma::debug::DebugBehaviorTreeNode::State>();
			auto numChildren = p->Read<uint32_t>();
			node.children.reserve(numChildren);
			for(auto i = decltype(numChildren) {0}; i < numChildren; ++i) {
				node.children.push_back(pragma::util::make_shared<pragma::debug::DebugBehaviorTreeNode>());
				fReadTree(p, *node.children.back());
			}
		};
		fReadTree(packet, *dbgTree);
	}
	else {
		if(dbgAiSchedule == nullptr)
			return;
		auto *pEl = dbgAiSchedule->GetGUIElement();
		if(pEl == nullptr)
			return;
		dbgTree = std::static_pointer_cast<pragma::debug::DebugBehaviorTreeNode>(dbgAiSchedule->GetUserData(0));
		auto *hEnt = static_cast<EntityHandle *>(dbgAiSchedule->GetUserData(1).get());
		if(dbgTree == nullptr || hEnt == nullptr || hEnt->valid() == false) {
			dbgAiSchedule = nullptr;
			return;
		}
		ent = hEnt->get();

		std::function<void(NetPacket &, pragma::debug::DebugBehaviorTreeNode &, pragma::gui::types::WITreeListElement *)> fReadUpdates = nullptr;
		fReadUpdates = [&fReadUpdates, bUseGraphicVisualization](NetPacket &p, pragma::debug::DebugBehaviorTreeNode &node, pragma::gui::types::WITreeListElement *pEl) {
			if(bUseGraphicVisualization == false && pEl == nullptr)
				return;
			auto state = p->Read<pragma::debug::DebugBehaviorTreeNode::State>();
			if(state == pragma::debug::DebugBehaviorTreeNode::State::Invalid)
				return;
			node.state = state;
			node.active = p->Read<bool>();
			node.lastStartTime = p->Read<float>();
			node.lastEndTime = p->Read<float>();
			node.executionIndex = p->Read<uint64_t>();

			if(bUseGraphicVisualization == false) {
				Color col;
				std::string text;
				fGetStateInfo(node, col, text);
				auto *pText = pEl->GetTextElement();
				if(pText != nullptr) {
					pText->SetColor(col);
					//pText->SetText(text);
				}

				auto &guiChildren = pEl->GetItems();
				for(auto i = decltype(node.children.size()) {0}; i < node.children.size(); ++i) {
					if(i >= guiChildren.size())
						break;
					auto &child = node.children[i];
					auto &guiChild = guiChildren[i];
					fReadUpdates(p, *child, const_cast<pragma::gui::types::WITreeListElement *>(static_cast<const pragma::gui::types::WITreeListElement *>(guiChild.get())));
				}
			}
			else {
				for(auto &child : node.children)
					fReadUpdates(p, *child, nullptr);
			}
		};
		auto *pRoot = (bUseGraphicVisualization == false) ? static_cast<pragma::gui::types::WITreeList *>(pEl)->GetRootItem() : nullptr;
		if(bUseGraphicVisualization == false && pRoot == nullptr)
			return;
		if(bUseGraphicVisualization == false) {
			for(uint8_t i = 0; i < 2; ++i) {
				auto &children = pRoot->GetItems();
				if(children.empty() || children.front().IsValid() == false)
					return;
				pRoot = const_cast<pragma::gui::types::WITreeListElement *>(static_cast<const pragma::gui::types::WITreeListElement *>(children.front().get()));
			}
		}

		fReadUpdates(packet, *dbgTree, pRoot);
		if(bUseGraphicVisualization == true)
			fUpdateGraphicalGUI(*dbgTree, static_cast<pragma::gui::types::WILuaBase &>(*pEl));
		return;
	}

	pragma::gui::types::WIBase *pEl = nullptr;
	if(bUseGraphicVisualization == false) {
		auto *pTreeList = pragma::gui::WGUI::GetInstance().Create<pragma::gui::types::WITreeList>();
		pEl = pTreeList;
		if(pTreeList == nullptr)
			return;
		pTreeList->SetVisible(false);
		std::function<void(const pragma::debug::DebugBehaviorTreeNode &, pragma::gui::types::WITreeListElement *)> fAddItems = nullptr;
		fAddItems = [&fAddItems](const pragma::debug::DebugBehaviorTreeNode &node, pragma::gui::types::WITreeListElement *pEl) {
			Color col;
			std::string text;
			fGetStateInfo(node, col, text);

			auto *pChild = pEl->AddItem(text);
			auto *pText = pChild->GetTextElement();
			if(pText != nullptr) {
				pText->SetColor(col);
				pText->EnableShadow(true);
				pText->SetShadowColor(colors::Black);
				pText->SetShadowOffset(1, 1);
			}
			for(auto &dbgChild : node.children)
				fAddItems(*dbgChild, pChild);
		};
		auto *pRoot = pTreeList->AddItem("");
		fAddItems(*dbgTree, pRoot);
		pTreeList->SetSize(1024, 1024); // TODO: Size to contents
	}
	else {
		auto *pGraphics = dynamic_cast<pragma::gui::types::WILuaBase *>(pragma::get_cgame()->CreateGUIElement("WIDebugBehaviorTree"));
		if(pGraphics == nullptr)
			return;
		fUpdateGraphicalGUI(*dbgTree, *pGraphics);
		pEl = pGraphics;
	}

	auto pGenericComponent = ent->GetComponent<pragma::CGenericComponent>();
	CallbackHandle cbOnRemove {};
	if(pGenericComponent.valid()) {
		cbOnRemove = pGenericComponent->BindEventUnhandled(pragma::ecs::baseEntity::EVENT_ON_REMOVE, [](std::reference_wrapper<pragma::ComponentEvent> evData) { dbgAiSchedule = nullptr; });
	}

	dbgAiSchedule = std::make_unique<DebugGameGUI>([pEl]() { return pEl->GetHandle(); });
	dbgAiSchedule->CallOnRemove([cbOnRemove]() mutable {
		if(cbOnRemove.IsValid())
			cbOnRemove.Remove();
	});
	dbgAiSchedule->SetUserData(0, dbgTree);
	dbgAiSchedule->SetUserData(1, pragma::util::make_shared<EntityHandle>(ent->GetHandle()));
}

void NET_cl_CMD_CALL_RESPONSE(NetPacket packet)
{
	auto resultFlags = packet->Read<uint8_t>();
	if(resultFlags == 0)
		Con::COUT << "> Serverside command execution has failed." << Con::endl;
	else {
		if(resultFlags == 1)
			Con::COUT << "> Serverside command has been executed successfully." << Con::endl;
		else {
			auto val = packet->ReadString();
			Con::COUT << "> Serverside command has been executed successfully. New value: " << val << Con::endl;
		}
	}
}

void NET_cl_ADD_SHARED_COMPONENT(NetPacket packet)
{
	auto *ent = pragma::networking::read_entity(packet);
	if(ent == nullptr || pragma::get_cgame() == nullptr)
		return;
	auto componentId = packet->Read<pragma::ComponentId>();
	auto &componentManager = static_cast<pragma::CEntityComponentManager &>(pragma::get_cgame()->GetEntityComponentManager());
	auto &componentTypes = componentManager.GetRegisteredComponentTypes();
	auto &svComponentToClComponentTable = componentManager.GetServerComponentIdToClientComponentIdTable();
	if(componentId >= svComponentToClComponentTable.size())
		return;
	ent->AddComponent(svComponentToClComponentTable.at(componentId));
}

namespace {
	auto _ = pragma::console::client::register_variable_listener<bool>(
	  "debug_ai_navigation", +[](pragma::NetworkState *state, const pragma::console::ConVar &, bool, bool val) {
		  if(!check_cheats("debug_ai_navigation", state))
			  return;
		  if(pragma::get_cgame() == nullptr)
			  return;
		  if(val == false)
			  s_aiNavDebugObjects.clear();
		  NetPacket p {};
		  p->Write<bool>(val);
		  pragma::get_client_state()->SendPacket(pragma::networking::net_messages::server::DEBUG_AI_NAVIGATION, p, pragma::networking::Protocol::SlowReliable);
	  });
}

void NET_cl_DEBUG_DRAWPOINT(NetPacket packet)
{
	auto pos = packet->Read<Vector3>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(pos);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	pragma::debug::DebugRenderer::DrawPoint(renderInfo);
}
void NET_cl_DEBUG_DRAWLINE(NetPacket packet)
{
	auto start = packet->Read<Vector3>();
	auto end = packet->Read<Vector3>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	pragma::debug::DebugRenderer::DrawLine(start, end, renderInfo);
}
void NET_cl_DEBUG_DRAWBOX(NetPacket packet)
{
	auto center = packet->Read<Vector3>();
	auto min = packet->Read<Vector3>();
	auto max = packet->Read<Vector3>();
	auto ang = packet->Read<EulerAngles>();
	auto col = packet->Read<Color>();
	auto bOutlineColor = packet->Read<bool>();
	Color colOutline = {};
	auto dur = packet->Read<float>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(center);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	renderInfo.SetRotation(uquat::create(ang));
	if(bOutlineColor == true) {
		colOutline = packet->Read<Color>();
		renderInfo.SetOutlineColor(colOutline);
	}
	pragma::debug::DebugRenderer::DrawBox(min, max, renderInfo);
}
void NET_cl_DEBUG_DRAWTEXT(NetPacket packet)
{
	auto text = packet->ReadString();
	auto pos = packet->Read<Vector3>();
	auto bSize = packet->Read<bool>();
	Vector2 size {};
	auto scale = 1.f;
	if(bSize == true)
		size = packet->Read<Vector2>();
	else
		scale = packet->Read<float>();
	auto bColor = packet->Read<bool>();
	Color col {};
	if(bColor == true)
		col = packet->Read<Color>();
	auto duration = packet->Read<float>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(pos);
	if(bColor == true)
		renderInfo.SetColor(col);
	renderInfo.SetDuration(duration);
	if(bSize == true)
		pragma::debug::DebugRenderer::DrawText(renderInfo, text, size);
	else
		pragma::debug::DebugRenderer::DrawText(renderInfo, text, scale);
}
void NET_cl_DEBUG_DRAWSPHERE(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto radius = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	auto recursionLevel = packet->Read<uint32_t>();
	auto bOutline = packet->Read<bool>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	if(bOutline) {
		auto colOutline = packet->Read<Color>();
		renderInfo.SetOutlineColor(colOutline);
	}
	pragma::debug::DebugRenderer::DrawSphere(renderInfo, radius, recursionLevel);
}
void NET_cl_DEBUG_DRAWCONE(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto dir = packet->Read<Vector3>();
	auto dist = packet->Read<float>();
	auto angle = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto duration = packet->Read<float>();
	auto segmentCount = packet->Read<uint32_t>();
	auto bOutline = packet->Read<bool>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(duration);
	if(bOutline) {
		auto colOutline = packet->Read<Color>();
		renderInfo.SetOutlineColor(colOutline);
	}
	pragma::debug::DebugRenderer::DrawCone(renderInfo, dir, dist, angle, segmentCount);
}
void NET_cl_DEBUG_DRAWAXIS(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto ang = packet->Read<EulerAngles>();
	auto dur = packet->Read<float>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetRotation(uquat::create(ang));
	renderInfo.SetDuration(dur);
	pragma::debug::DebugRenderer::DrawAxis(renderInfo);
}
void NET_cl_DEBUG_DRAWPATH(NetPacket packet)
{
	auto numPath = packet->Read<uint32_t>();
	std::vector<Vector3> path;
	path.reserve(numPath);
	for(auto i = decltype(numPath) {0}; i < numPath; ++i)
		path.push_back(packet->Read<Vector3>());
	auto col = packet->Read<Color>();
	auto duration = packet->Read<float>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetColor(col);
	renderInfo.SetDuration(duration);
	pragma::debug::DebugRenderer::DrawPath(path, renderInfo);
}
void NET_cl_DEBUG_DRAWSPLINE(NetPacket packet)
{
	auto numPath = packet->Read<uint32_t>();
	std::vector<Vector3> path;
	path.reserve(numPath);
	for(auto i = decltype(numPath) {0}; i < numPath; ++i)
		path.push_back(packet->Read<Vector3>());
	auto col = packet->Read<Color>();
	auto numSegments = packet->Read<uint32_t>();
	auto curvature = packet->Read<float>();
	auto duration = packet->Read<float>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetColor(col);
	renderInfo.SetDuration(duration);
	pragma::debug::DebugRenderer::DrawSpline(path, numSegments, curvature, renderInfo);
}
void NET_cl_DEBUG_DRAWPLANE(NetPacket packet)
{
	auto n = packet->Read<Vector3>();
	auto d = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	pragma::debug::DebugRenderer::DrawPlane(n, d, renderInfo);
}
void NET_cl_DEBUG_DRAW_MESH(NetPacket packet)
{
	auto numTris = packet->Read<uint32_t>();
	std::vector<Vector3> verts;
	verts.resize(numTris * 3);
	packet->Read(verts.data(), pragma::util::size_of_container(verts));
	auto color = packet->Read<Color>();
	auto colorOutline = packet->Read<Color>();
	auto duration = packet->Read<float>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetColor(color);
	renderInfo.SetOutlineColor(colorOutline);
	renderInfo.SetDuration(duration);
	pragma::debug::DebugRenderer::DrawMesh(verts, renderInfo);
}
void NET_cl_DEBUG_DRAWTRUNCATEDCONE(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto startRadius = packet->Read<float>();
	auto dir = packet->Read<Vector3>();
	auto dist = packet->Read<float>();
	auto endRadius = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	auto segmentCount = packet->Read<uint32_t>();
	auto bOutline = packet->Read<bool>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	if(bOutline) {
		auto colOutline = packet->Read<Color>();
		renderInfo.SetOutlineColor(colOutline);
	}
	pragma::debug::DebugRenderer::DrawTruncatedCone(renderInfo, startRadius, dir, dist, endRadius, segmentCount);
}
void NET_cl_DEBUG_DRAWCYLINDER(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto dir = packet->Read<Vector3>();
	auto dist = packet->Read<float>();
	auto radius = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	auto segmentCount = packet->Read<uint32_t>();
	auto bOutline = packet->Read<bool>();
	pragma::debug::DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	if(bOutline) {
		auto colOutline = packet->Read<Color>();
		renderInfo.SetOutlineColor(colOutline);
	}
	pragma::debug::DebugRenderer::DrawCylinder(renderInfo, dir, dist, radius, segmentCount);
}
