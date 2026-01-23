// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_animated;

#define DEBUG_VERBOSE_ANIMATION 0

using namespace pragma;

void BaseAnimatedComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT = registerEvent("HANDLE_ANIMATION_EVENT", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_PLAY_ANIMATION = registerEvent("ON_PLAY_ANIMATION", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION = registerEvent("ON_PLAY_LAYERED_ANIMATION", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ACTIVITY = registerEvent("ON_PLAY_LAYERED_ACTIVITY", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE = registerEvent("ON_ANIMATION_COMPLETE", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_START = registerEvent("ON_LAYERED_ANIMATION_START", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE = registerEvent("ON_LAYERED_ANIMATION_COMPLETE", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_ANIMATION_START = registerEvent("ON_ANIMATION_START", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_TRANSLATE_LAYERED_ANIMATION = registerEvent("TRANSLATE_LAYERED_ANIMATION", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_TRANSLATE_ANIMATION = registerEvent("TRANSLATE_ANIMATION", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_TRANSLATE_ACTIVITY = registerEvent("TRANSLATE_ACTIVITY", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_MAINTAIN_ANIMATIONS = registerEvent("MAINTAIN_ANIMATIONS", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_MAINTAIN_ANIMATION_MT = registerEvent("MAINTAIN_ANIMATION", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_MAINTAIN_ANIMATION_MOVEMENT = registerEvent("MAINTAIN_ANIMATION_MOVEMENT", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_SHOULD_UPDATE_BONES = registerEvent("SHOULD_UPDATE_BONES", ComponentEventInfo::Type::Explicit);

	baseAnimatedComponent::EVENT_ON_PLAY_ACTIVITY = registerEvent("ON_PLAY_ACTIVITY", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_STOP_LAYERED_ANIMATION = registerEvent("ON_STOP_LAYERED_ANIMATION", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_BONE_TRANSFORM_CHANGED = registerEvent("ON_BONE_TRANSFORM_CHANGED", ComponentEventInfo::Type::Broadcast);
	baseAnimatedComponent::EVENT_ON_ANIMATIONS_UPDATED_MT = registerEvent("ON_ANIMATIONS_UPDATED_MT", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_UPDATE_BONE_POSES_MT = registerEvent("UPDATE_BONE_POSES", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_BONE_POSES_FINALIZED_MT = registerEvent("ON_BONE_POSES_FINALIZED", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_BLEND_ANIMATION_MT = registerEvent("ON_BLEND_ANIMATION", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_PLAY_ANIMATION = registerEvent("PLAY_ANIMATION", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_ANIMATION_RESET = registerEvent("ON_ANIMATION_RESET", ComponentEventInfo::Type::Broadcast);
	baseAnimatedComponent::EVENT_ON_ANIMATIONS_UPDATED = registerEvent("ON_ANIMATIONS_UPDATED", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_UPDATE_SKELETON = registerEvent("ON_ANIMATIONS_UPDATED", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_POST_ANIMATION_UPDATE = registerEvent("POST_ANIMATION_UPDATE", ComponentEventInfo::Type::Explicit);
	baseAnimatedComponent::EVENT_ON_RESET_POSE = registerEvent("ON_RESET_POSE", ComponentEventInfo::Type::Broadcast);
}

BaseAnimatedComponent::BaseAnimatedComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_playbackRate(util::FloatProperty::Create(1.f)) {}

void BaseAnimatedComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { OnModelChanged(static_cast<CEOnModelChanged &>(evData.get()).model); });

	BindEventUnhandled(basePhysicsComponent::EVENT_ON_PRE_PHYSICS_SIMULATE, [this](std::reference_wrapper<ComponentEvent> evData) {
		if(IsPlayingAnimation() == false)
			return;
		auto pPhysComponent = GetEntity().GetPhysicsComponent();
		if(!pPhysComponent)
			return;
		auto *phys = pPhysComponent->GetPhysicsObject();
		if(phys != nullptr && phys->IsController() == true)
			MaintainAnimationMovement(m_animDisplacement);
	});

	auto &ent = GetEntity();
	auto *mdlComponent = static_cast<BaseModelComponent *>(ent.AddComponent("model").get());
	if(mdlComponent != nullptr) {
		auto &mdl = mdlComponent->GetModel();
		OnModelChanged(mdl);
	}

	// SetTickPolicy(TickPolicy::WhenVisible);
}

void BaseAnimatedComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	GetNetworkState().GetGameState()->GetAnimationUpdateManager().UpdateEntityState(GetEntity());
}

void BaseAnimatedComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	GetNetworkState().GetGameState()->GetAnimationUpdateManager().UpdateEntityState(GetEntity());
}

bool BaseAnimatedComponent::IsAnimated() const { return math::is_flag_set(m_stateFlags, StateFlags::IsAnimated); }
void BaseAnimatedComponent::UpdateAnimations(double dt)
{
	if(ShouldUpdateBones() == false) {
		m_stateFlags &= ~StateFlags::IsAnimated;
		return;
	}
	m_stateFlags |= StateFlags::IsAnimated;
	auto &ent = GetEntity();
	auto pTimeScaleComponent = ent.GetTimeScaleComponent();
	MaintainAnimations(dt * (pTimeScaleComponent.valid() ? pTimeScaleComponent->GetEffectiveTimeScale() : 1.f));
}

void BaseAnimatedComponent::ResetAnimation(const std::shared_ptr<asset::Model> &mdl)
{
	m_animSlots.clear();
	m_baseAnim = {};
	m_blendControllers.clear();
	m_bones.clear();
	m_processedBones.clear();
	m_bindPose = nullptr;
	math::set_flag(m_stateFlags, StateFlags::AbsolutePosesDirty);
	ApplyAnimationEventTemplates();
	if(mdl == nullptr || mdl->HasVertexWeights() == false)
		return;
	m_bindPose = mdl->GetReference().shared_from_this();
	std::vector<asset::BlendController> &blendControllers = mdl->GetBlendControllers();
	for(unsigned int i = 0; i < blendControllers.size(); i++) {
		auto &blend = blendControllers[i];
		int val;
		if(blend.max < 0)
			val = blend.max;
		else
			val = 0;
		m_blendControllers.insert(std::unordered_map<unsigned int, int>::value_type(i, val));
	}
	auto &skeleton = mdl->GetSkeleton();
	auto numBones = skeleton.GetBoneCount();
	m_bones.reserve(numBones);
	for(unsigned int i = 0; i < numBones; i++)
		m_bones.push_back({});
	std::unordered_map<std::string, unsigned int> *animations;
	mdl->GetAnimations(&animations);
	std::unordered_map<std::string, unsigned int>::iterator it;
	int autoplaySlot = 1'200; // Arbitrary start slot number for autoplay layered animations
	for(it = animations->begin(); it != animations->end(); it++) {
		unsigned int animID = it->second;
		auto anim = mdl->GetAnimation(animID);
		if(anim->HasFlag(FAnim::Autoplay)) {
			PlayLayeredAnimation(autoplaySlot, animID);
			autoplaySlot++;
		}
	}

	m_processedBones.resize(numBones);
	for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
		math::ScaledTransform pose {};
		m_bindPose->GetBonePose(i, pose);
		m_processedBones[i] = pose;
	}

	for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
		auto bone = skeleton.GetBone(i);
		if(bone.expired())
			continue;
		auto pose = m_processedBones[i];
		auto parent = bone.lock()->parent.lock();
		if(parent)
			pose = m_processedBones[parent->ID].GetInverse() * pose;
		m_bones[i] = pose;
	}
}

const ComponentMemberInfo *BaseAnimatedComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto numStatic = GetStaticMemberCount();
	if(idx < numStatic)
		return BaseEntityComponent::GetMemberInfo(idx);
	return DynamicMemberRegister::GetMemberInfo(idx - numStatic);
}
std::optional<ComponentMemberIndex> BaseAnimatedComponent::DoGetMemberIndex(const std::string &name) const
{
	auto idx = BaseEntityComponent::DoGetMemberIndex(name);
	if(idx.has_value())
		return idx;
	idx = DynamicMemberRegister::GetMemberIndex(name);
	if(idx.has_value())
		return *idx + GetStaticMemberCount();
	return std::optional<ComponentMemberIndex> {};
}

void BaseAnimatedComponent::OnModelChanged(const std::shared_ptr<asset::Model> &mdl)
{
	ResetAnimation(mdl);
	BroadcastEvent(baseAnimatedComponent::EVENT_ON_ANIMATION_RESET);

	util::ScopeGuard sg {[this]() { OnMembersChanged(); }};
	ClearMembers();
	if(!mdl)
		return;
	auto &skeleton = mdl->GetSkeleton();
	auto relRef = Frame::Create(mdl->GetReference());
	relRef->Localize(skeleton);
	auto &bones = skeleton.GetBones();
	ReserveMembers(bones.size() * 3);
	std::function<void(const animation::Bone &, const std::string &)> fAddBone = nullptr;
	fAddBone = [&](const animation::Bone &bone, const std::string &parentPathName) {
		const auto &name = bone.name;
		std::string lname = name;
		// pragma::string::to_lower(lname);

		std::shared_ptr<ents::ParentTypeMetaData> parentMetaData {};
		if(!parentPathName.empty()) {
			parentMetaData = pragma::util::make_shared<ents::ParentTypeMetaData>();
			parentMetaData->parentProperty = parentPathName;
		}

		math::ScaledTransform defaultPose;
		relRef->GetBonePose(bone.ID, defaultPose);

		auto posePathName = "bone/" + lname + "/pose";
		auto posPathName = "bone/" + lname + "/position";
		auto rotPathName = "bone/" + lname + "/rotation";
		auto scalePathName = "bone/" + lname + "/scale";
		auto poseMetaData = pragma::util::make_shared<ents::PoseTypeMetaData>();
		poseMetaData->posProperty = posPathName;
		poseMetaData->rotProperty = rotPathName;
		poseMetaData->scaleProperty = scalePathName;

		auto poseComponentMetaData = pragma::util::make_shared<ents::PoseComponentTypeMetaData>();
		poseComponentMetaData->poseProperty = posePathName;

		auto coordMetaData = pragma::util::make_shared<ents::CoordinateTypeMetaData>();
		coordMetaData->space = math::CoordinateSpace::Local;
		coordMetaData->parentProperty = parentPathName;

		auto memberInfoPose = ComponentMemberInfo::CreateDummy();
		memberInfoPose.SetName(posePathName);
		memberInfoPose.AddTypeMetaData(poseMetaData);
		if(parentMetaData)
			memberInfoPose.AddTypeMetaData(parentMetaData);
		memberInfoPose.type = ents::EntityMemberType::ScaledTransform;
		memberInfoPose.userIndex = bone.ID;
		memberInfoPose.SetFlag(ComponentMemberFlags::HideInInterface);
		memberInfoPose.AddTypeMetaData(coordMetaData);
		memberInfoPose.SetGetterFunction<BaseAnimatedComponent, math::ScaledTransform,
		  static_cast<void (*)(const ComponentMemberInfo &, BaseAnimatedComponent &, math::ScaledTransform &)>([](const ComponentMemberInfo &memberInfo, BaseAnimatedComponent &component, math::ScaledTransform &outValue) {
			  if(!component.GetBonePose(memberInfo.userIndex, outValue)) {
				  outValue = {};
				  return;
			  }
		  })>();
		memberInfoPose.SetSetterFunction<BaseAnimatedComponent, math::ScaledTransform,
		  static_cast<void (*)(const ComponentMemberInfo &, BaseAnimatedComponent &, const math::ScaledTransform &)>(
		    [](const ComponentMemberInfo &memberInfo, BaseAnimatedComponent &component, const math::ScaledTransform &value) { component.SetBonePose(memberInfo.userIndex, value); })>();

		auto memberInfoPos = ComponentMemberInfo::CreateDummy();
		memberInfoPos.SetName("bone/" + lname + "/position");
		memberInfoPos.type = ents::EntityMemberType::Vector3;
		memberInfoPos.userIndex = bone.ID;
		memberInfoPos.SetDefault<Vector3>(defaultPose.GetOrigin());
		memberInfoPos.AddTypeMetaData(coordMetaData);
		memberInfoPos.AddTypeMetaData(poseComponentMetaData);
		if(parentMetaData)
			memberInfoPos.AddTypeMetaData(parentMetaData);
		memberInfoPos.SetGetterFunction<BaseAnimatedComponent, Vector3, static_cast<void (*)(const ComponentMemberInfo &, BaseAnimatedComponent &, Vector3 &)>([](const ComponentMemberInfo &memberInfo, BaseAnimatedComponent &component, Vector3 &outValue) {
			if(!component.GetBonePos(memberInfo.userIndex, outValue)) {
				outValue = {};
				return;
			}
		})>();
		memberInfoPos.SetSetterFunction<BaseAnimatedComponent, Vector3,
		  static_cast<void (*)(const ComponentMemberInfo &, BaseAnimatedComponent &, const Vector3 &)>([](const ComponentMemberInfo &memberInfo, BaseAnimatedComponent &component, const Vector3 &value) { component.SetBonePos(memberInfo.userIndex, value); })>();

		auto memberInfoRot = memberInfoPos;
		memberInfoRot.SetName("bone/" + lname + "/rotation");
		memberInfoRot.type = ents::EntityMemberType::Quaternion;
		memberInfoRot.userIndex = bone.ID;
		memberInfoRot.SetDefault<Quat>(defaultPose.GetRotation());
		memberInfoRot.AddTypeMetaData(coordMetaData);
		memberInfoRot.AddTypeMetaData(poseComponentMetaData);
		if(parentMetaData)
			memberInfoRot.AddTypeMetaData(parentMetaData);
		memberInfoRot.SetGetterFunction<BaseAnimatedComponent, Quat, static_cast<void (*)(const ComponentMemberInfo &, BaseAnimatedComponent &, Quat &)>([](const ComponentMemberInfo &memberInfo, BaseAnimatedComponent &component, Quat &outValue) {
			if(!component.GetBoneRot(memberInfo.userIndex, outValue)) {
				outValue = uquat::identity();
				return;
			}
		})>();
		memberInfoRot.SetSetterFunction<BaseAnimatedComponent, Quat,
		  static_cast<void (*)(const ComponentMemberInfo &, BaseAnimatedComponent &, const Quat &)>([](const ComponentMemberInfo &memberInfo, BaseAnimatedComponent &component, const Quat &value) { component.SetBoneRot(memberInfo.userIndex, value); })>();

		auto memberInfoScale = memberInfoPos;
		memberInfoScale.SetName("bone/" + lname + "/scale");
		memberInfoScale.userIndex = bone.ID;
		memberInfoScale.SetDefault<Vector3>(defaultPose.GetScale());
		memberInfoScale.AddTypeMetaData(coordMetaData);
		memberInfoScale.AddTypeMetaData(poseMetaData);
		if(parentMetaData)
			memberInfoScale.AddTypeMetaData(parentMetaData);
		memberInfoScale.SetGetterFunction<BaseAnimatedComponent, Vector3, static_cast<void (*)(const ComponentMemberInfo &, BaseAnimatedComponent &, Vector3 &)>([](const ComponentMemberInfo &memberInfo, BaseAnimatedComponent &component, Vector3 &outValue) {
			if(!component.GetBoneScale(memberInfo.userIndex, outValue)) {
				outValue = Vector3 {1.f, 1.f, 1.f};
				return;
			}
		})>();
		memberInfoScale.SetSetterFunction<BaseAnimatedComponent, Vector3,
		  static_cast<void (*)(const ComponentMemberInfo &, BaseAnimatedComponent &, const Vector3 &)>([](const ComponentMemberInfo &memberInfo, BaseAnimatedComponent &component, const Vector3 &value) { component.SetBoneScale(memberInfo.userIndex, value); })>();

		RegisterMember(std::move(memberInfoPose));
		RegisterMember(std::move(memberInfoPos));
		RegisterMember(std::move(memberInfoRot));
		RegisterMember(std::move(memberInfoScale));

		for(auto &pair : bone.children)
			fAddBone(*pair.second, posePathName);
	};
	for(auto &pair : skeleton.GetRootBones())
		fAddBone(*pair.second, "");
}

CallbackHandle BaseAnimatedComponent::BindAnimationEvent(AnimationEvent::Type eventId, const std::function<void(std::reference_wrapper<const AnimationEvent>)> &fCallback)
{
	auto it = m_boundAnimEvents.find(eventId);
	if(it != m_boundAnimEvents.end()) {
		if(it->second.IsValid())
			it->second.Remove();
		m_boundAnimEvents.erase(it);
	}
	auto hCb = FunctionCallback<void, std::reference_wrapper<const AnimationEvent>>::Create(fCallback);
	m_boundAnimEvents.insert(std::make_pair(eventId, hCb));
	return hCb;
}

bool BaseAnimatedComponent::IsPlayingAnimation() const { return (GetAnimation() >= 0) ? true : false; }

float BaseAnimatedComponent::GetAnimationDuration() const
{
	int seq = GetAnimation();
	if(seq == -1)
		return 0.f;
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return 0.f;
	auto anim = hModel->GetAnimation(seq);
	if(anim == nullptr)
		return 0.f;
	return (1.f - m_baseAnim.cycle) * anim->GetDuration();
}

int BaseAnimatedComponent::SelectWeightedAnimation(Activity activity, int animAvoid) const
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return -1;
	return hModel->SelectWeightedAnimation(activity, animAvoid);
}

void BaseAnimatedComponent::SetLastAnimationBlendScale(float scale) { m_baseAnim.lastAnim.blendScale = scale; }

void BaseAnimatedComponent::SetBlendController(unsigned int controller, float val)
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return;
	auto it = m_blendControllers.find(controller);
	if(it == m_blendControllers.end())
		return;
	auto *blend = hModel->GetBlendController(it->first);
	if(blend == nullptr)
		return;
	//if(it->second != val)
	//	std::cout<<"Changed from "<<it->second<<" to "<<val<<std::endl;
	auto min = static_cast<float>(blend->min);
	auto max = static_cast<float>(blend->max);
	if(val > max) {
		if(blend->loop == true)
			val = min + (val - max);
		else
			val = max;
	}
	else if(val < min) {
		if(blend->loop == true)
			val = max + val;
		else
			val = min;
	}
	it->second = val;
}
void BaseAnimatedComponent::SetBlendController(const std::string &controller, float val)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	if(!mdlComponent)
		return;
	int id = mdlComponent->LookupBlendController(controller);
	if(id == -1)
		return;
	SetBlendController(id, val);
}
const std::unordered_map<unsigned int, float> &BaseAnimatedComponent::GetBlendControllers() const { return m_blendControllers; }
float BaseAnimatedComponent::GetBlendController(const std::string &controller) const
{
	auto mdlComponent = GetEntity().GetModelComponent();
	if(!mdlComponent)
		return 0.f;
	int id = mdlComponent->LookupBlendController(controller);
	if(id == -1)
		return 0.f;
	return GetBlendController(id);
}
float BaseAnimatedComponent::GetBlendController(unsigned int controller) const
{
	auto it = m_blendControllers.find(controller);
	if(it == m_blendControllers.end())
		return 0;
	return it->second;
}
static Frame *get_frame_from_cycle(animation::Animation &anim, float cycle, uint32_t frameOffset = 0) { return anim.GetFrame(static_cast<uint32_t>((anim.GetFrameCount() - 1) * cycle) + frameOffset).get(); }
bool BaseAnimatedComponent::GetBlendFramesFromCycle(animation::Animation &anim, float cycle, Frame **outFrameSrc, Frame **outFrameDst, float &outInterpFactor, int32_t frameOffset)
{
	auto frameVal = (anim.GetFrameCount() - 1) * cycle;
	outInterpFactor = frameVal - static_cast<float>(math::floor(frameVal));
	*outFrameSrc = anim.GetFrame(math::max(static_cast<int32_t>(frameVal) + frameOffset, 0)).get();
	if(*outFrameSrc == nullptr)
		return false;
	auto *f = anim.GetFrame(math::max(static_cast<int32_t>(frameVal) + 1 + frameOffset, 0)).get();
	if(f == *outFrameSrc) // No need to blend if both frames are the same
	{
		outInterpFactor = 0.f;
		*outFrameDst = nullptr;
	}
	else
		*outFrameDst = f;
	return true;
}
void BaseAnimatedComponent::GetAnimationBlendController(animation::Animation *anim, float cycle, std::array<AnimationBlendInfo, 2> &bcFrames, float *blendScale) const
{
	// Obsolete; TODO: Remove this!
}
void BaseAnimatedComponent::ClearPreviousAnimation()
{
	for(auto &pair : m_animSlots) {
		auto &animInfo = pair.second;
		auto &lastAnim = animInfo.lastAnim;
		lastAnim.blendTimeScale = {0.f, 0.f};
		lastAnim.animation = -1;
	}
	auto &animInfo = m_baseAnim;
	auto &lastAnim = animInfo.lastAnim;
	lastAnim.blendTimeScale = {0.f, 0.f};
	lastAnim.animation = -1;
}
Frame *BaseAnimatedComponent::GetPreviousAnimationBlendFrame(AnimationSlotInfo &animInfo, double tDelta, float &blendScale)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return nullptr;
	Frame *frameLastAnim = nullptr;
	auto &lastAnim = animInfo.lastAnim;
	if(lastAnim.animation != -1) {
		lastAnim.blendTimeScale.second -= static_cast<float>(tDelta);
		if(lastAnim.blendTimeScale.second <= 0.f) {
			lastAnim.blendTimeScale = {0.f, 0.f};
			lastAnim.animation = -1;
		}
		else {
			auto anim = hModel->GetAnimation(lastAnim.animation);
			if(anim != nullptr) {
				auto frameLast = math::floor((anim->GetFrameCount() - 1) * lastAnim.cycle);
				frameLastAnim = anim->GetFrame(frameLast).get();
			}
		}
		blendScale = ((lastAnim.blendTimeScale.first != 0.f) ? (lastAnim.blendTimeScale.second / lastAnim.blendTimeScale.first) : 0.f) * lastAnim.blendScale;
	}
	return frameLastAnim;
}
void BaseAnimatedComponent::ApplyAnimationBlending(AnimationSlotInfo &animInfo, double tDelta)
{
	// TODO: This is obsolete, remove it!
}

bool BaseAnimatedComponent::MaintainAnimation(AnimationSlotInfo &animInfo, double dt, int32_t layeredSlot)
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return false;
	CEMaintainAnimation evData {animInfo, dt};
	if(InvokeEventCallbacks(baseAnimatedComponent::EVENT_MAINTAIN_ANIMATION_MT, evData) == util::EventReply::Handled)
		return false;
	if(animInfo.animation == -1)
		return false;
	auto animId = animInfo.animation;
	auto anim = hModel->GetAnimation(animId);
	if(anim == nullptr)
		return false;
	auto act = anim->GetActivity();
	auto numFrames = anim->GetFrameCount();
	auto animSpeed = GetPlaybackRate();
	if(numFrames > 0)
		animSpeed *= static_cast<float>(anim->GetFPS()) / static_cast<float>(numFrames);

	auto &cycle = animInfo.cycle;
	auto cycleLast = cycle;
	auto cycleNew = cycle + static_cast<float>(dt) * animSpeed;
	if(layeredSlot == -1) {
		if(math::abs(cycleNew - cycleLast) < 0.00001f && math::is_flag_set(m_stateFlags, StateFlags::BaseAnimationDirty) == false)
			return false;
		math::set_flag(m_stateFlags, StateFlags::BaseAnimationDirty, false);
	}
	auto bLoop = anim->HasFlag(FAnim::Loop) || math::is_flag_set(animInfo.flags, FPlayAnim::Loop);
	auto bComplete = (cycleNew >= 1.f) ? true : false;
	if(bComplete == true) {
		cycle = 1.f;
		if(&animInfo == &m_baseAnim) // Only if this is the main animation
		{
			ComponentEventQueueInfo evInfo {};
			evInfo.id = baseAnimatedComponent::EVENT_ON_ANIMATION_COMPLETE;
			evInfo.eventData = std::make_unique<CEOnAnimationComplete>(animId, act);
			m_queuedEvents.push(std::move(evInfo));
		}
		else {
			ComponentEventQueueInfo evInfo {};
			evInfo.id = baseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_COMPLETE;
			evInfo.eventData = std::make_unique<CELayeredAnimationInfo>(layeredSlot, animId, act);
			m_queuedEvents.push(std::move(evInfo));
		}
		if(cycleLast > 0.f) // If current cycle is 0 but we're also complete, that means the animation was started and finished within a single frame. Calling the block below may result in endless recursion, so we need to make sure the animation stays for this frame.
		{
			if(cycle != 1.f || animId != animInfo.animation) {
				SetBaseAnimationDirty();
				return MaintainAnimation(animInfo, dt);
			}
			if(bLoop == true) {
				cycleNew -= floor(cycleNew);
				if(anim->HasFlag(FAnim::NoRepeat)) {
					animId = SelectWeightedAnimation(act, animId);
					cycle = cycleNew;
					SetBaseAnimationDirty();
					return MaintainAnimation(animInfo, dt);
				}
			}
			else
				cycleNew = 1.f;
		}
		else
			cycleNew = 1.f;
		cycle = cycleNew;
	}
	else
		cycle = cycleNew;

#if DEBUG_VERBOSE_ANIMATION == 1
	if(&animInfo == &m_baseAnim) {
		Con::COUT << GetEntity().GetClass() << " is playing base animation '" << hModel->GetAnimationName(animId) << "'"
		          << ": Cycle " << cycle << " => " << cycleNew << "; Looping: " << (bLoop ? "true" : "false") << "; Frame Count: " << numFrames << "; Speed: " << animSpeed << Con::endl;
	}
#endif

	// TODO: All of this is very inefficient and involves a lot of unnecessary buffers. FIXME

	// Initialize buffer for blended/interpolated animation data
	auto &animBoneList = anim->GetBoneList();
	auto numBones = animBoneList.size();
	std::vector<math::Transform> bonePoses {};
	std::vector<Vector3> boneScales {};

	// Blend between the last frame and the current frame of this animation.
	Frame *srcFrame, *dstFrame;
	float interpFactor;
	if(GetBlendFramesFromCycle(*anim, cycle, &srcFrame, &dstFrame, interpFactor) == false)
		return false; // This shouldn't happen unless the animation has no frames

	//if(dstFrame)
	{
		bonePoses.resize(numBones);
		boneScales.resize(numBones, Vector3 {1.f, 1.f, 1.f});
	}

	// Blend Controllers
	auto *animBcData = anim->GetBlendController();
	if(animBcData) {
		auto *bc = hModel->GetBlendController(animBcData->controller);
		if(animBcData->transitions.empty() == false && bc != nullptr) {
			auto bcValue = GetBlendController(animBcData->controller);
			auto *trSrc = &animBcData->transitions.front();
			auto *trDst = &animBcData->transitions.back();
			for(auto &tr : animBcData->transitions) {
				if(tr.transition <= bcValue && tr.transition > trSrc->transition)
					trSrc = &tr;
				if(tr.transition >= bcValue && tr.transition < trDst->transition)
					trDst = &tr;
			}
			auto offset = (trDst->transition - trSrc->transition);
			auto interpFactor = 0.f;
			if(offset > 0.f)
				interpFactor = (bcValue - trSrc->transition) / offset;

			auto blendAnimSrc = hModel->GetAnimation(trSrc->animation);
			auto blendAnimDst = hModel->GetAnimation(trDst->animation);
			if(blendAnimSrc != nullptr && blendAnimDst != nullptr) {
				// Note: A blend controller blends between two different animations. That means that for each animation
				// we have to interpolate the animation's frame, and then interpolate (i.e. blend) the resulting bone poses
				// of both animations.

				// Interpolated poses of source animation
				Frame *srcFrame, *dstFrame;
				float animInterpFactor;
				std::vector<math::Transform> ppBonePosesSrc {};
				std::vector<Vector3> ppBoneScalesSrc {};
				if(GetBlendFramesFromCycle(*blendAnimSrc, cycle, &srcFrame, &dstFrame, animInterpFactor)) {
					if(dstFrame) {
						ppBonePosesSrc.resize(numBones);
						ppBoneScalesSrc.resize(numBones, Vector3 {1.f, 1.f, 1.f});
						BlendBonePoses(srcFrame->GetBoneTransforms(), &srcFrame->GetBoneScales(), dstFrame->GetBoneTransforms(), &dstFrame->GetBoneScales(), ppBonePosesSrc, &ppBoneScalesSrc, *blendAnimSrc, animInterpFactor);
					}
					else {
						ppBonePosesSrc = srcFrame->GetBoneTransforms();
						ppBoneScalesSrc = srcFrame->GetBoneScales();
					}
				}

				// Interpolated poses of destination animation
				std::vector<math::Transform> ppBonePosesDst {};
				std::vector<Vector3> ppBoneScalesDst {};
				if(GetBlendFramesFromCycle(*blendAnimDst, cycle, &srcFrame, &dstFrame, animInterpFactor)) {
					if(dstFrame) {
						ppBonePosesDst.resize(numBones);
						ppBoneScalesDst.resize(numBones, Vector3 {1.f, 1.f, 1.f});
						BlendBonePoses(srcFrame->GetBoneTransforms(), &srcFrame->GetBoneScales(), dstFrame->GetBoneTransforms(), &dstFrame->GetBoneScales(), ppBonePosesDst, &ppBoneScalesDst, *blendAnimSrc, animInterpFactor);
					}
					else {
						ppBonePosesDst = srcFrame->GetBoneTransforms();
						ppBoneScalesDst = srcFrame->GetBoneScales();
					}
				}

				// Interpolate between the two frames
				BlendBonePoses(ppBonePosesSrc, &ppBoneScalesSrc, ppBonePosesDst, &ppBoneScalesDst, bonePoses, &boneScales, *blendAnimSrc, interpFactor);

				if(animBcData->animationPostBlendController != std::numeric_limits<uint32_t>::max() && animBcData->animationPostBlendTarget != std::numeric_limits<uint32_t>::max()) {
					auto blendAnimPost = hModel->GetAnimation(animBcData->animationPostBlendTarget);
					if(blendAnimPost && GetBlendFramesFromCycle(*blendAnimPost, cycle, &srcFrame, &dstFrame, animInterpFactor)) {
						if(dstFrame) {
							ppBonePosesSrc.resize(numBones);
							ppBoneScalesSrc.resize(numBones, Vector3 {1.f, 1.f, 1.f});
							BlendBonePoses(srcFrame->GetBoneTransforms(), &srcFrame->GetBoneScales(), dstFrame->GetBoneTransforms(), &dstFrame->GetBoneScales(), ppBonePosesSrc, &ppBoneScalesSrc, *blendAnimPost, animInterpFactor);
						}
						else {
							ppBonePosesSrc = srcFrame->GetBoneTransforms();
							ppBoneScalesSrc = srcFrame->GetBoneScales();
						}

						// Interpolate between the two frames
						auto bcValuePostBlend = GetBlendController(animBcData->animationPostBlendController);
						auto interpFactor = 1.f - bcValuePostBlend;
						BlendBonePoses(ppBonePosesSrc, &ppBoneScalesSrc, bonePoses, &boneScales, bonePoses, &boneScales, *blendAnimPost, interpFactor);
					}
				}
			}
		}
	}
	else {
		if(dstFrame) {
			BlendBonePoses(srcFrame->GetBoneTransforms(), &srcFrame->GetBoneScales(), dstFrame->GetBoneTransforms(), &dstFrame->GetBoneScales(), bonePoses, &boneScales, *anim, interpFactor);
		}
		else {
			// Destination frame can be nullptr if no interpolation is required.
			bonePoses = srcFrame->GetBoneTransforms();
			boneScales = srcFrame->GetBoneScales();
		}
		//

		// Blend between previous animation and this animation
		float interpFactorLastAnim;
		auto *lastPlayedFrameOfPreviousAnim = GetPreviousAnimationBlendFrame(animInfo, dt, interpFactorLastAnim);
		if(lastPlayedFrameOfPreviousAnim) {
			auto lastAnim = hModel->GetAnimation(animInfo.lastAnim.animation);
			if(lastAnim) {
				BlendBonePoses(lastPlayedFrameOfPreviousAnim->GetBoneTransforms(), &lastPlayedFrameOfPreviousAnim->GetBoneScales(), bonePoses, &boneScales, bonePoses, &boneScales, *lastAnim, 1.f - interpFactorLastAnim);
			}
		}
		//
	}
	//

	animInfo.bonePoses = std::move(bonePoses);
	animInfo.boneScales = std::move(boneScales);

	CEOnBlendAnimation evDataBlend {animInfo, act, animInfo.bonePoses, (animInfo.boneScales.empty() == false) ? &animInfo.boneScales : nullptr};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_BLEND_ANIMATION_MT, evDataBlend);

	// Animation events
	auto frameLast = (cycleLast != 0.f) ? static_cast<int32_t>((numFrames - 1) * cycleLast) : -1;
	auto frameCycle = (numFrames > 0) ? ((numFrames - 1) * cycle) : 0.f;
	auto frameID = math::floor(frameCycle);

	if(frameID < frameLast)
		frameID = numFrames;

	m_animEventQueue.push({});
	auto &eventItem = m_animEventQueue.back();
	eventItem.animId = animId;
	eventItem.animation = anim;
	eventItem.frameId = frameID;
	eventItem.lastFrame = frameLast;
	return true;
}

void BaseAnimatedComponent::SetBindPose(const Frame &frame) { m_bindPose = frame.shared_from_this(); }
const Frame *BaseAnimatedComponent::GetBindPose() const { return m_bindPose.get(); }

bool BaseAnimatedComponent::MaintainGestures(double dt)
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return false;
	auto &baseAnimInfo = m_baseAnim;
	auto anim = hModel->GetAnimation(baseAnimInfo.animation);
	auto &bones = anim->GetBoneList();
	auto &bonePoses = baseAnimInfo.bonePoses;
	auto &boneScales = baseAnimInfo.boneScales;

	// Update gestures
	for(auto it = m_animSlots.begin(); it != m_animSlots.end();) {
		auto &animInfo = it->second;
		if(MaintainAnimation(animInfo, dt, it->first) == true) {
			auto anim = hModel->GetAnimation(animInfo.animation);
			auto baseAnim = hModel->GetAnimation(baseAnimInfo.animation);
			TransformBoneFrames(bonePoses, !boneScales.empty() ? &boneScales : nullptr, baseAnim, anim, animInfo.bonePoses, !animInfo.boneScales.empty() ? &animInfo.boneScales : nullptr, anim->HasFlag(FAnim::Gesture));
			if(animInfo.cycle >= 1.f) {
				if(anim->HasFlag(FAnim::Loop) == false) {
					it = m_animSlots.erase(it); // No need to keep the gesture information around anymore
					continue;
				}
			}
		}
		++it;
	}
	return true;
}
bool BaseAnimatedComponent::PreMaintainAnimations(double dt)
{
	CEMaintainAnimations evData {dt};
	if(InvokeEventCallbacks(baseAnimatedComponent::EVENT_MAINTAIN_ANIMATIONS, evData) == util::EventReply::Handled) {
		InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_ANIMATIONS_UPDATED_MT);
		InvokeEventCallbacks(baseAnimatedComponent::EVENT_UPDATE_BONE_POSES_MT);
		InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_BONE_POSES_FINALIZED_MT);
		return false;
	}
	return true;
}
bool BaseAnimatedComponent::MaintainAnimations(double dt)
{
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return false;

	auto r = MaintainAnimation(m_baseAnim, dt);
	if(r == true)
		MaintainGestures(dt);

	auto &baseAnimInfo = m_baseAnim;
	auto anim = hModel->GetAnimation(baseAnimInfo.animation);
	if(!anim)
		return false;
	auto &bones = anim->GetBoneList();
	auto &bonePoses = baseAnimInfo.bonePoses;
	auto &boneScales = baseAnimInfo.boneScales;
	// Apply animation to skeleton
	auto n = math::min(bones.size(), bonePoses.size());
	for(auto i = decltype(n) {0}; i < n; ++i) {
		auto boneId = bones[i];
		auto &pose = bonePoses.at(i);
		SetBonePose(boneId, pose);
		if(boneScales.empty() == false)
			SetBoneScale(boneId, boneScales.at(i));
	}

	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_ANIMATIONS_UPDATED_MT);
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_UPDATE_BONE_POSES_MT);
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_BONE_POSES_FINALIZED_MT);
	return r;
}
void BaseAnimatedComponent::HandleAnimationEvents()
{
	while(!m_queuedEvents.empty()) {
		auto &evData = m_queuedEvents.front();
		InvokeEventCallbacks(evData.id, *evData.eventData);
		m_queuedEvents.pop();
	}
	const auto fHandleAnimationEvents = [this](uint32_t animId, const std::shared_ptr<animation::Animation> &anim, int32_t frameId) {
		auto *events = anim->GetEvents(frameId);
		if(events) {
			for(auto &ev : *events)
				HandleAnimationEvent(*ev);
		}
		auto *customEvents = GetAnimationEvents(animId, frameId);
		if(customEvents != nullptr) {
			for(auto &ev : *customEvents) {
				if(ev.callback.first == true) // Is it a callback event?
				{
					if(ev.callback.second.IsValid()) {
						auto *f = ev.callback.second.get();
						if(typeid(*f) == typeid(LuaCallback)) {
							auto *lf = static_cast<LuaCallback *>(f);
							lf->Call<void>();
						}
						else
							(*f)();
					}
				}
				else
					HandleAnimationEvent(ev);
			}
		}
	};
	while(!m_animEventQueue.empty()) {
		auto &eventItem = m_animEventQueue.front();

		for(auto i = eventItem.lastFrame + 1; i <= eventItem.frameId; ++i)
			fHandleAnimationEvents(eventItem.animId, eventItem.animation, i);

		if(static_cast<int32_t>(eventItem.frameId) < eventItem.lastFrame) {
			for(auto i = decltype(eventItem.frameId) {0}; i <= eventItem.frameId; ++i)
				fHandleAnimationEvents(eventItem.animId, eventItem.animation, i);
		}

		m_animEventQueue.pop();
	}

	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_ANIMATIONS_UPDATED);
}
Activity BaseAnimatedComponent::TranslateActivity(Activity act)
{
	CETranslateActivity evTranslateActivityData {act};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_TRANSLATE_ACTIVITY, evTranslateActivityData);
	return act;
}

float BaseAnimatedComponent::GetCycle() const { return m_baseAnim.cycle; }
void BaseAnimatedComponent::SetCycle(float cycle)
{
	if(cycle == m_baseAnim.cycle)
		return;
	m_baseAnim.cycle = cycle;
	SetBaseAnimationDirty();
}

void BaseAnimatedComponent::ResetPose()
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return;
	auto &skeleton = mdl->GetSkeleton();
	auto &ref = mdl->GetReference();
	auto numBones = skeleton.GetBoneCount();
	std::function<void(const animation::Bone &, const math::ScaledTransform &)> resetBonePose = nullptr;
	resetBonePose = [this, &resetBonePose, &ref](const animation::Bone &bone, const math::ScaledTransform &parentPose) {
		math::ScaledTransform pose;
		ref.GetBonePose(bone.ID, pose);
		for(auto &[childId, child] : bone.children)
			resetBonePose(*child, pose);
		pose = parentPose.GetInverse() * pose;
		SetBonePose(bone.ID, pose);
	};
	for(auto &[boneId, bone] : skeleton.GetRootBones())
		resetBonePose(*bone, {});
	BroadcastEvent(baseAnimatedComponent::EVENT_ON_RESET_POSE);
}

