/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/ldef_angle.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/lmodel.h"
#include "pragma/lua/classes/lanimation.h"
#include "pragma/lua/classes/lskeleton.h"
#include "pragma/lua/classes/lvector.h"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/physics/collisionmesh.h"
#include "pragma/lua/classes/lcollisionmesh.h"
#include "luasystem.h"
#include "pragma/model/model.h"
#include "pragma/asset/util_asset.hpp"
#include "pragma/physics/collisionmesh.h"
#include "pragma/physics/physsoftbodyinfo.hpp"
#include "pragma/model/animation/vertex_animation.hpp"
#include "pragma/model/animation/flex_animation.hpp"
#include "pragma/model/modelmesh.h"
#include "pragma/model/animation/meta_rig.hpp"
#include "pragma/lua/ostream_operator_alias.hpp"
#include <material_manager2.hpp>
#include <luabind/iterator_policy.hpp>
#include <pragma/lua/lua_call.hpp>
#include <pragma/lua/lua_util_class.hpp>
#include <sharedutils/util_path.hpp>
#include <sharedutils/util_file.h>
#include <mathutil/vertex.hpp>
#include <panima/animation.hpp>
#include "pragma/model/animation/skeleton.hpp"
#include "pragma/model/animation/bone.hpp"
#include <fsys/ifile.hpp>
#include <udm.hpp>

extern DLLNETWORK Engine *engine;

void Lua::ModelMeshGroup::register_class(luabind::class_<::ModelMeshGroup> &classDef)
{
	classDef.scope[luabind::def("Create", &Create)];
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def("GetName", &GetName);
	classDef.def("GetMeshes", &GetMeshes);
	classDef.def("AddMesh", &AddMesh);
	classDef.def("ClearMeshes", static_cast<void (*)(lua_State *, ::ModelMeshGroup &)>([](lua_State *l, ::ModelMeshGroup &meshGroup) { meshGroup.GetMeshes().clear(); }));
	classDef.def("SetMeshes", static_cast<void (*)(lua_State *, ::ModelMeshGroup &, luabind::object)>([](lua_State *l, ::ModelMeshGroup &meshGroup, luabind::object tMeshes) {
		auto idxMeshes = 2;
		Lua::CheckTable(l, idxMeshes);
		auto &meshes = meshGroup.GetMeshes();
		meshes = {};
		auto numMeshes = Lua::GetObjectLength(l, idxMeshes);
		meshes.reserve(idxMeshes);
		for(auto i = decltype(numMeshes) {0u}; i < numMeshes; ++i) {
			Lua::PushInt(l, i + 1);
			Lua::GetTableValue(l, idxMeshes);
			auto &subMesh = Lua::Check<std::shared_ptr<::ModelMesh>>(l, -1);
			meshes.push_back(subMesh);
			Lua::Pop(l, 1);
		}
	}));
	classDef.def("GetMesh", static_cast<void (*)(lua_State *, ::ModelMeshGroup &, uint32_t)>([](lua_State *l, ::ModelMeshGroup &meshGroup, uint32_t index) {
		auto &meshes = meshGroup.GetMeshes();
		if(index >= meshes.size())
			return;
		Lua::Push(l, meshes.at(index));
	}));
	classDef.def("GetMeshCount", static_cast<void (*)(lua_State *, ::ModelMeshGroup &, uint32_t)>([](lua_State *l, ::ModelMeshGroup &meshGroup, uint32_t index) { Lua::PushInt(l, meshGroup.GetMeshCount()); }));
}
void Lua::ModelMeshGroup::Create(lua_State *l, const std::string &name) { Lua::Push<std::shared_ptr<::ModelMeshGroup>>(l, ::ModelMeshGroup::Create(name)); }
void Lua::ModelMeshGroup::GetName(lua_State *l, ::ModelMeshGroup &meshGroup) { Lua::PushString(l, meshGroup.GetName()); }
void Lua::ModelMeshGroup::GetMeshes(lua_State *l, ::ModelMeshGroup &meshGroup)
{
	auto &meshes = meshGroup.GetMeshes();
	auto t = Lua::CreateTable(l);
	int32_t i = 1;
	for(auto &mesh : meshes) {
		Lua::PushInt(l, i);
		Lua::Push<std::shared_ptr<ModelMesh>>(l, mesh);
		Lua::SetTableValue(l, t);
		++i;
	}
}
void Lua::ModelMeshGroup::AddMesh(lua_State *, ::ModelMeshGroup &meshGroup, ModelMesh &mesh) { meshGroup.AddMesh(mesh.shared_from_this()); }

//////////////////////////

