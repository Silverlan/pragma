// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.flex;
using namespace pragma;

void CFlexComponent::ResolveFlexAnimation(const LookupIdentifier &lookupId) const
{
	if(lookupId.resolved() || lookupId.name.has_value() == false)
		return;
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	lookupId.id = mdl->LookupFlexAnimation(*lookupId.name);
}
CFlexComponent::FlexAnimationData *CFlexComponent::FindFlexAnimationData(uint32_t flexAnimId)
{
	auto it = std::find_if(m_flexAnimations.begin(), m_flexAnimations.end(), [flexAnimId](const FlexAnimationData &data) { return data.flexAnimationId == flexAnimId; });
	return (it != m_flexAnimations.end()) ? &*it : nullptr;
}
void CFlexComponent::SetFlexAnimationCycle(const LookupIdentifier &id, float cycle)
{
	ResolveFlexAnimation(id);
	if(id.resolved() == false)
		return;
	auto *flexAnimData = FindFlexAnimationData(*id.id);
	if(flexAnimData == nullptr)
		return;
	flexAnimData->t = cycle;
}
float CFlexComponent::GetFlexAnimationCycle(const LookupIdentifier &id) const
{
	ResolveFlexAnimation(id);
	if(id.resolved() == false)
		return 0.f;
	auto *flexAnimData = FindFlexAnimationData(*id.id);
	return flexAnimData ? flexAnimData->t : 0.f;
}
void CFlexComponent::PlayFlexAnimation(const LookupIdentifier &id, bool loop, bool reset)
{
	ResolveFlexAnimation(id);
	if(id.resolved() == false)
		return;
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto flexAnimId = *id.id;
	auto it = std::find_if(m_flexAnimations.begin(), m_flexAnimations.end(), [flexAnimId](const FlexAnimationData &flexAnimData) { return flexAnimData.flexAnimationId == flexAnimId; });
	if(it == m_flexAnimations.end()) {
		m_flexAnimations.push_back({});
		it = m_flexAnimations.end() - 1;
	}

	auto &flexAnimData = *it;
	flexAnimData.flexAnimationId = flexAnimId;
	flexAnimData.loop = loop;
	if(reset)
		flexAnimData.t = 0.f;
}
void CFlexComponent::SetFlexAnimationPlaybackRate(const LookupIdentifier &id, float playbackRate)
{
	ResolveFlexAnimation(id);
	if(id.resolved() == false)
		return;
	auto *flexAnimData = FindFlexAnimationData(*id.id);
	if(flexAnimData == nullptr)
		return;
	flexAnimData->playbackRate = playbackRate;
}
const std::vector<CFlexComponent::FlexAnimationData> &CFlexComponent::GetFlexAnimations() const { return m_flexAnimations; }
void CFlexComponent::StopFlexAnimation(const LookupIdentifier &id)
{
	ResolveFlexAnimation(id);
	if(id.resolved() == false)
		return;
	auto &mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto flexAnimId = *id.id;
	auto it = std::find_if(m_flexAnimations.begin(), m_flexAnimations.end(), [flexAnimId](const FlexAnimationData &flexAnimData) { return flexAnimData.flexAnimationId == flexAnimId; });
	if(it == m_flexAnimations.end())
		return;
	m_flexAnimations.erase(it);
}
void CFlexComponent::MaintainFlexAnimations(float dt)
{
	auto &mdl = GetEntity().GetModel();
	if(m_flexAnimations.empty() || mdl == nullptr)
		return;
	for(auto it = m_flexAnimations.begin(); it != m_flexAnimations.end();) {
		auto &flexAnimData = *it;
		auto *flexAnim = mdl->GetFlexAnimation(flexAnimData.flexAnimationId);
		if(flexAnim == nullptr) {
			++it;
			continue;
		}
		auto &frames = flexAnim->GetFrames();
		if(frames.empty()) {
			++it;
			continue;
		}
		auto fps = flexAnim->GetFps();
		auto numFrames = frames.size();
		auto dtAnim = (fps > 0.f) ? ((dt * flexAnimData.playbackRate) / (numFrames / fps)) : 0.f;
		flexAnimData.t += dtAnim;

		auto &t = flexAnimData.t;
		if(flexAnimData.loop)
			t = fmodf(t, 1.f);
		else
			t = math::min(t, 1.f);
		auto tFrame = t * (frames.size() - 1);
		auto frameId0 = math::floor(tFrame);
		auto frameId1 = frameId0 + 1;
		auto endOfAnimation = (tFrame >= 1.f);
		if((endOfAnimation && flexAnimData.loop == false) || frameId1 >= numFrames)
			frameId1 = frameId0;

		auto &frame0 = frames[frameId0];
		auto &frame1 = frames[frameId1];

		auto &flexControllerIds = flexAnim->GetFlexControllerIds();
		auto &values0 = frame0->GetValues();
		auto &values1 = frame1->GetValues();
		tFrame = fmodf(tFrame, 1.f);
		for(auto i = decltype(flexControllerIds.size()) {0u}; i < flexControllerIds.size(); ++i) {
			auto v = math::lerp(values0[i], values1[i], tFrame);
			SetFlexController(flexControllerIds[i], v);
		}
		if(endOfAnimation && flexAnimData.loop == false) {
			it = m_flexAnimations.erase(it);
			continue;
		}
		++it;
	}
}
