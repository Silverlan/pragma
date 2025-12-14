// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.flex;
import :game;
import se_script;

using namespace pragma;

static auto cvFlexPhonemeDrag = console::get_client_con_var("cl_flex_phoneme_drag");

ComponentEventId cFlexComponent::EVENT_ON_FLEX_CONTROLLERS_UPDATED = INVALID_COMPONENT_ID;
void CFlexComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	BaseFlexComponent::RegisterEvents(componentManager, registerEvent);
	cFlexComponent::EVENT_ON_FLEX_CONTROLLERS_UPDATED = registerEvent("ON_FLEX_CONTROLLERS_UPDATED", ComponentEventInfo::Type::Explicit);
}

void CFlexComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CFlexComponent::UpdateFlexControllers(float dt)
{
	// TODO: Update every frame!
	auto t = get_cgame()->CurTime();
	MaintainFlexAnimations(dt);
	auto flexDrag = cvFlexPhonemeDrag->GetFloat();
	for(auto &pair : m_flexControllers) {
		auto &info = pair.second;
		if(info.endTime != 0.f && t >= info.endTime && info.targetValue != 0.f)
			info.targetValue = 0.f;
		auto newVal = math::lerp(info.value, info.targetValue, math::min(dt / flexDrag, 1.f));
		if(!m_flexDataUpdateRequired && math::abs(newVal - info.value) > 0.001f)
			m_flexDataUpdateRequired = true;
		info.value = newVal;
	}
	InvokeEventCallbacks(cFlexComponent::EVENT_ON_FLEX_CONTROLLERS_UPDATED);
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

void CFlexComponent::UpdateEyeFlexes(asset::Eyeball &eyeball, uint32_t eyeballIdx)
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
		upperLid += GetFlexWeight(upperIndices[i]) * math::asin(upperValues[i] / eyeball.radius);
	for(auto i = decltype(lowererIndices.size()) {0u}; i < lowererIndices.size(); ++i)
		lowerLid += GetFlexWeight(lowererIndices[i]) * math::asin(lowererValues[i] / eyeball.radius);

	float sinupper, cosupper, sinlower, coslower;
	sinupper = math::sin(upperLid);
	cosupper = math::cos(upperLid);

	sinlower = math::sin(lowerLid);
	coslower = math::cos(lowerLid);

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

void CFlexComponent::OnModelChanged(const std::shared_ptr<asset::Model> &mdl)
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
	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { OnModelChanged(static_cast<CEOnModelChanged &>(evData.get()).model); });
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
		val = math::clamp(val, flexC->min, flexC->max);
	auto it = m_flexControllers.find(flexId);
	if(it == m_flexControllers.end())
		it = m_flexControllers.insert(std::make_pair(flexId, FlexControllerInfo {})).first;
	auto &flexInfo = it->second;
	flexInfo.targetValue = val;
	flexInfo.endTime = (duration > 0.f) ? get_cgame()->CurTime() + duration : 0.f;
	//InitializeVertexAnimationBuffer();

	//if(m_vertexAnimationBuffer == nullptr) // prosper TODO
	//	InitializeVertexAnimationBuffer(); // prosper TODO
	if(AreFlexControllerUpdateListenersEnabled()) {
		CEOnFlexControllerChanged evData {flexId, flexInfo.targetValue};
		InvokeEventCallbacks(cFlexComponent::EVENT_ON_FLEX_CONTROLLER_CHANGED, evData);
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
	auto numFlexes = math::min(flexes.size(), m_flexWeights.size());
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
void CFlexComponent::UpdateSoundPhonemes(audio::CALSound &snd)
{
	audio::impl::BufferBase *buf = snd->GetBuffer();
	if(buf == nullptr)
		buf = snd->GetDecoder();
	auto userData = (buf != nullptr) ? buf->GetUserData() : nullptr;
	if(userData == nullptr)
		return;
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto &phonemeMap = mdl->GetPhonemeMap();
	auto &phonemeData = *static_cast<source_engine::script::SoundPhonemeData *>(userData.get());
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
				source_engine::script::SoundPhonemeData::PhonemeData *nextPhoneme = nullptr;
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
				if(nextPhoneme != nullptr && math::abs(nextPhoneme->tStart - phoneme.tEnd) < 0.1f) {
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
							flexWeight = math::lerp(pair.second, it->second, 1.f - weight); // Interpolate between current phoneme and next phoneme
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

namespace Lua::Flex {
	std::optional<float> GetFlexController(CFlexComponent &hEnt, uint32_t flexId)
	{
		auto val = 0.f;
		if(hEnt.GetFlexController(flexId, val) == false)
			return {};
		return val;
	}
	std::optional<float> GetFlexController(CFlexComponent &hEnt, const std::string &flexController)
	{
		auto flexId = 0u;
		auto mdlComponent = hEnt.GetEntity().GetModelComponent();
		if(!mdlComponent || mdlComponent->LookupFlexController(flexController, flexId) == false)
			return {};
		auto val = 0.f;
		if(hEnt.GetFlexController(flexId, val) == false)
			return {};
		return val;
	}
	std::optional<float> CalcFlexValue(CFlexComponent &hEnt, uint32_t flexId)
	{
		auto val = 0.f;
		if(hEnt.CalcFlexValue(flexId, val) == false)
			return {};
		return val;
	}
};

void CFlexComponent::RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts)
{
	BaseFlexComponent::RegisterLuaBindings(l, modEnts);
	auto defCFlex = pragma::LuaCore::create_entity_component_class<CFlexComponent, BaseFlexComponent>("FlexComponent");
	defCFlex.def("SetFlexController",
	  static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t, float, float, bool)>([](lua::State *l, CFlexComponent &hEnt, uint32_t flexId, float value, float duration, bool clampToLimits) { hEnt.SetFlexController(flexId, value, duration, clampToLimits); }));
	defCFlex.def("SetFlexController", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t, float, float)>([](lua::State *l, CFlexComponent &hEnt, uint32_t flexId, float value, float duration) { hEnt.SetFlexController(flexId, value, duration); }));
	defCFlex.def("SetFlexController", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t, float)>([](lua::State *l, CFlexComponent &hEnt, uint32_t flexId, float value) { hEnt.SetFlexController(flexId, value); }));
	defCFlex.def("SetFlexController", static_cast<void (*)(lua::State *, CFlexComponent &, const std::string &, float, float, bool)>([](lua::State *l, CFlexComponent &hEnt, const std::string &flexName, float value, float duration, bool clampToLimits) {
		hEnt.SetFlexController(flexName, value, duration, clampToLimits);
	}));
	defCFlex.def("SetFlexController",
	  static_cast<void (*)(lua::State *, CFlexComponent &, const std::string &, float, float)>([](lua::State *l, CFlexComponent &hEnt, const std::string &flexName, float value, float duration) { hEnt.SetFlexController(flexName, value, duration); }));
	defCFlex.def("SetFlexController", static_cast<void (*)(lua::State *, CFlexComponent &, const std::string &, float)>([](lua::State *l, CFlexComponent &hEnt, const std::string &flexName, float value) { hEnt.SetFlexController(flexName, value); }));
	defCFlex.def("GetFlexController", static_cast<std::optional<float> (*)(CFlexComponent &, uint32_t)>(&Lua::Flex::GetFlexController));
	defCFlex.def("GetFlexController", static_cast<std::optional<float> (*)(CFlexComponent &, const std::string &)>(&Lua::Flex::GetFlexController));
	defCFlex.def("GetScaledFlexController", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t)>([](lua::State *l, CFlexComponent &hEnt, uint32_t flexControllerId) {
		float value;
		if(hEnt.GetScaledFlexController(flexControllerId, value) == false)
			return;
		Lua::PushNumber(l, value);
	}));
	defCFlex.def("SetFlexControllerScale", static_cast<void (*)(lua::State *, CFlexComponent &, float)>([](lua::State *l, CFlexComponent &hEnt, float scale) { hEnt.SetFlexControllerScale(scale); }));
	defCFlex.def("GetFlexControllerScale", static_cast<void (*)(lua::State *, CFlexComponent &)>([](lua::State *l, CFlexComponent &hEnt) { Lua::PushNumber(l, hEnt.GetFlexControllerScale()); }));
	defCFlex.def("CalcFlexValue", &Lua::Flex::CalcFlexValue);
	defCFlex.def("GetFlexWeights", static_cast<void (*)(lua::State *, CFlexComponent &)>([](lua::State *l, CFlexComponent &hEnt) {
		auto &flexWeights = hEnt.GetFlexWeights();
		auto t = Lua::CreateTable(l);
		for(auto i = decltype(flexWeights.size()) {0u}; i < flexWeights.size(); ++i) {
			Lua::PushInt(l, i + 1);
			Lua::PushNumber(l, flexWeights.at(i));
			Lua::SetTableValue(l, t);
		}
	}));
	defCFlex.def("GetFlexWeight", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t)>([](lua::State *l, CFlexComponent &hEnt, uint32_t flexId) {
		float weight;
		if(hEnt.GetFlexWeight(flexId, weight) == false)
			return;
		Lua::PushNumber(l, weight);
	}));
	defCFlex.def("SetFlexWeight", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t, float)>([](lua::State *l, CFlexComponent &hEnt, uint32_t flexId, float weight) { hEnt.SetFlexWeight(flexId, weight); }));
	defCFlex.def("ClearFlexWeightOverride", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t)>([](lua::State *l, CFlexComponent &hEnt, uint32_t flexId) { hEnt.ClearFlexWeightOverride(flexId); }));
	defCFlex.def("HasFlexWeightOverride", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t)>([](lua::State *l, CFlexComponent &hEnt, uint32_t flexId) { Lua::PushBool(l, hEnt.HasFlexWeightOverride(flexId)); }));
	defCFlex.def("SetFlexWeightOverride", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t, float)>([](lua::State *l, CFlexComponent &hEnt, uint32_t flexId, float weight) { hEnt.SetFlexWeightOverride(flexId, weight); }));
	defCFlex.def("GetFlexAnimations", static_cast<luabind::object (*)(lua::State *, CFlexComponent &)>([](lua::State *l, CFlexComponent &hEnt) -> luabind::object {
		auto &flexAnims = hEnt.GetFlexAnimations();
		auto t = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &flexAnim : flexAnims)
			t[idx++] = flexAnim.flexAnimationId;
		return t;
	}));
	defCFlex.def("GetFlexAnimationCount", static_cast<uint32_t (*)(lua::State *, CFlexComponent &)>([](lua::State *l, CFlexComponent &hEnt) -> uint32_t { return hEnt.GetFlexAnimations().size(); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t, bool, bool)>([](lua::State *l, CFlexComponent &hEnt, uint32_t id, bool loop, bool reset) { hEnt.PlayFlexAnimation(id, loop, reset); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t, bool)>([](lua::State *l, CFlexComponent &hEnt, uint32_t id, bool loop) { hEnt.PlayFlexAnimation(id, loop); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t)>([](lua::State *l, CFlexComponent &hEnt, uint32_t id) { hEnt.PlayFlexAnimation(id); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua::State *, CFlexComponent &, const std::string &, bool, bool)>([](lua::State *l, CFlexComponent &hEnt, const std::string &id, bool loop, bool reset) { hEnt.PlayFlexAnimation(id, loop, reset); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua::State *, CFlexComponent &, const std::string &, bool)>([](lua::State *l, CFlexComponent &hEnt, const std::string &id, bool loop) { hEnt.PlayFlexAnimation(id, loop); }));
	defCFlex.def("PlayFlexAnimation", static_cast<void (*)(lua::State *, CFlexComponent &, const std::string &)>([](lua::State *l, CFlexComponent &hEnt, const std::string &id) { hEnt.PlayFlexAnimation(id); }));
	defCFlex.def("StopFlexAnimation", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t)>([](lua::State *l, CFlexComponent &hEnt, uint32_t id) { hEnt.StopFlexAnimation(id); }));
	defCFlex.def("StopFlexAnimation", static_cast<void (*)(lua::State *, CFlexComponent &, const std::string &)>([](lua::State *l, CFlexComponent &hEnt, const std::string &id) { hEnt.StopFlexAnimation(id); }));
	defCFlex.def("GetFlexAnimationCycle", static_cast<float (*)(lua::State *, CFlexComponent &, uint32_t)>([](lua::State *l, CFlexComponent &hEnt, uint32_t id) -> float { return hEnt.GetFlexAnimationCycle(id); }));
	defCFlex.def("GetFlexAnimationCycle", static_cast<float (*)(lua::State *, CFlexComponent &, const std::string &)>([](lua::State *l, CFlexComponent &hEnt, const std::string &id) -> float { return hEnt.GetFlexAnimationCycle(id); }));
	defCFlex.def("SetFlexAnimationCycle", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t, float)>([](lua::State *l, CFlexComponent &hEnt, uint32_t id, float cycle) { hEnt.SetFlexAnimationCycle(id, cycle); }));
	defCFlex.def("SetFlexAnimationCycle", static_cast<void (*)(lua::State *, CFlexComponent &, const std::string &, float)>([](lua::State *l, CFlexComponent &hEnt, const std::string &id, float cycle) { hEnt.SetFlexAnimationCycle(id, cycle); }));
	defCFlex.def("SetFlexAnimationPlaybackRate", static_cast<void (*)(lua::State *, CFlexComponent &, uint32_t, float)>([](lua::State *l, CFlexComponent &hEnt, uint32_t id, float playbackRate) { hEnt.SetFlexAnimationPlaybackRate(id, playbackRate); }));
	defCFlex.def("SetFlexAnimationPlaybackRate", static_cast<void (*)(lua::State *, CFlexComponent &, const std::string &, float)>([](lua::State *l, CFlexComponent &hEnt, const std::string &id, float playbackRate) { hEnt.SetFlexAnimationPlaybackRate(id, playbackRate); }));
	defCFlex.add_static_constant("EVENT_ON_FLEX_CONTROLLERS_UPDATED", cFlexComponent::EVENT_ON_FLEX_CONTROLLERS_UPDATED);
	modEnts[defCFlex];
}