int BaseAnimatedComponent::GetAnimation() const { return m_baseAnim.animation; }
animation::Animation *BaseAnimatedComponent::GetAnimationObject() const
{
	auto animId = GetAnimation();
	if(animId == -1)
		return nullptr;
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return nullptr;
	auto anim = hModel->GetAnimation(animId);
	if(anim == nullptr)
		return nullptr;
	return anim.get();
}
int32_t BaseAnimatedComponent::GetLayeredAnimation(uint32_t slot) const
{
	auto it = m_animSlots.find(slot);
	if(it == m_animSlots.end())
		return -1;
	return it->second.animation;
}
Activity BaseAnimatedComponent::GetLayeredActivity(uint32_t slot) const
{
	auto it = m_animSlots.find(slot);
	if(it == m_animSlots.end())
		return Activity::Invalid;
	return it->second.activity;
}

void BaseAnimatedComponent::PlayAnimation(int animation, FPlayAnim flags)
{
	auto bSkipAnim = false;
	CEOnPlayAnimation evData {m_baseAnim.animation, animation, flags};
	if(InvokeEventCallbacks(baseAnimatedComponent::EVENT_PLAY_ANIMATION, evData) == util::EventReply::Handled)
		return;
	if(m_baseAnim.animation == animation && (flags & FPlayAnim::Reset) == FPlayAnim::None) {
		auto &hModel = GetEntity().GetModel();
		if(hModel != nullptr) {
			auto anim = hModel->GetAnimation(animation);
			if(anim != nullptr && anim->HasFlag(FAnim::Loop))
				return;
		}
	}
	if(animation < -1)
		animation = -1;

	CETranslateAnimation evTranslateAnimData {animation, flags};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_TRANSLATE_ANIMATION, evTranslateAnimData);

	if(animation == m_baseAnim.animation && m_baseAnim.cycle == 0.f && m_baseAnim.flags == flags)
		return; // No change
	auto &lastAnim = m_baseAnim.lastAnim;
	if(m_baseAnim.animation != -1 && m_baseAnim.animation != animation && m_baseAnim.cycle > 0.f) {
		lastAnim.animation = m_baseAnim.animation;
		lastAnim.cycle = m_baseAnim.cycle;
		lastAnim.flags = m_baseAnim.flags;
		lastAnim.blendTimeScale = {0.f, 0.f};
		lastAnim.blendScale = 1.f;

		// Update animation fade time
		auto &hModel = GetEntity().GetModel();
		if(hModel != nullptr) {
			auto anim = hModel->GetAnimation(animation);
			auto animLast = hModel->GetAnimation(m_baseAnim.animation);
			if(anim != nullptr && animLast != nullptr) {
				auto bAnimFadeIn = anim->HasFadeInTime();
				auto bAnimLastFadeOut = animLast->HasFadeOutTime();
				auto animFadeIn = anim->GetFadeInTime();
				auto animFadeOut = anim->GetFadeOutTime();
				// UNUSED(animFadeOut);
				auto animLastFadeOut = animLast->GetFadeOutTime();
				const auto defaultFadeOutTime = 0.2f;
				if(bAnimFadeIn == true) {
					if(bAnimLastFadeOut == true)
						lastAnim.blendTimeScale.first = (animFadeIn > animLastFadeOut) ? animFadeIn : animLastFadeOut;
					else
						lastAnim.blendTimeScale.first = animFadeIn;
				}
				else if(bAnimLastFadeOut == true)
					lastAnim.blendTimeScale.first = animLastFadeOut;
				else
					lastAnim.blendTimeScale.first = defaultFadeOutTime;
				lastAnim.blendTimeScale.second = lastAnim.blendTimeScale.first;
			}
		}
		//
	}
	else
		lastAnim.animation = -1;
	m_baseAnim.animation = animation;
	m_baseAnim.cycle = 0;
	m_baseAnim.flags = flags;
	m_baseAnim.activity = Activity::Invalid;
	SetBaseAnimationDirty();
	auto &hModel = GetEntity().GetModel();
	if(hModel != nullptr) {
		auto anim = hModel->GetAnimation(animation);
		if(anim != nullptr) {
			m_baseAnim.activity = anim->GetActivity();

			// We'll set all bones that are unused by the animation to
			// their respective reference pose
			auto &ref = hModel->GetReference();
			auto &boneMap = anim->GetBoneMap();
			auto &skeleton = hModel->GetSkeleton();
			auto numBones = skeleton.GetBoneCount();
			for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
				auto bone = skeleton.GetBone(i);
				auto it = boneMap.find(i);
				if(it != boneMap.end() || bone.expired())
					continue;
				auto parent = bone.lock()->parent;
				math::ScaledTransform poseParent {};
				if(!parent.expired()) {
					ref.GetBonePose(parent.lock()->ID, poseParent);
					poseParent = poseParent.GetInverse();
				}

				math::ScaledTransform pose {};
				auto *pos = ref.GetBonePosition(i);
				if(pos)
					pose.SetOrigin(*pos);
				auto *rot = ref.GetBoneOrientation(i);
				if(rot)
					pose.SetRotation(*rot);
				auto *scale = ref.GetBoneScale(i);
				if(scale)
					pose.SetScale(*scale);

				pose = poseParent * pose;
				SetBonePose(i, pos ? &pose.GetOrigin() : nullptr, rot ? &pose.GetRotation() : nullptr, scale ? &pose.GetScale() : nullptr);
			}
		}
	}

	CEOnAnimationStart evAnimStartData {m_baseAnim.animation, m_baseAnim.activity, m_baseAnim.flags};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_ANIMATION_START, evAnimStartData);
}

