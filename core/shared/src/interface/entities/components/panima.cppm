// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

#undef GetCurrentTime

export module pragma.shared:entities.components.panima;

export import :entities.components.base;
export import :game.animation_channel_cache_data;
export import :model.animation.animation_event;
export import :model.animation.enums;
export import :types;
import panima;

export {
	namespace pragma {
		class GlobalAnimationChannelQueueProcessor;
		struct DLLNETWORK AnimationManagerData {
			std::string name;
			panima::PAnimationManager animationManager;
			std::vector<AnimationChannelCacheData> channelCache;
			bool isChannelCacheDirty = false;
		};
		namespace panimaComponent {
			REGISTER_COMPONENT_EVENT(EVENT_HANDLE_ANIMATION_EVENT)
			REGISTER_COMPONENT_EVENT(EVENT_ON_PLAY_ANIMATION)
			REGISTER_COMPONENT_EVENT(EVENT_ON_ANIMATION_COMPLETE)
			REGISTER_COMPONENT_EVENT(EVENT_ON_ANIMATION_START)
			REGISTER_COMPONENT_EVENT(EVENT_MAINTAIN_ANIMATIONS)
			REGISTER_COMPONENT_EVENT(EVENT_ON_ANIMATIONS_UPDATED)
			REGISTER_COMPONENT_EVENT(EVENT_PLAY_ANIMATION)
			REGISTER_COMPONENT_EVENT(EVENT_TRANSLATE_ANIMATION)
			REGISTER_COMPONENT_EVENT(EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER)
		}
		class DLLNETWORK PanimaComponent final : public BaseEntityComponent {
		  public:
			enum class PropertyFlags : uint8_t {
				None = 0,
				Disabled = 1u,
				AlwaysDirty = Disabled << 1u,
			};

			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static std::optional<std::pair<std::string, util::Path>> ParseComponentChannelPath(const panima::ChannelPath &path);

			PanimaComponent(ecs::BaseEntity &ent);
			void SetPlaybackRate(float rate);
			float GetPlaybackRate() const;
			const util::PFloatProperty &GetPlaybackRateProperty() const;

			std::vector<std::shared_ptr<AnimationManagerData>> &GetAnimationManagers() { return m_animationManagers; }
			const std::vector<std::shared_ptr<AnimationManagerData>> &GetAnimationManagers() const { return const_cast<PanimaComponent *>(this)->GetAnimationManagers(); }
			panima::PAnimationManager AddAnimationManager(std::string name, int32_t priority = 0);
			panima::PAnimationManager GetAnimationManager(std::string name);
			void RemoveAnimationManager(const panima::AnimationManager &player);
			void RemoveAnimationManager(const std::string_view &name);
			void ClearAnimationManagers();

			bool UpdateAnimations(GlobalAnimationChannelQueueProcessor &channelQueueProcessor, double dt);
			bool MaintainAnimations(GlobalAnimationChannelQueueProcessor &channelQueueProcessor, double dt);
			void AdvanceAnimations(GlobalAnimationChannelQueueProcessor &channelQueueProcessor, double dt);
			void ApplyAnimationValues(GlobalAnimationChannelQueueProcessor *channelQueueProcessor);
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

			bool SetPropertyFlag(const std::string &propName, PropertyFlags flag, bool enabled);
			bool IsPropertyFlagSet(const std::string &propName, PropertyFlags flag) const;

			bool SetPropertyAlwaysDirty(const std::string &propName, bool alwaysDirty);
			bool IsPropertyAlwaysDirty(const std::string &propName) const;

			void SetPropertyEnabled(const std::string &propName, bool enabled);
			bool IsPropertyEnabled(const std::string &propName) const;
			bool IsPropertyAnimated(panima::AnimationManager &manager, const std::string &propName) const;
			void UpdateAnimationChannelSubmitters();

			// Return the animation value for the specified property WITHOUT applying math expressions
			bool GetRawAnimatedPropertyValue(panima::AnimationManager &manager, const std::string &propName, udm::Type type, void *outValue) const;
			bool GetRawPropertyValue(panima::AnimationManager &manager, const std::string &propName, udm::Type type, void *outValue) const;

			virtual void InitializeLuaObject(lua::State *l) override;
			virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
			using BaseEntityComponent::Load;
		  protected:
			bool UnsetPropertyFlags(const char *propName, PropertyFlags flags);
			virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
			void UpdateAnimationData(GlobalAnimationChannelQueueProcessor *channelQueueProcessor, AnimationManagerData &amd);
			bool GetRawAnimatedPropertyValue(panima::AnimationManager &manager, const std::string &propName, udm::Type type, void *outValue, const ComponentMemberInfo **optOutMemberInfo, BaseEntityComponent **optOutComponent) const;
			std::vector<std::shared_ptr<AnimationManagerData>>::iterator FindAnimationManager(const std::string_view &name);
			AnimationManagerData *FindAnimationManagerData(panima::AnimationManager &manager);
			void InitializeAnimationChannelValueSubmitters();
			void InitializeAnimationChannelValueSubmitters(AnimationManagerData &amData);
			void ResetAnimation(const std::shared_ptr<asset::Model> &mdl);
			util::PFloatProperty m_playbackRate = nullptr;
			std::vector<std::shared_ptr<AnimationManagerData>> m_animationManagers;
			std::unordered_map<const char *, PropertyFlags> m_propertyFlags;
		};

		struct DLLNETWORK CEAnim2OnAnimationComplete : public ComponentEvent {
			CEAnim2OnAnimationComplete(const panima::AnimationSet &set, int32_t animation, Activity activity);
			virtual void PushArguments(lua::State *l) override;
			const panima::AnimationSet &set;
			int32_t animation;
			Activity activity;
		};
		struct DLLNETWORK CEAnim2HandleAnimationEvent : public ComponentEvent {
			CEAnim2HandleAnimationEvent(const AnimationEvent &animationEvent);
			virtual void PushArguments(lua::State *l) override;
			void PushArgumentVariadic(lua::State *l);
			const AnimationEvent &animationEvent;
		};
		struct DLLNETWORK CEAnim2OnPlayAnimation : public ComponentEvent {
			CEAnim2OnPlayAnimation(const panima::AnimationSet &set, panima::AnimationId animation, panima::PlaybackFlags flags);
			virtual void PushArguments(lua::State *l) override;
			const panima::AnimationSet &set;
			panima::AnimationId animation;
			panima::PlaybackFlags flags;
		};
		struct DLLNETWORK CEAnim2OnAnimationStart : public ComponentEvent {
			CEAnim2OnAnimationStart(const panima::AnimationSet &set, int32_t animation, Activity activity, panima::PlaybackFlags flags);
			virtual void PushArguments(lua::State *l) override;
			const panima::AnimationSet &set;
			int32_t animation;
			Activity activity;
			panima::PlaybackFlags flags;
		};
		struct DLLNETWORK CEAnim2TranslateAnimation : public ComponentEvent {
			CEAnim2TranslateAnimation(const panima::AnimationSet &set, panima::AnimationId &animation, panima::PlaybackFlags &flags);
			virtual void PushArguments(lua::State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua::State *l) override;
			const panima::AnimationSet &set;
			panima::AnimationId &animation;
			panima::PlaybackFlags &flags;
		};
		struct DLLNETWORK CEAnim2MaintainAnimations : public ComponentEvent {
			CEAnim2MaintainAnimations(double deltaTime);
			virtual void PushArguments(lua::State *l) override;
			double deltaTime;
		};
		struct DLLNETWORK CEAnim2InitializeChannelValueSubmitter : public ComponentEvent {
			CEAnim2InitializeChannelValueSubmitter(util::Path &path);
			virtual void PushArguments(lua::State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua::State *l) override;
			util::Path &path;
			panima::ChannelValueSubmitter submitter {};
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::PanimaComponent::PropertyFlags)
};
