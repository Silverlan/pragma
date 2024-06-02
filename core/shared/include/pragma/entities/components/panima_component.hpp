/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PANIMA_COMPONENT_HPP__
#define __PANIMA_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/types.hpp"
#include <panima/types.hpp>
#include <sharedutils/property/util_property.hpp>
#include <unordered_set>

#undef GetCurrentTime

struct AnimationEvent;
namespace util {
	class Path;
};
namespace pragma {
	class DLLNETWORK PanimaComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_HANDLE_ANIMATION_EVENT;
		static ComponentEventId EVENT_ON_PLAY_ANIMATION;
		static ComponentEventId EVENT_ON_ANIMATION_COMPLETE;
		static ComponentEventId EVENT_ON_ANIMATION_START;
		static ComponentEventId EVENT_MAINTAIN_ANIMATIONS;
		static ComponentEventId EVENT_ON_ANIMATIONS_UPDATED;
		static ComponentEventId EVENT_PLAY_ANIMATION;
		static ComponentEventId EVENT_TRANSLATE_ANIMATION;
		static ComponentEventId EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static std::optional<std::pair<std::string, util::Path>> ParseComponentChannelPath(const panima::ChannelPath &path);

		PanimaComponent(BaseEntity &ent);
		void SetPlaybackRate(float rate);
		float GetPlaybackRate() const;
		const util::PFloatProperty &GetPlaybackRateProperty() const;

		std::vector<std::pair<std::string, panima::PAnimationManager>> &GetAnimationManagers() { return m_animationManagers; }
		const std::vector<std::pair<std::string, panima::PAnimationManager>> &GetAnimationManagers() const { return const_cast<PanimaComponent *>(this)->GetAnimationManagers(); }
		panima::PAnimationManager AddAnimationManager(std::string name, int32_t priority = 0);
		panima::PAnimationManager GetAnimationManager(std::string name);
		void RemoveAnimationManager(const panima::AnimationManager &player);
		void RemoveAnimationManager(const std::string_view &name);
		void ClearAnimationManagers();

		bool UpdateAnimations(double dt);
		bool MaintainAnimations(double dt);
		void AdvanceAnimations(double dt);
		void DebugPrint(std::stringstream &ss);
		void DebugPrint();

		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		void PlayAnimation(panima::AnimationManager &manager, panima::Animation &anim);
		void ReloadAnimation(panima::AnimationManager &manager);
		float GetCurrentTime(panima::AnimationManager &manager) const;
		void SetCurrentTime(panima::AnimationManager &manager, float time);
		float GetCurrentTimeFraction(panima::AnimationManager &manager) const;
		void SetCurrentTimeFraction(panima::AnimationManager &manager, float t);
		void ReloadAnimation();

		void SetPropertyEnabled(const std::string &propName, bool enabled);
		bool IsPropertyEnabled(const std::string &propName) const;
		bool IsPropertyAnimated(panima::AnimationManager &manager, const std::string &propName) const;
		void UpdateAnimationChannelSubmitters();

		// Return the animation value for the specified property WITHOUT applying math expressions
		bool GetRawAnimatedPropertyValue(panima::AnimationManager &manager, const std::string &propName, udm::Type type, void *outValue) const;
		bool GetRawPropertyValue(panima::AnimationManager &manager, const std::string &propName, udm::Type type, void *outValue) const;

		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		using BaseEntityComponent::Load;
	  protected:
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		void InvokeValueSubmitters(panima::AnimationManager &manager);
		bool GetRawAnimatedPropertyValue(panima::AnimationManager &manager, const std::string &propName, udm::Type type, void *outValue, const ComponentMemberInfo **optOutMemberInfo, pragma::BaseEntityComponent **optOutComponent) const;
		std::vector<std::pair<std::string, panima::PAnimationManager>>::iterator FindAnimationManager(const std::string_view &name);
		void InitializeAnimationChannelValueSubmitters();
		void InitializeAnimationChannelValueSubmitters(panima::AnimationManager &manager);
		void ResetAnimation(const std::shared_ptr<Model> &mdl);
		util::PFloatProperty m_playbackRate = nullptr;
		std::vector<std::pair<std::string, panima::PAnimationManager>> m_animationManagers;
		std::unordered_set<const char *> m_disabledProperties;
	};

	struct DLLNETWORK CEAnim2OnAnimationComplete : public ComponentEvent {
		CEAnim2OnAnimationComplete(const panima::AnimationSet &set, int32_t animation, Activity activity);
		virtual void PushArguments(lua_State *l) override;
		const panima::AnimationSet &set;
		int32_t animation;
		Activity activity;
	};
	struct DLLNETWORK CEAnim2HandleAnimationEvent : public ComponentEvent {
		CEAnim2HandleAnimationEvent(const AnimationEvent &animationEvent);
		virtual void PushArguments(lua_State *l) override;
		void PushArgumentVariadic(lua_State *l);
		const AnimationEvent &animationEvent;
	};
	struct DLLNETWORK CEAnim2OnPlayAnimation : public ComponentEvent {
		CEAnim2OnPlayAnimation(const panima::AnimationSet &set, panima::AnimationId animation, panima::PlaybackFlags flags);
		virtual void PushArguments(lua_State *l) override;
		const panima::AnimationSet &set;
		panima::AnimationId animation;
		panima::PlaybackFlags flags;
	};
	struct DLLNETWORK CEAnim2OnAnimationStart : public ComponentEvent {
		CEAnim2OnAnimationStart(const panima::AnimationSet &set, int32_t animation, Activity activity, panima::PlaybackFlags flags);
		virtual void PushArguments(lua_State *l) override;
		const panima::AnimationSet &set;
		int32_t animation;
		Activity activity;
		panima::PlaybackFlags flags;
	};
	struct DLLNETWORK CEAnim2TranslateAnimation : public ComponentEvent {
		CEAnim2TranslateAnimation(const panima::AnimationSet &set, panima::AnimationId &animation, panima::PlaybackFlags &flags);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		const panima::AnimationSet &set;
		panima::AnimationId &animation;
		panima::PlaybackFlags &flags;
	};
	struct DLLNETWORK CEAnim2MaintainAnimations : public ComponentEvent {
		CEAnim2MaintainAnimations(double deltaTime);
		virtual void PushArguments(lua_State *l) override;
		double deltaTime;
	};
	struct DLLNETWORK CEAnim2InitializeChannelValueSubmitter : public ComponentEvent {
		CEAnim2InitializeChannelValueSubmitter(util::Path &path);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		util::Path &path;
		panima::ChannelValueSubmitter submitter {};
	};
};

#endif
