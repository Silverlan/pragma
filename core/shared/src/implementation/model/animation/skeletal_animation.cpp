// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :model.animation.skeletal_animation;

import panima;

pragma::Activity pragma::animation::skeletal::get_activity(const panima::Animation &anim)
{
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	auto act = pragma::Activity::Invalid;
	const_cast<Animation &>(anim).GetProperties()["activity"](act);
	return act;
#else
	return Activity::Invalid;
#endif
}
void pragma::animation::skeletal::set_activity(panima::Animation &anim, Activity act)
{
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	anim.GetProperties()["activity"] = act;
#endif
}
uint8_t pragma::animation::skeletal::get_activity_weight(const panima::Animation &anim)
{
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	uint8_t weight = 1;
	const_cast<Animation &>(anim).GetProperties()["activityWeight"](weight);
	return weight;
#else
	return 0;
#endif
}
void pragma::animation::skeletal::set_activity_weight(panima::Animation &anim, uint8_t weight)
{
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	anim.GetProperties()["activityWeight"] = weight;
#endif
}
std::pair<Vector3, Vector3> pragma::animation::skeletal::get_render_bounds(const panima::Animation &anim)
{
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	auto udmRenderBounds = const_cast<Animation &>(anim).GetProperties()["renderBounds"];
	std::pair<Vector3, Vector3> bounds {};
	udmRenderBounds["min"](bounds.first);
	udmRenderBounds["max"](bounds.second);
	return bounds;
#else
	return {};
#endif
}
void pragma::animation::skeletal::set_render_bounds(panima::Animation &anim, const Vector3 &min, const Vector3 &max)
{
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	auto udmRenderBounds = anim.GetProperties()["renderBounds"];
	udmRenderBounds["min"] = min;
	udmRenderBounds["max"] = max;
#endif
}
pragma::animation::skeletal::BoneChannelMap pragma::animation::skeletal::get_bone_channel_map(const panima::Animation &animation, const Skeleton &skeleton)
{
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	BoneChannelMap boneChannelMap;
	auto &channels = animation.GetChannels();
	for(auto i = decltype(channels.size()) {0u}; i < channels.size(); ++i) {
		auto &channel = channels[i];
		auto it = channel->targetPath.begin();
		auto end = channel->targetPath.end();
		if(it == end || *it != SK_ANIMATED_COMPONENT_NAME)
			continue;
		++it;
		auto boneId = (it != end) ? skeleton.LookupBone(*it) : -1;
		if(boneId == -1)
			continue;
		++it;
		if(it == end)
			continue;
		auto &attr = *it;
		auto &channelDesc = boneChannelMap[boneId];
		if(attr == ANIMATION_CHANNEL_PATH_POSITION) {
			channelDesc.positionChannel = i;
			continue;
		}
		if(attr == ANIMATION_CHANNEL_PATH_ROTATION) {
			channelDesc.rotationChannel = i;
			continue;
		}
		if(attr == ANIMATION_CHANNEL_PATH_SCALE) {
			channelDesc.scaleChannel = i;
			continue;
		}
	}
	return boneChannelMap;
#else
	return {};
#endif
}
void pragma::animation::skeletal::animation_slice_to_animated_pose(const BoneChannelMap &boneChannelMap, const panima::Slice &slice, Pose &pose)
{
	auto &transforms = pose.GetTransforms();
	pose.SetTransformCount(boneChannelMap.size());
	uint32_t idx = 0;
	for(auto &pair : boneChannelMap) {
		auto boneId = pair.first;
		pose.SetBoneIndex(idx, boneId);

		auto &pose = transforms[idx];
		auto &channelDesc = pair.second;
		if(channelDesc.positionChannel != AnimBoneChannelDesc::INVALID_CHANNEL)
			pose.SetOrigin(slice.channelValues[channelDesc.positionChannel]->GetValue<Vector3>());
		if(channelDesc.rotationChannel != AnimBoneChannelDesc::INVALID_CHANNEL)
			pose.SetRotation(slice.channelValues[channelDesc.rotationChannel]->GetValue<Quat>());
		if(channelDesc.scaleChannel != AnimBoneChannelDesc::INVALID_CHANNEL)
			pose.SetScale(slice.channelValues[channelDesc.scaleChannel]->GetValue<Vector3>());
		++idx;
	}
}
pragma::util::EnumRegister &pragma::animation::skeletal::get_activity_enum_register()
{
	static util::EnumRegister g_reg {};
	return g_reg;
}
bool pragma::animation::skeletal::is_bone_position_channel(const panima::Channel &channel)
{
	/*auto it = channel.targetPath.begin();
	auto end = channel.targetPath.end();
	if(it == end)
		return false;
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	--end;
#endif
	return *it == SK_ANIMATED_COMPONENT_NAME && *end == panima::ANIMATION_CHANNEL_PATH_POSITION;*/
	return false;
}
bool pragma::animation::skeletal::is_bone_rotation_channel(const panima::Channel &channel)
{
	/*auto it = channel.targetPath.begin();
	auto end = channel.targetPath.end();
	if(it == end)
		return false;
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	--end;
#endif
	return *it == SK_ANIMATED_COMPONENT_NAME && *end == panima::ANIMATION_CHANNEL_PATH_ROTATION;*/
	return false;
}
bool pragma::animation::skeletal::is_bone_scale_channel(const panima::Channel &channel)
{
	/*auto it = channel.targetPath.begin();
	auto end = channel.targetPath.end();
	if(it == end)
		return false;
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	--end;
#endif
	return *it == SK_ANIMATED_COMPONENT_NAME && *end == panima::ANIMATION_CHANNEL_PATH_SCALE;*/
	return false;
}
void pragma::animation::skeletal::translate(panima::Animation &anim, const Vector3 &translation)
{
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	auto renderBounds = get_render_bounds(anim);
	renderBounds.first += translation;
	renderBounds.second += translation;
	set_render_bounds(anim, renderBounds.first, renderBounds.second);
	for(auto &channel : anim.GetChannels()) {
		if(is_bone_position_channel(*channel) == false)
			continue;
		for(auto &v : channel->It<Vector3>())
			v += translation;
	}
#endif
}
void pragma::animation::skeletal::rotate(panima::Animation &anim, const Quat &rotation)
{
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	auto renderBounds = get_render_bounds(anim);
	uvec::rotate(&renderBounds.first, rotation);
	uvec::rotate(&renderBounds.second, rotation);
	set_render_bounds(anim, renderBounds.first, renderBounds.second);
	for(auto &channel : anim.GetChannels()) {
		if(is_bone_position_channel(*channel)) {
			for(auto &v : channel->It<Vector3>())
				uvec::rotate(&v, rotation);
			continue;
		}
		if(is_bone_rotation_channel(*channel)) {
			for(auto &v : channel->It<Quat>())
				v = rotation * v;
		}
	}
#endif
}
void pragma::animation::skeletal::scale(panima::Animation &anim, const Vector3 &scale)
{
#ifdef PRAGMA_ENABLE_ANIMATION_SYSTEM_2
	auto renderBounds = get_render_bounds(anim);
	renderBounds.first *= scale;
	renderBounds.second *= scale;
	set_render_bounds(anim, renderBounds.first, renderBounds.second);
	for(auto &channel : anim.GetChannels()) {
		if(!is_bone_scale_channel(*channel))
			continue;
		for(auto &v : channel->It<Vector3>())
			v *= scale;
	}
#endif
}
