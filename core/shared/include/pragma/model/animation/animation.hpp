/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __PRAGMA_ANIMATION_HPP__
#define __PRAGMA_ANIMATION_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/model/animation/frame.h"
#include "pragma/model/animation/fanim.h"
#include "pragma/model/animation/activities.h"
#include "pragma/model/animation/animation_event.h"
#include "pragma/types.hpp"
#include <sharedutils/util_enum_register.hpp>
#include <optional>
#include <vector>

#define PRAGMA_ANIMATION_VERSION 2

namespace pragma {
	enum class Axis : uint8_t;
};

struct DLLNETWORK AnimationBlendControllerTransition {
	uint32_t animation = std::numeric_limits<uint32_t>::max();
	float transition = 0.f;

	bool operator==(const AnimationBlendControllerTransition &other) const { return animation == other.animation && umath::abs(transition - other.transition) < 0.001f; }
	bool operator!=(const AnimationBlendControllerTransition &other) const { return !operator==(other); }
};

struct DLLNETWORK AnimationBlendController {
	uint32_t controller;
	std::vector<AnimationBlendControllerTransition> transitions;

	// An optional post blend target, which will be blended towards depending on the specified controller.
	// Primary used for directional movement animations with several cardinal animations and one center animation.
	uint32_t animationPostBlendTarget = std::numeric_limits<uint32_t>::max();
	uint32_t animationPostBlendController = std::numeric_limits<uint32_t>::max();

	bool operator==(const AnimationBlendController &other) const { return controller == other.controller && transitions == other.transitions && animationPostBlendTarget == other.animationPostBlendTarget && animationPostBlendController == other.animationPostBlendController; }
	bool operator!=(const AnimationBlendController &other) const { return !operator==(other); }
};

class VFilePtrInternalReal;
namespace udm {
	struct AssetData;
};
namespace pragma::animation {
	class Skeleton;
	class Animation;
	class DLLNETWORK Animation : public std::enable_shared_from_this<Animation> {
	  public:
		static util::EnumRegister &GetActivityEnumRegister();
		static util::EnumRegister &GetEventEnumRegister();
		static constexpr uint32_t PANIM_VERSION = 1;
		static constexpr auto PANIM_IDENTIFIER = "PANI";
		enum class DLLNETWORK ShareMode : uint32_t {
			None = 0,
			Frames = 1,
			Events = 2,
		};
		static std::shared_ptr<Animation> Create();
		static std::shared_ptr<Animation> Create(const Animation &other, ShareMode share = ShareMode::None);
		static std::shared_ptr<Animation> Load(const udm::AssetData &data, std::string &outErr, const pragma::animation::Skeleton *optSkeleton = nullptr, const Frame *optReference = nullptr);
		const std::pair<Vector3, Vector3> &GetRenderBounds() const;
		void SetRenderBounds(const Vector3 &min, const Vector3 &max);
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
		float GetDuration() const;
		std::shared_ptr<Frame> GetFrame(unsigned int ID);
		const std::vector<uint16_t> &GetBoneList() const;
		const std::unordered_map<uint32_t, uint32_t> &GetBoneMap() const;
		uint32_t AddBoneId(uint32_t id);
		void SetBoneId(uint32_t localIdx, uint32_t id);
		void SetBoneList(const std::vector<uint16_t> &list);
		void ReserveBoneIds(uint32_t count);
		unsigned int GetBoneCount();
		unsigned int GetFrameCount();
		std::vector<std::shared_ptr<Frame>> &GetFrames();
		void AddEvent(unsigned int frame, AnimationEvent *ev);
		std::vector<std::shared_ptr<AnimationEvent>> *GetEvents(unsigned int frame);
		float GetFadeInTime();
		float GetFadeOutTime();
		bool HasFadeInTime();
		bool HasFadeOutTime();
		void SetFadeInTime(float t);
		void SetFadeOutTime(float t);
		AnimationBlendController &SetBlendController(uint32_t controller);
		AnimationBlendController *GetBlendController();
		const AnimationBlendController *GetBlendController() const;
		void ClearBlendController();
		void Validate();
		void Localize(const pragma::animation::Skeleton &skeleton);
		void Rotate(const pragma::animation::Skeleton &skeleton, const Quat &rot);
		void Translate(const pragma::animation::Skeleton &skeleton, const Vector3 &t);
		void Scale(const Vector3 &scale);
		void Mirror(pragma::Axis axis);
		// Reverses all frames in the animation
		void Reverse();

		int32_t LookupBone(uint32_t boneId) const;

		void SetBoneWeight(uint32_t boneId, float weight);
		float GetBoneWeight(uint32_t boneId) const;
		bool GetBoneWeight(uint32_t boneId, float &weight) const;
		const std::vector<float> &GetBoneWeights() const;
		std::vector<float> &GetBoneWeights();

		std::shared_ptr<panima::Animation> ToPanimaAnimation(const pragma::animation::Skeleton &skel, const Frame *optRefPose = nullptr) const;

		// If reference frame is specified, it will be used to optimize frame data and reduce the file size
		bool Save(udm::AssetDataArg outData, std::string &outErr, const Frame *optReference = nullptr, bool enableOptimizations = true);
		bool SaveLegacy(std::shared_ptr<VFilePtrInternalReal> &f);

		bool operator==(const Animation &other) const;
		bool operator!=(const Animation &other) const { return !operator==(other); }
	  private:
		static util::EnumRegister s_activityEnumRegister;
		static util::EnumRegister s_eventEnumRegister;
		bool LoadFromAssetData(const udm::AssetData &data, std::string &outErr, const pragma::animation::Skeleton *optSkeleton = nullptr, const Frame *optReference = nullptr);
		Animation();
		Animation(const Animation &other, ShareMode share = ShareMode::None);

		std::vector<std::shared_ptr<Frame>> m_frames;
		// Contains a list of model bone Ids which are used by this animation
		std::vector<pragma::animation::BoneId> m_boneIds;
		std::vector<float> m_boneWeights;
		// Maps a model bone id to a local bone id (m_boneIds index)
		std::unordered_map<uint32_t, uint32_t> m_boneIdMap;
		std::unordered_map<unsigned int, std::vector<std::shared_ptr<AnimationEvent>>> m_events;
		FAnim m_flags;
		Activity m_activity;
		unsigned char m_activityWeight;
		unsigned char m_fps;
		std::pair<Vector3, Vector3> m_renderBounds;
		std::optional<AnimationBlendController> m_blendController = {};
		std::unique_ptr<float> m_fadeIn;
		std::unique_ptr<float> m_fadeOut;
	};
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(pragma::animation::Animation::ShareMode)

#endif