void BaseAnimatedComponent::SetBaseAnimationDirty() { math::set_flag(m_stateFlags, StateFlags::BaseAnimationDirty, true); }
void BaseAnimatedComponent::SetAbsolutePosesDirty() { math::set_flag(m_stateFlags, StateFlags::AbsolutePosesDirty, true); }

int32_t BaseAnimatedComponent::SelectTranslatedAnimation(Activity &inOutActivity) const
{
	inOutActivity = const_cast<BaseAnimatedComponent *>(this)->TranslateActivity(inOutActivity);
	return SelectWeightedAnimation(inOutActivity, m_baseAnim.animation);
}

bool BaseAnimatedComponent::PlayActivity(Activity activity, FPlayAnim flags)
{
	if(GetActivity() == activity && (flags & FPlayAnim::Reset) == FPlayAnim::None)
		return true;
	auto seq = SelectTranslatedAnimation(activity);

	CEOnPlayActivity evDataActivity {activity, flags};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_PLAY_ACTIVITY, evDataActivity);

	PlayAnimation(seq, flags);
	//m_baseAnim.activity = activity;
	return (seq == -1) ? false : true;
}

Activity BaseAnimatedComponent::GetActivity() const
{
	if(m_baseAnim.animation == -1)
		return Activity::Invalid;
	if(m_baseAnim.activity != Activity::Invalid)
		return m_baseAnim.activity;
	auto &hModel = GetEntity().GetModel();
	if(hModel == nullptr)
		return Activity::Invalid;
	auto anim = hModel->GetAnimation(m_baseAnim.animation);
	if(anim == nullptr)
		return Activity::Invalid;
	return anim->GetActivity();
}

