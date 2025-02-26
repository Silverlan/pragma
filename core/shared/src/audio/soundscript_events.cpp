/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/audio/soundscript.h"
#include "pragma/audio/soundscript_events.h"
#include "pragma/audio/alsound_type.h"
#include <sharedutils/util_string.h>
#include <udm.hpp>

#undef CreateEvent

extern DLLNETWORK Engine *engine;

SoundScriptEventContainer::~SoundScriptEventContainer() { m_events.clear(); }
SoundScriptEventContainer::SoundScriptEventContainer(SoundScriptManager *manager) : m_manager(manager) {}
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
std::vector<std::shared_ptr<SoundScriptEvent>> &SoundScriptEventContainer::GetEvents() { return m_events; }
void SoundScriptEventContainer::InitializeEvents(udm::LinkedPropertyWrapper &prop)
{
	for(auto udmEvent : prop["events"]) {
		std::string type;
		udmEvent["type"](type);
		auto *ev = CreateEvent(type);
		if(ev == nullptr)
			continue;
		udmEvent["repeat"](ev->repeat);
		auto propTime = udmEvent["time"];
		ev->eventOffset = SoundScriptValue {propTime};
		ev->Initialize(udmEvent);
	}
}
void SoundScriptEventContainer::PrecacheSounds()
{
	for(auto &ev : m_events)
		ev->Precache();
}

//////////////////////////////

SSEBase::SSEBase(SoundScriptEvent *ev, double tStart, float evOffset) : event(ev), timeCreated(tStart), eventOffset(evOffset) {}

SSESound::SSESound(std::shared_ptr<ALSound> snd, SSEPlaySound *ev, double tStart, float eventOffset) : SSEBase(ev, tStart, eventOffset), sound(snd) {}

ALSound *SSESound::operator->() { return sound.get(); }

//////////////////////////////

SoundScriptEvent::SoundScriptEvent(SoundScriptManager *manager, float off, bool bRepeat) : SoundScriptEventContainer(manager), eventOffset(off), repeat(bRepeat) {}
SoundScriptEvent::~SoundScriptEvent() {}
void SoundScriptEvent::Initialize(udm::LinkedPropertyWrapper &prop) { InitializeEvents(prop); }
void SoundScriptEvent::Precache()
{
	for(auto &ev : m_events)
		ev->Precache();
}
SSEBase *SoundScriptEvent::CreateEvent(double tStart) { return new SSEBase(this, tStart, eventOffset.GetValue()); }

//////////////////////////////

ALChannel SSEPlaySound::GetChannel()
{
	auto channel = mode;
	if(position != -1)
		channel = ALChannel::Mono;
	return channel;
}

SSESound *SSEPlaySound::CreateSound(double tStart, const std::function<std::shared_ptr<ALSound>(const std::string &, ALChannel, ALCreateFlags)> &createSound)
{
	int numSounds = static_cast<int>(sources.size());
	if(numSounds == 0)
		return NULL;
	unsigned int r = umath::random(0, numSounds - 1);
	auto createFlags = ALCreateFlags::None;
	if(stream == true)
		createFlags |= ALCreateFlags::Stream;
	std::shared_ptr<ALSound> snd = createSound(sources[r].c_str(), GetChannel(), createFlags);
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
	if(global) {
		snd->SetRelative(true);
		snd->SetPosition({});
	}
	if(type.IsSet() == true)
		snd->SetType(static_cast<ALSoundType>(umath::to_integral(snd->GetType()) | static_cast<uint32_t>(type.GetValue())));

	if(startTime.IsSet() || endTime.IsSet()) {
		auto start = (startTime.IsSet()) ? startTime.GetValue() : 0.f;
		auto end = (endTime.IsSet()) ? endTime.GetValue() : snd->GetDuration();
		snd->SetRange(start, end);
	}
	if(fadeInTime.IsSet())
		snd->SetFadeInDuration(fadeInTime.GetValue());
	if(fadeOutTime.IsSet())
		snd->SetFadeOutDuration(fadeOutTime.GetValue());
	return new SSESound(snd, this, tStart, eventOffset.GetValue());
}

