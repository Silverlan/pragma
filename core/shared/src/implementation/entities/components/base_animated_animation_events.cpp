// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_animated;

using namespace pragma;

BaseAnimatedComponent::CustomAnimationEvent::CustomAnimationEvent(const AnimationEvent &ev) : AnimationEvent(ev) {}
BaseAnimatedComponent::CustomAnimationEvent::CustomAnimationEvent(const std::function<void(void)> &f) { callback = {true, FunctionCallback<void>::Create(f)}; }
BaseAnimatedComponent::CustomAnimationEvent::CustomAnimationEvent(const CallbackHandle &cb) { callback = {true, cb}; }

void BaseAnimatedComponent::ApplyAnimationEventTemplate(const TemplateAnimationEvent &t)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(hModel == nullptr)
		return;
	auto animId = t.animId;
	if(t.animId == -1)
		animId = hModel->LookupAnimation(t.anim);
	if(animId == -1)
		return;
	auto frameId = t.frameId;
	auto it0 = m_animEvents.find(animId);
	if(it0 == m_animEvents.end())
		it0 = m_animEvents.insert(decltype(m_animEvents)::value_type(animId, {})).first;
	auto it1 = it0->second.find(frameId);
	if(it1 == it0->second.end())
		it1 = it0->second.insert(decltype(it0->second)::value_type(frameId, {})).first;

	auto &events = it1->second;
	events.push_back(t.ev);
}
void BaseAnimatedComponent::ApplyAnimationEventTemplates()
{
	m_animEvents.clear();
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(hModel == nullptr)
		return;
	for(auto &t : m_animEventTemplates)
		ApplyAnimationEventTemplate(t);
}
void BaseAnimatedComponent::AddAnimationEvent(uint32_t animId, uint32_t frameId, const AnimationEvent &ev)
{
	m_animEventTemplates.push_back({});
	auto &t = m_animEventTemplates.back();
	t.animId = animId;
	t.frameId = frameId;
	t.ev = ev;
	ApplyAnimationEventTemplate(t);
}
void BaseAnimatedComponent::AddAnimationEvent(const std::string &name, uint32_t frameId, const AnimationEvent &ev)
{
	m_animEventTemplates.push_back({});
	auto &t = m_animEventTemplates.back();
	t.anim = name;
	t.frameId = frameId;
	t.ev = ev;
	ApplyAnimationEventTemplate(t);
}
CallbackHandle BaseAnimatedComponent::AddAnimationEvent(const std::string &name, uint32_t frameId, const std::function<void(void)> &f)
{
	m_animEventTemplates.push_back({});
	auto &t = m_animEventTemplates.back();
	t.anim = name;
	t.frameId = frameId;
	t.ev = {f};
	ApplyAnimationEventTemplate(t);
	return t.ev.callback.second;
}
CallbackHandle BaseAnimatedComponent::AddAnimationEvent(const std::string &name, uint32_t frameId, const CallbackHandle &cb)
{
	m_animEventTemplates.push_back({});
	auto &t = m_animEventTemplates.back();
	t.anim = name;
	t.frameId = frameId;
	t.ev = {cb};
	ApplyAnimationEventTemplate(t);
	return t.ev.callback.second;
}
CallbackHandle BaseAnimatedComponent::AddAnimationEvent(uint32_t animId, uint32_t frameId, const std::function<void(void)> &f) { return AddAnimationEvent(animId, frameId, FunctionCallback<void>::Create(f)); }
CallbackHandle BaseAnimatedComponent::AddAnimationEvent(uint32_t animId, uint32_t frameId, const CallbackHandle &cb)
{
	m_animEventTemplates.push_back({});
	auto &t = m_animEventTemplates.back();
	t.animId = animId;
	t.frameId = frameId;
	t.ev = {cb};
	ApplyAnimationEventTemplate(t);
	return t.ev.callback.second;
}
void BaseAnimatedComponent::ClearAnimationEvents()
{
	m_animEvents.clear();
	m_animEventTemplates.clear();
}
void BaseAnimatedComponent::ClearAnimationEvents(uint32_t animId)
{
	auto it = m_animEvents.find(animId);
	if(it != m_animEvents.end())
		it->second.clear();

	for(auto it = m_animEventTemplates.begin(); it != m_animEventTemplates.end();) {
		auto &t = *it;
		if(t.animId == animId)
			it = m_animEventTemplates.erase(it);
		else
			++it;
	}
}
void BaseAnimatedComponent::ClearAnimationEvents(uint32_t animId, uint32_t frameId)
{
	auto it0 = m_animEvents.find(animId);
	if(it0 != m_animEvents.end()) {
		auto it1 = it0->second.find(frameId);
		if(it1 != it0->second.end())
			it1->second.clear();
	}

	for(auto it = m_animEventTemplates.begin(); it != m_animEventTemplates.end();) {
		auto &t = *it;
		if(t.animId == animId && t.frameId == frameId)
			it = m_animEventTemplates.erase(it);
		else
			++it;
	}
}
void BaseAnimatedComponent::ClearAnimationEvents(const std::string &anim)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(hModel != nullptr) {
		auto animId = hModel->LookupAnimation(anim);
		if(animId != -1)
			ClearAnimationEvents(animId);
	}

	for(auto it = m_animEventTemplates.begin(); it != m_animEventTemplates.end();) {
		auto &t = *it;
		if(t.anim == anim)
			it = m_animEventTemplates.erase(it);
		else
			++it;
	}
}
void BaseAnimatedComponent::ClearAnimationEvents(const std::string &anim, uint32_t frameId)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hModel = mdlComponent ? mdlComponent->GetModel() : nullptr;
	if(hModel != nullptr) {
		auto animId = hModel->LookupAnimation(anim);
		if(animId != -1)
			ClearAnimationEvents(animId, frameId);
	}

	for(auto it = m_animEventTemplates.begin(); it != m_animEventTemplates.end();) {
		auto &t = *it;
		if(t.anim == anim && t.frameId == frameId)
			it = m_animEventTemplates.erase(it);
		else
			++it;
	}
}
/*
void BaseAnimatedComponent::RemoveAnimationEvent(uint32_t animId,uint32_t frameId,uint32_t idx)
{
	auto it0 = m_animEvents.find(animId);
	if(it0 == m_animEvents.end())
		return;
	auto it1 = it0->second.find(frameId);
	if(it1 == it0->second.end())
		return;
	auto &events = it1->second;
	if(idx >= events.size())
		return;
	events.erase(events.begin() +idx);
	if(!events.empty())
		return;
	it0->second.erase(it1);
	if(!it0->second.empty())
		return;
	m_animEvents.erase(it0);
}
*/
std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::vector<BaseAnimatedComponent::CustomAnimationEvent>>> &BaseAnimatedComponent::GetAnimationEvents() { return m_animEvents; }
std::vector<BaseAnimatedComponent::CustomAnimationEvent> *BaseAnimatedComponent::GetAnimationEvents(uint32_t animId, uint32_t frameId)
{
	auto it0 = m_animEvents.find(animId);
	if(it0 == m_animEvents.end())
		return nullptr;
	auto it1 = it0->second.find(frameId);
	if(it1 == it0->second.end())
		return nullptr;
	return &it1->second;
}
void BaseAnimatedComponent::InjectAnimationEvent(const AnimationEvent &ev) { HandleAnimationEvent(ev); }