void Lua::Joint::GetType(lua_State *l, JointInfo &joint) { Lua::PushInt(l, joint.type); }
void Lua::Joint::GetChildBoneId(lua_State *l, JointInfo &joint) { Lua::PushInt(l, joint.child); }
void Lua::Joint::GetParentBoneId(lua_State *l, JointInfo &joint) { Lua::PushInt(l, joint.parent); }
void Lua::Joint::GetCollisionsEnabled(lua_State *l, JointInfo &joint) { Lua::PushBool(l, joint.collide); }
void Lua::Joint::GetKeyValues(lua_State *l, JointInfo &joint)
{
	auto t = Lua::CreateTable(l);
	for(auto &pair : joint.args) {
		Lua::PushString(l, pair.first);
		Lua::PushString(l, pair.second);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Joint::SetType(lua_State *l, JointInfo &joint, uint32_t type) { joint.type = static_cast<JointType>(type); }
void Lua::Joint::SetCollisionMeshId(lua_State *l, JointInfo &joint, uint32_t meshId) { joint.child = meshId; }
void Lua::Joint::SetParentCollisionMeshId(lua_State *l, JointInfo &joint, uint32_t meshId) { joint.parent = meshId; }
void Lua::Joint::SetCollisionsEnabled(lua_State *l, JointInfo &joint, bool bEnabled) { joint.collide = bEnabled; }
void Lua::Joint::SetKeyValues(lua_State *l, JointInfo &joint, luabind::object keyValues)
{
	Lua::CheckTable(l, 2);

	Lua::PushNil(l);
	joint.args.clear();
	while(Lua::GetNextPair(l, 2) != 0) {
		auto *key = Lua::CheckString(l, -2);
		auto *val = Lua::CheckString(l, -1);
		joint.args[key] = val;
		Lua::Pop(l, 1);
	}
}
void Lua::Joint::SetKeyValue(lua_State *l, JointInfo &joint, const std::string &key, const std::string &val) { joint.args[key] = val; }
void Lua::Joint::RemoveKeyValue(lua_State *l, JointInfo &joint, const std::string &key)
{
	auto it = joint.args.find(key);
	if(it == joint.args.end())
		return;
	joint.args.erase(it);
}

//////////////////////////

static std::ostream &operator<<(std::ostream &out, const Model &mdl)
{
	out << "Model";

	out << "[Name:" << mdl.GetName() << "]";
	out << "[MeshGroups:" << mdl.GetMeshGroups().size() << "]";
	out << "[SubMeshes:" << mdl.GetSubMeshCount() << "]";
	out << "[Verts:" << mdl.GetVertexCount() << "]";
	out << "[Tris:" << mdl.GetTriangleCount() << "]";
	out << "[Materials:" << mdl.GetMetaInfo().textures.size() << "]";
	out << "[CollisionMeshes:" << mdl.GetCollisionMeshes().size() << "]";
	out << "[Anims:" << mdl.GetAnimations().size() << "]";
	out << "[Flexes:" << mdl.GetFlexes().size() << "]";
	out << "[FlexControllers:" << mdl.GetFlexControllers().size() << "]";
	out << "[MorphTargetAnims:" << mdl.GetVertexAnimations().size() << "]";
	out << "[IkControllers:" << mdl.GetIKControllers().size() << "]";
	out << "[Bones:" << mdl.GetSkeleton().GetBones().size() << "]";
	out << "[Mass:" << mdl.GetMass() << "]";
	out << "[Attachments:" << mdl.GetAttachments().size() << "]";
	out << "[Hitboxes:" << mdl.GetHitboxCount() << "]";
	out << "[Eyeballs:" << mdl.GetEyeballs().size() << "]";
	return out;
}

namespace pragma::animation {
	std::ostream &operator<<(std::ostream &out, const pragma::animation::MetaRigBone &boneInfo)
	{
		out << "MetaRigBone";
		out << "[BoneId:" << boneInfo.boneId << "]";
		out << "[Min:" << boneInfo.bounds.first.x << ", " << boneInfo.bounds.first.y << ", " << boneInfo.bounds.first.z << "]";
		out << "[Max:" << boneInfo.bounds.second.x << ", " << boneInfo.bounds.second.y << ", " << boneInfo.bounds.second.z << "]";
		auto ang = EulerAngles {boneInfo.normalizedRotationOffset};
		out << "[Ang:" << ang.p << ", " << ang.y << "," << ang.r << "]";
		return out;
	}
	std::ostream &operator<<(std::ostream &out, const pragma::animation::MetaRigBlendShape &blendShapeInfo)
	{
		out << "MetaRigBlendShape";
		out << "[FlexControllerId:" << blendShapeInfo.flexControllerId << "]";
		return out;
	}
	std::ostream &operator<<(std::ostream &out, const pragma::animation::MetaRig &rig)
	{
		out << "MetaRig";
		return out;
	}
};

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(umath, Vertex);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(umath, VertexWeight);
#endif

template<typename TResult, typename TBoneIdentifier, bool (::Model::*GetValue)(pragma::animation::BoneId, TResult &, umath::CoordinateSpace) const>
std::optional<TResult> get_reference_bone_value(const ::Model &mdl, TBoneIdentifier boneIdentifier, umath::CoordinateSpace space)
{
	pragma::animation::BoneId boneId;
	if constexpr(std::is_same_v<TBoneIdentifier, const std::string &>)
		boneId = mdl.LookupBone(boneIdentifier);
	else
		boneId = boneIdentifier;
	TResult result;
	if(!(mdl.*GetValue)(boneId, result, space))
		return {};
	return result;
}

template<typename TResult, typename TBoneIdentifier, bool (::Model::*GetValue)(pragma::animation::BoneId, TResult &, umath::CoordinateSpace) const>
std::optional<TResult> get_reference_bone_value_ls(const ::Model &mdl, TBoneIdentifier boneIdentifier)
{
	return get_reference_bone_value<TResult, TBoneIdentifier, GetValue>(mdl, boneIdentifier, umath::CoordinateSpace::Local);
}

template<typename TResult>
void def_bone_methods(luabind::class_<::Model> &classDef)
{
	// Note: luabind::default_parameter_policy would be a better choice here, but doesn't work for the CoordinateSpace parameter for some unknown reason
	classDef.def("GetReferenceBonePose", &get_reference_bone_value_ls<umath::ScaledTransform, TResult, &::Model::GetReferenceBonePose>);
	classDef.def("GetReferenceBonePos", &get_reference_bone_value_ls<Vector3, TResult, &::Model::GetReferenceBonePos>);
	classDef.def("GetReferenceBoneRot", &get_reference_bone_value_ls<Quat, TResult, &::Model::GetReferenceBoneRot>);
	classDef.def("GetReferenceBoneScale", &get_reference_bone_value_ls<Vector3, TResult, &::Model::GetReferenceBoneScale>);

	classDef.def("GetReferenceBonePose", &get_reference_bone_value<umath::ScaledTransform, TResult, &::Model::GetReferenceBonePose>);
	classDef.def("GetReferenceBonePos", &get_reference_bone_value<Vector3, TResult, &::Model::GetReferenceBonePos>);
	classDef.def("GetReferenceBoneRot", &get_reference_bone_value<Quat, TResult, &::Model::GetReferenceBoneRot>);
	classDef.def("GetReferenceBoneScale", &get_reference_bone_value<Vector3, TResult, &::Model::GetReferenceBoneScale>);
}

void Lua::Model::register_class(lua_State *l, luabind::class_<::Model> &classDef, luabind::class_<::ModelMesh> &classDefModelMesh, luabind::class_<::ModelSubMesh> &classDefModelSubMesh)
{
	classDef.add_static_constant("INVALID_BONE_INDEX", pragma::animation::INVALID_BONE_INDEX);
	classDef.def(luabind::tostring(luabind::self));
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def("RemoveUnusedMaterialReferences", &::Model::RemoveUnusedMaterialReferences);
	classDef.def("GetCollisionMeshes", &GetCollisionMeshes);
	classDef.def("ClearCollisionMeshes", &ClearCollisionMeshes);
	classDef.def("GetSkeleton", &GetSkeleton);
	classDef.def("TransformBone", &::Model::TransformBone);
	classDef.def("TransformBone", &::Model::TransformBone, luabind::default_parameter_policy<4, umath::CoordinateSpace::World> {});
	classDef.def("GetAttachmentCount", &GetAttachmentCount);
	classDef.def("GetAttachments", &GetAttachments);
	classDef.def("GetAttachment", static_cast<void (*)(lua_State *, ::Model &, const std::string &)>(&GetAttachment));
	classDef.def("GetAttachment", static_cast<void (*)(lua_State *, ::Model &, int32_t)>(&GetAttachment));
	classDef.def("LookupAttachment", &LookupAttachment);
	classDef.def("LookupBone", &LookupBone);
	classDef.def("LookupAnimation", &LookupAnimation);
	classDef.def("AddAttachment", static_cast<void (*)(lua_State *, ::Model &, const std::string &, const std::string &, const Vector3 &, const EulerAngles &)>(&AddAttachment));
	classDef.def("AddAttachment", static_cast<void (*)(lua_State *, ::Model &, const std::string &, uint32_t, const Vector3 &, const EulerAngles &)>(&AddAttachment));
	classDef.def("SetAttachmentData", static_cast<void (*)(lua_State *, ::Model &, const std::string &, luabind::object)>(&SetAttachmentData));
	classDef.def("SetAttachmentData", static_cast<void (*)(lua_State *, ::Model &, uint32_t, luabind::object)>(&SetAttachmentData));
	classDef.def("RemoveAttachment", static_cast<void (*)(lua_State *, ::Model &, const std::string &)>(&RemoveAttachment));
	classDef.def("RemoveAttachment", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&RemoveAttachment));

	classDef.def("FindBoneTwistAxis", &::Model::FindBoneTwistAxis);
	classDef.def("FindBoneAxisForDirection", &::Model::FindBoneAxisForDirection);
	classDef.scope[luabind::def("get_twist_axis_rotation_offset", &::Model::GetTwistAxisRotationOffset)];

	classDef.def("GetObjectAttachments", &GetObjectAttachments);
	classDef.def("AddObjectAttachment", &AddObjectAttachment);
	classDef.def("GetObjectAttachmentCount", &GetObjectAttachmentCount);
	classDef.def("GetObjectAttachment", &GetObjectAttachment);
	classDef.def("LookupObjectAttachment", &LookupObjectAttachment);
	classDef.def("RemoveObjectAttachment", static_cast<void (*)(lua_State *, ::Model &, const std::string &)>(&RemoveObjectAttachment));
	classDef.def("RemoveObjectAttachment", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&RemoveObjectAttachment));

	classDef.def("GetBlendControllerCount", &GetBlendControllerCount);
	classDef.def("GetBlendControllers", &GetBlendControllers);
	classDef.def("GetBlendController", static_cast<void (*)(lua_State *, ::Model &, const std::string &)>(&GetBlendController));
	classDef.def("GetBlendController", static_cast<void (*)(lua_State *, ::Model &, int32_t)>(&GetBlendController));
	classDef.def("LookupBlendController", &LookupBlendController);
	classDef.def("GetAnimationCount", &GetAnimationCount);
	classDef.def("GetAnimations", &GetAnimations);
	classDef.def("GetAnimationNames", &GetAnimationNames);
	classDef.def("GetAnimation", static_cast<void (*)(lua_State *, ::Model &, const char *)>(&GetAnimation));
	classDef.def("GetAnimation", static_cast<void (*)(lua_State *, ::Model &, unsigned int)>(&GetAnimation));
	classDef.def("GetAnimationName", &GetAnimationName);

	def_bone_methods<pragma::animation::BoneId>(classDef);
	def_bone_methods<const std::string &>(classDef);

	classDef.def("CalcBoneLength", &::Model::CalcBoneLength);
	classDef.def("PrecacheTextureGroup", &PrecacheTextureGroup);
	classDef.def("PrecacheTextureGroups", &PrecacheTextureGroups);
	classDef.def("GetReferencePose", &GetReference);
	//classDef.def("GetReferenceBoneMatrix",&GetReferenceBoneMatrix);
	//classDef.def("SetReferenceBoneMatrix",&SetReferenceBoneMatrix);
	classDef.def("GetLocalBoneTransform", &GetLocalBonePosition);
	classDef.def("LookupBodyGroup", &LookupBodyGroup);
	classDef.def("GetBaseMeshGroupIds", &GetBaseMeshGroupIds);
	classDef.def("SetBaseMeshGroupIds", &SetBaseMeshGroupIds);
	classDef.def("AddBaseMeshGroupId", &AddBaseMeshGroupId);
	classDef.def("GetMeshGroupId", &GetMeshGroupId);
	classDef.def("GetMeshGroup", static_cast<void (*)(lua_State *, ::Model &, const std::string &)>(&GetMeshGroup));
	classDef.def("GetMeshGroup", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&GetMeshGroup));
	classDef.def("GetMeshes", static_cast<void (*)(lua_State *, ::Model &, const std::string &)>(&Lua::Model::GetMeshes));
	classDef.def("GetMeshes", static_cast<void (*)(lua_State *, ::Model &, luabind::object)>(&Lua::Model::GetMeshes));
	classDef.def("GetMeshGroups", &Lua::Model::GetMeshGroups);
	classDef.def("AddMeshGroup", static_cast<void (*)(lua_State *, ::Model &, const std::string &)>(&Lua::Model::AddMeshGroup));
	classDef.def("AddMeshGroup", static_cast<void (*)(lua_State *, ::Model &, ::ModelMeshGroup &)>(&Lua::Model::AddMeshGroup));
	classDef.def("UpdateCollisionBounds", &Lua::Model::UpdateCollisionBounds);
	classDef.def("UpdateRenderBounds", &Lua::Model::UpdateRenderBounds);
	classDef.def("Update", static_cast<void (*)(lua_State *, ::Model &)>(&Lua::Model::Update));
	classDef.def("Update", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&Lua::Model::Update));
	classDef.def("GetName", &Lua::Model::GetName);
	classDef.def("GetMass", &Lua::Model::GetMass);
	classDef.def("SetMass", &Lua::Model::SetMass);
	classDef.def("GetBoneCount", &Lua::Model::GetBoneCount);
	classDef.def("GetCollisionBounds", &Lua::Model::GetCollisionBounds);
	classDef.def("GetRenderBounds", &Lua::Model::GetRenderBounds);
	classDef.def("SetCollisionBounds", &Lua::Model::SetCollisionBounds);
	classDef.def("SetRenderBounds", &Lua::Model::SetRenderBounds);
	classDef.def("AddCollisionMesh", &Lua::Model::AddCollisionMesh);
	classDef.def("AddMaterial", &Lua::Model::AddMaterial);
	classDef.def("SetMaterial", &Lua::Model::SetMaterial);
	classDef.def("GetMaterials", &Lua::Model::GetMaterials);
	classDef.def("GetMaterial", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t idx) {
		auto *mat = mdl.GetMaterial(idx);
		if(mat == nullptr)
			return;
		Lua::Push<Material *>(l, mat);
	}));
	classDef.def("GetMaterialIndex", static_cast<void (*)(lua_State *, ::Model &, ::ModelSubMesh &, uint32_t)>([](lua_State *l, ::Model &mdl, ::ModelSubMesh &mesh, uint32_t skinId) {
		auto idx = mdl.GetMaterialIndex(mesh, skinId);
		if(idx.has_value() == false)
			return;
		Lua::PushInt(l, *idx);
	}));
	classDef.def(
	  "GetSubMesh", +[](lua_State *l, ::Model &mdl, uint32_t mgId, uint32_t mId, uint32_t smId) -> std::shared_ptr<ModelSubMesh> {
		  auto *sm = mdl.GetSubMesh(mgId, mId, smId);
		  if(!sm)
			  return nullptr;
		  return sm->shared_from_this();
	  });
	classDef.def(
	  "FindSubMeshId", +[](lua_State *l, ::Model &mdl, ModelSubMesh &smTgt) -> std::optional<std::tuple<uint32_t, uint32_t, uint32_t>> {
		  auto &meshGroups = mdl.GetMeshGroups();
		  for(uint32_t mgId = 0; auto &mg : meshGroups) {
			  for(uint32_t mId = 0; auto &m : mg->GetMeshes()) {
				  for(uint32_t smId = 0; auto &sm : m->GetSubMeshes()) {
					  if(sm.get() == &smTgt)
						  return std::tuple<uint32_t, uint32_t, uint32_t> {mgId, mId, smId};
					  ++smId;
				  }
				  ++mId;
			  }
			  ++mgId;
		  }
		  return {};
	  });
	classDef.def("GetMaterialCount", &Lua::Model::GetMaterialCount);
	classDef.def("GetMeshGroupCount", &Lua::Model::GetMeshGroupCount);
	classDef.def("GetMeshCount", &Lua::Model::GetMeshCount);
	classDef.def("GetSubMeshCount", &Lua::Model::GetSubMeshCount);
	classDef.def("GetCollisionMeshCount", &Lua::Model::GetCollisionMeshCount);
	classDef.def("GetBodyGroupId", &Lua::Model::GetBodyGroupId);
	classDef.def("GetBodyGroupCount", &Lua::Model::GetBodyGroupCount);
	classDef.def("AddHitbox", &Lua::Model::AddHitbox);
	classDef.def("GetHitboxCount", &Lua::Model::GetHitboxCount);
	classDef.def("GetHitboxGroup", &Lua::Model::GetHitboxGroup);
	classDef.def("GetHitboxBounds", &Lua::Model::GetHitboxBounds);
	classDef.def("GetHitboxBones", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&Lua::Model::GetHitboxBones));
	classDef.def("GetHitboxBones", static_cast<void (*)(lua_State *, ::Model &)>(&Lua::Model::GetHitboxBones));
	classDef.def("SetHitboxGroup", &Lua::Model::SetHitboxGroup);
	classDef.def("SetHitboxBounds", &Lua::Model::SetHitboxBounds);
	classDef.def("RemoveHitbox", &Lua::Model::RemoveHitbox);
	classDef.def("GetBodyGroups", &Lua::Model::GetBodyGroups);
	classDef.def("GetBodyGroup", &Lua::Model::GetBodyGroup);
	classDef.def("GetExtensionData", &::Model::GetExtensionData);
	classDef.def("GenerateHitboxes", &::Model::GenerateHitboxes);
	classDef.def("GenerateMetaRig", &::Model::GenerateMetaRig);
	classDef.def("GenerateMetaBlendShapes", &::Model::GenerateMetaBlendShapes);
	classDef.def("GetMetaRig", &::Model::GetMetaRig);
	classDef.def("ClearMetaRig", &::Model::ClearMetaRig);
	classDef.def("GetMetaRigReferencePose", &::Model::GetMetaRigReferencePose);
	classDef.def(
	  "GenerateStandardMetaRigReferenceBonePoses", +[](const ::Model &mdl) -> std::optional<std::vector<umath::ScaledTransform>> {
		  std::vector<umath::ScaledTransform> poses;
		  if(!mdl.GenerateStandardMetaRigReferenceBonePoses(poses))
			  return {};
		  return poses;
	  });
	classDef.def("GetMetaRigBoneParentId", &::Model::GetMetaRigBoneParentId);
	classDef.def("GetTextureGroupCount", &Lua::Model::GetTextureGroupCount);
	classDef.def("GetTextureGroups", &Lua::Model::GetTextureGroups);
	classDef.def("GetTextureGroup", &Lua::Model::GetTextureGroup);
	classDef.def("SaveLegacy", &Lua::Model::Save);
	classDef.def("Save", static_cast<void (*)(lua_State *, ::Model &, udm::AssetData &)>([](lua_State *l, ::Model &mdl, udm::AssetData &assetData) {
		auto *nw = engine->GetNetworkState(l);
		auto *game = nw ? nw->GetGameState() : nullptr;
		if(game == nullptr)
			return;
		std::string err;
		auto result = mdl.Save(*game, assetData, err);
		if(result == false)
			Lua::PushString(l, err);
		else
			Lua::PushBool(l, result);
	}));
	classDef.def("Save", static_cast<void (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) {
		auto *nw = engine->GetNetworkState(l);
		auto *game = nw ? nw->GetGameState() : nullptr;
		if(game == nullptr)
			return;
		std::string err;
		auto result = mdl.Save(*game, err);
		if(result == false)
			Lua::PushString(l, err);
		else
			Lua::PushBool(l, result);
	}));
	classDef.def(
	  "Save", +[](lua_State *l, ::Engine *engine, ::Model &mdl, const std::string &fname) {
		  auto *nw = engine->GetNetworkState(l);
		  auto *game = nw ? nw->GetGameState() : nullptr;
		  if(game == nullptr)
			  return;
		  std::string err;
		  auto result = mdl.Save(*game, fname, err);
		  if(result == false)
			  Lua::PushString(l, err);
		  else {
			  engine->PollResourceWatchers();
			  Lua::PushBool(l, result);
		  }
	  });
	classDef.def("Copy", static_cast<void (*)(lua_State *, ::Model &)>(&Lua::Model::Copy));
	classDef.def("Copy", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&Lua::Model::Copy));
	classDef.def("GetVertexCount", &Lua::Model::GetVertexCount);
	classDef.def("GetTriangleCount", &Lua::Model::GetTriangleCount);
	classDef.def("GetMaterialNames", &Lua::Model::GetTextures);
	classDef.def("GetMaterialPaths", &Lua::Model::GetTexturePaths);
	classDef.def("LoadMaterials", static_cast<void (*)(lua_State *, ::Model &)>(&Lua::Model::LoadMaterials));
	classDef.def("LoadMaterials", static_cast<void (*)(lua_State *, ::Model &, bool)>(&Lua::Model::LoadMaterials));
	classDef.def("AddMaterialPath", &Lua::Model::AddTexturePath);
	classDef.def("RemoveMaterialPath", &Lua::Model::RemoveTexturePath);
	classDef.def("SetMaterialPaths", &Lua::Model::SetTexturePaths);
	classDef.def("RemoveMaterial", &Lua::Model::RemoveTexture);
	classDef.def("ClearMaterials", &Lua::Model::ClearTextures);
	classDef.def("Rotate", &Lua::Model::Rotate);
	classDef.def("Translate", &Lua::Model::Translate);
	classDef.def("Scale", &Lua::Model::Scale);
	classDef.def("GetEyeOffset", &Lua::Model::GetEyeOffset);
	classDef.def("SetEyeOffset", &Lua::Model::SetEyeOffset);
	classDef.def("AddAnimation", &Lua::Model::AddAnimation);
	classDef.def("RemoveAnimation", &Lua::Model::RemoveAnimation);
	classDef.def("ClearAnimations", &Lua::Model::ClearAnimations);
	//classDef.def("ClipAgainstPlane",&Lua::Model::ClipAgainstPlane);
	classDef.def("ClearMeshGroups", &Lua::Model::ClearMeshGroups);
	classDef.def("RemoveMeshGroup", &Lua::Model::RemoveMeshGroup);
	classDef.def("ClearBaseMeshGroupIds", &Lua::Model::ClearBaseMeshGroupIds);
	classDef.def("AddTextureGroup", &Lua::Model::AddTextureGroup);
	classDef.def("Merge", static_cast<void (*)(lua_State *, ::Model &, ::Model &, uint32_t)>(&Lua::Model::Merge));
	classDef.def("Merge", static_cast<void (*)(lua_State *, ::Model &, ::Model &)>(&Lua::Model::Merge));
	classDef.def("GetLODCount", &Lua::Model::GetLODCount);
	classDef.def("GetLODData", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&Lua::Model::GetLODData));
	classDef.def("GetLODData", static_cast<void (*)(lua_State *, ::Model &)>(&Lua::Model::GetLODData));
	classDef.def("GetLOD", &Lua::Model::GetLOD);
	classDef.def("GenerateLowLevelLODs", &::Model::GenerateLowLevelLODs);
	classDef.def("TranslateLODMeshes", static_cast<void (*)(lua_State *, ::Model &, uint32_t, luabind::object)>(&Lua::Model::TranslateLODMeshes));
	classDef.def("TranslateLODMeshes", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&Lua::Model::TranslateLODMeshes));
	classDef.def("GetJoints", &Lua::Model::GetJoints);
	classDef.def("SetReferencePoses", &::Model::SetReferencePoses);
	classDef.def("SetReferencePoses", &::Model::SetReferencePoses, luabind::default_parameter_policy<3, false> {});
	classDef.def("AddJoint", static_cast<JointInfo *(*)(lua_State *, ::Model &, JointType, pragma::animation::BoneId, pragma::animation::BoneId)>([](lua_State *l, ::Model &mdl, JointType type, pragma::animation::BoneId child, pragma::animation::BoneId parent) -> JointInfo * {
		auto &joint = mdl.AddJoint(type, child, parent);
		return &joint;
	}));
	classDef.def("ClearJoints", static_cast<void (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) { mdl.GetJoints().clear(); }));
	classDef.def("GetVertexAnimations", &Lua::Model::GetVertexAnimations);
	classDef.def("GetVertexAnimation", &Lua::Model::GetVertexAnimation);
	classDef.def("AddVertexAnimation", &Lua::Model::AddVertexAnimation);
	classDef.def("RemoveVertexAnimation", &Lua::Model::RemoveVertexAnimation);
	classDef.def("GetBodyGroupMeshes", static_cast<void (*)(lua_State *, ::Model &, luabind::object, uint32_t)>(&Lua::Model::GetBodyGroupMeshes));
	classDef.def("GetBodyGroupMeshes", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&Lua::Model::GetBodyGroupMeshes));
	classDef.def("GetBodyGroupMeshes", static_cast<void (*)(lua_State *, ::Model &)>(&Lua::Model::GetBodyGroupMeshes));
	classDef.def("GetBodyGroupMesh", static_cast<void (*)(lua_State *, ::Model &, uint32_t, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t bodyGroupId, uint32_t bgValue) {
		uint32_t meshId;
		if(mdl.GetMesh(bodyGroupId, bgValue, meshId) == false)
			return;
		Lua::PushInt(l, meshId);
	}));
	classDef.def("GetFlexControllers", &Lua::Model::GetFlexControllers);
	classDef.def("LookupFlexController", &Lua::Model::GetFlexControllerId);
	classDef.def("GetFlexController", static_cast<void (*)(lua_State *, ::Model &, const std::string &)>(&Lua::Model::GetFlexController));
	classDef.def("GetFlexController", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&Lua::Model::GetFlexController));
	classDef.def(
	  "AddFlexController", +[](::Model &mdl, const std::string &name, float min, float max) -> pragma::animation::FlexControllerId {
		  uint32_t id;
		  if(mdl.GetFlexControllerId(name, id))
			  return id;
		  mdl.GetFlexControllers().push_back({name, min, max});
		  return mdl.GetFlexControllers().size() - 1;
	  });
	classDef.def("GetFlexes", &Lua::Model::GetFlexes);
	classDef.def(
	  "AddFlex", +[](::Model &mdl, const ::Flex &flex) { mdl.GetFlexes().push_back(flex); });
	classDef.def("LookupFlex", &Lua::Model::GetFlexId);
	classDef.def("GetFlexFormula", static_cast<void (*)(lua_State *, ::Model &, const std::string &)>(&Lua::Model::GetFlexFormula));
	classDef.def("GetFlexFormula", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&Lua::Model::GetFlexFormula));
	classDef.def("GetFlexControllerCount", static_cast<void (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) { Lua::PushInt(l, mdl.GetFlexControllerCount()); }));
	classDef.def("GetFlexCount", static_cast<void (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) { Lua::PushInt(l, mdl.GetFlexCount()); }));
	classDef.def("CalcFlexWeight", static_cast<void (*)(lua_State *, ::Model &, uint32_t, luabind::object)>([](lua_State *l, ::Model &mdl, uint32_t flexId, luabind::object oFc) {
		Lua::CheckFunction(l, 3);
		auto weight = mdl.CalcFlexWeight(
		  flexId,
		  [&oFc, l](uint32_t fcId) -> std::optional<float> {
			  auto result = Lua::CallFunction(
			    l,
			    [oFc, fcId](lua_State *l) -> Lua::StatusCode {
				    oFc.push(l);
				    Lua::PushInt(l, fcId);
				    return Lua::StatusCode::Ok;
			    },
			    1);
			  if(result != Lua::StatusCode::Ok)
				  return std::optional<float> {};
			  if(Lua::IsSet(l, -1) == false)
				  return std::optional<float> {};
			  return Lua::CheckNumber(l, -1);
		  },
		  [](uint32_t fcId) -> std::optional<float> { return std::optional<float> {}; });
		if(weight.has_value() == false)
			return;
		Lua::PushNumber(l, *weight);
	}));
	classDef.def("CalcReferenceAttachmentPose", static_cast<void (*)(lua_State *, ::Model &, int32_t)>([](lua_State *l, ::Model &mdl, int32_t attIdx) {
		auto t = mdl.CalcReferenceAttachmentPose(attIdx);
		if(t.has_value() == false)
			return;
		Lua::Push<umath::ScaledTransform>(l, *t);
	}));
	classDef.def("CalcReferenceBonePose", static_cast<void (*)(lua_State *, ::Model &, int32_t)>([](lua_State *l, ::Model &mdl, int32_t boneIdx) {
		auto t = mdl.CalcReferenceBonePose(boneIdx);
		if(t.has_value() == false)
			return;
		Lua::Push<umath::ScaledTransform>(l, *t);
	}));
	classDef.def("IsRootBone", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t boneId) { Lua::PushBool(l, mdl.IsRootBone(boneId)); }));
	classDef.def("GetFlags", static_cast<void (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) { Lua::PushInt(l, mdl.GetMetaInfo().flags); }));
	classDef.def("SetFlags", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t flags) { mdl.GetMetaInfo().flags = static_cast<::Model::Flags>(flags); }));
	classDef.def("HasFlag", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t flag) { Lua::PushBool(l, umath::is_flag_set(mdl.GetMetaInfo().flags, static_cast<::Model::Flags>(flag)) != 0); }));
	classDef.def("IsStatic", static_cast<void (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) { Lua::PushBool(l, umath::is_flag_set(mdl.GetMetaInfo().flags, ::Model::Flags::Static)); }));
	classDef.def(
	  "GetEyeball", +[](lua_State *l, ::Model &mdl, uint32_t eyeIdx) {
		  auto *eyeball = mdl.GetEyeball(eyeIdx);
		  if(!eyeball)
			  return;
		  Lua::Push<Eyeball *>(l, eyeball);
	  });
	classDef.def("GetEyeballs", static_cast<void (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) {
		auto &eyeballs = mdl.GetEyeballs();
		auto t = Lua::CreateTable(l);
		for(auto i = decltype(eyeballs.size()) {0u}; i < eyeballs.size(); ++i) {
			auto &eyeball = eyeballs.at(i);
			Lua::PushInt(l, i + 1);
			Lua::Push<Eyeball *>(l, &eyeball);
			Lua::SetTableValue(l, t);
		}
	}));
	classDef.def("GetEyeballCount", static_cast<void (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) { Lua::PushInt(l, mdl.GetEyeballCount()); }));
	classDef.def("IsStatic", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t eyeballIndex) {
		auto *eyeball = mdl.GetEyeball(eyeballIndex);
		if(eyeball == nullptr)
			return;
		Lua::Push<Eyeball *>(l, eyeball);
	}));
	classDef.def("AddEyeball", static_cast<void (*)(lua_State *, ::Model &, Eyeball &)>([](lua_State *l, ::Model &mdl, Eyeball &eyeball) { mdl.AddEyeball(eyeball); }));

	classDef.def("GetIKControllers", &Lua::Model::GetIKControllers);
	classDef.def("GetIKController", &Lua::Model::GetIKController);
	classDef.def("LookupIKController", &Lua::Model::LookupIKController);
	classDef.def("AddIKController", static_cast<void (*)(lua_State *, ::Model &, const std::string &, uint32_t, const std::string &, uint32_t)>(&Lua::Model::AddIKController));
	classDef.def("AddIKController", static_cast<void (*)(lua_State *, ::Model &, const std::string &, uint32_t, const std::string &)>(&Lua::Model::AddIKController));
	classDef.def("RemoveIKController", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>(&Lua::Model::RemoveIKController));
	classDef.def("RemoveIKController", static_cast<void (*)(lua_State *, ::Model &, const std::string &)>(&Lua::Model::RemoveIKController));

	classDef.def("AddIncludeModel", &Lua::Model::AddIncludeModel);
	classDef.def("GetIncludeModels", &Lua::Model::GetIncludeModels);

	classDef.def("GetPhonemeMap", &Lua::Model::GetPhonemeMap);
	classDef.def("SetPhonemeMap", &Lua::Model::SetPhonemeMap);
	classDef.def(
	  "GetPhonemes", +[](const ::Model &mdl) -> std::vector<std::string> {
		  auto &phonemeMap = mdl.GetPhonemeMap();
		  std::vector<std::string> phonemes;
		  phonemes.reserve(phonemeMap.phonemes.size());
		  for(auto &[name, info] : phonemeMap.phonemes)
			  phonemes.push_back(name);
		  return phonemes;
	  });

	classDef.def("AssignDistinctMaterial", &Lua::Model::AssignDistinctMaterial);

	classDef.def("SelectWeightedAnimation", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t activity) { Lua::PushInt(l, mdl.SelectWeightedAnimation(static_cast<Activity>(activity))); }));
	classDef.def("SelectWeightedAnimation", static_cast<void (*)(lua_State *, ::Model &, uint32_t, int32_t)>([](lua_State *l, ::Model &mdl, uint32_t activity, int32_t animIgnore) { Lua::PushInt(l, mdl.SelectWeightedAnimation(static_cast<Activity>(activity), animIgnore)); }));
	classDef.def("SelectFirstAnimation", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t activity) { Lua::PushInt(l, mdl.SelectFirstAnimation(static_cast<Activity>(activity))); }));
	classDef.def("GetAnimationActivityWeight", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t anim) { Lua::PushInt(l, mdl.GetAnimationActivityWeight(anim)); }));
	classDef.def("GetAnimationActivity", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t anim) { Lua::PushInt(l, umath::to_integral(mdl.GetAnimationActivity(anim))); }));
	classDef.def("GetAnimationDuration", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t anim) { Lua::PushNumber(l, mdl.GetAnimationDuration(anim)); }));
	classDef.def("HasVertexWeights", static_cast<void (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) { Lua::PushBool(l, mdl.HasVertexWeights()); }));

	// Flex animations
	classDef.def("GetFlexAnimations", static_cast<luabind::object (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) -> luabind::object { return Lua::vector_to_table(l, mdl.GetFlexAnimations()); }));
	classDef.def("GetFlexAnimationNames", static_cast<luabind::object (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) -> luabind::object { return Lua::vector_to_table(l, mdl.GetFlexAnimationNames()); }));
	classDef.def("GetFlexAnimationCount", static_cast<uint32_t (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) -> uint32_t { return mdl.GetFlexAnimations().size(); }));
	classDef.def("AddFlexAnimation", static_cast<std::shared_ptr<FlexAnimation> (*)(lua_State *, ::Model &, const std::string &)>([](lua_State *l, ::Model &mdl, const std::string &name) -> std::shared_ptr<FlexAnimation> {
		auto anim = std::make_shared<FlexAnimation>();
		mdl.AddFlexAnimation(name, *anim);
		return anim;
	}));
	classDef.def("AddFlexAnimation", static_cast<void (*)(lua_State *, ::Model &, const std::string &, FlexAnimation &)>([](lua_State *l, ::Model &mdl, const std::string &name, FlexAnimation &flexAnim) { mdl.AddFlexAnimation(name, flexAnim); }));
	classDef.def("LookupFlexAnimation", static_cast<luabind::object (*)(lua_State *, ::Model &, const std::string &)>([](lua_State *l, ::Model &mdl, const std::string &name) -> luabind::object {
		auto id = mdl.LookupFlexAnimation(name);
		if(id.has_value() == false)
			return {};
		return luabind::object {l, *id};
	}));
	classDef.def("GetFlexAnimation", static_cast<luabind::object (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t idx) -> luabind::object {
		auto *flexAnim = mdl.GetFlexAnimation(idx);
		if(flexAnim == nullptr)
			return {};
		return luabind::object {l, flexAnim->shared_from_this()};
	}));
	classDef.def("GetFlexAnimationName", static_cast<luabind::object (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t idx) -> luabind::object {
		auto *name = mdl.GetFlexAnimationName(idx);
		if(name == nullptr)
			return {};
		return luabind::object {l, *name};
	}));
	classDef.def("ClearFlexAnimations", static_cast<void (*)(lua_State *, ::Model &)>([](lua_State *l, ::Model &mdl) { mdl.GetFlexAnimations().clear(); }));
	classDef.def("RemoveFlexAnimation", static_cast<void (*)(lua_State *, ::Model &, uint32_t)>([](lua_State *l, ::Model &mdl, uint32_t idx) {
		auto &flexAnims = mdl.GetFlexAnimations();
		if(idx >= flexAnims.size())
			return;
		flexAnims.erase(flexAnims.begin() + idx);
	}));

	classDef.add_static_constant("FLAG_NONE", umath::to_integral(::Model::Flags::None));
	classDef.add_static_constant("FLAG_BIT_STATIC", umath::to_integral(::Model::Flags::Static));
	classDef.add_static_constant("FLAG_BIT_INANIMATE", umath::to_integral(::Model::Flags::Inanimate));
	classDef.add_static_constant("FLAG_BIT_DONT_PRECACHE_TEXTURE_GROUPS", umath::to_integral(::Model::Flags::DontPrecacheTextureGroups));

	classDef.add_static_constant("FMERGE_NONE", umath::to_integral(::Model::MergeFlags::None));
	classDef.add_static_constant("FMERGE_ANIMATIONS", umath::to_integral(::Model::MergeFlags::Animations));
	classDef.add_static_constant("FMERGE_ATTACHMENTS", umath::to_integral(::Model::MergeFlags::Attachments));
	classDef.add_static_constant("FMERGE_BLEND_CONTROLLERS", umath::to_integral(::Model::MergeFlags::BlendControllers));
	classDef.add_static_constant("FMERGE_HITBOXES", umath::to_integral(::Model::MergeFlags::Hitboxes));
	classDef.add_static_constant("FMERGE_JOINTS", umath::to_integral(::Model::MergeFlags::Joints));
	classDef.add_static_constant("FMERGE_COLLISION_MESHES", umath::to_integral(::Model::MergeFlags::CollisionMeshes));
	classDef.add_static_constant("FMERGE_MESHES", umath::to_integral(::Model::MergeFlags::Meshes));
	classDef.add_static_constant("FMERGE_ALL", umath::to_integral(::Model::MergeFlags::All));

	classDef.add_static_constant("FCOPY_NONE", umath::to_integral(::Model::CopyFlags::None));
	classDef.add_static_constant("FCOPY_SHALLOW", umath::to_integral(::Model::CopyFlags::ShallowCopy));
	classDef.add_static_constant("FCOPY_BIT_MESHES", umath::to_integral(::Model::CopyFlags::CopyMeshesBit));
	classDef.add_static_constant("FCOPY_BIT_ANIMATIONS", umath::to_integral(::Model::CopyFlags::CopyAnimationsBit));
	classDef.add_static_constant("FCOPY_BIT_VERTEX_ANIMATIONS", umath::to_integral(::Model::CopyFlags::CopyVertexAnimationsBit));
	classDef.add_static_constant("FCOPY_BIT_COLLISION_MESHES", umath::to_integral(::Model::CopyFlags::CopyCollisionMeshesBit));
	classDef.add_static_constant("FCOPY_BIT_FLEX_ANIMATIONS", umath::to_integral(::Model::CopyFlags::CopyFlexAnimationsBit));
	classDef.add_static_constant("FCOPY_BIT_COPY_UNIQUE_IDS", umath::to_integral(::Model::CopyFlags::CopyUniqueIdsBit));
	classDef.add_static_constant("FCOPY_BIT_COPY_VERTEX_DATA", umath::to_integral(::Model::CopyFlags::CopyVertexData));
	classDef.add_static_constant("FCOPY_DEEP", umath::to_integral(::Model::CopyFlags::DeepCopy));

	classDef.add_static_constant("FUPDATE_NONE", umath::to_integral(ModelUpdateFlags::None));
	classDef.add_static_constant("FUPDATE_BOUNDS", umath::to_integral(ModelUpdateFlags::UpdateBounds));
	classDef.add_static_constant("FUPDATE_PRIMITIVE_COUNTS", umath::to_integral(ModelUpdateFlags::UpdatePrimitiveCounts));
	classDef.add_static_constant("FUPDATE_COLLISION_SHAPES", umath::to_integral(ModelUpdateFlags::UpdateCollisionShapes));
	classDef.add_static_constant("FUPDATE_TANGENTS", umath::to_integral(ModelUpdateFlags::UpdateTangents));
	classDef.add_static_constant("FUPDATE_VERTEX_BUFFER", umath::to_integral(ModelUpdateFlags::UpdateVertexBuffer));
	classDef.add_static_constant("FUPDATE_INDEX_BUFFER", umath::to_integral(ModelUpdateFlags::UpdateIndexBuffer));
	classDef.add_static_constant("FUPDATE_WEIGHT_BUFFER", umath::to_integral(ModelUpdateFlags::UpdateWeightBuffer));
	classDef.add_static_constant("FUPDATE_ALPHA_BUFFER", umath::to_integral(ModelUpdateFlags::UpdateAlphaBuffer));
	classDef.add_static_constant("FUPDATE_VERTEX_ANIMATION_BUFFER", umath::to_integral(ModelUpdateFlags::UpdateVertexAnimationBuffer));
	classDef.add_static_constant("FUPDATE_CHILDREN", umath::to_integral(ModelUpdateFlags::UpdateChildren));
	classDef.add_static_constant("FUPDATE_BUFFERS", umath::to_integral(ModelUpdateFlags::UpdateBuffers));
	classDef.add_static_constant("FUPDATE_ALL", umath::to_integral(ModelUpdateFlags::All));
	classDef.add_static_constant("FUPDATE_ALL_DATA", umath::to_integral(ModelUpdateFlags::AllData));

	classDef.add_static_constant("OBJECT_ATTACHMENT_TYPE_MODEL", umath::to_integral(ObjectAttachment::Type::Model));
	classDef.add_static_constant("OBJECT_ATTACHMENT_TYPE_PARTICLE_SYSTEM", umath::to_integral(ObjectAttachment::Type::ParticleSystem));

	classDef.scope[luabind::def(
	  "Load", +[](lua_State *l, ::Game &game, udm::AssetData &assetData) -> Lua::var<::ModelSubMesh, std::pair<bool, std::string>> {
		  auto mdl = game.CreateModel(false);
		  std::string err;
		  auto result = ::Model::Load(*mdl, *game.GetNetworkState(), assetData, err);
		  if(result)
			  return luabind::object {l, mdl};
		  return luabind::object {l, std::pair<bool, std::string> {false, err}};
	  })];

	// Eyeball
	auto classDefEyeball = luabind::class_<::Eyeball>("Eyeball");
	classDefEyeball.def(luabind::constructor<>());
	classDefEyeball.def_readwrite("name", &::Eyeball::name);
	classDefEyeball.def_readwrite("boneIndex", &::Eyeball::boneIndex);
	classDefEyeball.def_readwrite("origin", &::Eyeball::origin);
	classDefEyeball.def_readwrite("zOffset", &::Eyeball::zOffset);
	classDefEyeball.def_readwrite("radius", &::Eyeball::radius);
	classDefEyeball.def_readwrite("up", &::Eyeball::up);
	classDefEyeball.def_readwrite("forward", &::Eyeball::forward);
	classDefEyeball.def_readwrite("irisMaterialIndex", &::Eyeball::irisMaterialIndex);
	classDefEyeball.def_readwrite("maxDilationFactor", &::Eyeball::maxDilationFactor);
	classDefEyeball.def_readwrite("irisUvRadius", &::Eyeball::irisUvRadius);
	classDefEyeball.def_readwrite("irisScale", &::Eyeball::irisScale);
	classDefEyeball.def("GetUpperLidFlexIndices", static_cast<luabind::object (*)(lua_State *, Eyeball &)>([](lua_State *l, Eyeball &eyeball) -> luabind::object {
		auto t = luabind::newtable(l);
		t[1] = eyeball.upperLid.raiserFlexIndex;
		t[2] = eyeball.upperLid.neutralFlexIndex;
		t[3] = eyeball.upperLid.lowererFlexIndex;
		return t;
	}));
	classDefEyeball.def("GetUpperLidFlexAngles", static_cast<luabind::object (*)(lua_State *, Eyeball &)>([](lua_State *l, Eyeball &eyeball) -> luabind::object {
		auto t = luabind::newtable(l);
		t[1] = eyeball.upperLid.raiserValue;
		t[2] = eyeball.upperLid.neutralValue;
		t[3] = eyeball.upperLid.lowererValue;
		return t;
	}));
	classDefEyeball.def("GetLowerLidFlexIndices", static_cast<luabind::object (*)(lua_State *, Eyeball &)>([](lua_State *l, Eyeball &eyeball) -> luabind::object {
		auto t = luabind::newtable(l);
		t[1] = eyeball.lowerLid.raiserFlexIndex;
		t[2] = eyeball.lowerLid.neutralFlexIndex;
		t[3] = eyeball.lowerLid.lowererFlexIndex;
		return t;
	}));
	classDefEyeball.def("GetLowerLidFlexAngles", static_cast<luabind::object (*)(lua_State *, Eyeball &)>([](lua_State *l, Eyeball &eyeball) -> luabind::object {
		auto t = luabind::newtable(l);
		t[1] = eyeball.lowerLid.raiserValue;
		t[2] = eyeball.lowerLid.neutralValue;
		t[3] = eyeball.lowerLid.lowererValue;
		return t;
	}));
	classDefEyeball.def("GetUpperLidFlexIndex", static_cast<int32_t (*)(lua_State *, Eyeball &)>([](lua_State *l, Eyeball &eyeball) -> int32_t { return eyeball.upperLid.lidFlexIndex; }));
	classDefEyeball.def("GetLowerLidFlexIndex", static_cast<int32_t (*)(lua_State *, Eyeball &)>([](lua_State *l, Eyeball &eyeball) -> int32_t { return eyeball.lowerLid.lidFlexIndex; }));
	classDef.scope[classDefEyeball];

	// Flex
	auto classDefFlex = luabind::class_<::Flex>("Flex");
	classDefFlex.def(luabind::constructor<>());
	classDefFlex.def(
	  "__tostring", +[](const ::Flex &flex) -> std::string {
		  std::stringstream ss;
		  ss << "Flex";
		  ss << "[" << flex.GetName() << "]";
		  return ss.str();
	  });
	classDefFlex.def(
	  "SetName", +[](::Flex &flex, const std::string &name) { flex.SetName(name); });
	classDefFlex.def("GetName", static_cast<void (*)(lua_State *, ::Flex &)>([](lua_State *l, ::Flex &flex) { Lua::PushString(l, flex.GetName()); }));
	classDefFlex.def(
	  "AddOperation", +[](::Flex &flex, const ::Flex::Operation &op) { flex.GetOperations().push_back(op); });
	classDefFlex.def("GetOperations", static_cast<void (*)(lua_State *, ::Flex &)>([](lua_State *l, ::Flex &flex) {
		auto t = Lua::CreateTable(l);
		auto &ops = flex.GetOperations();
		for(auto i = decltype(ops.size()) {0u}; i < ops.size(); ++i) {
			Lua::PushInt(l, i + 1u);
			Lua::Push<::Flex::Operation *>(l, &ops.at(i));
			Lua::SetTableValue(l, t);
		}
	}));
	classDefFlex.def("GetFrameIndex", static_cast<void (*)(lua_State *, ::Flex &)>([](lua_State *l, ::Flex &flex) { Lua::PushInt(l, flex.GetFrameIndex()); }));
	classDefFlex.def("GetVertexAnimation", static_cast<void (*)(lua_State *, ::Flex &)>([](lua_State *l, ::Flex &flex) {
		auto anim = flex.GetVertexAnimation() ? flex.GetVertexAnimation()->shared_from_this() : nullptr;
		if(anim == nullptr)
			return;
		Lua::Push<std::shared_ptr<::VertexAnimation>>(l, anim);
	}));
	classDefFlex.def("SetVertexAnimation", static_cast<void (*)(lua_State *, ::Flex &, ::VertexAnimation &, uint32_t)>([](lua_State *l, ::Flex &flex, ::VertexAnimation &anim, uint32_t frameIndex) { flex.SetVertexAnimation(anim, frameIndex); }));
	classDefFlex.def("SetVertexAnimation", static_cast<void (*)(lua_State *, ::Flex &, ::VertexAnimation &)>([](lua_State *l, ::Flex &flex, ::VertexAnimation &anim) { flex.SetVertexAnimation(anim); }));
	classDefFlex.add_static_constant("OP_NONE", umath::to_integral(::Flex::Operation::Type::None));
	classDefFlex.add_static_constant("OP_CONST", umath::to_integral(::Flex::Operation::Type::Const));
	classDefFlex.add_static_constant("OP_FETCH", umath::to_integral(::Flex::Operation::Type::Fetch));
	classDefFlex.add_static_constant("OP_FETCH2", umath::to_integral(::Flex::Operation::Type::Fetch2));
	classDefFlex.add_static_constant("OP_ADD", umath::to_integral(::Flex::Operation::Type::Add));
	classDefFlex.add_static_constant("OP_SUB", umath::to_integral(::Flex::Operation::Type::Sub));
	classDefFlex.add_static_constant("OP_MUL", umath::to_integral(::Flex::Operation::Type::Mul));
	classDefFlex.add_static_constant("OP_DIV", umath::to_integral(::Flex::Operation::Type::Div));
	classDefFlex.add_static_constant("OP_NEG", umath::to_integral(::Flex::Operation::Type::Neg));
	classDefFlex.add_static_constant("OP_EXP", umath::to_integral(::Flex::Operation::Type::Exp));
	classDefFlex.add_static_constant("OP_OPEN", umath::to_integral(::Flex::Operation::Type::Open));
	classDefFlex.add_static_constant("OP_CLOSE", umath::to_integral(::Flex::Operation::Type::Close));
	classDefFlex.add_static_constant("OP_COMMA", umath::to_integral(::Flex::Operation::Type::Comma));
	classDefFlex.add_static_constant("OP_MAX", umath::to_integral(::Flex::Operation::Type::Max));
	classDefFlex.add_static_constant("OP_MIN", umath::to_integral(::Flex::Operation::Type::Min));
	classDefFlex.add_static_constant("OP_TWO_WAY0", umath::to_integral(::Flex::Operation::Type::TwoWay0));
	classDefFlex.add_static_constant("OP_TWO_WAY1", umath::to_integral(::Flex::Operation::Type::TwoWay1));
	classDefFlex.add_static_constant("OP_N_WAY", umath::to_integral(::Flex::Operation::Type::NWay));
	classDefFlex.add_static_constant("OP_COMBO", umath::to_integral(::Flex::Operation::Type::Combo));
	classDefFlex.add_static_constant("OP_DOMINATE", umath::to_integral(::Flex::Operation::Type::Dominate));
	classDefFlex.add_static_constant("OP_DME_LOWER_EYELID", umath::to_integral(::Flex::Operation::Type::DMELowerEyelid));
	classDefFlex.add_static_constant("OP_DME_UPPER_EYELID", umath::to_integral(::Flex::Operation::Type::DMEUpperEyelid));

	// Operation
	auto classDefFlexOp = luabind::class_<::Flex::Operation>("Operation");
	classDefFlexOp.def(luabind::constructor<>());
	classDefFlexOp.def_readwrite("type", reinterpret_cast<uint32_t Flex::Operation::*>(&Flex::Operation::type));
	classDefFlexOp.def_readwrite("index", reinterpret_cast<int32_t Flex::Operation::*>(&Flex::Operation::d));
	classDefFlexOp.def_readwrite("value", reinterpret_cast<float Flex::Operation::*>(&Flex::Operation::d));
	classDefFlexOp.def(
	  "SetName", +[](::Flex &flex, const std::string name) { flex.SetName(name); });
	classDefFlexOp.def("GetName", static_cast<void (*)(lua_State *, ::Flex &)>([](lua_State *l, ::Flex &flex) { Lua::PushString(l, flex.GetName()); }));
	classDefFlex.scope[classDefFlexOp];

	classDef.scope[classDefFlex];

	// Frame
	auto classDefFrame = luabind::class_<::Frame>("Frame")
	                       .def("GetBoneMatrix", &Lua::Frame::GetBoneMatrix)
	                       .def("GetBoneTransform", &Lua::Frame::GetBonePosition)
	                       .def("GetBoneRotation", &Lua::Frame::GetBoneOrientation)
	                       .def("SetBonePosition", &Lua::Frame::SetBonePosition)
	                       .def("SetBoneRotation", &Lua::Frame::SetBoneOrientation)
	                       .def("Localize", static_cast<void (*)(lua_State *, ::Frame &, pragma::animation::Animation &, pragma::animation::Skeleton *)>(&Lua::Frame::Localize))
	                       .def("Localize", static_cast<void (*)(lua_State *, ::Frame &, pragma::animation::Skeleton *)>(&Lua::Frame::Localize))
	                       .def("Globalize", static_cast<void (*)(lua_State *, ::Frame &, pragma::animation::Animation &, pragma::animation::Skeleton *)>(&Lua::Frame::Globalize))
	                       .def("Globalize", static_cast<void (*)(lua_State *, ::Frame &, pragma::animation::Skeleton *)>(&Lua::Frame::Globalize))
	                       .def("CalcRenderBounds", &Lua::Frame::CalcRenderBounds)
	                       .def("Rotate", &Lua::Frame::Rotate)
	                       .def("Translate", &Lua::Frame::Translate)
	                       .def("Scale", &Lua::Frame::Scale)
	                       .def("GetMoveTranslation", &Lua::Frame::GetMoveTranslation)
	                       .def("GetMoveTranslationX", &Lua::Frame::GetMoveTranslationX)
	                       .def("GetMoveTranslationZ", &Lua::Frame::GetMoveTranslationZ)
	                       .def("SetMoveTranslation", &Lua::Frame::SetMoveTranslation)
	                       .def("SetMoveTranslationX", &Lua::Frame::SetMoveTranslationX)
	                       .def("SetMoveTranslationZ", &Lua::Frame::SetMoveTranslationZ)
	                       .def("SetBoneScale", &Lua::Frame::SetBoneScale)
	                       .def("GetBoneScale", &Lua::Frame::GetBoneScale)
	                       .def("SetBoneTransform", static_cast<void (*)(lua_State *, ::Frame &, unsigned int, const Vector3 &, const Quat &, const Vector3 &)>(&Lua::Frame::SetBoneTransform))
	                       .def("SetBoneTransform", static_cast<void (*)(lua_State *, ::Frame &, unsigned int, const Vector3 &, const Quat &)>(&Lua::Frame::SetBoneTransform))
	                       .def("GetLocalBoneTransform", &Lua::Frame::GetLocalBoneTransform)
	                       .def("GetBoneCount", &Lua::Frame::GetBoneCount)
	                       .def("SetBoneCount", &Lua::Frame::SetBoneCount)
	                       .def("SetBonePose", static_cast<void (*)(lua_State *, ::Frame &, uint32_t, const umath::ScaledTransform &)>(&Lua::Frame::SetBonePose))
	                       .def("SetBonePose", static_cast<void (*)(lua_State *, ::Frame &, uint32_t, const umath::Transform &)>(&Lua::Frame::SetBonePose))
	                       .def("GetBonePose", &Lua::Frame::GetBonePose)
	                       .def("GetFlexControllerWeights", static_cast<void (*)(lua_State *, ::Frame &)>([](lua_State *l, ::Frame &frame) {
		                       auto &flexFrameData = frame.GetFlexFrameData();
		                       auto t = Lua::CreateTable(l);
		                       auto n = flexFrameData.flexControllerWeights.size();
		                       for(auto i = decltype(n) {0u}; i < n; ++i) {
			                       Lua::PushInt(l, i + 1);
			                       Lua::PushNumber(l, flexFrameData.flexControllerWeights.at(i));
			                       Lua::SetTableValue(l, t);
		                       }
	                       }))
	                       .def("GetFlexControllerIds", static_cast<void (*)(lua_State *, ::Frame &)>([](lua_State *l, ::Frame &frame) {
		                       auto &flexFrameData = frame.GetFlexFrameData();
		                       auto t = Lua::CreateTable(l);
		                       auto n = flexFrameData.flexControllerIds.size();
		                       for(auto i = decltype(n) {0u}; i < n; ++i) {
			                       Lua::PushInt(l, i + 1);
			                       Lua::PushInt(l, flexFrameData.flexControllerIds.at(i));
			                       Lua::SetTableValue(l, t);
		                       }
	                       }))
	                       .def("SetFlexControllerWeights", static_cast<void (*)(lua_State *, ::Frame &, luabind::object)>([](lua_State *l, ::Frame &frame, luabind::object) {
		                       auto &flexFrameData = frame.GetFlexFrameData();
		                       flexFrameData.flexControllerIds.clear();
		                       flexFrameData.flexControllerWeights.clear();

		                       auto t = Lua::CreateTable(l);
		                       Lua::CheckTable(l, 2);

		                       Lua::PushNil(l);
		                       while(Lua::GetNextPair(l, 2) != 0) {
			                       auto flexCId = Lua::CheckInt(l, -2);
			                       auto weight = Lua::CheckNumber(l, -1);
			                       flexFrameData.flexControllerIds.push_back(flexCId);
			                       flexFrameData.flexControllerWeights.push_back(weight);

			                       Lua::Pop(l, 1);
		                       }
	                       }))
	                       .def("Copy", static_cast<void (*)(lua_State *, ::Frame &)>([](lua_State *l, ::Frame &frame) {
		                       auto cpy = ::Frame::Create(frame);
		                       Lua::Push(l, cpy);
	                       }));
	classDefFrame.scope[luabind::def("Create", &Lua::Frame::Create)];

	// Animation
	auto classDefAnimation = luabind::class_<pragma::animation::Animation>("Animation")
	                           .def("ToPanimaAnimation", &::pragma::animation::Animation::ToPanimaAnimation)
	                           .def("ToPanimaAnimation", &::pragma::animation::Animation::ToPanimaAnimation, luabind::default_parameter_policy<3, static_cast<const ::Frame *>(nullptr)> {})
	                           .def("GetFrame", &Lua::Animation::GetFrame)
	                           .def("GetBoneList", &Lua::Animation::GetBoneList)
	                           .def("GetActivity", &pragma::animation::Animation::GetActivity)
	                           .def("GetActivityName", static_cast<luabind::object (*)(lua_State *, pragma::animation::Animation &)>([](lua_State *l, pragma::animation::Animation &anim) -> luabind::object {
		                           auto &reg = pragma::animation::Animation::GetActivityEnumRegister();
		                           auto *name = reg.GetEnumName(umath::to_integral(anim.GetActivity()));
		                           if(name == nullptr)
			                           return {};
		                           return luabind::object {l, *name};
	                           }))
	                           .def("GetBoneId", static_cast<int32_t (*)(lua_State *, pragma::animation::Animation &, uint32_t)>([](lua_State *l, pragma::animation::Animation &anim, uint32_t idx) -> int32_t {
		                           auto boneList = anim.GetBoneList();
		                           return (idx < boneList.size()) ? boneList[idx] : -1;
	                           }))
	                           .def("SetActivity", &pragma::animation::Animation::SetActivity)
	                           .def("GetActivityWeight", &pragma::animation::Animation::GetActivityWeight)
	                           .def("SetActivityWeight", &pragma::animation::Animation::SetActivityWeight)
	                           .def("GetFPS", &pragma::animation::Animation::GetFPS)
	                           .def("SetFPS", &pragma::animation::Animation::SetFPS)
	                           .def("GetFlags", &pragma::animation::Animation::GetFlags)
	                           .def("SetFlags", &pragma::animation::Animation::SetFlags)
	                           .def("AddFlags", &pragma::animation::Animation::AddFlags)
	                           .def("RemoveFlags", &pragma::animation::Animation::RemoveFlags)
	                           .def("AddFrame", &Lua::Animation::AddFrame)
	                           .def("GetFrames", &Lua::Animation::GetFrames)
	                           .def("GetDuration", &pragma::animation::Animation::GetDuration)
	                           .def("GetBoneCount", &pragma::animation::Animation::GetBoneCount)
	                           .def("GetFrameCount", &pragma::animation::Animation::GetFrameCount)
	                           .def("AddEvent", &Lua::Animation::AddEvent)
	                           .def("GetEvents", static_cast<void (*)(lua_State *, pragma::animation::Animation &, uint32_t)>(&Lua::Animation::GetEvents))
	                           .def("GetEvents", static_cast<void (*)(lua_State *, pragma::animation::Animation &)>(&Lua::Animation::GetEvents))
	                           .def("GetEventCount", static_cast<void (*)(lua_State *, pragma::animation::Animation &, uint32_t)>(&Lua::Animation::GetEventCount))
	                           .def("GetEventCount", static_cast<void (*)(lua_State *, pragma::animation::Animation &)>(&Lua::Animation::GetEventCount))
	                           .def("GetFadeInTime", &pragma::animation::Animation::GetFadeInTime)
	                           .def("GetFadeOutTime", &pragma::animation::Animation::GetFadeOutTime)
	                           .def("GetBlendController", &Lua::Animation::GetBlendController)
	                           .def("CalcRenderBounds", &Lua::Animation::CalcRenderBounds)
	                           .def("GetRenderBounds", &Lua::Animation::GetRenderBounds)
	                           .def("Rotate", &pragma::animation::Animation::Rotate)
	                           .def("Translate", &pragma::animation::Animation::Translate)
	                           .def("Scale", &pragma::animation::Animation::Scale)
	                           .def("Reverse", &pragma::animation::Animation::Reverse)
	                           .def("RemoveEvent", &Lua::Animation::RemoveEvent)
	                           .def("SetEventData", &Lua::Animation::SetEventData)
	                           .def("SetEventType", &Lua::Animation::SetEventType)
	                           .def("SetEventArgs", &Lua::Animation::SetEventArgs)
	                           .def("LookupBone", &Lua::Animation::LookupBone)
	                           .def("SetBoneList", &Lua::Animation::SetBoneList)
	                           .def("AddBoneId", &pragma::animation::Animation::AddBoneId)
	                           .def("SetFadeInTime", &pragma::animation::Animation::SetFadeInTime)
	                           .def("SetFadeOutTime", &pragma::animation::Animation::SetFadeOutTime)
	                           .def("SetBoneWeight", &pragma::animation::Animation::SetBoneWeight)
	                           .def("GetBoneWeight", static_cast<float (pragma::animation::Animation::*)(uint32_t) const>(&pragma::animation::Animation::GetBoneWeight))
	                           .def("GetBoneWeights", &Lua::Animation::GetBoneWeights)
	                           .def("ClearFrames", static_cast<void (*)(lua_State *, pragma::animation::Animation &)>([](lua_State *l, pragma::animation::Animation &anim) { anim.GetFrames().clear(); }))
	                           /*.def("GetBoneId",static_cast<void(*)(lua_State*,pragma::animation::Animation&,uint32_t)>([](lua_State *l,pragma::animation::Animation &anim,uint32_t idx) {
			auto &boneList = anim.GetBoneList();
			if(idx >= boneList.size())
				return;
			Lua::PushInt(l,boneList[idx]);
		}))*/
	                           .def("Save", static_cast<void (*)(lua_State *, pragma::animation::Animation &, udm::AssetData &)>([](lua_State *l, pragma::animation::Animation &anim, udm::AssetData &assetData) {
		                           std::string err;
		                           auto result = anim.Save(assetData, err);
		                           if(result == false)
			                           Lua::PushString(l, err);
		                           else
			                           Lua::PushBool(l, result);
	                           }));

	classDefAnimation.scope[luabind::def("Create", static_cast<std::shared_ptr<::pragma::animation::Animation> (*)()>(&::pragma::animation::Animation::Create)),
	  luabind::def("Create", static_cast<std::shared_ptr<::pragma::animation::Animation> (*)(const ::pragma::animation::Animation &, ::pragma::animation::Animation::ShareMode)>(&::pragma::animation::Animation::Create)),
	  luabind::def("Create", static_cast<std::shared_ptr<::pragma::animation::Animation> (*)(const ::pragma::animation::Animation &, ::pragma::animation::Animation::ShareMode)>(&::pragma::animation::Animation::Create),
	    luabind::default_parameter_policy<2, ::pragma::animation::Animation::ShareMode::None> {})];
	classDefAnimation.scope[luabind::def("Load", &Lua::Animation::Load), luabind::def("RegisterActivity", &Lua::Animation::RegisterActivityEnum), luabind::def("RegisterEvent", &Lua::Animation::RegisterEventEnum), luabind::def("GetActivityEnums", &Lua::Animation::GetActivityEnums),
	  luabind::def("GetEventEnums", &Lua::Animation::GetEventEnums), luabind::def("GetActivityEnumName", &Lua::Animation::GetActivityEnumName), luabind::def("GetEventEnumName", &Lua::Animation::GetEventEnumName), luabind::def("FindActivityId", &Lua::Animation::FindActivityId),
	  luabind::def("FindEventId", &Lua::Animation::FindEventId), classDefFrame];
	classDefAnimation.scope[luabind::def("Load", static_cast<void (*)(lua_State *, udm::AssetData &)>([](lua_State *l, udm::AssetData &assetData) {
		std::string err;
		auto anim = pragma::animation::Animation::Load(assetData, err);
		if(anim == nullptr) {
			Lua::PushBool(l, false);
			Lua::PushString(l, err);
			return;
		}
		Lua::Push(l, anim);
	}))];
	//for(auto &pair : ANIMATION_EVENT_NAMES)
	//	classDefAnimation.add_static_constant(pair.second.c_str(),pair.first);

	classDefAnimation.add_static_constant("FLAG_LOOP", umath::to_integral(FAnim::Loop));
	classDefAnimation.add_static_constant("FLAG_NOREPEAT", umath::to_integral(FAnim::NoRepeat));
	classDefAnimation.add_static_constant("FLAG_MOVEX", umath::to_integral(FAnim::MoveX));
	classDefAnimation.add_static_constant("FLAG_MOVEZ", umath::to_integral(FAnim::MoveZ));
	classDefAnimation.add_static_constant("FLAG_AUTOPLAY", umath::to_integral(FAnim::Autoplay));
	classDefAnimation.add_static_constant("FLAG_GESTURE", umath::to_integral(FAnim::Gesture));

	//for(auto &pair : ACTIVITY_NAMES)
	//	classDefAnimation.add_static_constant(pair.second.c_str(),pair.first);

	auto defBoneInfo = luabind::class_<pragma::animation::MetaRigBone>("MetaRigBone");
	defBoneInfo.def(luabind::tostring(luabind::self));
	defBoneInfo.def_readwrite("boneId", &pragma::animation::MetaRigBone::boneId);
	defBoneInfo.def_readwrite("radius", &pragma::animation::MetaRigBone::radius);
	defBoneInfo.def_readwrite("length", &pragma::animation::MetaRigBone::length);
	defBoneInfo.def_readwrite("normalizedRotationOffset", &pragma::animation::MetaRigBone::normalizedRotationOffset);
	defBoneInfo.property(
	  "min", +[](const pragma::animation::MetaRigBone &boneInfo) -> Vector3 { return boneInfo.bounds.first; }, +[](pragma::animation::MetaRigBone &boneInfo, const Vector3 &min) { boneInfo.bounds.first = min; });
	defBoneInfo.property(
	  "max", +[](const pragma::animation::MetaRigBone &boneInfo) -> Vector3 { return boneInfo.bounds.second; }, +[](pragma::animation::MetaRigBone &boneInfo, const Vector3 &max) { boneInfo.bounds.second = max; });
	classDef.scope[defBoneInfo];

	auto defBlendShapeInfo = luabind::class_<pragma::animation::MetaRigBlendShape>("MetaRigBlendShape");
	defBlendShapeInfo.def(luabind::tostring(luabind::self));
	defBlendShapeInfo.def_readwrite("flexControllerId", &pragma::animation::MetaRigBlendShape::flexControllerId);
	classDef.scope[defBlendShapeInfo];

	auto defRig = luabind::class_<pragma::animation::MetaRig>("MetaRig");
	defRig.scope[luabind::def(
	  "load", +[](lua_State *l, const std::string &fileName, const pragma::animation::Skeleton &skeleton) -> Lua::variadic<std::shared_ptr<pragma::animation::MetaRig>, std::pair<bool, std::string>> {
		  auto udmData = udm::Data::Load(fileName);
		  if(!udmData)
			  return luabind::object {l, std::pair<bool, std::string> {false, "Failed to load file '" + fileName + "'!"}};
		  auto metaRig = std::make_shared<pragma::animation::MetaRig>();
		  std::string err;
		  auto res = metaRig->Load(skeleton, udmData->GetAssetData(), err);
		  if(!res)
			  return luabind::object {l, std::pair<bool, std::string> {false, err}};
		  return luabind::object {l, metaRig};
	  })];
	defRig.def_readwrite("rigType", &pragma::animation::MetaRig::rigType);
	defRig.def_readwrite("forwardFacingRotationOffset", &pragma::animation::MetaRig::forwardFacingRotationOffset);
	defRig.def_readwrite("forwardAxis", &pragma::animation::MetaRig::forwardAxis);
	defRig.def_readwrite("upAxis", &pragma::animation::MetaRig::upAxis);
	defRig.def_readwrite("min", &pragma::animation::MetaRig::min);
	defRig.def_readwrite("max", &pragma::animation::MetaRig::max);
	defRig.def(luabind::tostring(luabind::self));
	defRig.add_static_constant("ROOT_BONE", umath::to_integral(pragma::animation::META_RIG_ROOT_BONE_TYPE));
	defRig.scope[luabind::def("get_bone_name", &pragma::animation::get_meta_rig_bone_type_name)];
	defRig.scope[luabind::def("get_bone_enum", &pragma::animation::get_meta_rig_bone_type_enum)];
	defRig.scope[luabind::def("get_bone_side", &pragma::animation::get_meta_rig_bone_type_side)];
	defRig.scope[luabind::def("get_bone_parent", &pragma::animation::get_meta_rig_bone_parent_type)];
	defRig.scope[luabind::def("get_bone_children", &pragma::animation::get_meta_rig_bone_children)];
	defRig.scope[luabind::def("get_blend_shape_name", &pragma::animation::get_blend_shape_name)];
	defRig.scope[luabind::def("get_blend_shape_enum", &pragma::animation::get_blend_shape_enum)];
	defRig.scope[luabind::def("get_meta_rig_bone_ids", &pragma::animation::get_meta_rig_bone_ids)];
	defRig.scope[luabind::def("get_root_meta_bone_id", &pragma::animation::get_root_meta_bone_id)];
	defRig.def(
	  "GetNormalizedBoneInfo", +[](const pragma::animation::MetaRig &metaRig, pragma::animation::MetaRigBoneType boneType) -> const pragma::animation::MetaRigBone * {
		  auto idx = umath::to_integral(boneType);
		  if(idx >= metaRig.bones.size())
			  throw std::runtime_error {"Invalid bone type index!"};
		  return &metaRig.bones[idx];
	  });
	defRig.def(
	  "GetBoneId", +[](const pragma::animation::MetaRig &metaRig, pragma::animation::MetaRigBoneType boneType) -> std::optional<pragma::animation::BoneId> {
		  auto boneId = metaRig.GetBoneId(boneType);
		  if(boneId == pragma::animation::INVALID_BONE_INDEX)
			  return {};
		  return boneId;
	  });
	defRig.def(
	  "Save", +[](lua_State *l, const pragma::animation::MetaRig &metaRig, const pragma::animation::Skeleton &skeleton, const std::string &fileName) -> std::pair<bool, std::optional<std::string>> {
		  std::string err;
		  auto udmData = udm::Data::Create();
		  auto res = metaRig.Save(skeleton, udmData->GetAssetData(), err);
		  if(!res)
			  return {false, err};
		  std::string writeFileName = fileName;
		  if(Lua::file::validate_write_operation(l, writeFileName) == false)
			  return {false, "Failed to save as file '" + fileName + "'!"};

		  try {
			  if(!udmData->SaveAscii(writeFileName))
				  return {false, "Failed to save as file '" + writeFileName + "'!"};
		  }
		  catch(const udm::Exception &err) {
			  return {false, err.what()};
		  }
		  return {true, {}};
	  });
	defRig.def("GetBone", &pragma::animation::MetaRig::GetBone);
	defRig.def("FindMetaBoneType", &pragma::animation::MetaRig::FindMetaBoneType);
	defRig.def("GetBlendShape", &pragma::animation::MetaRig::GetBlendShape);
	defRig.def("GetReferenceScale", &pragma::animation::MetaRig::GetReferenceScale);
	defRig.def("DebugPrint", &pragma::animation::MetaRig::DebugPrint);
	defRig.add_static_constant("RIG_TYPE_BIPED", umath::to_integral(pragma::animation::RigType::Biped));
	defRig.add_static_constant("RIG_TYPE_QUADRUPED", umath::to_integral(pragma::animation::RigType::Quadruped));

	defRig.add_static_constant("BONE_SIDE_LEFT", umath::to_integral(pragma::animation::BoneSide::Left));
	defRig.add_static_constant("BONE_SIDE_RIGHT", umath::to_integral(pragma::animation::BoneSide::Right));

	defRig.add_static_constant("BONE_TYPE_HIPS", umath::to_integral(pragma::animation::MetaRigBoneType::Hips));
	defRig.add_static_constant("BONE_TYPE_PELVIS", umath::to_integral(pragma::animation::MetaRigBoneType::Pelvis));
	defRig.add_static_constant("BONE_TYPE_SPINE", umath::to_integral(pragma::animation::MetaRigBoneType::Spine));
	defRig.add_static_constant("BONE_TYPE_SPINE1", umath::to_integral(pragma::animation::MetaRigBoneType::Spine1));
	defRig.add_static_constant("BONE_TYPE_SPINE2", umath::to_integral(pragma::animation::MetaRigBoneType::Spine2));
	defRig.add_static_constant("BONE_TYPE_SPINE3", umath::to_integral(pragma::animation::MetaRigBoneType::Spine3));
	defRig.add_static_constant("BONE_TYPE_NECK", umath::to_integral(pragma::animation::MetaRigBoneType::Neck));
	defRig.add_static_constant("BONE_TYPE_HEAD", umath::to_integral(pragma::animation::MetaRigBoneType::Head));
	defRig.add_static_constant("BONE_TYPE_JAW", umath::to_integral(pragma::animation::MetaRigBoneType::Jaw));
	defRig.add_static_constant("BONE_TYPE_LEFT_EAR", umath::to_integral(pragma::animation::MetaRigBoneType::LeftEar));
	defRig.add_static_constant("BONE_TYPE_RIGHT_EAR", umath::to_integral(pragma::animation::MetaRigBoneType::RightEar));
	defRig.add_static_constant("BONE_TYPE_LEFT_EYE", umath::to_integral(pragma::animation::MetaRigBoneType::LeftEye));
	defRig.add_static_constant("BONE_TYPE_RIGHT_EYE", umath::to_integral(pragma::animation::MetaRigBoneType::RightEye));
	defRig.add_static_constant("BONE_TYPE_CENTER_EYE", umath::to_integral(pragma::animation::MetaRigBoneType::CenterEye));
	defRig.add_static_constant("BONE_TYPE_LEFT_UPPER_ARM", umath::to_integral(pragma::animation::MetaRigBoneType::LeftUpperArm));
	defRig.add_static_constant("BONE_TYPE_LEFT_LOWER_ARM", umath::to_integral(pragma::animation::MetaRigBoneType::LeftLowerArm));
	defRig.add_static_constant("BONE_TYPE_LEFT_HAND", umath::to_integral(pragma::animation::MetaRigBoneType::LeftHand));
	defRig.add_static_constant("BONE_TYPE_RIGHT_UPPER_ARM", umath::to_integral(pragma::animation::MetaRigBoneType::RightUpperArm));
	defRig.add_static_constant("BONE_TYPE_RIGHT_LOWER_ARM", umath::to_integral(pragma::animation::MetaRigBoneType::RightLowerArm));
	defRig.add_static_constant("BONE_TYPE_RIGHT_HAND", umath::to_integral(pragma::animation::MetaRigBoneType::RightHand));
	defRig.add_static_constant("BONE_TYPE_LEFT_UPPER_LEG", umath::to_integral(pragma::animation::MetaRigBoneType::LeftUpperLeg));
	defRig.add_static_constant("BONE_TYPE_LEFT_LOWER_LEG", umath::to_integral(pragma::animation::MetaRigBoneType::LeftLowerLeg));
	defRig.add_static_constant("BONE_TYPE_LEFT_FOOT", umath::to_integral(pragma::animation::MetaRigBoneType::LeftFoot));
	defRig.add_static_constant("BONE_TYPE_LEFT_TOE", umath::to_integral(pragma::animation::MetaRigBoneType::LeftToe));
	defRig.add_static_constant("BONE_TYPE_RIGHT_UPPER_LEG", umath::to_integral(pragma::animation::MetaRigBoneType::RightUpperLeg));
	defRig.add_static_constant("BONE_TYPE_RIGHT_LOWER_LEG", umath::to_integral(pragma::animation::MetaRigBoneType::RightLowerLeg));
	defRig.add_static_constant("BONE_TYPE_RIGHT_FOOT", umath::to_integral(pragma::animation::MetaRigBoneType::RightFoot));
	defRig.add_static_constant("BONE_TYPE_RIGHT_TOE", umath::to_integral(pragma::animation::MetaRigBoneType::RightToe));
	defRig.add_static_constant("BONE_TYPE_LEFT_THUMB1", umath::to_integral(pragma::animation::MetaRigBoneType::LeftThumb1));
	defRig.add_static_constant("BONE_TYPE_LEFT_THUMB2", umath::to_integral(pragma::animation::MetaRigBoneType::LeftThumb2));
	defRig.add_static_constant("BONE_TYPE_LEFT_THUMB3", umath::to_integral(pragma::animation::MetaRigBoneType::LeftThumb3));
	defRig.add_static_constant("BONE_TYPE_LEFT_INDEX_FINGER1", umath::to_integral(pragma::animation::MetaRigBoneType::LeftIndexFinger1));
	defRig.add_static_constant("BONE_TYPE_LEFT_INDEX_FINGER2", umath::to_integral(pragma::animation::MetaRigBoneType::LeftIndexFinger2));
	defRig.add_static_constant("BONE_TYPE_LEFT_INDEX_FINGER3", umath::to_integral(pragma::animation::MetaRigBoneType::LeftIndexFinger3));
	defRig.add_static_constant("BONE_TYPE_LEFT_MIDDLE_FINGER1", umath::to_integral(pragma::animation::MetaRigBoneType::LeftMiddleFinger1));
	defRig.add_static_constant("BONE_TYPE_LEFT_MIDDLE_FINGER2", umath::to_integral(pragma::animation::MetaRigBoneType::LeftMiddleFinger2));
	defRig.add_static_constant("BONE_TYPE_LEFT_MIDDLE_FINGER3", umath::to_integral(pragma::animation::MetaRigBoneType::LeftMiddleFinger3));
	defRig.add_static_constant("BONE_TYPE_LEFT_RING_FINGER1", umath::to_integral(pragma::animation::MetaRigBoneType::LeftRingFinger1));
	defRig.add_static_constant("BONE_TYPE_LEFT_RING_FINGER2", umath::to_integral(pragma::animation::MetaRigBoneType::LeftRingFinger2));
	defRig.add_static_constant("BONE_TYPE_LEFT_RING_FINGER3", umath::to_integral(pragma::animation::MetaRigBoneType::LeftRingFinger3));
	defRig.add_static_constant("BONE_TYPE_LEFT_LITTLE_FINGER1", umath::to_integral(pragma::animation::MetaRigBoneType::LeftLittleFinger1));
	defRig.add_static_constant("BONE_TYPE_LEFT_LITTLE_FINGER2", umath::to_integral(pragma::animation::MetaRigBoneType::LeftLittleFinger2));
	defRig.add_static_constant("BONE_TYPE_LEFT_LITTLE_FINGER3", umath::to_integral(pragma::animation::MetaRigBoneType::LeftLittleFinger3));
	defRig.add_static_constant("BONE_TYPE_RIGHT_THUMB1", umath::to_integral(pragma::animation::MetaRigBoneType::RightThumb1));
	defRig.add_static_constant("BONE_TYPE_RIGHT_THUMB2", umath::to_integral(pragma::animation::MetaRigBoneType::RightThumb2));
	defRig.add_static_constant("BONE_TYPE_RIGHT_THUMB3", umath::to_integral(pragma::animation::MetaRigBoneType::RightThumb3));
	defRig.add_static_constant("BONE_TYPE_RIGHT_INDEX_FINGER1", umath::to_integral(pragma::animation::MetaRigBoneType::RightIndexFinger1));
	defRig.add_static_constant("BONE_TYPE_RIGHT_INDEX_FINGER2", umath::to_integral(pragma::animation::MetaRigBoneType::RightIndexFinger2));
	defRig.add_static_constant("BONE_TYPE_RIGHT_INDEX_FINGER3", umath::to_integral(pragma::animation::MetaRigBoneType::RightIndexFinger3));
	defRig.add_static_constant("BONE_TYPE_RIGHT_MIDDLE_FINGER1", umath::to_integral(pragma::animation::MetaRigBoneType::RightMiddleFinger1));
	defRig.add_static_constant("BONE_TYPE_RIGHT_MIDDLE_FINGER2", umath::to_integral(pragma::animation::MetaRigBoneType::RightMiddleFinger2));
	defRig.add_static_constant("BONE_TYPE_RIGHT_MIDDLE_FINGER3", umath::to_integral(pragma::animation::MetaRigBoneType::RightMiddleFinger3));
	defRig.add_static_constant("BONE_TYPE_RIGHT_RING_FINGER1", umath::to_integral(pragma::animation::MetaRigBoneType::RightRingFinger1));
	defRig.add_static_constant("BONE_TYPE_RIGHT_RING_FINGER2", umath::to_integral(pragma::animation::MetaRigBoneType::RightRingFinger2));
	defRig.add_static_constant("BONE_TYPE_RIGHT_RING_FINGER3", umath::to_integral(pragma::animation::MetaRigBoneType::RightRingFinger3));
	defRig.add_static_constant("BONE_TYPE_RIGHT_LITTLE_FINGER1", umath::to_integral(pragma::animation::MetaRigBoneType::RightLittleFinger1));
	defRig.add_static_constant("BONE_TYPE_RIGHT_LITTLE_FINGER2", umath::to_integral(pragma::animation::MetaRigBoneType::RightLittleFinger2));
	defRig.add_static_constant("BONE_TYPE_RIGHT_LITTLE_FINGER3", umath::to_integral(pragma::animation::MetaRigBoneType::RightLittleFinger3));
	defRig.add_static_constant("BONE_TYPE_TAIL_BASE", umath::to_integral(pragma::animation::MetaRigBoneType::TailBase));
	defRig.add_static_constant("BONE_TYPE_TAIL_MIDDLE", umath::to_integral(pragma::animation::MetaRigBoneType::TailMiddle));
	defRig.add_static_constant("BONE_TYPE_TAIL_MIDDLE1", umath::to_integral(pragma::animation::MetaRigBoneType::TailMiddle1));
	defRig.add_static_constant("BONE_TYPE_TAIL_TIP", umath::to_integral(pragma::animation::MetaRigBoneType::TailTip));
	defRig.add_static_constant("BONE_TYPE_LEFT_WING", umath::to_integral(pragma::animation::MetaRigBoneType::LeftWing));
	defRig.add_static_constant("BONE_TYPE_LEFT_WING_MIDDLE", umath::to_integral(pragma::animation::MetaRigBoneType::LeftWingMiddle));
	defRig.add_static_constant("BONE_TYPE_LEFT_WING_TIP", umath::to_integral(pragma::animation::MetaRigBoneType::LeftWingTip));
	defRig.add_static_constant("BONE_TYPE_RIGHT_WING", umath::to_integral(pragma::animation::MetaRigBoneType::RightWing));
	defRig.add_static_constant("BONE_TYPE_RIGHT_WING_MIDDLE", umath::to_integral(pragma::animation::MetaRigBoneType::RightWingMiddle));
	defRig.add_static_constant("BONE_TYPE_RIGHT_WING_TIP", umath::to_integral(pragma::animation::MetaRigBoneType::RightWingTip));
	defRig.add_static_constant("BONE_TYPE_LEFT_BREAST_BASE", umath::to_integral(pragma::animation::MetaRigBoneType::LeftBreastBase));
	defRig.add_static_constant("BONE_TYPE_LEFT_BREAST_MIDDLE", umath::to_integral(pragma::animation::MetaRigBoneType::LeftBreastMiddle));
	defRig.add_static_constant("BONE_TYPE_LEFT_BREAST_TIP", umath::to_integral(pragma::animation::MetaRigBoneType::LeftBreastTip));
	defRig.add_static_constant("BONE_TYPE_RIGHT_BREAST_BASE", umath::to_integral(pragma::animation::MetaRigBoneType::RightBreastBase));
	defRig.add_static_constant("BONE_TYPE_RIGHT_BREAST_MIDDLE", umath::to_integral(pragma::animation::MetaRigBoneType::RightBreastMiddle));
	defRig.add_static_constant("BONE_TYPE_RIGHT_BREAST_TIP", umath::to_integral(pragma::animation::MetaRigBoneType::RightBreastTip));
	defRig.add_static_constant("BONE_TYPE_COUNT", umath::to_integral(pragma::animation::MetaRigBoneType::Count));
	defRig.add_static_constant("BONE_TYPE_STANDARD_START", umath::to_integral(pragma::animation::MetaRigBoneType::StandardStart));
	defRig.add_static_constant("BONE_TYPE_STANDARD_END", umath::to_integral(pragma::animation::MetaRigBoneType::StandardEnd));
	defRig.add_static_constant("BONE_TYPE_INVALID", umath::to_integral(pragma::animation::MetaRigBoneType::Invalid));
	static_assert(umath::to_integral(pragma::animation::MetaRigBoneType::Count) == 74, "Update this list when new bone types are addded!");

	defRig.add_static_constant("BODY_PART_LOWER_BODY", umath::to_integral(pragma::animation::BodyPart::LowerBody));
	defRig.add_static_constant("BODY_PART_UPPER_BODY", umath::to_integral(pragma::animation::BodyPart::UpperBody));
	defRig.add_static_constant("BODY_PART_HEAD", umath::to_integral(pragma::animation::BodyPart::Head));
	defRig.add_static_constant("BODY_PART_LEFT_ARM", umath::to_integral(pragma::animation::BodyPart::LeftArm));
	defRig.add_static_constant("BODY_PART_RIGHT_ARM", umath::to_integral(pragma::animation::BodyPart::RightArm));
	defRig.add_static_constant("BODY_PART_LEFT_LEG", umath::to_integral(pragma::animation::BodyPart::LeftLeg));
	defRig.add_static_constant("BODY_PART_RIGHT_LEG", umath::to_integral(pragma::animation::BodyPart::RightLeg));
	defRig.add_static_constant("BODY_PART_TAIL", umath::to_integral(pragma::animation::BodyPart::Tail));
	defRig.add_static_constant("BODY_PART_LEFT_WING", umath::to_integral(pragma::animation::BodyPart::LeftWing));
	defRig.add_static_constant("BODY_PART_RIGHT_WING", umath::to_integral(pragma::animation::BodyPart::RightWing));
	defRig.add_static_constant("BODY_PART_LEFT_BREAST", umath::to_integral(pragma::animation::BodyPart::LeftBreast));
	defRig.add_static_constant("BODY_PART_RIGHT_BREAST", umath::to_integral(pragma::animation::BodyPart::RightBreast));
	static_assert(umath::to_integral(pragma::animation::BodyPart::Count) == 12, "Update this list when new bone types are addded!");

	classDef.scope[defRig];

	// Flex Animation
	auto classDefFlexAnim = luabind::class_<FlexAnimation>("FlexAnimation");
	classDefFlexAnim.def("GetFps", static_cast<float (*)(lua_State *, FlexAnimation &)>([](lua_State *l, FlexAnimation &flexAnim) -> float { return flexAnim.GetFps(); }));
	classDefFlexAnim.def("SetFps", static_cast<void (*)(lua_State *, FlexAnimation &, float)>([](lua_State *l, FlexAnimation &flexAnim, float fps) { return flexAnim.SetFps(fps); }));
	classDefFlexAnim.def("GetFrames", static_cast<luabind::object (*)(lua_State *, FlexAnimation &)>([](lua_State *l, FlexAnimation &flexAnim) -> luabind::object { return Lua::vector_to_table(l, flexAnim.GetFrames()); }));
	classDefFlexAnim.def("GetFrame", static_cast<std::shared_ptr<FlexAnimationFrame> (*)(lua_State *, FlexAnimation &, uint32_t)>([](lua_State *l, FlexAnimation &flexAnim, uint32_t frameId) -> std::shared_ptr<FlexAnimationFrame> {
		auto &frames = flexAnim.GetFrames();
		if(frameId >= frames.size())
			return nullptr;
		return frames[frameId];
	}));
	classDefFlexAnim.def("GetFrameCount", static_cast<uint32_t (*)(lua_State *, FlexAnimation &)>([](lua_State *l, FlexAnimation &flexAnim) -> uint32_t { return flexAnim.GetFrames().size(); }));
	classDefFlexAnim.def("GetFlexControllerIds", static_cast<luabind::object (*)(lua_State *, FlexAnimation &)>([](lua_State *l, FlexAnimation &flexAnim) -> luabind::object { return Lua::vector_to_table(l, flexAnim.GetFlexControllerIds()); }));
	classDefFlexAnim.def("SetFlexControllerIds", static_cast<void (*)(lua_State *, FlexAnimation &, luabind::table<>)>([](lua_State *l, FlexAnimation &flexAnim, luabind::table<> tIds) { flexAnim.SetFlexControllerIds(Lua::table_to_vector<FlexControllerId>(l, tIds, 2)); }));
	classDefFlexAnim.def("AddFlexControllerId", static_cast<uint32_t (*)(lua_State *, FlexAnimation &, FlexControllerId)>([](lua_State *l, FlexAnimation &flexAnim, FlexControllerId id) -> uint32_t { return flexAnim.AddFlexControllerId(id); }));
	classDefFlexAnim.def("SetFlexControllerValue", static_cast<void (*)(lua_State *, FlexAnimation &, uint32_t, FlexControllerId, float)>([](lua_State *l, FlexAnimation &flexAnim, uint32_t frameId, FlexControllerId id, float val) {
		auto &frames = flexAnim.GetFrames();
		frames.reserve(frameId + 1);
		while(frames.size() <= frameId)
			flexAnim.AddFrame();

		auto &frame = frames[frameId];
		auto idx = flexAnim.AddFlexControllerId(id);
		frame->GetValues()[idx] = val;
	}));
	classDefFlexAnim.def("GetFlexControllerCount", static_cast<uint32_t (*)(lua_State *, FlexAnimation &)>([](lua_State *l, FlexAnimation &flexAnim) -> uint32_t {
		auto &flexControllerIds = flexAnim.GetFlexControllerIds();
		return flexControllerIds.size();
	}));
	classDefFlexAnim.def("LookupLocalFlexControllerIndex", static_cast<luabind::object (*)(lua_State *, FlexAnimation &, FlexControllerId)>([](lua_State *l, FlexAnimation &flexAnim, FlexControllerId id) -> luabind::object {
		auto &ids = flexAnim.GetFlexControllerIds();
		auto it = std::find(ids.begin(), ids.end(), id);
		if(it == ids.end())
			return {};
		return luabind::object {l, it - ids.begin()};
	}));
	classDefFlexAnim.def("AddFrame", static_cast<std::shared_ptr<FlexAnimationFrame> (*)(lua_State *, FlexAnimation &)>([](lua_State *l, FlexAnimation &flexAnim) -> std::shared_ptr<FlexAnimationFrame> { return flexAnim.AddFrame().shared_from_this(); }));
	classDefFlexAnim.def("ClearFrames", static_cast<void (*)(lua_State *, FlexAnimation &)>([](lua_State *l, FlexAnimation &flexAnim) { flexAnim.GetFrames().clear(); }));
	classDefFlexAnim.def("RemoveFrame", static_cast<void (*)(lua_State *, FlexAnimation &, uint32_t)>([](lua_State *l, FlexAnimation &flexAnim, uint32_t idx) {
		auto &frames = flexAnim.GetFrames();
		if(idx >= frames.size())
			return;
		frames.erase(frames.begin() + idx);
	}));
	classDefFlexAnim.def("Save", static_cast<void (*)(lua_State *, FlexAnimation &, udm::AssetData &)>([](lua_State *l, FlexAnimation &flexAnim, udm::AssetData &assetData) {
		std::string err;
		auto result = flexAnim.Save(assetData, err);
		if(result == false)
			Lua::PushString(l, err);
		else
			Lua::PushBool(l, result);
	}));
	classDefFlexAnim.scope[luabind::def("Load", static_cast<luabind::object (*)(lua_State *, LFile &)>([](lua_State *l, LFile &f) -> luabind::object {
		auto fptr = f.GetHandle();
		if(fptr == nullptr)
			return {};
		return luabind::object {l, FlexAnimation::Load(*fptr)};
	}))];
	classDefFlexAnim.scope[luabind::def("Load", static_cast<void (*)(lua_State *, udm::AssetData &)>([](lua_State *l, udm::AssetData &assetData) {
		std::string err;
		auto anim = FlexAnimation::Load(assetData, err);
		if(anim == nullptr) {
			Lua::PushBool(l, false);
			Lua::PushString(l, err);
			return;
		}
		Lua::Push(l, anim);
	}))];

	auto classDefFlexAnimFrame = luabind::class_<FlexAnimationFrame>("Frame");
	classDefFlexAnimFrame.def("GetFlexControllerValues", static_cast<luabind::object (*)(lua_State *, FlexAnimationFrame &)>([](lua_State *l, FlexAnimationFrame &flexAnimFrame) -> luabind::object { return Lua::vector_to_table(l, flexAnimFrame.GetValues()); }));
	classDefFlexAnimFrame.def("SetFlexControllerValues", static_cast<void (*)(lua_State *, FlexAnimationFrame &, luabind::table<>)>([](lua_State *l, FlexAnimationFrame &flexAnimFrame, luabind::table<> t) { flexAnimFrame.GetValues() = Lua::table_to_vector<float>(l, t, 2); }));
	classDefFlexAnimFrame.def("GetFlexControllerValue", static_cast<void (*)(lua_State *, FlexAnimationFrame &, uint32_t)>([](lua_State *l, FlexAnimationFrame &flexAnimFrame, uint32_t id) {
		auto &values = flexAnimFrame.GetValues();
		if(id >= values.size())
			return;
		Lua::PushNumber(l, values[id]);
	}));
	classDefFlexAnimFrame.def("GetFlexControllerValueCount", static_cast<uint32_t (*)(lua_State *, FlexAnimationFrame &)>([](lua_State *l, FlexAnimationFrame &flexAnimFrame) -> uint32_t {
		auto &values = flexAnimFrame.GetValues();
		return values.size();
	}));
	classDefFlexAnimFrame.def("SetFlexControllerValue", static_cast<void (*)(lua_State *, FlexAnimationFrame &, uint32_t, float)>([](lua_State *l, FlexAnimationFrame &flexAnimFrame, uint32_t id, float val) {
		auto &values = flexAnimFrame.GetValues();
		if(id >= values.size())
			return;
		values[id] = val;
	}));
	classDefFlexAnim.scope[classDefFlexAnimFrame];

	classDef.scope[classDefFlexAnim];

	// Vertex Animation
	auto classDefVertexAnimation = luabind::class_<::VertexAnimation>("VertexAnimation")
	                                 .def("Rotate", static_cast<void (*)(lua_State *, ::VertexAnimation &, const Quat &)>([](lua_State *l, ::VertexAnimation &vertAnim, const Quat &rot) { vertAnim.Rotate(rot); }))
	                                 .def("GetMeshAnimations", &Lua::VertexAnimation::GetMeshAnimations)
	                                 .def("GetName", &Lua::VertexAnimation::GetName);

	auto classDefMeshVertexFrame = luabind::class_<::MeshVertexFrame>("Frame")
	                                 .def("Rotate", static_cast<void (*)(lua_State *, ::MeshVertexFrame &, const Quat &)>([](lua_State *l, ::MeshVertexFrame &meshVertFrame, const Quat &rot) { meshVertFrame.Rotate(rot); }))
	                                 .def("GetVertices", &Lua::MeshVertexFrame::GetVertices)
	                                 .def("SetVertexCount", &Lua::MeshVertexFrame::SetVertexCount)
	                                 .def("SetVertexPosition", &Lua::MeshVertexFrame::SetVertexPosition)
	                                 .def("GetVertexPosition", &Lua::MeshVertexFrame::GetVertexPosition)
	                                 .def("SetVertexNormal", &Lua::MeshVertexFrame::SetVertexNormal)
	                                 .def("GetVertexNormal", &Lua::MeshVertexFrame::GetVertexNormal)
	                                 .def("GetVertexCount", static_cast<void (*)(lua_State *, ::MeshVertexFrame &)>([](lua_State *l, ::MeshVertexFrame &meshVertFrame) { Lua::PushInt(l, meshVertFrame.GetVertexCount()); }))
	                                 .def("GetFlags", static_cast<void (*)(lua_State *, ::MeshVertexFrame &)>([](lua_State *l, ::MeshVertexFrame &meshVertFrame) { Lua::PushInt(l, umath::to_integral(meshVertFrame.GetFlags())); }))
	                                 .def("SetFlags", static_cast<void (*)(lua_State *, ::MeshVertexFrame &, uint32_t)>([](lua_State *l, ::MeshVertexFrame &meshVertFrame, uint32_t flags) { meshVertFrame.SetFlags(static_cast<::MeshVertexFrame::Flags>(flags)); }))
	                                 .def("SetDeltaValue", static_cast<void (*)(lua_State *, ::MeshVertexFrame &, uint32_t, float)>([](lua_State *l, ::MeshVertexFrame &meshVertFrame, uint32_t vertId, float value) { meshVertFrame.SetDeltaValue(vertId, value); }))
	                                 .def("GetDeltaValue", static_cast<void (*)(lua_State *, ::MeshVertexFrame &, uint32_t)>([](lua_State *l, ::MeshVertexFrame &meshVertFrame, uint32_t vertId) {
		                                 float value;
		                                 if(meshVertFrame.GetDeltaValue(vertId, value) == false)
			                                 return;
		                                 Lua::PushNumber(l, value);
	                                 }));
	classDefMeshVertexFrame.add_static_constant("FLAG_NONE", umath::to_integral(::MeshVertexFrame::Flags::None));
	classDefMeshVertexFrame.add_static_constant("FLAG_BIT_HAS_DELTA_VALUES", umath::to_integral(::MeshVertexFrame::Flags::HasDeltaValues));

	auto classDefMeshVertexAnimation = luabind::class_<::MeshVertexAnimation>("MeshAnimation")
	                                     .def("Rotate", static_cast<void (*)(lua_State *, ::MeshVertexAnimation &, const Quat &)>([](lua_State *l, ::MeshVertexAnimation &meshVertAnim, const Quat &rot) { meshVertAnim.Rotate(rot); }))
	                                     .def("GetFrames", &Lua::MeshVertexAnimation::GetFrames)
	                                     .def("GetMesh", &Lua::MeshVertexAnimation::GetMesh);
	classDefMeshVertexAnimation.scope[classDefMeshVertexFrame];
	classDefVertexAnimation.scope[classDefMeshVertexAnimation];

	auto classDefSkeleton = luabind::class_<pragma::animation::Skeleton>("Skeleton");
	classDefSkeleton.scope[luabind::def(
	  "create", +[]() { return std::make_shared<pragma::animation::Skeleton>(); })];
	classDefSkeleton.def(
	  "DebugPrint", +[](const pragma::animation::Skeleton &skeleton) {
		  std::stringstream ss;
		  std::function<void(const std::string &, const pragma::animation::Bone &)> printBone = nullptr;
		  printBone = [&ss, &printBone](const std::string &t, const pragma::animation::Bone &bone) {
			  ss << t << bone.name << "\n";
			  for(auto &[childId, child] : bone.children)
				  printBone(t + "\t", *child);
		  };
		  for(auto &[boneId, bone] : skeleton.GetRootBones())
			  printBone("", *bone);
		  Con::cout << ss.str() << Con::endl;
	  });
	classDefSkeleton.def("GetBone", &Lua::Skeleton::GetBone);
	classDefSkeleton.def("GetRootBones", &Lua::Skeleton::GetRootBones);
	classDefSkeleton.def("GetBones", &Lua::Skeleton::GetBones);
	classDefSkeleton.def("LookupBone", &Lua::Skeleton::LookupBone);
	classDefSkeleton.def("AddBone", static_cast<std::shared_ptr<pragma::animation::Bone> (*)(lua_State *, pragma::animation::Skeleton &, const std::string &, pragma::animation::Bone &)>(&Lua::Skeleton::AddBone));
	classDefSkeleton.def("AddBone", static_cast<std::shared_ptr<pragma::animation::Bone> (*)(lua_State *, pragma::animation::Skeleton &, const std::string &)>(&Lua::Skeleton::AddBone));
	classDefSkeleton.def("GetBoneCount", &pragma::animation::Skeleton::GetBoneCount);
	classDefSkeleton.def("Merge", &pragma::animation::Skeleton::Merge);
	classDefSkeleton.def("ClearBones", &Lua::Skeleton::ClearBones);
	classDefSkeleton.def("MakeRootBone", Lua::Skeleton::MakeRootBone);
	classDefSkeleton.def("GetBoneHierarchy", Lua::Skeleton::GetBoneHierarchy);
	classDefSkeleton.def("IsRootBone", static_cast<bool (*)(lua_State *, pragma::animation::Skeleton &, const std::string &)>(&Lua::Skeleton::IsRootBone));
	classDefSkeleton.def("IsRootBone", static_cast<bool (*)(lua_State *, pragma::animation::Skeleton &, uint32_t)>(&Lua::Skeleton::IsRootBone));
	classDefSkeleton.def(
	  "ToDebugString", +[](const pragma::animation::Skeleton &skeleton) -> std::string {
		  std::stringstream ss;
		  std::function<void(const pragma::animation::Bone &, const std::string &t)> fprint = nullptr;
		  fprint = [&fprint, &ss](const pragma::animation::Bone &bone, const std::string &t) {
			  ss << t << bone.name << "\n";
			  for(auto &pair : bone.children)
				  fprint(*pair.second, t + "\t");
		  };
		  for(auto &pair : skeleton.GetRootBones())
			  fprint(*pair.second, "");
		  return ss.str();
	  });
	Lua::Bone::register_class(l, classDefSkeleton);

	auto modelMeshGroupClassDef = luabind::class_<::ModelMeshGroup>("MeshGroup");
	Lua::ModelMeshGroup::register_class(modelMeshGroupClassDef);

	auto collisionMeshClassDef = luabind::class_<::CollisionMesh>("CollisionMesh");
	Lua::CollisionMesh::register_class(collisionMeshClassDef);

	// Vertex
	auto defVertex = luabind::class_<umath::Vertex>("Vertex");
	defVertex.def(luabind::constructor<const Vector3 &, const ::Vector2 &, const Vector3 &, const ::Vector4 &>());
	defVertex.def(luabind::constructor<const Vector3 &, const ::Vector2 &, const Vector3 &>());
	defVertex.def(luabind::constructor<const Vector3 &, const Vector3 &>());
	defVertex.def(luabind::constructor<>());
	defVertex.def(luabind::tostring(luabind::self));
	defVertex.def(luabind::const_self == luabind::const_self);
	defVertex.def_readwrite("position", &umath::Vertex::position);
	defVertex.def_readwrite("uv", &umath::Vertex::uv);
	defVertex.def_readwrite("normal", &umath::Vertex::normal);
	defVertex.def_readwrite("tangent", &umath::Vertex::tangent);
	defVertex.def("Copy", &Lua::Vertex::Copy);
	defVertex.def("GetBiTangent", &umath::Vertex::GetBiTangent);
	classDef.scope[defVertex];

	auto defVertWeight = luabind::class_<umath::VertexWeight>("VertexWeight");
	defVertWeight.def(luabind::constructor<const ::Vector4i &, const ::Vector4 &>());
	defVertWeight.def(luabind::constructor<>());
	defVertWeight.def(luabind::tostring(luabind::self));
	defVertWeight.def(luabind::const_self == luabind::const_self);
	defVertWeight.def_readwrite("boneIds", &umath::VertexWeight::boneIds);
	defVertWeight.def_readwrite("weights", &umath::VertexWeight::weights);
	defVertWeight.def("Copy", &Lua::VertexWeight::Copy);
	classDef.scope[defVertWeight];

	// Joint
	auto defJoint = luabind::class_<JointInfo>("Joint");
	defJoint.def_readwrite("collide", &JointInfo::collide);

	defJoint.def("GetType", &Lua::Joint::GetType);
	defJoint.def("GetChildBoneId", &Lua::Joint::GetChildBoneId);
	defJoint.def("GetParentBoneId", &Lua::Joint::GetParentBoneId);
	defJoint.def("GetCollisionsEnabled", &Lua::Joint::GetCollisionsEnabled);
	defJoint.def("GetKeyValues", &Lua::Joint::GetKeyValues);

	defJoint.def("SetType", &Lua::Joint::SetType);
	defJoint.def("SetCollisionMeshId", &Lua::Joint::SetCollisionMeshId);
	defJoint.def("SetParentCollisionMeshId", &Lua::Joint::SetParentCollisionMeshId);
	defJoint.def("SetCollisionsEnabled", &Lua::Joint::SetCollisionsEnabled);
	defJoint.def("SetKeyValues", &Lua::Joint::SetKeyValues);
	defJoint.def("SetKeyValue", &Lua::Joint::SetKeyValue);
	defJoint.def("RemoveKeyValue", &Lua::Joint::RemoveKeyValue);
	defJoint.def("GetArgs", static_cast<luabind::object (*)(lua_State *, JointInfo &)>([](lua_State *l, JointInfo &jointInfo) -> luabind::object { return Lua::map_to_table(l, jointInfo.args); }));
	defJoint.def("SetArgs", static_cast<void (*)(lua_State *, JointInfo &, luabind::table<>)>([](lua_State *l, JointInfo &jointInfo, luabind::table<> t) { jointInfo.args = Lua::table_to_map<std::string, std::string>(l, t, 2); }));

	defJoint.add_static_constant("TYPE_NONE", umath::to_integral(JointType::None));
	defJoint.add_static_constant("TYPE_FIXED", umath::to_integral(JointType::Fixed));
	defJoint.add_static_constant("TYPE_BALLSOCKET", umath::to_integral(JointType::BallSocket));
	defJoint.add_static_constant("TYPE_HINGE", umath::to_integral(JointType::Hinge));
	defJoint.add_static_constant("TYPE_SLIDER", umath::to_integral(JointType::Slider));
	defJoint.add_static_constant("TYPE_CONETWIST", umath::to_integral(JointType::ConeTwist));
	defJoint.add_static_constant("TYPE_DOF", umath::to_integral(JointType::DOF));
	classDef.scope[defJoint];

	auto defBoxCreateInfo = luabind::class_<pragma::model::BoxCreateInfo>("BoxCreateInfo");
	defBoxCreateInfo.def(luabind::constructor<const Vector3 &, const Vector3 &>());
	defBoxCreateInfo.def(luabind::constructor<>());
	defBoxCreateInfo.def_readwrite("min", &pragma::model::BoxCreateInfo::min);
	defBoxCreateInfo.def_readwrite("max", &pragma::model::BoxCreateInfo::max);
	classDef.scope[defBoxCreateInfo];

	auto defSphereCreateInfo = luabind::class_<pragma::model::SphereCreateInfo>("SphereCreateInfo");
	defSphereCreateInfo.def(luabind::constructor<const Vector3 &, float>());
	defSphereCreateInfo.def(luabind::constructor<>());
	defSphereCreateInfo.def_readwrite("origin", &pragma::model::SphereCreateInfo::origin);
	defSphereCreateInfo.def_readwrite("radius", &pragma::model::SphereCreateInfo::radius);
	defSphereCreateInfo.def_readwrite("recursionLevel", &pragma::model::SphereCreateInfo::recursionLevel);
	classDef.scope[defSphereCreateInfo];

	auto defCylinderCreateInfo = luabind::class_<pragma::model::CylinderCreateInfo>("CylinderCreateInfo");
	defCylinderCreateInfo.def(luabind::constructor<float, float>());
	defCylinderCreateInfo.def(luabind::constructor<>());
	defCylinderCreateInfo.def_readwrite("radius", &pragma::model::CylinderCreateInfo::radius);
	defCylinderCreateInfo.def_readwrite("length", &pragma::model::CylinderCreateInfo::length);
	defCylinderCreateInfo.def_readwrite("segmentCount", &pragma::model::CylinderCreateInfo::segmentCount);
	classDef.scope[defCylinderCreateInfo];

	auto defConeCreateInfo = luabind::class_<pragma::model::ConeCreateInfo>("ConeCreateInfo");
	defConeCreateInfo.def(luabind::constructor<umath::Degree, float>());
	defConeCreateInfo.def(luabind::constructor<float, float, float>());
	defConeCreateInfo.def(luabind::constructor<>());
	defConeCreateInfo.def_readwrite("startRadius", &pragma::model::ConeCreateInfo::startRadius);
	defConeCreateInfo.def_readwrite("length", &pragma::model::ConeCreateInfo::length);
	defConeCreateInfo.def_readwrite("endRadius", &pragma::model::ConeCreateInfo::endRadius);
	defConeCreateInfo.def_readwrite("segmentCount", &pragma::model::ConeCreateInfo::segmentCount);
	classDef.scope[defConeCreateInfo];

	auto defEllipticConeCreateInfo = luabind::class_<pragma::model::EllipticConeCreateInfo, pragma::model::ConeCreateInfo>("EllipticConeCreateInfo");
	defEllipticConeCreateInfo.def(luabind::constructor<umath::Degree, umath::Degree, float>());
	defEllipticConeCreateInfo.def(luabind::constructor<float, float, float, float, float>());
	defEllipticConeCreateInfo.def(luabind::constructor<>());
	defEllipticConeCreateInfo.def_readwrite("startRadiusY", &pragma::model::EllipticConeCreateInfo::startRadiusY);
	defEllipticConeCreateInfo.def_readwrite("endRadiusY", &pragma::model::EllipticConeCreateInfo::endRadiusY);
	classDef.scope[defEllipticConeCreateInfo];

	auto defCircleCreateInfo = luabind::class_<pragma::model::CircleCreateInfo>("CircleCreateInfo");
	defCircleCreateInfo.def(luabind::constructor<float, bool>());
	defCircleCreateInfo.def(luabind::constructor<float, bool>(), luabind::default_parameter_policy<2, true> {});
	defCircleCreateInfo.def(luabind::constructor<>());
	defCircleCreateInfo.def_readwrite("radius", &pragma::model::CircleCreateInfo::radius);
	defCircleCreateInfo.def_readwrite("doubleSided", &pragma::model::CircleCreateInfo::doubleSided);
	defCircleCreateInfo.def_readwrite("segmentCount", &pragma::model::CircleCreateInfo::segmentCount);
	defCircleCreateInfo.def_readwrite("totalAngle", &pragma::model::CircleCreateInfo::totalAngle);
	classDef.scope[defCircleCreateInfo];

	auto defRingCreateInfo = luabind::class_<pragma::model::RingCreateInfo>("RingCreateInfo");
	defRingCreateInfo.def(luabind::constructor<float, float, bool>());
	defRingCreateInfo.def(luabind::constructor<float, float, bool>(), luabind::default_parameter_policy<3, true> {});
	defRingCreateInfo.def(luabind::constructor<>());
	defRingCreateInfo.def_readwrite("innerRadius", &pragma::model::RingCreateInfo::innerRadius);
	defRingCreateInfo.def_readwrite("outerRadius", &pragma::model::RingCreateInfo::outerRadius);
	defRingCreateInfo.def_readwrite("doubleSided", &pragma::model::RingCreateInfo::doubleSided);
	defRingCreateInfo.def_readwrite("segmentCount", &pragma::model::RingCreateInfo::segmentCount);
	defRingCreateInfo.def_readwrite("totalAngle", &pragma::model::RingCreateInfo::totalAngle);
	classDef.scope[defRingCreateInfo];

	// Assign definitions
	classDef.scope[classDefSkeleton];
	classDef.scope[modelMeshGroupClassDef];
	classDef.scope[collisionMeshClassDef];
	classDef.scope[classDefAnimation];
	classDef.scope[classDefVertexAnimation];
	classDefModelMesh.scope[classDefModelSubMesh];
	classDef.scope[classDefModelMesh];
}

