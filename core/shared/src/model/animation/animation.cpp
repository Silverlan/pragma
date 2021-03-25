/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/model/animation/animation.h"
#include "pragma/model/animation/activities.h"
#include <udm.hpp>
#include <mathutil/umath.h>
#pragma optimize("",off)
decltype(Animation::s_activityEnumRegister) Animation::s_activityEnumRegister;
decltype(Animation::s_eventEnumRegister) Animation::s_eventEnumRegister;

util::EnumRegister &Animation::GetActivityEnumRegister() {return s_activityEnumRegister;}
util::EnumRegister &Animation::GetEventEnumRegister() {return s_eventEnumRegister;}


std::shared_ptr<Animation> Animation::Load(const udm::AssetData &data,std::string &outErr)
{
	auto anim = Animation::Create();
	if(anim->LoadFromAssetData(data,outErr) == false)
		return nullptr;
	return anim;
}

template<class T0,class T1>
	static void copy_safe(const T0 &src,T1 &dst,uint32_t srcStartIndex,uint32_t dstStartIndex,uint32_t count)
{
	auto *memPtr0 = reinterpret_cast<const uint8_t*>(src.data());
	auto *memPtr1 = reinterpret_cast<uint8_t*>(dst.data());
	memPtr0 += srcStartIndex *sizeof(T0::value_type);
	memPtr1 += dstStartIndex *sizeof(T1::value_type);

	auto *memPtr0End = memPtr0 +sizeof(T0::value_type) *src.size();
	auto *memPtr1End = memPtr1 +sizeof(T1::value_type) *dst.size();

	if(memPtr0 > memPtr0End)
		throw std::runtime_error{"Memory out of bounds!"};

	if(memPtr1 > memPtr1End)
		throw std::runtime_error{"Memory out of bounds!"};

	auto *memPtr0Write = memPtr0 +count *sizeof(T0::value_type);
	auto *memPtr1WriteEnd = memPtr1 +count *sizeof(T0::value_type);

	if(memPtr0Write > memPtr0End)
		throw std::runtime_error{"Memory out of bounds!"};

	if(memPtr1WriteEnd > memPtr1End)
		throw std::runtime_error{"Memory out of bounds!"};

	memcpy(memPtr1,memPtr0,count *sizeof(T0::value_type));
}

