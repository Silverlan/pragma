#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/audio/soundscript.h"
#include "pragma/audio/soundscript_events.h"
#include <sharedutils/util_string.h>

#undef CreateEvent

extern DLLENGINE Engine *engine;

SoundScriptEventContainer::~SoundScriptEventContainer()
{
	m_events.clear();
}
SoundScriptEventContainer::SoundScriptEventContainer(SoundScriptManager *manager)
	: m_manager(manager)
{}
SoundScriptEvent *SoundScriptEventContainer::CreateEvent(std::string name)
{
	auto *ev = m_manager->CreateEvent(name);
	m_events.push_back(std::shared_ptr<SoundScriptEvent>(ev));
	return ev;
}
SoundScriptEvent *SoundScriptEventContainer::CreateEvent()
{
	auto *ev = m_manager->CreateEvent();
	m_events.push_back(std::shared_ptr<SoundScriptEvent>(ev));
	return ev;
}
std::vector<std::shared_ptr<SoundScriptEvent>> &SoundScriptEventContainer::GetEvents() {return m_events;}
void SoundScriptEventContainer::InitializeEvents(const std::shared_ptr<ds::Block> &data)
{
	auto &playSound = data->GetValue("playsound");
	if(playSound != NULL)
	{
		auto it = ds::Iterator(*playSound);
		while(it.IsValid())
		{
			auto *block = it.get();
			if(block->IsBlock())
			{
				bool repeat = block->GetBool("repeat");
				SoundScriptEvent *ev = CreateEvent("playsound");
				ev->eventOffset = SoundScriptValue(std::static_pointer_cast<ds::Block>(block->shared_from_this()),"time");
				ev->repeat = repeat;
				ev->Initialize(std::static_pointer_cast<ds::Block>(block->shared_from_this()));
			}
			it++;
		}
	}
	auto &lua = data->GetValue("lua");
	if(lua != NULL)
	{
		auto it = ds::Iterator(*lua);
		while(it.IsValid())
		{
			auto *block = it.get();
			if(block->IsBlock())
			{
				std::string name = block->GetString("name");
				bool repeat = block->GetBool("repeat");
				SoundScriptEvent *ev = CreateEvent("lua");
				ev->eventOffset = SoundScriptValue(std::static_pointer_cast<ds::Block>(block->shared_from_this()),"time");
				ev->repeat = repeat;
				ev->Initialize(std::static_pointer_cast<ds::Block>(block->shared_from_this()));
			}
			it++;
		}
	}
	auto &ev = data->GetValue("event");
	if(ev != NULL)
	{
		auto it = ds::Iterator(*ev);
		while(it.IsValid())
		{
			auto *block = it.get();
			if(block->IsBlock())
			{
				bool repeat = block->GetBool("repeat");
				SoundScriptEvent *ev = CreateEvent();
				ev->eventOffset = SoundScriptValue(std::static_pointer_cast<ds::Block>(block->shared_from_this()),"time");
				ev->repeat = repeat;
				ev->Initialize(std::static_pointer_cast<ds::Block>(block->shared_from_this()));
			}
			it++;
		}
	}
}
void SoundScriptEventContainer::PrecacheSounds()
{
	for(auto &ev : m_events)
		ev->Precache();
}

//////////////////////////////

SSEBase::SSEBase(SoundScriptEvent *ev,double tStart,float evOffset)
	: event(ev),timeCreated(tStart),eventOffset(evOffset)
{}

SSESound::SSESound(std::shared_ptr<ALSound> snd,SSEPlaySound *ev,double tStart,float eventOffset)
	: SSEBase(ev,tStart,eventOffset),sound(snd)
{}

ALSound *SSESound::operator->() {return sound.get();}

//////////////////////////////

