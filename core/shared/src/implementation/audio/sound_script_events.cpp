// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#undef CreateEvent

module pragma.shared;

import :audio.sound_script_events;

pragma::audio::SoundScriptEventContainer::~SoundScriptEventContainer() { m_events.clear(); }
pragma::audio::SoundScriptEventContainer::SoundScriptEventContainer(SoundScriptManager *manager) : m_manager(manager) {}
pragma::audio::SoundScriptEvent *pragma::audio::SoundScriptEventContainer::CreateEvent(std::string name)
{
	auto *ev = m_manager->CreateEvent(name);
	m_events.push_back(std::shared_ptr<SoundScriptEvent>(ev));
	return ev;
}
pragma::audio::SoundScriptEvent *pragma::audio::SoundScriptEventContainer::CreateEvent()
{
	auto *ev = m_manager->CreateEvent();
	m_events.push_back(std::shared_ptr<SoundScriptEvent>(ev));
	return ev;
}
std::vector<std::shared_ptr<pragma::audio::SoundScriptEvent>> &pragma::audio::SoundScriptEventContainer::GetEvents() { return m_events; }
void pragma::audio::SoundScriptEventContainer::InitializeEvents(udm::LinkedPropertyWrapper &prop)
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
void pragma::audio::SoundScriptEventContainer::PrecacheSounds()
{
	for(auto &ev : m_events)
		ev->Precache();
}

//////////////////////////////

pragma::audio::SSEBase::SSEBase(SoundScriptEvent *ev, double tStart, float evOffset) : event(ev), timeCreated(tStart), eventOffset(evOffset) {}

pragma::audio::SSESound::SSESound(std::shared_ptr<pragma::audio::ALSound> snd, SSEPlaySound *ev, double tStart, float eventOffset) : SSEBase(ev, tStart, eventOffset), sound(snd) {}

pragma::audio::ALSound *pragma::audio::SSESound::operator->() { return sound.get(); }

//////////////////////////////

pragma::audio::SoundScriptEvent::SoundScriptEvent(SoundScriptManager *manager, float off, bool bRepeat) : SoundScriptEventContainer(manager), eventOffset(off), repeat(bRepeat) {}
pragma::audio::SoundScriptEvent::~SoundScriptEvent() {}
void pragma::audio::SoundScriptEvent::Initialize(udm::LinkedPropertyWrapper &prop) { InitializeEvents(prop); }
void pragma::audio::SoundScriptEvent::Precache()
{
	for(auto &ev : m_events)
		ev->Precache();
}
pragma::audio::SSEBase *pragma::audio::SoundScriptEvent::CreateEvent(double tStart) { return new SSEBase(this, tStart, eventOffset.GetValue()); }

//////////////////////////////

pragma::audio::ALChannel pragma::audio::SSEPlaySound::GetChannel()
{
	auto channel = mode;
	if(position != -1)
		channel = pragma::audio::ALChannel::Mono;
	return channel;
}

pragma::audio::SSESound *pragma::audio::SSEPlaySound::CreateSound(double tStart, const std::function<std::shared_ptr<pragma::audio::ALSound>(const std::string &, ALChannel, pragma::audio::ALCreateFlags)> &createSound)
{
	int numSounds = static_cast<int>(sources.size());
	if(numSounds == 0)
		return nullptr;
	unsigned int r = pragma::math::random(0, numSounds - 1);
	auto createFlags = pragma::audio::ALCreateFlags::None;
	if(stream == true)
		createFlags |= pragma::audio::ALCreateFlags::Stream;
	std::shared_ptr<pragma::audio::ALSound> snd = createSound(sources[r].c_str(), GetChannel(), createFlags);
	if(snd.get() == nullptr)
		return nullptr;
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
		snd->SetType(static_cast<pragma::audio::ALSoundType>(pragma::math::to_integral(snd->GetType()) | static_cast<uint32_t>(type.GetValue())));

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

void pragma::audio::SSEPlaySound::Precache()
{
	pragma::audio::SoundScriptEvent::Precache();
	for(unsigned int i = 0; i < sources.size(); i++)
		PrecacheSound(sources[i].c_str());
}

void pragma::audio::SSEPlaySound::PrecacheSound(const char *name) { pragma::Engine::Get()->GetServerNetworkState()->PrecacheSound(name, GetChannel()); }
void pragma::audio::SSEPlaySound::Initialize(udm::LinkedPropertyWrapper &prop)
{
	pragma::audio::SoundScriptEvent::Initialize(prop);
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

	mode = pragma::audio::ALChannel::Auto;
	std::string strMode;
	prop["mode"](strMode);
	if(strMode == "mono")
		mode = pragma::audio::ALChannel::Mono;
	else if(strMode == "both")
		mode = pragma::audio::ALChannel::Both;

	position = -1;
	std::string strPos;
	prop["position"](strPos);
	if(strPos == "random") {
		position = -2;
		if(mode == pragma::audio::ALChannel::Auto)
			mode = pragma::audio::ALChannel::Mono;
	}
	if(position == -1 && prop["position"]) {
		if(mode == pragma::audio::ALChannel::Auto)
			mode = pragma::audio::ALChannel::Mono;
		prop["position"](position);
		if(position < 0)
			position = -1;
	}
	if(mode == pragma::audio::ALChannel::Mono) {
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
		type = pragma::math::to_integral(udm::string_to_flags<pragma::audio::ALSoundType>(udmType, pragma::audio::ALSoundType::Generic));
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

pragma::audio::SoundScriptValue::SoundScriptValue(float f) { Initialize(f); }
pragma::audio::SoundScriptValue::SoundScriptValue(float min, float max) { Initialize(min, max); }
pragma::audio::SoundScriptValue::SoundScriptValue(udm::LinkedPropertyWrapper &prop)
{
	Initialize(0.f);
	Load(prop);
}
bool pragma::audio::SoundScriptValue::Load(const udm::LinkedPropertyWrapper &prop)
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
void pragma::audio::SoundScriptValue::Initialize(float f)
{
	m_min = f;
	m_max = f;
}
void pragma::audio::SoundScriptValue::Initialize(float min, float max)
{
	m_min = min;
	m_max = max;
}
float pragma::audio::SoundScriptValue::GetValue() const { return pragma::math::random(m_min, m_max); }
bool pragma::audio::SoundScriptValue::IsSet() const { return m_bIsSet; }