bool Animation::LoadFromAssetData(const udm::AssetData &data,std::string &outErr)
{
	if(data.GetAssetType() != PANIM_IDENTIFIER)
	{
		outErr = "Incorrect format!";
		return false;
	}

	auto udm = *data;
	auto version = data.GetAssetVersion();
	if(version < 1)
	{
		outErr = "Invalid version!";
		return false;
	}
	// if(version > PANIM_VERSION)
	// 	return false;
	auto activity = udm["activity"];
	if(activity && activity->IsType(udm::Type::String))
	{
		auto id = Animation::GetActivityEnumRegister().RegisterEnum(activity->GetValue<udm::String>());
		m_activity = (id != util::EnumRegister::InvalidEnum) ? static_cast<Activity>(id) : Activity::Invalid;
	}

	udm["activityWeight"](m_activityWeight);
	udm["fps"](m_fps);

	m_renderBounds.first = udm["renderBounds"]["min"](Vector3{std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()});
	m_renderBounds.second = udm["renderBounds"]["max"](Vector3{std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest()});

	auto fadeInTime = udm["fadeInTime"];
	if(fadeInTime)
		m_fadeIn = std::make_unique<float>(fadeInTime(0.f));

	auto fadeOutTime = udm["fadeOutTime"];
	if(fadeOutTime)
		m_fadeOut = std::make_unique<float>(fadeOutTime(0.f));

	udm["bones"](m_boneIds);
	auto numBones = m_boneIds.size();
	m_boneIdMap.reserve(numBones);
	for(auto i=decltype(numBones){0u};i<numBones;++i)
		m_boneIdMap[m_boneIds[i]] = i;
	
	udm["boneWeights"](m_boneWeights);

	auto udmBlendController = udm["blendController"];
	if(udmBlendController)
	{
		m_blendController = AnimationBlendController{};
		udmBlendController["controller"](m_blendController->controller);
		
		auto udmTransitions = udmBlendController["transitions"];
		m_blendController->transitions.resize(udmTransitions.GetSize());
		uint32_t idxTransition = 0;
		for(auto &udmTransition : udmTransitions)
		{
			udmTransition["animation"](m_blendController->transitions[idxTransition].animation);
			udmTransition["transition"](m_blendController->transitions[idxTransition].transition);
			++idxTransition;
		}
		udmBlendController["animationPostBlendController"](m_blendController->animationPostBlendController);
		udmBlendController["animationPostBlendTarget"](m_blendController->animationPostBlendTarget);
	}

	m_flags = FAnim::None;
	auto udmFlags = udm["flags"];
	if(udmFlags)
	{
		auto readFlag = [this,&udmFlags](FAnim flag,const std::string &name) {
			auto udmFlag = udmFlags[name];
			if(udmFlag && udmFlag(false))
				m_flags |= flag;
		};
		readFlag(FAnim::Loop,"loop");
		readFlag(FAnim::NoRepeat,"noRepeat");
		readFlag(FAnim::Autoplay,"autoplay");
		readFlag(FAnim::Gesture,"gesture");
		readFlag(FAnim::NoMoveBlend,"noMoveBlend");
		static_assert(umath::to_integral(FAnim::Count) == 7,"Update this list when new flags have been added!");
	}

	auto udmFrameData = udm["frameTransforms"];
	if(udmFrameData)
	{
		std::vector<umath::Transform> transforms;
		if(udmFrameData.GetBlobData(transforms) == udm::BlobResult::Success)
		{
			auto numFrames = transforms.size() /m_boneIds.size();
			m_frames.resize(numFrames);
			uint32_t offset = 0;
			for(auto &frame : m_frames)
			{
				frame = Frame::Create(numBones);
				auto &frameTransforms = frame->GetBoneTransforms();
				copy_safe(transforms,frameTransforms,offset,0,frameTransforms.size());
				offset += frameTransforms.size();
			}
		}
	}

	auto udmFrameScales = udm["frameScales"];
	if(udmFrameScales)
	{
		std::vector<Vector3> scales;
		if(udmFrameScales.GetBlobData(scales) == udm::BlobResult::Success)
		{
			uint32_t offset = 0;
			for(auto &frame : m_frames)
			{
				auto &frameScales = frame->GetBoneScales();
				frameScales.resize(numBones);
				copy_safe(scales,frameScales,offset,0,frameScales.size());
				offset += frameScales.size();
			}
		}
	}

	auto udmFrameMoveTranslations = udm["frameMoveTranslations"];
	if(udmFrameMoveTranslations)
	{
		std::vector<Vector2> moveTranslations;
		if(udmFrameMoveTranslations.GetBlobData(moveTranslations) == udm::BlobResult::Success)
		{
			for(auto i=decltype(m_frames.size()){0u};i<m_frames.size();++i)
			{
				auto &frame = m_frames[i];
				frame->SetMoveOffset(moveTranslations[i]);
			}
		}
	}

	auto udmEvents = udm["events"];
	if(udmEvents)
	{
		for(auto &udmEvent : udmEvents)
		{
			auto frameIndex = udmEvent["frame"].ToValue<uint32_t>();
			if(frameIndex.has_value() == false)
				continue;
			auto it = m_events.find(*frameIndex);
			if(it == m_events.end())
				it = m_events.insert(std::make_pair(*frameIndex,std::vector<std::shared_ptr<AnimationEvent>>{})).first;

			auto &frameEvents = it->second;
			if(frameEvents.size() == frameEvents.capacity())
				frameEvents.reserve(frameEvents.size() *1.1);

			auto name = udmEvent["name"](std::string{});
			if(name.empty())
				continue;
			auto id = Animation::GetEventEnumRegister().RegisterEnum(name);
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

bool Animation::Save(udm::AssetData &outData,std::string &outErr)
{
	outData.SetAssetType(PANIM_IDENTIFIER);
	outData.SetAssetVersion(PANIM_VERSION);
	auto udm = *outData;

	auto act = GetActivity();
	auto *activityName = Animation::GetActivityEnumRegister().GetEnumName(umath::to_integral(act));
	if(activityName)
		udm["activity"] = *activityName;
	else
		udm.Add("activity",udm::Type::Nil);
	udm["activityWeight"] = static_cast<uint8_t>(GetActivityWeight());
	udm["fps"] = static_cast<float>(GetFPS());

	auto &renderBounds = GetRenderBounds();
	udm["renderBounds"]["min"] = renderBounds.first;
	udm["renderBounds"]["max"] = renderBounds.second;

	if(HasFadeInTime())
		udm["fadeInTime"] = GetFadeInTime();
	else
		udm.Add("fadeInTime",udm::Type::Nil);
	
	if(HasFadeOutTime())
		udm["fadeOutTime"] = GetFadeOutTime();
	else
		udm.Add("fadeOutTime",udm::Type::Nil);

	auto &bones = GetBoneList();
	auto numBones = bones.size();
	udm["bones"] = bones;
	
	auto &weights = GetBoneWeights();
	auto it = std::find_if(weights.begin(),weights.end(),[](const float weight){
		return (weight != 1.f) ? true : false;
	});
	auto hasWeights = (it != weights.end());
	if(hasWeights)
		udm["boneWeights"] = weights;

	auto *blendController = GetBlendController();
	if(blendController)
	{
		udm["blendController"]["controller"] = blendController->controller;
		auto &transitions = blendController->transitions;
		auto udmTransitions = udm["blendController"].AddArray("transitions",transitions.size());
		for(auto i=decltype(transitions.size()){0u};i<transitions.size();++i)
		{
			auto &transition = transitions[i];
			auto udmTransition = udmTransitions[i];
			udmTransition["animation"] = transition.animation;
			udmTransition["transition"] = transition.transition;
		}
		udm["blendController"]["animationPostBlendController"] = blendController->animationPostBlendController;
		udm["blendController"]["animationPostBlendTarget"] = blendController->animationPostBlendTarget;
	}

	auto animFlags = GetFlags();
	auto writeFlag = [&udm,animFlags](FAnim flag,const std::string &name) {
		if(umath::is_flag_set(animFlags,flag) == false)
			return;
		udm["flags"][name] = true;
	};
	writeFlag(FAnim::Loop,"loop");
	writeFlag(FAnim::NoRepeat,"noRepeat");
	writeFlag(FAnim::Autoplay,"autoplay");
	writeFlag(FAnim::Gesture,"gesture");
	writeFlag(FAnim::NoMoveBlend,"noMoveBlend");
	static_assert(umath::to_integral(FAnim::Count) == 7,"Update this list when new flags have been added!");
	
	std::vector<umath::Transform> transforms;
	transforms.resize(m_frames.size() *numBones);
	std::vector<Vector3> scales;
	std::vector<Vector2> moveTranslations;
	uint32_t offset = 0;
	uint32_t frameIdx = 0;
	for(auto &frame : m_frames)
	{
		auto &frameTransforms = frame->GetBoneTransforms();
		if(frameTransforms.size() != numBones)
		{
			outErr = "Number of transforms (" +std::to_string(frameTransforms.size()) +" in frame does not match number of bones (" +std::to_string(numBones) +")!";
			return false;
		}
		copy_safe(frameTransforms,transforms,0,offset,frameTransforms.size());

		if(frame->HasScaleTransforms())
		{
			if(scales.empty())
				scales.resize(m_frames.size() *numBones,Vector3{1.f,1.f,1.f});
			auto &frameScales = frame->GetBoneScales();
			if(frameScales.size() != numBones)
			{
				outErr = "Number of scales (" +std::to_string(frameTransforms.size()) +" in frame does not match number of bones (" +std::to_string(numBones) +")!";
				return false;
			}
			copy_safe(frameScales,scales,0,offset,frameScales.size());
		}

		if(frame->GetMoveOffset())
		{
			auto &moveOffset = *frame->GetMoveOffset();
			if(moveOffset.x != 0.f || moveOffset.y != 0.f)
			{
				if(moveTranslations.empty())
					moveTranslations.resize(m_frames.size(),Vector2{0.f,0.f});
				moveTranslations.at(frameIdx) = moveOffset;
			}
		}
		offset += frameTransforms.size();
		++frameIdx;
	}
	udm["frames"] = static_cast<uint32_t>(m_frames.size());
	udm["frameTransforms"] = udm::compress_lz4_blob(transforms);
	if(!scales.empty())
		udm["frameScales"] = udm::compress_lz4_blob(scales);
	if(!moveTranslations.empty())
		udm["frameMoveTranslations"] = udm::compress_lz4_blob(moveTranslations);

	uint32_t numEvents = 0;
	for(auto &pair : m_events)
		numEvents += pair.second.size();
	auto udmEvents = udm.AddArray("events",numEvents);
	uint32_t evIdx = 0;
	for(auto &pair : m_events)
	{
		for(auto &ev : pair.second)
		{
			auto udmEvent = udmEvents[evIdx++];
			udmEvent["frame"] = pair.first;

			auto *eventName = Animation::GetEventEnumRegister().GetEnumName(umath::to_integral(ev->eventID));
			udmEvent["name"] = (eventName != nullptr) ? *eventName : "";
			udmEvent["args"] = ev->arguments;
		}
	}
	return true;
}

bool Animation::SaveLegacy(VFilePtrReal &f)
{
	f->Write<uint32_t>(PRAGMA_ANIMATION_VERSION);
	auto offsetToLen = f->Tell();
	f->Write<uint64_t>(0);
	auto animFlags = GetFlags();
	auto bMoveX = ((animFlags &FAnim::MoveX) == FAnim::MoveX) ? true : false;
	auto bMoveZ = ((animFlags &FAnim::MoveZ) == FAnim::MoveZ) ? true : false;
	auto bHasMovement = (bMoveX || bMoveZ) ? true : false;

	auto act = GetActivity();
	auto *activityName = Animation::GetActivityEnumRegister().GetEnumName(umath::to_integral(act));
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
	auto it = std::find_if(weights.begin(),weights.end(),[](const float weight){
		return (weight != 1.f) ? true : false;
		});
	if(it == weights.end())
		f->Write<bool>(false);
	else
	{
		f->Write<bool>(true);
		for(auto i=decltype(numBones){0};i<numBones;++i)
			f->Write<float>((i < weights.size()) ? weights.at(i) : 1.f);
	}

	auto *blendController = GetBlendController();
	f->Write<bool>(blendController);
	if(blendController)
	{
		f->Write<int32_t>(blendController->controller);
		auto &transitions = blendController->transitions;
		f->Write<int8_t>(static_cast<int8_t>(transitions.size()));
		for(auto &t : transitions)
		{
			f->Write<uint32_t>(t.animation -1); // Account for reference pose
			f->Write<float>(t.transition);
		}

		f->Write<int32_t>(blendController->animationPostBlendController);
		f->Write<int32_t>(blendController->animationPostBlendTarget);
	}

	auto numFrames = GetFrameCount();
	f->Write<uint32_t>(numFrames);
	for(auto i=decltype(numFrames){0};i<numFrames;++i)
	{
		auto &frame = *GetFrame(i);
		for(auto j=decltype(numBones){0};j<numBones;++j)
		{
			auto &pos = *frame.GetBonePosition(static_cast<uint32_t>(j));
			auto &rot = *frame.GetBoneOrientation(static_cast<uint32_t>(j));
			f->Write<Quat>(rot);
			f->Write<Vector3>(pos);
		}

		if(frame.HasScaleTransforms())
		{
			auto &scales = frame.GetBoneScales();
			f->Write<uint32_t>(scales.size());
			f->Write(scales.data(),scales.size() *sizeof(scales.front()));
		}
		else
			f->Write<uint32_t>(static_cast<uint32_t>(0u));

		auto *animEvents = GetEvents(i);
		auto numEvents = (animEvents != nullptr) ? animEvents->size() : 0;
		f->Write<uint16_t>(static_cast<uint16_t>(numEvents));
		if(animEvents != nullptr)
		{
			for(auto &ev : *animEvents)
			{
				auto *eventName = Animation::GetEventEnumRegister().GetEnumName(umath::to_integral(ev->eventID));
				f->WriteString((eventName != nullptr) ? *eventName : "");
				f->Write<uint8_t>(static_cast<uint8_t>(ev->arguments.size()));
				for(auto &arg : ev->arguments)
					f->WriteString(arg);
			}
		}

		if(bHasMovement == true)
		{
			auto &moveOffset = *frame.GetMoveOffset();
			if(bMoveX == true)
				f->Write<float>(moveOffset.x);
			if(bMoveZ == true)
				f->Write<float>(moveOffset.y);
		}
	}

	auto curOffset = f->Tell();
	auto len = curOffset -offsetToLen;
	f->Seek(offsetToLen);
	f->Write<uint64_t>(len);
	f->Seek(curOffset);
	return true;
}

std::shared_ptr<Animation> Animation::Create()
{
	return std::shared_ptr<Animation>(new Animation{});
}
std::shared_ptr<Animation> Animation::Create(const Animation &other,ShareMode share)
{
	return std::shared_ptr<Animation>(new Animation{other,share});
}

Animation::Animation()
	: m_flags(FAnim::None),m_activity(Activity::Invalid),m_activityWeight(1),m_fps(24),
	m_fadeIn(nullptr),m_fadeOut(nullptr)
{}

Animation::Animation(const Animation &other,ShareMode share)
	: m_boneIds(other.m_boneIds),m_boneIdMap(other.m_boneIdMap),m_flags(other.m_flags),m_activity(other.m_activity),
	m_activityWeight(other.m_activityWeight),m_fps(other.m_fps),m_boneWeights(other.m_boneWeights),
	m_renderBounds(other.m_renderBounds),m_blendController{other.m_blendController}
{
	m_fadeIn = (other.m_fadeIn != nullptr) ? std::make_unique<float>(*other.m_fadeIn) : nullptr;
	m_fadeOut = (other.m_fadeOut != nullptr) ? std::make_unique<float>(*other.m_fadeOut) : nullptr;

	if((share &ShareMode::Frames) != ShareMode::None)
		m_frames = other.m_frames;
	else
	{
		m_frames.reserve(other.m_frames.size());
		for(auto &frame : other.m_frames)
			m_frames.push_back(Frame::Create(*frame));
	}

	if((share &ShareMode::Events) != ShareMode::None)
		m_events = other.m_events;
	else
	{
		for(auto &pair : other.m_events)
		{
			m_events[pair.first] = std::vector<std::shared_ptr<AnimationEvent>>{};
			auto &events = m_events[pair.first];
			events.reserve(pair.second.size());
			for(auto &ev : pair.second)
				events.push_back(std::make_unique<AnimationEvent>(*ev));
		}
	}
	static_assert(sizeof(Animation) == 312,"Update this function when making changes to this class!");
}

void Animation::Reverse()
{
	std::reverse(m_frames.begin(),m_frames.end());
}

void Animation::Rotate(const Skeleton &skeleton,const Quat &rot)
{
	uvec::rotate(&m_renderBounds.first,rot);
	uvec::rotate(&m_renderBounds.second,rot);
	for(auto &frame : m_frames)
		frame->Rotate(*this,skeleton,rot);
}
void Animation::Translate(const Skeleton &skeleton,const Vector3 &t)
{
	m_renderBounds.first += t;
	m_renderBounds.second += t;
	for(auto &frame : m_frames)
		frame->Translate(*this,skeleton,t);
}

void Animation::Scale(const Vector3 &scale)
{
	m_renderBounds.first *= scale;
	m_renderBounds.second *= scale;
	for(auto &frame : m_frames)
		frame->Scale(scale);
}

int32_t Animation::LookupBone(uint32_t boneId) const
{
	if(boneId < m_boneIds.size() && m_boneIds.at(boneId) == boneId) // Faster than map lookup and this statement is true for most cases
		return boneId;
	auto it = m_boneIdMap.find(boneId);
	if(it == m_boneIdMap.end())
		return -1;
	return it->second;
}

void Animation::CalcRenderBounds(Model &mdl)
{
	m_renderBounds = {
		{std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()},
		{std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest()}
	};
	for(auto &frame : m_frames)
	{
		auto frameBounds = frame->CalcRenderBounds(*this,mdl);
		for(uint8_t j=0;j<3;++j)
		{
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

const std::pair<Vector3,Vector3> &Animation::GetRenderBounds() const {return m_renderBounds;}
void Animation::SetRenderBounds(const Vector3 &min,const Vector3 &max) {m_renderBounds = {min,max};}

std::vector<std::shared_ptr<Frame>> &Animation::GetFrames() {return m_frames;}

void Animation::Localize(const Skeleton &skeleton)
{
	for(auto it=m_frames.begin();it!=m_frames.end();++it)
		(*it)->Localize(*this,skeleton);
}
AnimationBlendController &Animation::SetBlendController(uint32_t controller)
{
	m_blendController = AnimationBlendController{};
	m_blendController->controller = controller;
	return *m_blendController;
}
AnimationBlendController *Animation::GetBlendController() {return m_blendController.has_value() ? &*m_blendController : nullptr;}
const AnimationBlendController *Animation::GetBlendController() const {return const_cast<Animation*>(this)->GetBlendController();}
void Animation::ClearBlendController() {m_blendController = {};}
float Animation::GetFadeInTime()
{
	if(m_fadeIn == nullptr)
		return 0.f;
	return *m_fadeIn;
}
float Animation::GetFadeOutTime()
{
	if(m_fadeOut == nullptr)
		return 0.f;
	return *m_fadeOut;
}
void Animation::SetFadeInTime(float t)
{
	if(m_fadeIn == nullptr)
		m_fadeIn = std::make_unique<float>();
	*m_fadeIn = t;
}
void Animation::SetFadeOutTime(float t)
{
	if(m_fadeOut == nullptr)
		m_fadeOut = std::make_unique<float>();
	*m_fadeOut = t;
}
bool Animation::HasFadeInTime() {return (m_fadeIn != nullptr) ? true : false;}
bool Animation::HasFadeOutTime() {return (m_fadeOut != nullptr) ? true : false;}
Activity Animation::GetActivity() const {return m_activity;}
void Animation::SetActivity(Activity activity) {m_activity = activity;}
unsigned char Animation::GetActivityWeight() const {return m_activityWeight;}
void Animation::SetActivityWeight(unsigned char weight) {m_activityWeight = weight;}
unsigned char Animation::GetFPS() {return m_fps;}
void Animation::SetFPS(unsigned char fps) {m_fps = fps;}
float Animation::GetDuration()
{
	if(m_fps == 0)
		return 0.f;
	return float(m_frames.size()) /float(m_fps);
}

FAnim Animation::GetFlags() const {return m_flags;}
void Animation::SetFlags(FAnim flags) {m_flags = flags;}
bool Animation::HasFlag(FAnim flag) const {return ((m_flags &flag) == flag) ? true : false;}
void Animation::AddFlags(FAnim flags) {m_flags |= flags;}
void Animation::RemoveFlags(FAnim flags) {m_flags &= ~flags;}

const std::vector<unsigned int> &Animation::GetBoneList() const {return m_boneIds;}
const std::unordered_map<uint32_t,uint32_t> &Animation::GetBoneMap() const {return m_boneIdMap;}
uint32_t Animation::AddBoneId(uint32_t id)
{
	auto it = m_boneIdMap.find(id);
	if(it != m_boneIdMap.end())
		return it->second;
	m_boneIds.push_back(id);
	m_boneIdMap.insert(std::make_pair(id,m_boneIds.size() -1));
	return m_boneIds.size() -1;
}
void Animation::SetBoneId(uint32_t localIdx,uint32_t id)
{
	if(localIdx >= m_boneIds.size())
		return;
	auto &oldId = m_boneIds.at(localIdx);
	auto it = m_boneIdMap.find(oldId);
	if(it != m_boneIdMap.end())
		m_boneIdMap.erase(it);

	oldId = id;
	m_boneIdMap.insert(std::make_pair(id,localIdx));
}
void Animation::SetBoneList(const std::vector<uint32_t> &list)
{
	m_boneIds = list;
	m_boneIdMap.clear();
	m_boneIdMap.reserve(list.size());
	for(auto i=decltype(list.size()){0};i<list.size();++i)
		m_boneIdMap.insert(std::make_pair(list.at(i),i));
}
void Animation::ReserveBoneIds(uint32_t count)
{
	m_boneIds.reserve(count);
	m_boneIdMap.reserve(count);
}

void Animation::AddFrame(std::shared_ptr<Frame> frame) {m_frames.push_back(frame);}

std::shared_ptr<Frame> Animation::GetFrame(unsigned int ID)
{
	if(ID >= m_frames.size())
		return nullptr;
	return m_frames[ID];
}

unsigned int Animation::GetFrameCount() {return CUInt32(m_frames.size());}

unsigned int Animation::GetBoneCount() {return CUInt32(m_boneIds.size());}

void Animation::AddEvent(unsigned int frame,AnimationEvent *ev)
{
	auto it = m_events.find(frame);
	if(it == m_events.end())
		m_events[frame] = std::vector<std::shared_ptr<AnimationEvent>>{};
	m_events[frame].push_back(std::shared_ptr<AnimationEvent>(ev));
}

std::vector<std::shared_ptr<AnimationEvent>> *Animation::GetEvents(unsigned int frame)
{
	auto it = m_events.find(frame);
	if(it == m_events.end())
		return nullptr;
	return &it->second;
}

float Animation::GetBoneWeight(uint32_t boneId) const
{
	auto weight = 1.f;
	GetBoneWeight(boneId,weight);
	return weight;
}
bool Animation::GetBoneWeight(uint32_t boneId,float &weight) const
{
	if(boneId >= m_boneWeights.size())
		return false;
	weight = m_boneWeights.at(boneId);
	return true;
}
const std::vector<float> &Animation::GetBoneWeights() const {return const_cast<Animation*>(this)->GetBoneWeights();}
std::vector<float> &Animation::GetBoneWeights() {return m_boneWeights;}
void Animation::SetBoneWeight(uint32_t boneId,float weight)
{
	if(boneId >= m_boneIds.size())
		return;
	if(m_boneIds.size() > m_boneWeights.size())
		m_boneWeights.resize(m_boneIds.size(),1.f);
	m_boneWeights.at(boneId) = weight;
}

bool Animation::operator==(const Animation &other) const
{
	if(m_frames.size() != other.m_frames.size() || m_boneWeights.size() != other.m_boneWeights.size() || static_cast<bool>(m_fadeIn) != static_cast<bool>(other.m_fadeIn) || static_cast<bool>(m_fadeOut) != static_cast<bool>(other.m_fadeOut) || m_events.size() != other.m_events.size())
		return false;
	if(m_fadeIn && umath::abs(*m_fadeIn -*other.m_fadeIn) > 0.001f)
		return false;
	if(m_fadeOut && umath::abs(*m_fadeOut -*other.m_fadeOut) > 0.001f)
		return false;
	for(auto i=decltype(m_frames.size()){0u};i<m_frames.size();++i)
	{
		if(*m_frames[i] != *other.m_frames[i])
			return false;
	}
	for(auto &pair : m_events)
	{
		if(other.m_events.find(pair.first) == other.m_events.end())
			return false;
	}
	for(auto i=decltype(m_boneWeights.size()){0u};i<m_boneWeights.size();++i)
	{
		if(umath::abs(m_boneWeights[i] -other.m_boneWeights[i]) > 0.001f)
			return false;
	}
	static_assert(sizeof(Animation) == 312,"Update this function when making changes to this class!");
	return m_boneIds == other.m_boneIds && m_boneIdMap == other.m_boneIdMap && m_flags == other.m_flags && m_activity == other.m_activity &&
		m_activityWeight == other.m_activityWeight && uvec::cmp(m_renderBounds.first,other.m_renderBounds.first) && uvec::cmp(m_renderBounds.second,other.m_renderBounds.second) && m_blendController == other.m_blendController;
}
#pragma optimize("",on)
