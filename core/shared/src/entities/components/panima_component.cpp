/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_time_scale_component.hpp"
#include "pragma/entities/components/panima_component_channel_submitter.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/game/animation_update_manager.hpp"
#include "pragma/model/model.h"
#include "pragma/model/animation/animation.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/debug/intel_vtune.hpp"
#include <sharedutils/util_uri.hpp>
#include <panima/animation_manager.hpp>
#include <panima/channel.hpp>
#include <panima/animation.hpp>
#include <panima/animation_set.hpp>
#include <panima/player.hpp>
#include <panima/channel_t.hpp>

using namespace pragma;

ComponentEventId PanimaComponent::EVENT_HANDLE_ANIMATION_EVENT = pragma::INVALID_COMPONENT_ID;
ComponentEventId PanimaComponent::EVENT_ON_PLAY_ANIMATION = pragma::INVALID_COMPONENT_ID;
ComponentEventId PanimaComponent::EVENT_ON_ANIMATION_COMPLETE = pragma::INVALID_COMPONENT_ID;
ComponentEventId PanimaComponent::EVENT_ON_ANIMATION_START = pragma::INVALID_COMPONENT_ID;
ComponentEventId PanimaComponent::EVENT_MAINTAIN_ANIMATIONS = pragma::INVALID_COMPONENT_ID;
ComponentEventId PanimaComponent::EVENT_ON_ANIMATIONS_UPDATED = pragma::INVALID_COMPONENT_ID;
ComponentEventId PanimaComponent::EVENT_PLAY_ANIMATION = pragma::INVALID_COMPONENT_ID;
ComponentEventId PanimaComponent::EVENT_TRANSLATE_ANIMATION = pragma::INVALID_COMPONENT_ID;
ComponentEventId PanimaComponent::EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER = pragma::INVALID_COMPONENT_ID;
void PanimaComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_HANDLE_ANIMATION_EVENT = registerEvent("A2_HANDLE_ANIMATION_EVENT", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_PLAY_ANIMATION = registerEvent("A2_ON_PLAY_ANIMATION", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_ANIMATION_COMPLETE = registerEvent("A2_ON_ANIMATION_COMPLETE", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_ANIMATION_START = registerEvent("A2_ON_ANIMATION_START", ComponentEventInfo::Type::Broadcast);
	EVENT_MAINTAIN_ANIMATIONS = registerEvent("A2_MAINTAIN_ANIMATIONS", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_ANIMATIONS_UPDATED = registerEvent("A2_ON_ANIMATIONS_UPDATED", ComponentEventInfo::Type::Broadcast);
	EVENT_PLAY_ANIMATION = registerEvent("A2_PLAY_ANIMATION", ComponentEventInfo::Type::Broadcast);
	EVENT_TRANSLATE_ANIMATION = registerEvent("A2_TRANSLATE_ANIMATION", ComponentEventInfo::Type::Broadcast);
	EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER = registerEvent("A2_INITIALIZE_CHANNEL_VALUE_SUBMITTER", ComponentEventInfo::Type::Broadcast);
}
std::optional<std::pair<std::string, util::Path>> PanimaComponent::ParseComponentChannelPath(const panima::ChannelPath &path)
{
	size_t offset = 0;
	if(path.path.GetComponent(offset, &offset) != "ec")
		return {};
	auto componentName = path.path.GetComponent(offset, &offset);
	if(offset == std::string::npos)
		return {};
	auto pathStr = path.path.GetString().substr(offset);
	ustring::replace(pathStr, "%20", " ");
	util::Path componentPath {std::move(pathStr)};
	return std::pair<std::string, util::Path> {std::string {componentName}, std::move(componentPath)};
}
PanimaComponent::PanimaComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_playbackRate(util::FloatProperty::Create(1.f)) {}
void PanimaComponent::SetPlaybackRate(float rate) { *m_playbackRate = rate; }
float PanimaComponent::GetPlaybackRate() const { return *m_playbackRate; }
const util::PFloatProperty &PanimaComponent::GetPlaybackRateProperty() const { return m_playbackRate; }
std::vector<std::pair<std::string, panima::PAnimationManager>>::iterator PanimaComponent::FindAnimationManager(const std::string_view &name)
{
	return std::find_if(m_animationManagers.begin(), m_animationManagers.end(), [&name](const std::pair<std::string, panima::PAnimationManager> &pair) { return pair.first == name; });
}
panima::PAnimationManager PanimaComponent::GetAnimationManager(std::string name)
{
	auto it = FindAnimationManager(name);
	return (it != m_animationManagers.end()) ? it->second : nullptr;
}
panima::PAnimationManager PanimaComponent::AddAnimationManager(std::string name, int32_t priority)
{
	auto it = FindAnimationManager(name);
	if(it != m_animationManagers.end())
		return it->second;
	auto player = panima::AnimationManager::Create();
	panima::AnimationPlayerCallbackInterface callbackInteface {};
	callbackInteface.onPlayAnimation = [this](const panima::AnimationSet &set, panima::AnimationId animId, panima::PlaybackFlags flags) -> bool {
		CEAnim2OnPlayAnimation evData {set, animId, flags};
		return InvokeEventCallbacks(EVENT_PLAY_ANIMATION, evData) != util::EventReply::Handled;
	};
	callbackInteface.onStopAnimation = []() {

	};
	callbackInteface.translateAnimation = [this](const panima::AnimationSet &set, panima::AnimationId &animId, panima::PlaybackFlags &flags) {
		CEAnim2TranslateAnimation evTranslateAnimData {set, animId, flags};
		InvokeEventCallbacks(EVENT_TRANSLATE_ANIMATION, evTranslateAnimData);
	};
	auto r = player;
	m_animationManagers.push_back(std::make_pair<std::string, panima::PAnimationManager>(std::move(name), std::move(player)));
	std::sort(m_animationManagers.begin(), m_animationManagers.end(), [](const std::pair<std::string, panima::PAnimationManager> &a, const std::pair<std::string, panima::PAnimationManager> &b) { return a.second->GetPriority() < b.second->GetPriority(); });
	return r;
}
void PanimaComponent::RemoveAnimationManager(const std::string_view &name)
{
	auto it = FindAnimationManager(name);
	if(it == m_animationManagers.end())
		return;
	m_animationManagers.erase(it);
}
void PanimaComponent::RemoveAnimationManager(const panima::AnimationManager &player)
{
	auto it = std::find_if(m_animationManagers.begin(), m_animationManagers.end(), [&player](const std::pair<std::string, panima::PAnimationManager> &pair) { return pair.second.get() == &player; });
	if(it == m_animationManagers.end())
		return;
	m_animationManagers.erase(it);
}

template<typename T>
static std::string to_string(const T &value)
{
	if constexpr(std::is_same_v<T, Quat>)
		return uquat::to_string(value);
	else if constexpr(umath::is_matrix_type<T>)
		return umat::to_string(value);
	else if constexpr(umath::is_vector_type<T>)
		return uvec::to_string(value);
	else {
		std::stringstream ss;
		ss << value;
		return ss.str();
	}
}

void PanimaComponent::InitializeAnimationChannelValueSubmitters()
{
	for(auto &pair : m_animationManagers)
		InitializeAnimationChannelValueSubmitters(*pair.second);
}

template<uint32_t I>
    requires(I < 4)
static bool is_vector_component(const std::string &str)
{
	using namespace ustring::string_switch;
	switch(hash(str)) {
	case "x"_:
	case "r"_:
	case "red"_:
		return I == 0;
	case "y"_:
	case "g"_:
	case "green"_:
		return I == 1;
	case "z"_:
	case "b"_:
	case "blue"_:
		return I == 2;
	case "w"_:
	case "a"_:
	case "alpha"_:
		return I == 3;
	}
	return false;
}

void PanimaComponent::DebugPrint(std::stringstream &ss)
{
	auto printAnimManager = [&ss](const std::string &name, const panima::AnimationManager &manager) {
		auto *anim = manager.GetCurrentAnimation();
		ss << "AnimationManager[" << name << "]:\n";
		ss << "\tCurrent Animation: " << (anim ? anim->GetName() : "NULL") << "\n";
		if(anim) {
			ss << "\t\tFlags: " << magic_enum::flags::enum_name(anim->GetFlags()) << "\n";
			ss << "\t\tDuration: " << anim->GetDuration() << "\n";
		}
		auto &player = manager.GetPlayer();
		ss << "\tPlayback offset: " << player.GetCurrentTime() << "/" << player.GetDuration() << " (" << player.GetCurrentTimeFraction() << ")\n";
		ss << "\tPlayback rate: " << player.GetPlaybackRate() << "\n";
		if(anim) {
			auto &channels = anim->GetChannels();
			auto &channelValueSubmitters = manager.GetChannelValueSubmitters();
			ss << "\tChannels:\n";
			for(auto i = decltype(channels.size()) {0u}; i < channels.size(); ++i) {
				auto &channel = channels[i];
				ss << "\t\tPath: " << channel->targetPath.ToUri() << "\n";
				ss << "\t\t\tTime range: [" << channel->GetMinTime() << "," << channel->GetMaxTime() << "]\n";
				ss << "\t\t\tValue type: " << magic_enum::enum_name(channel->GetValueType()) << "\n";
				ss << "\t\t\tNumber of times/values: " << channel->GetTimeCount() << "/" << channel->GetValueCount() << "\n";
				auto hasSubmitter = (i < channelValueSubmitters.size() && channelValueSubmitters[i] != nullptr);
				ss << "\t\t\tHas submitter: " << (hasSubmitter ? "true" : "false") << "\n";

				udm::visit_ng(channel->GetValueType(), [&channel, &player, &ss](auto tag) {
					using T = typename decltype(tag)::type;
					if constexpr(is_animatable_type_v<T>) {
						auto val = channel->GetInterpolatedValue<T>(player.GetCurrentTime());
						auto valStr = udm::convert<T, udm::String>(val);
						ss << "\t\t\tInterpolated value at current timestamp: " << valStr << "\n";
					}
				});

				auto *expr = channel->GetValueExpression();
				if(expr)
					ss << "\t\t\tExpression: " << *expr << "\n";
			}
			ss << "\n";
		}
	};
	for(auto &pair : GetAnimationManagers())
		printAnimManager(pair.first, *pair.second);
}

void PanimaComponent::DebugPrint()
{
	std::stringstream ss;
	DebugPrint(ss);
	Con::cout << "Animation info: \n" << ss.str() << Con::endl;
}

void PanimaComponent::UpdateAnimationChannelSubmitters()
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("panima_update_channel_values");
#endif
	InitializeAnimationChannelValueSubmitters();
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
}

bool PanimaComponent::IsPropertyAnimated(panima::AnimationManager &manager, const std::string &propName) const
{
	auto *anim = manager.GetCurrentAnimation();
	if(!anim)
		return false;
	panima::ChannelPath channelPath {propName};
	auto componentPath = ParseComponentChannelPath(channelPath);
	if(!componentPath.has_value())
		return false;
	auto c = GetEntity().FindComponent(componentPath->first);
	if(c.expired())
		return false;
	auto memberIdx = c->GetMemberIndex(componentPath->second.GetString());
	if(!memberIdx.has_value())
		return false;
	auto *memberInfo = c->GetMemberInfo(*memberIdx);
	if(!memberInfo)
		return false;
	if(!pragma::ents::is_udm_member_type(memberInfo->type))
		return false;
	auto &channels = anim->GetChannels();
	auto numChannels = channels.size();
	for(auto i = decltype(numChannels) {0u}; i < numChannels; ++i) {
		auto &channel = channels[i];
		if(channel->targetPath == channelPath)
			return true;
	}
	return false;
}

void PanimaComponent::SetPropertyEnabled(const std::string &propName, bool enabled)
{
	panima::ChannelPath channelPath {propName};
	auto normalizedPath = channelPath.ToUri(false);
	if(enabled) {
		auto it = m_disabledProperties.find(pragma::register_global_string(normalizedPath));
		if(it != m_disabledProperties.end()) {
			m_disabledProperties.erase(it);
			InitializeAnimationChannelValueSubmitters();
		}
		return;
	}
	m_disabledProperties.insert(pragma::register_global_string(normalizedPath));
	InitializeAnimationChannelValueSubmitters();
}
bool PanimaComponent::IsPropertyEnabled(const std::string &propName) const
{
	panima::ChannelPath channelPath {propName};
	auto normalizedPath = channelPath.ToUri(false);
	return m_disabledProperties.find(pragma::register_global_string(normalizedPath)) == m_disabledProperties.end();
}
bool PanimaComponent::GetRawAnimatedPropertyValue(panima::AnimationManager &manager, const std::string &propName, udm::Type type, void *outValue, const ComponentMemberInfo **optOutMemberInfo, pragma::BaseEntityComponent **optOutComponent) const
{
	if(optOutMemberInfo)
		*optOutMemberInfo = nullptr;
	if(optOutComponent)
		*optOutComponent = nullptr;
	auto *anim = manager.GetCurrentAnimation();
	if(!anim)
		return false;
	panima::ChannelPath channelPath {propName};
	auto componentPath = ParseComponentChannelPath(channelPath);
	if(!componentPath.has_value())
		return false;
	auto c = GetEntity().FindComponent(componentPath->first);
	if(c.expired())
		return false;
	if(optOutComponent)
		*optOutComponent = c.get();
	auto memberIdx = c->GetMemberIndex(componentPath->second.GetString());
	if(!memberIdx.has_value())
		return false;
	auto *memberInfo = c->GetMemberInfo(*memberIdx);
	if(!memberInfo)
		return false;
	if(optOutMemberInfo)
		*optOutMemberInfo = memberInfo;
	if(!pragma::ents::is_udm_member_type(memberInfo->type))
		return false;
	auto memberType = static_cast<udm::Type>(memberInfo->type);
	auto &player = manager.GetPlayer();
	auto t = player.GetCurrentTime();
	auto &channels = anim->GetChannels();
	auto numChannels = channels.size();
	// See if there is an animation channel for this property, and if there is, return
	// its value for the current timestamp WITHOUT applying the math expression.
	for(auto i = decltype(numChannels) {0u}; i < numChannels; ++i) {
		auto &channel = channels[i];
		if(channel->targetPath != channelPath)
			continue;
		auto valueType = channel->GetValueType();
		return udm::visit_ng(valueType, [t, i, valueType, type, memberInfo, &channel, &player, outValue](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(pragma::is_animatable_type_v<T>) {
				auto value = channel->GetInterpolatedValue<T>(t, player.GetLastChannelTimestampIndex(i), memberInfo->interpolationFunction);
				if(valueType == type) {
					memcpy(outValue, &value, sizeof(value));
					return true;
				}
				// Conversion required
				return udm::visit_ng(type, [&value, outValue](auto tag) {
					using TOut = typename decltype(tag)::type;
					if constexpr(udm::is_convertible<T, TOut>()) {
						auto cvalue = udm::convert<T, TOut>(value);
						memcpy(outValue, &cvalue, sizeof(cvalue));
						return true;
					}
					return false;
				});
			}
			else
				return false;
		});
	}
	return false;
}
bool PanimaComponent::GetRawAnimatedPropertyValue(panima::AnimationManager &manager, const std::string &propName, udm::Type type, void *outValue) const { return GetRawAnimatedPropertyValue(manager, propName, type, outValue, nullptr, nullptr); }
bool PanimaComponent::GetRawPropertyValue(panima::AnimationManager &manager, const std::string &propName, udm::Type type, void *outValue) const
{
	const ComponentMemberInfo *memberInfo = nullptr;
	pragma::BaseEntityComponent *c = nullptr;
	auto res = GetRawAnimatedPropertyValue(manager, propName, type, outValue, &memberInfo, &c);
	if(res || !memberInfo)
		return res;
	auto memberType = static_cast<udm::Type>(memberInfo->type);
	// No animation channel found, just return the current property value of the component
	if(type == memberType) {
		memberInfo->getterFunction(*memberInfo, *c, outValue);
		return true;
	}
	// Conversion required
	return udm::visit_ng(memberType, [type, memberInfo, &c, outValue](auto tag) {
		using T = typename decltype(tag)::type;
		T value;
		memberInfo->getterFunction(*memberInfo, *c, &value);
		return udm::visit_ng(type, [&value, outValue](auto tag) {
			using TOut = typename decltype(tag)::type;
			if constexpr(udm::is_convertible<T, TOut>()) {
				auto cvalue = udm::convert<T, TOut>(value);
				memcpy(outValue, &cvalue, sizeof(cvalue));
				return true;
			}
			return false;
		});
	});
}

void PanimaComponent::InitializeAnimationChannelValueSubmitters(panima::AnimationManager &manager)
{
	auto *anim = manager.GetCurrentAnimation();
	auto &channelValueSubmitters = manager.GetChannelValueSubmitters();
	if(!anim) {
		channelValueSubmitters.clear();
		return;
	}
	auto &channels = anim->GetChannels();
	channelValueSubmitters.clear();
	channelValueSubmitters.resize(channels.size(), panima::ChannelValueSubmitter {});
	uint32_t numInvalidChannels = 0;
	auto shouldPrintWarning = [&numInvalidChannels]() {
		if(numInvalidChannels >= 5) {
			if(numInvalidChannels == 5)
				Con::cwar << "Additional warnings will be suppressed." << Con::endl;
			++numInvalidChannels;
			return false;
		}
		++numInvalidChannels;
		return true;
	};
	// TODO: Reload this when animation has changed, or if component data has changed (e.g. animated component has changed model and therefore bone positions and rotational data)
	for(auto it = channels.begin(); it != channels.end(); ++it) {
		auto &channel = *it;
		auto &path = channel->targetPath;
		if(!IsPropertyEnabled(path.path.GetString()))
			continue;
		size_t offset = 0;
		if(path.path.GetComponent(offset, &offset) != "ec") // First path component denotes the type, which always has to be 'ec' for entity component in this case
		{
			if(shouldPrintWarning())
				Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but path is not a valid entity component URI!" << Con::endl;
			continue;
		}
		auto componentPath = ParseComponentChannelPath(path);
		if(!componentPath.has_value()) {
			if(shouldPrintWarning())
				Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but could not determine path components!" << Con::endl;
			continue;
		}
		auto &componentTypeName = componentPath->first;
		// TODO: Needs to be updated whenever a new component has been added to the entity
		auto hComponent = GetEntity().FindComponent(componentTypeName);
		if(hComponent.expired()) {
			if(shouldPrintWarning())
				Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but entity has no component of type '" << componentTypeName << "'!" << Con::endl;
			continue;
		}
		auto &memberName = componentPath->second;
		if(memberName.IsEmpty()) {
			if(shouldPrintWarning())
				Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but no member name has been specified!" << Con::endl;
			continue;
		}
		auto memberPath = memberName;
		auto channelIdx = it - channels.begin();
		CEAnim2InitializeChannelValueSubmitter evData {memberPath};
		if(hComponent->InvokeEventCallbacks(EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER, evData) == util::EventReply::Handled) {
			if(evData.submitter == nullptr)
				continue;
			channelValueSubmitters[channelIdx] = std::move(evData.submitter);
			continue;
		}

		auto memberIdx = hComponent->GetMemberIndex(memberPath.GetString());
		if(!memberIdx.has_value()) {
			if(shouldPrintWarning())
				Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', entity component has no member with name '" << memberName << "'!" << Con::endl;
			continue;
		}
		auto channelValueType = channel->GetValueType();
		auto *memberInfo = hComponent->GetMemberInfo(*memberIdx);
		if(!memberInfo) {
			if(shouldPrintWarning())
				Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but member '" << memberPath.GetString() << "' with index '" << *memberIdx << "' is not valid!" << Con::endl;
			continue;
		}
		auto valueType = memberInfo->type;

		auto &component = *hComponent;
		auto *valueComponents = path.GetComponents();
		auto vsGetMemberChannelSubmitter = [valueComponents, &path, &memberIdx, channelIdx, &channelValueSubmitters, &component]<typename TMember>(auto tag) mutable {
			using TChannel = typename decltype(tag)::type;
			constexpr auto setMemberValue = [](const pragma::ComponentMemberInfo &memberInfo, pragma::BaseEntityComponent &component, const void *value, void *userData) { memberInfo.setterFunction(memberInfo, component, value); };
			if(!valueComponents || valueComponents->empty()) {
				if constexpr(std::is_same_v<TChannel, TMember>)
					channelValueSubmitters[channelIdx] = get_member_channel_submitter<TChannel, TMember, 0>(component, *memberIdx, setMemberValue);
				return;
			}
			constexpr auto channelType = udm::type_to_enum<TChannel>();
			constexpr auto memberType = udm::type_to_enum<TMember>();
			constexpr auto numComponentsChannel = get_component_count(channelType);
			constexpr auto numComponentsMember = get_component_count(memberType);
			if constexpr(numComponentsChannel > 0 && numComponentsMember > 0 && is_type_compatible(channelType, memberType)) {
				if(valueComponents->empty() || valueComponents->size() > numComponentsChannel) {
					Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but member component count is not in the allowed range of [1," << numComponentsChannel << "] for the type of the specified member!" << Con::endl;
					return;
				}
				std::array<uint32_t, numComponentsChannel> componentIndices;
				for(uint32_t idx = 0; auto &strComponent : *valueComponents) {
					switch(memberType) {
					case udm::Type::Vector2:
					case udm::Type::Vector2i:
						{
							if(is_vector_component<0>(strComponent))
								componentIndices[idx] = 0;
							else if(is_vector_component<1>(strComponent))
								componentIndices[idx] = 1;
							else {
								Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but member component '" << strComponent << "' is not recognized as a valid identifier for the member type!" << Con::endl;
								return; // Unknown component type
							}
							break;
						}
					case udm::Type::Vector3:
					case udm::Type::Vector3i:
						{
							if(is_vector_component<0>(strComponent))
								componentIndices[idx] = 0;
							else if(is_vector_component<1>(strComponent))
								componentIndices[idx] = 1;
							else if(is_vector_component<2>(strComponent))
								componentIndices[idx] = 2;
							else {
								Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but member component '" << strComponent << "' is not recognized as a valid identifier for the member type!" << Con::endl;
								return; // Unknown component type
							}
							break;
						}
					case udm::Type::Vector4:
					case udm::Type::Vector4i:
						{
							if(is_vector_component<0>(strComponent))
								componentIndices[idx] = 0;
							else if(is_vector_component<1>(strComponent))
								componentIndices[idx] = 1;
							else if(is_vector_component<2>(strComponent))
								componentIndices[idx] = 2;
							else if(is_vector_component<3>(strComponent))
								componentIndices[idx] = 3;
							else {
								Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but member component '" << strComponent << "' is not recognized as a valid identifier for the member type!" << Con::endl;
								return; // Unknown component type
							}
							break;
						}
					case udm::Type::Quaternion:
						{
							if(strComponent == "w")
								componentIndices[idx] = 0;
							else if(strComponent == "x")
								componentIndices[idx] = 1;
							else if(strComponent == "y")
								componentIndices[idx] = 2;
							else if(strComponent == "z")
								componentIndices[idx] = 3;
							else {
								Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but member component '" << strComponent << "' is not recognized as a valid identifier for the member type!" << Con::endl;
								return; // Unknown component type
							}
							break;
						}
					case udm::Type::EulerAngles:
						{
							if(strComponent == "p")
								componentIndices[idx] = 0;
							else if(strComponent == "y")
								componentIndices[idx] = 1;
							else if(strComponent == "r")
								componentIndices[idx] = 2;
							else {
								Con::cwar << "Attempted to play animation channel with path '" << path.ToUri() << "', but member component '" << strComponent << "' is not recognized as a valid identifier for the member type!" << Con::endl;
								return; // Unknown component type
							}
							break;
						}
					}
					++idx;
				}
				channelValueSubmitters[channelIdx]
				  = runtime_array_to_compile_time<TChannel, TMember, typename decltype(componentIndices)::value_type, 0, componentIndices.size(), numComponentsMember, get_member_channel_submitter_wrapper>(component, *memberIdx, setMemberValue, nullptr, componentIndices);
			}
		};

		auto vs = [&vsGetMemberChannelSubmitter, channelValueType](auto tag) mutable {
			using TMember = typename decltype(tag)::type;
			if constexpr(is_animatable_type_v<TMember>) {
				auto vs = [&vsGetMemberChannelSubmitter](auto tag) {
					using TChannel = typename decltype(tag)::type;
					if constexpr(is_animatable_type_v<TChannel>)
						vsGetMemberChannelSubmitter.template operator()<TMember>(tag);
				};
				if(udm::is_ng_type(channelValueType))
					udm::visit_ng(channelValueType, vs);
			}
		};
		auto udmType = ents::member_type_to_udm_type(valueType);
		if(udm::is_ng_type(udmType))
			udm::visit_ng(udmType, vs);
	}
}

void PanimaComponent::PlayAnimation(panima::AnimationManager &manager, panima::Animation &anim)
{
	manager->SetAnimation(anim);
	InitializeAnimationChannelValueSubmitters(manager);
}
void PanimaComponent::ReloadAnimation(panima::AnimationManager &manager)
{
	auto *anim = manager->GetAnimation();
	if(!anim)
		return;
	auto t = manager->GetCurrentTime();
	PlayAnimation(manager, *const_cast<panima::Animation *>(anim));
	manager->SetCurrentTime(t);
}
void PanimaComponent::ClearAnimationManagers() { m_animationManagers.clear(); }
bool PanimaComponent::UpdateAnimations(double dt)
{
	if(GetPlaybackRate() == 0.f)
		return false;
	return MaintainAnimations(dt);
}
bool PanimaComponent::MaintainAnimations(double dt)
{
	CEAnim2MaintainAnimations evData {dt};
	if(InvokeEventCallbacks(EVENT_MAINTAIN_ANIMATIONS, evData) == util::EventReply::Handled) {
		InvokeEventCallbacks(EVENT_ON_ANIMATIONS_UPDATED);
		return true;
	}

	AdvanceAnimations(dt);
	InvokeEventCallbacks(EVENT_ON_ANIMATIONS_UPDATED);
	return true;
}

float PanimaComponent::GetCurrentTime(panima::AnimationManager &manager) const { return manager->GetCurrentTime(); }
void PanimaComponent::SetCurrentTime(panima::AnimationManager &manager, float time)
{
	if(manager->GetCurrentTime() == time)
		return;
	manager->SetCurrentTime(time, true);
	InvokeValueSubmitters(manager);
}

float PanimaComponent::GetCurrentTimeFraction(panima::AnimationManager &manager) const { return manager->GetCurrentTimeFraction(); }
void PanimaComponent::SetCurrentTimeFraction(panima::AnimationManager &manager, float t)
{
	if(manager->GetCurrentTimeFraction() == t)
		return;
	manager->SetCurrentTimeFraction(t, true);
	InvokeValueSubmitters(manager);
}
void PanimaComponent::InvokeValueSubmitters(panima::AnimationManager &manager)
{
	auto *anim = manager.GetCurrentAnimation();
	if(!anim)
		return;
	auto &channelValueSubmitters = manager.GetChannelValueSubmitters();
	auto &channels = anim->GetChannels();
	auto n = umath::min(channelValueSubmitters.size(), channels.size());
	auto t = manager->GetCurrentTime();
	for(auto i = decltype(n) {0u}; i < n; ++i) {
		auto &submitter = channelValueSubmitters[i];
		if(!submitter)
			continue;
		auto &channel = channels[i];
		if(channel->GetTimeCount() == 0)
			continue;
		submitter(*channel, manager->GetLastChannelTimestampIndex(i), t);
	}
}

void PanimaComponent::AdvanceAnimations(double dt)
{
	auto &ent = GetEntity();
	auto pTimeScaleComponent = ent.GetTimeScaleComponent();
	dt *= (pTimeScaleComponent.valid() ? pTimeScaleComponent->GetEffectiveTimeScale() : 1.f);
	dt *= GetPlaybackRate();
	for(auto &pair : m_animationManagers) {
		auto &manager = pair.second;
		auto change = (*manager)->Advance(dt, true /* forceUpdate */);
		if(!change)
			continue;
		InvokeValueSubmitters(*manager);
	}
}
void PanimaComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PanimaComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(BaseEntityComponent::EVENT_ON_MEMBERS_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { InitializeAnimationChannelValueSubmitters(); });
}

void PanimaComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	GetNetworkState().GetGameState()->GetAnimationUpdateManager().UpdateEntityState(GetEntity());
}

void PanimaComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	GetNetworkState().GetGameState()->GetAnimationUpdateManager().UpdateEntityState(GetEntity());
}

void PanimaComponent::ReloadAnimation()
{
	InitializeAnimationChannelValueSubmitters();
	for(auto &pair : m_animationManagers)
		ReloadAnimation(*pair.second);
}
void PanimaComponent::Save(udm::LinkedPropertyWrapperArg udm) {}
void PanimaComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) {}
void PanimaComponent::ResetAnimation(const std::shared_ptr<Model> &mdl) {}

/////////////////

CEAnim2MaintainAnimations::CEAnim2MaintainAnimations(double deltaTime) : deltaTime {deltaTime} {}
void CEAnim2MaintainAnimations::PushArguments(lua_State *l) { Lua::PushNumber(l, deltaTime); }

/////////////////

CEAnim2TranslateAnimation::CEAnim2TranslateAnimation(const panima::AnimationSet &set, panima::AnimationId &animation, panima::PlaybackFlags &flags) : set {set}, animation(animation), flags(flags) {}
void CEAnim2TranslateAnimation::PushArguments(lua_State *l)
{
	Lua::Push<const panima::AnimationSet *>(l, &set);
	Lua::PushInt(l, animation);
	Lua::PushInt(l, umath::to_integral(flags));
}
uint32_t CEAnim2TranslateAnimation::GetReturnCount() { return 2; }
void CEAnim2TranslateAnimation::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l, -2))
		animation = Lua::CheckInt(l, -2);
	if(Lua::IsSet(l, -1))
		flags = static_cast<panima::PlaybackFlags>(Lua::CheckInt(l, -1));
}

