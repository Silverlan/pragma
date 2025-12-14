// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :audio.sound_script;

#undef CreateEvent

#pragma warning(disable : 4056)
pragma::audio::ALSoundScript::ALSoundScript(NetworkState *nw, unsigned int idx, SoundScript *script, NetworkState *state, bool bStream) : ALSound(nw), m_script(script), m_networkState(state), m_bStream(bStream) { m_index = idx; }
#pragma warning(default : 4056)

pragma::audio::ALSoundScript::~ALSoundScript()
{
	for(unsigned int i = 0; i < m_events.size(); i++)
		delete m_events[i];
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		delete m_sounds[i];
}

void pragma::audio::ALSoundScript::SetTargetPosition(unsigned int id, Vector3 pos)
{
	std::unordered_map<unsigned int, Vector3>::iterator it = m_positions.find(id);
	if(it != m_positions.end())
		it->second = pos;
	else
		m_positions.insert(std::unordered_map<unsigned int, Vector3>::value_type(id, pos));
	for(unsigned int i = 0; i < m_sounds.size(); i++) {
		SSESound *snd = m_sounds[i];
		SSEPlaySound *ev = static_cast<SSEPlaySound *>(snd->event);
		if(ev->position == CInt32(id)) {
			std::shared_ptr<ALSound> sound = snd->sound;
			sound->SetPosition(pos);
		}
	}
}

void pragma::audio::ALSoundScript::FadeIn(float time)
{
	//ALSound::FadeIn(time);
	float gain = GetGain();
	SetGain(0);
	CancelFade();
	if(!IsPlaying())
		Play();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(true, m_networkState->RealTime(), time, gain));
}

void pragma::audio::ALSoundScript::FadeOut(float time)
{
	//ALSound::FadeOut(time);
	if(!IsPlaying())
		return;
	float gain = GetGain();
	CancelFade();
	m_fade = std::unique_ptr<SoundFade>(new SoundFade(false, m_networkState->RealTime(), time, gain));
}

void pragma::audio::ALSoundScript::Initialize()
{
	//ALSound::Initialize();
}

std::shared_ptr<pragma::audio::ALSound> pragma::audio::ALSoundScript::CreateSound(const std::string &name, ALChannel channel, ALCreateFlags createFlags)
{
	auto flags = ALCreateFlags::None;
	if(channel == ALChannel::Mono)
		flags |= ALCreateFlags::Mono;
	if(m_bStream == true)
		flags |= ALCreateFlags::Stream;
	flags |= createFlags;
	return m_networkState->CreateSound(name, GetType(), flags);
}

