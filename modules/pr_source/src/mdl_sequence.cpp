#include "mdl_sequence.h"
#include "mdl.h"
#include <sharedutils/util_string.h>

const std::unordered_map<std::string,int32_t> animEvents = {
	{"AE_INVALID",-1},
	/*{"AE_EMPTY",0},
	{"AE_NPC_LEFTFOOT",1},
	{"AE_NPC_RIGHTFOOT",2},
	{"AE_NPC_BODYDROP_LIGHT",3},
	{"AE_NPC_BODYDROP_HEAVY",4},
	{"AE_NPC_SWISHSOUND",5},
	{"AE_NPC_180TURN",6},
	{"AE_NPC_ITEM_PICKUP",7},
	{"AE_NPC_WEAPON_DROP",8},
	{"AE_NPC_WEAPON_SET_SEQUENCE_NAME",9},
	{"AE_NPC_WEAPON_SET_SEQUENCE_NUMBER",10},
	{"AE_NPC_WEAPON_SET_ACTIVITY",11},
	{"AE_NPC_HOLSTER",12},
	{"AE_NPC_DRAW",13},
	{"AE_NPC_WEAPON_FIRE",14},

	{"AE_CL_PLAYSOUND",15},
	{"AE_SV_PLAYSOUND",16},
	{"AE_CL_STOPSOUND",17},

	{"AE_START_SCRIPTED_EFFECT",18},
	{"AE_STOP_SCRIPTED_EFFECT",19},

	{"AE_CLIENT_EFFECT_ATTACH",20},
	
	{"AE_MUZZLEFLASH",21},
	{"AE_NPC_MUZZLEFLASH",22},
	
	{"AE_THUMPER_THUMP",23},
	{"AE_AMMOCRATE_PICKUP_AMMO",24},

	{"AE_NPC_RAGDOLL",25},

	{"AE_NPC_ADDGESTURE",26},
	{"AE_NPC_RESTARTGESTURE",27},

	{"AE_NPC_ATTACK_BROADCAST",28},

	{"AE_NPC_HURT_INTERACTION_PARTNER",29},
	{"AE_NPC_SET_INTERACTION_CANTDIE",30},

	{"AE_SV_DUSTTRAIL",31},

	{"AE_CL_CREATE_PARTICLE_EFFECT",32},

	{"AE_RAGDOLL",33},

	{"AE_CL_ENABLE_BODYGROUP",34},
	{"AE_CL_DISABLE_BODYGROUP",35},
	{"AE_CL_BODYGROUP_SET_VALUE",36},
	{"AE_CL_BODYGROUP_SET_VALUE_CMODEL_WPN",37},

	{"AE_WPN_PRIMARYATTACK",38},
	{"AE_WPN_INCREMENTAMMO",39},

	{"AE_WPN_HIDE",40},
	{"AE_WPN_UNHIDE",41},

	{"AE_WPN_PLAYWPNSOUND",42},*/
	// Everything below 1000 is reserved for NPC-specific events

	{"SCRIPT_EVENT_DEAD",1000},
	{"SCRIPT_EVENT_NOINTERRUPT",1001},
	{"SCRIPT_EVENT_CANINTERRUPT",1002},
	{"SCRIPT_EVENT_FIREEVENT",1003},
	{"SCRIPT_EVENT_SOUND",1004},
	{"SCRIPT_EVENT_SENTENCE",1005},
	{"SCRIPT_EVENT_INAIR",1006},
	{"SCRIPT_EVENT_ENDANIMATION",1007},
	{"SCRIPT_EVENT_SOUND_VOICE",1008},
	{"SCRIPT_EVENT_SENTENCE_RND1",1009},
	{"SCRIPT_EVENT_NOT_DEAD",1010},
	{"SCRIPT_EVENT_EMPHASIS",1011},

	{"SCRIPT_EVENT_BODYGROUPON",1020},
	{"SCRIPT_EVENT_BODYGROUPOFF",1021},
	{"SCRIPT_EVENT_BODYGROUPTEMP",1022},

	{"SCRIPT_EVENT_FIRE_INPUT",1100},

	{"NPC_EVENT_BODYDROP_LIGHT",2001},
	{"NPC_EVENT_BODYDROP_HEAVY",2002},
	{"NPC_EVENT_SWISHSOUND",2010},
	{"NPC_EVENT_180TURN",2020},
	{"NPC_EVENT_ITEM_PICKUP",2040},
	{"NPC_EVENT_WEAPON_DROP",2041},
	{"NPC_EVENT_WEAPON_SET_SEQUENCE_NAME",2042},
	{"NPC_EVENT_WEAPON_SET_SEQUENCE_NUMBER",2043},
	{"NPC_EVENT_WEAPON_SET_ACTIVITY",2044},
	{"NPC_EVENT_LEFTFOOT",2050},
	{"NPC_EVENT_RIGHTFOOT",2051},
	{"NPC_EVENT_OPEN_DOOR",2060},

	{"EVENT_WEAPON_MELEE_HIT",3001},
	{"EVENT_WEAPON_SMG1",3002},
	{"EVENT_WEAPON_MELEE_SWISH",3003},
	{"EVENT_WEAPON_SHOTGUN_FIRE",3004},
	{"EVENT_WEAPON_THROW",3005},
	{"EVENT_WEAPON_AR1",3006},
	{"EVENT_WEAPON_AR2",3007},
	{"EVENT_WEAPON_HMG1",3008},
	{"EVENT_WEAPON_SMG2",3009},
	{"EVENT_WEAPON_MISSILE_FIRE",3010},
	{"EVENT_WEAPON_SNIPER_RIFLE_FIRE",3011},
	{"EVENT_WEAPON_AR2_GRENADE",3012},
	{"EVENT_WEAPON_THROW2",3013},
	{"EVENT_WEAPON_PISTOL_FIRE",3014},
	{"EVENT_WEAPON_RELOAD",3015},
	{"EVENT_WEAPON_THROW3",3016},
	{"EVENT_WEAPON_RELOAD_SOUND",3017},
	{"EVENT_WEAPON_RELOAD_FILL_CLIP",3018},
	{"EVENT_WEAPON_SMG1_BURST1",3101},
	{"EVENT_WEAPON_SMG1_BURSTN",3102},
	{"EVENT_WEAPON_AR2_ALTFIRE",3103},

	{"EVENT_WEAPON_SEQUENCE_FINISHED",3900},

	{"CL_EVENT_MUZZLEFLASH0",5001},
	{"CL_EVENT_MUZZLEFLASH1",5011},
	{"CL_EVENT_MUZZLEFLASH2",5021},
	{"CL_EVENT_MUZZLEFLASH3",5031},
	{"CL_EVENT_SPARK0",5002},
	{"CL_EVENT_NPC_MUZZLEFLASH0",5003},
	{"CL_EVENT_NPC_MUZZLEFLASH1",5013},
	{"CL_EVENT_NPC_MUZZLEFLASH2",5023},
	{"CL_EVENT_NPC_MUZZLEFLASH3",5033},
	{"CL_EVENT_SOUND",5004},
	{"CL_EVENT_EJECTBRASS1",6001},
	{"CL_EVENT_DISPATCHEFFECT0",9001},
	{"CL_EVENT_DISPATCHEFFECT1",9011},
	{"CL_EVENT_DISPATCHEFFECT2",9021},
	{"CL_EVENT_DISPATCHEFFECT3",9031},
	{"CL_EVENT_DISPATCHEFFECT4",9041},
	{"CL_EVENT_DISPATCHEFFECT5",9051},
	{"CL_EVENT_DISPATCHEFFECT6",9061},
	{"CL_EVENT_DISPATCHEFFECT7",9071},
	{"CL_EVENT_DISPATCHEFFECT8",9081},
	{"CL_EVENT_DISPATCHEFFECT9",9091},

	{"CL_EVENT_SPRITEGROUP_CREATE",6002},
	{"CL_EVENT_SPRITEGROUP_DESTROY",6003},
	{"CL_EVENT_FOOTSTEP_LEFT",6004},
	{"CL_EVENT_FOOTSTEP_RIGHT",6005}
};