SoundScriptEvent::SoundScriptEvent(SoundScriptManager *manager,float off,bool bRepeat)
	: SoundScriptEventContainer(manager),eventOffset(off),repeat(bRepeat)
{}
SoundScriptEvent::~SoundScriptEvent() {}
void SoundScriptEvent::Initialize(const std::shared_ptr<ds::Block> &data)
{
	InitializeEvents(data);
}
void SoundScriptEvent::Precache()
{
	for(auto &ev : m_events)
		ev->Precache();
}
SSEBase *SoundScriptEvent::CreateEvent(double tStart) {return new SSEBase(this,tStart,eventOffset.GetValue());}

//////////////////////////////

ALChannel SSEPlaySound::GetChannel()
{
	auto channel = mode;
	if(position != -1)
		channel = ALChannel::Mono;
	return channel;
}

SSESound *SSEPlaySound::CreateSound(double tStart,const std::function<std::shared_ptr<ALSound>(const std::string&,ALChannel,ALCreateFlags)> &createSound)
{
	int numSounds = static_cast<int>(sources.size());
	if(numSounds == 0)
		return NULL;
	unsigned int r = umath::random(0,numSounds -1);
	auto createFlags = ALCreateFlags::None;
	if(stream == true)
		createFlags |= ALCreateFlags::Stream;
	std::shared_ptr<ALSound> snd = createSound(sources[r].c_str(),GetChannel(),createFlags);
	if(snd.get() == NULL)
		return NULL;
	snd->SetMaxDistance(static_cast<float>(maxDistance));
	snd->SetGain(gain.GetValue());
	snd->SetPitch(pitch.GetValue());
	snd->SetLooping(loop);
	snd->SetOffset(offset.GetValue());
	snd->SetReferenceDistance(referenceDistance.GetValue());
	snd->SetRolloffFactor(rolloffFactor.GetValue());
	snd->SetMinGain(minGain.GetValue());
	snd->SetMaxGain(maxGain.GetValue());
	snd->SetInnerConeAngle(coneInnerAngle.GetValue());
	snd->SetOuterConeAngle(coneOuterAngle.GetValue());
	snd->SetOuterConeGain(coneOuterGain.GetValue());
	if(global)
	{
		snd->SetRelative(true);
		snd->SetPosition({});
	}
	if(type.IsSet() == true)
		snd->SetType(static_cast<ALSoundType>(umath::to_integral(snd->GetType()) | static_cast<uint32_t>(type.GetValue())));

	if(startTime.IsSet() || endTime.IsSet())
	{
		auto start = (startTime.IsSet()) ? startTime.GetValue() : 0.f;
		auto end = (endTime.IsSet()) ? endTime.GetValue() : snd->GetDuration();
		snd->SetRange(start,end);
	}
	if(fadeInTime.IsSet())
		snd->SetFadeInDuration(fadeInTime.GetValue());
	if(fadeOutTime.IsSet())
		snd->SetFadeOutDuration(fadeOutTime.GetValue());
	return new SSESound(snd,this,tStart,eventOffset.GetValue());
}

void SSEPlaySound::Precache()
{
	SoundScriptEvent::Precache();
	for(unsigned int i=0;i<sources.size();i++)
		PrecacheSound(sources[i].c_str());
}