/////////////////

CEAnim2OnAnimationStart::CEAnim2OnAnimationStart(const panima::AnimationSet &set, int32_t animation, Activity activity, panima::PlaybackFlags flags) : set {set}, animation(animation), activity(activity), flags(flags) {}
void CEAnim2OnAnimationStart::PushArguments(lua_State *l)
{
	Lua::Push<const panima::AnimationSet *>(l, &set);
	Lua::PushInt(l, animation);
	Lua::PushInt(l, umath::to_integral(activity));
	Lua::PushInt(l, umath::to_integral(flags));
}

/////////////////

CEAnim2OnAnimationComplete::CEAnim2OnAnimationComplete(const panima::AnimationSet &set, int32_t animation, Activity activity) : set {set}, animation(animation), activity(activity) {}
void CEAnim2OnAnimationComplete::PushArguments(lua_State *l)
{
	Lua::Push<const panima::AnimationSet *>(l, &set);
	Lua::PushInt(l, animation);
	Lua::PushInt(l, umath::to_integral(activity));
}

/////////////////

CEAnim2HandleAnimationEvent::CEAnim2HandleAnimationEvent(const AnimationEvent &animationEvent) : animationEvent(animationEvent) {}
void CEAnim2HandleAnimationEvent::PushArguments(lua_State *l)
{
	Lua::PushInt(l, static_cast<int32_t>(animationEvent.eventID));

	auto tArgs = Lua::CreateTable(l);
	auto &args = animationEvent.arguments;
	for(auto i = decltype(args.size()) {0}; i < args.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::PushString(l, args.at(i));
		Lua::SetTableValue(l, tArgs);
	}
}
void CEAnim2HandleAnimationEvent::PushArgumentVariadic(lua_State *l)
{
	auto &args = animationEvent.arguments;
	for(auto &arg : args)
		Lua::PushString(l, arg);
}

/////////////////

CEAnim2OnPlayAnimation::CEAnim2OnPlayAnimation(const panima::AnimationSet &set, panima::AnimationId animation, panima::PlaybackFlags flags) : set {set}, animation(animation), flags(flags) {}
void CEAnim2OnPlayAnimation::PushArguments(lua_State *l)
{
	Lua::Push<const panima::AnimationSet *>(l, &set);
	Lua::PushInt(l, animation);
	Lua::PushInt(l, umath::to_integral(flags));
}

/////////////////

CEAnim2InitializeChannelValueSubmitter::CEAnim2InitializeChannelValueSubmitter(util::Path &path) : path {path} {}
void CEAnim2InitializeChannelValueSubmitter::PushArguments(lua_State *l) {}
uint32_t CEAnim2InitializeChannelValueSubmitter::GetReturnCount() { return 0; }
void CEAnim2InitializeChannelValueSubmitter::HandleReturnValues(lua_State *l) {}
