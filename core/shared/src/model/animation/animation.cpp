/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/animation.hpp"
#include "pragma/model/animation/activities.h"
#include "pragma/logging.hpp"
#include <udm.hpp>
#include <mathutil/umath.h>
#include "pragma/model/animation/skeleton.hpp"
#include "pragma/model/model.h"
#include <panima/animation.hpp>
#include <panima/channel.hpp>
#include "pragma/model/animation/bone.hpp"
//#include <utility>

import bezierfit;

decltype(pragma::animation::Animation::s_activityEnumRegister) pragma::animation::Animation::s_activityEnumRegister;
decltype(pragma::animation::Animation::s_eventEnumRegister) pragma::animation::Animation::s_eventEnumRegister;

util::EnumRegister &pragma::animation::Animation::GetActivityEnumRegister() { return s_activityEnumRegister; }
util::EnumRegister &pragma::animation::Animation::GetEventEnumRegister() { return s_eventEnumRegister; }

std::shared_ptr<pragma::animation::Animation> pragma::animation::Animation::Load(const udm::AssetData &data, std::string &outErr, const pragma::animation::Skeleton *optSkeleton, const Frame *optReference)
{
	auto anim = pragma::animation::Animation::Create();
	if(anim->LoadFromAssetData(data, outErr, optSkeleton, optReference) == false)
		return nullptr;
	return anim;
}

template<class T0, class T1>
static void copy_safe(const T0 &src, T1 &dst, uint32_t srcStartIndex, uint32_t dstStartIndex, uint32_t count)
{
	auto *memPtr0 = reinterpret_cast<const uint8_t *>(src.data());
	auto *memPtr1 = reinterpret_cast<uint8_t *>(dst.data());
	memPtr0 += srcStartIndex * sizeof(typename T0::value_type);
	memPtr1 += dstStartIndex * sizeof(typename T1::value_type);

	auto *memPtr0End = memPtr0 + sizeof(typename T0::value_type) * src.size();
	auto *memPtr1End = memPtr1 + sizeof(typename T1::value_type) * dst.size();

	if(memPtr0 > memPtr0End)
		throw std::runtime_error {"Memory out of bounds!"};

	if(memPtr1 > memPtr1End)
		throw std::runtime_error {"Memory out of bounds!"};

	auto *memPtr0Write = memPtr0 + count * sizeof(typename T0::value_type);
	auto *memPtr1WriteEnd = memPtr1 + count * sizeof(typename T0::value_type);

	if(memPtr0Write > memPtr0End)
		throw std::runtime_error {"Memory out of bounds!"};

	if(memPtr1WriteEnd > memPtr1End)
		throw std::runtime_error {"Memory out of bounds!"};

	memcpy(memPtr1, memPtr0, count * sizeof(typename T0::value_type));
}

template<typename T>
static void apply_channel_animation_values(udm::LinkedPropertyWrapper &udmProp, float fps, const std::vector<float> &times, const std::vector<std::shared_ptr<Frame>> &frames, const std::function<void(Frame &, const T &)> &applyValue)
{
	if(fps == 0.f)
		return;
	std::vector<T> values;
	udmProp(values);
	auto stepTime = 1.f / fps;
	for(auto i = decltype(times.size()) {0u}; i < times.size(); ++i) {
		auto t = times[i];
		auto frameEnd = umath::round(t * fps);
		auto frameStart = frameEnd;
		if(i > 0) {
			auto tPrev = times[i - 1];
			auto dt = t - tPrev;
			auto nFrames = umath::round(dt / stepTime);
			assert(nFrames > 0);
			if(nFrames > 0)
				frameStart = frameEnd - (nFrames - 1);
		}
		else if(i == 0 && times.size() == 1)
			frameEnd = frames.size() - 1;
		for(auto frameIdx = frameStart; frameIdx <= frameEnd; ++frameIdx) {
			auto &frame = frames[frameIdx];
			applyValue(*frame, values[i]);
		}
	}
}