void Lua::Model::GetCollisionMeshes(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &meshes = mdl.GetCollisionMeshes();
	lua_newtable(l);
	int top = lua_gettop(l);
	for(auto i = decltype(meshes.size()) {0}; i < meshes.size(); ++i) {
		Lua::Push<std::shared_ptr<::CollisionMesh>>(l, meshes[i]);
		lua_rawseti(l, top, i + 1);
	}
}

void Lua::Model::ClearCollisionMeshes(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.GetCollisionMeshes().clear();
}

void Lua::Model::GetSkeleton(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &skeleton = mdl.GetSkeleton();
	luabind::object(l, &skeleton).push(l);
}

static void push_attachment(lua_State *l, const Attachment &att)
{
	auto tAtt = Lua::CreateTable(l);

	Lua::PushString(l, "angles");
	Lua::Push<EulerAngles>(l, att.angles);
	Lua::SetTableValue(l, tAtt);

	Lua::PushString(l, "bone");
	Lua::PushInt(l, att.bone);
	Lua::SetTableValue(l, tAtt);

	Lua::PushString(l, "name");
	Lua::PushString(l, att.name);
	Lua::SetTableValue(l, tAtt);

	Lua::PushString(l, "offset");
	Lua::Push<Vector3>(l, att.offset);
	Lua::SetTableValue(l, tAtt);
}
void Lua::Model::GetAttachmentCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetAttachments().size());
}
void Lua::Model::GetAttachment(lua_State *l, ::Model &mdl, int32_t attId)
{
	//Lua::CheckModel(l,1);
	auto *att = mdl.GetAttachment(attId);
	if(att == nullptr)
		return;
	push_attachment(l, *att);
}
void Lua::Model::GetAttachment(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *att = mdl.GetAttachment(name);
	if(att == nullptr)
		return;
	push_attachment(l, *att);
}
void Lua::Model::GetAttachments(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &attachments = mdl.GetAttachments();
	auto t = Lua::CreateTable(l);
	for(auto i = decltype(attachments.size()) {0}; i < attachments.size(); ++i) {
		auto &att = attachments[i];

		Lua::PushInt(l, i + 1);
		push_attachment(l, att);

		Lua::SetTableValue(l, t);
	}
}