void pragma::audio::ALSoundScript::InitializeEvent(SoundScriptEvent *ev)
{
	SSEPlaySound *ps = dynamic_cast<SSEPlaySound *>(ev);
	if(ps != nullptr) {
		SSESound *snd = ps->CreateSound(m_tPassed, [this](const std::string &name, ALChannel channel, ALCreateFlags createFlags) { return CreateSound(name, channel, createFlags); });
		if(snd != nullptr) {
			ALSound *als = snd->sound.get();
			auto bSetPos = false;
			if(ps->position != -1) {
				als->SetRelative(false);
				if(ps->position == -2) {
					Vector3 origin = GetPosition();
					Vector3 dir = uvec::create_random_unit_vector();
					dir.y = 0.f;
					uvec::normalize(&dir);
					dir *= 500.f;
					als->SetPosition(origin + dir);
					bSetPos = true;
				}
				else {
					std::unordered_map<unsigned int, Vector3>::iterator it = m_positions.find(ps->position);
					if(it != m_positions.end()) {
						bSetPos = true;
						als->SetPosition(it->second);
					}
				}
			}
			if(ps->position == 1)
				als->SetRelative(false);
			if(bSetPos == false) {
				auto pos = GetPosition();
				if(pos.x != 0.f || pos.y != 0.f || pos.z != 0.f)
					als->SetPosition(pos);
			}
			auto vel = GetVelocity();
			if(vel.x != 0.f || vel.y != 0.f || vel.z != 0.f)
				als->SetVelocity(vel);
			auto dir = GetDirection();
			if(dir.x != 0.f || dir.y != 0.f || dir.z != 0.f)
				als->SetDirection(dir);
			if(m_bRelative != ALSOUND_DEFAULT_RELATIVE)
				als->SetRelative(m_bRelative);
			if(m_refDist != ALSOUND_DEFAULT_REFERENCE_DISTANCE)
				als->SetReferenceDistance(m_refDist);
			if(m_rolloff.first != ALSOUND_DEFAULT_ROLLOFF_FACTOR)
				als->SetRolloffFactor(m_rolloff.first);
			if(m_rolloff.second != ALSOUND_DEFAULT_ROOM_ROLLOFF_FACTOR)
				als->SetRoomRolloffFactor(m_rolloff.second);
			if(m_maxDist != ALSOUND_DEFAULT_MAX_DISTANCE)
				als->SetMaxDistance(static_cast<float>(m_maxDist));
			if(m_minGain != ALSOUND_DEFAULT_MIN_GAIN)
				als->SetMinGain(ALSOUND_DEFAULT_MIN_GAIN);
			if(m_maxGain != ALSOUND_DEFAULT_MAX_GAIN)
				als->SetMaxGain(ALSOUND_DEFAULT_MAX_GAIN);
			if(m_coneInnerAngle != ALSOUND_DEFAULT_CONE_INNER_ANGLE)
				als->SetInnerConeAngle(m_coneInnerAngle);
			if(m_coneOuterAngle != ALSOUND_DEFAULT_CONE_OUTER_ANGLE)
				als->SetOuterConeAngle(m_coneOuterAngle);
			if(m_coneOuterGain.first != ALSOUND_DEFAULT_CONE_OUTER_GAIN)
				als->SetOuterConeGain(m_coneOuterGain.first);
			if(m_coneOuterGain.second != ALSOUND_DEFAULT_CONE_OUTER_GAIN_HF)
				als->SetOuterConeGainHF(m_coneOuterGain.second);
			if(m_priority != ALSOUND_DEFAULT_PRIORITY)
				als->SetPriority(m_priority);
			if(m_dopplerFactor != ALSOUND_DEFAULT_DOPPLER_FACTOR)
				als->SetDopplerFactor(m_dopplerFactor);
			if(m_airAbsorptionFactor != ALSOUND_DEFAULT_AIR_ABSORPTION_FACTOR)
				als->SetAirAbsorptionFactor(m_airAbsorptionFactor);
			if(m_orientation.first != ALSOUND_DEFAULT_ORIENTATION_AT || m_orientation.second != ALSOUND_DEFAULT_ORIENTATION_UP)
				als->SetOrientation(m_orientation.first, m_orientation.second);
			if(!pragma::math::equals<float>(m_stereoAngles.first, ALSOUND_DEFAULT_STEREO_ANGLE_LEFT))
				als->SetLeftStereoAngle(m_stereoAngles.first);
			if(!pragma::math::equals<float>(m_stereoAngles.second, ALSOUND_DEFAULT_STEREO_ANGLE_RIGHT))
				als->SetRightStereoAngle(m_stereoAngles.second);
			if(std::get<0>(m_gainAuto) != ALSOUND_DEFAULT_DIRECT_GAIN_HF_AUTO || std::get<1>(m_gainAuto) != ALSOUND_DEFAULT_SEND_GAIN_AUTO || std::get<2>(m_gainAuto) != ALSOUND_DEFAULT_SEND_GAIN_HF_AUTO)
				als->SetGainAuto(std::get<0>(m_gainAuto), std::get<1>(m_gainAuto), std::get<2>(m_gainAuto));
			if(m_directFilter.gain != ALSOUND_DEFAULT_DIRECT_FILTER_GAIN || m_directFilter.gainHF != ALSOUND_DEFAULT_DIRECT_FILTER_GAIN_HF || m_directFilter.gainLF != ALSOUND_DEFAULT_DIRECT_FILTER_GAIN_LF)
				als->SetDirectFilter(m_directFilter);
			als->SetSource(GetSource());
			float dur = als->GetDuration();
			if(dur > m_duration)
				m_duration = dur;
			m_sounds.push_back(snd);
			als->SetGain(als->GetGain() * GetGain());
			als->SetPitch(als->GetPitch() * GetPitch());
			CallCallbacks<void, ALSound *>("OnSoundCreated", als);
			als->Play();

			SetType(GetType() | als->GetType());
		}
	}
	else {
		//SSELua *lua = dynamic_cast<SSELua*>(ev); // Currently not implemented
		//if(lua != nullptr)
		//	;
		//else
		m_events.push_back(ev->CreateEvent(m_tPassed));
	}
}

