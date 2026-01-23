// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.base_animated;

export import :entities.components.base;
export import :model.animation;
export import pragma.util;

export {
	namespace pragma {
		namespace baseAnimatedComponent {
			REGISTER_COMPONENT_EVENT(EVENT_HANDLE_ANIMATION_EVENT)
			REGISTER_COMPONENT_EVENT(EVENT_ON_PLAY_ANIMATION)
			REGISTER_COMPONENT_EVENT(EVENT_ON_PLAY_LAYERED_ANIMATION)
			REGISTER_COMPONENT_EVENT(EVENT_ON_PLAY_LAYERED_ACTIVITY)
			REGISTER_COMPONENT_EVENT(EVENT_ON_ANIMATION_COMPLETE)
			REGISTER_COMPONENT_EVENT(EVENT_ON_LAYERED_ANIMATION_START)
			REGISTER_COMPONENT_EVENT(EVENT_ON_LAYERED_ANIMATION_COMPLETE)
			REGISTER_COMPONENT_EVENT(EVENT_ON_ANIMATION_START)
			REGISTER_COMPONENT_EVENT(EVENT_TRANSLATE_LAYERED_ANIMATION)
			REGISTER_COMPONENT_EVENT(EVENT_TRANSLATE_ANIMATION)
			REGISTER_COMPONENT_EVENT(EVENT_TRANSLATE_ACTIVITY)
			REGISTER_COMPONENT_EVENT(EVENT_MAINTAIN_ANIMATIONS)
			REGISTER_COMPONENT_EVENT(EVENT_MAINTAIN_ANIMATION_MT)
			REGISTER_COMPONENT_EVENT(EVENT_MAINTAIN_ANIMATION_MOVEMENT)
			REGISTER_COMPONENT_EVENT(EVENT_SHOULD_UPDATE_BONES)
			REGISTER_COMPONENT_EVENT(EVENT_ON_PLAY_ACTIVITY)
			REGISTER_COMPONENT_EVENT(EVENT_ON_STOP_LAYERED_ANIMATION)
			REGISTER_COMPONENT_EVENT(EVENT_ON_BONE_TRANSFORM_CHANGED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_ANIMATIONS_UPDATED_MT)
			REGISTER_COMPONENT_EVENT(EVENT_UPDATE_BONE_POSES_MT)
			REGISTER_COMPONENT_EVENT(EVENT_ON_BONE_POSES_FINALIZED_MT)
			REGISTER_COMPONENT_EVENT(EVENT_ON_BLEND_ANIMATION_MT)
			REGISTER_COMPONENT_EVENT(EVENT_PLAY_ANIMATION)
			REGISTER_COMPONENT_EVENT(EVENT_ON_ANIMATION_RESET)
			REGISTER_COMPONENT_EVENT(EVENT_ON_ANIMATIONS_UPDATED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_UPDATE_SKELETON)
			REGISTER_COMPONENT_EVENT(EVENT_POST_ANIMATION_UPDATE)
			REGISTER_COMPONENT_EVENT(EVENT_ON_RESET_POSE)
		}
		class DLLNETWORK BaseAnimatedComponent : public BaseEntityComponent, public DynamicMemberRegister {
		  public:
			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

			enum class StateFlags : uint8_t {
				None = 0u,
				AbsolutePosesDirty = 1u,
				BaseAnimationDirty = AbsolutePosesDirty << 1u,
				IsAnimated = BaseAnimationDirty << 1u,
				SkeletonUpdateListenerEnabled = IsAnimated << 1u,
				NeedsPostAnimationUpdate = SkeletonUpdateListenerEnabled << 1u,
			};

			struct DLLNETWORK AnimationSlotInfo {
			  public:
				AnimationSlotInfo() = default;
				AnimationSlotInfo(int32_t panimation, int32_t panimationLast = -1) : animation(panimation), cycle(0.f) {}
				Activity activity = Activity::Invalid;
				int32_t animation = -1;
				float cycle = 0.f;
				FPlayAnim flags = FPlayAnim::Default;
				std::vector<math::Transform> bonePoses;
				std::vector<Vector3> boneScales;

				// These are only used if the animation has a blend-controller
				std::vector<math::Transform> bonePosesBc;
				std::vector<Vector3> boneScalesBc;

				// Keep a reference to our last animation for blending
				struct {
					int32_t animation = -1;
					float cycle = 0.f;
					FPlayAnim flags = FPlayAnim::Default;
					std::pair<float, float> blendTimeScale = {0.f, 0.f};
					float blendScale = 1.f;
				} lastAnim;
			};
			static bool GetBlendFramesFromCycle(animation::Animation &anim, float cycle, Frame **outFrameSrc, Frame **outFrameDst, float &outInterpFactor, int32_t frameOffset = 0);

			virtual void Initialize() override;
			virtual void OnEntitySpawn() override;
			virtual void OnRemove() override;

			virtual void MaintainAnimationMovement(const Vector3 &disp);

			bool GetReferenceBonePose(animation::BoneId boneId, math::Transform &outPose, math::CoordinateSpace space = math::CoordinateSpace::Object) const;
			bool GetReferenceBonePose(animation::BoneId boneId, math::ScaledTransform &outPose, math::CoordinateSpace space = math::CoordinateSpace::Object) const;
			bool GetReferenceBonePos(animation::BoneId boneId, Vector3 &outPos, math::CoordinateSpace space = math::CoordinateSpace::Object) const;
			bool GetReferenceBoneRot(animation::BoneId boneId, Quat &outRot, math::CoordinateSpace space = math::CoordinateSpace::Object) const;
			bool GetReferenceBoneScale(animation::BoneId boneId, Vector3 &outScale, math::CoordinateSpace space = math::CoordinateSpace::Object) const;
			bool GetReferenceBonePose(animation::BoneId boneId, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale = nullptr, math::CoordinateSpace space = math::CoordinateSpace::Object) const;

			bool GetBonePose(animation::BoneId boneId, math::Transform &outPose, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
			bool GetBonePose(animation::BoneId boneId, math::ScaledTransform &outPose, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
			bool GetBonePos(animation::BoneId boneId, Vector3 &outPos, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
			bool GetBoneRot(animation::BoneId boneId, Quat &outRot, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
			bool GetBoneScale(animation::BoneId boneId, Vector3 &outScale, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
			bool GetBonePose(animation::BoneId boneId, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale = nullptr, math::CoordinateSpace space = math::CoordinateSpace::Local) const;

			bool SetBonePose(animation::BoneId boneId, const math::Transform &pose, math::CoordinateSpace space = math::CoordinateSpace::Local);
			bool SetBonePose(animation::BoneId boneId, const math::ScaledTransform &pose, math::CoordinateSpace space = math::CoordinateSpace::Local);
			bool SetBonePos(animation::BoneId boneId, const Vector3 &pos, math::CoordinateSpace space = math::CoordinateSpace::Local);
			bool SetBoneRot(animation::BoneId boneId, const Quat &rot, math::CoordinateSpace space = math::CoordinateSpace::Local);
			bool SetBoneScale(animation::BoneId boneId, const Vector3 &scale, math::CoordinateSpace space = math::CoordinateSpace::Local);
			bool SetBonePose(animation::BoneId boneId, const Vector3 *optPos, const Quat *optRot, const Vector3 *optScale = nullptr, math::CoordinateSpace space = math::CoordinateSpace::Local);

			void SetSkeletonUpdateListenerEnabled(bool enabled);
			bool IsSkeletonUpdateListenerEnabled() const;

			bool IsPlayingAnimation() const;
			bool CalcAnimationMovementSpeed(float *x, float *z, int32_t frameOffset = 0) const;

			std::optional<Mat4> GetBoneMatrix(unsigned int boneID) const;

			bool IsAnimated() const;
			FPlayAnim GetBaseAnimationFlags() const;
			void SetBaseAnimationFlags(FPlayAnim flags);

			std::optional<FPlayAnim> GetLayeredAnimationFlags(uint32_t layerIdx) const;
			void SetLayeredAnimationFlags(uint32_t layerIdx, FPlayAnim flags);

			std::optional<animation::BoneId> GetMetaBoneId(animation::MetaRigBoneType boneType) const;
			bool SetMetaBonePose(animation::MetaRigBoneType boneType, const math::ScaledTransform &pose, math::CoordinateSpace space = math::CoordinateSpace::Local);
			bool SetMetaBonePose(animation::MetaRigBoneType boneType, const Vector3 *optPos, const Quat *optRot = nullptr, const Vector3 *optScale = nullptr, math::CoordinateSpace space = math::CoordinateSpace::Local);
			bool GetMetaBonePose(animation::MetaRigBoneType boneType, math::ScaledTransform &outPose, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
			bool GetMetaBonePose(animation::MetaRigBoneType boneType, Vector3 *optOutPos, Quat *optOutRot = nullptr, Vector3 *optOutScale = nullptr, math::CoordinateSpace space = math::CoordinateSpace::Local) const;
			bool MetaBonePoseToSkeletal(animation::MetaRigBoneType boneType, math::ScaledTransform &pose) const;
			bool MetaBoneRotationToSkeletal(animation::MetaRigBoneType boneType, Quat &rot) const;

			float GetCycle() const;
			void SetCycle(float cycle);
			animation::Animation *GetAnimationObject() const;
			int32_t GetAnimation() const;

			void ResetPose();

			Activity GetActivity() const;
			int32_t GetLayeredAnimation(uint32_t slot) const;
			Activity GetLayeredActivity(uint32_t slot) const;

			virtual void PlayAnimation(int animation, FPlayAnim flags = FPlayAnim::Default);
			virtual void PlayLayeredAnimation(int slot, int animation, FPlayAnim flags = FPlayAnim::Default);
			bool PlayActivity(Activity activity, FPlayAnim flags = FPlayAnim::Default);
			bool PlayLayeredActivity(int slot, Activity activity, FPlayAnim flags = FPlayAnim::Default);
			bool PlayLayeredAnimation(int slot, std::string animation, FPlayAnim flags = FPlayAnim::Default);
			virtual void StopLayeredAnimation(int slot);
			void StopLayeredAnimations();
			bool PlayAnimation(const std::string &animation, FPlayAnim flags = FPlayAnim::Default);
			void SetPlaybackRate(float rate);
			float GetPlaybackRate() const;
			const util::PFloatProperty &GetPlaybackRateProperty() const;
			int32_t SelectTranslatedAnimation(Activity &inOutActivity) const;
			int SelectWeightedAnimation(Activity activity, int animAvoid = -1) const;
			// Returns the time left until the current animation has finished playing
			float GetAnimationDuration() const;

			void SetBlendController(unsigned int controller, float val);
			void SetBlendController(const std::string &controller, float val);
			float GetBlendController(const std::string &controller) const;
			float GetBlendController(unsigned int controller) const;
			const std::unordered_map<unsigned int, float> &GetBlendControllers() const;

			void HandleAnimationEvents();
			void AddAnimationEvent(const std::string &name, uint32_t frameId, const AnimationEvent &ev);
			CallbackHandle AddAnimationEvent(const std::string &name, uint32_t frameId, const std::function<void(void)> &f);
			CallbackHandle AddAnimationEvent(const std::string &name, uint32_t frameId, const CallbackHandle &cb);

			void AddAnimationEvent(uint32_t animId, uint32_t frameId, const AnimationEvent &ev);
			CallbackHandle AddAnimationEvent(uint32_t animId, uint32_t frameId, const std::function<void(void)> &f);
			CallbackHandle AddAnimationEvent(uint32_t animId, uint32_t frameId, const CallbackHandle &cb);
			void ClearAnimationEvents();
			void ClearAnimationEvents(uint32_t animId);
			void ClearAnimationEvents(uint32_t animId, uint32_t frameId);
			void InjectAnimationEvent(const AnimationEvent &ev);

			void ClearAnimationEvents(const std::string &anim);
			void ClearAnimationEvents(const std::string &anim, uint32_t frameId);
			//void RemoveAnimationEvent(uint32_t animId,uint32_t frameId,uint32_t idx);

			void SetLastAnimationBlendScale(float scale);

			const std::vector<math::ScaledTransform> &GetProcessedBones() const;
			std::vector<math::ScaledTransform> &GetProcessedBones();

			// Transforms all bone positions / rotations to entity space
			bool UpdateSkeleton();
			virtual bool UpdateBonePoses();
			void PostAnimationsUpdated();

			void SetPostAnimationUpdateEnabled(bool enabled);
			bool IsPostAnimationUpdateEnabled() const;

			bool ShouldUpdateBones() const;
			UInt32 GetBoneCount() const;
			const std::vector<math::ScaledTransform> &GetBoneTransforms() const;
			std::vector<math::ScaledTransform> &GetBoneTransforms();
			const std::vector<math::ScaledTransform> &GetProcessedBoneTransforms() const;
			std::vector<math::ScaledTransform> &GetProcessedBoneTransforms();
			const AnimationSlotInfo &GetBaseAnimationInfo() const;
			AnimationSlotInfo &GetBaseAnimationInfo();
			const std::unordered_map<uint32_t, AnimationSlotInfo> &GetAnimationSlotInfos() const;
			std::unordered_map<uint32_t, AnimationSlotInfo> &GetAnimationSlotInfos();

			Activity TranslateActivity(Activity act);
			void SetBaseAnimationDirty();
			void SetAbsolutePosesDirty();
			void ClearPreviousAnimation();

			void BlendBonePoses(const std::vector<math::Transform> &srcBonePoses, const std::vector<Vector3> *optSrcBoneScales, const std::vector<math::Transform> &dstBonePoses, const std::vector<Vector3> *optDstBoneScales, std::vector<math::Transform> &outBonePoses,
			  std::vector<Vector3> *optOutBoneScales, animation::Animation &anim, float interpFactor) const;
			void BlendBoneFrames(std::vector<math::Transform> &tgt, std::vector<Vector3> *tgtScales, std::vector<math::Transform> &add, std::vector<Vector3> *addScales, float blendScale) const;

			bool PreMaintainAnimations(double dt);
			virtual bool MaintainAnimations(double dt);
			void UpdateAnimations(double dt);
			bool MaintainGestures(double dt);

			virtual bool GetVertexTransformMatrix(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, math::ScaledTransform &outPose) const;
			virtual std::optional<Mat4> GetVertexTransformMatrix(const geometry::ModelSubMesh &subMesh, uint32_t vertexId) const;
			virtual bool GetLocalVertexPosition(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, Vector3 &pos, const std::optional<Vector3> &vertexOffset = {}) const;
			bool GetVertexPosition(uint32_t meshGroupId, uint32_t meshId, uint32_t subMeshId, uint32_t vertexId, Vector3 &pos) const;
			bool GetVertexPosition(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, Vector3 &pos) const;

			void SetBindPose(const Frame &frame);
			const Frame *GetBindPose() const;

			CallbackHandle BindAnimationEvent(AnimationEvent::Type eventId, const std::function<void(std::reference_wrapper<const AnimationEvent>)> &fCallback);
			virtual const ComponentMemberInfo *GetMemberInfo(ComponentMemberIndex idx) const override;

			virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
			using BaseEntityComponent::Load;
		  protected:
			BaseAnimatedComponent(ecs::BaseEntity &ent);
			virtual std::optional<ComponentMemberIndex> DoGetMemberIndex(const std::string &name) const override;
			virtual void OnModelChanged(const std::shared_ptr<asset::Model> &mdl);
			virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
			virtual void ResetAnimation(const std::shared_ptr<asset::Model> &mdl);

			struct DLLNETWORK AnimationBlendInfo {
				float scale = 0.f;
				animation::Animation *animation = nullptr;
				Frame *frameSrc = nullptr; // Frame to blend from
				Frame *frameDst = nullptr; // Frame to blend to
			};

			struct AnimationEventQueueItem {
				int32_t animId = -1;
				std::shared_ptr<animation::Animation> animation = nullptr;
				int32_t lastFrame = -1;
				uint32_t frameId = 0;
			};

			// Custom animation events
			struct CustomAnimationEvent : public AnimationEvent {
				std::pair<bool, CallbackHandle> callback = {false, {}};
				CustomAnimationEvent(const AnimationEvent &ev);
				CustomAnimationEvent(const std::function<void(void)> &f);
				CustomAnimationEvent(const CallbackHandle &cb);
				CustomAnimationEvent() = default;
			};
			struct TemplateAnimationEvent {
				TemplateAnimationEvent() = default;
				TemplateAnimationEvent(const TemplateAnimationEvent &) = default;
				int32_t animId = -1;
				uint32_t frameId = 0;
				std::string anim;
				CustomAnimationEvent ev = {};
			};
			//

			bool MaintainAnimation(AnimationSlotInfo &animInfo, double dt, int32_t layeredSlot = -1);
			virtual void ApplyAnimationBlending(AnimationSlotInfo &animInfo, double tDelta);
			void HandleAnimationEvent(const AnimationEvent &ev);
			void PlayLayeredAnimation(int slot, int animation, FPlayAnim flags, AnimationSlotInfo **animInfo);
			void GetAnimationBlendController(animation::Animation *anim, float cycle, std::array<AnimationBlendInfo, 2> &bcFrames, float *blendScale) const;
			Frame *GetPreviousAnimationBlendFrame(AnimationSlotInfo &animInfo, double tDelta, float &blendScale);

			// Animations
			void TransformBoneFrames(std::vector<math::Transform> &bonePoses, std::vector<Vector3> *boneScales, animation::Animation &anim, Frame *frameBlend, bool bAdd = true);
			void TransformBoneFrames(std::vector<math::Transform> &tgt, std::vector<Vector3> *boneScales, const std::shared_ptr<animation::Animation> &baseAnim, const std::shared_ptr<animation::Animation> &anim, std::vector<math::Transform> &add,
			  std::vector<Vector3> *addScales, bool bAdd = true);
			//

			std::unordered_map<uint32_t, AnimationSlotInfo> m_animSlots = {};
			AnimationSlotInfo m_baseAnim = {};

			Vector3 m_animDisplacement = {};
			std::vector<math::ScaledTransform> m_bones = {};
			std::vector<math::ScaledTransform> m_processedBones = {}; // Bone positions / rotations in entity space
		  protected:
			// We have to collect the animation events for the current frame and execute them after ALL animations have been completed (In case some events need to access animation data)
			std::queue<AnimationEventQueueItem> m_animEventQueue = std::queue<AnimationEventQueueItem> {};

			// Custom animation events
			void ApplyAnimationEventTemplate(const TemplateAnimationEvent &t);
			void ApplyAnimationEventTemplates();
			std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::vector<CustomAnimationEvent>>> &GetAnimationEvents();
			std::vector<CustomAnimationEvent> *GetAnimationEvents(uint32_t animId, uint32_t frameId);

			std::vector<TemplateAnimationEvent> m_animEventTemplates;
			std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::vector<CustomAnimationEvent>>> m_animEvents;

			struct ComponentEventQueueInfo {
				ComponentEventId id;
				std::unique_ptr<ComponentEvent> eventData;
			};
			std::queue<ComponentEventQueueInfo> m_queuedEvents;

			StateFlags m_stateFlags = StateFlags::AbsolutePosesDirty;
			std::shared_ptr<const Frame> m_bindPose = nullptr;
			std::unordered_map<unsigned int, float> m_blendControllers = {};
			util::PFloatProperty m_playbackRate = nullptr;

			std::unordered_map<AnimationEvent::Type, CallbackHandle> m_boundAnimEvents;
		};

		// Events

		struct DLLNETWORK CEOnBoneTransformChanged : public ComponentEvent {
			CEOnBoneTransformChanged(UInt32 boneId, const Vector3 *pos, const Quat *rot, const Vector3 *scale);
			virtual void PushArguments(lua::State *l) override;
			UInt32 boneId;
			const Vector3 *pos;
			const Quat *rot;
			const Vector3 *scale;
		};
		struct DLLNETWORK CEOnAnimationComplete : public ComponentEvent {
			CEOnAnimationComplete(int32_t animation, Activity activity);
			virtual void PushArguments(lua::State *l) override;
			int32_t animation;
			Activity activity;
		};
		struct DLLNETWORK CELayeredAnimationInfo : public ComponentEvent {
			CELayeredAnimationInfo(int32_t slot, int32_t animation, Activity activity);
			virtual void PushArguments(lua::State *l) override;
			int32_t slot;
			int32_t animation;
			Activity activity;
		};
		struct DLLNETWORK CEHandleAnimationEvent : public ComponentEvent {
			CEHandleAnimationEvent(const AnimationEvent &animationEvent);
			virtual void PushArguments(lua::State *l) override;
			void PushArgumentVariadic(lua::State *l);
			const AnimationEvent &animationEvent;
		};
		struct DLLNETWORK CEOnPlayAnimation : public ComponentEvent {
			CEOnPlayAnimation(int32_t previousAnimation, int32_t animation, FPlayAnim flags);
			virtual void PushArguments(lua::State *l) override;
			int32_t previousAnimation;
			int32_t animation;
			FPlayAnim flags;
		};
		struct DLLNETWORK CEOnPlayActivity : public ComponentEvent {
			CEOnPlayActivity(Activity activity, FPlayAnim flags);
			virtual void PushArguments(lua::State *l) override;
			Activity activity;
			FPlayAnim flags;
		};
		struct DLLNETWORK CEOnPlayLayeredActivity : public ComponentEvent {
			CEOnPlayLayeredActivity(int slot, Activity activity, FPlayAnim flags);
			virtual void PushArguments(lua::State *l) override;
			int slot;
			Activity activity;
			FPlayAnim flags;
		};
		struct DLLNETWORK CEOnPlayLayeredAnimation : public CEOnPlayAnimation {
			CEOnPlayLayeredAnimation(int32_t slot, int32_t previousAnimation, int32_t animation, FPlayAnim flags);
			virtual void PushArguments(lua::State *l) override;
			int32_t slot;
		};
		struct DLLNETWORK CEOnStopLayeredAnimation : public ComponentEvent {
			CEOnStopLayeredAnimation(int32_t slot, BaseAnimatedComponent::AnimationSlotInfo &slotInfo);
			virtual void PushArguments(lua::State *l) override;
			int32_t slot;
			BaseAnimatedComponent::AnimationSlotInfo &slotInfo;
		};
		struct DLLNETWORK CETranslateLayeredActivity : public ComponentEvent {
			CETranslateLayeredActivity(int32_t &slot, Activity &activity, FPlayAnim &flags);
			virtual void PushArguments(lua::State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua::State *l) override;
			int32_t &slot;
			Activity &activity;
			FPlayAnim &flags;
		};
		struct DLLNETWORK CEOnAnimationStart : public ComponentEvent {
			CEOnAnimationStart(int32_t animation, Activity activity, FPlayAnim flags);
			virtual void PushArguments(lua::State *l) override;
			int32_t animation;
			Activity activity;
			FPlayAnim flags;
		};
		struct DLLNETWORK CETranslateLayeredAnimation : public ComponentEvent {
			CETranslateLayeredAnimation(int32_t &slot, int32_t &animation, FPlayAnim &flags);
			virtual void PushArguments(lua::State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua::State *l) override;
			int32_t &slot;
			int32_t &animation;
			FPlayAnim &flags;
		};
		struct DLLNETWORK CETranslateAnimation : public ComponentEvent {
			CETranslateAnimation(int32_t &animation, FPlayAnim &flags);
			virtual void PushArguments(lua::State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua::State *l) override;
			int32_t &animation;
			FPlayAnim &flags;
		};
		struct DLLNETWORK CETranslateActivity : public ComponentEvent {
			CETranslateActivity(Activity &activity);
			virtual void PushArguments(lua::State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua::State *l) override;
			Activity &activity;
		};
		struct DLLNETWORK CEOnBlendAnimation : public ComponentEvent {
			CEOnBlendAnimation(BaseAnimatedComponent::AnimationSlotInfo &slotInfo, Activity activity, std::vector<math::Transform> &bonePoses, std::vector<Vector3> *boneScales);
			virtual void PushArguments(lua::State *l) override;
			BaseAnimatedComponent::AnimationSlotInfo &slotInfo;
			Activity activity;
			std::vector<math::Transform> &bonePoses;
			std::vector<Vector3> *boneScales;
		};
		struct DLLNETWORK CEMaintainAnimations : public ComponentEvent {
			CEMaintainAnimations(double deltaTime);
			virtual void PushArguments(lua::State *l) override;
			double deltaTime;
		};
		struct DLLNETWORK CEMaintainAnimation : public ComponentEvent {
			CEMaintainAnimation(BaseAnimatedComponent::AnimationSlotInfo &slotInfo, double deltaTime);
			virtual void PushArguments(lua::State *l) override;
			BaseAnimatedComponent::AnimationSlotInfo &slotInfo;
			double deltaTime;
		};
		struct DLLNETWORK CEMaintainAnimationMovement : public ComponentEvent {
			CEMaintainAnimationMovement(const Vector3 &displacement);
			virtual void PushArguments(lua::State *l) override;
			const Vector3 &displacement;
		};
		struct DLLNETWORK CEShouldUpdateBones : public ComponentEvent {
			CEShouldUpdateBones();
			virtual void PushArguments(lua::State *l) override;
			bool shouldUpdate = true;
		};
		struct DLLNETWORK CEOnUpdateSkeleton : public ComponentEvent {
			CEOnUpdateSkeleton();
			virtual void PushArguments(lua::State *l) override;
			virtual uint32_t GetReturnCount() override;
			virtual void HandleReturnValues(lua::State *l) override;
			bool bonePosesHaveChanged = false;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::BaseAnimatedComponent::StateFlags)
};
