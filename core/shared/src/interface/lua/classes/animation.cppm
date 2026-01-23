// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.animation;

export import :model.animation;
export import :scripting.lua.api;
import :scripting.lua.libraries.file;

export namespace Lua {
	namespace Animation {
		DLLNETWORK void PushAnimationEvent(lua::State *l, const pragma::AnimationEvent &ev);
		DLLNETWORK void GetAnimationEventArguments(lua::State *l, int32_t tArgs, std::vector<std::string> &args);

		DLLNETWORK void Load(lua::State *l, LFile &f);
		DLLNETWORK uint32_t RegisterActivityEnum(lua::State *l, const std::string &name);
		DLLNETWORK uint32_t RegisterEventEnum(lua::State *l, const std::string &name);
		DLLNETWORK luabind::tableT<std::string> GetActivityEnums(lua::State *l);
		DLLNETWORK luabind::tableT<std::string> GetEventEnums(lua::State *l);
		DLLNETWORK luabind::optional<std::string> GetActivityEnumName(lua::State *l, uint32_t id);
		DLLNETWORK luabind::optional<std::string> GetEventEnumName(lua::State *l, uint32_t id);
		DLLNETWORK luabind::optional<uint32_t> FindActivityId(lua::State *l, const std::string &name);
		DLLNETWORK luabind::optional<uint32_t> FindEventId(lua::State *l, const std::string &name);

		DLLNETWORK void GetFrame(lua::State *l, pragma::animation::Animation &anim, unsigned int ID);
		DLLNETWORK luabind::tableT<uint16_t> GetBoneList(lua::State *l, pragma::animation::Animation &anim);
		DLLNETWORK void AddFrame(lua::State *l, pragma::animation::Animation &anim, Frame &frame);
		DLLNETWORK void GetFrames(lua::State *l, pragma::animation::Animation &anim);
		DLLNETWORK void AddEvent(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t eventId, luabind::object tArgs);
		DLLNETWORK void GetEvents(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId);
		DLLNETWORK void GetEvents(lua::State *l, pragma::animation::Animation &anim);
		DLLNETWORK void GetEventCount(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId);
		DLLNETWORK void GetEventCount(lua::State *l, pragma::animation::Animation &anim);
		DLLNETWORK void GetBlendController(lua::State *l, pragma::animation::Animation &anim);
		DLLNETWORK void CalcRenderBounds(lua::State *l, pragma::animation::Animation &anim, const std::shared_ptr<pragma::asset::Model> &mdl);
		DLLNETWORK void GetRenderBounds(lua::State *l, pragma::animation::Animation &anim, const std::shared_ptr<pragma::asset::Model> &mdl);
		DLLNETWORK void RemoveEvent(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx);
		DLLNETWORK void SetEventData(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx, uint32_t type, luabind::object tArgs);
		DLLNETWORK void SetEventType(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx, uint32_t type);
		DLLNETWORK void SetEventArgs(lua::State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx, luabind::object tArgs);
		DLLNETWORK void LookupBone(lua::State *l, pragma::animation::Animation &anim, uint32_t boneId);
		DLLNETWORK void SetBoneList(lua::State *l, pragma::animation::Animation &anim, luabind::object);