static void push_blend_controller(lua_State *l, const BlendController &blendController)
{
	auto tController = Lua::CreateTable(l);

	Lua::PushString(l, "loop");
	Lua::PushBool(l, blendController.loop);
	Lua::SetTableValue(l, tController);

	Lua::PushString(l, "min");
	Lua::PushInt(l, blendController.min);
	Lua::SetTableValue(l, tController);

	Lua::PushString(l, "max");
	Lua::PushInt(l, blendController.max);
	Lua::SetTableValue(l, tController);

	Lua::PushString(l, "name");
	Lua::PushString(l, blendController.name);
	Lua::SetTableValue(l, tController);
}
void Lua::Model::LookupAnimation(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto id = mdl.LookupAnimation(name);
	Lua::PushInt(l, id);
}
void Lua::Model::LookupAttachment(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto attId = mdl.LookupAttachment(name);
	Lua::PushInt(l, attId);
}
void Lua::Model::LookupBone(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto boneId = mdl.LookupBone(name);
	Lua::PushInt(l, boneId);
}
void Lua::Model::AddAttachment(lua_State *l, ::Model &mdl, const std::string &name, const std::string &boneName, const Vector3 &offset, const EulerAngles &ang)
{
	//Lua::CheckModel(l,1);
	auto boneId = mdl.LookupBone(boneName);
	if(boneId < 0)
		return;
	AddAttachment(l, mdl, name, boneId, offset, ang);
}
void Lua::Model::AddAttachment(lua_State *l, ::Model &mdl, const std::string &name, uint32_t boneId, const Vector3 &offset, const EulerAngles &ang)
{
	//Lua::CheckModel(l,1);
	mdl.AddAttachment(name, boneId, offset, ang);
}
struct LuaAttachmentData {
	EulerAngles *angles = nullptr;
	int32_t bone = -1;
	const char *name = nullptr;
	Vector3 *offset = nullptr;
};
static void get_attachment(lua_State *l, LuaAttachmentData &att, int32_t t)
{
	Lua::PushString(l, "angles");
	Lua::GetTableValue(l, t);
	if(Lua::IsNil(l, -1) == false)
		att.angles = Lua::CheckEulerAngles(l, -1);
	Lua::Pop(l, 1);

	Lua::PushString(l, "bone");
	Lua::GetTableValue(l, t);
	if(Lua::IsNil(l, -1) == false)
		att.bone = Lua::CheckInt(l, -1);
	Lua::Pop(l, 1);

	Lua::PushString(l, "name");
	Lua::GetTableValue(l, t);
	if(Lua::IsNil(l, -1) == false)
		att.name = Lua::CheckString(l, -1);
	Lua::Pop(l, 1);

	Lua::PushString(l, "offset");
	Lua::GetTableValue(l, t);
	if(Lua::IsNil(l, -1) == false)
		att.offset = Lua::CheckVector(l, -1);
	Lua::Pop(l, 1);
}
void Lua::Model::SetAttachmentData(lua_State *l, ::Model &mdl, const std::string &name, luabind::object data)
{
	//Lua::CheckModel(l,1);
	int32_t t = 3;
	Lua::CheckTable(l, t);
	LuaAttachmentData attNew {};
	get_attachment(l, attNew, t);
	auto *att = mdl.GetAttachment(name);
	if(att == nullptr)
		return;
	if(attNew.angles != nullptr)
		att->angles = *attNew.angles;
	if(attNew.name != nullptr)
		att->name = attNew.name;
	if(attNew.offset != nullptr)
		att->offset = *attNew.offset;
}
void Lua::Model::SetAttachmentData(lua_State *l, ::Model &mdl, uint32_t attId, luabind::object data)
{
	//Lua::CheckModel(l,1);
	int32_t t = 3;
	Lua::CheckTable(l, t);
	LuaAttachmentData attNew {};
	get_attachment(l, attNew, t);
	auto *att = mdl.GetAttachment(attId);
	if(att == nullptr)
		return;
	if(attNew.angles != nullptr)
		att->angles = *attNew.angles;
	if(attNew.name != nullptr)
		att->name = attNew.name;
	if(attNew.offset != nullptr)
		att->offset = *attNew.offset;
}
void Lua::Model::RemoveAttachment(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveAttachment(name);
}
void Lua::Model::RemoveAttachment(lua_State *l, ::Model &mdl, uint32_t attId)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveAttachment(attId);
}
void Lua::Model::GetBlendControllerCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetBlendControllers().size());
}
void Lua::Model::GetBlendController(lua_State *l, ::Model &mdl, int32_t blendControllerId)
{
	//Lua::CheckModel(l,1);
	auto *blendController = mdl.GetBlendController(blendControllerId);
	if(blendController == nullptr)
		return;
	push_blend_controller(l, *blendController);
}
void Lua::Model::GetBlendController(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *blendController = mdl.GetBlendController(name);
	if(blendController == nullptr)
		return;
	push_blend_controller(l, *blendController);
}
void Lua::Model::GetBlendControllers(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto t = Lua::CreateTable(l);
	auto &blendControllers = mdl.GetBlendControllers();
	for(auto i = decltype(blendControllers.size()) {0}; i < blendControllers.size(); ++i) {
		auto &blendController = blendControllers[i];

		Lua::PushInt(l, i + 1);
		push_blend_controller(l, blendController);

		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::LookupBlendController(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.LookupBlendController(name));
}

void Lua::Model::GetAnimationCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	std::unordered_map<std::string, uint32_t> *anims;
	mdl.GetAnimations(&anims);
	Lua::PushInt(l, anims->size());
}

