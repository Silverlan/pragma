// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :scripting.lua.classes.animation;

void Lua::Animation::Load(lua::State *l, LFile &f)
{
	auto fptr = f.GetHandle();
	auto offset = fptr->Tell();
	auto len = strlen(::udm::HEADER_IDENTIFIER);
	auto isUdmFormat = true;
	for(auto i = decltype(len) {0u}; i < len; ++i) {
		if(fptr->ReadChar() != ::udm::HEADER_IDENTIFIER[i]) {
			isUdmFormat = false;
			break;
		}
	}
	fptr->Seek(offset);
	if(isUdmFormat) {
		auto udmData = pragma::util::load_udm_asset(std::make_unique<ufile::FileWrapper>(fptr));
		if(udmData == nullptr)
			return;
		std::string err;
		auto anim = pragma::animation::Animation::Load(udmData->GetAssetData(), err);
		if(anim == nullptr)
			return;
		Push(l, anim);
		return;
	}

	FWAD wad;
	auto anim = std::shared_ptr<pragma::animation::Animation>(wad.ReadData(33, *f.GetHandle())); // Animation version has been introduced at model version 33, after which the model version is no longer taken into account, so we'll always treat it as model version 33 here
	if(anim == nullptr)
		return;
	Lua::Push<std::shared_ptr<pragma::animation::Animation>>(l, anim);
}
uint32_t Lua::Animation::RegisterActivityEnum(lua::State *l, const std::string &name)
{
	auto &reg = pragma::animation::Animation::GetActivityEnumRegister();
	return reg.RegisterEnum(name);
}
uint32_t Lua::Animation::RegisterEventEnum(lua::State *l, const std::string &name)
{
	auto &reg = pragma::animation::Animation::GetEventEnumRegister();
	return reg.RegisterEnum(name);
}
luabind::tableT<std::string> Lua::Animation::GetActivityEnums(lua::State *l)
{
	auto &reg = pragma::animation::Animation::GetActivityEnumRegister();
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	reg.Lock();
	for(auto &name : reg.GetEnums())
		t[idx++] = name;
	reg.Unlock();
	return t;
}
luabind::tableT<std::string> Lua::Animation::GetEventEnums(lua::State *l)
{
	auto &reg = pragma::animation::Animation::GetEventEnumRegister();
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	reg.Lock();
	for(auto &name : reg.GetEnums())
		t[idx++] = name;
	reg.Unlock();
	return t;
}
luabind::optional<std::string> Lua::Animation::GetActivityEnumName(lua::State *l, uint32_t id)
{
	auto &reg = pragma::animation::Animation::GetActivityEnumRegister();
	auto *name = reg.GetEnumName(id);
	if(name == nullptr)
		return nil;
	return luabind::object {l, *name};
}
luabind::optional<std::string> Lua::Animation::GetEventEnumName(lua::State *l, uint32_t id)
{
	auto &reg = pragma::animation::Animation::GetEventEnumRegister();
	auto *name = reg.GetEnumName(id);
	if(name == nullptr)
		return nil;
	return luabind::object {l, *name};
}
luabind::optional<uint32_t> Lua::Animation::FindActivityId(lua::State *l, const std::string &name)
{
	auto &reg = pragma::animation::Animation::GetActivityEnumRegister();
	uint32_t id;
	if(reg.GetEnumValue(name, id) == false)
		return nil;
	return luabind::object {l, id};
}
luabind::optional<uint32_t> Lua::Animation::FindEventId(lua::State *l, const std::string &name)
{
	auto &reg = pragma::animation::Animation::GetEventEnumRegister();
	uint32_t id;
	if(reg.GetEnumValue(name, id) == false)
		return nil;
	return luabind::object {l, id};
}
luabind::tableT<uint16_t> Lua::Animation::GetBoneList(lua::State *l, pragma::animation::Animation &anim) { return vector_to_table(l, anim.GetBoneList()); }
void Lua::Animation::GetFrame(lua::State *l, pragma::animation::Animation &anim, unsigned int ID)
{
	auto frame = anim.GetFrame(ID);
	if(frame == nullptr)
		return;
	luabind::object(l, frame).push(l);
}
void Lua::Animation::AddFrame(lua::State *, pragma::animation::Animation &anim, ::Frame &frame) { anim.AddFrame(frame.shared_from_this()); }
void Lua::Animation::GetFrames(lua::State *l, pragma::animation::Animation &anim)
{
	auto numFrames = anim.GetFrameCount();
	auto t = CreateTable(l);
	for(auto i = decltype(numFrames) {0}; i < numFrames; ++i) {
		auto frame = anim.GetFrame(i);
		PushInt(l, i + 1);
		Lua::Push<std::shared_ptr<::Frame>>(l, frame);
		SetTableValue(l, t);
	}
}
void Lua::Animation::GetAnimationEventArguments(lua::State *l, int32_t tArgs, std::vector<std::string> &args)
{
	auto numArgs = GetObjectLength(l, tArgs);
	for(auto i = decltype(numArgs) {0}; i < numArgs; ++i) {
		PushInt(l, i + 1); /* 1 */
		GetTableValue(l, tArgs);

		auto *arg = CheckString(l, -1);
		args.push_back(arg);

		Pop(l, 1);
	}
}
void Lua::Animation::AddEvent(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t eventId, luabind::object args)
{
	int32_t tArgs = 4;
	CheckTable(l, tArgs);
	auto ev = std::make_unique<pragma::AnimationEvent>();
	ev->eventID = static_cast<pragma::AnimationEvent::Type>(eventId);

	GetAnimationEventArguments(l, tArgs, ev->arguments);

	auto *pEv = ev.get();
	ev.release();
	anim.AddEvent(frameId, pEv);
}
void Lua::Animation::PushAnimationEvent(lua::State *l, const pragma::AnimationEvent &ev)
{
	auto tEvent = CreateTable(l);

	PushString(l, "type");
	PushInt(l, ev.eventID);
	SetTableValue(l, tEvent);

	PushString(l, "arguments");
	auto tArgs = CreateTable(l);
	auto &args = ev.arguments;
	for(auto j = decltype(args.size()) {0}; j < args.size(); ++j) {
		PushInt(l, j + 1);
		PushString(l, args[j]);
		SetTableValue(l, tArgs);
	}
	SetTableValue(l, tEvent);
}
void Lua::Animation::GetEvents(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId)
{
	auto *events = anim.GetEvents(frameId);

	auto t = CreateTable(l);
	if(events == nullptr)
		return;
	for(auto i = decltype(events->size()) {0}; i < events->size(); ++i) {
		PushInt(l, i + 1);
		PushAnimationEvent(l, *(*events)[i]);

		SetTableValue(l, t);
	}
}
void Lua::Animation::GetEvents(lua::State *l, pragma::animation::Animation &anim)
{
	auto numFrames = anim.GetFrameCount();
	auto t = CreateTable(l);
	for(auto i = decltype(numFrames) {0}; i < numFrames; ++i) {
		auto *events = anim.GetEvents(i);
		if(events == nullptr)
			continue;
		PushInt(l, i);
		auto tEvent = CreateTable(l);
		for(auto j = decltype(events->size()) {0}; j < events->size(); ++j) {
			auto &ev = (*events)[j];
			PushInt(l, j + 1);
			PushAnimationEvent(l, *ev);
			SetTableValue(l, tEvent);
		}
		SetTableValue(l, t);
	}
}
void Lua::Animation::GetEventCount(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId)
{
	auto *events = anim.GetEvents(frameId);
	PushInt(l, (events != nullptr) ? events->size() : 0);
}
void Lua::Animation::GetEventCount(lua::State *l, pragma::animation::Animation &anim)
{
	auto numFrames = anim.GetFrameCount();
	uint32_t numEvents = 0;
	for(auto i = decltype(numFrames) {0}; i < numFrames; ++i) {
		auto *events = anim.GetEvents(i);
		if(events == nullptr)
			continue;
		numEvents += static_cast<uint32_t>(events->size());
	}
	PushInt(l, numEvents);
}
static void push_blend_controller(lua::State *l, AnimationBlendController &bc)
{
	auto t = Lua::CreateTable(l); /* 1 */

	Lua::PushString(l, "controller"); /* 2 */
	Lua::PushInt(l, bc.controller);   /* 3 */
	Lua::SetTableValue(l, t);         /* 1 */

	Lua::PushString(l, "transitions");       /* 2 */
	auto tTransitions = Lua::CreateTable(l); /* 3 */
	auto &transitions = bc.transitions;
	for(auto i = decltype(transitions.size()) {0}; i < transitions.size(); ++i) {
		Lua::PushInt(l, i + 1); /* 4 */

		auto tTransition = Lua::CreateTable(l); /* 5 */
		auto &t = transitions[i];

		Lua::PushString(l, "animation");    /* 6 */
		Lua::PushInt(l, t.animation);       /* 7 */
		Lua::SetTableValue(l, tTransition); /* 5 */

		Lua::PushString(l, "transition");   /* 6 */
		Lua::PushInt(l, t.transition);      /* 7 */
		Lua::SetTableValue(l, tTransition); /* 5 */

		Lua::SetTableValue(l, tTransitions); /* 3 */
	}

	Lua::PushString(l, "animationPostBlendController"); /* 2 */
	Lua::PushInt(l, bc.animationPostBlendController);   /* 3 */
	Lua::SetTableValue(l, t);                           /* 1 */

	Lua::PushString(l, "animationPostBlendTarget"); /* 2 */
	Lua::PushInt(l, bc.animationPostBlendTarget);   /* 3 */
	Lua::SetTableValue(l, t);                       /* 1 */

	Lua::SetTableValue(l, t); /* 1 */
}
void Lua::Animation::GetBlendController(lua::State *l, pragma::animation::Animation &anim)
{
	auto *bc = anim.GetBlendController();
	if(bc == nullptr)
		return;
	push_blend_controller(l, *bc);
}
void Lua::Animation::CalcRenderBounds(lua::State *l, pragma::animation::Animation &anim, const std::shared_ptr<pragma::asset::Model> &mdl)
{
	anim.CalcRenderBounds(*mdl);
	GetRenderBounds(l, anim, mdl);
}
void Lua::Animation::GetRenderBounds(lua::State *l, pragma::animation::Animation &anim, const std::shared_ptr<pragma::asset::Model> &)
{
	auto &bounds = anim.GetRenderBounds();
	Lua::Push<Vector3>(l, bounds.first);
	Lua::Push<Vector3>(l, bounds.second);
}
void Lua::Animation::RemoveEvent(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx)
{
	auto *events = anim.GetEvents(frameId);
	if(events == nullptr || idx >= events->size())
		return;
	events->erase(events->begin() + idx);
}
void Lua::Animation::SetEventData(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx, uint32_t type, luabind::object oArgs)
{
	auto *events = anim.GetEvents(frameId);
	if(events == nullptr || idx >= events->size())
		return;
	auto &ev = events->at(idx);

	int32_t tArgs = 5;
	CheckTable(l, tArgs);

	ev->eventID = static_cast<pragma::AnimationEvent::Type>(type);
	ev->arguments.clear();
	GetAnimationEventArguments(l, tArgs, ev->arguments);
}
void Lua::Animation::SetEventType(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx, uint32_t type)
{
	auto *events = anim.GetEvents(frameId);
	if(events == nullptr || idx >= events->size())
		return;
	auto &ev = events->at(idx);

	ev->eventID = static_cast<pragma::AnimationEvent::Type>(type);
}
void Lua::Animation::SetEventArgs(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx, luabind::object oArgs)
{
	auto *events = anim.GetEvents(frameId);
	if(events == nullptr || idx >= events->size())
		return;
	auto &ev = events->at(idx);

	int32_t tArgs = 4;
	CheckTable(l, tArgs);

	ev->arguments.clear();
	GetAnimationEventArguments(l, tArgs, ev->arguments);
}
void Lua::Animation::LookupBone(lua::State *l, pragma::animation::Animation &anim, uint32_t boneId)
{
	auto &list = anim.GetBoneList();
	auto it = std::find(list.begin(), list.end(), boneId);
	if(it == list.end())
		return;
	PushInt(l, it - list.begin());
}
void Lua::Animation::SetBoneList(lua::State *l, pragma::animation::Animation &anim, luabind::object o)
{
	int32_t t = 2;
	auto numBones = GetObjectLength(l, t);
	std::vector<uint16_t> list;
	list.reserve(numBones);
	for(auto i = decltype(numBones) {0}; i < numBones; ++i) {
		PushInt(l, i + 1);
		GetTableValue(l, t);
		list.push_back(CheckInt(l, -1));

		Pop(l, 1);
	}
	anim.SetBoneList(list);
}
void Lua::Animation::GetBoneWeights(lua::State *l, pragma::animation::Animation &anim)
{
	auto &weights = anim.GetBoneWeights();
	auto t = CreateTable(l);
	for(auto i = decltype(weights.size()) {0}; i < weights.size(); ++i) {
		PushInt(l, i + 1);
		PushNumber(l, weights.at(i));
		SetTableValue(l, t);
	}
}

