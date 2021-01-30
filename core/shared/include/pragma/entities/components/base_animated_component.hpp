/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __BASE_ANIMATED_COMPONENT_HPP__
#define __BASE_ANIMATED_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/model/animation/play_animation_flags.hpp"
#include "pragma/model/animation/activities.h"
#include "pragma/model/animation/animation_event.h"
#include <sharedutils/property/util_property.hpp>
#include <pragma/math/orientation.h>
#include <mathutil/transform.hpp>
#include <mathutil/uvec.h>

class Animation;
class Frame;
class ModelSubMesh;
struct AnimationEvent;
enum class ALSoundType : int32_t;
namespace pragma
{
	class DLLNETWORK BaseAnimatedComponent
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_HANDLE_ANIMATION_EVENT;
		static ComponentEventId EVENT_ON_PLAY_ANIMATION;
		static ComponentEventId EVENT_ON_PLAY_LAYERED_ANIMATION;
		static ComponentEventId EVENT_ON_PLAY_LAYERED_ACTIVITY;
		static ComponentEventId EVENT_ON_ANIMATION_COMPLETE;
		static ComponentEventId EVENT_ON_LAYERED_ANIMATION_START;
		static ComponentEventId EVENT_ON_LAYERED_ANIMATION_COMPLETE;
		static ComponentEventId EVENT_ON_ANIMATION_START;
		static ComponentEventId EVENT_TRANSLATE_LAYERED_ANIMATION;
		static ComponentEventId EVENT_TRANSLATE_ANIMATION;
		static ComponentEventId EVENT_TRANSLATE_ACTIVITY;
		static ComponentEventId EVENT_MAINTAIN_ANIMATIONS;
		static ComponentEventId EVENT_MAINTAIN_ANIMATION;
		static ComponentEventId EVENT_MAINTAIN_ANIMATION_MOVEMENT;
		static ComponentEventId EVENT_SHOULD_UPDATE_BONES;

