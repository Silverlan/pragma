#include "stdafx_shared.h"
#include "pragma/model/animation/animation.h"
#include "pragma/model/animation/activities.h"
#include <mathutil/umath.h>

decltype(Animation::s_activityEnumRegister) Animation::s_activityEnumRegister;
decltype(Animation::s_eventEnumRegister) Animation::s_eventEnumRegister;

util::EnumRegister &Animation::GetActivityEnumRegister() {return s_activityEnumRegister;}
util::EnumRegister &Animation::GetEventEnumRegister() {return s_eventEnumRegister;}

Animation::Animation()
	: m_flags(FAnim::None),m_activity(Activity::Invalid),m_activityWeight(1),m_fps(24),
	m_fadeIn(nullptr),m_fadeOut(nullptr),m_blendController(nullptr)
{}

Animation::Animation(const Animation &other,ShareMode share)
	: m_boneIds(other.m_boneIds),m_boneIdMap(other.m_boneIdMap),m_flags(other.m_flags),m_activity(other.m_activity),
	m_activityWeight(other.m_activityWeight),m_fps(other.m_fps),m_boneWeights(other.m_boneWeights),
	m_renderBounds(other.m_renderBounds)
{
	m_fadeIn = (other.m_fadeIn != nullptr) ? std::make_unique<float>(*other.m_fadeIn) : nullptr;
	m_fadeOut = (other.m_fadeOut != nullptr) ? std::make_unique<float>(*other.m_fadeOut) : nullptr;
	m_blendController = (other.m_blendController != nullptr) ? std::make_unique<AnimationBlendController>(*other.m_blendController) : nullptr;

	if((share &ShareMode::Frames) != ShareMode::None)
		m_frames = other.m_frames;
	else
	{
		m_frames.reserve(other.m_frames.size());
		for(auto &frame : other.m_frames)
			m_frames.push_back(std::make_shared<Frame>(*frame));
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
AnimationBlendController *Animation::SetBlendController(unsigned int controller)
{
	m_blendController = std::make_unique<AnimationBlendController>();
	m_blendController->controller = controller;
	return m_blendController.get();
}
AnimationBlendController *Animation::GetBlendController()
{
	return m_blendController.get();
}
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
void Animation::AddBoneId(uint32_t id)
{
	auto it = m_boneIdMap.find(id);
	if(it != m_boneIdMap.end())
		return;
	m_boneIds.push_back(id);
	m_boneIdMap.insert(std::make_pair(id,m_boneIds.size() -1));
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
