#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "pragma/networkdefinitions.h"
#include "pragma/model/animation/frame.h"
#include "pragma/model/animation/fanim.h"
#include "pragma/model/animation/activities.h"
#include "pragma/model/animation/animation_event.h"
#include <sharedutils/util_enum_register.hpp>
#include <vector>

struct DLLNETWORK AnimationBlendControllerTransition
{
	unsigned int animation;
	int transition;
};

struct DLLNETWORK AnimationBlendController
{
	unsigned int controller;
	std::vector<AnimationBlendControllerTransition> transitions;
};

class DLLNETWORK Animation
{
private:
	static util::EnumRegister s_activityEnumRegister;
	static util::EnumRegister s_eventEnumRegister;

	std::vector<std::shared_ptr<Frame>> m_frames;
	// Contains a list of model bone Ids which are used by this animation
	std::vector<uint32_t> m_boneIds;
	std::vector<float> m_boneWeights;
	// Maps a model bone id to a local bone id (m_boneIds index)
	std::unordered_map<uint32_t,uint32_t> m_boneIdMap;
	std::unordered_map<unsigned int,std::vector<std::shared_ptr<AnimationEvent>>> m_events;
	FAnim m_flags;
	Activity m_activity;
	unsigned char m_activityWeight;
	unsigned char m_fps;
	std::pair<Vector3,Vector3> m_renderBounds;
	std::unique_ptr<AnimationBlendController> m_blendController;
	std::unique_ptr<float> m_fadeIn;
	std::unique_ptr<float> m_fadeOut;
public:
	static util::EnumRegister &GetActivityEnumRegister();
	static util::EnumRegister &GetEventEnumRegister();
	enum class DLLNETWORK ShareMode : uint32_t
	{
		None = 0,
		Frames = 1,
		Events = 2,
	};
	Animation();
	Animation(const Animation &other,ShareMode share=ShareMode::None);
	const std::pair<Vector3,Vector3> &GetRenderBounds() const;
	void SetRenderBounds(const Vector3 &min,const Vector3 &max);
	void CalcRenderBounds(Model &mdl);
	Activity GetActivity() const;
	void SetActivity(Activity activity);
	unsigned char GetActivityWeight() const;
	void SetActivityWeight(unsigned char weight);
	unsigned char GetFPS();
	void SetFPS(unsigned char fps);
	void SetFlags(FAnim flags);
	FAnim GetFlags() const;
	bool HasFlag(FAnim flag) const;
	void AddFlags(FAnim flags);
	void RemoveFlags(FAnim flags);
	void AddFrame(std::shared_ptr<Frame> frame);
	float GetDuration();
	std::shared_ptr<Frame> GetFrame(unsigned int ID);
	const std::vector<unsigned int> &GetBoneList() const;
	const std::unordered_map<uint32_t,uint32_t> &GetBoneMap() const;
	void AddBoneId(uint32_t id);
	void SetBoneId(uint32_t localIdx,uint32_t id);
	void SetBoneList(const std::vector<uint32_t> &list);
	void ReserveBoneIds(uint32_t count);
	unsigned int GetBoneCount();
	unsigned int GetFrameCount();
	std::vector<std::shared_ptr<Frame>> &GetFrames();
	void AddEvent(unsigned int frame,AnimationEvent *ev);
	std::vector<std::shared_ptr<AnimationEvent>> *GetEvents(unsigned int frame);
	float GetFadeInTime();
	float GetFadeOutTime();
	bool HasFadeInTime();
	bool HasFadeOutTime();
	void SetFadeInTime(float t);
	void SetFadeOutTime(float t);
	AnimationBlendController *SetBlendController(unsigned int controller);
	AnimationBlendController *GetBlendController();
	void Localize(const Skeleton &skeleton);
	void Rotate(const Skeleton &skeleton,const Quat &rot);
	void Translate(const Skeleton &skeleton,const Vector3 &t);
	// Reverses all frames in the animation
	void Reverse();

	int32_t LookupBone(uint32_t boneId) const;

	void SetBoneWeight(uint32_t boneId,float weight);
	float GetBoneWeight(uint32_t boneId) const;
	bool GetBoneWeight(uint32_t boneId,float &weight) const;
	const std::vector<float> &GetBoneWeights() const;
	std::vector<float> &GetBoneWeights();
};

REGISTER_BASIC_ARITHMETIC_OPERATORS(Animation::ShareMode)

#endif
