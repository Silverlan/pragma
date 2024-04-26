/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_flex_component.hpp"
#include "pragma/entities/components/c_eye_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/console/c_cvar.h"
#include "pragma/audio/c_alsound.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <stack>
#include <pragma/model/model.h>
#include <se_scene.hpp>
#include <alsound_buffer.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

extern DLLCLIENT CGame *c_game;

static auto cvFlexPhonemeDrag = GetClientConVar("cl_flex_phoneme_drag");

ComponentEventId CFlexComponent::EVENT_ON_FLEX_CONTROLLERS_UPDATED = INVALID_COMPONENT_ID;
void CFlexComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseFlexComponent::RegisterEvents(componentManager, registerEvent);
	EVENT_ON_FLEX_CONTROLLERS_UPDATED = registerEvent("ON_FLEX_CONTROLLERS_UPDATED", ComponentEventInfo::Type::Explicit);
}

void CFlexComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CFlexComponent::UpdateFlexControllers(float dt)
{
	// TODO: Update every frame!
	auto t = c_game->CurTime();
	MaintainFlexAnimations(dt);
	auto flexDrag = cvFlexPhonemeDrag->GetFloat();
	for(auto &pair : m_flexControllers) {
		auto &info = pair.second;
		if(info.endTime != 0.f && t >= info.endTime && info.targetValue != 0.f)
			info.targetValue = 0.f;
		auto newVal = umath::lerp(info.value, info.targetValue, umath::min(dt / flexDrag, 1.f));
		if(!m_flexDataUpdateRequired && umath::abs(newVal - info.value) > 0.001f)
			m_flexDataUpdateRequired = true;
		info.value = newVal;
	}
	InvokeEventCallbacks(EVENT_ON_FLEX_CONTROLLERS_UPDATED);
}

void CFlexComponent::SetFlexWeight(uint32_t flexIdx, float weight)
{
	if(flexIdx >= m_flexWeights.size() || m_flexWeights.at(flexIdx) == weight)
		return;
	m_flexWeights.at(flexIdx) = weight;
	m_updatedFlexWeights.at(flexIdx) = true;
	m_flexDataUpdateRequired = true;
}

void CFlexComponent::SetFlexWeightOverride(uint32_t flexId, float weight)
{
	if(flexId < m_flexOverrides.size()) {
		if(m_flexOverrides.at(flexId) != weight) {
			m_flexOverrides.at(flexId) = weight;
			m_flexDataUpdateRequired = true;
		}
		return;
	}
	auto mdl = GetEntity().GetModel();
	if(mdl == nullptr || flexId >= mdl->GetFlexCount())
		return;
	m_flexOverrides.resize(flexId + 1);
	m_flexOverrides.at(flexId) = weight;
	m_flexDataUpdateRequired = true;
}
void CFlexComponent::ClearFlexWeightOverride(uint32_t flexId)
{
	if(flexId >= m_flexOverrides.size() || m_flexOverrides.at(flexId).has_value() == false)
		return;
	m_flexOverrides.at(flexId) = {};
	m_flexDataUpdateRequired = true;
}
bool CFlexComponent::HasFlexWeightOverride(uint32_t flexId) const
{
	if(flexId >= m_flexOverrides.size())
		return false;
	return m_flexOverrides.at(flexId).has_value();
}

void CFlexComponent::UpdateEyeFlexes(Eyeball &eyeball, uint32_t eyeballIdx)
{
	// TODO: Move this code to eye component
	auto eyeC = GetEntity().GetComponent<CEyeComponent>();
	auto *eyeballData = eyeC.valid() ? eyeC->GetEyeballData(eyeballIdx) : nullptr;
	if(eyeballData == nullptr)
		return;
	auto &state = eyeballData->state;
	Vector3 headup {};
	Vector3 headforward {};
	Vector3 pos {};

	// Get weighted position of eyeball angles based on the "raiser", "neutral", and "lowerer" controls
	auto upperLid = 0.f;
	auto lowerLid = 0.f;
	std::array<int32_t, 3> upperIndices = {eyeball.upperLid.lowererFlexIndex, eyeball.upperLid.neutralFlexIndex, eyeball.upperLid.raiserFlexIndex};
	std::array<float, 3> upperValues = {eyeball.upperLid.lowererValue, eyeball.upperLid.neutralValue, eyeball.upperLid.raiserValue};
	std::array<int32_t, 3> lowererIndices = {eyeball.lowerLid.lowererFlexIndex, eyeball.lowerLid.neutralFlexIndex, eyeball.lowerLid.raiserFlexIndex};
	std::array<float, 3> lowererValues = {eyeball.lowerLid.lowererValue, eyeball.lowerLid.neutralValue, eyeball.lowerLid.raiserValue};
	for(auto i = decltype(upperIndices.size()) {0u}; i < upperIndices.size(); ++i)
		upperLid += GetFlexWeight(upperIndices[i]) * umath::asin(upperValues[i] / eyeball.radius);
	for(auto i = decltype(lowererIndices.size()) {0u}; i < lowererIndices.size(); ++i)
		lowerLid += GetFlexWeight(lowererIndices[i]) * umath::asin(lowererValues[i] / eyeball.radius);

	float sinupper, cosupper, sinlower, coslower;
	sinupper = umath::sin(upperLid);
	cosupper = umath::cos(upperLid);

	sinlower = umath::sin(lowerLid);
	coslower = umath::cos(lowerLid);

	// To head space
	headup = state.up;
	headforward = state.forward;

	// Upper lid
	pos = headup * (sinupper * eyeball.radius);
	pos = pos + (cosupper * eyeball.radius) * headforward;
	SetFlexWeight(eyeball.upperLid.lidFlexIndex, uvec::dot(pos, eyeball.up));

	// Lower lid
	pos = headup * (sinlower * eyeball.radius);
	pos = pos + (coslower * eyeball.radius) * headforward;
	SetFlexWeight(eyeball.lowerLid.lidFlexIndex, uvec::dot(pos, eyeball.up));
}