void Lua::Model::GetAnimationNames(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	std::unordered_map<std::string, unsigned int> *anims;
	mdl.GetAnimations(&anims);
	std::unordered_map<std::string, unsigned int>::iterator i;
	lua_newtable(l);
	int top = lua_gettop(l);
	int n = 1;
	for(i = anims->begin(); i != anims->end(); i++) {
		lua_pushstring(l, i->first.c_str());
		lua_rawseti(l, top, n);
		n++;
	}
}

void Lua::Model::GetAnimations(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto t = Lua::CreateTable(l);
	auto &anims = mdl.GetAnimations();
	int32_t idx = 1;
	for(auto &anim : anims) {
		Lua::PushInt(l, idx++);
		Lua::Push<std::shared_ptr<pragma::animation::Animation>>(l, anim);
		Lua::SetTableValue(l, t);
	}
}

void Lua::Model::GetAnimationName(lua_State *l, ::Model &mdl, unsigned int animID)
{
	//Lua::CheckModel(l,1);
	std::string name;
	if(mdl.GetAnimationName(animID, name) == false)
		return;
	Lua::PushString(l, name);
}

void Lua::Model::GetAnimation(lua_State *l, ::Model &mdl, unsigned int animID)
{
	//Lua::CheckModel(l,1);
	auto anim = mdl.GetAnimation(animID);
	if(anim == nullptr)
		return;
	Lua::Push<std::shared_ptr<pragma::animation::Animation>>(l, anim);
}