bool pragma::audio::ALSoundScript::HandleEvents(SoundScriptEvent *ev, float eventOffset, float lastOffset, float newOffset)
{
	if(eventOffset <= lastOffset || eventOffset > newOffset)
		return false;
	auto &events = ev->GetEvents();
	for(unsigned int i = 0; i < events.size(); i++)
		InitializeEvent(events[i].get());
	return true;
}

void pragma::audio::ALSoundScript::SetState(ALState state)
{
	auto old = GetState();
	if(state != old)
		CallCallbacks<void, ALState, ALState>("OnStateChanged", old, state);
	ALSoundBase::SetState(state);
}
pragma::audio::ALState pragma::audio::ALSoundScript::GetState() const { return ALSoundBase::GetState(); }

uint32_t pragma::audio::ALSoundScript::GetSoundCount() const { return m_sounds.size(); }
pragma::audio::ALSound *pragma::audio::ALSoundScript::GetSound(uint32_t idx) { return (idx < m_sounds.size()) ? m_sounds.at(idx)->sound.get() : nullptr; }

bool pragma::audio::ALSoundScript::IsSoundScript() const { return true; }

void pragma::audio::ALSoundScript::Update()
{
	//ALSound::Update();
	auto old = GetState();
	UpdateState();
	if(GetState() == ALState::Stopped) {
		CancelFade();
		CheckStateChange(old);
	}
	else if(m_fade) {
		double t = m_networkState->RealTime() - m_fade->start;
		if(t >= m_fade->duration)
			CancelFade();
		else {
			float gain = CFloat(t / m_fade->duration) * m_fade->gain;
			if(m_fade->fadein)
				SetGain(gain);
			else
				SetGain(m_fade->gain - gain);
		}
	}
	if(GetState() == ALState::Playing) {
		double t = m_networkState->RealTime();
		double tDelta = t - m_tLastUpdate;
		tDelta *= GetPitch() / 1.f;
		float dur = GetDuration();
		m_offset += CFloat(tDelta) / dur;
		//float secOffset = GetSecOffset();
		if(m_offset > 1.0) {
			if(m_sounds.empty() && m_events.empty()) {
				m_offset = 1.0;
				m_tLastUpdate = t;
				SetState(ALState::Stopped);
			}
			else if(IsLooping())
				m_offset -= std::floor(m_offset);
			else
				m_offset = 1.0;
		}
	}
}

void pragma::audio::ALSoundScript::PostUpdate()
{
	//double tLastUpdate = m_tLastUpdate;
	double tPassed = m_tPassed;
	if(GetState() == ALState::Playing) {
		double t = m_networkState->RealTime();
		double tDelta = t - m_tLastUpdate;
		tDelta *= GetPitch() / 1.f;
		m_tLastUpdate = t;
		m_tPassed += tDelta;
	}
	auto numSounds = m_sounds.size();
	//double t = m_networkState->RealTime();
	for(size_t i = 0; i < numSounds; i++) {
		SSESound *snd = m_sounds[i];
		if((*snd)->IsPlaying())
			HandleEvents(snd->event, snd->eventOffset, (*snd)->GetLastTimeOffset(), (*snd)->GetTimeOffset());
		else if(!(*snd)->IsPaused() && !(*snd)->IsLooping()) {
			SoundScriptEvent *ev = snd->event;
			bool bRepeat = ev->repeat;
			delete snd;
			m_sounds.erase(m_sounds.begin() + i);
			numSounds--;
			i--;
			if(bRepeat == true)
				InitializeEvent(ev);
		}
	}
	if(numSounds == 0 && GetState() == ALState::Playing)
		SetState(ALState::Stopped);
	auto numEvents = m_events.size();
	for(size_t i = 0; i < numEvents; i++) {
		SSEBase *sse = m_events[i];
		if(HandleEvents(sse->event, CFloat(sse->timeCreated + sse->eventOffset), CFloat(tPassed), CFloat(m_tPassed)) == true) {
			SoundScriptEvent *ev = sse->event;
			bool bRepeat = ev->repeat;
			delete sse;
			m_events.erase(m_events.begin() + i);
			numEvents--;
			i--;
			if(bRepeat == true)
				InitializeEvent(ev);
		}
	}
	ALSound::PostUpdate();
}