void CFlexComponent::UpdateEyeFlexes()
{
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto &eyeballs = mdl->GetEyeballs();
	for(auto eyeballIdx = decltype(eyeballs.size()) {0u}; eyeballIdx < eyeballs.size(); ++eyeballIdx)
		UpdateEyeFlexes(eyeballs.at(eyeballIdx), eyeballIdx);
}

void CFlexComponent::OnModelChanged(const std::shared_ptr<Model> &mdl)
{
	m_flexControllers.clear();
	m_flexWeights.clear();
	m_updatedFlexWeights.clear();
	m_flexOverrides.clear();
	if(mdl == nullptr)
		return;
	m_flexWeights.resize(mdl->GetFlexCount(), 0.f);
	m_updatedFlexWeights.resize(m_flexWeights.size(), false);
}

void CFlexComponent::Initialize()
{
	BaseFlexComponent::Initialize();
	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { OnModelChanged(static_cast<pragma::CEOnModelChanged &>(evData.get()).model); });
	// This has been moved to vertex animated component
	//BindEventUnhandled(CRenderComponent::EVENT_ON_UPDATE_RENDER_DATA_MT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
	//	UpdateFlexWeightsMT();
	//});
	OnModelChanged(GetEntity().GetModel());
	GetEntity().AddComponent<CVertexAnimatedComponent>();

	SetTickPolicy(TickPolicy::Always); // TODO
}

void CFlexComponent::OnTick(double dt) { UpdateFlexControllers(dt); }

void CFlexComponent::SetFlexController(uint32_t flexId, float val, float duration, bool clampToLimits)
{
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto *flexC = mdl->GetFlexController(flexId);
	if(flexC == nullptr)
		return;
	if(clampToLimits)
		val = umath::clamp(val, flexC->min, flexC->max);
	auto it = m_flexControllers.find(flexId);
	if(it == m_flexControllers.end())
		it = m_flexControllers.insert(std::make_pair(flexId, FlexControllerInfo {})).first;
	auto &flexInfo = it->second;
	flexInfo.targetValue = val;
	flexInfo.endTime = (duration > 0.f) ? c_game->CurTime() + duration : 0.f;
	//InitializeVertexAnimationBuffer();

	//if(m_vertexAnimationBuffer == nullptr) // prosper TODO
	//	InitializeVertexAnimationBuffer(); // prosper TODO
	if(AreFlexControllerUpdateListenersEnabled()) {
		CEOnFlexControllerChanged evData {flexId, flexInfo.targetValue};
		InvokeEventCallbacks(EVENT_ON_FLEX_CONTROLLER_CHANGED, evData);
	}
}
bool CFlexComponent::GetFlexController(uint32_t flexId, float &val) const
{
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr || mdl->GetFlexController(flexId) == nullptr)
		return false;
	auto it = m_flexControllers.find(flexId);
	if(it != m_flexControllers.end())
		val = it->second.value;
	else
		val = 0.f;
	return true;
}

void CFlexComponent::UpdateFlexWeightsMT()
{
	if(m_flexDataUpdateRequired == false)
		return;
	auto mdlC = static_cast<CModelComponent *>(GetEntity().GetModelComponent());
	if(mdlC == nullptr || mdlC->GetLOD() > 0)
		return;
	auto &mdl = mdlC->GetModel();
	if(mdl == nullptr)
		return;
	auto &flexes = mdl->GetFlexes();
	assert(flexes.size() == m_flexWeights.size());
	auto numFlexes = umath::min(flexes.size(), m_flexWeights.size());
	for(auto flexId = decltype(numFlexes) {0u}; flexId < numFlexes; ++flexId) {
		auto flexVal = 0.f;
		UpdateFlexWeight(flexId, flexVal);
		m_flexWeights.at(flexId) = flexVal;
	}
	// UpdateEyeFlexes();

	// Clear for next update
	std::fill(m_updatedFlexWeights.begin(), m_updatedFlexWeights.end(), false);
	m_flexDataUpdateRequired = false;
}