bool pragma::animation::Animation::LoadFromAssetData(const udm::AssetData &data, std::string &outErr, const pragma::animation::Skeleton *optSkeleton, const Frame *optReference)
{
	if(data.GetAssetType() != PANIM_IDENTIFIER) {
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1) {
		outErr = "Invalid version!";
		return false;
	}
	// if(version > PANIM_VERSION)
	// 	return false;
	auto activity = udm["activity"];
	if(activity && activity->IsType(udm::Type::String)) {
		auto id = pragma::animation::Animation::GetActivityEnumRegister().RegisterEnum(activity->GetValue<udm::String>());
		m_activity = (id != util::EnumRegister::InvalidEnum) ? static_cast<Activity>(id) : Activity::Invalid;
	}

	udm["activityWeight"](m_activityWeight);
	udm["fps"](m_fps);

	float duration = 0.f;
	udm["duration"](duration);

	m_renderBounds.first = udm["renderBounds"]["min"](Vector3 {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
	m_renderBounds.second = udm["renderBounds"]["max"](Vector3 {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()});

	auto fadeInTime = udm["fadeInTime"];
	if(fadeInTime)
		m_fadeIn = std::make_unique<float>(fadeInTime(0.f));

	auto fadeOutTime = udm["fadeOutTime"];
	if(fadeOutTime)
		m_fadeOut = std::make_unique<float>(fadeOutTime(0.f));

	auto udmBlendController = udm["blendController"];
	if(udmBlendController) {
		m_blendController = AnimationBlendController {};
		udmBlendController["controller"](m_blendController->controller);

		auto udmTransitions = udmBlendController["transitions"];
		m_blendController->transitions.resize(udmTransitions.GetSize());
		uint32_t idxTransition = 0;
		for(auto &udmTransition : udmTransitions) {
			udmTransition["animation"](m_blendController->transitions[idxTransition].animation);
			udmTransition["transition"](m_blendController->transitions[idxTransition].transition);
			++idxTransition;
		}
		udmBlendController["animationPostBlendController"](m_blendController->animationPostBlendController);
		udmBlendController["animationPostBlendTarget"](m_blendController->animationPostBlendTarget);
	}

	m_flags = FAnim::None;
	auto udmFlags = udm["flags"];
	if(udmFlags) {
		if(udmFlags.IsType(udm::Type::String)) {
			auto flags = magic_enum::flags::enum_cast<FAnim>(udmFlags.GetValue<udm::String>());
			if(flags.has_value())
				m_flags = *flags;
		}
		else {
			auto readFlag = [this, &udmFlags](FAnim flag, const std::string &name) {
				auto udmFlag = udmFlags[name];
				if(udmFlag && udmFlag(false))
					m_flags |= flag;
			};
			readFlag(FAnim::Loop, "loop");
			readFlag(FAnim::MoveX, "moveX");
			readFlag(FAnim::MoveZ, "moveZ");
			readFlag(FAnim::NoRepeat, "noRepeat");
			readFlag(FAnim::Autoplay, "autoplay");
			readFlag(FAnim::Gesture, "gesture");
			readFlag(FAnim::NoMoveBlend, "noMoveBlend");
			static_assert(umath::to_integral(FAnim::Count) == 7, "Update this list when new flags have been added!");
		}
	}

	std::vector<pragma::animation::BoneId> nodeToLocalBoneId;
	if(udm["bones"]) {
		// Backwards compatibility
		udm["bones"](m_boneIds);
		auto numBones = m_boneIds.size();
		m_boneIdMap.reserve(numBones);
		for(auto i = decltype(numBones) {0u}; i < numBones; ++i)
			m_boneIdMap[m_boneIds[i]] = i;

		udm["boneWeights"](m_boneWeights);
	}
	else {
		auto udmNodes = udm["nodes"];
		m_boneIds.reserve(udmNodes.GetSize());
		uint32_t nodeIdx = 0;
		for(auto udmNode : udmNodes) {
			std::string type;
			udmNode["type"](type);
			if(type == "bone") {
				if(udmNode["bone"]) {
					uint32_t boneIdx = 0;
					udmNode["bone"](boneIdx);
					nodeToLocalBoneId[nodeIdx] = m_boneIds.size();
					m_boneIds.push_back(boneIdx);

					if(udmNode["weight"]) {
						auto weight = 0.f;
						udmNode["weight"](weight);
						m_boneWeights.push_back(weight);
					}
				}
				else {
					auto offset = m_boneIds.size();
					auto udmSet = udmNode["set"];
					if(m_boneIds.empty())
						udmSet(m_boneIds);
					else {
						auto n = udmSet.GetSize();
						m_boneIds.resize(offset + n);
						udmSet.GetBlobData(m_boneIds.data() + offset, n * sizeof(pragma::animation::BoneId), udm::Type::UInt16);
					}
					nodeToLocalBoneId.resize(m_boneIds.size(), std::numeric_limits<pragma::animation::BoneId>::max());
					for(auto i = offset; i < m_boneIds.size(); ++i)
						nodeToLocalBoneId[nodeIdx + (i - offset)] = i;

					auto udmWeights = udmNode["weights"];
					if(udmWeights) {
						if(m_boneWeights.empty())
							udmWeights(m_boneWeights);
						else {
							auto n = udmWeights.GetSize();
							auto offset = m_boneWeights.size();
							m_boneWeights.resize(offset + n);
							udmWeights.GetBlobData(m_boneWeights.data() + offset, n * sizeof(float), udm::Type::Float);
						}
					}
				}
			}
			auto udmSet = udmNode["set"];
			nodeIdx += udmSet ? udmSet.GetSize() : 1;
		}

		auto numBones = m_boneIds.size();
		m_boneIdMap.reserve(numBones);
		for(auto i = decltype(numBones) {0u}; i < numBones; ++i)
			m_boneIdMap[m_boneIds[i]] = i;
	}

	auto udmFrameData = udm["frameTransforms"];
	if(udmFrameData) {
		// Backwards compatibility
		auto numBones = m_boneIds.size();
		std::vector<umath::Transform> transforms;
		udmFrameData(transforms);
		if(!transforms.empty()) {
			auto numFrames = transforms.size() / m_boneIds.size();
			m_frames.resize(numFrames);
			uint32_t offset = 0;
			for(auto &frame : m_frames) {
				frame = Frame::Create(numBones);
				auto &frameTransforms = frame->GetBoneTransforms();
				copy_safe(transforms, frameTransforms, offset, 0, frameTransforms.size());
				offset += frameTransforms.size();
			}
		}

		auto udmFrameScales = udm["frameScales"];
		if(udmFrameScales) {
			std::vector<Vector3> scales;
			udmFrameScales(scales);
			if(!scales.empty()) {
				uint32_t offset = 0;
				for(auto &frame : m_frames) {
					auto &frameScales = frame->GetBoneScales();
					frameScales.resize(numBones);
					copy_safe(scales, frameScales, offset, 0, frameScales.size());
					offset += frameScales.size();
				}
			}
		}

		auto udmFrameMoveTranslations = udm["frameMoveTranslations"];
		if(udmFrameMoveTranslations) {
			std::vector<Vector2> moveTranslations;
			udmFrameMoveTranslations(moveTranslations);
			if(!moveTranslations.empty()) {
				for(auto i = decltype(m_frames.size()) {0u}; i < m_frames.size(); ++i) {
					auto &frame = m_frames[i];
					frame->SetMoveOffset(moveTranslations[i]);
				}
			}
		}
	}
	else {
		auto numFrames = umath::round(duration * m_fps);
		m_frames.resize(numFrames);
		auto isGesture = umath::is_flag_set(m_flags, FAnim::Gesture);
		for(auto &frame : m_frames) {
			frame = Frame::Create(m_boneIds.size());
			if(isGesture || !optReference || !optSkeleton)
				continue;
			auto &refBones = optSkeleton->GetBones();
			for(auto boneIdx = decltype(refBones.size()) {0u}; boneIdx < refBones.size(); ++boneIdx) {
				auto it = m_boneIdMap.find(boneIdx);
				if(it == m_boneIdMap.end())
					continue;
				auto localBoneId = it->second;
				auto *pos = optReference->GetBonePosition(boneIdx);
				if(pos)
					frame->SetBonePosition(localBoneId, *pos);

				auto *rot = optReference->GetBoneOrientation(boneIdx);
				if(rot)
					frame->SetBoneOrientation(localBoneId, *rot);

				auto *scale = optReference->GetBoneScale(boneIdx);
				if(scale)
					frame->SetBoneScale(localBoneId, *scale);
			}
		}
		auto udmChannels = udm["channels"];
		for(auto udmChannel : udmChannels) {
			uint16_t nodeId = 0;
			udmChannel["node"](nodeId);
			if(nodeId >= nodeToLocalBoneId.size()) {
				spdlog::debug("WARNING: Node {} of animation UDM channel exceeds number of bones ({})! Ignoring...", nodeId, nodeToLocalBoneId.size());
				continue;
			}
			auto localBoneId = nodeToLocalBoneId[nodeId];

			std::vector<float> times;
			udmChannel["times"](times);

			std::string property;
			udmChannel["property"](property);

			auto udmValues = udmChannel["values"];
			if(property == "position") {
				apply_channel_animation_values<Vector3>(udmValues, m_fps, times, m_frames, [localBoneId](Frame &frame, const Vector3 &val) { frame.SetBonePosition(localBoneId, val); });
			}
			else if(property == "rotation") {
				apply_channel_animation_values<Quat>(udmValues, m_fps, times, m_frames, [localBoneId](Frame &frame, const Quat &val) { frame.SetBoneOrientation(localBoneId, val); });
			}
			else if(property == "scale") {
				apply_channel_animation_values<Vector3>(udmValues, m_fps, times, m_frames, [localBoneId](Frame &frame, const Vector3 &val) { frame.SetBoneScale(localBoneId, val); });
			}
			else if(property == "offset") {
				// TODO
				std::vector<Vector2> moveOffsets;
				udmValues(moveOffsets);
				for(auto i = decltype(times.size()) {0u}; i < times.size(); ++i) {
					auto t = times[i];
					auto frameIdx = umath::round(t * m_fps);
					auto &frame = m_frames[frameIdx];
					frame->SetMoveOffset(moveOffsets[i]);
				}
			}
		}
	}

	auto udmEvents = udm["events"];
	if(udmEvents) {
		for(auto &udmEvent : udmEvents) {
			auto udmTime = udmEvent["time"];
			if(!udmTime)
				continue;
			auto frameIndex = umath::round(udmTime.ToValue<float>(0.f) * m_fps);
			auto it = m_events.find(frameIndex);
			if(it == m_events.end())
				it = m_events.insert(std::make_pair(frameIndex, std::vector<std::shared_ptr<AnimationEvent>> {})).first;

			auto &frameEvents = it->second;
			if(frameEvents.size() == frameEvents.capacity())
				frameEvents.reserve(frameEvents.size() * 1.1);

			auto name = udmEvent["name"](std::string {});
			if(name.empty())
				continue;
			auto id = pragma::animation::Animation::GetEventEnumRegister().RegisterEnum(name);
			if(id == util::EnumRegister::InvalidEnum)
				continue;
			auto ev = std::make_shared<AnimationEvent>();
			ev->eventID = static_cast<AnimationEvent::Type>(id);
			udmEvent["args"](ev->arguments);
			frameEvents.push_back(ev);
		}
	}
	return true;
}

struct Channel {
	std::vector<float> times;
	virtual bool CompareValues(const void *v0, const void *v1) const = 0;
	virtual bool CompareWithDefault(const void *v) const = 0;
	virtual const void *GetValue(size_t idx) const = 0;
	virtual size_t GetValueCount() const = 0;
	virtual void AddValue(const void *v) = 0;
	virtual void PopBack() { times.pop_back(); }
	virtual const void *GetReferenceValue(const Frame &reference, uint32_t boneId) const = 0;
};

struct PositionChannel : public Channel {
	std::vector<Vector3> values;
	static const Vector3 &Cast(const void *v) { return *static_cast<const Vector3 *>(v); }
	virtual bool CompareValues(const void *v0, const void *v1) const override { return uvec::distance_sqr(Cast(v0), Cast(v1)) < 0.001f; }
	virtual bool CompareWithDefault(const void *v) const override { return uvec::distance_sqr(Cast(v), uvec::ORIGIN) < 0.001f; }
	virtual const void *GetValue(size_t idx) const override { return &values[idx]; }
	virtual size_t GetValueCount() const override { return values.size(); }
	virtual void AddValue(const void *v) override { values.push_back(Cast(v)); }
	virtual void PopBack() override
	{
		values.pop_back();
		Channel::PopBack();
	}
	virtual const void *GetReferenceValue(const Frame &reference, uint32_t boneId) const override { return reference.GetBonePosition(boneId); }
};

struct RotationChannel : public Channel {
	std::vector<Quat> values;
	static const Quat &Cast(const void *v) { return *static_cast<const Quat *>(v); }
	virtual bool CompareValues(const void *v0, const void *v1) const override { return uquat::distance(Cast(v0), Cast(v1)) < 0.001f; }
	virtual bool CompareWithDefault(const void *v) const override
	{
		constexpr Quat identity {1.f, 0.f, 0.f, 0.f};
		return uquat::distance(Cast(v), identity) < 0.001f;
	}
	virtual const void *GetValue(size_t idx) const override { return &values[idx]; }
	virtual size_t GetValueCount() const override { return values.size(); }
	virtual void AddValue(const void *v) override { values.push_back(Cast(v)); }
	virtual void PopBack() override
	{
		values.pop_back();
		Channel::PopBack();
	}
	virtual const void *GetReferenceValue(const Frame &reference, uint32_t boneId) const override { return reference.GetBoneOrientation(boneId); }
};

struct ScaleChannel : public Channel {
	std::vector<Vector3> values;
	static const Vector3 &Cast(const void *v) { return *static_cast<const Vector3 *>(v); }
	virtual bool CompareValues(const void *v0, const void *v1) const override { return uvec::distance_sqr(Cast(v0), Cast(v1)) < 0.001f; }
	virtual bool CompareWithDefault(const void *v) const override
	{
		constexpr Vector3 identity {1.f, 1.f, 1.f};
		return uvec::distance_sqr(Cast(v), identity) < 0.001f;
	}
	virtual const void *GetValue(size_t idx) const override { return &values[idx]; }
	virtual size_t GetValueCount() const override { return values.size(); }
	virtual void AddValue(const void *v) override { values.push_back(Cast(v)); }
	virtual void PopBack() override
	{
		values.pop_back();
		Channel::PopBack();
	}
	virtual const void *GetReferenceValue(const Frame &reference, uint32_t boneId) const override { return reference.GetBoneScale(boneId); }
};

struct MoveChannel : public Channel {
	std::vector<Vector2> values;
	static const Vector2 &Cast(const void *v) { return *static_cast<const Vector2 *>(v); }
	virtual bool CompareValues(const void *v0, const void *v1) const override { return glm::length2(Cast(v0) - Cast(v1)) < 0.001f; }
	virtual bool CompareWithDefault(const void *v) const override { return glm::length2(Cast(v)) < 0.001f; }
	virtual const void *GetValue(size_t idx) const override { return &values[idx]; }
	virtual size_t GetValueCount() const override { return values.size(); }
	virtual void AddValue(const void *v) override { values.push_back(Cast(v)); }
	virtual void PopBack() override
	{
		values.pop_back();
		Channel::PopBack();
	}
	virtual const void *GetReferenceValue(const Frame &reference, uint32_t boneId) const override { return nullptr; }
};

static constexpr auto ENABLE_ANIMATION_SAVE_OPTIMIZATION = true;
template<class TChannel>
static void write_channel_value(std::shared_ptr<Channel> &channel, uint32_t numFrames, uint32_t frameIdx, float t, const std::function<const void *()> &fGetValue)
{
	if(!channel) {
		channel = std::make_shared<TChannel>();
		channel->times.reserve(numFrames);
		static_cast<TChannel *>(channel.get())->values.reserve(numFrames);
	}
	auto *curVal = fGetValue();
	if constexpr(ENABLE_ANIMATION_SAVE_OPTIMIZATION) {
		auto &values = static_cast<TChannel *>(channel.get())->values;
		if(values.size() > 1) {
			auto &prevPrevVal = values[values.size() - 2];
			auto &prevVal = values.back();
			if(channel->CompareValues(&prevVal, &prevPrevVal) && channel->CompareValues(curVal, &prevVal)) {
				// We got a pair of three matching values, so we
				// can just change the time-value for the last value to
				// our new time
				channel->times.back() = t;
				return;
			}
		}
	}
	channel->times.push_back(t);
	channel->AddValue(curVal);
};

bool pragma::animation::Animation::Save(udm::AssetDataArg outData, std::string &outErr, const Frame *optReference, bool enableOptimizations)
{
	outData.SetAssetType(PANIM_IDENTIFIER);
	outData.SetAssetVersion(PANIM_VERSION);
	auto udm = *outData;

	auto act = GetActivity();
	auto *activityName = pragma::animation::Animation::GetActivityEnumRegister().GetEnumName(umath::to_integral(act));
	if(activityName)
		udm["activity"] = *activityName;
	else
		udm.Add("activity", udm::Type::Nil);
	udm["activityWeight"] = static_cast<uint8_t>(GetActivityWeight());

	auto &renderBounds = GetRenderBounds();
	udm["renderBounds"]["min"] = renderBounds.first;
	udm["renderBounds"]["max"] = renderBounds.second;

	udm["fps"] = GetFPS();
	udm["duration"] = GetDuration();

	if(HasFadeInTime())
		udm["fadeInTime"] = GetFadeInTime();
	else
		udm.Add("fadeInTime", udm::Type::Nil);

	if(HasFadeOutTime())
		udm["fadeOutTime"] = GetFadeOutTime();
	else
		udm.Add("fadeOutTime", udm::Type::Nil);

	auto bones = GetBoneList();
	if(bones.empty() && !m_frames.empty()) {
		auto &frame = m_frames.front();
		auto numBones = frame->GetBoneCount();
		bones.resize(numBones);
		for(size_t i = 0; i < numBones; ++i)
			bones[i] = i;
	}
	auto numBones = bones.size();
	auto hasMove = false;
	for(auto &frame : m_frames) {
		if(frame->GetMoveOffset()) {
			hasMove = true;
			break;
		}
	}
	auto numNodes = numBones + (hasMove ? 1u : 0u);

	auto *blendController = GetBlendController();
	if(blendController) {
		udm["blendController"]["controller"] = blendController->controller;
		auto &transitions = blendController->transitions;
		auto udmTransitions = udm["blendController"].AddArray("transitions", transitions.size());
		for(auto i = decltype(transitions.size()) {0u}; i < transitions.size(); ++i) {
			auto &transition = transitions[i];
			auto udmTransition = udmTransitions[i];
			udmTransition["animation"] = transition.animation;
			udmTransition["transition"] = transition.transition;
		}
		udm["blendController"]["animationPostBlendController"] = blendController->animationPostBlendController;
		udm["blendController"]["animationPostBlendTarget"] = blendController->animationPostBlendTarget;
	}

	auto animFlags = GetFlags();
	/*auto writeFlag = [&udm,animFlags](FAnim flag,const std::string &name) {
		if(umath::is_flag_set(animFlags,flag) == false)
			return;
		udm["flags"][name] = true;
	};
	writeFlag(FAnim::Loop,"loop");
	writeFlag(FAnim::NoRepeat,"noRepeat");
	writeFlag(FAnim::Autoplay,"autoplay");
	writeFlag(FAnim::Gesture,"gesture");
	writeFlag(FAnim::NoMoveBlend,"noMoveBlend");
	static_assert(umath::to_integral(FAnim::Count) == 7,"Update this list when new flags have been added!");*/
	udm["flags"] = magic_enum::flags::enum_name(animFlags);

	std::vector<std::unordered_map<std::string, std::shared_ptr<Channel>>> nodeChannels {};
	nodeChannels.resize(numNodes);

	auto numFrames = m_frames.size();
	const auto defaultRotation = uquat::identity();
	const Vector3 defaultScale {1.f, 1.f, 1.f};
	// Note: Pragma currently uses a frame-based animation system, but the UDM format is
	// channel-based. Pragma will be transitioned to a channel-based system in the future, but
	// until then we'll have to convert it when saving or loading an animation.
	// The saving process also automatically does several optimizations, such as erasing redundant
	// positions/rotations/scales from the animation data.
	for(auto frameIdx = decltype(numFrames) {0u}; frameIdx < numFrames; ++frameIdx) {
		auto &frame = m_frames[frameIdx];
		auto &frameTransforms = frame->GetBoneTransforms();
		auto &scales = frame->GetBoneScales();
		if(frameTransforms.size() != numBones || (!scales.empty() && scales.size() != numBones)) {
			outErr = "Number of transforms (" + std::to_string(frameTransforms.size()) + ") in frame does not match number of bones (" + std::to_string(numBones) + ")!";
			return false;
		}
		auto t = frameIdx / static_cast<float>(m_fps);
		auto hasScales = frame->HasScaleTransforms();
		for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
			auto &channels = nodeChannels[i];

			write_channel_value<PositionChannel>(channels["position"], numFrames, frameIdx, t, [&frameTransforms, i]() -> const void * { return &frameTransforms[i].GetOrigin(); });
			write_channel_value<RotationChannel>(channels["rotation"], numFrames, frameIdx, t, [&frameTransforms, i]() -> const void * { return &frameTransforms[i].GetRotation(); });
			if(hasScales) {
				write_channel_value<ScaleChannel>(channels["scale"], numFrames, frameIdx, t, [&scales, i]() -> const void * { return &scales[i]; });
			}
		}
	}

	auto weights = GetBoneWeights();
	auto isGesture = HasFlag(FAnim::Gesture);
	if(!isGesture) {
		if constexpr(ENABLE_ANIMATION_SAVE_OPTIMIZATION) {
			if(enableOptimizations) {
				// We may be able to remove some channels altogether if they're empty,
				// or are equivalent to the reference pose (or identity value if the animation
				// is a gesture)
				for(auto it = nodeChannels.begin(); it != nodeChannels.begin() + numBones;) {
					auto boneId = bones[it - nodeChannels.begin()];
					auto &channels = *it;
					for(auto &pair : channels) {
						// If channel only has two values and they're both the same, we can get rid of the second one
						auto &channel = pair.second;
						auto n = channel->GetValueCount();
						if(n == 2) {
							auto *v0 = channel->GetValue(0);
							auto *v1 = channel->GetValue(1);
							if(channel->CompareValues(v0, v1) == false)
								continue;
							channel->PopBack();
						}

						n = channel->GetValueCount();
						if(n != 1)
							continue;
						if(isGesture) {
							if(channel->CompareWithDefault(channel->GetValue(0)))
								channel->PopBack();
							continue;
						}
						if(!optReference)
							continue;
						auto *ref = channel->GetReferenceValue(*optReference, boneId);
						if(!ref || !channel->CompareValues(channel->GetValue(0), ref))
							continue;
						channel->PopBack();
					}
					for(auto itChannel = channels.begin(); itChannel != channels.end();) {
						auto &channel = itChannel->second;
						if(channel->times.empty())
							itChannel = channels.erase(itChannel);
						else
							++itChannel;
					}
					if(channels.empty()) {
						auto i = (it - nodeChannels.begin());
						bones.erase(bones.begin() + i);
						if(!weights.empty())
							weights.erase(weights.begin() + i);
						--numBones;
						it = nodeChannels.erase(it);
					}
					else
						++it;
				}
			}
		}
	}

	auto it = std::find_if(weights.begin(), weights.end(), [](const float weight) { return (weight != 1.f) ? true : false; });
	auto hasWeights = (it != weights.end());

	auto udmNodes = udm.AddArray("nodes", 1u + (hasMove ? 1u : 0u));
	auto udmNodeBones = udmNodes[0];
	udmNodeBones["type"] = "bone";
	udmNodeBones["set"] = bones;
	if(hasWeights)
		udmNodeBones["weights"] = weights;
	std::optional<uint32_t> nodeMove {};
	if(hasMove) {
		auto udmNodeMove = udmNodes[1];
		udmNodeMove["type"] = "move";
		nodeMove = numBones;
	}

	if(nodeMove.has_value()) {
		for(auto frameIdx = decltype(numFrames) {0u}; frameIdx < numFrames; ++frameIdx) {
			auto &frame = m_frames[frameIdx];
			if(!frame->GetMoveOffset())
				continue;
			auto &moveOffset = *frame->GetMoveOffset();
			auto &channels = nodeChannels[*nodeMove];
			auto &moveChannel = channels["offset"];
			if(!moveChannel) {
				moveChannel = std::make_shared<MoveChannel>();
				moveChannel->times.reserve(numFrames);
				static_cast<MoveChannel *>(moveChannel.get())->values.reserve(numFrames);
			}
			auto t = frameIdx / static_cast<float>(m_fps);
			moveChannel->times.push_back(t);
			static_cast<MoveChannel *>(moveChannel.get())->values.push_back(*frame->GetMoveOffset());
		}
	}

	uint32_t numChannels = 0;
	for(auto &nodeChannel : nodeChannels)
		numChannels += nodeChannel.size();
	auto udmChannels = udm.AddArray("channels", numChannels, udm::Type::Element, udm::ArrayType::Compressed);
	uint32_t channelIdx = 0;
	for(auto nodeIdx = decltype(nodeChannels.size()) {0u}; nodeIdx < nodeChannels.size(); ++nodeIdx) {
		for(auto &pair : nodeChannels[nodeIdx]) {
			auto &channel = pair.second;
			auto udmChannel = udmChannels[channelIdx++];
			udmChannel["node"] = static_cast<uint16_t>(nodeIdx);
			udmChannel["property"] = pair.first;
			udmChannel.AddArray("times", channel->times);
			if(pair.first == "offset")
				udmChannel.AddArray("values", static_cast<MoveChannel &>(*channel).values);
			else if(pair.first == "position")
				udmChannel.AddArray("values", static_cast<PositionChannel &>(*channel).values);
			else if(pair.first == "rotation")
				udmChannel.AddArray("values", static_cast<RotationChannel &>(*channel).values);
			else if(pair.first == "scale")
				udmChannel.AddArray("values", static_cast<ScaleChannel &>(*channel).values);
		}
	}

	uint32_t numEvents = 0;
	for(auto &pair : m_events)
		numEvents += pair.second.size();
	auto udmEvents = udm.AddArray("events", numEvents);
	uint32_t evIdx = 0;
	for(auto &pair : m_events) {
		for(auto &ev : pair.second) {
			auto udmEvent = udmEvents[evIdx++];
			udmEvent["time"] = pair.first / static_cast<float>(m_fps);

			auto *eventName = pragma::animation::Animation::GetEventEnumRegister().GetEnumName(umath::to_integral(ev->eventID));
			udmEvent["name"] = (eventName != nullptr) ? *eventName : "";
			udmEvent["args"] = ev->arguments;
		}
	}
	return true;
}

bool pragma::animation::Animation::SaveLegacy(VFilePtrReal &f)
{
	f->Write<uint32_t>(PRAGMA_ANIMATION_VERSION);
	auto offsetToLen = f->Tell();
	f->Write<uint64_t>(0);
	auto animFlags = GetFlags();
	auto bMoveX = ((animFlags & FAnim::MoveX) == FAnim::MoveX) ? true : false;
	auto bMoveZ = ((animFlags & FAnim::MoveZ) == FAnim::MoveZ) ? true : false;
	auto bHasMovement = (bMoveX || bMoveZ) ? true : false;

	auto act = GetActivity();
	auto *activityName = pragma::animation::Animation::GetActivityEnumRegister().GetEnumName(umath::to_integral(act));
	f->WriteString((activityName != nullptr) ? *activityName : "");

	f->Write<uint8_t>(GetActivityWeight());
	f->Write<uint32_t>(umath::to_integral(animFlags));
	f->Write<uint32_t>(GetFPS());

	// Version 0x0007
	auto &renderBounds = GetRenderBounds();
	f->Write<Vector3>(renderBounds.first);
	f->Write<Vector3>(renderBounds.second);

	auto bFadeIn = HasFadeInTime();
	f->Write<bool>(bFadeIn);
	if(bFadeIn == true)
		f->Write<float>(GetFadeInTime());

	auto bFadeOut = HasFadeOutTime();
	f->Write<bool>(bFadeOut);
	if(bFadeOut == true)
		f->Write<float>(GetFadeOutTime());

	auto &bones = GetBoneList();
	auto numBones = bones.size();
	f->Write<uint32_t>(static_cast<uint32_t>(numBones));
	for(auto &boneId : bones)
		f->Write<uint32_t>(boneId);

	// Version 0x0012
	auto &weights = GetBoneWeights();
	auto it = std::find_if(weights.begin(), weights.end(), [](const float weight) { return (weight != 1.f) ? true : false; });
	if(it == weights.end())
		f->Write<bool>(false);
	else {
		f->Write<bool>(true);
		for(auto i = decltype(numBones) {0}; i < numBones; ++i)
			f->Write<float>((i < weights.size()) ? weights.at(i) : 1.f);
	}

	auto *blendController = GetBlendController();
	f->Write<bool>(blendController);
	if(blendController) {
		f->Write<int32_t>(blendController->controller);
		auto &transitions = blendController->transitions;
		f->Write<int8_t>(static_cast<int8_t>(transitions.size()));
		for(auto &t : transitions) {
			f->Write<uint32_t>(t.animation - 1); // Account for reference pose
			f->Write<float>(t.transition);
		}

		f->Write<int32_t>(blendController->animationPostBlendController);
		f->Write<int32_t>(blendController->animationPostBlendTarget);
	}

	auto numFrames = GetFrameCount();
	f->Write<uint32_t>(numFrames);
	for(auto i = decltype(numFrames) {0}; i < numFrames; ++i) {
		auto &frame = *GetFrame(i);
		for(auto j = decltype(numBones) {0}; j < numBones; ++j) {
			auto &pos = *frame.GetBonePosition(static_cast<uint32_t>(j));
			auto &rot = *frame.GetBoneOrientation(static_cast<uint32_t>(j));
			f->Write<Quat>(rot);
			f->Write<Vector3>(pos);
		}

		if(frame.HasScaleTransforms()) {
			auto &scales = frame.GetBoneScales();
			f->Write<uint32_t>(scales.size());
			f->Write(scales.data(), scales.size() * sizeof(scales.front()));
		}
		else
			f->Write<uint32_t>(static_cast<uint32_t>(0u));

		auto *animEvents = GetEvents(i);
		auto numEvents = (animEvents != nullptr) ? animEvents->size() : 0;
		f->Write<uint16_t>(static_cast<uint16_t>(numEvents));
		if(animEvents != nullptr) {
			for(auto &ev : *animEvents) {
				auto *eventName = pragma::animation::Animation::GetEventEnumRegister().GetEnumName(umath::to_integral(ev->eventID));
				f->WriteString((eventName != nullptr) ? *eventName : "");
				f->Write<uint8_t>(static_cast<uint8_t>(ev->arguments.size()));
				for(auto &arg : ev->arguments)
					f->WriteString(arg);
			}
		}

		if(bHasMovement == true) {
			auto &moveOffset = *frame.GetMoveOffset();
			if(bMoveX == true)
				f->Write<float>(moveOffset.x);
			if(bMoveZ == true)
				f->Write<float>(moveOffset.y);
		}
	}

	auto curOffset = f->Tell();
	auto len = curOffset - offsetToLen;
	f->Seek(offsetToLen);
	f->Write<uint64_t>(len);
	f->Seek(curOffset);
	return true;
}

std::shared_ptr<pragma::animation::Animation> pragma::animation::Animation::Create() { return std::shared_ptr<Animation>(new Animation {}); }
std::shared_ptr<pragma::animation::Animation> pragma::animation::Animation::Create(const Animation &other, ShareMode share) { return std::shared_ptr<Animation>(new Animation {other, share}); }

pragma::animation::Animation::Animation() : m_flags(FAnim::None), m_activity(Activity::Invalid), m_activityWeight(1), m_fps(24), m_fadeIn(nullptr), m_fadeOut(nullptr) {}

pragma::animation::Animation::Animation(const Animation &other, ShareMode share)
    : m_boneIds(other.m_boneIds), m_boneIdMap(other.m_boneIdMap), m_flags(other.m_flags), m_activity(other.m_activity), m_activityWeight(other.m_activityWeight), m_fps(other.m_fps), m_boneWeights(other.m_boneWeights), m_renderBounds(other.m_renderBounds),
      m_blendController {other.m_blendController}
{
	m_fadeIn = (other.m_fadeIn != nullptr) ? std::make_unique<float>(*other.m_fadeIn) : nullptr;
	m_fadeOut = (other.m_fadeOut != nullptr) ? std::make_unique<float>(*other.m_fadeOut) : nullptr;

	if((share & ShareMode::Frames) != ShareMode::None)
		m_frames = other.m_frames;
	else {
		m_frames.reserve(other.m_frames.size());
		for(auto &frame : other.m_frames)
			m_frames.push_back(Frame::Create(*frame));
	}

	if((share & ShareMode::Events) != ShareMode::None)
		m_events = other.m_events;
	else {
		for(auto &pair : other.m_events) {
			m_events[pair.first] = std::vector<std::shared_ptr<AnimationEvent>> {};
			auto &events = m_events[pair.first];
			events.reserve(pair.second.size());
			for(auto &ev : pair.second)
				events.push_back(std::make_unique<AnimationEvent>(*ev));
		}
	}
#ifdef _MSC_VER
	static_assert(sizeof(Animation) == 312, "Update this function when making changes to this class!");
#endif
}

void pragma::animation::Animation::Reverse() { std::reverse(m_frames.begin(), m_frames.end()); }

void pragma::animation::Animation::Rotate(const pragma::animation::Skeleton &skeleton, const Quat &rot)
{
	uvec::rotate(&m_renderBounds.first, rot);
	uvec::rotate(&m_renderBounds.second, rot);
	for(auto &frame : m_frames)
		frame->Rotate(*this, skeleton, rot);
}
void pragma::animation::Animation::Translate(const pragma::animation::Skeleton &skeleton, const Vector3 &t)
{
	m_renderBounds.first += t;
	m_renderBounds.second += t;
	for(auto &frame : m_frames)
		frame->Translate(*this, skeleton, t);
}

void pragma::animation::Animation::Scale(const Vector3 &scale)
{
	m_renderBounds.first *= scale;
	m_renderBounds.second *= scale;
	for(auto &frame : m_frames)
		frame->Scale(scale);
}

void pragma::animation::Animation::Mirror(pragma::Axis axis)
{
	auto transform = pragma::model::get_mirror_transform_vector(axis);
	for(auto &frame : m_frames)
		frame->Mirror(axis);

	m_renderBounds.first *= transform;
	m_renderBounds.second *= transform;
	uvec::to_min_max(m_renderBounds.first, m_renderBounds.second);
}

int32_t pragma::animation::Animation::LookupBone(uint32_t boneId) const
{
	if(boneId < m_boneIds.size() && m_boneIds.at(boneId) == boneId) // Faster than map lookup and this statement is true for most cases
		return boneId;
	auto it = m_boneIdMap.find(boneId);
	if(it == m_boneIdMap.end())
		return -1;
	return it->second;
}

void pragma::animation::Animation::CalcRenderBounds(Model &mdl)
{
	m_renderBounds = {{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()}, {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()}};
	for(auto &frame : m_frames) {
		auto frameBounds = frame->CalcRenderBounds(*this, mdl);
		for(uint8_t j = 0; j < 3; ++j) {
			if(frameBounds.first[j] < m_renderBounds.first[j])
				m_renderBounds.first[j] = frameBounds.first[j];
			if(frameBounds.first[j] < m_renderBounds.second[j])
				m_renderBounds.second[j] = frameBounds.first[j];

			if(frameBounds.second[j] < m_renderBounds.first[j])
				m_renderBounds.first[j] = frameBounds.second[j];
			if(frameBounds.second[j] < m_renderBounds.second[j])
				m_renderBounds.second[j] = frameBounds.second[j];
		}
	}
}

const std::pair<Vector3, Vector3> &pragma::animation::Animation::GetRenderBounds() const { return m_renderBounds; }
void pragma::animation::Animation::SetRenderBounds(const Vector3 &min, const Vector3 &max) { m_renderBounds = {min, max}; }

std::vector<std::shared_ptr<Frame>> &pragma::animation::Animation::GetFrames() { return m_frames; }

void pragma::animation::Animation::Localize(const pragma::animation::Skeleton &skeleton)
{
	for(auto it = m_frames.begin(); it != m_frames.end(); ++it)
		(*it)->Localize(*this, skeleton);
}
AnimationBlendController &pragma::animation::Animation::SetBlendController(uint32_t controller)
{
	m_blendController = AnimationBlendController {};
	m_blendController->controller = controller;
	return *m_blendController;
}
AnimationBlendController *pragma::animation::Animation::GetBlendController() { return m_blendController.has_value() ? &*m_blendController : nullptr; }
const AnimationBlendController *pragma::animation::Animation::GetBlendController() const { return const_cast<Animation *>(this)->GetBlendController(); }
void pragma::animation::Animation::ClearBlendController() { m_blendController = {}; }
void pragma::animation::Animation::Validate()
{
	for(auto &frame : GetFrames())
		frame->Validate();
	for(auto &w : GetBoneWeights())
		pragma::model::validate_value(w);
	pragma::model::validate_value(GetDuration());
	pragma::model::validate_value(GetFadeInTime());
	pragma::model::validate_value(GetFadeOutTime());
	auto &[min, max] = GetRenderBounds();
	pragma::model::validate_value(min);
	pragma::model::validate_value(max);
}
float pragma::animation::Animation::GetFadeInTime()
{
	if(m_fadeIn == nullptr)
		return 0.f;
	return *m_fadeIn;
}
float pragma::animation::Animation::GetFadeOutTime()
{
	if(m_fadeOut == nullptr)
		return 0.f;
	return *m_fadeOut;
}
void pragma::animation::Animation::SetFadeInTime(float t)
{
	if(m_fadeIn == nullptr)
		m_fadeIn = std::make_unique<float>();
	*m_fadeIn = t;
}
void pragma::animation::Animation::SetFadeOutTime(float t)
{
	if(m_fadeOut == nullptr)
		m_fadeOut = std::make_unique<float>();
	*m_fadeOut = t;
}
bool pragma::animation::Animation::HasFadeInTime() { return (m_fadeIn != nullptr) ? true : false; }
bool pragma::animation::Animation::HasFadeOutTime() { return (m_fadeOut != nullptr) ? true : false; }
Activity pragma::animation::Animation::GetActivity() const { return m_activity; }
void pragma::animation::Animation::SetActivity(Activity activity) { m_activity = activity; }
unsigned char pragma::animation::Animation::GetActivityWeight() const { return m_activityWeight; }
void pragma::animation::Animation::SetActivityWeight(unsigned char weight) { m_activityWeight = weight; }
unsigned char pragma::animation::Animation::GetFPS() { return m_fps; }
void pragma::animation::Animation::SetFPS(unsigned char fps) { m_fps = fps; }
float pragma::animation::Animation::GetDuration() const
{
	if(m_fps == 0)
		return 0.f;
	return float(m_frames.size()) / float(m_fps);
}

FAnim pragma::animation::Animation::GetFlags() const { return m_flags; }
void pragma::animation::Animation::SetFlags(FAnim flags) { m_flags = flags; }
bool pragma::animation::Animation::HasFlag(FAnim flag) const { return ((m_flags & flag) == flag) ? true : false; }
void pragma::animation::Animation::AddFlags(FAnim flags) { m_flags |= flags; }
void pragma::animation::Animation::RemoveFlags(FAnim flags) { m_flags &= ~flags; }

const std::vector<uint16_t> &pragma::animation::Animation::GetBoneList() const { return m_boneIds; }
const std::unordered_map<uint32_t, uint32_t> &pragma::animation::Animation::GetBoneMap() const { return m_boneIdMap; }
uint32_t pragma::animation::Animation::AddBoneId(uint32_t id)
{
	auto it = m_boneIdMap.find(id);
	if(it != m_boneIdMap.end())
		return it->second;
	m_boneIds.push_back(id);
	m_boneIdMap.insert(std::make_pair(id, m_boneIds.size() - 1));
	return m_boneIds.size() - 1;
}
void pragma::animation::Animation::SetBoneId(uint32_t localIdx, uint32_t id)
{
	if(localIdx >= m_boneIds.size())
		return;
	auto &oldId = m_boneIds.at(localIdx);
	auto it = m_boneIdMap.find(oldId);
	if(it != m_boneIdMap.end())
		m_boneIdMap.erase(it);

	oldId = id;
	m_boneIdMap.insert(std::make_pair(id, localIdx));
}
void pragma::animation::Animation::SetBoneList(const std::vector<uint16_t> &list)
{
	m_boneIds = list;
	m_boneIdMap.clear();
	m_boneIdMap.reserve(list.size());
	for(auto i = decltype(list.size()) {0}; i < list.size(); ++i)
		m_boneIdMap.insert(std::make_pair(list.at(i), i));
}
void pragma::animation::Animation::ReserveBoneIds(uint32_t count)
{
	m_boneIds.reserve(count);
	m_boneIdMap.reserve(count);
}

void pragma::animation::Animation::AddFrame(std::shared_ptr<Frame> frame) { m_frames.push_back(frame); }

std::shared_ptr<Frame> pragma::animation::Animation::GetFrame(unsigned int ID)
{
	if(ID >= m_frames.size())
		return nullptr;
	return m_frames[ID];
}

unsigned int pragma::animation::Animation::GetFrameCount() { return CUInt32(m_frames.size()); }

unsigned int pragma::animation::Animation::GetBoneCount() { return CUInt32(m_boneIds.size()); }

void pragma::animation::Animation::AddEvent(unsigned int frame, AnimationEvent *ev)
{
	auto it = m_events.find(frame);
	if(it == m_events.end())
		m_events[frame] = std::vector<std::shared_ptr<AnimationEvent>> {};
	m_events[frame].push_back(std::shared_ptr<AnimationEvent>(ev));
}

std::vector<std::shared_ptr<AnimationEvent>> *pragma::animation::Animation::GetEvents(unsigned int frame)
{
	auto it = m_events.find(frame);
	if(it == m_events.end())
		return nullptr;
	return &it->second;
}

float pragma::animation::Animation::GetBoneWeight(uint32_t boneId) const
{
	auto weight = 1.f;
	GetBoneWeight(boneId, weight);
	return weight;
}
bool pragma::animation::Animation::GetBoneWeight(uint32_t boneId, float &weight) const
{
	if(boneId >= m_boneWeights.size())
		return false;
	weight = m_boneWeights.at(boneId);
	return true;
}
const std::vector<float> &pragma::animation::Animation::GetBoneWeights() const { return const_cast<Animation *>(this)->GetBoneWeights(); }
std::vector<float> &pragma::animation::Animation::GetBoneWeights() { return m_boneWeights; }
void pragma::animation::Animation::SetBoneWeight(uint32_t boneId, float weight)
{
	if(boneId >= m_boneIds.size())
		return;
	if(m_boneIds.size() > m_boneWeights.size())
		m_boneWeights.resize(m_boneIds.size(), 1.f);
	m_boneWeights.at(boneId) = weight;
}

std::shared_ptr<panima::Animation> pragma::animation::Animation::ToPanimaAnimation(const pragma::animation::Skeleton &skel, const Frame *optRefPose) const
{
	auto &anim = const_cast<pragma::animation::Animation &>(*this);
	auto &boneList = anim.GetBoneList();
	auto &frames = anim.GetFrames();

	std::shared_ptr<Frame> refPoseRel = nullptr;
	if(optRefPose) {
		refPoseRel = Frame::Create(*optRefPose);
		refPoseRel->Localize(skel);
	}
	auto fps = anim.GetFPS();
	uint32_t frameIdx = 0;
	struct ValueData {
		float time;
		umath::ScaledTransform pose;
	};
	std::unordered_map<std::string, std::vector<ValueData>> boneValues;
	for(auto &frame : frames) {
		auto t = frameIdx / static_cast<float>(fps - 1);
		uint32_t i = 0;
		for(auto boneIdx : boneList) {
			auto bone = skel.GetBone(boneIdx).lock();
			std::string name = bone->name;
			auto it = boneValues.find(name);
			if(it == boneValues.end()) {
				it = boneValues.insert(std::make_pair(name, std::vector<ValueData> {})).first;
				it->second.reserve(frames.size());
			}
			auto &valueData = it->second;
			valueData.push_back({t, *frame->GetBoneTransform(i)});

			++i;
		}
		++frameIdx;
	}

	struct BoneChannelData {
		std::shared_ptr<panima::Channel> positionChannel;
		std::shared_ptr<panima::Channel> rotationChannel;
		std::shared_ptr<panima::Channel> scaleChannel;
	};
	std::unordered_map<std::string, std::shared_ptr<BoneChannelData>> boneNameToChannelData;
	boneNameToChannelData.reserve(boneValues.size());
	auto panimaAnim = std::make_shared<panima::Animation>();
	for(auto &pair : boneValues) {
		auto &boneName = pair.first;
		auto &valueData = pair.second;
		auto basePath = "ec/animated/bone/" + boneName + "/";
		auto boneId = skel.LookupBone(boneName);

		enum class PoseComponent : uint8_t { Position = 0, Rotation, Scale };
		auto toChannel = [&valueData, &refPoseRel, &skel, &boneName](PoseComponent eComponent) -> std::shared_ptr<panima::Channel> {
			std::array<std::vector<Vector2>, 3> components;
			for(auto &cdata : components)
				cdata.reserve(valueData.size());

			auto getPoseValue = [eComponent](const umath::ScaledTransform &pose) {
				switch(eComponent) {
				case PoseComponent::Position:
					return pose.GetOrigin();
				case PoseComponent::Rotation:
					{
						EulerAngles ang {pose.GetRotation()};
						return Vector3 {ang.p, ang.y, ang.r};
					}
				case PoseComponent::Scale:
					return pose.GetScale();
				}
				//unreachable
				return Vector3 {0, 0, 0};
			};
			for(auto &vd : valueData) {
				auto val = getPoseValue(vd.pose);
				components[0].push_back({vd.time, val.x});
				components[1].push_back({vd.time, val.y});
				components[2].push_back({vd.time, val.z});
			}

			auto origCdata = components;
			for(auto &cdata : components)
				cdata = bezierfit::reduce(cdata);

			std::vector<float> times;
			std::vector<Vector3> values;
			std::array<std::vector<Vector2>::iterator, 3> iterators = {components[0].begin(), components[1].begin(), components[2].begin()};
			std::optional<float> prevTime {};
			auto getNextTime = [&iterators, &components, &prevTime]() -> std::optional<float> {
				std::optional<float> t {};
				for(auto i = decltype(iterators.size()) {0u}; i < iterators.size(); ++i) {
					auto &it = iterators[i];
					while(it != components[i].end() && prevTime && fabs(it->x - *prevTime) < panima::Channel::TIME_EPSILON) {
						// Already covered; Skip
						++it;
					}
					if(it == components[i].end())
						continue;
					auto tc = it->x;
					if(!t || tc < *t) {
						t = tc;
						++it;
					}
				}
				prevTime = t;
				return t;
			};

			times.reserve(components[0].size() + components[1].size() + components[2].size());
			values.reserve(times.capacity());

			// Calculate data value for each timestamp
			auto t = getNextTime();
			while(t) {
				// Find index for t
				auto it = std::lower_bound(valueData.begin(), valueData.end(), *t - panima::Channel::TIME_EPSILON, [](const ValueData &valueData, float t) { return valueData.time < t; });
				assert(it != valueData.end());
				if(it == valueData.end())
					throw std::logic_error {"Timestamp value not found!"};
				auto idx = it - valueData.begin();
				times.push_back(*t);
				values.push_back(Vector3 {origCdata[0][idx].y, origCdata[1][idx].y, origCdata[2][idx].y});

				// Insert into xdata
				t = getNextTime();
			}

			if(values.size() == 2) {
				auto &v0 = values[0];
				auto &v1 = values[1];
				if(uvec::cmp<Vector3>(v0, v1)) {
					times.erase(times.end() - 1);
					values.erase(values.end() - 1);
				}
			}

			if(times.empty())
				return nullptr;

			auto channel = std::make_shared<panima::Channel>();

			auto &timeArray = channel->GetTimesArray();
			timeArray.Resize(times.size());
			memcpy(timeArray.GetValuePtr(0), times.data(), util::size_of_container(times));

			if(eComponent != PoseComponent::Rotation) {
				auto &valueArray = channel->GetValueArray();
				valueArray.SetValueType(udm::Type::Vector3);
				valueArray.Resize(values.size());
				memcpy(valueArray.GetValuePtr(0), values.data(), util::size_of_container(values));
			}
			else {
				auto &valueArray = channel->GetValueArray();
				valueArray.SetValueType(udm::Type::Quaternion);

				std::vector<Quat> quatValues;
				quatValues.reserve(values.size());
				for(auto &v : values)
					quatValues.push_back(uquat::create(EulerAngles {v.x, v.y, v.z}));

				valueArray.Resize(quatValues.size());
				memcpy(valueArray.GetValuePtr(0), quatValues.data(), util::size_of_container(quatValues));
			}

			channel->Optimize();

			if(channel->GetValueCount() == 1 && refPoseRel) {
				// If there is only one value, we may be able to skip the channel altogether, if it is the same as from the reference pose
				auto boneId = skel.LookupBone(boneName);
				umath::ScaledTransform pose;
				if(refPoseRel->GetBonePose(boneId, pose)) {
					switch(eComponent) {
					case PoseComponent::Position:
						{
							auto &val = pose.GetOrigin();
							auto &channelVal = channel->GetValue<Vector3>(0);
							if(uvec::is_equal<Vector3>(channelVal, val))
								return nullptr;
							break;
						}
					case PoseComponent::Rotation:
						{
							auto &val = pose.GetRotation();
							auto &channelVal = channel->GetValue<Quat>(0);
							if(uvec::is_equal<Quat>(channelVal, val))
								return nullptr;
							break;
						}
					case PoseComponent::Scale:
						{
							auto &val = pose.GetScale();
							auto &channelVal = channel->GetValue<Vector3>(0);
							if(uvec::is_equal<Vector3>(channelVal, val))
								return nullptr;
							break;
						}
					}
				}
			}

			return channel;
		};

		auto positionChannel = toChannel(PoseComponent::Position);
		if(positionChannel) {
			positionChannel->targetPath = panima::ChannelPath {basePath + "position"};
			panimaAnim->AddChannel(*positionChannel);
		}

		auto rotationChannel = toChannel(PoseComponent::Rotation);
		if(rotationChannel) {
			rotationChannel->targetPath = panima::ChannelPath {basePath + "rotation"};
			panimaAnim->AddChannel(*rotationChannel);
		}

		auto scaleChannel = toChannel(PoseComponent::Scale);
		if(scaleChannel) {
			scaleChannel->targetPath = panima::ChannelPath {basePath + "scale"};
			panimaAnim->AddChannel(*scaleChannel);
		}
	}
	return panimaAnim;
}

bool pragma::animation::Animation::operator==(const Animation &other) const
{
	if(m_frames.size() != other.m_frames.size() || m_boneWeights.size() != other.m_boneWeights.size() || static_cast<bool>(m_fadeIn) != static_cast<bool>(other.m_fadeIn) || static_cast<bool>(m_fadeOut) != static_cast<bool>(other.m_fadeOut) || m_events.size() != other.m_events.size())
		return false;
	if(m_fadeIn && umath::abs(*m_fadeIn - *other.m_fadeIn) > 0.001f)
		return false;
	if(m_fadeOut && umath::abs(*m_fadeOut - *other.m_fadeOut) > 0.001f)
		return false;
	for(auto i = decltype(m_frames.size()) {0u}; i < m_frames.size(); ++i) {
		if(*m_frames[i] != *other.m_frames[i])
			return false;
	}
	for(auto &pair : m_events) {
		if(other.m_events.find(pair.first) == other.m_events.end())
			return false;
	}
	for(auto i = decltype(m_boneWeights.size()) {0u}; i < m_boneWeights.size(); ++i) {
		if(umath::abs(m_boneWeights[i] - other.m_boneWeights[i]) > 0.001f)
			return false;
	}
#ifdef _MSC_VER
	static_assert(sizeof(Animation) == 312, "Update this function when making changes to this class!");
#endif
	return m_boneIds == other.m_boneIds && m_boneIdMap == other.m_boneIdMap && m_flags == other.m_flags && m_activity == other.m_activity && m_activityWeight == other.m_activityWeight && uvec::cmp(m_renderBounds.first, other.m_renderBounds.first)
	  && uvec::cmp(m_renderBounds.second, other.m_renderBounds.second) && m_blendController == other.m_blendController;
}
