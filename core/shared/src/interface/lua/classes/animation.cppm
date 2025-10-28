// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include <string>

#include <cinttypes>
#include <optional>
#include <vector>
#include <memory>

#include <ostream>

export module pragma.shared:scripting.lua.classes.animation;

export import :model.animation;
export import :scripting.lua.api;
import :scripting.lua.libraries.file;

export namespace Lua {
	namespace Animation {
		DLLNETWORK void PushAnimationEvent(lua_State *l, const pragma::AnimationEvent &ev);
		DLLNETWORK void GetAnimationEventArguments(lua_State *l, int32_t tArgs, std::vector<std::string> &args);

		DLLNETWORK void Load(lua_State *l, LFile &f);
		DLLNETWORK uint32_t RegisterActivityEnum(lua_State *l, const std::string &name);
		DLLNETWORK uint32_t RegisterEventEnum(lua_State *l, const std::string &name);
		DLLNETWORK luabind::tableT<std::string> GetActivityEnums(lua_State *l);
		DLLNETWORK luabind::tableT<std::string> GetEventEnums(lua_State *l);
		DLLNETWORK luabind::optional<std::string> GetActivityEnumName(lua_State *l, uint32_t id);
		DLLNETWORK luabind::optional<std::string> GetEventEnumName(lua_State *l, uint32_t id);
		DLLNETWORK luabind::optional<uint32_t> FindActivityId(lua_State *l, const std::string &name);
		DLLNETWORK luabind::optional<uint32_t> FindEventId(lua_State *l, const std::string &name);

		DLLNETWORK void GetFrame(lua_State *l, pragma::animation::Animation &anim, unsigned int ID);
		DLLNETWORK luabind::tableT<uint16_t> GetBoneList(lua_State *l, pragma::animation::Animation &anim);
		DLLNETWORK void AddFrame(lua_State *l, pragma::animation::Animation &anim, ::Frame &frame);
		DLLNETWORK void GetFrames(lua_State *l, pragma::animation::Animation &anim);
		DLLNETWORK void AddEvent(lua_State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t eventId, luabind::object tArgs);
		DLLNETWORK void GetEvents(lua_State *l, pragma::animation::Animation &anim, uint32_t frameId);
		DLLNETWORK void GetEvents(lua_State *l, pragma::animation::Animation &anim);
		DLLNETWORK void GetEventCount(lua_State *l, pragma::animation::Animation &anim, uint32_t frameId);
		DLLNETWORK void GetEventCount(lua_State *l, pragma::animation::Animation &anim);
		DLLNETWORK void GetBlendController(lua_State *l, pragma::animation::Animation &anim);
		DLLNETWORK void CalcRenderBounds(lua_State *l, pragma::animation::Animation &anim, const std::shared_ptr<pragma::Model> &mdl);
		DLLNETWORK void GetRenderBounds(lua_State *l, pragma::animation::Animation &anim, const std::shared_ptr<pragma::Model> &mdl);
		DLLNETWORK void RemoveEvent(lua_State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx);
		DLLNETWORK void SetEventData(lua_State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx, uint32_t type, luabind::object tArgs);
		DLLNETWORK void SetEventType(lua_State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx, uint32_t type);
		DLLNETWORK void SetEventArgs(lua_State *l, pragma::animation::Animation &anim, uint32_t frameId, uint32_t idx, luabind::object tArgs);
		DLLNETWORK void LookupBone(lua_State *l, pragma::animation::Animation &anim, uint32_t boneId);
		DLLNETWORK void SetBoneList(lua_State *l, pragma::animation::Animation &anim, luabind::object);