void Lua::Model::GetAnimation(lua_State *l, ::Model &mdl, const char *name)
{
	//Lua::CheckModel(l,1);
	int animID = mdl.LookupAnimation(name);
	if(animID == -1)
		return;
	GetAnimation(l, mdl, animID);
}

void Lua::Model::PrecacheTextureGroup(lua_State *, ::Model &mdl, unsigned int group)
{
	//Lua::CheckModel(l,1);
	mdl.PrecacheTextureGroup(group);
}

void Lua::Model::PrecacheTextureGroups(lua_State *, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.PrecacheTextureGroups();
}

void Lua::Model::GetReference(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &ref = mdl.GetReference();
	Lua::Push<std::shared_ptr<::Frame>>(l, ref.shared_from_this());
}
/*void Lua::Model::GetReferenceBoneMatrix(lua_State *l,::Model &mdl,uint32_t boneId)
{
	//Lua::CheckModel(l,1);
	auto *mat = mdl.GetBindPoseBoneMatrix(boneId);
	if(mat == nullptr)
		return;
	Lua::Push<Mat4>(l,*mat);
}
void Lua::Model::SetReferenceBoneMatrix(lua_State *l,::Model &mdl,uint32_t boneId,const Mat4 &mat)
{
	//Lua::CheckModel(l,1);
	mdl.SetBindPoseBoneMatrix(boneId,mat);
}*/
void Lua::Model::GetLocalBonePosition(lua_State *l, ::Model &mdl, UInt32 animId, UInt32 frameId, UInt32 boneId)
{
	//Lua::CheckModel(l,1);
	Vector3 pos;
	Quat rot;
	Vector3 scale;
	mdl.GetLocalBonePosition(animId, frameId, boneId, pos, rot, &scale);
	Lua::Push<Vector3>(l, pos);
	Lua::Push<Quat>(l, rot);
	Lua::Push<Vector3>(l, scale);
}
void Lua::Model::LookupBodyGroup(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto id = mdl.GetBodyGroupId(name);
	Lua::PushInt(l, id);
}
void Lua::Model::GetBaseMeshGroupIds(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &baseMeshes = mdl.GetBaseMeshes();
	auto t = Lua::CreateTable(l);
	int32_t n = 1;
	for(auto &meshId : baseMeshes) {
		Lua::PushInt(l, n);
		Lua::PushInt(l, meshId);
		Lua::SetTableValue(l, t);
		++n;
	}
}
void Lua::Model::SetBaseMeshGroupIds(lua_State *l, ::Model &mdl, luabind::object o)
{
	//Lua::CheckModel(l,1);
	int32_t tIdx = 2;
	Lua::CheckTable(l, tIdx);
	std::vector<uint32_t> ids;
	auto numIds = Lua::GetObjectLength(l, tIdx);
	ids.reserve(numIds);
	for(auto i = decltype(numIds) {0}; i < numIds; ++i) {
		Lua::PushInt(l, i + 1);
		Lua::GetTableValue(l, tIdx);
		auto groupId = Lua::CheckInt(l, -1);
		ids.push_back(groupId);

		Lua::Pop(l, 1);
	}
	mdl.GetBaseMeshes() = ids;
}
void Lua::Model::AddBaseMeshGroupId(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto groupId = Lua::CheckInt(l, 1);
	auto &ids = mdl.GetBaseMeshes();
	auto it = std::find(ids.begin(), ids.end(), groupId);
	if(it != ids.end())
		return;
	ids.push_back(groupId);
}
void Lua::Model::GetMeshGroupId(lua_State *l, ::Model &mdl, uint32_t bodyGroupId, uint32_t groupId)
{
	//Lua::CheckModel(l,1);
	uint32_t meshId = uint32_t(-1);
	auto r = mdl.GetMesh(bodyGroupId, groupId, meshId);
	UNUSED(r);
	Lua::PushInt(l, static_cast<int32_t>(meshId));
}