void BaseAnimatedComponent::SetPlaybackRate(float rate) { *m_playbackRate = rate; }
float BaseAnimatedComponent::GetPlaybackRate() const { return *m_playbackRate; }
const util::PFloatProperty &BaseAnimatedComponent::GetPlaybackRateProperty() const { return m_playbackRate; }

void BaseAnimatedComponent::HandleAnimationEvent(const AnimationEvent &ev)
{
	auto bHandled = false;
	CEHandleAnimationEvent evData {ev};
	if(InvokeEventCallbacks(baseAnimatedComponent::EVENT_HANDLE_ANIMATION_EVENT, evData) == util::EventReply::Handled)
		return;
	auto it = m_boundAnimEvents.find(ev.eventID);
	if(it != m_boundAnimEvents.end()) {
		it->second.Call<void, std::reference_wrapper<const AnimationEvent>>(ev);
		return;
	}
	switch(ev.eventID) {
	case AnimationEvent::Type::EmitSound:
		{
			if(ev.arguments.size() > 0) {
				auto pSoundEmitterComponent = static_cast<BaseSoundEmitterComponent *>(GetEntity().FindComponent("sound_emitter").get());
				if(pSoundEmitterComponent != nullptr) {
					BaseSoundEmitterComponent::SoundInfo sndInfo {};
					sndInfo.transmit = false;
					pSoundEmitterComponent->EmitSound(ev.arguments.front(), audio::ALSoundType::Generic, sndInfo);
				}
			}
			break;
		}
	default:; //Con::COUT<<"WARNING: Unhandled animation event "<<ev->eventID<<Con::endl;
	}
}