import::mdl::Sequence::Event::Event(const VFilePtr &f)
{
	auto offset = f->Tell();
	auto stdEv = f->Read<mstudioevent_t>();
	m_cycle = stdEv.cycle;
	m_options = stdEv.options.data();

	if(stdEv.szeventindex != 0)
	{
		f->Seek(offset +stdEv.szeventindex);
		m_name = f->ReadString();
	}
	auto name = m_name;
	for(auto &pair : animEvents)
	{
		if(pair.second == stdEv.event)
		{
			name = pair.first;
			break;
		}
	}
	if(name.empty())
	{
		std::string prefix;
		if(stdEv.event < 1'000)
			prefix = "NPC_";
		else if(stdEv.event < 2'000)
			prefix = "SCRIPT_EVENT_";
		else if(stdEv.event < 3'000)
			prefix = "NPC_EVENT_";
		else if(stdEv.event < 4'000)
			prefix = "EVENT_WEAPON_";
		else if(stdEv.event >= 5'000 && stdEv.event < 6'000)
			prefix = "CL_EVENT_";
		name = prefix +std::to_string(stdEv.event);
	}
	const std::unordered_map<std::string,std::string> prefixReplace = {
		{"SCRIPT_EVENT_","SCRIPT_"},
		{"NPC_EVENT_","NPC_"},
		{"EVENT_WEAPON_","WEAPON_"},
		{"CL_EVENT_","CL_"}
	};
	for(auto &pair : prefixReplace)
	{
		if(ustring::compare(name.c_str(),pair.first.c_str(),true,pair.first.length()) == true)
		{
			name = pair.second +name.substr(pair.first.length());
			break;
		}
	}
	m_event = ::Animation::GetEventEnumRegister().RegisterEnum("EVENT_" +name);

	f->Seek(offset +sizeof(mstudioevent_t));
}

float import::mdl::Sequence::Event::GetCycle() const {return m_cycle;}
int32_t import::mdl::Sequence::Event::GetEvent() const {return m_event;}
const std::string &import::mdl::Sequence::Event::GetName() const {return m_name;}
const std::string &import::mdl::Sequence::Event::GetOptions() const {return m_options;}

void import::mdl::Sequence::Event::DebugPrint(const std::string &t)
{
	std::cout<<t<<"Cycle: "<<m_cycle<<std::endl;
	std::cout<<t<<"Event: "<<m_event<<std::endl;
	std::cout<<t<<"Name: "<<m_name<<std::endl;
	std::cout<<t<<"Options: "<<m_options<<std::endl;
}

/////////////////////////

void import::mdl::Sequence::DebugPrint(const std::string &t)
{
	std::cout<<t<<"Activity: "<<m_activity<<std::endl;
	std::cout<<t<<"Activity weight: "<<m_activityWeight<<std::endl;
	std::cout<<t<<"Fade in time: "<<m_fadeInTime<<std::endl;
	std::cout<<t<<"Fade out time: "<<m_fadeOutTime<<std::endl;
	std::cout<<t<<"Bounds: ("<<m_min.x<<","<<m_min.y<<","<<m_min.z<<") ("<<m_max.x<<","<<m_max.y<<","<<m_max.z<<")"<<std::endl;
	std::cout<<t<<"Events:"<<std::endl;
	for(auto &ev : m_events)
	{
		ev.DebugPrint(t +"\t");
		std::cout<<std::endl;
	}
}

import::mdl::Sequence::Sequence(const VFilePtr &f,int32_t boneCount)
{
	auto offset = f->Tell();
	auto stdSeqDesc = f->Read<mdl::mstudioseqdesc_t>();
	m_activity = stdSeqDesc.activity;
	m_activityWeight = stdSeqDesc.actweight;
	m_fadeInTime = stdSeqDesc.fadeintime;
	m_fadeOutTime = stdSeqDesc.fadeouttime;
	m_min = stdSeqDesc.bbmin;
	m_max = stdSeqDesc.bbmax;

	m_poseParam.paramIdx = stdSeqDesc.paramindex;
	m_poseParam.start = stdSeqDesc.paramstart;
	m_poseParam.end = stdSeqDesc.paramend;
	m_poseParam.numBlends = stdSeqDesc.numblends;

	umath::swap(m_min.y,m_min.z);
	umath::swap(m_max.y,m_max.z);
	umath::negate(m_min.y);
	umath::negate(m_max.y);

	m_flags = stdSeqDesc.flags;

	if(stdSeqDesc.szlabelindex != 0)
	{
		f->Seek(offset +stdSeqDesc.szlabelindex);
		m_name = f->ReadString();
	}

	if(stdSeqDesc.szactivitynameindex != 0)
	{
		f->Seek(offset +stdSeqDesc.szactivitynameindex);
		m_activityName = f->ReadString();
	}

	if(stdSeqDesc.eventindex != 0)
	{
		f->Seek(offset +stdSeqDesc.eventindex);
		m_events.reserve(stdSeqDesc.numevents);
		for(auto i=decltype(stdSeqDesc.numevents){0};i<stdSeqDesc.numevents;++i)
			m_events.push_back(Event(f));
	}

	if(stdSeqDesc.weightlistindex != 0)
	{
		f->Seek(offset +stdSeqDesc.weightlistindex);
		m_weights.resize(boneCount);
		f->Read(m_weights.data(),m_weights.size() *sizeof(m_weights.front()));
		auto it = std::find_if(m_weights.begin(),m_weights.end(),[](const float &weight) {
			return (weight != 1.f) ? true : false;
		});
		if(it != m_weights.end())
			m_bDefaultBoneWeights = false;
	}

	auto animIndexCount = stdSeqDesc.groupsize[0] *stdSeqDesc.groupsize[1];
	if(animIndexCount > 0 && stdSeqDesc.animindexindex != 0)
	{
		f->Seek(offset +stdSeqDesc.animindexindex);
		m_animations.resize(animIndexCount);
		f->Read(m_animations.data(),sizeof(m_animations.front()) *animIndexCount);
	}

	f->Seek(offset +sizeof(mdl::mstudioseqdesc_t));
}

const std::string &import::mdl::Sequence::GetName() const {return m_name;}
const std::string &import::mdl::Sequence::GetActivityName() const {return m_activityName;}
const import::mdl::Sequence::PoseParameter &import::mdl::Sequence::GetPoseParameter() const {return m_poseParam;}
const std::vector<int16_t> &import::mdl::Sequence::GetAnimationIndices() const {return m_animations;}
const std::vector<float> &import::mdl::Sequence::GetWeights() const {return m_weights;}
bool import::mdl::Sequence::IsUsingDefaultBoneWeights() const {return m_bDefaultBoneWeights;}
int32_t import::mdl::Sequence::GetActivity() const {return m_activity;}
int32_t import::mdl::Sequence::GetActivityWeight() const {return m_activityWeight;}
int32_t import::mdl::Sequence::GetFlags() const {return m_flags;}
float import::mdl::Sequence::GetFadeInTime() const {return m_fadeInTime;}
float import::mdl::Sequence::GetFadeOutTime() const {return m_fadeOutTime;}
const Vector3 &import::mdl::Sequence::GetMin() const {return m_min;}
const Vector3 &import::mdl::Sequence::GetMax() const {return m_max;}
const std::vector<import::mdl::Sequence::Event> &import::mdl::Sequence::GetEvents() const {return m_events;}