		DLLNETWORK void GetBoneWeights(lua_State *l, pragma::animation::Animation &anim);
	};
	namespace Frame {
		DLLNETWORK void Create(lua_State *l, uint32_t numBones);
		DLLNETWORK void GetBoneMatrix(lua_State *l, ::Frame &frame, unsigned int boneID);
		DLLNETWORK void GetBonePosition(lua_State *l, ::Frame &frame, unsigned int boneID);
		DLLNETWORK void GetBoneOrientation(lua_State *l, ::Frame &frame, unsigned int boneID);
		DLLNETWORK void SetBonePosition(lua_State *l, ::Frame &frame, unsigned int boneID, const Vector3 &pos);
		DLLNETWORK void SetBoneOrientation(lua_State *l, ::Frame &frame, unsigned int boneID, const Quat &rot);
		DLLNETWORK void SetBoneTransform(lua_State *l, ::Frame &frame, unsigned int boneID, const Vector3 &pos, const Quat &rot);
		DLLNETWORK void SetBoneTransform(lua_State *l, ::Frame &frame, unsigned int boneID, const Vector3 &pos, const Quat &rot, const Vector3 &scale);
		DLLNETWORK void Localize(lua_State *l, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton);
		DLLNETWORK void Globalize(lua_State *l, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton);
		DLLNETWORK void Localize(lua_State *l, ::Frame &frame, pragma::animation::Skeleton *skeleton);
		DLLNETWORK void Globalize(lua_State *l, ::Frame &frame, pragma::animation::Skeleton *skeleton);
		DLLNETWORK void CalcRenderBounds(lua_State *l, ::Frame &frame, pragma::animation::Animation &anim, const std::shared_ptr<pragma::Model> &mdl);
		DLLNETWORK void Rotate(lua_State *l, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton, const Quat &rot);
		DLLNETWORK void Translate(lua_State *l, ::Frame &frame, pragma::animation::Animation &anim, pragma::animation::Skeleton *skeleton, const Vector3 &t);
		DLLNETWORK void Scale(lua_State *l, ::Frame &frame, const Vector3 &scale);
		DLLNETWORK void GetMoveTranslation(lua_State *l, ::Frame &frame);
		DLLNETWORK void GetMoveTranslationX(lua_State *l, ::Frame &frame);
		DLLNETWORK void GetMoveTranslationZ(lua_State *l, ::Frame &frame);
		DLLNETWORK void SetMoveTranslation(lua_State *l, ::Frame &frame, float x, float z);
		DLLNETWORK void SetMoveTranslationX(lua_State *l, ::Frame &frame, float x);
		DLLNETWORK void SetMoveTranslationZ(lua_State *l, ::Frame &frame, float z);
		DLLNETWORK void SetBoneScale(lua_State *l, ::Frame &frame, uint32_t boneId, const Vector3 &scale);
		DLLNETWORK void GetBoneScale(lua_State *l, ::Frame &frame, uint32_t boneId);
		DLLNETWORK void GetLocalBoneTransform(lua_State *, ::Frame &frame, pragma::animation::Skeleton &skeleton, uint32_t boneId);
		DLLNETWORK void GetBoneCount(lua_State *, ::Frame &frame);
		DLLNETWORK void SetBoneCount(lua_State *, ::Frame &frame, uint32_t boneCount);
		DLLNETWORK void SetBonePose(lua_State *, ::Frame &frame, uint32_t boneId, const umath::ScaledTransform &pose);
		DLLNETWORK void SetBonePose(lua_State *, ::Frame &frame, uint32_t boneId, const umath::Transform &pose);
		DLLNETWORK void GetBonePose(lua_State *, ::Frame &frame, uint32_t boneId);
	};
	namespace VertexAnimation {
		DLLNETWORK void GetMeshAnimations(lua_State *l, ::VertexAnimation &anim);
		DLLNETWORK void GetName(lua_State *l, ::VertexAnimation &anim);
	};
	namespace MeshVertexAnimation {
		DLLNETWORK void GetFrames(lua_State *l, ::MeshVertexAnimation &anim);
		DLLNETWORK void GetMesh(lua_State *l, ::MeshVertexAnimation &anim);
	};
	namespace MeshVertexFrame {
		DLLNETWORK void GetVertices(lua_State *l, ::pragma::MeshVertexFrame &frame);
		DLLNETWORK void SetVertexCount(lua_State *l, ::pragma::MeshVertexFrame &frame, uint32_t numVerts);
		DLLNETWORK void SetVertexPosition(lua_State *l, ::pragma::MeshVertexFrame &frame, uint32_t vertIdx, const Vector3 &pos);
		DLLNETWORK void GetVertexPosition(lua_State *l, ::pragma::MeshVertexFrame &frame, uint32_t vertIdx);
		DLLNETWORK void SetVertexNormal(lua_State *l, ::pragma::MeshVertexFrame &frame, uint32_t vertIdx, const Vector3 &n);
		DLLNETWORK void GetVertexNormal(lua_State *l, ::pragma::MeshVertexFrame &frame, uint32_t vertIdx);
	};
};