void pragma::audio::ALSoundScript::Play()
{
	auto state = GetState();
	if(state == ALState::Initial) {
		auto &events = m_script->GetEvents();
		for(unsigned int i = 0; i < events.size(); i++)
			InitializeEvent(events[i].get());
	}
	if(state != ALState::Paused)
		ALSoundBase::SetOffset(0.f);
	SetState(ALState::Playing);
	m_tLastUpdate = m_networkState->RealTime();
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->Play();
}
void pragma::audio::ALSoundScript::Stop()
{
	SetState(ALState::Stopped);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->Stop();
}
void pragma::audio::ALSoundScript::Pause()
{
	SetState(ALState::Paused);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->Pause();
}
void pragma::audio::ALSoundScript::Rewind()
{
	ALSoundBase::SetOffset(0.f);
	SetState(ALState::Initial);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->Rewind();
}
void pragma::audio::ALSoundScript::SetOffset(float offset)
{
	offset = std::min(offset, 1.f);
	ALSoundBase::SetOffset(offset);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetOffset(offset);
}
float pragma::audio::ALSoundScript::GetOffset() const { return ALSoundBase::GetOffset(); }
void pragma::audio::ALSoundScript::SetPitch(float pitch)
{
	float pitchPrev = GetPitch();
	ALSoundBase::SetPitch(pitch);
	for(unsigned int i = 0; i < m_sounds.size(); i++) {
		ALSound *als = m_sounds[i]->sound.get();
		if(pitchPrev == 0.f)
			als->SetPitch(pitch);
		else
			als->SetPitch((als->GetPitch() / pitchPrev) * pitch);
	}
}
float pragma::audio::ALSoundScript::GetPitch() const { return ALSoundBase::GetPitch(); }
void pragma::audio::ALSoundScript::SetLooping(bool loop)
{
	ALSoundBase::SetLooping(loop);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetLooping(loop);
}
bool pragma::audio::ALSoundScript::IsLooping() const
{
	if(ALSoundBase::IsLooping() == true)
		return true;
	// Return true if one of our child-sounds is looping
	for(auto *snd : m_sounds) {
		if(snd->sound != nullptr && snd->sound->IsLooping() == true)
			return true;
	}
	return false;
}
bool pragma::audio::ALSoundScript::IsPlaying() const { return ALSoundBase::IsPlaying(); }
bool pragma::audio::ALSoundScript::IsPaused() const { return ALSoundBase::IsPaused(); }
bool pragma::audio::ALSoundScript::IsStopped() const { return ALSoundBase::IsStopped(); }
void pragma::audio::ALSoundScript::SetGain(float gain)
{
	float gainPrev = GetGain();
	ALSoundBase::SetGain(gain);
	for(unsigned int i = 0; i < m_sounds.size(); i++) {
		ALSound *als = m_sounds[i]->sound.get();
		if(gainPrev == 0.f)
			als->SetGain(gain);
		else
			als->SetGain((als->GetGain() / gainPrev) * gain);
	}
}
float pragma::audio::ALSoundScript::GetGain() const { return ALSoundBase::GetGain(); }
void pragma::audio::ALSoundScript::SetPosition(const Vector3 &pos)
{
	ALSoundBase::SetPosition(pos);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetPosition(pos);
}
Vector3 pragma::audio::ALSoundScript::GetPosition() const { return ALSoundBase::GetPosition(); }
void pragma::audio::ALSoundScript::SetVelocity(const Vector3 &vel)
{
	ALSoundBase::SetVelocity(vel);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetVelocity(vel);
}
Vector3 pragma::audio::ALSoundScript::GetVelocity() const { return ALSoundBase::GetVelocity(); }
void pragma::audio::ALSoundScript::SetDirection(const Vector3 &dir)
{
	ALSoundBase::SetDirection(dir);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetDirection(dir);
}
Vector3 pragma::audio::ALSoundScript::GetDirection() const { return ALSoundBase::GetDirection(); }
void pragma::audio::ALSoundScript::SetRelative(bool b)
{
	ALSoundBase::SetRelative(b);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetRelative(b);
}
bool pragma::audio::ALSoundScript::IsRelative() const { return ALSoundBase::IsRelative(); }
void pragma::audio::ALSoundScript::SetTimeOffset(float sec)
{
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetTimeOffset(sec);
}
float pragma::audio::ALSoundScript::GetTimeOffset() const
{
	if(m_sounds.empty())
		return 0.f; //ALSound::GetTimeOffset();
	return (*m_sounds.front())->GetTimeOffset();
}
float pragma::audio::ALSoundScript::GetDuration() const { return ALSoundBase::GetDuration(); }
float pragma::audio::ALSoundScript::GetReferenceDistance() const { return ALSoundBase::GetReferenceDistance(); }
void pragma::audio::ALSoundScript::SetReferenceDistance(float dist)
{
	ALSoundBase::SetReferenceDistance(dist);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetReferenceDistance(dist);
}
void pragma::audio::ALSoundScript::SetRoomRolloffFactor(float roomFactor)
{
	ALSoundBase::SetRoomRolloffFactor(roomFactor);
	for(auto *snd : m_sounds)
		(*snd)->SetRoomRolloffFactor(roomFactor);
}
float pragma::audio::ALSoundScript::GetRolloffFactor() const { return ALSoundBase::GetRolloffFactor(); }
void pragma::audio::ALSoundScript::SetRolloffFactor(float factor)
{
	ALSoundBase::SetRolloffFactor(factor);
	for(auto *snd : m_sounds)
		(*snd)->SetRolloffFactor(factor);
}
float pragma::audio::ALSoundScript::GetRoomRolloffFactor() const { return ALSoundBase::GetRoomRolloffFactor(); }
float pragma::audio::ALSoundScript::GetMaxDistance() const { return ALSoundBase::GetMaxDistance(); }
void pragma::audio::ALSoundScript::SetMaxDistance(float dist)
{
	ALSoundBase::SetMaxDistance(dist);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetMaxDistance(dist);
}
float pragma::audio::ALSoundScript::GetMinGain() const { return ALSoundBase::GetMinGain(); }
void pragma::audio::ALSoundScript::SetMinGain(float gain)
{
	ALSoundBase::SetMinGain(gain);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetMinGain(gain);
}
float pragma::audio::ALSoundScript::GetMaxGain() const { return ALSoundBase::GetMaxGain(); }
void pragma::audio::ALSoundScript::SetMaxGain(float gain)
{
	ALSoundBase::SetMaxGain(gain);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetMaxGain(gain);
}
float pragma::audio::ALSoundScript::GetInnerConeAngle() const { return ALSoundBase::GetInnerConeAngle(); }
void pragma::audio::ALSoundScript::SetInnerConeAngle(float ang)
{
	ALSoundBase::SetInnerConeAngle(ang);
	for(auto *snd : m_sounds)
		(*snd)->SetInnerConeAngle(ang);
}
float pragma::audio::ALSoundScript::GetOuterConeAngle() const { return ALSoundBase::GetOuterConeAngle(); }
void pragma::audio::ALSoundScript::SetOuterConeAngle(float ang)
{
	ALSoundBase::SetOuterConeAngle(ang);
	for(auto *snd : m_sounds)
		(*snd)->SetOuterConeAngle(ang);
}
float pragma::audio::ALSoundScript::GetOuterConeGain() const { return ALSoundBase::GetOuterConeGain(); }
float pragma::audio::ALSoundScript::GetOuterConeGainHF() const { return ALSoundBase::GetOuterConeGainHF(); }
void pragma::audio::ALSoundScript::SetOuterConeGain(float gain)
{
	ALSoundBase::SetOuterConeGain(gain);
	for(auto *snd : m_sounds)
		(*snd)->SetOuterConeGain(gain);
}
void pragma::audio::ALSoundScript::SetOuterConeGainHF(float gain)
{
	ALSoundBase::SetOuterConeGainHF(gain);
	for(auto *snd : m_sounds)
		(*snd)->SetOuterConeGainHF(gain);
}