const BaseAnimatedComponent::AnimationSlotInfo &BaseAnimatedComponent::GetBaseAnimationInfo() const { return const_cast<BaseAnimatedComponent &>(*this).GetBaseAnimationInfo(); }
BaseAnimatedComponent::AnimationSlotInfo &BaseAnimatedComponent::GetBaseAnimationInfo() { return m_baseAnim; }
const std::unordered_map<uint32_t, BaseAnimatedComponent::AnimationSlotInfo> &BaseAnimatedComponent::GetAnimationSlotInfos() const { return const_cast<BaseAnimatedComponent &>(*this).GetAnimationSlotInfos(); }
std::unordered_map<uint32_t, BaseAnimatedComponent::AnimationSlotInfo> &BaseAnimatedComponent::GetAnimationSlotInfos() { return m_animSlots; }

bool BaseAnimatedComponent::PlayAnimation(const std::string &name, FPlayAnim flags)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	if(!mdlComponent)
		return false;
	auto prevAnim = GetAnimation();
	int anim = mdlComponent->LookupAnimation(name);

	CEOnPlayAnimation evData {prevAnim, anim, flags};
	if(InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_PLAY_ANIMATION, evData) == util::EventReply::Handled)
		return false;

	PlayAnimation(anim, flags);
	return true;
}