		static ComponentEventId EVENT_ON_PLAY_ACTIVITY;
		static ComponentEventId EVENT_ON_STOP_LAYERED_ANIMATION;
		static ComponentEventId EVENT_ON_BONE_TRANSFORM_CHANGED;
		static ComponentEventId EVENT_ON_ANIMATIONS_UPDATED;
		static ComponentEventId EVENT_ON_BLEND_ANIMATION;
		static ComponentEventId EVENT_PLAY_ANIMATION;
		static ComponentEventId EVENT_ON_ANIMATION_RESET;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);

		enum class StateFlags : uint8_t
		{
			None = 0u,
			AbsolutePosesDirty = 1u,
			BaseAnimationDirty = AbsolutePosesDirty<<1u
		};

		struct DLLNETWORK AnimationSlotInfo
		{
		public:
			AnimationSlotInfo()=default;
			AnimationSlotInfo(int32_t panimation,int32_t panimationLast=-1)
				: animation(panimation),cycle(0.f)
			{}
			Activity activity = Activity::Invalid;
			int32_t animation = -1;
			float cycle = 0.f;
			FPlayAnim flags = FPlayAnim::Default;
			std::vector<umath::Transform> bonePoses;
			std::vector<Vector3> boneScales;

			// These are only used if the animation has a blend-controller
			std::vector<umath::Transform> bonePosesBc;
			std::vector<Vector3> boneScalesBc;

			// Keep a reference to our last animation for blending
			struct
			{
				int32_t animation = -1;
				float cycle = 0.f;
				FPlayAnim flags = FPlayAnim::Default;
				std::pair<float,float> blendTimeScale = {0.f,0.f};
				float blendScale = 1.f;
			} lastAnim;
		};
		static bool GetBlendFramesFromCycle(Animation &anim,float cycle,Frame **outFrameSrc,Frame **outFrameDst,float &outInterpFactor,int32_t frameOffset=0);

		virtual void Initialize() override;

		virtual void MaintainAnimationMovement(const Vector3 &disp);

		void SetGlobalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale);
		void SetGlobalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot);
		void SetGlobalBonePosition(UInt32 boneId,const Vector3 &pos);
		void SetGlobalBoneRotation(UInt32 boneId,const Quat &rot);

		bool IsPlayingAnimation() const;
		bool CalcAnimationMovementSpeed(float *x,float *z,int32_t frameOffset=0) const;

		void SetBoneScale(uint32_t boneId,const Vector3 &scale);
		const Vector3 *GetBoneScale(uint32_t boneId) const;
		std::optional<Mat4> GetBoneMatrix(unsigned int boneID) const;

		// Returns the bone position / rotation in world space. Very expensive.
		Bool GetGlobalBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot,Vector3 *scale=nullptr) const;
		Bool GetGlobalBonePosition(UInt32 boneId,Vector3 &pos) const;
		Bool GetGlobalBoneRotation(UInt32 boneId,Quat &rot) const;
		// Returns the bone position / rotation in entity space. Very expensive.
		Bool GetLocalBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot,Vector3 *scale=nullptr) const;
		Bool GetLocalBonePosition(UInt32 boneId,Vector3 &pos) const;
		Bool GetLocalBoneRotation(UInt32 boneId,Quat &rot) const;

		// Returns the bone position / rotation in bone space.
		Bool GetBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot,Vector3 &scale) const;
		Bool GetBonePosition(UInt32 boneId,Vector3 &pos,Quat &rot) const;
		Bool GetBonePosition(UInt32 boneId,Vector3 &pos) const;
		Bool GetBoneRotation(UInt32 boneId,Quat &rot) const;
		// Returns the bone position / angles in bone space.
		Bool GetBonePosition(UInt32 boneId,Vector3 &pos,EulerAngles &ang) const;
		Bool GetBoneAngles(UInt32 boneId,EulerAngles &ang) const;
		// Returns the bone position in bone space, or null if the bone doesn't exist.
		const Vector3 *GetBonePosition(UInt32 boneId) const;
		// Returns the bone rotation in bone space, or null if the bone doesn't exist.
		const Quat *GetBoneRotation(UInt32 boneId) const;
		void SetBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale);
		void SetBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot);
		void SetBonePosition(UInt32 boneId,const Vector3 &pos,const EulerAngles &ang);
		void SetBonePosition(UInt32 boneId,const Vector3 &pos);
		void SetBoneRotation(UInt32 boneId,const Quat &rot);
		void SetLocalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot,const Vector3 &scale);
		void SetLocalBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot);
		void SetLocalBonePosition(UInt32 boneId,const Vector3 &pos);
		void SetLocalBoneRotation(UInt32 boneId,const Quat &rot);

		float GetCycle() const;
		void SetCycle(float cycle);
		Animation *GetAnimationObject() const;
		int32_t GetAnimation() const;

		Activity GetActivity() const;
		int32_t GetLayeredAnimation(uint32_t slot) const;
		Activity GetLayeredActivity(uint32_t slot) const;

		virtual void PlayAnimation(int animation,FPlayAnim flags=FPlayAnim::Default);
		virtual void PlayLayeredAnimation(int slot,int animation,FPlayAnim flags=FPlayAnim::Default);
		bool PlayActivity(Activity activity,FPlayAnim flags=FPlayAnim::Default);
		bool PlayLayeredActivity(int slot,Activity activity,FPlayAnim flags=FPlayAnim::Default);
		bool PlayLayeredAnimation(int slot,std::string animation,FPlayAnim flags=FPlayAnim::Default);
		virtual void StopLayeredAnimation(int slot);
		bool PlayAnimation(const std::string &animation,FPlayAnim flags=FPlayAnim::Default);
		void SetPlaybackRate(float rate);
		float GetPlaybackRate() const;
		const util::PFloatProperty &GetPlaybackRateProperty() const;
		int32_t SelectTranslatedAnimation(Activity &inOutActivity) const;
		int SelectWeightedAnimation(Activity activity,int animAvoid=-1) const;
		// Returns the time left until the current animation has finished playing
		float GetAnimationDuration() const;

		void SetBlendController(unsigned int controller,float val);
		void SetBlendController(const std::string &controller,float val);
		float GetBlendController(const std::string &controller) const;
		float GetBlendController(unsigned int controller) const;
		const std::unordered_map<unsigned int,float> &GetBlendControllers() const;

		void AddAnimationEvent(const std::string &name,uint32_t frameId,const AnimationEvent &ev);
		CallbackHandle AddAnimationEvent(const std::string &name,uint32_t frameId,const std::function<void(void)> &f);
		CallbackHandle AddAnimationEvent(const std::string &name,uint32_t frameId,const CallbackHandle &cb);

		void AddAnimationEvent(uint32_t animId,uint32_t frameId,const AnimationEvent &ev);
		CallbackHandle AddAnimationEvent(uint32_t animId,uint32_t frameId,const std::function<void(void)> &f);
		CallbackHandle AddAnimationEvent(uint32_t animId,uint32_t frameId,const CallbackHandle &cb);
		void ClearAnimationEvents();
		void ClearAnimationEvents(uint32_t animId);
		void ClearAnimationEvents(uint32_t animId,uint32_t frameId);
		void InjectAnimationEvent(const AnimationEvent &ev);

		void ClearAnimationEvents(const std::string &anim);
		void ClearAnimationEvents(const std::string &anim,uint32_t frameId);
		//void RemoveAnimationEvent(uint32_t animId,uint32_t frameId,uint32_t idx);

		void SetLastAnimationBlendScale(float scale);

		const std::vector<umath::ScaledTransform> &GetProcessedBones() const;
		std::vector<umath::ScaledTransform> &GetProcessedBones();

		void SetBonePosition(UInt32 boneId,const Vector3 &pos,const Quat &rot,const Vector3 *scale,Bool updatePhysics);

		// Transforms all bone positions / rotations to entity space
		void UpdateSkeleton();

		bool ShouldUpdateBones() const;
		UInt32 GetBoneCount() const;
		const std::vector<umath::ScaledTransform> &GetBoneTransforms() const;
		std::vector<umath::ScaledTransform> &GetBoneTransforms();
		const std::vector<umath::ScaledTransform> &GetProcessedBoneTransforms() const;
		std::vector<umath::ScaledTransform> &GetProcessedBoneTransforms();
		const AnimationSlotInfo &GetBaseAnimationInfo() const;
		AnimationSlotInfo &GetBaseAnimationInfo();
		const std::unordered_map<uint32_t,AnimationSlotInfo> &GetAnimationSlotInfos() const;
		std::unordered_map<uint32_t,AnimationSlotInfo> &GetAnimationSlotInfos();

		Activity TranslateActivity(Activity act);
		void SetBaseAnimationDirty();

		void BlendBonePoses(
			const std::vector<umath::Transform> &srcBonePoses,const std::vector<Vector3> *optSrcBoneScales,
			const std::vector<umath::Transform> &dstBonePoses,const std::vector<Vector3> *optDstBoneScales,
			std::vector<umath::Transform> &outBonePoses,std::vector<Vector3> *optOutBoneScales,
			Animation &anim,float interpFactor
		) const;
		void BlendBoneFrames(
			std::vector<umath::Transform> &tgt,std::vector<Vector3> *tgtScales,std::vector<umath::Transform> &add,std::vector<Vector3> *addScales,float blendScale
		) const;

		virtual bool MaintainAnimations(double dt);

		virtual std::optional<Mat4> GetVertexTransformMatrix(const ModelSubMesh &subMesh,uint32_t vertexId) const;
		virtual bool GetLocalVertexPosition(const ModelSubMesh &subMesh,uint32_t vertexId,Vector3 &pos,const std::optional<Vector3> &vertexOffset={}) const;
		bool GetVertexPosition(uint32_t meshGroupId,uint32_t meshId,uint32_t subMeshId,uint32_t vertexId,Vector3 &pos) const;
		bool GetVertexPosition(const ModelSubMesh &subMesh,uint32_t vertexId,Vector3 &pos) const;

		void SetBindPose(const Frame &frame);
		const Frame *GetBindPose() const;

		CallbackHandle BindAnimationEvent(AnimationEvent::Type eventId,const std::function<void(std::reference_wrapper<const AnimationEvent>)> &fCallback);

		virtual void Save(DataStream &ds) override;
		using BaseEntityComponent::Load;
	protected:
		BaseAnimatedComponent(BaseEntity &ent);
		virtual void OnModelChanged(const std::shared_ptr<Model> &mdl);
		virtual void Load(DataStream &ds,uint32_t version) override;
		virtual void ResetAnimation(const std::shared_ptr<Model> &mdl);
		
		struct DLLNETWORK AnimationBlendInfo
		{
			float scale = 0.f;
			Animation *animation = nullptr;
			Frame *frameSrc = nullptr; // Frame to blend from
			Frame *frameDst = nullptr; // Frame to blend to
		};

		struct AnimationEventQueueItem
		{
			int32_t animId = -1;
			std::shared_ptr<Animation> animation = nullptr;
			int32_t lastFrame = -1;
			uint32_t frameId = 0;
		};

		// Custom animation events
		struct CustomAnimationEvent
			: public AnimationEvent
		{
			std::pair<bool,CallbackHandle> callback = {false,{}};
			CustomAnimationEvent(const AnimationEvent &ev);
			CustomAnimationEvent(const std::function<void(void)> &f);
			CustomAnimationEvent(const CallbackHandle &cb);
			CustomAnimationEvent()=default;
		};
		struct TemplateAnimationEvent
		{
			TemplateAnimationEvent()=default;
			TemplateAnimationEvent(const TemplateAnimationEvent&)=default;
			int32_t animId = -1;
			uint32_t frameId = 0;
			std::string anim;
			CustomAnimationEvent ev = {};
		};
		//

		bool MaintainAnimation(AnimationSlotInfo &animInfo,double dt,int32_t layeredSlot=-1);
		virtual void ApplyAnimationBlending(AnimationSlotInfo &animInfo,double tDelta);
		void HandleAnimationEvent(const AnimationEvent &ev);
		void PlayLayeredAnimation(int slot,int animation,FPlayAnim flags,AnimationSlotInfo **animInfo);
		void GetAnimationBlendController(Animation *anim,float cycle,std::array<AnimationBlendInfo,2> &bcFrames,float *blendScale) const;
		Frame *GetPreviousAnimationBlendFrame(AnimationSlotInfo &animInfo,double tDelta,float &blendScale);

		// Animations
		void TransformBoneFrames(std::vector<umath::Transform> &bonePoses,std::vector<Vector3> *boneScales,Animation &anim,Frame *frameBlend,bool bAdd=true);
		void TransformBoneFrames(std::vector<umath::Transform> &tgt,std::vector<Vector3> *boneScales,const std::shared_ptr<Animation> &anim,std::vector<umath::Transform> &add,std::vector<Vector3> *addScales,bool bAdd=true);
		//

		std::unordered_map<uint32_t,AnimationSlotInfo> m_animSlots = {};
		AnimationSlotInfo m_baseAnim = {};

		Vector3 m_animDisplacement = {};
		std::vector<umath::ScaledTransform> m_bones = {};
		std::vector<umath::ScaledTransform> m_processedBones = {}; // Bone positions / rotations in entity space
	private:
		// We have to collect the animation events for the current frame and execute them after ALL animations have been completed (In case some events need to access animation data)
		std::queue<AnimationEventQueueItem> m_animEventQueue = std::queue<AnimationEventQueueItem>{};

		// Custom animation events
		void ApplyAnimationEventTemplate(const TemplateAnimationEvent &t);
		void ApplyAnimationEventTemplates();
		std::unordered_map<uint32_t,std::unordered_map<uint32_t,std::vector<CustomAnimationEvent>>> &GetAnimationEvents();
		std::vector<CustomAnimationEvent> *GetAnimationEvents(uint32_t animId,uint32_t frameId);

		std::vector<TemplateAnimationEvent> m_animEventTemplates;
		std::unordered_map<uint32_t,std::unordered_map<uint32_t,std::vector<CustomAnimationEvent>>> m_animEvents;
		
		StateFlags m_stateFlags = StateFlags::AbsolutePosesDirty;
		std::shared_ptr<const Frame> m_bindPose = nullptr;
		std::unordered_map<unsigned int,float> m_blendControllers = {};
		util::PFloatProperty m_playbackRate = nullptr;

		std::unordered_map<AnimationEvent::Type,CallbackHandle> m_boundAnimEvents;
	};

	// Events

	struct DLLNETWORK CEOnBoneTransformChanged
		: public ComponentEvent
	{
		CEOnBoneTransformChanged(UInt32 boneId,const Vector3 *pos,const Quat *rot,const Vector3 *scale);
		virtual void PushArguments(lua_State *l) override;
		UInt32 boneId;
		const Vector3 *pos;
		const Quat *rot;
		const Vector3 *scale;
	};
	struct DLLNETWORK CEOnAnimationComplete
		: public ComponentEvent
	{
		CEOnAnimationComplete(int32_t animation,Activity activity);
		virtual void PushArguments(lua_State *l) override;
		int32_t animation;
		Activity activity;
	};
	struct DLLNETWORK CELayeredAnimationInfo
		: public ComponentEvent
	{
		CELayeredAnimationInfo(int32_t slot,int32_t animation,Activity activity);
		virtual void PushArguments(lua_State *l) override;
		int32_t slot;
		int32_t animation;
		Activity activity;
	};
	struct DLLNETWORK CEHandleAnimationEvent
		: public ComponentEvent
	{
		CEHandleAnimationEvent(const AnimationEvent &animationEvent);
		virtual void PushArguments(lua_State *l) override;
		void PushArgumentVariadic(lua_State *l);
		const AnimationEvent &animationEvent;
	};
	struct DLLNETWORK CEOnPlayAnimation
		: public ComponentEvent
	{
		CEOnPlayAnimation(int32_t previousAnimation,int32_t animation,pragma::FPlayAnim flags);
		virtual void PushArguments(lua_State *l) override;
		int32_t previousAnimation;
		int32_t animation;
		pragma::FPlayAnim flags;
	};
	struct DLLNETWORK CEOnPlayActivity
		: public ComponentEvent
	{
		CEOnPlayActivity(Activity activity,FPlayAnim flags);
		virtual void PushArguments(lua_State *l) override;
		Activity activity;
		pragma::FPlayAnim flags;
	};
	struct DLLNETWORK CEOnPlayLayeredActivity
		: public ComponentEvent
	{
		CEOnPlayLayeredActivity(int slot,Activity activity,FPlayAnim flags);
		virtual void PushArguments(lua_State *l) override;
		int slot;
		Activity activity;
		FPlayAnim flags;
	};
	struct DLLNETWORK CEOnPlayLayeredAnimation
		: public CEOnPlayAnimation
	{
		CEOnPlayLayeredAnimation(int32_t slot,int32_t previousAnimation,int32_t animation,pragma::FPlayAnim flags);
		virtual void PushArguments(lua_State *l) override;
		int32_t slot;
	};
	struct DLLNETWORK CEOnStopLayeredAnimation
		: public ComponentEvent
	{
		CEOnStopLayeredAnimation(int32_t slot,BaseAnimatedComponent::AnimationSlotInfo &slotInfo);
		virtual void PushArguments(lua_State *l) override;
		int32_t slot;
		BaseAnimatedComponent::AnimationSlotInfo &slotInfo;
	};
	struct DLLNETWORK CETranslateLayeredActivity
		: public ComponentEvent
	{
		CETranslateLayeredActivity(int32_t &slot,Activity &activity,pragma::FPlayAnim &flags);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		int32_t &slot;
		Activity &activity;
		pragma::FPlayAnim &flags;
	};
	struct DLLNETWORK CEOnAnimationStart
		: public ComponentEvent
	{
		CEOnAnimationStart(int32_t animation,Activity activity,pragma::FPlayAnim flags);
		virtual void PushArguments(lua_State *l) override;
		int32_t animation;
		Activity activity;
		pragma::FPlayAnim flags;
	};
	struct DLLNETWORK CETranslateLayeredAnimation
		: public ComponentEvent
	{
		CETranslateLayeredAnimation(int32_t &slot,int32_t &animation,pragma::FPlayAnim &flags);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		int32_t &slot;
		int32_t &animation;
		pragma::FPlayAnim &flags;
	};
	struct DLLNETWORK CETranslateAnimation
		: public ComponentEvent
	{
		CETranslateAnimation(int32_t &animation,pragma::FPlayAnim &flags);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		int32_t &animation;
		pragma::FPlayAnim &flags;
	};
	struct DLLNETWORK CETranslateActivity
		: public ComponentEvent
	{
		CETranslateActivity(Activity &activity);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		Activity &activity;
	};
	struct DLLNETWORK CEOnBlendAnimation
		: public ComponentEvent
	{
		CEOnBlendAnimation(BaseAnimatedComponent::AnimationSlotInfo &slotInfo,Activity activity,std::vector<umath::Transform> &bonePoses,std::vector<Vector3> *boneScales);
		virtual void PushArguments(lua_State *l) override;
		BaseAnimatedComponent::AnimationSlotInfo &slotInfo;
		Activity activity;
		std::vector<umath::Transform> &bonePoses;
		std::vector<Vector3> *boneScales;
	};
	struct DLLNETWORK CEMaintainAnimations
		: public ComponentEvent
	{
		CEMaintainAnimations(double deltaTime);
		virtual void PushArguments(lua_State *l) override;
		double deltaTime;
	};
	struct DLLNETWORK CEMaintainAnimation
		: public ComponentEvent
	{
		CEMaintainAnimation(BaseAnimatedComponent::AnimationSlotInfo &slotInfo,double deltaTime);
		virtual void PushArguments(lua_State *l) override;
		BaseAnimatedComponent::AnimationSlotInfo &slotInfo;
		double deltaTime;
	};
	struct DLLNETWORK CEMaintainAnimationMovement
		: public ComponentEvent
	{
		CEMaintainAnimationMovement(const Vector3 &displacement);
		virtual void PushArguments(lua_State *l) override;
		const Vector3 &displacement;
	};
	struct DLLNETWORK CEShouldUpdateBones
		: public ComponentEvent
	{
		CEShouldUpdateBones();
		virtual void PushArguments(lua_State *l) override;
		bool shouldUpdate = true;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::BaseAnimatedComponent::StateFlags)

#endif