		DLLNETWORK void GetBoneWeights(lua::State *l, pragma::animation::Animation &anim);
	};
	namespace Frame {
		DLLNETWORK void Create(lua::State *l, uint32_t numBones);
		DLLNETWORK void GetBoneMatrix(lua::State *l, ::Frame &frame, unsigned int boneID);
		DLLNETWORK void GetBonePosition(lua::State *l, ::Frame &frame, unsigned int boneID);
		DLLNETWORK void GetBoneOrientation(lua::State *l, ::Frame &frame, unsigned int boneID);
		DLLNETWORK void SetBonePosition(lua::State *l, ::Frame &frame, unsigned int boneID, const Vector3 &pos);
		DLLNETWORK void SetBoneOrientation(lua::State *l, ::Frame &frame, unsigned int boneID, const Quat &rot);
		DLLNETWORK void SetBoneTransform(lua::State *l, ::Frame &frame, unsigned int boneID, const Vector3 &pos, const Quat &rot);
		DLLNETWORK void SetBoneTransform(lua::State *l, ::Frame &frame, unsigned int boneID, const Vector3 &pos, const Quat &rot, const Vector3 &scale);
		DLLNETWORK void Localize(lua::State *l, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton);
		DLLNETWORK void Globalize(lua::State *l, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton);
		DLLNETWORK void Localize(lua::State *l, ::Frame &frame, pragma::animation::Skeleton *skeleton);
		DLLNETWORK void Globalize(lua::State *l, ::Frame &frame, pragma::animation::Skeleton *skeleton);
		DLLNETWORK void CalcRenderBounds(lua::State *l, ::Frame &frame, pragma::animation::Animation &anim, const std::shared_ptr<pragma::asset::Model> &mdl);
		DLLNETWORK void Rotate(lua::State *l, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton, const Quat &rot);
		DLLNETWORK void Translate(lua::State *l, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton, const Vector3 &t);
		DLLNETWORK void Scale(lua::State *l, ::Frame &frame, const Vector3 &scale);
		DLLNETWORK void GetMoveTranslation(lua::State *l, ::Frame &frame);
		DLLNETWORK void GetMoveTranslationX(lua::State *l, ::Frame &frame);
		DLLNETWORK void GetMoveTranslationZ(lua::State *l, ::Frame &frame);
		DLLNETWORK void SetMoveTranslation(lua::State *l, ::Frame &frame, float x, float z);
		DLLNETWORK void SetMoveTranslationX(lua::State *l, ::Frame &frame, float x);
		DLLNETWORK void SetMoveTranslationZ(lua::State *l, ::Frame &frame, float z);
		DLLNETWORK void SetBoneScale(lua::State *l, ::Frame &frame, uint32_t boneId, const Vector3 &scale);
		DLLNETWORK void GetBoneScale(lua::State *l, ::Frame &frame, uint32_t boneId);
		DLLNETWORK void GetLocalBoneTransform(lua::State *, ::Frame &frame, pragma::animation::Skeleton &skeleton, uint32_t boneId);
		DLLNETWORK void GetBoneCount(lua::State *, ::Frame &frame);
		DLLNETWORK void SetBoneCount(lua::State *, ::Frame &frame, uint32_t boneCount);
		DLLNETWORK void SetBonePose(lua::State *, ::Frame &frame, uint32_t boneId, const pragma::math::ScaledTransform &pose);
		DLLNETWORK void SetBonePose(lua::State *, ::Frame &frame, uint32_t boneId, const pragma::math::Transform &pose);
		DLLNETWORK void GetBonePose(lua::State *, ::Frame &frame, uint32_t boneId);
	};
	namespace VertexAnimation {
		DLLNETWORK void GetMeshAnimations(lua::State *l, pragma::animation::VertexAnimation &anim);
		DLLNETWORK void GetName(lua::State *l, pragma::animation::VertexAnimation &anim);
	};
	namespace MeshVertexAnimation {
		DLLNETWORK void GetFrames(lua::State *l, pragma::animation::MeshVertexAnimation &anim);
		DLLNETWORK void GetMesh(lua::State *l, pragma::animation::MeshVertexAnimation &anim);
	};
	namespace MeshVertexFrame {
		DLLNETWORK void GetVertices(lua::State *l, pragma::animation::MeshVertexFrame &frame);
		DLLNETWORK void SetVertexCount(lua::State *l, pragma::animation::MeshVertexFrame &frame, uint32_t numVerts);
		DLLNETWORK void SetVertexPosition(lua::State *l, pragma::animation::MeshVertexFrame &frame, uint32_t vertIdx, const Vector3 &pos);
		DLLNETWORK void GetVertexPosition(lua::State *l, pragma::animation::MeshVertexFrame &frame, uint32_t vertIdx);
		DLLNETWORK void SetVertexNormal(lua::State *l, pragma::animation::MeshVertexFrame &frame, uint32_t vertIdx, const Vector3 &n);
		DLLNETWORK void GetVertexNormal(lua::State *l, pragma::animation::MeshVertexFrame &frame, uint32_t vertIdx);
	};
};