void Lua::Model::GetMeshGroup(lua_State *l, ::Model &mdl, const std::string &meshGroupName)
{
	//Lua::CheckModel(l,1);
	auto meshGroup = mdl.GetMeshGroup(meshGroupName);
	if(meshGroup == nullptr)
		return;
	Lua::Push<std::shared_ptr<::ModelMeshGroup>>(l, meshGroup);
}

void Lua::Model::GetMeshGroup(lua_State *l, ::Model &mdl, uint32_t mgId)
{
	//Lua::CheckModel(l,1);
	auto group = mdl.GetMeshGroup(mgId);
	if(group == nullptr)
		return;
	Lua::Push<decltype(group)>(l, group);
}

void Lua::Model::GetMeshes(lua_State *l, ::Model &mdl, const std::string &meshGroup)
{
	//Lua::CheckModel(l,1);
	auto *meshes = mdl.GetMeshes(meshGroup);
	auto t = Lua::CreateTable(l);
	if(meshes != nullptr) {
		int32_t n = 1;
		for(auto &mesh : *meshes) {
			Lua::PushInt(l, n);
			Lua::Push<std::shared_ptr<ModelMesh>>(l, mesh);
			Lua::SetTableValue(l, t);
			++n;
		}
	}
}

void Lua::Model::GetMeshes(lua_State *l, ::Model &mdl, luabind::object o)
{
	//Lua::CheckModel(l,1);
	Lua::CheckTable(l, 2);
	std::vector<uint32_t> meshIds;
	Lua::PushNil(l);
	while(Lua::GetNextPair(l, 2) != 0) {
		auto meshId = Lua::CheckInt(l, -1);
		meshIds.push_back(static_cast<uint32_t>(meshId));
		Lua::Pop(l, 1);
	}

	std::vector<std::shared_ptr<::ModelMesh>> meshes;
	mdl.GetMeshes(meshIds, meshes);

	auto t = Lua::CreateTable(l);
	int32_t n = 1;
	for(auto &mesh : meshes) {
		Lua::PushInt(l, n);
		Lua::Push<std::shared_ptr<ModelMesh>>(l, mesh);
		Lua::SetTableValue(l, t);
		++n;
	}
}

void Lua::Model::GetMeshGroups(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &meshGroups = mdl.GetMeshGroups();
	auto t = Lua::CreateTable(l);
	int32_t n = 1;
	for(auto &meshGroup : meshGroups) {
		Lua::PushInt(l, n);
		Lua::Push<std::shared_ptr<::ModelMeshGroup>>(l, meshGroup);
		Lua::SetTableValue(l, t);
		++n;
	}
}

void Lua::Model::AddMeshGroup(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto meshGroup = mdl.AddMeshGroup(name);
	Lua::Push<std::shared_ptr<::ModelMeshGroup>>(l, meshGroup);
}

void Lua::Model::AddMeshGroup(lua_State *, ::Model &mdl, ::ModelMeshGroup &meshGroup)
{
	//Lua::CheckModel(l,1);
	auto pMeshGroup = meshGroup.shared_from_this();
	mdl.AddMeshGroup(pMeshGroup);
}