void SSEPlaySound::PrecacheSound(const char *name) {engine->GetServerNetworkState()->PrecacheSound(name,GetChannel());}
void SSEPlaySound::Initialize(const std::shared_ptr<ds::Block> &data)
{
	SoundScriptEvent::Initialize(data);
	auto &source = data->GetValue("source");
	if(source != NULL)
	{
		if(source->IsBlock())
		{
			std::string src;
			int id = 0;
			auto srcBlock = std::static_pointer_cast<ds::Block>(source);
			while(srcBlock->GetString(std::to_string(id),&src))
			{
				sources.push_back(src);
				id++;
			}
		}
		else if(!source->IsContainer())
			sources.push_back(static_cast<ds::Value*>(source.get())->GetString());
	}
	loop = data->GetBool("loop");
	global = data->GetBool("global");
	stream = data->GetBool("stream");

	mode = ALChannel::Auto;
	if(data->IsString("mode"))
	{
		auto val = data->GetString("mode");
		ustring::to_lower(val);
		if(val == "mono")
			mode = ALChannel::Mono;
		else if(val == "both")
			mode = ALChannel::Both;
	}

	position = -1;
	if(data->IsString("position"))
	{
		if(mode == ALChannel::Auto)
			mode = ALChannel::Mono;
		std::string pos = data->GetString("position");
		StringToLower(pos);
		if(pos == "random")
			position = -2;
	}
	if(position == -1 && data->GetInt("position",&position) && position < 0)
		position = -1;
	if(mode == ALChannel::Mono)
	{
		maxDistance = 2'048.f;
		referenceDistance = 64.f;
	}
	auto fMaxDistance = 0.f;
	if(data->GetFloat("max_distance",&fMaxDistance) == true)
		maxDistance = fMaxDistance;
	gain.Load(data,"gain");
	pitch.Load(data,"pitch");
	offset.Load(data,"offset");
	referenceDistance.Load(data,"reference_distance");
	rolloffFactor.Load(data,"rolloff_factor");
	minGain.Load(data,"mingain");
	maxGain.Load(data,"maxgain");
	coneInnerAngle.Load(data,"cone_inner_angle");
	coneOuterAngle.Load(data,"cone_outer_angle");
	coneOuterGain.Load(data,"cone_outer_gain");
	type.Load(data,"type");

	startTime.Load(data,"start");
	endTime.Load(data,"end");
	fadeInTime.Load(data,"fadein");
	fadeOutTime.Load(data,"fadeout");
}

//////////////////////////////

SoundScriptValue::SoundScriptValue(float f) {Initialize(f);}
SoundScriptValue::SoundScriptValue(std::string s) {Initialize(s);}
SoundScriptValue::SoundScriptValue(const std::shared_ptr<ds::Block> &data,const char *name)
{
	Initialize(0.f);
	Load(data,name);
}
bool SoundScriptValue::Load(const std::shared_ptr<ds::Block> &data,const char *name)
{
	if(data->IsString(name))
	{
		Initialize(data->GetString(name));
		m_bIsSet = true;
		return true;
	}
	else
	{
		auto &dataVal = data->GetValue(name);
		if(dataVal != NULL && dataVal->IsBlock())
		{
			auto block = std::static_pointer_cast<ds::Block>(dataVal);
			std::string vMin;
			if(block->GetString("0",&vMin))
			{
				std::string vMax;
				if(block->GetString("1",&vMax))
				{
					std::string blockVal = vMin;
					blockVal += ",";
					blockVal += vMax;
					Initialize(blockVal);
					m_bIsSet = true;
					return true;
				}
				Initialize(static_cast<float>(atof(vMin.c_str())));
				m_bIsSet = true;
				return true;
			}
			Initialize(0.f);
			m_bIsSet = true;
			return true;
		}
		float f;
		if(data->GetFloat(name,&f))
		{
			Initialize(f);
			m_bIsSet = true;
			return true;
		}
	}
	return false;
}
void SoundScriptValue::Initialize(float f)
{
	m_min = f;
	m_max = f;
}
void SoundScriptValue::Initialize(std::string s)
{
	Initialize(0.f);
	std::vector<std::string> exp;
	ustring::explode(s,",",exp);
	if(exp.empty())
		return;
	ustring::remove_whitespace(exp[0]);
	if(exp.size() == 1)
	{
		m_min = static_cast<float>(atof(exp[0].c_str()));
		m_max = m_min;
		return;
	}
	ustring::remove_whitespace(exp[1]);
	m_min = static_cast<float>(atof(exp[0].c_str()));
	m_max = static_cast<float>(atof(exp[1].c_str()));
}
float SoundScriptValue::GetValue() const {return umath::random(m_min,m_max);}
bool SoundScriptValue::IsSet() const {return m_bIsSet;}