void BaseAnimatedComponent::PlayLayeredAnimation(int slot, int animation, FPlayAnim flags, AnimationSlotInfo **animInfo)
{
	auto prevAnim = GetLayeredAnimation(slot);

	CETranslateLayeredAnimation evData {slot, animation, flags};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_TRANSLATE_LAYERED_ANIMATION, evData);

	CEOnPlayLayeredAnimation evDataPlay {slot, prevAnim, animation, flags};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ANIMATION, evDataPlay);

	auto &slotInfo = m_animSlots[slot] = {animation};
	slotInfo.flags = flags;
	if(animInfo != nullptr)
		*animInfo = &slotInfo;

	CELayeredAnimationInfo evDataStart {slot, slotInfo.animation, slotInfo.activity};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_LAYERED_ANIMATION_START, evDataStart);
}
void BaseAnimatedComponent::PlayLayeredAnimation(int slot, int animation, FPlayAnim flags) { PlayLayeredAnimation(slot, animation, flags, nullptr); }
bool BaseAnimatedComponent::PlayLayeredAnimation(int slot, std::string animation, FPlayAnim flags)
{
	auto mdlComponent = GetEntity().GetModelComponent();
	if(!mdlComponent)
		return false;
	auto anim = mdlComponent->LookupAnimation(animation);
	if(anim == -1)
		return false;
	PlayLayeredAnimation(slot, anim, flags);
	return true;
}
bool BaseAnimatedComponent::PlayLayeredActivity(int slot, Activity activity, FPlayAnim flags)
{
	CEOnPlayLayeredActivity evData {slot, activity, flags};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_PLAY_LAYERED_ACTIVITY, evData);

	int32_t animAvoid = -1;
	auto it = m_animSlots.find(slot);
	if(it != m_animSlots.end())
		animAvoid = it->second.animation;
	auto seq = SelectWeightedAnimation(activity, animAvoid);
	AnimationSlotInfo *animInfo = nullptr;
	PlayLayeredAnimation(slot, seq, flags, &animInfo);
	if(animInfo != nullptr)
		animInfo->activity = activity;
	return (seq == -1) ? false : true;
}
void BaseAnimatedComponent::StopLayeredAnimations()
{
	std::vector<uint32_t> curSlots;
	curSlots.reserve(m_animSlots.size());
	for(auto &[slotId, slotInfo] : m_animSlots)
		curSlots.push_back(slotId);
	for(auto slotId : curSlots)
		StopLayeredAnimation(slotId);
}
void BaseAnimatedComponent::StopLayeredAnimation(int slot)
{
	auto it = m_animSlots.find(slot);
	if(it == m_animSlots.end())
		return;
	CEOnStopLayeredAnimation evData {slot, it->second};
	InvokeEventCallbacks(baseAnimatedComponent::EVENT_ON_STOP_LAYERED_ANIMATION, evData);
	m_animSlots.erase(it);
}

