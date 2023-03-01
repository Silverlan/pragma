/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/cinematic/choreographic_scene.hpp"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/entities/components/c_flex_component.hpp"
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT ClientState *client;

choreography::Scene::Scene() : uts::TimelineScene() {}
choreography::Channel::Channel(uts::TimelineScene &scene, const std::string &name) : uts::Channel(scene, name) {}
choreography::Event::Event(uts::Channel &channel) : uts::Event(channel) {}
CBaseEntity *choreography::Event::GetActor() const { return static_cast<CBaseEntity *>(m_actor.get()); }
void choreography::Event::SetActor(CBaseEntity &actor) { m_actor = actor.GetHandle(); }
void choreography::Event::ClearActor() { m_actor = EntityHandle {}; }

choreography::AudioEvent::AudioEvent(uts::Channel &channel, const std::string &snd) : Event(channel), m_soundSource(snd) {}
void choreography::AudioEvent::Initialize() {}
void choreography::AudioEvent::Reset()
{
	Event::Reset();
	m_sound = nullptr;
}
void choreography::AudioEvent::Start()
{
	Event::Start();
	m_sound = nullptr;
	auto *actor = GetActor();
	if(actor == nullptr)
		return;
	auto pSoundEmitterComponent = actor->GetComponent<pragma::CSoundEmitterComponent>();
	if(pSoundEmitterComponent.expired())
		return;
	m_sound = pSoundEmitterComponent->CreateSound(m_soundSource, ALSoundType::Voice);
	if(m_sound == nullptr)
		return;
	m_sound->Play();
}
choreography::AudioEvent::State choreography::AudioEvent::HandleTick(double t, double dt)
{
	if(m_sound == nullptr)
		return State::Complete;
	return m_sound->IsPlaying() ? State::Pending : State::Complete;
}

////////////////////////

choreography::FacialFlexEvent::FacialFlexEvent(uts::Channel &channel) : Event(channel) {}
void choreography::FacialFlexEvent::SetFlexControllerValues(const std::string &name, const std::vector<TimeValue> &values, const std::vector<TimeValue> *lrDistribution)
{
	auto &info = m_values.insert(std::make_pair(name, FlexControllerInfo {})).first->second;
	info.values = values;
	if(lrDistribution == nullptr)
		info.stereo = false;
	else {
		info.leftRightDistribution = *lrDistribution;
		info.stereo = true;
	}
}
bool choreography::FacialFlexEvent::GetInterpolatedValue(double t, const std::vector<TimeValue> &srcValues, float &v, bool bGetLastAvailable) const
{
	auto tmpIdx = 0u;
	auto valIdx = std::numeric_limits<uint32_t>::max();
	while(tmpIdx < srcValues.size() && t > srcValues.at(tmpIdx).time)
		valIdx = tmpIdx++;
	if(valIdx >= srcValues.size()) {
		if(bGetLastAvailable == false || srcValues.empty())
			return false;
		valIdx = srcValues.size() - 1u;
	}
	if(t < srcValues.at(valIdx).time)
		return false;
	auto &tv = srcValues.at(valIdx);
	v = tv.value;
	if(valIdx + 1u < srcValues.size()) {
		auto tvNext = srcValues.at(valIdx + 1u);
		v = umath::lerp(v, tvNext.value, (t - tv.time) / (tvNext.time - tv.time));
	}
	return true;
}
choreography::FacialFlexEvent::State choreography::FacialFlexEvent::HandleTick(double t, double dt)
{
	auto *actor = GetActor();
	auto pFlexComponent = (actor != nullptr) ? actor->GetComponent<pragma::CFlexComponent>() : pragma::ComponentHandle<pragma::CFlexComponent> {};
	if(pFlexComponent.valid()) {
		for(auto &pair : m_values) {
			auto v = 0.f;
			if(GetInterpolatedValue(t, pair.second.values, v) == true) {
				if(pair.second.stereo == false) {
					pFlexComponent->SetFlexController(pair.first, v, 0.1f);
					Con::cout << "Flex Controller: " << pair.first << " (" << v << ")" << Con::endl;
				}
				else {
					auto vDistribution = 0.f;
					if(GetInterpolatedValue(t, pair.second.leftRightDistribution, vDistribution, true) == true) {
						auto leftDistribution = umath::clamp(vDistribution / 0.5f, 0.f, 1.f);
						auto rightDistribution = umath::clamp((1.f - vDistribution) / 0.5f, 0.f, 1.f);
						pFlexComponent->SetFlexController("left_" + pair.first, v * leftDistribution, 0.1f);
						pFlexComponent->SetFlexController("right_" + pair.first, v * rightDistribution, 0.1f);
						Con::cout << "Flex Controller: " << ("left_" + pair.first) << " (" << (v * leftDistribution) << ")" << Con::endl;
						//Con::cout<<"Flex Controller: "<<("right_" +pair.first)<<" ("<<(v *rightDistribution)<<")"<<Con::endl;
					}
				}
			}
		}
	}
	return Event::HandleTick(t, dt);
}
void choreography::FacialFlexEvent::Initialize() {}
void choreography::FacialFlexEvent::Start()
{
	Event::Start();
	for(auto &pair : m_values)
		pair.second.m_lastValue = 0u;
}
void choreography::FacialFlexEvent::Reset()
{
	Event::Reset();
	for(auto &pair : m_values)
		pair.second.m_lastValue = std::numeric_limits<uint32_t>::max();
}