///////////////////////////////////////

void Lua::Frame::Create(lua::State *l, uint32_t numBones) { Lua::Push<std::shared_ptr<::Frame>>(l, ::Frame::Create(numBones)); }
void Lua::Frame::GetBoneMatrix(lua::State *l, ::Frame &frame, unsigned int boneID)
{
	auto mat = umat::identity();
	if(frame.GetBoneMatrix(boneID, &mat) == false)
		return;
	luabind::object(l, mat).push(l);
}

void Lua::Frame::GetBonePosition(lua::State *l, ::Frame &frame, unsigned int boneID)
{
	Vector3 *pos = frame.GetBonePosition(boneID);
	if(pos == nullptr)
		return;
	auto *rot = frame.GetBoneOrientation(boneID);
	auto *scale = frame.GetBoneScale(boneID);
	luabind::object(l, *pos).push(l);
	luabind::object(l, *rot).push(l);
	Lua::Push<Vector3>(l, (scale != nullptr) ? *scale : Vector3 {1.f, 1.f, 1.f});
}

void Lua::Frame::GetBoneOrientation(lua::State *l, ::Frame &frame, unsigned int boneID)
{
	auto *rot = frame.GetBoneOrientation(boneID);
	if(rot == nullptr)
		return;
	luabind::object(l, *rot).push(l);
}
void Lua::Frame::SetBonePosition(lua::State *, ::Frame &frame, unsigned int boneID, const Vector3 &pos) { frame.SetBonePosition(boneID, pos); }
void Lua::Frame::SetBoneOrientation(lua::State *, ::Frame &frame, unsigned int boneID, const Quat &rot) { frame.SetBoneOrientation(boneID, rot); }
void Lua::Frame::SetBoneTransform(lua::State *l, ::Frame &frame, unsigned int boneID, const Vector3 &pos, const Quat &rot)
{
	frame.SetBonePosition(boneID, pos);
	frame.SetBoneOrientation(boneID, rot);
}
void Lua::Frame::SetBoneTransform(lua::State *l, ::Frame &frame, unsigned int boneID, const Vector3 &pos, const Quat &rot, const Vector3 &scale)
{
	SetBoneTransform(l, frame, boneID, pos, rot);
	frame.SetBoneScale(boneID, scale);
}
void Lua::Frame::Localize(lua::State *, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton) { frame.Localize(anim, *skeleton); }
void Lua::Frame::Globalize(lua::State *, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton) { frame.Globalize(anim, *skeleton); }
void Lua::Frame::Localize(lua::State *, ::Frame &frame, pragma::animation::Skeleton *skeleton) { frame.Localize(*skeleton); }
void Lua::Frame::Globalize(lua::State *, ::Frame &frame, pragma::animation::Skeleton *skeleton) { frame.Globalize(*skeleton); }
void Lua::Frame::CalcRenderBounds(lua::State *l, ::Frame &frame, pragma::animation::Animation &anim, const std::shared_ptr<pragma::asset::Model> &mdl)
{
	auto renderBounds = frame.CalcRenderBounds(anim, *mdl);
	Lua::Push<Vector3>(l, renderBounds.first);
	Lua::Push<Vector3>(l, renderBounds.second);
}
void Lua::Frame::Rotate(lua::State *, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton, const Quat &rot) { frame.Rotate(anim, *skeleton, rot); }
void Lua::Frame::Translate(lua::State *, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton, const Vector3 &t) { frame.Translate(anim, *skeleton, t); }
void Lua::Frame::Scale(lua::State *l, ::Frame &frame, const Vector3 &scale) { frame.Scale(scale); }
void Lua::Frame::GetMoveTranslation(lua::State *l, ::Frame &frame)
{
	::Vector2 translation;
	frame.GetMoveOffset(&translation.x, &translation.y);
	PushNumber(l, translation.x);
	PushNumber(l, translation.y);
}
void Lua::Frame::GetMoveTranslationX(lua::State *l, ::Frame &frame)
{
	::Vector2 translation;
	frame.GetMoveOffset(&translation.x, &translation.y);
	PushNumber(l, translation.x);
}
void Lua::Frame::GetMoveTranslationZ(lua::State *l, ::Frame &frame)
{
	::Vector2 translation;
	frame.GetMoveOffset(&translation.x, &translation.y);
	PushNumber(l, translation.y);
}
void Lua::Frame::SetMoveTranslation(lua::State *l, ::Frame &frame, float x, float z) { frame.SetMoveOffset(x, z); }
void Lua::Frame::SetMoveTranslationX(lua::State *l, ::Frame &frame, float x)
{
	::Vector2 translation;
	frame.GetMoveOffset(&translation.x, &translation.y);
	frame.SetMoveOffset(x, translation.y);
}
void Lua::Frame::SetMoveTranslationZ(lua::State *l, ::Frame &frame, float z)
{
	::Vector2 translation;
	frame.GetMoveOffset(&translation.x, &translation.y);
	frame.SetMoveOffset(translation.x, z);
}
void Lua::Frame::SetBoneScale(lua::State *l, ::Frame &frame, uint32_t boneId, const Vector3 &scale) { frame.SetBoneScale(boneId, scale); }
void Lua::Frame::GetBoneScale(lua::State *l, ::Frame &frame, uint32_t boneId)
{
	auto *scale = frame.GetBoneScale(boneId);
	if(scale == nullptr)
		return;
	Lua::Push<Vector3>(l, *scale);
}
void Lua::Frame::GetLocalBoneTransform(lua::State *l, ::Frame &frame, pragma::animation::Skeleton &skeleton, uint32_t boneId)
{
	auto wpBone = skeleton.GetBone(boneId);
	if(wpBone.expired())
		return;
	auto bone = wpBone.lock();
	Vector3 pos {};
	auto rot = uquat::identity();

	auto fGetTransform = [&frame](uint32_t boneId, Vector3 **pos, Quat **rot, Vector3 **scale) {
		*pos = frame.GetBonePosition(boneId);
		*rot = frame.GetBoneOrientation(boneId);
		*scale = frame.GetBoneScale(boneId);
	};

	// See also baseentity_bones.cpp
	std::function<void(const std::shared_ptr<pragma::animation::Bone> &, Vector3 &, Quat &, Vector3 *)> fIterateHierarchy = nullptr;
	fIterateHierarchy = [fGetTransform, &fIterateHierarchy](const std::shared_ptr<pragma::animation::Bone> &bone, Vector3 &pos, Quat &rot, Vector3 *scale) {
		Vector3 *tpos = nullptr;
		Quat *trot = nullptr;
		Vector3 *tscale = nullptr;
		fGetTransform(bone->ID, &tpos, &trot, &tscale);
		assert(tpos != nullptr && trot != nullptr);
		if(tpos == nullptr || trot == nullptr) {
			pos = {};
			rot = {};
			if(scale != nullptr)
				*scale = Vector3 {1.f, 1.f, 1.f};
			return;
		}
		if(bone->parent.expired() == true) {
			pos = *tpos;
			rot = *trot;
			if(scale != nullptr)
				*scale = (tscale != nullptr) ? *tscale : Vector3 {1.f, 1.f, 1.f};
			return;
		}
		fIterateHierarchy(bone->parent.lock(), pos, rot, scale);
		auto bonePos = *tpos;
		auto &boneRot = *trot;
		uvec::rotate(&bonePos, rot);
		if(scale != nullptr)
			bonePos *= *scale;
		pos += bonePos;

		if(scale != nullptr)
			*scale *= (tscale != nullptr) ? *tscale : Vector3 {1.f, 1.f, 1.f};

		rot = rot * boneRot;
	};
	Vector3 nscale(1.f, 1.f, 1.f);
	fIterateHierarchy(bone, pos, rot, &nscale);

	Lua::Push<Vector3>(l, pos);
	Lua::Push<Quat>(l, rot);
	Lua::Push<Vector3>(l, nscale);
}
void Lua::Frame::GetBoneCount(lua::State *l, ::Frame &frame) { PushInt(l, frame.GetBoneCount()); }
void Lua::Frame::SetBoneCount(lua::State *l, ::Frame &frame, uint32_t boneCount) { frame.SetBoneCount(boneCount); }
void Lua::Frame::SetBonePose(lua::State *, ::Frame &frame, uint32_t boneId, const pragma::math::ScaledTransform &pose) { frame.SetBonePose(boneId, pose); }
void Lua::Frame::SetBonePose(lua::State *, ::Frame &frame, uint32_t boneId, const pragma::math::Transform &pose) { frame.SetBonePose(boneId, pose); }
void Lua::Frame::GetBonePose(lua::State *l, ::Frame &frame, uint32_t boneId)
{
	pragma::math::ScaledTransform pose;
	if(frame.GetBonePose(boneId, pose) == false)
		return;
	Push(l, pose);
}
///////////////////////////////////////
void Lua::VertexAnimation::GetMeshAnimations(lua::State *l, pragma::animation::VertexAnimation &anim)
{
	auto &meshAnims = anim.GetMeshAnimations();
	auto t = CreateTable(l);
	auto meshAnimIdx = 1u;
	for(auto meshAnim : meshAnims) {
		PushInt(l, meshAnimIdx++);
		Lua::Push<std::shared_ptr<pragma::animation::MeshVertexAnimation>>(l, meshAnim);
		SetTableValue(l, t);
	}
}
void Lua::VertexAnimation::GetName(lua::State *l, pragma::animation::VertexAnimation &anim) { PushString(l, anim.GetName()); }
///////////////////////////////////////
void Lua::MeshVertexAnimation::GetFrames(lua::State *l, pragma::animation::MeshVertexAnimation &anim)
{
	auto &frames = anim.GetFrames();
	auto t = CreateTable(l);
	auto frameIdx = 1u;
	for(auto &frame : frames) {
		PushInt(l, frameIdx++);
		Lua::Push<std::shared_ptr<pragma::animation::MeshVertexFrame>>(l, frame);
		SetTableValue(l, t);
	}
}
void Lua::MeshVertexAnimation::GetMesh(lua::State *l, pragma::animation::MeshVertexAnimation &anim)
{
	auto *mesh = anim.GetMesh();
	auto *subMesh = anim.GetSubMesh();
	if(mesh == nullptr || subMesh == nullptr)
		return;
	Lua::Push<std::shared_ptr<pragma::geometry::ModelMesh>>(l, mesh->shared_from_this());
	Lua::Push<std::shared_ptr<pragma::geometry::ModelSubMesh>>(l, subMesh->shared_from_this());
}
///////////////////////////////////////
void Lua::MeshVertexFrame::GetVertices(lua::State *l, pragma::animation::MeshVertexFrame &frame)
{
	auto &verts = frame.GetVertices();
	auto t = CreateTable(l);
	auto vertIdx = 1u;
	for(auto &v : verts) {
		PushInt(l, vertIdx++);
		Lua::Push<Vector3>(l, {pragma::math::float16_to_float32(v.at(0)), pragma::math::float16_to_float32(v.at(1)), pragma::math::float16_to_float32(v.at(2))});
		SetTableValue(l, t);
	}
}
void Lua::MeshVertexFrame::SetVertexCount(lua::State *l, pragma::animation::MeshVertexFrame &frame, uint32_t numVerts) { frame.SetVertexCount(numVerts); }
void Lua::MeshVertexFrame::SetVertexPosition(lua::State *l, pragma::animation::MeshVertexFrame &frame, uint32_t vertIdx, const Vector3 &pos) { frame.SetVertexPosition(vertIdx, pos); }
void Lua::MeshVertexFrame::GetVertexPosition(lua::State *l, pragma::animation::MeshVertexFrame &frame, uint32_t vertIdx)
{
	Vector3 pos {};
	if(frame.GetVertexPosition(vertIdx, pos) == false)
		return;
	Lua::Push<Vector3>(l, pos);
}
void Lua::MeshVertexFrame::SetVertexNormal(lua::State *l, pragma::animation::MeshVertexFrame &frame, uint32_t vertIdx, const Vector3 &n) { frame.SetVertexNormal(vertIdx, n); }
void Lua::MeshVertexFrame::GetVertexNormal(lua::State *l, pragma::animation::MeshVertexFrame &frame, uint32_t vertIdx)
{
	Vector3 n {};
	if(frame.GetVertexPosition(vertIdx, n) == false)
		return;
	Lua::Push<Vector3>(l, n);
}