const std::vector<float> &CFlexComponent::GetFlexWeights() const { return m_flexWeights; }

float CFlexComponent::GetFlexWeight(uint32_t flexId) const
{
	auto weight = 0.f;
	GetFlexWeight(flexId, weight);
	return weight;
}
bool CFlexComponent::GetFlexWeight(uint32_t flexId, float &outWeight) const
{
	if(flexId >= m_flexWeights.size())
		return false;
	outWeight = m_flexWeights.at(flexId);
	return true;
}

bool CFlexComponent::CalcFlexValue(uint32_t flexId, float &val) const { return const_cast<CFlexComponent *>(this)->UpdateFlexWeight(flexId, val, false); }

bool CFlexComponent::UpdateFlexWeight(uint32_t flexId, float &val, bool storeInCache)
{
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr || flexId >= m_updatedFlexWeights.size())
		return false;
	if(m_updatedFlexWeights.at(flexId)) {
		val = m_flexWeights.at(flexId);
		return true;
	}
	auto *flex = mdl->GetFlex(flexId);
	if(flex == nullptr)
		return false;
	std::optional<float> weight {};
	if(flexId < m_flexOverrides.size() && m_flexOverrides.at(flexId).has_value())
		weight = m_flexOverrides.at(flexId);
	else {
		weight = mdl->CalcFlexWeight(
		  flexId,
		  [this](uint32_t fcIdx) -> std::optional<float> {
			  auto val = 0.f;
			  if(GetScaledFlexController(fcIdx, val) == false)
				  return {};
			  return val;
		  },
		  [this](uint32_t flexIdx) -> std::optional<float> { return m_updatedFlexWeights.at(flexIdx) ? m_flexWeights.at(flexIdx) : std::optional<float> {}; });
	}
	if(weight.has_value() == false)
		return false;
	val = *weight;
	if(storeInCache) {
		m_flexWeights.at(flexId) = val;
		m_updatedFlexWeights.at(flexId) = true;
	}
	return true;
}
void CFlexComponent::UpdateSoundPhonemes(CALSound &snd)
{
	al::impl::BufferBase *buf = snd->GetBuffer();
	if(buf == nullptr)
		buf = snd->GetDecoder();
	auto userData = (buf != nullptr) ? buf->GetUserData() : nullptr;
	if(userData == nullptr)
		return;
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto &phonemeMap = mdl->GetPhonemeMap();
	auto &phonemeData = *static_cast<se::SoundPhonemeData *>(userData.get());
	auto t = snd.GetTimeOffset();
	for(auto itWord = phonemeData.words.begin(); itWord != phonemeData.words.end(); ++itWord) {
		auto &word = *itWord;
		if(t < word.tStart || t >= word.tEnd)
			continue;
		for(auto itPhoneme = word.phonemes.begin(); itPhoneme != word.phonemes.end(); ++itPhoneme) {
			auto &phoneme = *itPhoneme;
			if(t < phoneme.tStart || t >= phoneme.tEnd)
				continue;
			auto it = phonemeMap.phonemes.find(phoneme.phoneme);
			if(it != phonemeMap.phonemes.end()) {
				// Find next phoneme to interpolate
				se::SoundPhonemeData::PhonemeData *nextPhoneme = nullptr;
				PhonemeInfo *nextPhonemeInfo = nullptr;
				auto itPhonemeNext = itPhoneme + 1;
				if(itPhonemeNext != word.phonemes.end())
					nextPhoneme = &(*itPhonemeNext);
				else {
					auto itWordNext = itWord + 1;
					if(itWordNext != phonemeData.words.end()) {
						auto &nextWord = *itWordNext;
						if(nextWord.phonemes.empty() == false)
							nextPhoneme = &nextWord.phonemes.front();
					}
				}
				if(nextPhoneme != nullptr && umath::abs(nextPhoneme->tStart - phoneme.tEnd) < 0.1f) {
					auto it = phonemeMap.phonemes.find(nextPhoneme->phoneme);
					if(it != phonemeMap.phonemes.end())
						nextPhonemeInfo = &it->second;
				}
				else
					nextPhoneme = nullptr;
				//

				auto weight = 1.f - (t - phoneme.tStart) / (phoneme.tEnd - phoneme.tStart);
				for(auto &pair : it->second.flexControllers) {
					auto id = 0u;
					if(mdl->GetFlexControllerId(pair.first, id) == false)
						continue;
					auto flexWeight = pair.second * weight;
					if(nextPhonemeInfo != nullptr) {
						auto it = nextPhonemeInfo->flexControllers.find(pair.first);
						if(it != nextPhonemeInfo->flexControllers.end())
							flexWeight = umath::lerp(pair.second, it->second, 1.f - weight); // Interpolate between current phoneme and next phoneme
					}
					auto curFlexWeight = 0.f;
					if(GetFlexController(id, curFlexWeight) == true)
						SetFlexController(id, flexWeight, 0.1f);
				}
			}
			break;
		}
	}
}