const std::vector<math::ScaledTransform> &BaseAnimatedComponent::GetProcessedBones() const { return const_cast<BaseAnimatedComponent *>(this)->GetProcessedBones(); }
std::vector<math::ScaledTransform> &BaseAnimatedComponent::GetProcessedBones() { return m_processedBones; }

bool BaseAnimatedComponent::CalcAnimationMovementSpeed(float *x, float *z, int32_t frameOffset) const
{
	auto &ent = GetEntity();
	auto &hMdl = ent.GetModel();
	auto animId = GetAnimation();
	if(hMdl == nullptr || animId == -1)
		return false;
	auto anim = hMdl->GetAnimation(animId);
	if(anim == nullptr || (((x != nullptr && anim->HasFlag(FAnim::MoveX) == false) || x == nullptr) && ((z != nullptr && anim->HasFlag(FAnim::MoveZ) == false) || z == nullptr)))
		return false;

	std::array<Frame *, 2> frames = {nullptr, nullptr};
	auto blendScale = 0.f;
	if(GetBlendFramesFromCycle(*anim, GetCycle(), &frames[0], &frames[1], blendScale, frameOffset) == false)
		return false; // Animation doesn't have any frames?
	auto animSpeed = GetPlaybackRate();
	std::array<float, 2> blendScales = {1.f - blendScale, blendScale};
	Vector2 mvOffset {0.f, 0.f};
	for(auto i = decltype(frames.size()) {0}; i < frames.size(); ++i) {
		auto *frame = frames[i];
		if(frame == nullptr)
			continue;
		auto *moveOffset = frame->GetMoveOffset();
		if(moveOffset == nullptr)
			continue;
		mvOffset += *moveOffset * blendScales[i] * animSpeed;
	}
	if(x != nullptr)
		*x = mvOffset.x;
	if(z != nullptr)
		*z = mvOffset.y;
	return true;
}

static void write_anim_flags(udm::LinkedPropertyWrapperArg udm, FPlayAnim flags)
{
	udm::write_flag(udm["flags"], flags, FPlayAnim::Reset, "reset");
	udm::write_flag(udm["flags"], flags, FPlayAnim::Transmit, "transmit");
	udm::write_flag(udm["flags"], flags, FPlayAnim::SnapTo, "snapTo");
	udm::write_flag(udm["flags"], flags, FPlayAnim::Loop, "loop");
	static_assert(magic_enum::enum_count<FPlayAnim>() == 4);
}

static FPlayAnim read_anim_flags(udm::LinkedPropertyWrapperArg &udm)
{
	auto flags = FPlayAnim::None;
	udm::read_flag(udm["flags"], flags, FPlayAnim::Reset, "reset");
	udm::read_flag(udm["flags"], flags, FPlayAnim::Transmit, "transmit");
	udm::read_flag(udm["flags"], flags, FPlayAnim::SnapTo, "snapTo");
	udm::read_flag(udm["flags"], flags, FPlayAnim::Loop, "loop");
	static_assert(magic_enum::enum_count<FPlayAnim>() == 4);
	return flags;
}

static void write_animation_slot_info(udm::LinkedPropertyWrapperArg udm, const BaseAnimatedComponent::AnimationSlotInfo &slotInfo)
{
	udm["activity"] = slotInfo.activity;
	udm["animation"] = slotInfo.animation;
	udm["cycle"] = slotInfo.cycle;
	write_anim_flags(udm["flags"], slotInfo.flags);

	udm["bonePoses"] = udm::compress_lz4_blob(slotInfo.bonePoses);
	udm["boneScales"] = udm::compress_lz4_blob(slotInfo.boneScales);
	udm["bonePosesBc"] = udm::compress_lz4_blob(slotInfo.bonePosesBc);
	udm["boneScalesBc"] = udm::compress_lz4_blob(slotInfo.boneScalesBc);

	udm["lastAnimation"]["animation"] = slotInfo.lastAnim.animation;
	udm["lastAnimation"]["cycle"] = slotInfo.lastAnim.cycle;
	write_anim_flags(udm["lastAnimation"]["flags"], slotInfo.lastAnim.flags);
	udm["lastAnimation"]["blendFadeIn"] = slotInfo.lastAnim.blendTimeScale.first;
	udm["lastAnimation"]["blendFadeOut"] = slotInfo.lastAnim.blendTimeScale.second;
	udm["lastAnimation"]["blendScale"] = slotInfo.lastAnim.blendScale;
}
void BaseAnimatedComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["playbackRate"] = GetPlaybackRate();

	// Write blend controllers
	auto &blendControllers = GetBlendControllers();
	auto udmBlendControllers = udm.AddArray("blendControllers", blendControllers.size());
	uint32_t idx = 0;
	for(auto &pair : blendControllers) {
		auto udmBlendController = udmBlendControllers[idx++];
		udmBlendController["slot"] = pair.first;
		udmBlendController["value"] = pair.second;
	}

	// Write animations
	write_animation_slot_info(udm, GetBaseAnimationInfo());
	auto &animSlotInfos = GetAnimationSlotInfos();
	auto udmAnimations = udm.AddArray("animations", animSlotInfos.size());
	idx = 0;
	for(auto &pair : animSlotInfos) {
		auto udmAnimation = udmAnimations[idx++];
		udmAnimation["slot"] = pair.first;
		write_animation_slot_info(udmAnimation, pair.second);
	}

	udm["animDisplacement"] = m_animDisplacement;
}
static void read_animation_slot_info(udm::LinkedPropertyWrapperArg udm, BaseAnimatedComponent::AnimationSlotInfo &slotInfo)
{
	udm["activity"](slotInfo.activity);
	udm["animation"](slotInfo.animation);
	udm["cycle"](slotInfo.cycle);
	slotInfo.flags = read_anim_flags(udm["flags"]);

	udm["bonePoses"].GetBlobData(slotInfo.bonePoses);
	udm["boneScales"].GetBlobData(slotInfo.boneScales);
	udm["bonePosesBc"].GetBlobData(slotInfo.bonePosesBc);
	udm["boneScalesBc"].GetBlobData(slotInfo.boneScalesBc);

	udm["lastAnimation"]["animation"](slotInfo.lastAnim.animation);
	udm["lastAnimation"]["cycle"](slotInfo.lastAnim.cycle);
	udm["lastAnimation"]["blendFadeIn"](slotInfo.lastAnim.blendTimeScale.first);
	udm["lastAnimation"]["blendFadeOut"](slotInfo.lastAnim.blendTimeScale.second);
	slotInfo.lastAnim.flags = read_anim_flags(udm["lastAnimation"]["flags"]);
	udm["lastAnimation"]["blendScale"](slotInfo.lastAnim.blendScale);
}
void BaseAnimatedComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	auto playbackRate = GetPlaybackRate();
	;
	udm["playbackRate"](playbackRate);
	SetPlaybackRate(playbackRate);

	// Read blend controllers
	auto udmBlendControllers = udm["blendControllers"];
	auto numBlendControllers = udmBlendControllers.GetSize();
	m_blendControllers.reserve(numBlendControllers);
	for(auto i = decltype(numBlendControllers) {0u}; i < numBlendControllers; ++i) {
		auto udmBlendController = udmBlendControllers[i];
		uint32_t slot = 0;
		udmBlendController["slot"](slot);
		auto value = 0.f;
		udmBlendController["value"](value);
		m_blendControllers[slot] = value;
	}

	// Read animations
	read_animation_slot_info(udm, GetBaseAnimationInfo());
	auto &animSlots = GetAnimationSlotInfos();
	auto udmAnimations = udm["animations"];
	auto numAnims = udmAnimations.GetSize();
	animSlots.reserve(numAnims);
	for(auto i = decltype(numAnims) {0u}; i < numAnims; ++i) {
		auto udmAnimation = udmAnimations[i];
		uint32_t slot = 0;
		udmAnimation["slot"](slot);
		auto it = animSlots.insert(std::make_pair(slot, AnimationSlotInfo {})).first;
		read_animation_slot_info(udmAnimation, it->second);
	}

	udm["animDisplacement"](m_animDisplacement);
}

/////////////////

CEHandleAnimationEvent::CEHandleAnimationEvent(const AnimationEvent &animationEvent) : animationEvent(animationEvent) {}
void CEHandleAnimationEvent::PushArguments(lua::State *l)
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
void CEHandleAnimationEvent::PushArgumentVariadic(lua::State *l)
{
	auto &args = animationEvent.arguments;
	for(auto &arg : args)
		Lua::PushString(l, arg);
}

/////////////////