void pragma::audio::ALSoundScript::SetFlags(unsigned int flags)
{
	ALSound::SetFlags(flags);
	for(unsigned int i = 0; i < m_sounds.size(); i++)
		(*m_sounds[i])->SetFlags(flags);
}
uint32_t pragma::audio::ALSoundScript::GetPriority() { return ALSoundBase::GetPriority(); }
void pragma::audio::ALSoundScript::SetPriority(uint32_t priority)
{
	ALSoundBase::SetPriority(priority);
	for(auto *snd : m_sounds)
		(*snd)->SetPriority(priority);
}
void pragma::audio::ALSoundScript::SetOrientation(const Vector3 &at, const Vector3 &up)
{
	ALSoundBase::SetOrientation(at, up);
	for(auto *snd : m_sounds)
		(*snd)->SetOrientation(at, up);
}
std::pair<Vector3, Vector3> pragma::audio::ALSoundScript::GetOrientation() const
{
	return ALSoundBase::GetOrientation();
	;
}
void pragma::audio::ALSoundScript::SetDopplerFactor(float factor)
{
	ALSoundBase::SetDopplerFactor(factor);
	for(auto *snd : m_sounds)
		(*snd)->SetDopplerFactor(factor);
}
float pragma::audio::ALSoundScript::GetDopplerFactor() const { return ALSoundBase::GetDopplerFactor(); }
void pragma::audio::ALSoundScript::SetLeftStereoAngle(float ang)
{
	ALSoundBase::SetLeftStereoAngle(ang);
	for(auto *snd : m_sounds)
		(*snd)->SetLeftStereoAngle(ang);
}
float pragma::audio::ALSoundScript::GetLeftStereoAngle() const { return ALSoundBase::GetLeftStereoAngle(); }
void pragma::audio::ALSoundScript::SetRightStereoAngle(float ang)
{
	ALSoundBase::SetRightStereoAngle(ang);
	for(auto *snd : m_sounds)
		(*snd)->SetRightStereoAngle(ang);
}
float pragma::audio::ALSoundScript::GetRightStereoAngle() const { return ALSoundBase::GetRightStereoAngle(); }
void pragma::audio::ALSoundScript::SetAirAbsorptionFactor(float factor)
{
	ALSoundBase::SetAirAbsorptionFactor(factor);
	for(auto *snd : m_sounds)
		(*snd)->SetAirAbsorptionFactor(factor);
}
float pragma::audio::ALSoundScript::GetAirAbsorptionFactor() const { return ALSoundBase::GetAirAbsorptionFactor(); }
void pragma::audio::ALSoundScript::SetGainAuto(bool directHF, bool send, bool sendHF)
{
	ALSoundBase::SetGainAuto(directHF, send, sendHF);
	for(auto *snd : m_sounds)
		(*snd)->SetGainAuto(directHF, send, sendHF);
}
std::tuple<bool, bool, bool> pragma::audio::ALSoundScript::GetGainAuto() const { return ALSoundBase::GetGainAuto(); }
void pragma::audio::ALSoundScript::SetDirectFilter(const SoundEffectParams &params)
{
	ALSoundBase::SetDirectFilter(params);
	for(auto *snd : m_sounds)
		(*snd)->SetDirectFilter(params);
}
const pragma::audio::SoundEffectParams &pragma::audio::ALSoundScript::GetDirectFilter() const { return ALSoundBase::GetDirectFilter(); }

bool pragma::audio::ALSoundScript::AddEffect(const std::string &effectName, const SoundEffectParams &params)
{
	for(auto *snd : m_sounds)
		(*snd)->AddEffect(effectName, params);
	return true;
}
void pragma::audio::ALSoundScript::RemoveEffect(const std::string &effectName)
{
	for(auto *snd : m_sounds)
		(*snd)->RemoveEffect(effectName);
}
void pragma::audio::ALSoundScript::SetEffectParameters(const std::string &effectName, const SoundEffectParams &params)
{
	for(auto *snd : m_sounds)
		(*snd)->SetEffectParameters(effectName, params);
}