void SSEPlaySound::Precache()
{
	SoundScriptEvent::Precache();
	for(unsigned int i = 0; i < sources.size(); i++)
		PrecacheSound(sources[i].c_str());
}

void SSEPlaySound::PrecacheSound(const char *name) { engine->GetServerNetworkState()->PrecacheSound(name, GetChannel()); }
void SSEPlaySound::Initialize(udm::LinkedPropertyWrapper &prop)
{
	SoundScriptEvent::Initialize(prop);
	auto udmSources = prop["source"];
	if(udmSources) {
		udmSources.GetBlobData(sources);

		std::string source;
		udmSources(source);
		if(!source.empty())
			sources.push_back(source);
	}
	prop["loop"](loop);
	prop["global"](global);
	prop["stream"](stream);

	mode = ALChannel::Auto;
	std::string strMode;
	prop["mode"](strMode);
	if(strMode == "mono")
		mode = ALChannel::Mono;
	else if(strMode == "both")
		mode = ALChannel::Both;

	position = -1;
	std::string strPos;
	prop["position"](strPos);
	if(strPos == "random") {
		position = -2;
		if(mode == ALChannel::Auto)
			mode = ALChannel::Mono;
	}
	if(position == -1 && prop["position"]) {
		if(mode == ALChannel::Auto)
			mode = ALChannel::Mono;
		prop["position"](position);
		if(position < 0)
			position = -1;
	}
	if(mode == ALChannel::Mono) {
		maxDistance = 2'048.f;
		referenceDistance = 64.f;
	}
	prop["max_distance"](maxDistance);
	gain.Load(prop["gain"]);
	pitch.Load(prop["pitch"]);
	offset.Load(prop["offset"]);
	referenceDistance.Load(prop["reference_distance"]);
	rolloffFactor.Load(prop["rolloff_factor"]);
	minGain.Load(prop["mingain"]);
	maxGain.Load(prop["maxgain"]);
	coneInnerAngle.Load(prop["cone_inner_angle"]);
	coneOuterAngle.Load(prop["cone_outer_angle"]);
	coneOuterGain.Load(prop["cone_outer_gain"]);
	auto udmType = prop["sound_type"];
	if(udmType.IsType(udm::Type::String)) {
		type = umath::to_integral(udm::string_to_flags<ALSoundType>(udmType, ALSoundType::Generic));
		type.SetSet(true);
	}
	else
		type.Load(prop["sound_type"]);

	startTime.Load(prop["start"]);
	endTime.Load(prop["end"]);
	fadeInTime.Load(prop["fadein"]);
	fadeOutTime.Load(prop["fadeout"]);
}

//////////////////////////////

SoundScriptValue::SoundScriptValue(float f) { Initialize(f); }
SoundScriptValue::SoundScriptValue(float min, float max) { Initialize(min, max); }
SoundScriptValue::SoundScriptValue(udm::LinkedPropertyWrapper &prop)
{
	Initialize(0.f);
	Load(prop);
}
bool SoundScriptValue::Load(const udm::LinkedPropertyWrapper &prop)
{
	auto &udmVal = prop;
	if(udmVal["min"] && udmVal["max"]) {
		auto min = 0.f;
		auto max = 0.f;
		udmVal["min"](min);
		udmVal["max"](max);
		Initialize(min, max);
		m_bIsSet = true;
		return true;
	}
	auto val = udmVal.ToValue<udm::Float>();
	if(val.has_value()) {
		Initialize(*val);
		m_bIsSet = true;
		return true;
	}
	auto valVec = udmVal.ToValue<udm::Vector2>();
	if(valVec.has_value()) {
		Initialize(valVec->x, valVec->y);
		m_bIsSet = true;
		return true;
	}
	return false;
}
void SoundScriptValue::Initialize(float f)
{
	m_min = f;
	m_max = f;
}
void SoundScriptValue::Initialize(float min, float max)
{
	m_min = min;
	m_max = max;
}
float SoundScriptValue::GetValue() const { return umath::random(m_min, m_max); }
bool SoundScriptValue::IsSet() const { return m_bIsSet; }