CEOnPlayAnimation::CEOnPlayAnimation(int32_t previousAnimation, int32_t animation, FPlayAnim flags) : previousAnimation(previousAnimation), animation(animation), flags(flags) {}
void CEOnPlayAnimation::PushArguments(lua::State *l)
{
	Lua::PushInt(l, previousAnimation);
	Lua::PushInt(l, animation);
	Lua::PushInt(l, math::to_integral(flags));
}

/////////////////

CEOnPlayLayeredAnimation::CEOnPlayLayeredAnimation(int32_t slot, int32_t previousAnimation, int32_t animation, FPlayAnim flags) : CEOnPlayAnimation(previousAnimation, animation, flags), slot(slot) {}
void CEOnPlayLayeredAnimation::PushArguments(lua::State *l)
{
	CEOnPlayAnimation::PushArguments(l);
	Lua::PushInt(l, slot);
}

/////////////////

CETranslateLayeredActivity::CETranslateLayeredActivity(int32_t &slot, Activity &activity, FPlayAnim &flags) : slot(slot), activity(activity), flags(flags) {}
void CETranslateLayeredActivity::PushArguments(lua::State *l)
{
	Lua::PushInt(l, slot);
	Lua::PushInt(l, math::to_integral(activity));
	Lua::PushInt(l, math::to_integral(flags));
}
uint32_t CETranslateLayeredActivity::GetReturnCount() { return 3; }
void CETranslateLayeredActivity::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -3))
		slot = Lua::CheckInt(l, -3);
	if(Lua::IsSet(l, -2))
		activity = static_cast<Activity>(Lua::CheckInt(l, -2));
	if(Lua::IsSet(l, -1))
		flags = static_cast<FPlayAnim>(Lua::CheckInt(l, -1));
}

/////////////////

CEOnAnimationComplete::CEOnAnimationComplete(int32_t animation, Activity activity) : animation(animation), activity(activity) {}
void CEOnAnimationComplete::PushArguments(lua::State *l)
{
	Lua::PushInt(l, animation);
	Lua::PushInt(l, math::to_integral(activity));
}

/////////////////

CELayeredAnimationInfo::CELayeredAnimationInfo(int32_t slot, int32_t animation, Activity activity) : slot(slot), animation(animation), activity(activity) {}
void CELayeredAnimationInfo::PushArguments(lua::State *l)
{
	Lua::PushInt(l, slot);
	Lua::PushInt(l, animation);
	Lua::PushInt(l, math::to_integral(activity));
}

/////////////////

CEOnAnimationStart::CEOnAnimationStart(int32_t animation, Activity activity, FPlayAnim flags) : animation(animation), activity(activity), flags(flags) {}
void CEOnAnimationStart::PushArguments(lua::State *l)
{
	Lua::PushInt(l, animation);
	Lua::PushInt(l, math::to_integral(activity));
	Lua::PushInt(l, math::to_integral(flags));
}

/////////////////

CETranslateLayeredAnimation::CETranslateLayeredAnimation(int32_t &slot, int32_t &animation, FPlayAnim &flags) : slot(slot), animation(animation), flags(flags) {}
void CETranslateLayeredAnimation::PushArguments(lua::State *l)
{
	Lua::PushInt(l, slot);
	Lua::PushInt(l, animation);
	Lua::PushInt(l, math::to_integral(flags));
}
uint32_t CETranslateLayeredAnimation::GetReturnCount() { return 3; }
void CETranslateLayeredAnimation::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -3))
		slot = Lua::CheckInt(l, -3);
	if(Lua::IsSet(l, -2))
		animation = Lua::CheckInt(l, -2);
	if(Lua::IsSet(l, -1))
		flags = static_cast<FPlayAnim>(Lua::CheckInt(l, -1));
}

/////////////////

CETranslateAnimation::CETranslateAnimation(int32_t &animation, FPlayAnim &flags) : animation(animation), flags(flags) {}
void CETranslateAnimation::PushArguments(lua::State *l)
{
	Lua::PushInt(l, animation);
	Lua::PushInt(l, math::to_integral(flags));
}
uint32_t CETranslateAnimation::GetReturnCount() { return 2; }
void CETranslateAnimation::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -2))
		animation = Lua::CheckInt(l, -2);
	if(Lua::IsSet(l, -1))
		flags = static_cast<FPlayAnim>(Lua::CheckInt(l, -1));
}

/////////////////

CETranslateActivity::CETranslateActivity(Activity &activity) : activity(activity) {}
void CETranslateActivity::PushArguments(lua::State *l) { Lua::PushInt(l, math::to_integral(activity)); }
uint32_t CETranslateActivity::GetReturnCount() { return 1; }
void CETranslateActivity::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -1))
		activity = static_cast<Activity>(Lua::CheckInt(l, -1));
}

/////////////////

CEOnBoneTransformChanged::CEOnBoneTransformChanged(UInt32 boneId, const Vector3 *pos, const Quat *rot, const Vector3 *scale) : boneId {boneId}, pos {pos}, rot {rot}, scale {scale} {}
void CEOnBoneTransformChanged::PushArguments(lua::State *l)
{
	Lua::PushInt(l, boneId);
	if(pos != nullptr)
		Lua::Push<Vector3>(l, *pos);
	else
		Lua::PushNil(l);

	if(rot != nullptr)
		Lua::Push<Quat>(l, *rot);
	else
		Lua::PushNil(l);

	if(scale != nullptr)
		Lua::Push<Vector3>(l, *scale);
	else
		Lua::PushNil(l);
}

/////////////////

CEOnPlayActivity::CEOnPlayActivity(Activity activity, FPlayAnim flags) : activity {activity}, flags {flags} {}
void CEOnPlayActivity::PushArguments(lua::State *l)
{
	Lua::PushInt(l, math::to_integral(activity));
	Lua::PushInt(l, math::to_integral(flags));
}

/////////////////

CEOnPlayLayeredActivity::CEOnPlayLayeredActivity(int slot, Activity activity, FPlayAnim flags) : slot {slot}, activity {activity}, flags {flags} {}
void CEOnPlayLayeredActivity::PushArguments(lua::State *l)
{
	Lua::PushInt(l, slot);
	Lua::PushInt(l, math::to_integral(activity));
	Lua::PushInt(l, math::to_integral(flags));
}

/////////////////

CEOnStopLayeredAnimation::CEOnStopLayeredAnimation(int32_t slot, BaseAnimatedComponent::AnimationSlotInfo &slotInfo) : slot {slot}, slotInfo {slotInfo} {}
void CEOnStopLayeredAnimation::PushArguments(lua::State *l)
{
	Lua::PushInt(l, slot);
	Lua::PushInt(l, slotInfo.animation);
	Lua::PushInt(l, math::to_integral(slotInfo.activity));
}

/////////////////

CEOnBlendAnimation::CEOnBlendAnimation(BaseAnimatedComponent::AnimationSlotInfo &slotInfo, Activity activity, std::vector<math::Transform> &bonePoses, std::vector<Vector3> *boneScales)
    : slotInfo {slotInfo}, activity {activity}, bonePoses {bonePoses}, boneScales {boneScales}
{
}
void CEOnBlendAnimation::PushArguments(lua::State *l)
{
	Lua::PushInt(l, slotInfo.animation);
	Lua::PushInt(l, math::to_integral(activity));
}

/////////////////

CEMaintainAnimations::CEMaintainAnimations(double deltaTime) : deltaTime {deltaTime} {}
void CEMaintainAnimations::PushArguments(lua::State *l) { Lua::PushNumber(l, deltaTime); }

/////////////////

CEMaintainAnimation::CEMaintainAnimation(BaseAnimatedComponent::AnimationSlotInfo &slotInfo, double deltaTime) : slotInfo {slotInfo}, deltaTime {deltaTime} {}
void CEMaintainAnimation::PushArguments(lua::State *l)
{
	Lua::PushInt(l, slotInfo.animation);
	Lua::PushInt(l, math::to_integral(slotInfo.activity));
	Lua::PushNumber(l, deltaTime);
}

/////////////////

CEMaintainAnimationMovement::CEMaintainAnimationMovement(const Vector3 &displacement) : displacement {displacement} {}
void CEMaintainAnimationMovement::PushArguments(lua::State *l) { Lua::Push<Vector3>(l, displacement); }

/////////////////

CEShouldUpdateBones::CEShouldUpdateBones() {}
void CEShouldUpdateBones::PushArguments(lua::State *l) {}

/////////////////

CEOnUpdateSkeleton::CEOnUpdateSkeleton() {}
void CEOnUpdateSkeleton::PushArguments(lua::State *l) { Lua::PushBool(l, bonePosesHaveChanged); }
uint32_t CEOnUpdateSkeleton::GetReturnCount() { return 1; }
void CEOnUpdateSkeleton::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -1))
		bonePosesHaveChanged = Lua::CheckBool(l, -1);
}