void Lua::Model::UpdateCollisionBounds(lua_State *, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.CalculateCollisionBounds();
}
void Lua::Model::UpdateRenderBounds(lua_State *, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.CalculateRenderBounds();
}
void Lua::Model::Update(lua_State *, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.Update();
}
void Lua::Model::Update(lua_State *, ::Model &mdl, uint32_t flags)
{
	//Lua::CheckModel(l,1);
	mdl.Update(static_cast<ModelUpdateFlags>(flags));
}
void Lua::Model::GetName(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushString(l, mdl.GetName());
}
void Lua::Model::GetMass(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushNumber(l, mdl.GetMass());
}
void Lua::Model::SetMass(lua_State *, ::Model &mdl, float mass)
{
	//Lua::CheckModel(l,1);
	mdl.SetMass(mass);
}
void Lua::Model::GetBoneCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetBoneCount());
}
void Lua::Model::GetCollisionBounds(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Vector3 min, max;
	mdl.GetCollisionBounds(min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}
void Lua::Model::GetRenderBounds(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Vector3 min, max;
	mdl.GetRenderBounds(min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}
void Lua::Model::SetCollisionBounds(lua_State *, ::Model &mdl, const Vector3 &min, const Vector3 &max)
{
	//Lua::CheckModel(l,1);
	mdl.SetCollisionBounds(min, max);
}
void Lua::Model::SetRenderBounds(lua_State *, ::Model &mdl, const Vector3 &min, const Vector3 &max)
{
	//Lua::CheckModel(l,1);
	mdl.SetRenderBounds(min, max);
}
void Lua::Model::AddCollisionMesh(lua_State *, ::Model &mdl, ::CollisionMesh &colMesh)
{
	//Lua::CheckModel(l,1);
	mdl.AddCollisionMesh(colMesh.shared_from_this());
}
void Lua::Model::AddMaterial(lua_State *l, ::Model &mdl, uint32_t textureGroup, Material *mat)
{
	if(!mat)
		return;
	//Lua::CheckModel(l,1);
	std::optional<uint32_t> skinTexIdx {};
	auto r = mdl.AddMaterial(textureGroup, mat, {}, &skinTexIdx);
	Lua::PushInt(l, r);
	if(skinTexIdx.has_value())
		Lua::PushInt(l, *skinTexIdx);
}
void Lua::Model::SetMaterial(lua_State *l, ::Model &mdl, uint32_t matId, ::Material *mat)
{
	//Lua::CheckModel(l,1);
	mdl.SetMaterial(matId, mat);
}
void Lua::Model::GetMaterials(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto *nw = engine->GetNetworkState(l);
	auto &matManager = nw->GetMaterialManager();
	auto t = Lua::CreateTable(l);
	uint32_t idx = 1;
	auto &mats = mdl.GetMaterials();
	for(auto &mat : mats) {
		auto *pmat = mat.get();
		if(pmat == nullptr)
			pmat = matManager.GetErrorMaterial();
		Lua::PushInt(l, idx++);
		Lua::Push<Material *>(l, pmat);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetMaterialCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetMaterials().size());
}
void Lua::Model::GetMeshGroupCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetMeshGroupCount());
}
void Lua::Model::GetMeshCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetMeshCount());
}
void Lua::Model::GetSubMeshCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetSubMeshCount());
}
void Lua::Model::GetCollisionMeshCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetCollisionMeshCount());
}
void Lua::Model::GetBodyGroupId(lua_State *l, ::Model &mdl, const std::string &bodyGroupName)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetBodyGroupId(bodyGroupName));
}
void Lua::Model::GetBodyGroupCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetBodyGroupCount());
}
static void push_body_group(lua_State *l, const BodyGroup &bg)
{
	auto t = Lua::CreateTable(l);
	Lua::PushString(l, "name");
	Lua::PushString(l, bg.name);
	Lua::SetTableValue(l, t);

	Lua::PushString(l, "meshGroups");
	auto tMg = Lua::CreateTable(l);
	for(auto i = decltype(bg.meshGroups.size()) {0u}; i < bg.meshGroups.size(); ++i) {
		Lua::PushInt(l, i + 1u);
		Lua::PushInt(l, bg.meshGroups.at(i));
		Lua::SetTableValue(l, tMg);
	}
	Lua::SetTableValue(l, t);
}
void Lua::Model::GetBodyGroups(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto t = Lua::CreateTable(l);
	auto &bodyGroups = mdl.GetBodyGroups();
	for(auto i = decltype(bodyGroups.size()) {0u}; i < bodyGroups.size(); ++i) {
		auto &bg = bodyGroups.at(i);
		Lua::PushInt(l, i + 1u);
		push_body_group(l, bg);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetBodyGroup(lua_State *l, ::Model &mdl, uint32_t bgId)
{
	//Lua::CheckModel(l,1);
	auto *bg = mdl.GetBodyGroup(bgId);
	if(bg == nullptr)
		return;
	push_body_group(l, *bg);
}
void Lua::Model::AddHitbox(lua_State *, ::Model &mdl, uint32_t boneId, uint32_t hitGroup, const Vector3 &min, const Vector3 &max)
{
	//Lua::CheckModel(l,1);
	mdl.AddHitbox(boneId, static_cast<HitGroup>(hitGroup), min, max);
}
void Lua::Model::GetHitboxCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetHitboxCount());
}
void Lua::Model::GetHitboxGroup(lua_State *l, ::Model &mdl, uint32_t boneId)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetHitboxGroup(boneId));
}
void Lua::Model::GetHitboxBounds(lua_State *l, ::Model &mdl, uint32_t boneId)
{
	//Lua::CheckModel(l,1);
	Vector3 min {0.f, 0.f, 0.f};
	Vector3 max {0.f, 0.f, 0.f};
	mdl.GetHitboxBounds(boneId, min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}
void Lua::Model::GetHitboxBones(lua_State *l, ::Model &mdl, uint32_t hitGroup)
{
	//Lua::CheckModel(l,1);
	auto boneIds = mdl.GetHitboxBones(static_cast<HitGroup>(hitGroup));
	auto t = Lua::CreateTable(l);
	for(auto i = decltype(boneIds.size()) {0}; i < boneIds.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::PushInt(l, boneIds[i]);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetHitboxBones(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto boneIds = mdl.GetHitboxBones();
	auto t = Lua::CreateTable(l);
	for(auto i = decltype(boneIds.size()) {0}; i < boneIds.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::PushInt(l, boneIds[i]);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::SetHitboxGroup(lua_State *l, ::Model &mdl, uint32_t boneId, uint32_t hitGroup)
{
	//Lua::CheckModel(l,1);
	auto &hitboxes = mdl.GetHitboxes();
	auto it = hitboxes.find(boneId);
	if(it == hitboxes.end())
		return;
	it->second.group = static_cast<HitGroup>(hitGroup);
}
void Lua::Model::SetHitboxBounds(lua_State *l, ::Model &mdl, uint32_t boneId, const Vector3 &min, const Vector3 &max)
{
	//Lua::CheckModel(l,1);
	auto &hitboxes = mdl.GetHitboxes();
	auto it = hitboxes.find(boneId);
	if(it == hitboxes.end())
		return;
	it->second.min = min;
	it->second.max = max;
}
void Lua::Model::RemoveHitbox(lua_State *l, ::Model &mdl, uint32_t boneId)
{
	//Lua::CheckModel(l,1);
	auto &hitboxes = mdl.GetHitboxes();
	auto it = hitboxes.find(boneId);
	if(it == hitboxes.end())
		return;
	hitboxes.erase(it);
}
void Lua::Model::GetTextureGroupCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetTextureGroups().size());
}
static void push_texture_group(lua_State *l, TextureGroup &group)
{
	auto tGroup = Lua::CreateTable(l);
	for(auto j = decltype(group.textures.size()) {0}; j < group.textures.size(); ++j) {
		Lua::PushInt(l, j + 1);
		Lua::PushInt(l, group.textures[j]);
		Lua::SetTableValue(l, tGroup);
	}
}
void Lua::Model::GetTextureGroups(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &texGroups = mdl.GetTextureGroups();
	auto t = Lua::CreateTable(l);
	for(auto i = decltype(texGroups.size()) {0}; i < texGroups.size(); ++i) {
		auto &group = texGroups[i];

		Lua::PushInt(l, i + 1);
		push_texture_group(l, group);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetTextureGroup(lua_State *l, ::Model &mdl, uint32_t id)
{
	//Lua::CheckModel(l,1);
	auto *group = mdl.GetTextureGroup(id);
	if(group == nullptr)
		return;
	push_texture_group(l, *group);
}

void Lua::Model::Save(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto mdlName = name;
	std::string rootPath;
	if(Lua::file::validate_write_operation(l, mdlName, rootPath) == false) {
		Lua::PushBool(l, false);
		return;
	}
	auto r = mdl.SaveLegacy(engine->GetNetworkState(l)->GetGameState(), mdlName, rootPath);
	Lua::PushBool(l, r);
}

void Lua::Model::Copy(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto cpy = mdl.Copy(engine->GetNetworkState(l)->GetGameState());
	Lua::Push<decltype(cpy)>(l, cpy);
}

void Lua::Model::Copy(lua_State *l, ::Model &mdl, uint32_t copyFlags)
{
	//Lua::CheckModel(l,1);
	auto cpy = mdl.Copy(engine->GetNetworkState(l)->GetGameState(), static_cast<::Model::CopyFlags>(copyFlags));
	Lua::Push<decltype(cpy)>(l, cpy);
}

void Lua::Model::GetVertexCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetVertexCount());
}
void Lua::Model::GetTriangleCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetTriangleCount());
}
void Lua::Model::GetTextures(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &meta = mdl.GetMetaInfo();
	auto tTextures = Lua::CreateTable(l);
	for(auto i = decltype(meta.textures.size()) {0}; i < meta.textures.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::PushString(l, meta.textures[i]);
		Lua::SetTableValue(l, tTextures);
	}
}
void Lua::Model::GetTexturePaths(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &meta = mdl.GetMetaInfo();
	auto tTexturePaths = Lua::CreateTable(l);
	for(auto i = decltype(meta.texturePaths.size()) {0}; i < meta.texturePaths.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::PushString(l, meta.texturePaths[i]);
		Lua::SetTableValue(l, tTexturePaths);
	}
}
void Lua::Model::LoadMaterials(lua_State *l, ::Model &mdl) { LoadMaterials(l, mdl, false); }
void Lua::Model::LoadMaterials(lua_State *l, ::Model &mdl, bool bReload)
{
	//Lua::CheckModel(l,1);
	auto *nw = engine->GetNetworkState(l);
	mdl.LoadMaterials(bReload);
}
void Lua::Model::AddTexturePath(lua_State *, ::Model &mdl, const std::string &path)
{
	//Lua::CheckModel(l,1);
	mdl.AddTexturePath(path);
}

void Lua::Model::RemoveTexturePath(lua_State *, ::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveTexturePath(idx);
}
void Lua::Model::SetTexturePaths(lua_State *l, ::Model &mdl, luabind::object o)
{
	//Lua::CheckModel(l,1);
	Lua::CheckTable(l, 2);
	std::vector<std::string> texturePaths;
	auto num = Lua::GetObjectLength(l, 2);
	texturePaths.reserve(num);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l, 2) != 0) {
		auto *path = Lua::CheckString(l, -1);
		texturePaths.push_back(path);
		Lua::Pop(l, 1);
	}
	mdl.SetTexturePaths(texturePaths);
}
void Lua::Model::RemoveTexture(lua_State *, ::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveTexture(idx);
}
void Lua::Model::ClearTextures(lua_State *, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.ClearTextures();
}
void Lua::Model::Rotate(lua_State *, ::Model &mdl, const Quat &rot)
{
	//Lua::CheckModel(l,1);
	mdl.Rotate(rot);
}
void Lua::Model::Translate(lua_State *, ::Model &mdl, const Vector3 &t)
{
	//Lua::CheckModel(l,1);
	mdl.Translate(t);
}
void Lua::Model::Scale(lua_State *, ::Model &mdl, const Vector3 &scale)
{
	//Lua::CheckModel(l,1);
	mdl.Scale(scale);
}
void Lua::Model::GetEyeOffset(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::Push<Vector3>(l, mdl.GetEyeOffset());
}
void Lua::Model::SetEyeOffset(lua_State *l, ::Model &mdl, const Vector3 &offset)
{
	//Lua::CheckModel(l,1);
	mdl.SetEyeOffset(offset);
}
void Lua::Model::AddAnimation(lua_State *l, ::Model &mdl, const std::string &name, pragma::animation::Animation &anim)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.AddAnimation(name, anim.shared_from_this()));
}
void Lua::Model::RemoveAnimation(lua_State *l, ::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	auto &anims = mdl.GetAnimations();
	if(idx >= anims.size())
		return;
	anims.erase(anims.begin() + idx);
}
void Lua::Model::ClearAnimations(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.GetAnimations().clear();
	mdl.GetAnimationNames().clear();
}
void Lua::Model::ClipAgainstPlane(lua_State *l, ::Model &mdl, const Vector3 &n, double d, ::Model &clippedMdlA, ::Model &clippedMdlB)
{
	//Lua::CheckModel(l,1);
	mdl.ClipAgainstPlane(n, d, clippedMdlA, clippedMdlB);
}
void Lua::Model::ClearMeshGroups(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.GetMeshGroups() = {};
	mdl.GetBaseMeshes() = {};
}
void Lua::Model::RemoveMeshGroup(lua_State *l, ::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	auto &groups = mdl.GetMeshGroups();
	if(idx >= groups.size())
		return;
	groups.erase(groups.begin() + idx);
	auto &baseMeshes = mdl.GetBaseMeshes();
	for(auto it = baseMeshes.begin(); it != baseMeshes.end();) {
		auto &id = *it;
		if(id == idx)
			it = baseMeshes.erase(it);
		else {
			if(id > idx)
				--id;
			++it;
		}
	}
}
void Lua::Model::ClearBaseMeshGroupIds(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.GetBaseMeshes() = {};
}
void Lua::Model::AddTextureGroup(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.CreateTextureGroup();
	Lua::PushInt(l, mdl.GetTextureGroups().size() - 1);
}
void Lua::Model::Merge(lua_State *l, ::Model &mdl, ::Model &mdlOther)
{
	//Lua::CheckModel(l,1);
	mdl.Merge(mdlOther);
}
void Lua::Model::Merge(lua_State *l, ::Model &mdl, ::Model &mdlOther, uint32_t mergeFlags)
{
	//Lua::CheckModel(l,1);
	mdl.Merge(mdlOther, static_cast<::Model::MergeFlags>(mergeFlags));
}
void Lua::Model::GetLODCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetLODCount());
}
static void push_lod(lua_State *l, const LODInfo &info)
{
	auto tLod = Lua::CreateTable(l);

	Lua::PushString(l, "lod");
	Lua::PushInt(l, info.lod);
	Lua::SetTableValue(l, tLod);

	Lua::PushString(l, "meshGroupReplacements");
	auto tGroupReplacements = Lua::CreateTable(l);
	for(auto &pair : info.meshReplacements) {
		Lua::PushInt(l, pair.first);
		int32_t meshId = (pair.second == MODEL_NO_MESH) ? -1 : pair.second;
		Lua::PushInt(l, meshId);
		Lua::SetTableValue(l, tGroupReplacements);
	}
	Lua::SetTableValue(l, tLod);
}
void Lua::Model::GetLODData(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &lods = mdl.GetLODs();
	auto tLods = Lua::CreateTable(l);
	for(auto i = decltype(lods.size()) {0}; i < lods.size(); ++i) {
		auto &lodInfo = lods.at(i);
		Lua::PushInt(l, i + 1);
		push_lod(l, lodInfo);
		Lua::SetTableValue(l, tLods);
	}
}
void Lua::Model::GetLOD(lua_State *l, ::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	auto lod = mdl.GetLOD(idx);
	Lua::PushInt(l, lod);
}
void Lua::Model::GetLODData(lua_State *l, ::Model &mdl, uint32_t lod)
{
	//Lua::CheckModel(l,1);
	auto *lodInfo = mdl.GetLODInfo(lod);
	if(lodInfo == nullptr)
		return;
	push_lod(l, *lodInfo);
}
void Lua::Model::TranslateLODMeshes(lua_State *l, ::Model &mdl, uint32_t lod)
{
	//Lua::CheckModel(l,1);
	std::vector<uint32_t> meshIds;
	auto numMeshGroups = mdl.GetMeshGroupCount();
	meshIds.reserve(numMeshGroups);
	for(auto i = decltype(numMeshGroups) {0}; i < numMeshGroups; ++i)
		meshIds.push_back(i);
	auto r = mdl.TranslateLODMeshes(lod, meshIds);
	Lua::PushBool(l, r);
	if(r == false)
		return;
	auto t = Lua::CreateTable(l);
	for(auto i = decltype(meshIds.size()) {0}; i < meshIds.size(); ++i) {
		auto id = meshIds.at(i);
		Lua::PushInt(l, i + 1);
		Lua::PushInt(l, (id == MODEL_NO_MESH) ? -1 : static_cast<int32_t>(id));
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::TranslateLODMeshes(lua_State *l, ::Model &mdl, uint32_t lod, luabind::object o)
{
	//Lua::CheckModel(l,1);
	int32_t t = 3;
	Lua::CheckTable(l, t);

	std::vector<uint32_t> meshIds;
	auto numMeshGroups = Lua::GetObjectLength(l, t);
	meshIds.reserve(numMeshGroups);
	for(auto i = decltype(numMeshGroups) {0}; i < numMeshGroups; ++i) {
		Lua::PushInt(l, i + 1);
		Lua::GetTableValue(l, t);
		meshIds.push_back(Lua::CheckInt(l, -1));

		Lua::Pop(l, 1);
	}

	auto r = mdl.TranslateLODMeshes(lod, meshIds);
	Lua::PushBool(l, r);
	if(r == false)
		return;
	auto tTranslated = Lua::CreateTable(l);
	for(auto i = decltype(meshIds.size()) {0}; i < meshIds.size(); ++i) {
		auto id = meshIds.at(i);
		Lua::PushInt(l, i + 1);
		Lua::PushInt(l, (id == MODEL_NO_MESH) ? -1 : static_cast<int32_t>(id));
		Lua::SetTableValue(l, tTranslated);
	}
}
void Lua::Model::GetJoints(lua_State *l, ::Model &mdl)
{
	auto &joints = mdl.GetJoints();
	auto t = Lua::CreateTable(l);
	uint32_t idx = 1;
	for(auto &joint : joints) {
		Lua::PushInt(l, idx++);
		Lua::Push<JointInfo *>(l, &joint);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetVertexAnimations(lua_State *l, ::Model &mdl)
{
	auto &vertexAnims = mdl.GetVertexAnimations();
	auto t = Lua::CreateTable(l);
	auto animIdx = 1u;
	for(auto &anim : vertexAnims) {
		Lua::PushInt(l, animIdx++);
		Lua::Push<std::shared_ptr<::VertexAnimation>>(l, anim);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetVertexAnimation(lua_State *l, ::Model &mdl, const std::string &name)
{
	auto *anim = mdl.GetVertexAnimation(name);
	if(anim == nullptr)
		return;
	Lua::Push<std::shared_ptr<::VertexAnimation>>(l, *anim);
}
void Lua::Model::AddVertexAnimation(lua_State *l, ::Model &mdl, const std::string &name)
{
	auto anim = mdl.AddVertexAnimation(name);
	Lua::Push<std::shared_ptr<::VertexAnimation>>(l, anim);
}
void Lua::Model::RemoveVertexAnimation(lua_State *l, ::Model &mdl, const std::string &name) { mdl.RemoveVertexAnimation(name); }
static void get_body_group_meshes(lua_State *l, ::Model &mdl, const std::vector<uint32_t> &bodyGroupIds, uint32_t lod)
{
	std::vector<std::shared_ptr<ModelMesh>> meshes {};
	mdl.GetBodyGroupMeshes(bodyGroupIds, lod, meshes);
	auto t = Lua::CreateTable(l);
	auto idx = 1;
	for(auto &mesh : meshes) {
		Lua::PushInt(l, idx++);
		Lua::Push(l, mesh);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetBodyGroupMeshes(lua_State *l, ::Model &mdl, luabind::object oBodygroups, uint32_t lod)
{
	auto tBodygroups = 2;
	Lua::CheckTable(l, tBodygroups);
	std::vector<uint32_t> bodyGroupIds;

	auto numBgs = Lua::GetObjectLength(l, tBodygroups);
	bodyGroupIds.reserve(numBgs);
	for(auto i = decltype(numBgs) {0u}; i < numBgs; ++i) {
		Lua::PushInt(l, i + 1);
		Lua::GetTableValue(l, tBodygroups);

		auto groupId = Lua::CheckInt(l, -1);
		bodyGroupIds.push_back(groupId);

		Lua::Pop(l, 1);
	}

	get_body_group_meshes(l, mdl, bodyGroupIds, lod);
}
void Lua::Model::GetBodyGroupMeshes(lua_State *l, ::Model &mdl, uint32_t lod)
{
	std::vector<uint32_t> bodygroups {};
	bodygroups.resize(mdl.GetBodyGroupCount(), 0u);

	get_body_group_meshes(l, mdl, bodygroups, lod);
}
void Lua::Model::GetBodyGroupMeshes(lua_State *l, ::Model &mdl) { GetBodyGroupMeshes(l, mdl, 0u); }
static void push_flex_controller(lua_State *l, const FlexController &fc)
{
	auto tFc = Lua::CreateTable(l);

	Lua::PushString(l, "name");
	Lua::PushString(l, fc.name);
	Lua::SetTableValue(l, tFc);

	Lua::PushString(l, "min");
	Lua::PushNumber(l, fc.min);
	Lua::SetTableValue(l, tFc);

	Lua::PushString(l, "max");
	Lua::PushNumber(l, fc.max);
	Lua::SetTableValue(l, tFc);
}
void Lua::Model::GetFlexController(lua_State *l, ::Model &mdl, const std::string &name)
{
	auto *fc = mdl.GetFlexController(name);
	if(fc == nullptr)
		return;
	push_flex_controller(l, *fc);
}
void Lua::Model::GetFlexController(lua_State *l, ::Model &mdl, uint32_t id)
{
	auto *fc = mdl.GetFlexController(id);
	if(fc == nullptr)
		return;
	push_flex_controller(l, *fc);
}
void Lua::Model::GetFlexControllers(lua_State *l, ::Model &mdl)
{
	auto t = Lua::CreateTable(l);
	auto &flexControllers = mdl.GetFlexControllers();
	auto fcId = 1u;
	for(auto &fc : flexControllers) {
		Lua::PushInt(l, fcId++);
		push_flex_controller(l, fc);

		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetFlexControllerId(lua_State *l, ::Model &mdl, const std::string &name)
{
	auto id = 0u;
	if(mdl.GetFlexControllerId(name, id) == false)
		Lua::PushInt(l, -1);
	else
		Lua::PushInt(l, id);
}
void Lua::Model::GetFlexes(lua_State *l, ::Model &mdl)
{
	auto &flexes = mdl.GetFlexes();
	auto t = Lua::CreateTable(l);
	auto flexIdx = 1u;
	for(auto &flex : flexes) {
		Lua::PushInt(l, flexIdx++);
		Lua::Push<Flex *>(l, &flex);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetFlexId(lua_State *l, ::Model &mdl, const std::string &name)
{
	auto flexId = 0u;
	if(mdl.GetFlexId(name, flexId) == false)
		Lua::PushInt(l, -1);
	else
		Lua::PushInt(l, flexId);
}
void Lua::Model::GetFlexFormula(lua_State *l, ::Model &mdl, uint32_t flexId)
{
	std::string formula;
	if(mdl.GetFlexFormula(flexId, formula) == false)
		return;
	Lua::PushString(l, formula);
}
void Lua::Model::GetFlexFormula(lua_State *l, ::Model &mdl, const std::string &flexName)
{
	std::string formula;
	if(mdl.GetFlexFormula(flexName, formula) == false)
		return;
	Lua::PushString(l, formula);
}
void Lua::Model::GetIKControllers(lua_State *l, ::Model &mdl)
{
	auto &ikControllers = mdl.GetIKControllers();
	auto t = Lua::CreateTable(l);
	auto ikControllerIdx = 1u;
	for(auto &ikController : ikControllers) {
		Lua::PushInt(l, ikControllerIdx++);
		Lua::Push<std::shared_ptr<IKController>>(l, ikController);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetIKController(lua_State *l, ::Model &mdl, uint32_t id)
{
	auto *ikController = mdl.GetIKController(id);
	if(ikController == nullptr)
		return;
	Lua::Push<std::shared_ptr<IKController>>(l, ikController->shared_from_this());
}
void Lua::Model::LookupIKController(lua_State *l, ::Model &mdl, const std::string &name)
{
	auto ikControllerId = std::numeric_limits<uint32_t>::max();
	if(mdl.LookupIKController(name, ikControllerId) == false) {
		Lua::PushInt(l, -1);
		return;
	}
	Lua::PushInt(l, ikControllerId);
}
void Lua::Model::AddIKController(lua_State *l, ::Model &mdl, const std::string &name, uint32_t chainLength, const std::string &type, uint32_t method)
{
	auto *ikController = mdl.AddIKController(name, chainLength, type, static_cast<util::ik::Method>(method));
	if(ikController == nullptr)
		return;
	Lua::Push<std::shared_ptr<IKController>>(l, ikController->shared_from_this());
}
void Lua::Model::AddIKController(lua_State *l, ::Model &mdl, const std::string &name, uint32_t chainLength, const std::string &type) { AddIKController(l, mdl, name, chainLength, type, umath::to_integral(util::ik::Method::Default)); }
void Lua::Model::RemoveIKController(lua_State *l, ::Model &mdl, uint32_t id) { mdl.RemoveIKController(id); }
void Lua::Model::RemoveIKController(lua_State *l, ::Model &mdl, const std::string &name) { mdl.RemoveIKController(name); }
void Lua::Model::AddIncludeModel(lua_State *l, ::Model &mdl, const std::string &modelName) { mdl.GetMetaInfo().includes.push_back(modelName); }
void Lua::Model::GetIncludeModels(lua_State *l, ::Model &mdl)
{
	auto t = Lua::CreateTable(l);
	auto &includes = mdl.GetMetaInfo().includes;
	for(auto i = decltype(includes.size()) {0u}; i < includes.size(); ++i) {
		auto &inc = includes.at(i);
		Lua::PushInt(l, i + 1);
		Lua::PushString(l, inc);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetPhonemeMap(lua_State *l, ::Model &mdl)
{
	auto &phonemeMap = mdl.GetPhonemeMap();
	if(phonemeMap.phonemes.empty())
		return;
	auto t = Lua::CreateTable(l);
	for(auto &pair : phonemeMap.phonemes) {
		Lua::PushString(l, pair.first);
		auto tFlexControllers = Lua::CreateTable(l);
		for(auto &fc : pair.second.flexControllers) {
			Lua::PushString(l, fc.first);
			Lua::PushNumber(l, fc.second);
			Lua::SetTableValue(l, tFlexControllers);
		}
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::AssignDistinctMaterial(lua_State *l, ::Model &mdl, ::ModelMeshGroup &group, ::ModelMesh &mesh, ::ModelSubMesh &subMesh)
{
	auto idx = mdl.AssignDistinctMaterial(group, mesh, subMesh);
	if(idx.has_value() == false)
		return;
	Lua::PushInt(l, *idx);
}
void Lua::Model::SetPhonemeMap(lua_State *l, ::Model &mdl, luabind::object o)
{
	auto t = 2;
	Lua::CheckTable(l, t);
	auto &phonemeMap = mdl.GetPhonemeMap();
	phonemeMap = {};

	Lua::PushNil(l);
	while(Lua::GetNextPair(l, t) != 0) {
		std::string phoneme = Lua::CheckString(l, -2);
		auto tPhoneme = Lua::GetStackTop(l);
		Lua::CheckTable(l, tPhoneme);

		auto it = phonemeMap.phonemes.find(phoneme);
		if(it == phonemeMap.phonemes.end())
			phonemeMap.phonemes.insert(std::make_pair(phoneme, PhonemeInfo {}));

		Lua::PushNil(l);
		while(Lua::GetNextPair(l, tPhoneme) != 0) {
			std::string flexController = Lua::CheckString(l, -2);
			auto value = Lua::CheckNumber(l, -1);

			auto itInfo = it->second.flexControllers.find(flexController);
			if(itInfo == it->second.flexControllers.end())
				itInfo = it->second.flexControllers.insert(std::make_pair(flexController, value)).first;
			itInfo->second = value;

			Lua::Pop(l, 1);
		}

		Lua::Pop(l, 1);
	}
}
static void push_object_attachment(lua_State *l, const ObjectAttachment &att)
{
	auto tAtt = Lua::CreateTable(l);

	Lua::PushString(l, "name");
	Lua::PushString(l, att.name);
	Lua::SetTableValue(l, tAtt);

	Lua::PushString(l, "attachment");
	Lua::PushString(l, att.attachment);
	Lua::SetTableValue(l, tAtt);

	Lua::PushString(l, "type");
	Lua::PushInt(l, umath::to_integral(att.type));
	Lua::SetTableValue(l, tAtt);

	Lua::PushString(l, "keyvalues");
	auto t = Lua::CreateTable(l);
	for(auto &pair : att.keyValues) {
		Lua::PushString(l, pair.first);
		Lua::PushString(l, pair.second);
		Lua::SetTableValue(l, t);
	}
	Lua::SetTableValue(l, tAtt);
}
void Lua::Model::GetObjectAttachments(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &objAttachments = mdl.GetObjectAttachments();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto &objAttachment : objAttachments) {
		Lua::PushInt(l, idx++);
		push_object_attachment(l, objAttachment);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::AddObjectAttachment(lua_State *l, ::Model &mdl, uint32_t type, const std::string &name, const std::string &attachment, luabind::object oKeyValues)
{
	//Lua::CheckModel(l,1);
	std::unordered_map<std::string, std::string> keyValues;
	auto numKeyValues = Lua::GetObjectLength(l, 5);

	Lua::PushNil(l);
	while(Lua::GetNextPair(l, 5) != 0) {
		auto *key = Lua::CheckString(l, -2);
		auto *val = Lua::CheckString(l, -1);
		keyValues.insert(std::make_pair(key, val));
		Lua::Pop(l, 1);
	}

	auto attId = mdl.AddObjectAttachment(static_cast<ObjectAttachment::Type>(type), name, attachment, keyValues);
	Lua::PushInt(l, attId);
}
void Lua::Model::GetObjectAttachmentCount(lua_State *l, ::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::PushInt(l, mdl.GetObjectAttachmentCount());
}
void Lua::Model::GetObjectAttachment(lua_State *l, ::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	auto *att = mdl.GetObjectAttachment(idx);
	if(att == nullptr)
		return;
	push_object_attachment(l, *att);
}
void Lua::Model::LookupObjectAttachment(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto attId = 0u;
	auto rAttId = -1;
	if(mdl.LookupObjectAttachment(name, attId) == true)
		rAttId = attId;
	Lua::PushInt(l, rAttId);
}
void Lua::Model::RemoveObjectAttachment(lua_State *l, ::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveObjectAttachment(name);
}
void Lua::Model::RemoveObjectAttachment(lua_State *l, ::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveObjectAttachment(idx);
}
