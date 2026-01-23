// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.shared;

import :model.animation.enums;
import :scripting.lua.classes.model;
import panima;

void Lua::ModelMeshGroup::register_class(luabind::class_<pragma::asset::ModelMeshGroup> &classDef)
{
	classDef.scope[luabind::def("Create", &Create)];
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def("GetName", &GetName);
	classDef.def("GetMeshes", &GetMeshes);
	classDef.def("AddMesh", &AddMesh);
	classDef.def("ClearMeshes", static_cast<void (*)(lua::State *, pragma::asset::ModelMeshGroup &)>([](lua::State *l, pragma::asset::ModelMeshGroup &meshGroup) { meshGroup.GetMeshes().clear(); }));
	classDef.def("SetMeshes", static_cast<void (*)(lua::State *, pragma::asset::ModelMeshGroup &, luabind::object)>([](lua::State *l, pragma::asset::ModelMeshGroup &meshGroup, luabind::object tMeshes) {
		auto idxMeshes = 2;
		CheckTable(l, idxMeshes);
		auto &meshes = meshGroup.GetMeshes();
		meshes = {};
		auto numMeshes = GetObjectLength(l, idxMeshes);
		meshes.reserve(idxMeshes);
		for(auto i = decltype(numMeshes) {0u}; i < numMeshes; ++i) {
			PushInt(l, i + 1);
			GetTableValue(l, idxMeshes);
			auto &subMesh = Lua::Check<std::shared_ptr<pragma::geometry::ModelMesh>>(l, -1);
			meshes.push_back(subMesh);
			Pop(l, 1);
		}
	}));
	classDef.def("GetMesh", static_cast<void (*)(lua::State *, pragma::asset::ModelMeshGroup &, uint32_t)>([](lua::State *l, pragma::asset::ModelMeshGroup &meshGroup, uint32_t index) {
		auto &meshes = meshGroup.GetMeshes();
		if(index >= meshes.size())
			return;
		Push(l, meshes.at(index));
	}));
	classDef.def("GetMeshCount", static_cast<void (*)(lua::State *, pragma::asset::ModelMeshGroup &, uint32_t)>([](lua::State *l, pragma::asset::ModelMeshGroup &meshGroup, uint32_t index) { PushInt(l, meshGroup.GetMeshCount()); }));
}
void Lua::ModelMeshGroup::Create(lua::State *l, const std::string &name) { Lua::Push<std::shared_ptr<pragma::asset::ModelMeshGroup>>(l, pragma::asset::ModelMeshGroup::Create(name)); }
void Lua::ModelMeshGroup::GetName(lua::State *l, pragma::asset::ModelMeshGroup &meshGroup) { PushString(l, meshGroup.GetName()); }
void Lua::ModelMeshGroup::GetMeshes(lua::State *l, pragma::asset::ModelMeshGroup &meshGroup)
{
	auto &meshes = meshGroup.GetMeshes();
	auto t = CreateTable(l);
	int32_t i = 1;
	for(auto &mesh : meshes) {
		PushInt(l, i);
		Lua::Push<std::shared_ptr<pragma::geometry::ModelMesh>>(l, mesh);
		SetTableValue(l, t);
		++i;
	}
}
void Lua::ModelMeshGroup::AddMesh(lua::State *, pragma::asset::ModelMeshGroup &meshGroup, pragma::geometry::ModelMesh &mesh) { meshGroup.AddMesh(mesh.shared_from_this()); }

//////////////////////////

void Lua::Joint::GetType(lua::State *l, pragma::physics::JointInfo &joint) { PushInt(l, joint.type); }
void Lua::Joint::GetChildBoneId(lua::State *l, pragma::physics::JointInfo &joint) { PushInt(l, joint.child); }
void Lua::Joint::GetParentBoneId(lua::State *l, pragma::physics::JointInfo &joint) { PushInt(l, joint.parent); }
void Lua::Joint::GetCollisionsEnabled(lua::State *l, pragma::physics::JointInfo &joint) { PushBool(l, joint.collide); }
void Lua::Joint::GetKeyValues(lua::State *l, pragma::physics::JointInfo &joint)
{
	auto t = CreateTable(l);
	for(auto &pair : joint.args) {
		PushString(l, pair.first);
		PushString(l, pair.second);
		SetTableValue(l, t);
	}
}
void Lua::Joint::SetType(lua::State *l, pragma::physics::JointInfo &joint, uint32_t type) { joint.type = static_cast<pragma::physics::JointType>(type); }
void Lua::Joint::SetCollisionMeshId(lua::State *l, pragma::physics::JointInfo &joint, uint32_t meshId) { joint.child = meshId; }
void Lua::Joint::SetParentCollisionMeshId(lua::State *l, pragma::physics::JointInfo &joint, uint32_t meshId) { joint.parent = meshId; }
void Lua::Joint::SetCollisionsEnabled(lua::State *l, pragma::physics::JointInfo &joint, bool bEnabled) { joint.collide = bEnabled; }
void Lua::Joint::SetKeyValues(lua::State *l, pragma::physics::JointInfo &joint, luabind::object keyValues)
{
	CheckTable(l, 2);

	PushNil(l);
	joint.args.clear();
	while(GetNextPair(l, 2) != 0) {
		auto *key = CheckString(l, -2);
		auto *val = CheckString(l, -1);
		joint.args[key] = val;
		Pop(l, 1);
	}
}
void Lua::Joint::SetKeyValue(lua::State *l, pragma::physics::JointInfo &joint, const std::string &key, const std::string &val) { joint.args[key] = val; }
void Lua::Joint::RemoveKeyValue(lua::State *l, pragma::physics::JointInfo &joint, const std::string &key)
{
	auto it = joint.args.find(key);
	if(it == joint.args.end())
		return;
	joint.args.erase(it);
}

//////////////////////////

namespace pragma::asset {
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
}

namespace pragma::animation {
	std::ostream &operator<<(std::ostream &out, const MetaRigBone &boneInfo)
	{
		out << "MetaRigBone";
		out << "[BoneId:" << boneInfo.boneId << "]";
		out << "[Min:" << boneInfo.bounds.first.x << ", " << boneInfo.bounds.first.y << ", " << boneInfo.bounds.first.z << "]";
		out << "[Max:" << boneInfo.bounds.second.x << ", " << boneInfo.bounds.second.y << ", " << boneInfo.bounds.second.z << "]";
		auto ang = EulerAngles {boneInfo.normalizedRotationOffset};
		out << "[Ang:" << ang.p << ", " << ang.y << "," << ang.r << "]";
		return out;
	}
	std::ostream &operator<<(std::ostream &out, const MetaRigBlendShape &blendShapeInfo)
	{
		out << "MetaRigBlendShape";
		out << "[FlexControllerId:" << blendShapeInfo.flexControllerId << "]";
		return out;
	}
	std::ostream &operator<<(std::ostream &out, const MetaRig &rig)
	{
		out << "MetaRig";
		return out;
	}
};

template<typename TResult, typename TBoneIdentifier, bool (pragma::asset::Model::*GetValue)(pragma::animation::BoneId, TResult &, pragma::math::CoordinateSpace) const>
std::optional<TResult> get_reference_bone_value(const pragma::asset::Model &mdl, TBoneIdentifier boneIdentifier, pragma::math::CoordinateSpace space)
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

template<typename TResult, typename TBoneIdentifier, bool (pragma::asset::Model::*GetValue)(pragma::animation::BoneId, TResult &, pragma::math::CoordinateSpace) const>
std::optional<TResult> get_reference_bone_value_ls(const pragma::asset::Model &mdl, TBoneIdentifier boneIdentifier)
{
	return get_reference_bone_value<TResult, TBoneIdentifier, GetValue>(mdl, boneIdentifier, pragma::math::CoordinateSpace::Local);
}

template<typename TResult>
void def_bone_methods(luabind::class_<pragma::asset::Model> &classDef)
{
	// Note: luabind::default_parameter_policy would be a better choice here, but doesn't work for the CoordinateSpace parameter for some unknown reason
	classDef.def("GetReferenceBonePose", &get_reference_bone_value_ls<pragma::math::ScaledTransform, TResult, &pragma::asset::Model::GetReferenceBonePose>);
	classDef.def("GetReferenceBonePos", &get_reference_bone_value_ls<Vector3, TResult, &pragma::asset::Model::GetReferenceBonePos>);
	classDef.def("GetReferenceBoneRot", &get_reference_bone_value_ls<Quat, TResult, &pragma::asset::Model::GetReferenceBoneRot>);
	classDef.def("GetReferenceBoneScale", &get_reference_bone_value_ls<Vector3, TResult, &pragma::asset::Model::GetReferenceBoneScale>);

	classDef.def("GetReferenceBonePose", &get_reference_bone_value<pragma::math::ScaledTransform, TResult, &pragma::asset::Model::GetReferenceBonePose>);
	classDef.def("GetReferenceBonePos", &get_reference_bone_value<Vector3, TResult, &pragma::asset::Model::GetReferenceBonePos>);
	classDef.def("GetReferenceBoneRot", &get_reference_bone_value<Quat, TResult, &pragma::asset::Model::GetReferenceBoneRot>);
	classDef.def("GetReferenceBoneScale", &get_reference_bone_value<Vector3, TResult, &pragma::asset::Model::GetReferenceBoneScale>);
}

void Lua::Model::register_class(lua::State *l, luabind::class_<pragma::asset::Model> &classDef, luabind::class_<pragma::geometry::ModelMesh> &classDefModelMesh, luabind::class_<pragma::geometry::ModelSubMesh> &classDefModelSubMesh)
{
	classDef.add_static_constant("INVALID_BONE_INDEX", pragma::animation::INVALID_BONE_INDEX);
	classDef.def(luabind::tostring(luabind::self));
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def("Validate", &pragma::asset::Model::Validate);
	classDef.def("RemoveUnusedMaterialReferences", &pragma::asset::Model::RemoveUnusedMaterialReferences);
	classDef.def("GetCollisionMeshes", &GetCollisionMeshes);
	classDef.def("ClearCollisionMeshes", &ClearCollisionMeshes);
	classDef.def("GetSkeleton", &GetSkeleton);
	classDef.def("TransformBone", &pragma::asset::Model::TransformBone);
	classDef.def("TransformBone", &pragma::asset::Model::TransformBone, luabind::default_parameter_policy<4, pragma::math::CoordinateSpace::World> {});
	classDef.def("GetAttachmentCount", &GetAttachmentCount);
	classDef.def("GetAttachments", &GetAttachments);
	classDef.def("GetAttachment", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &)>(&GetAttachment));
	classDef.def("GetAttachment", static_cast<void (*)(lua::State *, pragma::asset::Model &, int32_t)>(&GetAttachment));
	classDef.def("LookupAttachment", &LookupAttachment);
	classDef.def("LookupBone", &LookupBone);
	classDef.def("LookupAnimation", &LookupAnimation);
	classDef.def("AddAttachment", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &, const std::string &, const Vector3 &, const EulerAngles &)>(&AddAttachment));
	classDef.def("AddAttachment", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &, uint32_t, const Vector3 &, const EulerAngles &)>(&AddAttachment));
	classDef.def("SetAttachmentData", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &, luabind::object)>(&SetAttachmentData));
	classDef.def("SetAttachmentData", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t, luabind::object)>(&SetAttachmentData));
	classDef.def("RemoveAttachment", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &)>(&RemoveAttachment));
	classDef.def("RemoveAttachment", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&RemoveAttachment));

	classDef.def("FindBoneTwistAxis", &pragma::asset::Model::FindBoneTwistAxis);
	classDef.def("FindBoneAxisForDirection", &pragma::asset::Model::FindBoneAxisForDirection);
	classDef.scope[luabind::def("get_twist_axis_rotation_offset", &pragma::asset::Model::GetTwistAxisRotationOffset)];

	classDef.def("GetObjectAttachments", &GetObjectAttachments);
	classDef.def("AddObjectAttachment", &AddObjectAttachment);
	classDef.def("GetObjectAttachmentCount", &GetObjectAttachmentCount);
	classDef.def("GetObjectAttachment", &GetObjectAttachment);
	classDef.def("LookupObjectAttachment", &LookupObjectAttachment);
	classDef.def("RemoveObjectAttachment", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &)>(&RemoveObjectAttachment));
	classDef.def("RemoveObjectAttachment", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&RemoveObjectAttachment));

	classDef.def("GetBlendControllerCount", &GetBlendControllerCount);
	classDef.def("GetBlendControllers", &GetBlendControllers);
	classDef.def("GetBlendController", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &)>(&GetBlendController));
	classDef.def("GetBlendController", static_cast<void (*)(lua::State *, pragma::asset::Model &, int32_t)>(&GetBlendController));
	classDef.def("LookupBlendController", &LookupBlendController);
	classDef.def("GetAnimationCount", &GetAnimationCount);
	classDef.def("GetAnimations", &GetAnimations);
	classDef.def("GetAnimationNames", &GetAnimationNames);
	classDef.def("GetAnimation", static_cast<void (*)(lua::State *, pragma::asset::Model &, const char *)>(&GetAnimation));
	classDef.def("GetAnimation", static_cast<void (*)(lua::State *, pragma::asset::Model &, unsigned int)>(&GetAnimation));
	classDef.def("GetAnimationName", &GetAnimationName);

	def_bone_methods<pragma::animation::BoneId>(classDef);
	def_bone_methods<const std::string &>(classDef);

	classDef.def("CalcBoneLength", &pragma::asset::Model::CalcBoneLength);
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
	classDef.def("GetMeshGroup", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &)>(&GetMeshGroup));
	classDef.def("GetMeshGroup", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&GetMeshGroup));
	classDef.def("GetMeshes", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &)>(&GetMeshes));
	classDef.def("GetMeshes", static_cast<void (*)(lua::State *, pragma::asset::Model &, luabind::object)>(&GetMeshes));
	classDef.def("GetMeshGroups", +[](pragma::asset::Model &mdl) -> std::vector<std::shared_ptr<pragma::asset::ModelMeshGroup>> {
		return mdl.GetMeshGroups();
	});
	classDef.def("AddMeshGroup", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &)>(&AddMeshGroup));
	classDef.def("AddMeshGroup", static_cast<void (*)(lua::State *, pragma::asset::Model &, pragma::asset::ModelMeshGroup &)>(&AddMeshGroup));
	classDef.def("UpdateCollisionBounds", &UpdateCollisionBounds);
	classDef.def("UpdateRenderBounds", &UpdateRenderBounds);
	classDef.def("Update", static_cast<void (*)(lua::State *, pragma::asset::Model &)>(&Update));
	classDef.def("Update", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&Update));
	classDef.def("GetName", &GetName);
	classDef.def("GetMass", &GetMass);
	classDef.def("SetMass", &SetMass);
	classDef.def("GetBoneCount", &GetBoneCount);
	classDef.def("GetCollisionBounds", &GetCollisionBounds);
	classDef.def("GetRenderBounds", &GetRenderBounds);
	classDef.def("SetCollisionBounds", &SetCollisionBounds);
	classDef.def("SetRenderBounds", &SetRenderBounds);
	classDef.def("AddCollisionMesh", &AddCollisionMesh);
	classDef.def("AddMaterial", &AddMaterial);
	classDef.def("SetMaterial", &SetMaterial);
	classDef.def("GetMaterials", &GetMaterials);
	classDef.def("GetMaterial", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t idx) {
		auto *mat = mdl.GetMaterial(idx);
		if(mat == nullptr)
			return;
		Lua::Push<pragma::material::Material *>(l, mat);
	}));
	classDef.def("GetMaterialIndex", static_cast<void (*)(lua::State *, pragma::asset::Model &, pragma::geometry::ModelSubMesh &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, pragma::geometry::ModelSubMesh &mesh, uint32_t skinId) {
		auto idx = mdl.GetMaterialIndex(mesh, skinId);
		if(idx.has_value() == false)
			return;
		PushInt(l, *idx);
	}));
	classDef.def(
	  "GetSubMesh", +[](lua::State *l, pragma::asset::Model &mdl, uint32_t mgId, uint32_t mId, uint32_t smId) -> std::shared_ptr<pragma::geometry::ModelSubMesh> {
		  auto *sm = mdl.GetSubMesh(mgId, mId, smId);
		  if(!sm)
			  return nullptr;
		  return sm->shared_from_this();
	  });
	classDef.def(
	  "FindSubMeshId", +[](lua::State *l, pragma::asset::Model &mdl, pragma::geometry::ModelSubMesh &smTgt) -> std::optional<std::tuple<uint32_t, uint32_t, uint32_t>> {
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
	classDef.def("GetMaterialCount", &GetMaterialCount);
	classDef.def("GetMeshGroupCount", &GetMeshGroupCount);
	classDef.def("GetMeshCount", &GetMeshCount);
	classDef.def("GetSubMeshCount", &GetSubMeshCount);
	classDef.def("GetCollisionMeshCount", &GetCollisionMeshCount);
	classDef.def("GetBodyGroupId", &GetBodyGroupId);
	classDef.def("GetBodyGroupCount", &GetBodyGroupCount);
	classDef.def("AddHitbox", &AddHitbox);
	classDef.def("GetHitboxCount", &GetHitboxCount);
	classDef.def("GetHitboxGroup", &GetHitboxGroup);
	classDef.def("GetHitboxBounds", &GetHitboxBounds);
	classDef.def("GetHitboxBones", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&GetHitboxBones));
	classDef.def("GetHitboxBones", static_cast<void (*)(lua::State *, pragma::asset::Model &)>(&GetHitboxBones));
	classDef.def("SetHitboxGroup", &SetHitboxGroup);
	classDef.def("SetHitboxBounds", &SetHitboxBounds);
	classDef.def("RemoveHitbox", &RemoveHitbox);
	classDef.def("GetBodyGroups", &GetBodyGroups);
	classDef.def("GetBodyGroup", &GetBodyGroup);
	classDef.def("GetExtensionData", &pragma::asset::Model::GetExtensionData);
	classDef.def("GenerateHitboxes", &pragma::asset::Model::GenerateHitboxes);
	classDef.def("GenerateMetaRig", &pragma::asset::Model::GenerateMetaRig);
	classDef.def("GenerateMetaBlendShapes", &pragma::asset::Model::GenerateMetaBlendShapes);
	classDef.def("GetMetaRig", &pragma::asset::Model::GetMetaRig);
	classDef.def("ClearMetaRig", &pragma::asset::Model::ClearMetaRig);
	classDef.def("GetMetaRigReferencePose", &pragma::asset::Model::GetMetaRigReferencePose);
	classDef.def(
	  "GenerateStandardMetaRigReferenceBonePoses", +[](const pragma::asset::Model &mdl) -> std::optional<std::vector<pragma::math::ScaledTransform>> {
		  std::vector<pragma::math::ScaledTransform> poses;
		  if(!mdl.GenerateStandardMetaRigReferenceBonePoses(poses))
			  return {};
		  return poses;
	  });
	classDef.def("CalcNormalizedMetaBoneRotation", &pragma::asset::Model::CalcNormalizedMetaBoneRotation);
	classDef.def("RetargetMetaBoneRotation", &pragma::asset::Model::RetargetMetaBoneRotation);
	classDef.def("GetMetaRigBoneParentId", &pragma::asset::Model::GetMetaRigBoneParentId);
	classDef.def("GetTextureGroupCount", &GetTextureGroupCount);
	classDef.def("GetTextureGroups", &GetTextureGroups);
	classDef.def("GetTextureGroup", &GetTextureGroup);
	classDef.def("SaveLegacy", &Save);
	classDef.def("Save", static_cast<void (*)(lua::State *, pragma::asset::Model &, ::udm::AssetData &)>([](lua::State *l, pragma::asset::Model &mdl, ::udm::AssetData &assetData) {
		auto *nw = pragma::Engine::Get()->GetNetworkState(l);
		auto *game = nw ? nw->GetGameState() : nullptr;
		if(game == nullptr)
			return;
		std::string err;
		auto result = mdl.Save(*game, assetData, err);
		if(result == false)
			PushString(l, err);
		else
			PushBool(l, result);
	}));
	classDef.def("Save", static_cast<void (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) {
		auto *nw = pragma::Engine::Get()->GetNetworkState(l);
		auto *game = nw ? nw->GetGameState() : nullptr;
		if(game == nullptr)
			return;
		std::string err;
		auto result = mdl.Save(*game, err);
		if(result == false)
			PushString(l, err);
		else
			PushBool(l, result);
	}));
	classDef.def(
	  "Save", +[](lua::State *l, pragma::Engine *engine, pragma::asset::Model &mdl, const std::string &fname) {
		  auto *nw = pragma::Engine::Get()->GetNetworkState(l);
		  auto *game = nw ? nw->GetGameState() : nullptr;
		  if(game == nullptr)
			  return;
		  std::string err;
		  auto result = mdl.Save(*game, fname, err);
		  if(result == false)
			  PushString(l, err);
		  else {
			  pragma::Engine::Get()->PollResourceWatchers();
			  PushBool(l, result);
		  }
	  });
	classDef.def("Copy", static_cast<void (*)(lua::State *, pragma::asset::Model &)>(&Copy));
	classDef.def("Copy", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&Copy));
	classDef.def("GetVertexCount", &GetVertexCount);
	classDef.def("GetTriangleCount", &GetTriangleCount);
	classDef.def("GetMaterialNames", &GetTextures);
	classDef.def("GetMaterialPaths", &GetTexturePaths);
	classDef.def("LoadMaterials", static_cast<void (*)(lua::State *, pragma::asset::Model &)>(&LoadMaterials));
	classDef.def("LoadMaterials", static_cast<void (*)(lua::State *, pragma::asset::Model &, bool)>(&LoadMaterials));
	classDef.def("AddMaterialPath", &AddTexturePath);
	classDef.def("RemoveMaterialPath", &RemoveTexturePath);
	classDef.def("SetMaterialPaths", &SetTexturePaths);
	classDef.def("RemoveMaterial", &RemoveTexture);
	classDef.def("ClearMaterials", &ClearTextures);
	classDef.def("Rotate", &Rotate);
	classDef.def("Translate", &Translate);
	classDef.def("Scale", &Scale);
	classDef.def("GetEyeOffset", &GetEyeOffset);
	classDef.def("SetEyeOffset", &SetEyeOffset);
	classDef.def("AddAnimation", &AddAnimation);
	classDef.def("RemoveAnimation", &RemoveAnimation);
	classDef.def("ClearAnimations", &ClearAnimations);
	//classDef.def("ClipAgainstPlane",&Lua::Model::ClipAgainstPlane);
	classDef.def("ClearMeshGroups", &ClearMeshGroups);
	classDef.def("RemoveMeshGroup", &RemoveMeshGroup);
	classDef.def("ClearBaseMeshGroupIds", &ClearBaseMeshGroupIds);
	classDef.def("AddTextureGroup", &AddTextureGroup);
	classDef.def("Merge", static_cast<void (*)(lua::State *, pragma::asset::Model &, pragma::asset::Model &, uint32_t)>(&Merge));
	classDef.def("Merge", static_cast<void (*)(lua::State *, pragma::asset::Model &, pragma::asset::Model &)>(&Merge));
	classDef.def("GetLODCount", &GetLODCount);
	classDef.def("GetLODData", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&GetLODData));
	classDef.def("GetLODData", static_cast<void (*)(lua::State *, pragma::asset::Model &)>(&GetLODData));
	classDef.def("GetLOD", &GetLOD);
	classDef.def("GenerateLowLevelLODs", &pragma::asset::Model::GenerateLowLevelLODs);
	classDef.def("GenerateCollisionMeshes", &pragma::asset::Model::GenerateCollisionMeshes);
	classDef.def("GenerateCollisionMeshes", +[](pragma::asset::Model &mdl, bool convex, float mass) -> bool { return mdl.GenerateCollisionMeshes(convex, mass); });
	classDef.def("TranslateLODMeshes", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t, luabind::object)>(&TranslateLODMeshes));
	classDef.def("TranslateLODMeshes", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&TranslateLODMeshes));
	classDef.def("GetJoints", &GetJoints);
	classDef.def("SetReferencePoses", &pragma::asset::Model::SetReferencePoses);
	classDef.def("SetReferencePoses", &pragma::asset::Model::SetReferencePoses, luabind::default_parameter_policy<3, false> {});
	classDef.def("AddJoint", static_cast<pragma::physics::JointInfo *(*)(lua::State *, pragma::asset::Model &, pragma::physics::JointType, pragma::animation::BoneId, pragma::animation::BoneId)>([](lua::State *l, pragma::asset::Model &mdl, pragma::physics::JointType type, pragma::animation::BoneId child, pragma::animation::BoneId parent) -> pragma::physics::JointInfo * {
		auto &joint = mdl.AddJoint(type, child, parent);
		return &joint;
	}));
	classDef.def("ClearJoints", static_cast<void (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) { mdl.GetJoints().clear(); }));
	classDef.def("GetVertexAnimations", &GetVertexAnimations);
	classDef.def("GetVertexAnimation", &GetVertexAnimation);
	classDef.def("AddVertexAnimation", &AddVertexAnimation);
	classDef.def("RemoveVertexAnimation", &RemoveVertexAnimation);
	classDef.def("GetBodyGroupMeshes", static_cast<void (*)(lua::State *, pragma::asset::Model &, luabind::object, uint32_t)>(&GetBodyGroupMeshes));
	classDef.def("GetBodyGroupMeshes", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&GetBodyGroupMeshes));
	classDef.def("GetBodyGroupMeshes", static_cast<void (*)(lua::State *, pragma::asset::Model &)>(&GetBodyGroupMeshes));
	classDef.def("GetBodyGroupMesh", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t bodyGroupId, uint32_t bgValue) {
		uint32_t meshId;
		if(mdl.GetMesh(bodyGroupId, bgValue, meshId) == false)
			return;
		PushInt(l, meshId);
	}));
	classDef.def("GetFlexControllers", &GetFlexControllers);
	classDef.def("LookupFlexController", &GetFlexControllerId);
	classDef.def("GetFlexController", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &)>(&GetFlexController));
	classDef.def("GetFlexController", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&GetFlexController));
	classDef.def(
	  "AddFlexController", +[](pragma::asset::Model &mdl, const std::string &name, float min, float max) -> pragma::animation::FlexControllerId {
		  uint32_t id;
		  if(mdl.GetFlexControllerId(name, id))
			  return id;
		  mdl.GetFlexControllers().push_back({name, min, max});
		  return mdl.GetFlexControllers().size() - 1;
	  });
	classDef.def("GetFlexes", &GetFlexes);
	classDef.def("AddFlex", +[](pragma::asset::Model &mdl, const pragma::animation::Flex &flex) { mdl.GetFlexes().push_back(flex); });
	classDef.def("LookupFlex", &GetFlexId);
	classDef.def("GetFlexFormula", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &)>(&GetFlexFormula));
	classDef.def("GetFlexFormula", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&GetFlexFormula));
	classDef.def("GetFlexControllerCount", static_cast<void (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) { PushInt(l, mdl.GetFlexControllerCount()); }));
	classDef.def("GetFlexCount", static_cast<void (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) { PushInt(l, mdl.GetFlexCount()); }));
	classDef.def("CalcFlexWeight", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t, luabind::object)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t flexId, luabind::object oFc) {
		CheckFunction(l, 3);
		auto weight = mdl.CalcFlexWeight(
		  flexId,
		  [&oFc, l](uint32_t fcId) -> std::optional<float> {
			  auto result = CallFunction(
			    l,
			    [oFc, fcId](lua::State *l) -> StatusCode {
				    oFc.push(l);
				    PushInt(l, fcId);
				    return StatusCode::Ok;
			    },
			    1);
			  if(result != StatusCode::Ok)
				  return std::optional<float> {};
			  if(IsSet(l, -1) == false)
				  return std::optional<float> {};
			  return CheckNumber(l, -1);
		  },
		  [](uint32_t fcId) -> std::optional<float> { return std::optional<float> {}; });
		if(weight.has_value() == false)
			return;
		PushNumber(l, *weight);
	}));
	classDef.def("CalcReferenceAttachmentPose", static_cast<void (*)(lua::State *, pragma::asset::Model &, int32_t)>([](lua::State *l, pragma::asset::Model &mdl, int32_t attIdx) {
		auto t = mdl.CalcReferenceAttachmentPose(attIdx);
		if(t.has_value() == false)
			return;
		Lua::Push<pragma::math::ScaledTransform>(l, *t);
	}));
	classDef.def("CalcReferenceBonePose", static_cast<void (*)(lua::State *, pragma::asset::Model &, int32_t)>([](lua::State *l, pragma::asset::Model &mdl, int32_t boneIdx) {
		auto t = mdl.CalcReferenceBonePose(boneIdx);
		if(t.has_value() == false)
			return;
		Lua::Push<pragma::math::ScaledTransform>(l, *t);
	}));
	classDef.def("IsRootBone", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t boneId) { PushBool(l, mdl.IsRootBone(boneId)); }));
	classDef.def("GetFlags", static_cast<void (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) { PushInt(l, mdl.GetMetaInfo().flags); }));
	classDef.def("SetFlags", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t flags) { mdl.GetMetaInfo().flags = static_cast<pragma::asset::Model::Flags>(flags); }));
	classDef.def("HasFlag", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t flag) { PushBool(l, pragma::math::is_flag_set(mdl.GetMetaInfo().flags, static_cast<pragma::asset::Model::Flags>(flag)) != 0); }));
	classDef.def("IsStatic", static_cast<void (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) { PushBool(l, pragma::math::is_flag_set(mdl.GetMetaInfo().flags, pragma::asset::Model::Flags::Static)); }));
	classDef.def(
	  "GetEyeball", +[](lua::State *l, pragma::asset::Model &mdl, uint32_t eyeIdx) {
		  auto *eyeball = mdl.GetEyeball(eyeIdx);
		  if(!eyeball)
			  return;
		  Lua::Push<pragma::asset::Eyeball *>(l, eyeball);
	  });
	classDef.def("GetEyeballs", static_cast<void (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) {
		auto &eyeballs = mdl.GetEyeballs();
		auto t = CreateTable(l);
		for(auto i = decltype(eyeballs.size()) {0u}; i < eyeballs.size(); ++i) {
			auto &eyeball = eyeballs.at(i);
			PushInt(l, i + 1);
			Lua::Push<pragma::asset::Eyeball *>(l, &eyeball);
			SetTableValue(l, t);
		}
	}));
	classDef.def("GetEyeballCount", static_cast<void (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) { PushInt(l, mdl.GetEyeballCount()); }));
	classDef.def("IsStatic", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t eyeballIndex) {
		auto *eyeball = mdl.GetEyeball(eyeballIndex);
		if(eyeball == nullptr)
			return;
		Lua::Push<pragma::asset::Eyeball *>(l, eyeball);
	}));
	classDef.def("AddEyeball", static_cast<void (*)(lua::State *, pragma::asset::Model &, pragma::asset::Eyeball &)>([](lua::State *l, pragma::asset::Model &mdl, pragma::asset::Eyeball &eyeball) { mdl.AddEyeball(eyeball); }));

	classDef.def("GetIKControllers", &GetIKControllers);
	classDef.def("GetIKController", &GetIKController);
	classDef.def("LookupIKController", &LookupIKController);
	classDef.def("AddIKController", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &, uint32_t, const std::string &, uint32_t)>(&AddIKController));
	classDef.def("AddIKController", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &, uint32_t, const std::string &)>(&AddIKController));
	classDef.def("RemoveIKController", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>(&RemoveIKController));
	classDef.def("RemoveIKController", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &)>(&RemoveIKController));

	classDef.def("AddIncludeModel", &AddIncludeModel);
	classDef.def("GetIncludeModels", &GetIncludeModels);

	classDef.def("GetPhonemeMap", &GetPhonemeMap);
	classDef.def("SetPhonemeMap", &SetPhonemeMap);
	classDef.def(
	  "GetPhonemes", +[](const pragma::asset::Model &mdl) -> std::vector<std::string> {
		  auto &phonemeMap = mdl.GetPhonemeMap();
		  std::vector<std::string> phonemes;
		  phonemes.reserve(phonemeMap.phonemes.size());
		  for(auto &[name, info] : phonemeMap.phonemes)
			  phonemes.push_back(name);
		  return phonemes;
	  });

	classDef.def("AssignDistinctMaterial", &AssignDistinctMaterial);

	classDef.def("SelectWeightedAnimation", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t activity) { PushInt(l, mdl.SelectWeightedAnimation(static_cast<pragma::Activity>(activity))); }));
	classDef.def("SelectWeightedAnimation",
	  static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t, int32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t activity, int32_t animIgnore) { PushInt(l, mdl.SelectWeightedAnimation(static_cast<pragma::Activity>(activity), animIgnore)); }));
	classDef.def("SelectFirstAnimation", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t activity) { PushInt(l, mdl.SelectFirstAnimation(static_cast<pragma::Activity>(activity))); }));
	classDef.def("GetAnimationActivityWeight", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t anim) { PushInt(l, mdl.GetAnimationActivityWeight(anim)); }));
	classDef.def("GetAnimationActivity", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t anim) { PushInt(l, pragma::math::to_integral(mdl.GetAnimationActivity(anim))); }));
	classDef.def("GetAnimationDuration", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t anim) { PushNumber(l, mdl.GetAnimationDuration(anim)); }));
	classDef.def("HasVertexWeights", static_cast<void (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) { PushBool(l, mdl.HasVertexWeights()); }));

	// Flex animations
	classDef.def("GetFlexAnimations", static_cast<luabind::object (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) -> luabind::object { return vector_to_table(l, mdl.GetFlexAnimations()); }));
	classDef.def("GetFlexAnimationNames", static_cast<luabind::object (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) -> luabind::object { return vector_to_table(l, mdl.GetFlexAnimationNames()); }));
	classDef.def("GetFlexAnimationCount", static_cast<uint32_t (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) -> uint32_t { return mdl.GetFlexAnimations().size(); }));
	classDef.def("AddFlexAnimation", static_cast<std::shared_ptr<FlexAnimation> (*)(lua::State *, pragma::asset::Model &, const std::string &)>([](lua::State *l, pragma::asset::Model &mdl, const std::string &name) -> std::shared_ptr<FlexAnimation> {
		auto anim = pragma::util::make_shared<FlexAnimation>();
		mdl.AddFlexAnimation(name, *anim);
		return anim;
	}));
	classDef.def("AddFlexAnimation", static_cast<void (*)(lua::State *, pragma::asset::Model &, const std::string &, FlexAnimation &)>([](lua::State *l, pragma::asset::Model &mdl, const std::string &name, FlexAnimation &flexAnim) { mdl.AddFlexAnimation(name, flexAnim); }));
	classDef.def("LookupFlexAnimation", static_cast<luabind::object (*)(lua::State *, pragma::asset::Model &, const std::string &)>([](lua::State *l, pragma::asset::Model &mdl, const std::string &name) -> luabind::object {
		auto id = mdl.LookupFlexAnimation(name);
		if(id.has_value() == false)
			return {};
		return luabind::object {l, *id};
	}));
	classDef.def("GetFlexAnimation", static_cast<luabind::object (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t idx) -> luabind::object {
		auto *flexAnim = mdl.GetFlexAnimation(idx);
		if(flexAnim == nullptr)
			return {};
		return luabind::object {l, flexAnim->shared_from_this()};
	}));
	classDef.def("GetFlexAnimationName", static_cast<luabind::object (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t idx) -> luabind::object {
		auto *name = mdl.GetFlexAnimationName(idx);
		if(name == nullptr)
			return {};
		return luabind::object {l, *name};
	}));
	classDef.def("ClearFlexAnimations", static_cast<void (*)(lua::State *, pragma::asset::Model &)>([](lua::State *l, pragma::asset::Model &mdl) { mdl.GetFlexAnimations().clear(); }));
	classDef.def("RemoveFlexAnimation", static_cast<void (*)(lua::State *, pragma::asset::Model &, uint32_t)>([](lua::State *l, pragma::asset::Model &mdl, uint32_t idx) {
		auto &flexAnims = mdl.GetFlexAnimations();
		if(idx >= flexAnims.size())
			return;
		flexAnims.erase(flexAnims.begin() + idx);
	}));

	classDef.add_static_constant("FLAG_NONE", pragma::math::to_integral(pragma::asset::Model::Flags::None));
	classDef.add_static_constant("FLAG_BIT_STATIC", pragma::math::to_integral(pragma::asset::Model::Flags::Static));
	classDef.add_static_constant("FLAG_BIT_INANIMATE", pragma::math::to_integral(pragma::asset::Model::Flags::Inanimate));
	classDef.add_static_constant("FLAG_BIT_DONT_PRECACHE_TEXTURE_GROUPS", pragma::math::to_integral(pragma::asset::Model::Flags::DontPrecacheTextureGroups));

	classDef.add_static_constant("FMERGE_NONE", pragma::math::to_integral(pragma::asset::Model::MergeFlags::None));
	classDef.add_static_constant("FMERGE_ANIMATIONS", pragma::math::to_integral(pragma::asset::Model::MergeFlags::Animations));
	classDef.add_static_constant("FMERGE_ATTACHMENTS", pragma::math::to_integral(pragma::asset::Model::MergeFlags::Attachments));
	classDef.add_static_constant("FMERGE_BLEND_CONTROLLERS", pragma::math::to_integral(pragma::asset::Model::MergeFlags::BlendControllers));
	classDef.add_static_constant("FMERGE_HITBOXES", pragma::math::to_integral(pragma::asset::Model::MergeFlags::Hitboxes));
	classDef.add_static_constant("FMERGE_JOINTS", pragma::math::to_integral(pragma::asset::Model::MergeFlags::Joints));
	classDef.add_static_constant("FMERGE_COLLISION_MESHES", pragma::math::to_integral(pragma::asset::Model::MergeFlags::CollisionMeshes));
	classDef.add_static_constant("FMERGE_MESHES", pragma::math::to_integral(pragma::asset::Model::MergeFlags::Meshes));
	classDef.add_static_constant("FMERGE_ALL", pragma::math::to_integral(pragma::asset::Model::MergeFlags::All));

	classDef.add_static_constant("FCOPY_NONE", pragma::math::to_integral(pragma::asset::Model::CopyFlags::None));
	classDef.add_static_constant("FCOPY_SHALLOW", pragma::math::to_integral(pragma::asset::Model::CopyFlags::ShallowCopy));
	classDef.add_static_constant("FCOPY_BIT_MESHES", pragma::math::to_integral(pragma::asset::Model::CopyFlags::CopyMeshesBit));
	classDef.add_static_constant("FCOPY_BIT_ANIMATIONS", pragma::math::to_integral(pragma::asset::Model::CopyFlags::CopyAnimationsBit));
	classDef.add_static_constant("FCOPY_BIT_VERTEX_ANIMATIONS", pragma::math::to_integral(pragma::asset::Model::CopyFlags::CopyVertexAnimationsBit));
	classDef.add_static_constant("FCOPY_BIT_COLLISION_MESHES", pragma::math::to_integral(pragma::asset::Model::CopyFlags::CopyCollisionMeshesBit));
	classDef.add_static_constant("FCOPY_BIT_FLEX_ANIMATIONS", pragma::math::to_integral(pragma::asset::Model::CopyFlags::CopyFlexAnimationsBit));
	classDef.add_static_constant("FCOPY_BIT_COPY_UNIQUE_IDS", pragma::math::to_integral(pragma::asset::Model::CopyFlags::CopyUniqueIdsBit));
	classDef.add_static_constant("FCOPY_BIT_COPY_VERTEX_DATA", pragma::math::to_integral(pragma::asset::Model::CopyFlags::CopyVertexData));
	classDef.add_static_constant("FCOPY_DEEP", pragma::math::to_integral(pragma::asset::Model::CopyFlags::DeepCopy));

	classDef.add_static_constant("FUPDATE_NONE", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::None));
	classDef.add_static_constant("FUPDATE_BOUNDS", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::UpdateBounds));
	classDef.add_static_constant("FUPDATE_PRIMITIVE_COUNTS", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::UpdatePrimitiveCounts));
	classDef.add_static_constant("FUPDATE_INITIALIZE_COLLISION_SHAPES", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::InitializeCollisionShapes));
	classDef.add_static_constant("FUPDATE_CALCULATE_TANGENTS", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::CalculateTangents));
	classDef.add_static_constant("FUPDATE_VERTEX_BUFFER", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::UpdateVertexBuffer));
	classDef.add_static_constant("FUPDATE_INDEX_BUFFER", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::UpdateIndexBuffer));
	classDef.add_static_constant("FUPDATE_WEIGHT_BUFFER", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::UpdateWeightBuffer));
	classDef.add_static_constant("FUPDATE_ALPHA_BUFFER", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::UpdateAlphaBuffer));
	classDef.add_static_constant("FUPDATE_VERTEX_ANIMATION_BUFFER", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::UpdateVertexAnimationBuffer));
	classDef.add_static_constant("FUPDATE_CHILDREN", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::UpdateChildren));
	classDef.add_static_constant("FUPDATE_BUFFERS", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::UpdateBuffers));
	classDef.add_static_constant("FUPDATE_INITIALIZE", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::Initialize));
	classDef.add_static_constant("FUPDATE_ALL", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::All));
	classDef.add_static_constant("FUPDATE_ALL_DATA", pragma::math::to_integral(pragma::asset::ModelUpdateFlags::AllData));

	classDef.add_static_constant("OBJECT_ATTACHMENT_TYPE_MODEL", pragma::math::to_integral(pragma::asset::ObjectAttachment::Type::Model));
	classDef.add_static_constant("OBJECT_ATTACHMENT_TYPE_PARTICLE_SYSTEM", pragma::math::to_integral(pragma::asset::ObjectAttachment::Type::ParticleSystem));

	classDef.scope[luabind::def(
	  "Load", +[](lua::State *l, pragma::Game &game, ::udm::AssetData &assetData) -> var<pragma::geometry::ModelSubMesh, std::pair<bool, std::string>> {
		  auto mdl = game.CreateModel(false);
		  std::string err;
		  auto result = pragma::asset::Model::Load(*mdl, *game.GetNetworkState(), assetData, err);
		  if(result)
			  return luabind::object {l, mdl};
		  return luabind::object {l, std::pair<bool, std::string> {false, err}};
	  })];

	// pragma::asset::Eyeball
	auto classDefEyeball = luabind::class_<pragma::asset::Eyeball>("Eyeball");
	classDefEyeball.def(luabind::constructor<>());
	classDefEyeball.def_readwrite("name", &pragma::asset::Eyeball::name);
	classDefEyeball.def_readwrite("boneIndex", &pragma::asset::Eyeball::boneIndex);
	classDefEyeball.def_readwrite("origin", &pragma::asset::Eyeball::origin);
	classDefEyeball.def_readwrite("zOffset", &pragma::asset::Eyeball::zOffset);
	classDefEyeball.def_readwrite("radius", &pragma::asset::Eyeball::radius);
	classDefEyeball.def_readwrite("up", &pragma::asset::Eyeball::up);
	classDefEyeball.def_readwrite("forward", &pragma::asset::Eyeball::forward);
	classDefEyeball.def_readwrite("irisMaterialIndex", &pragma::asset::Eyeball::irisMaterialIndex);
	classDefEyeball.def_readwrite("maxDilationFactor", &pragma::asset::Eyeball::maxDilationFactor);
	classDefEyeball.def_readwrite("irisUvRadius", &pragma::asset::Eyeball::irisUvRadius);
	classDefEyeball.def_readwrite("irisScale", &pragma::asset::Eyeball::irisScale);
	classDefEyeball.def("GetUpperLidFlexIndices", static_cast<luabind::object (*)(lua::State *, pragma::asset::Eyeball &)>([](lua::State *l, pragma::asset::Eyeball &eyeball) -> luabind::object {
		auto t = luabind::newtable(l);
		t[1] = eyeball.upperLid.raiserFlexIndex;
		t[2] = eyeball.upperLid.neutralFlexIndex;
		t[3] = eyeball.upperLid.lowererFlexIndex;
		return t;
	}));
	classDefEyeball.def("GetUpperLidFlexAngles", static_cast<luabind::object (*)(lua::State *, pragma::asset::Eyeball &)>([](lua::State *l, pragma::asset::Eyeball &eyeball) -> luabind::object {
		auto t = luabind::newtable(l);
		t[1] = eyeball.upperLid.raiserValue;
		t[2] = eyeball.upperLid.neutralValue;
		t[3] = eyeball.upperLid.lowererValue;
		return t;
	}));
	classDefEyeball.def("GetLowerLidFlexIndices", static_cast<luabind::object (*)(lua::State *, pragma::asset::Eyeball &)>([](lua::State *l, pragma::asset::Eyeball &eyeball) -> luabind::object {
		auto t = luabind::newtable(l);
		t[1] = eyeball.lowerLid.raiserFlexIndex;
		t[2] = eyeball.lowerLid.neutralFlexIndex;
		t[3] = eyeball.lowerLid.lowererFlexIndex;
		return t;
	}));
	classDefEyeball.def("GetLowerLidFlexAngles", static_cast<luabind::object (*)(lua::State *, pragma::asset::Eyeball &)>([](lua::State *l, pragma::asset::Eyeball &eyeball) -> luabind::object {
		auto t = luabind::newtable(l);
		t[1] = eyeball.lowerLid.raiserValue;
		t[2] = eyeball.lowerLid.neutralValue;
		t[3] = eyeball.lowerLid.lowererValue;
		return t;
	}));
	classDefEyeball.def("GetUpperLidFlexIndex", static_cast<int32_t (*)(lua::State *, pragma::asset::Eyeball &)>([](lua::State *l, pragma::asset::Eyeball &eyeball) -> int32_t { return eyeball.upperLid.lidFlexIndex; }));
	classDefEyeball.def("GetLowerLidFlexIndex", static_cast<int32_t (*)(lua::State *, pragma::asset::Eyeball &)>([](lua::State *l, pragma::asset::Eyeball &eyeball) -> int32_t { return eyeball.lowerLid.lidFlexIndex; }));
	classDef.scope[classDefEyeball];

	// Flex
	auto classDefFlex = luabind::class_<pragma::animation::Flex>("Flex");
	classDefFlex.def(luabind::constructor<>());
	classDefFlex.def(
	  "__tostring", +[](const pragma::animation::Flex &flex) -> std::string {
		  std::stringstream ss;
		  ss << "Flex";
		  ss << "[" << flex.GetName() << "]";
		  return ss.str();
	  });
	classDefFlex.def("SetName", +[](pragma::animation::Flex &flex, const std::string &name) { flex.SetName(name); });
	classDefFlex.def("GetName", static_cast<void (*)(lua::State *, pragma::animation::Flex &)>([](lua::State *l, pragma::animation::Flex &flex) { PushString(l, flex.GetName()); }));
	classDefFlex.def("AddOperation", +[](pragma::animation::Flex &flex, const pragma::animation::Flex::Operation &op) { flex.GetOperations().push_back(op); });
	classDefFlex.def("GetOperations", static_cast<void (*)(lua::State *, pragma::animation::Flex &)>([](lua::State *l, pragma::animation::Flex &flex) {
		auto t = CreateTable(l);
		auto &ops = flex.GetOperations();
		for(auto i = decltype(ops.size()) {0u}; i < ops.size(); ++i) {
			PushInt(l, i + 1u);
			Lua::Push<pragma::animation::Flex::Operation *>(l, &ops.at(i));
			SetTableValue(l, t);
		}
	}));
	classDefFlex.def("GetFrameIndex", static_cast<void (*)(lua::State *, pragma::animation::Flex &)>([](lua::State *l, pragma::animation::Flex &flex) { PushInt(l, flex.GetFrameIndex()); }));
	classDefFlex.def("GetVertexAnimation", static_cast<void (*)(lua::State *, pragma::animation::Flex &)>([](lua::State *l, pragma::animation::Flex &flex) {
		auto anim = flex.GetVertexAnimation() ? flex.GetVertexAnimation()->shared_from_this() : nullptr;
		if(anim == nullptr)
			return;
		Lua::Push<std::shared_ptr<pragma::animation::VertexAnimation>>(l, anim);
	}));
	classDefFlex.def("SetVertexAnimation", static_cast<void (*)(lua::State *, pragma::animation::Flex &, pragma::animation::VertexAnimation &, uint32_t)>([](lua::State *l, pragma::animation::Flex &flex, pragma::animation::VertexAnimation &anim, uint32_t frameIndex) { flex.SetVertexAnimation(anim, frameIndex); }));
	classDefFlex.def("SetVertexAnimation", static_cast<void (*)(lua::State *, pragma::animation::Flex &, pragma::animation::VertexAnimation &)>([](lua::State *l, pragma::animation::Flex &flex, pragma::animation::VertexAnimation &anim) { flex.SetVertexAnimation(anim); }));
	classDefFlex.add_static_constant("OP_NONE", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::None));
	classDefFlex.add_static_constant("OP_CONST", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Const));
	classDefFlex.add_static_constant("OP_FETCH", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Fetch));
	classDefFlex.add_static_constant("OP_FETCH2", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Fetch2));
	classDefFlex.add_static_constant("OP_ADD", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Add));
	classDefFlex.add_static_constant("OP_SUB", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Sub));
	classDefFlex.add_static_constant("OP_MUL", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Mul));
	classDefFlex.add_static_constant("OP_DIV", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Div));
	classDefFlex.add_static_constant("OP_NEG", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Neg));
	classDefFlex.add_static_constant("OP_EXP", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Exp));
	classDefFlex.add_static_constant("OP_OPEN", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Open));
	classDefFlex.add_static_constant("OP_CLOSE", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Close));
	classDefFlex.add_static_constant("OP_COMMA", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Comma));
	classDefFlex.add_static_constant("OP_MAX", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Max));
	classDefFlex.add_static_constant("OP_MIN", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Min));
	classDefFlex.add_static_constant("OP_TWO_WAY0", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::TwoWay0));
	classDefFlex.add_static_constant("OP_TWO_WAY1", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::TwoWay1));
	classDefFlex.add_static_constant("OP_N_WAY", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::NWay));
	classDefFlex.add_static_constant("OP_COMBO", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Combo));
	classDefFlex.add_static_constant("OP_DOMINATE", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::Dominate));
	classDefFlex.add_static_constant("OP_DME_LOWER_EYELID", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::DMELowerEyelid));
	classDefFlex.add_static_constant("OP_DME_UPPER_EYELID", pragma::math::to_integral(pragma::animation::Flex::Operation::Type::DMEUpperEyelid));

	// Operation
	auto classDefFlexOp = luabind::class_<pragma::animation::Flex::Operation>("Operation");
	classDefFlexOp.def(luabind::constructor<>());
	classDefFlexOp.def_readwrite("type", reinterpret_cast<uint32_t pragma::animation::Flex::Operation::*>(&pragma::animation::Flex::Operation::type));
	classDefFlexOp.def_readwrite("index", reinterpret_cast<int32_t pragma::animation::Flex::Operation::*>(&pragma::animation::Flex::Operation::d));
	classDefFlexOp.def_readwrite("value", reinterpret_cast<float pragma::animation::Flex::Operation::*>(&pragma::animation::Flex::Operation::d));
	classDefFlexOp.def("SetName", +[](pragma::animation::Flex &flex, const std::string name) { flex.SetName(name); });
	classDefFlexOp.def("GetName", static_cast<void (*)(lua::State *, pragma::animation::Flex &)>([](lua::State *l, pragma::animation::Flex &flex) { PushString(l, flex.GetName()); }));
	classDefFlex.scope[classDefFlexOp];

	classDef.scope[classDefFlex];

	// Frame
	auto classDefFrame = luabind::class_<::Frame>("Frame")
	                       .def("GetBoneMatrix", &Frame::GetBoneMatrix)
	                       .def("GetBoneTransform", &Frame::GetBonePosition)
	                       .def("GetBoneRotation", &Frame::GetBoneOrientation)
	                       .def("SetBonePosition", &Frame::SetBonePosition)
	                       .def("SetBoneRotation", &Frame::SetBoneOrientation)
	                       .def("Localize", static_cast<void (*)(lua::State *, ::Frame &, pragma::animation::Animation &, pragma::animation::Skeleton *)>(&Frame::Localize))
	                       .def("Localize", static_cast<void (*)(lua::State *, ::Frame &, pragma::animation::Skeleton *)>(&Frame::Localize))
	                       .def("Globalize", static_cast<void (*)(lua::State *, ::Frame &, pragma::animation::Animation &, pragma::animation::Skeleton *)>(&Frame::Globalize))
	                       .def("Globalize", static_cast<void (*)(lua::State *, ::Frame &, pragma::animation::Skeleton *)>(&Frame::Globalize))
	                       .def("CalcRenderBounds", &Frame::CalcRenderBounds)
	                       .def("Rotate", &Frame::Rotate)
	                       .def("Translate", &Frame::Translate)
	                       .def("Scale", &Frame::Scale)
	                       .def("GetMoveTranslation", &Frame::GetMoveTranslation)
	                       .def("GetMoveTranslationX", &Frame::GetMoveTranslationX)
	                       .def("GetMoveTranslationZ", &Frame::GetMoveTranslationZ)
	                       .def("SetMoveTranslation", &Frame::SetMoveTranslation)
	                       .def("SetMoveTranslationX", &Frame::SetMoveTranslationX)
	                       .def("SetMoveTranslationZ", &Frame::SetMoveTranslationZ)
	                       .def("SetBoneScale", &Frame::SetBoneScale)
	                       .def("GetBoneScale", &Frame::GetBoneScale)
	                       .def("SetBoneTransform", static_cast<void (*)(lua::State *, ::Frame &, unsigned int, const Vector3 &, const Quat &, const Vector3 &)>(&Frame::SetBoneTransform))
	                       .def("SetBoneTransform", static_cast<void (*)(lua::State *, ::Frame &, unsigned int, const Vector3 &, const Quat &)>(&Frame::SetBoneTransform))
	                       .def("GetLocalBoneTransform", &Frame::GetLocalBoneTransform)
	                       .def("GetBoneCount", &Frame::GetBoneCount)
	                       .def("SetBoneCount", &Frame::SetBoneCount)
	                       .def("SetBonePose", static_cast<void (*)(lua::State *, ::Frame &, uint32_t, const pragma::math::ScaledTransform &)>(&Frame::SetBonePose))
	                       .def("SetBonePose", static_cast<void (*)(lua::State *, ::Frame &, uint32_t, const pragma::math::Transform &)>(&Frame::SetBonePose))
	                       .def("GetBonePose", &Frame::GetBonePose)
	                       .def("GetFlexControllerWeights", static_cast<void (*)(lua::State *, ::Frame &)>([](lua::State *l, ::Frame &frame) {
		                       auto &flexFrameData = frame.GetFlexFrameData();
		                       auto t = CreateTable(l);
		                       auto n = flexFrameData.flexControllerWeights.size();
		                       for(auto i = decltype(n) {0u}; i < n; ++i) {
			                       PushInt(l, i + 1);
			                       PushNumber(l, flexFrameData.flexControllerWeights.at(i));
			                       SetTableValue(l, t);
		                       }
	                       }))
	                       .def("GetFlexControllerIds", static_cast<void (*)(lua::State *, ::Frame &)>([](lua::State *l, ::Frame &frame) {
		                       auto &flexFrameData = frame.GetFlexFrameData();
		                       auto t = CreateTable(l);
		                       auto n = flexFrameData.flexControllerIds.size();
		                       for(auto i = decltype(n) {0u}; i < n; ++i) {
			                       PushInt(l, i + 1);
			                       PushInt(l, flexFrameData.flexControllerIds.at(i));
			                       SetTableValue(l, t);
		                       }
	                       }))
	                       .def("SetFlexControllerWeights", static_cast<void (*)(lua::State *, ::Frame &, luabind::object)>([](lua::State *l, ::Frame &frame, luabind::object) {
		                       auto &flexFrameData = frame.GetFlexFrameData();
		                       flexFrameData.flexControllerIds.clear();
		                       flexFrameData.flexControllerWeights.clear();

		                       auto t = CreateTable(l);
		                       CheckTable(l, 2);

		                       PushNil(l);
		                       while(GetNextPair(l, 2) != 0) {
			                       auto flexCId = CheckInt(l, -2);
			                       auto weight = CheckNumber(l, -1);
			                       flexFrameData.flexControllerIds.push_back(flexCId);
			                       flexFrameData.flexControllerWeights.push_back(weight);

			                       Pop(l, 1);
		                       }
	                       }))
	                       .def("Copy", static_cast<void (*)(lua::State *, ::Frame &)>([](lua::State *l, ::Frame &frame) {
		                       auto cpy = ::Frame::Create(frame);
		                       Push(l, cpy);
	                       }));
	classDefFrame.scope[luabind::def("Create", &Frame::Create)];

	// Animation
	auto classDefAnimation = luabind::class_<pragma::animation::Animation>("Animation")
	                           .def("ToPanimaAnimation", &pragma::animation::Animation::ToPanimaAnimation)
	                           .def("ToPanimaAnimation", &pragma::animation::Animation::ToPanimaAnimation, luabind::default_parameter_policy<3, static_cast<const ::Frame *>(nullptr)> {})
	                           .def("GetFrame", &Animation::GetFrame)
	                           .def("GetBoneList", &Animation::GetBoneList)
	                           .def("GetActivity", &pragma::animation::Animation::GetActivity)
	                           .def("GetActivityName", static_cast<luabind::object (*)(lua::State *, pragma::animation::Animation &)>([](lua::State *l, pragma::animation::Animation &anim) -> luabind::object {
		                           auto &reg = pragma::animation::Animation::GetActivityEnumRegister();
		                           auto *name = reg.GetEnumName(pragma::math::to_integral(anim.GetActivity()));
		                           if(name == nullptr)
			                           return {};
		                           return luabind::object {l, *name};
	                           }))
	                           .def("GetBoneId", static_cast<int32_t (*)(lua::State *, pragma::animation::Animation &, uint32_t)>([](lua::State *l, pragma::animation::Animation &anim, uint32_t idx) -> int32_t {
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
	                           .def("AddFrame", &Animation::AddFrame)
	                           .def("GetFrames", &Animation::GetFrames)
	                           .def("GetDuration", &pragma::animation::Animation::GetDuration)
	                           .def("GetBoneCount", &pragma::animation::Animation::GetBoneCount)
	                           .def("GetFrameCount", &pragma::animation::Animation::GetFrameCount)
	                           .def("AddEvent", &Animation::AddEvent)
	                           .def("GetEvents", static_cast<void (*)(lua::State *, pragma::animation::Animation &, uint32_t)>(&Animation::GetEvents))
	                           .def("GetEvents", static_cast<void (*)(lua::State *, pragma::animation::Animation &)>(&Animation::GetEvents))
	                           .def("GetEventCount", static_cast<void (*)(lua::State *, pragma::animation::Animation &, uint32_t)>(&Animation::GetEventCount))
	                           .def("GetEventCount", static_cast<void (*)(lua::State *, pragma::animation::Animation &)>(&Animation::GetEventCount))
	                           .def("GetFadeInTime", &pragma::animation::Animation::GetFadeInTime)
	                           .def("GetFadeOutTime", &pragma::animation::Animation::GetFadeOutTime)
	                           .def("GetBlendController", &Animation::GetBlendController)
	                           .def("CalcRenderBounds", &Animation::CalcRenderBounds)
	                           .def("GetRenderBounds", &Animation::GetRenderBounds)
	                           .def("Rotate", &pragma::animation::Animation::Rotate)
	                           .def("Translate", &pragma::animation::Animation::Translate)
	                           .def("Scale", &pragma::animation::Animation::Scale)
	                           .def("Reverse", &pragma::animation::Animation::Reverse)
	                           .def("RemoveEvent", &Animation::RemoveEvent)
	                           .def("SetEventData", &Animation::SetEventData)
	                           .def("SetEventType", &Animation::SetEventType)
	                           .def("SetEventArgs", &Animation::SetEventArgs)
	                           .def("LookupBone", &Animation::LookupBone)
	                           .def("SetBoneList", &Animation::SetBoneList)
	                           .def("AddBoneId", &pragma::animation::Animation::AddBoneId)
	                           .def("SetFadeInTime", &pragma::animation::Animation::SetFadeInTime)
	                           .def("SetFadeOutTime", &pragma::animation::Animation::SetFadeOutTime)
	                           .def("SetBoneWeight", &pragma::animation::Animation::SetBoneWeight)
	                           .def("GetBoneWeight", static_cast<float (pragma::animation::Animation::*)(uint32_t) const>(&pragma::animation::Animation::GetBoneWeight))
	                           .def("GetBoneWeights", &Animation::GetBoneWeights)
	                           .def("ClearFrames", static_cast<void (*)(lua::State *, pragma::animation::Animation &)>([](lua::State *l, pragma::animation::Animation &anim) { anim.GetFrames().clear(); }))
	                           /*.def("GetBoneId",static_cast<void(*)(lua::State*,pragma::animation::Animation&,uint32_t)>([](lua::State *l,pragma::animation::Animation &anim,uint32_t idx) {
			auto &boneList = anim.GetBoneList();
			if(idx >= boneList.size())
				return;
			Lua::PushInt(l,boneList[idx]);
		}))*/
	                           .def("Save", static_cast<void (*)(lua::State *, pragma::animation::Animation &, ::udm::AssetData &)>([](lua::State *l, pragma::animation::Animation &anim, ::udm::AssetData &assetData) {
		                           std::string err;
		                           auto result = anim.Save(assetData, err);
		                           if(result == false)
			                           PushString(l, err);
		                           else
			                           PushBool(l, result);
	                           }));

	classDefAnimation.scope[(luabind::def("Create", static_cast<std::shared_ptr<pragma::animation::Animation> (*)()>(&pragma::animation::Animation::Create)),
	  luabind::def("Create", static_cast<std::shared_ptr<pragma::animation::Animation> (*)(const pragma::animation::Animation &, pragma::animation::Animation::ShareMode)>(&pragma::animation::Animation::Create)),
	  luabind::def("Create", static_cast<std::shared_ptr<pragma::animation::Animation> (*)(const pragma::animation::Animation &, pragma::animation::Animation::ShareMode)>(&pragma::animation::Animation::Create),
	    luabind::default_parameter_policy<2, pragma::animation::Animation::ShareMode::None> {}))];
	classDefAnimation.scope[(luabind::def("Load", &Animation::Load), luabind::def("RegisterActivity", &Animation::RegisterActivityEnum), luabind::def("RegisterEvent", &Animation::RegisterEventEnum), luabind::def("GetActivityEnums", &Animation::GetActivityEnums),
	  luabind::def("GetEventEnums", &Animation::GetEventEnums), luabind::def("GetActivityEnumName", &Animation::GetActivityEnumName), luabind::def("GetEventEnumName", &Animation::GetEventEnumName), luabind::def("FindActivityId", &Animation::FindActivityId),
	  luabind::def("FindEventId", &Animation::FindEventId), classDefFrame)];
	classDefAnimation.scope[(luabind::def("Load", static_cast<void (*)(lua::State *, ::udm::AssetData &)>([](lua::State *l, ::udm::AssetData &assetData) {
		std::string err;
		auto anim = pragma::animation::Animation::Load(assetData, err);
		if(anim == nullptr) {
			PushBool(l, false);
			PushString(l, err);
			return;
		}
		Push(l, anim);
	})))];
	//for(auto &pair : ANIMATION_EVENT_NAMES)
	//	classDefAnimation.add_static_constant(pair.second.c_str(),pair.first);

	classDefAnimation.add_static_constant("FLAG_LOOP", pragma::math::to_integral(pragma::FAnim::Loop));
	classDefAnimation.add_static_constant("FLAG_NOREPEAT", pragma::math::to_integral(pragma::FAnim::NoRepeat));
	classDefAnimation.add_static_constant("FLAG_MOVEX", pragma::math::to_integral(pragma::FAnim::MoveX));
	classDefAnimation.add_static_constant("FLAG_MOVEZ", pragma::math::to_integral(pragma::FAnim::MoveZ));
	classDefAnimation.add_static_constant("FLAG_AUTOPLAY", pragma::math::to_integral(pragma::FAnim::Autoplay));
	classDefAnimation.add_static_constant("FLAG_GESTURE", pragma::math::to_integral(pragma::FAnim::Gesture));

	//for(auto &pair : ACTIVITY_NAMES)
	//	classDefAnimation.add_static_constant(pair.second.c_str(),pair.first);

	auto defBoneInfo = luabind::class_<pragma::animation::MetaRigBone>("MetaRigBone");
	defBoneInfo.def(luabind::tostring(luabind::self));
	defBoneInfo.def_readwrite("boneId", &pragma::animation::MetaRigBone::boneId);
	defBoneInfo.def_readwrite("radius", &pragma::animation::MetaRigBone::radius);
	defBoneInfo.def_readwrite("length", &pragma::animation::MetaRigBone::length);
	defBoneInfo.def_readwrite("normalizedRotationOffset", &pragma::animation::MetaRigBone::normalizedRotationOffset);
	defBoneInfo.property("min", +[](const pragma::animation::MetaRigBone &boneInfo) -> Vector3 { return boneInfo.bounds.first; }, +[](pragma::animation::MetaRigBone &boneInfo, const Vector3 &min) { boneInfo.bounds.first = min; });
	defBoneInfo.property("max", +[](const pragma::animation::MetaRigBone &boneInfo) -> Vector3 { return boneInfo.bounds.second; }, +[](pragma::animation::MetaRigBone &boneInfo, const Vector3 &max) { boneInfo.bounds.second = max; });
	classDef.scope[defBoneInfo];

	auto defBlendShapeInfo = luabind::class_<pragma::animation::MetaRigBlendShape>("MetaRigBlendShape");
	defBlendShapeInfo.def(luabind::tostring(luabind::self));
	defBlendShapeInfo.def_readwrite("flexControllerId", &pragma::animation::MetaRigBlendShape::flexControllerId);
	classDef.scope[defBlendShapeInfo];

	auto defRig = luabind::class_<pragma::animation::MetaRig>("MetaRig");
	defRig.scope[(luabind::def(
	  "load", +[](lua::State *l, const std::string &fileName, const pragma::animation::Skeleton &skeleton) -> variadic<std::shared_ptr<pragma::animation::MetaRig>, std::pair<bool, std::string>> {
		  auto udmData = ::udm::Data::Load(fileName);
		  if(!udmData)
			  return luabind::object {l, std::pair<bool, std::string> {false, "Failed to load file '" + fileName + "'!"}};
		  auto metaRig = pragma::util::make_shared<pragma::animation::MetaRig>();
		  std::string err;
		  auto res = metaRig->Load(skeleton, udmData->GetAssetData(), err);
		  if(!res)
			  return luabind::object {l, std::pair<bool, std::string> {false, err}};
		  return luabind::object {l, metaRig};
	  }))];
	defRig.def_readwrite("rigType", &pragma::animation::MetaRig::rigType);
	defRig.def_readwrite("forwardFacingRotationOffset", &pragma::animation::MetaRig::forwardFacingRotationOffset);
	defRig.def_readwrite("forwardAxis", &pragma::animation::MetaRig::forwardAxis);
	defRig.def_readwrite("upAxis", &pragma::animation::MetaRig::upAxis);
	defRig.def_readwrite("min", &pragma::animation::MetaRig::min);
	defRig.def_readwrite("max", &pragma::animation::MetaRig::max);
	defRig.def(luabind::tostring(luabind::self));
	defRig.add_static_constant("ROOT_BONE", pragma::math::to_integral(pragma::animation::META_RIG_ROOT_BONE_TYPE));
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
		  auto idx = pragma::math::to_integral(boneType);
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
	  "Save", +[](lua::State *l, const pragma::animation::MetaRig &metaRig, const pragma::animation::Skeleton &skeleton, const std::string &fileName) -> std::pair<bool, std::optional<std::string>> {
		  std::string err;
		  auto udmData = ::udm::Data::Create();
		  auto res = metaRig.Save(skeleton, udmData->GetAssetData(), err);
		  if(!res)
			  return {false, err};
		  std::string writeFileName = fileName;
		  if(file::validate_write_operation(l, writeFileName) == false)
			  return {false, "Failed to save as file '" + fileName + "'!"};

		  try {
			  if(!udmData->SaveAscii(writeFileName))
				  return {false, "Failed to save as file '" + writeFileName + "'!"};
		  }
		  catch(const ::udm::Exception &err) {
			  return {false, err.what()};
		  }
		  return {true, {}};
	  });
	defRig.def("GetBone", &pragma::animation::MetaRig::GetBone);
	defRig.def("FindMetaBoneType", &pragma::animation::MetaRig::FindMetaBoneType);
	defRig.def("GetBlendShape", &pragma::animation::MetaRig::GetBlendShape);
	defRig.def("GetReferenceScale", &pragma::animation::MetaRig::GetReferenceScale);
	defRig.def("DebugPrint", &pragma::animation::MetaRig::DebugPrint);
	defRig.add_static_constant("RIG_TYPE_BIPED", pragma::math::to_integral(pragma::animation::RigType::Biped));
	defRig.add_static_constant("RIG_TYPE_QUADRUPED", pragma::math::to_integral(pragma::animation::RigType::Quadruped));

	defRig.add_static_constant("BONE_SIDE_LEFT", pragma::math::to_integral(pragma::animation::BoneSide::Left));
	defRig.add_static_constant("BONE_SIDE_RIGHT", pragma::math::to_integral(pragma::animation::BoneSide::Right));

	defRig.add_static_constant("BONE_TYPE_HIPS", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Hips));
	defRig.add_static_constant("BONE_TYPE_PELVIS", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Pelvis));
	defRig.add_static_constant("BONE_TYPE_SPINE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Spine));
	defRig.add_static_constant("BONE_TYPE_SPINE1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Spine1));
	defRig.add_static_constant("BONE_TYPE_SPINE2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Spine2));
	defRig.add_static_constant("BONE_TYPE_SPINE3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Spine3));
	defRig.add_static_constant("BONE_TYPE_NECK", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Neck));
	defRig.add_static_constant("BONE_TYPE_HEAD", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Head));
	defRig.add_static_constant("BONE_TYPE_JAW", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Jaw));
	defRig.add_static_constant("BONE_TYPE_LEFT_EAR", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftEar));
	defRig.add_static_constant("BONE_TYPE_RIGHT_EAR", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightEar));
	defRig.add_static_constant("BONE_TYPE_LEFT_EYE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftEye));
	defRig.add_static_constant("BONE_TYPE_RIGHT_EYE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightEye));
	defRig.add_static_constant("BONE_TYPE_CENTER_EYE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::CenterEye));
	defRig.add_static_constant("BONE_TYPE_LEFT_UPPER_ARM", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftUpperArm));
	defRig.add_static_constant("BONE_TYPE_LEFT_LOWER_ARM", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftLowerArm));
	defRig.add_static_constant("BONE_TYPE_LEFT_HAND", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftHand));
	defRig.add_static_constant("BONE_TYPE_RIGHT_UPPER_ARM", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightUpperArm));
	defRig.add_static_constant("BONE_TYPE_RIGHT_LOWER_ARM", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightLowerArm));
	defRig.add_static_constant("BONE_TYPE_RIGHT_HAND", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightHand));
	defRig.add_static_constant("BONE_TYPE_LEFT_UPPER_LEG", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftUpperLeg));
	defRig.add_static_constant("BONE_TYPE_LEFT_LOWER_LEG", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftLowerLeg));
	defRig.add_static_constant("BONE_TYPE_LEFT_FOOT", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftFoot));
	defRig.add_static_constant("BONE_TYPE_LEFT_TOE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftToe));
	defRig.add_static_constant("BONE_TYPE_RIGHT_UPPER_LEG", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightUpperLeg));
	defRig.add_static_constant("BONE_TYPE_RIGHT_LOWER_LEG", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightLowerLeg));
	defRig.add_static_constant("BONE_TYPE_RIGHT_FOOT", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightFoot));
	defRig.add_static_constant("BONE_TYPE_RIGHT_TOE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightToe));
	defRig.add_static_constant("BONE_TYPE_LEFT_THUMB1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftThumb1));
	defRig.add_static_constant("BONE_TYPE_LEFT_THUMB2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftThumb2));
	defRig.add_static_constant("BONE_TYPE_LEFT_THUMB3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftThumb3));
	defRig.add_static_constant("BONE_TYPE_LEFT_INDEX_FINGER1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftIndexFinger1));
	defRig.add_static_constant("BONE_TYPE_LEFT_INDEX_FINGER2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftIndexFinger2));
	defRig.add_static_constant("BONE_TYPE_LEFT_INDEX_FINGER3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftIndexFinger3));
	defRig.add_static_constant("BONE_TYPE_LEFT_MIDDLE_FINGER1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftMiddleFinger1));
	defRig.add_static_constant("BONE_TYPE_LEFT_MIDDLE_FINGER2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftMiddleFinger2));
	defRig.add_static_constant("BONE_TYPE_LEFT_MIDDLE_FINGER3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftMiddleFinger3));
	defRig.add_static_constant("BONE_TYPE_LEFT_RING_FINGER1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftRingFinger1));
	defRig.add_static_constant("BONE_TYPE_LEFT_RING_FINGER2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftRingFinger2));
	defRig.add_static_constant("BONE_TYPE_LEFT_RING_FINGER3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftRingFinger3));
	defRig.add_static_constant("BONE_TYPE_LEFT_LITTLE_FINGER1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftLittleFinger1));
	defRig.add_static_constant("BONE_TYPE_LEFT_LITTLE_FINGER2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftLittleFinger2));
	defRig.add_static_constant("BONE_TYPE_LEFT_LITTLE_FINGER3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftLittleFinger3));
	defRig.add_static_constant("BONE_TYPE_RIGHT_THUMB1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightThumb1));
	defRig.add_static_constant("BONE_TYPE_RIGHT_THUMB2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightThumb2));
	defRig.add_static_constant("BONE_TYPE_RIGHT_THUMB3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightThumb3));
	defRig.add_static_constant("BONE_TYPE_RIGHT_INDEX_FINGER1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightIndexFinger1));
	defRig.add_static_constant("BONE_TYPE_RIGHT_INDEX_FINGER2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightIndexFinger2));
	defRig.add_static_constant("BONE_TYPE_RIGHT_INDEX_FINGER3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightIndexFinger3));
	defRig.add_static_constant("BONE_TYPE_RIGHT_MIDDLE_FINGER1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightMiddleFinger1));
	defRig.add_static_constant("BONE_TYPE_RIGHT_MIDDLE_FINGER2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightMiddleFinger2));
	defRig.add_static_constant("BONE_TYPE_RIGHT_MIDDLE_FINGER3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightMiddleFinger3));
	defRig.add_static_constant("BONE_TYPE_RIGHT_RING_FINGER1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightRingFinger1));
	defRig.add_static_constant("BONE_TYPE_RIGHT_RING_FINGER2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightRingFinger2));
	defRig.add_static_constant("BONE_TYPE_RIGHT_RING_FINGER3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightRingFinger3));
	defRig.add_static_constant("BONE_TYPE_RIGHT_LITTLE_FINGER1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightLittleFinger1));
	defRig.add_static_constant("BONE_TYPE_RIGHT_LITTLE_FINGER2", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightLittleFinger2));
	defRig.add_static_constant("BONE_TYPE_RIGHT_LITTLE_FINGER3", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightLittleFinger3));
	defRig.add_static_constant("BONE_TYPE_TAIL_BASE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::TailBase));
	defRig.add_static_constant("BONE_TYPE_TAIL_MIDDLE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::TailMiddle));
	defRig.add_static_constant("BONE_TYPE_TAIL_MIDDLE1", pragma::math::to_integral(pragma::animation::MetaRigBoneType::TailMiddle1));
	defRig.add_static_constant("BONE_TYPE_TAIL_TIP", pragma::math::to_integral(pragma::animation::MetaRigBoneType::TailTip));
	defRig.add_static_constant("BONE_TYPE_LEFT_WING", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftWing));
	defRig.add_static_constant("BONE_TYPE_LEFT_WING_MIDDLE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftWingMiddle));
	defRig.add_static_constant("BONE_TYPE_LEFT_WING_TIP", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftWingTip));
	defRig.add_static_constant("BONE_TYPE_RIGHT_WING", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightWing));
	defRig.add_static_constant("BONE_TYPE_RIGHT_WING_MIDDLE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightWingMiddle));
	defRig.add_static_constant("BONE_TYPE_RIGHT_WING_TIP", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightWingTip));
	defRig.add_static_constant("BONE_TYPE_LEFT_BREAST_BASE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftBreastBase));
	defRig.add_static_constant("BONE_TYPE_LEFT_BREAST_MIDDLE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftBreastMiddle));
	defRig.add_static_constant("BONE_TYPE_LEFT_BREAST_TIP", pragma::math::to_integral(pragma::animation::MetaRigBoneType::LeftBreastTip));
	defRig.add_static_constant("BONE_TYPE_RIGHT_BREAST_BASE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightBreastBase));
	defRig.add_static_constant("BONE_TYPE_RIGHT_BREAST_MIDDLE", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightBreastMiddle));
	defRig.add_static_constant("BONE_TYPE_RIGHT_BREAST_TIP", pragma::math::to_integral(pragma::animation::MetaRigBoneType::RightBreastTip));
	defRig.add_static_constant("BONE_TYPE_COUNT", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Count));
	defRig.add_static_constant("BONE_TYPE_STANDARD_START", pragma::math::to_integral(pragma::animation::MetaRigBoneType::StandardStart));
	defRig.add_static_constant("BONE_TYPE_STANDARD_END", pragma::math::to_integral(pragma::animation::MetaRigBoneType::StandardEnd));
	defRig.add_static_constant("BONE_TYPE_INVALID", pragma::math::to_integral(pragma::animation::MetaRigBoneType::Invalid));
	static_assert(pragma::math::to_integral(pragma::animation::MetaRigBoneType::Count) == 74, "Update this list when new bone types are addded!");

	defRig.add_static_constant("BODY_PART_LOWER_BODY", pragma::math::to_integral(pragma::animation::BodyPart::LowerBody));
	defRig.add_static_constant("BODY_PART_UPPER_BODY", pragma::math::to_integral(pragma::animation::BodyPart::UpperBody));
	defRig.add_static_constant("BODY_PART_HEAD", pragma::math::to_integral(pragma::animation::BodyPart::Head));
	defRig.add_static_constant("BODY_PART_LEFT_ARM", pragma::math::to_integral(pragma::animation::BodyPart::LeftArm));
	defRig.add_static_constant("BODY_PART_RIGHT_ARM", pragma::math::to_integral(pragma::animation::BodyPart::RightArm));
	defRig.add_static_constant("BODY_PART_LEFT_LEG", pragma::math::to_integral(pragma::animation::BodyPart::LeftLeg));
	defRig.add_static_constant("BODY_PART_RIGHT_LEG", pragma::math::to_integral(pragma::animation::BodyPart::RightLeg));
	defRig.add_static_constant("BODY_PART_TAIL", pragma::math::to_integral(pragma::animation::BodyPart::Tail));
	defRig.add_static_constant("BODY_PART_LEFT_WING", pragma::math::to_integral(pragma::animation::BodyPart::LeftWing));
	defRig.add_static_constant("BODY_PART_RIGHT_WING", pragma::math::to_integral(pragma::animation::BodyPart::RightWing));
	defRig.add_static_constant("BODY_PART_LEFT_BREAST", pragma::math::to_integral(pragma::animation::BodyPart::LeftBreast));
	defRig.add_static_constant("BODY_PART_RIGHT_BREAST", pragma::math::to_integral(pragma::animation::BodyPart::RightBreast));
	static_assert(pragma::math::to_integral(pragma::animation::BodyPart::Count) == 12, "Update this list when new bone types are addded!");

	classDef.scope[defRig];

	// Flex Animation
	auto classDefFlexAnim = luabind::class_<FlexAnimation>("FlexAnimation");
	classDefFlexAnim.def("GetFps", static_cast<float (*)(lua::State *, FlexAnimation &)>([](lua::State *l, FlexAnimation &flexAnim) -> float { return flexAnim.GetFps(); }));
	classDefFlexAnim.def("SetFps", static_cast<void (*)(lua::State *, FlexAnimation &, float)>([](lua::State *l, FlexAnimation &flexAnim, float fps) { return flexAnim.SetFps(fps); }));
	classDefFlexAnim.def("GetFrames", static_cast<luabind::object (*)(lua::State *, FlexAnimation &)>([](lua::State *l, FlexAnimation &flexAnim) -> luabind::object { return vector_to_table(l, flexAnim.GetFrames()); }));
	classDefFlexAnim.def("GetFrame", static_cast<std::shared_ptr<FlexAnimationFrame> (*)(lua::State *, FlexAnimation &, uint32_t)>([](lua::State *l, FlexAnimation &flexAnim, uint32_t frameId) -> std::shared_ptr<FlexAnimationFrame> {
		auto &frames = flexAnim.GetFrames();
		if(frameId >= frames.size())
			return nullptr;
		return frames[frameId];
	}));
	classDefFlexAnim.def("GetFrameCount", static_cast<uint32_t (*)(lua::State *, FlexAnimation &)>([](lua::State *l, FlexAnimation &flexAnim) -> uint32_t { return flexAnim.GetFrames().size(); }));
	classDefFlexAnim.def("GetFlexControllerIds", static_cast<luabind::object (*)(lua::State *, FlexAnimation &)>([](lua::State *l, FlexAnimation &flexAnim) -> luabind::object { return vector_to_table(l, flexAnim.GetFlexControllerIds()); }));
	classDefFlexAnim.def("SetFlexControllerIds",
	  static_cast<void (*)(lua::State *, FlexAnimation &, luabind::table<>)>([](lua::State *l, FlexAnimation &flexAnim, luabind::table<> tIds) { flexAnim.SetFlexControllerIds(Lua::table_to_vector<pragma::animation::FlexControllerId>(l, tIds, 2)); }));
	classDefFlexAnim.def("AddFlexControllerId", static_cast<uint32_t (*)(lua::State *, FlexAnimation &, pragma::animation::FlexControllerId)>([](lua::State *l, FlexAnimation &flexAnim, pragma::animation::FlexControllerId id) -> uint32_t { return flexAnim.AddFlexControllerId(id); }));
	classDefFlexAnim.def("SetFlexControllerValue", static_cast<void (*)(lua::State *, FlexAnimation &, uint32_t, pragma::animation::FlexControllerId, float)>([](lua::State *l, FlexAnimation &flexAnim, uint32_t frameId, pragma::animation::FlexControllerId id, float val) {
		auto &frames = flexAnim.GetFrames();
		frames.reserve(frameId + 1);
		while(frames.size() <= frameId)
			flexAnim.AddFrame();

		auto &frame = frames[frameId];
		auto idx = flexAnim.AddFlexControllerId(id);
		frame->GetValues()[idx] = val;
	}));
	classDefFlexAnim.def("GetFlexControllerCount", static_cast<uint32_t (*)(lua::State *, FlexAnimation &)>([](lua::State *l, FlexAnimation &flexAnim) -> uint32_t {
		auto &flexControllerIds = flexAnim.GetFlexControllerIds();
		return flexControllerIds.size();
	}));
	classDefFlexAnim.def("LookupLocalFlexControllerIndex", static_cast<luabind::object (*)(lua::State *, FlexAnimation &, pragma::animation::FlexControllerId)>([](lua::State *l, FlexAnimation &flexAnim, pragma::animation::FlexControllerId id) -> luabind::object {
		auto &ids = flexAnim.GetFlexControllerIds();
		auto it = std::find(ids.begin(), ids.end(), id);
		if(it == ids.end())
			return {};
		return luabind::object {l, it - ids.begin()};
	}));
	classDefFlexAnim.def("AddFrame", static_cast<std::shared_ptr<FlexAnimationFrame> (*)(lua::State *, FlexAnimation &)>([](lua::State *l, FlexAnimation &flexAnim) -> std::shared_ptr<FlexAnimationFrame> { return flexAnim.AddFrame().shared_from_this(); }));
	classDefFlexAnim.def("ClearFrames", static_cast<void (*)(lua::State *, FlexAnimation &)>([](lua::State *l, FlexAnimation &flexAnim) { flexAnim.GetFrames().clear(); }));
	classDefFlexAnim.def("RemoveFrame", static_cast<void (*)(lua::State *, FlexAnimation &, uint32_t)>([](lua::State *l, FlexAnimation &flexAnim, uint32_t idx) {
		auto &frames = flexAnim.GetFrames();
		if(idx >= frames.size())
			return;
		frames.erase(frames.begin() + idx);
	}));
	classDefFlexAnim.def("Save", static_cast<void (*)(lua::State *, FlexAnimation &, ::udm::AssetData &)>([](lua::State *l, FlexAnimation &flexAnim, ::udm::AssetData &assetData) {
		std::string err;
		auto result = flexAnim.Save(assetData, err);
		if(result == false)
			PushString(l, err);
		else
			PushBool(l, result);
	}));
	classDefFlexAnim.scope[luabind::def("Load", static_cast<luabind::object (*)(lua::State *, LFile &)>([](lua::State *l, LFile &f) -> luabind::object {
		auto fptr = f.GetHandle();
		if(fptr == nullptr)
			return {};
		return luabind::object {l, FlexAnimation::Load(*fptr)};
	}))];
	classDefFlexAnim.scope[luabind::def("Load", static_cast<void (*)(lua::State *, ::udm::AssetData &)>([](lua::State *l, ::udm::AssetData &assetData) {
		std::string err;
		auto anim = FlexAnimation::Load(assetData, err);
		if(anim == nullptr) {
			PushBool(l, false);
			PushString(l, err);
			return;
		}
		Push(l, anim);
	}))];

	auto classDefFlexAnimFrame = luabind::class_<FlexAnimationFrame>("Frame");
	classDefFlexAnimFrame.def("GetFlexControllerValues", static_cast<luabind::object (*)(lua::State *, FlexAnimationFrame &)>([](lua::State *l, FlexAnimationFrame &flexAnimFrame) -> luabind::object { return vector_to_table(l, flexAnimFrame.GetValues()); }));
	classDefFlexAnimFrame.def("SetFlexControllerValues", static_cast<void (*)(lua::State *, FlexAnimationFrame &, luabind::table<>)>([](lua::State *l, FlexAnimationFrame &flexAnimFrame, luabind::table<> t) { flexAnimFrame.GetValues() = Lua::table_to_vector<float>(l, t, 2); }));
	classDefFlexAnimFrame.def("GetFlexControllerValue", static_cast<void (*)(lua::State *, FlexAnimationFrame &, uint32_t)>([](lua::State *l, FlexAnimationFrame &flexAnimFrame, uint32_t id) {
		auto &values = flexAnimFrame.GetValues();
		if(id >= values.size())
			return;
		PushNumber(l, values[id]);
	}));
	classDefFlexAnimFrame.def("GetFlexControllerValueCount", static_cast<uint32_t (*)(lua::State *, FlexAnimationFrame &)>([](lua::State *l, FlexAnimationFrame &flexAnimFrame) -> uint32_t {
		auto &values = flexAnimFrame.GetValues();
		return values.size();
	}));
	classDefFlexAnimFrame.def("SetFlexControllerValue", static_cast<void (*)(lua::State *, FlexAnimationFrame &, uint32_t, float)>([](lua::State *l, FlexAnimationFrame &flexAnimFrame, uint32_t id, float val) {
		auto &values = flexAnimFrame.GetValues();
		if(id >= values.size())
			return;
		values[id] = val;
	}));
	classDefFlexAnim.scope[classDefFlexAnimFrame];

	classDef.scope[classDefFlexAnim];

	// Vertex Animation
	auto classDefVertexAnimation = luabind::class_<pragma::animation::VertexAnimation>("VertexAnimation")
	                                 .def("Rotate", static_cast<void (*)(lua::State *, pragma::animation::VertexAnimation &, const Quat &)>([](lua::State *l, pragma::animation::VertexAnimation &vertAnim, const Quat &rot) { vertAnim.Rotate(rot); }))
	                                 .def("GetMeshAnimations", &VertexAnimation::GetMeshAnimations)
	                                 .def("GetName", &VertexAnimation::GetName);

	auto classDefMeshVertexFrame = luabind::class_<pragma::animation::MeshVertexFrame>("Frame")
	                                 .def("Rotate", static_cast<void (*)(lua::State *, pragma::animation::MeshVertexFrame &, const Quat &)>([](lua::State *l, pragma::animation::MeshVertexFrame &meshVertFrame, const Quat &rot) { meshVertFrame.Rotate(rot); }))
	                                 .def("GetVertices", &MeshVertexFrame::GetVertices)
	                                 .def("SetVertexCount", &MeshVertexFrame::SetVertexCount)
	                                 .def("SetVertexPosition", &MeshVertexFrame::SetVertexPosition)
	                                 .def("GetVertexPosition", &MeshVertexFrame::GetVertexPosition)
	                                 .def("SetVertexNormal", &MeshVertexFrame::SetVertexNormal)
	                                 .def("GetVertexNormal", &MeshVertexFrame::GetVertexNormal)
	                                 .def("GetVertexCount", static_cast<void (*)(lua::State *, pragma::animation::MeshVertexFrame &)>([](lua::State *l, pragma::animation::MeshVertexFrame &meshVertFrame) { PushInt(l, meshVertFrame.GetVertexCount()); }))
	                                 .def("GetFlags", static_cast<void (*)(lua::State *, pragma::animation::MeshVertexFrame &)>([](lua::State *l, pragma::animation::MeshVertexFrame &meshVertFrame) { PushInt(l, pragma::math::to_integral(meshVertFrame.GetFlags())); }))
	                                 .def("SetFlags", static_cast<void (*)(lua::State *, pragma::animation::MeshVertexFrame &, uint32_t)>([](lua::State *l, pragma::animation::MeshVertexFrame &meshVertFrame, uint32_t flags) { meshVertFrame.SetFlags(static_cast<pragma::animation::MeshVertexFrame::Flags>(flags)); }))
	                                 .def("SetDeltaValue", static_cast<void (*)(lua::State *, pragma::animation::MeshVertexFrame &, uint32_t, float)>([](lua::State *l, pragma::animation::MeshVertexFrame &meshVertFrame, uint32_t vertId, float value) { meshVertFrame.SetDeltaValue(vertId, value); }))
	                                 .def("GetDeltaValue", static_cast<void (*)(lua::State *, pragma::animation::MeshVertexFrame &, uint32_t)>([](lua::State *l, pragma::animation::MeshVertexFrame &meshVertFrame, uint32_t vertId) {
		                                 float value;
		                                 if(meshVertFrame.GetDeltaValue(vertId, value) == false)
			                                 return;
		                                 PushNumber(l, value);
	                                 }));
	classDefMeshVertexFrame.add_static_constant("FLAG_NONE", pragma::math::to_integral(pragma::animation::MeshVertexFrame::Flags::None));
	classDefMeshVertexFrame.add_static_constant("FLAG_BIT_HAS_DELTA_VALUES", pragma::math::to_integral(pragma::animation::MeshVertexFrame::Flags::HasDeltaValues));

	auto classDefMeshVertexAnimation = luabind::class_<pragma::animation::MeshVertexAnimation>("MeshAnimation")
	                                     .def("Rotate", static_cast<void (*)(lua::State *, pragma::animation::MeshVertexAnimation &, const Quat &)>([](lua::State *l, pragma::animation::MeshVertexAnimation &meshVertAnim, const Quat &rot) { meshVertAnim.Rotate(rot); }))
	                                     .def("GetFrames", &MeshVertexAnimation::GetFrames)
	                                     .def("GetMesh", &MeshVertexAnimation::GetMesh);
	classDefMeshVertexAnimation.scope[classDefMeshVertexFrame];
	classDefVertexAnimation.scope[classDefMeshVertexAnimation];

	auto classDefSkeleton = luabind::class_<pragma::animation::Skeleton>("Skeleton");
	classDefSkeleton.scope[luabind::def("create", +[]() { return pragma::util::make_shared<pragma::animation::Skeleton>(); })];
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
		  Con::COUT << ss.str() << Con::endl;
	  });
	classDefSkeleton.def("GetBone", &Skeleton::GetBone);
	classDefSkeleton.def("GetRootBones", &Skeleton::GetRootBones);
	classDefSkeleton.def("GetBones", &Skeleton::GetBones);
	classDefSkeleton.def("LookupBone", &Skeleton::LookupBone);
	classDefSkeleton.def("AddBone", static_cast<std::shared_ptr<pragma::animation::Bone> (*)(lua::State *, pragma::animation::Skeleton &, const std::string &, pragma::animation::Bone &)>(&Skeleton::AddBone));
	classDefSkeleton.def("AddBone", static_cast<std::shared_ptr<pragma::animation::Bone> (*)(lua::State *, pragma::animation::Skeleton &, const std::string &)>(&Skeleton::AddBone));
	classDefSkeleton.def("GetBoneCount", &pragma::animation::Skeleton::GetBoneCount);
	classDefSkeleton.def("Merge", &pragma::animation::Skeleton::Merge);
	classDefSkeleton.def("ClearBones", &Skeleton::ClearBones);
	classDefSkeleton.def("MakeRootBone", Skeleton::MakeRootBone);
	classDefSkeleton.def("GetBoneHierarchy", Skeleton::GetBoneHierarchy);
	classDefSkeleton.def("IsRootBone", static_cast<bool (*)(lua::State *, pragma::animation::Skeleton &, const std::string &)>(&Skeleton::IsRootBone));
	classDefSkeleton.def("IsRootBone", static_cast<bool (*)(lua::State *, pragma::animation::Skeleton &, uint32_t)>(&Skeleton::IsRootBone));
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
	Bone::register_class(l, classDefSkeleton);

	auto modelMeshGroupClassDef = luabind::class_<pragma::asset::ModelMeshGroup>("MeshGroup");
	ModelMeshGroup::register_class(modelMeshGroupClassDef);

	auto collisionMeshClassDef = luabind::class_<pragma::physics::CollisionMesh>("CollisionMesh");
	CollisionMesh::register_class(collisionMeshClassDef);

	// Vertex
	auto defVertex = luabind::class_<pragma::math::Vertex>("Vertex");
	defVertex.def(luabind::constructor<const Vector3 &, const ::Vector2 &, const Vector3 &, const ::Vector4 &>());
	defVertex.def(luabind::constructor<const Vector3 &, const ::Vector2 &, const Vector3 &>());
	defVertex.def(luabind::constructor<const Vector3 &, const Vector3 &>());
	defVertex.def(luabind::constructor<>());
	defVertex.def(luabind::tostring(luabind::self));
	defVertex.def(luabind::const_self == luabind::const_self);
	defVertex.def_readwrite("position", &pragma::math::Vertex::position);
	defVertex.def_readwrite("uv", &pragma::math::Vertex::uv);
	defVertex.def_readwrite("normal", &pragma::math::Vertex::normal);
	defVertex.def_readwrite("tangent", &pragma::math::Vertex::tangent);
	defVertex.def("Copy", &Vertex::Copy);
	defVertex.def("GetBiTangent", &pragma::math::Vertex::GetBiTangent);
	classDef.scope[defVertex];

	auto defVertWeight = luabind::class_<pragma::math::VertexWeight>("VertexWeight");
	defVertWeight.def(luabind::constructor<const ::Vector4i &, const ::Vector4 &>());
	defVertWeight.def(luabind::constructor<>());
	defVertWeight.def(luabind::tostring(luabind::self));
	defVertWeight.def(luabind::const_self == luabind::const_self);
	defVertWeight.def_readwrite("boneIds", &pragma::math::VertexWeight::boneIds);
	defVertWeight.def_readwrite("weights", &pragma::math::VertexWeight::weights);
	defVertWeight.def("Copy", &VertexWeight::Copy);
	classDef.scope[defVertWeight];

	// Joint
	auto defJoint = luabind::class_<pragma::physics::JointInfo>("Joint");
	defJoint.def_readwrite("collide", &pragma::physics::JointInfo::collide);

	defJoint.def("GetType", &Joint::GetType);
	defJoint.def("GetChildBoneId", &Joint::GetChildBoneId);
	defJoint.def("GetParentBoneId", &Joint::GetParentBoneId);
	defJoint.def("GetCollisionsEnabled", &Joint::GetCollisionsEnabled);
	defJoint.def("GetKeyValues", &Joint::GetKeyValues);

	defJoint.def("SetType", &Joint::SetType);
	defJoint.def("SetCollisionMeshId", &Joint::SetCollisionMeshId);
	defJoint.def("SetParentCollisionMeshId", &Joint::SetParentCollisionMeshId);
	defJoint.def("SetCollisionsEnabled", &Joint::SetCollisionsEnabled);
	defJoint.def("SetKeyValues", &Joint::SetKeyValues);
	defJoint.def("SetKeyValue", &Joint::SetKeyValue);
	defJoint.def("RemoveKeyValue", &Joint::RemoveKeyValue);
	defJoint.def("GetArgs", static_cast<luabind::object (*)(lua::State *, pragma::physics::JointInfo &)>([](lua::State *l, pragma::physics::JointInfo &jointInfo) -> luabind::object { return map_to_table(l, jointInfo.args); }));
	defJoint.def("SetArgs", static_cast<void (*)(lua::State *, pragma::physics::JointInfo &, luabind::table<>)>([](lua::State *l, pragma::physics::JointInfo &jointInfo, luabind::table<> t) { jointInfo.args = Lua::table_to_map<std::string, std::string>(l, t, 2); }));

	defJoint.add_static_constant("TYPE_NONE", pragma::math::to_integral(pragma::physics::JointType::None));
	defJoint.add_static_constant("TYPE_FIXED", pragma::math::to_integral(pragma::physics::JointType::Fixed));
	defJoint.add_static_constant("TYPE_BALLSOCKET", pragma::math::to_integral(pragma::physics::JointType::BallSocket));
	defJoint.add_static_constant("TYPE_HINGE", pragma::math::to_integral(pragma::physics::JointType::Hinge));
	defJoint.add_static_constant("TYPE_SLIDER", pragma::math::to_integral(pragma::physics::JointType::Slider));
	defJoint.add_static_constant("TYPE_CONETWIST", pragma::math::to_integral(pragma::physics::JointType::ConeTwist));
	defJoint.add_static_constant("TYPE_DOF", pragma::math::to_integral(pragma::physics::JointType::DOF));
	classDef.scope[defJoint];

	auto defBoxCreateInfo = luabind::class_<pragma::geometry::BoxCreateInfo>("BoxCreateInfo");
	defBoxCreateInfo.def(luabind::constructor<const Vector3 &, const Vector3 &>());
	defBoxCreateInfo.def(luabind::constructor<>());
	defBoxCreateInfo.def_readwrite("min", &pragma::geometry::BoxCreateInfo::min);
	defBoxCreateInfo.def_readwrite("max", &pragma::geometry::BoxCreateInfo::max);
	classDef.scope[defBoxCreateInfo];

	auto defSphereCreateInfo = luabind::class_<pragma::geometry::SphereCreateInfo>("SphereCreateInfo");
	defSphereCreateInfo.def(luabind::constructor<const Vector3 &, float>());
	defSphereCreateInfo.def(luabind::constructor<>());
	defSphereCreateInfo.def_readwrite("origin", &pragma::geometry::SphereCreateInfo::origin);
	defSphereCreateInfo.def_readwrite("radius", &pragma::geometry::SphereCreateInfo::radius);
	defSphereCreateInfo.def_readwrite("recursionLevel", &pragma::geometry::SphereCreateInfo::recursionLevel);
	classDef.scope[defSphereCreateInfo];

	auto defCylinderCreateInfo = luabind::class_<pragma::geometry::CylinderCreateInfo>("CylinderCreateInfo");
	defCylinderCreateInfo.def(luabind::constructor<float, float>());
	defCylinderCreateInfo.def(luabind::constructor<>());
	defCylinderCreateInfo.def_readwrite("radius", &pragma::geometry::CylinderCreateInfo::radius);
	defCylinderCreateInfo.def_readwrite("length", &pragma::geometry::CylinderCreateInfo::length);
	defCylinderCreateInfo.def_readwrite("segmentCount", &pragma::geometry::CylinderCreateInfo::segmentCount);
	classDef.scope[defCylinderCreateInfo];

	auto defConeCreateInfo = luabind::class_<pragma::geometry::ConeCreateInfo>("ConeCreateInfo");
	defConeCreateInfo.def(luabind::constructor<pragma::math::Degree, float>());
	defConeCreateInfo.def(luabind::constructor<float, float, float>());
	defConeCreateInfo.def(luabind::constructor<>());
	defConeCreateInfo.def_readwrite("startRadius", &pragma::geometry::ConeCreateInfo::startRadius);
	defConeCreateInfo.def_readwrite("length", &pragma::geometry::ConeCreateInfo::length);
	defConeCreateInfo.def_readwrite("endRadius", &pragma::geometry::ConeCreateInfo::endRadius);
	defConeCreateInfo.def_readwrite("segmentCount", &pragma::geometry::ConeCreateInfo::segmentCount);
	classDef.scope[defConeCreateInfo];

	auto defEllipticConeCreateInfo = luabind::class_<pragma::geometry::EllipticConeCreateInfo, pragma::geometry::ConeCreateInfo>("EllipticConeCreateInfo");
	defEllipticConeCreateInfo.def(luabind::constructor<pragma::math::Degree, pragma::math::Degree, float>());
	defEllipticConeCreateInfo.def(luabind::constructor<float, float, float, float, float>());
	defEllipticConeCreateInfo.def(luabind::constructor<>());
	defEllipticConeCreateInfo.def_readwrite("startRadiusY", &pragma::geometry::EllipticConeCreateInfo::startRadiusY);
	defEllipticConeCreateInfo.def_readwrite("endRadiusY", &pragma::geometry::EllipticConeCreateInfo::endRadiusY);
	classDef.scope[defEllipticConeCreateInfo];

	auto defCircleCreateInfo = luabind::class_<pragma::geometry::CircleCreateInfo>("CircleCreateInfo");
	defCircleCreateInfo.def(luabind::constructor<float, bool>());
	defCircleCreateInfo.def(luabind::constructor<float, bool>(), luabind::default_parameter_policy<2, true> {});
	defCircleCreateInfo.def(luabind::constructor<>());
	defCircleCreateInfo.def_readwrite("radius", &pragma::geometry::CircleCreateInfo::radius);
	defCircleCreateInfo.def_readwrite("doubleSided", &pragma::geometry::CircleCreateInfo::doubleSided);
	defCircleCreateInfo.def_readwrite("segmentCount", &pragma::geometry::CircleCreateInfo::segmentCount);
	defCircleCreateInfo.def_readwrite("totalAngle", &pragma::geometry::CircleCreateInfo::totalAngle);
	classDef.scope[defCircleCreateInfo];

	auto defRingCreateInfo = luabind::class_<pragma::geometry::RingCreateInfo>("RingCreateInfo");
	defRingCreateInfo.def(luabind::constructor<float, float, bool>());
	defRingCreateInfo.def(luabind::constructor<float, float, bool>(), luabind::default_parameter_policy<3, true> {});
	defRingCreateInfo.def(luabind::constructor<>());
	defRingCreateInfo.def_readwrite("innerRadius", &pragma::geometry::RingCreateInfo::innerRadius);
	defRingCreateInfo.def_readwrite("outerRadius", &pragma::geometry::RingCreateInfo::outerRadius);
	defRingCreateInfo.def_readwrite("doubleSided", &pragma::geometry::RingCreateInfo::doubleSided);
	defRingCreateInfo.def_readwrite("segmentCount", &pragma::geometry::RingCreateInfo::segmentCount);
	defRingCreateInfo.def_readwrite("totalAngle", &pragma::geometry::RingCreateInfo::totalAngle);
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

void Lua::Model::GetCollisionMeshes(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &meshes = mdl.GetCollisionMeshes();
	CreateTable(l);
	int top = GetStackTop(l);
	for(auto i = decltype(meshes.size()) {0}; i < meshes.size(); ++i) {
		Lua::Push<std::shared_ptr<pragma::physics::CollisionMesh>>(l, meshes[i]);
		SetTableValue(l, top, i + 1);
	}
}

void Lua::Model::ClearCollisionMeshes(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.GetCollisionMeshes().clear();
}

void Lua::Model::GetSkeleton(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &skeleton = mdl.GetSkeleton();
	luabind::object(l, &skeleton).push(l);
}

static void push_attachment(lua::State *l, const pragma::asset::Attachment &att)
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
void Lua::Model::GetAttachmentCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetAttachments().size());
}
void Lua::Model::GetAttachment(lua::State *l, pragma::asset::Model &mdl, int32_t attId)
{
	//Lua::CheckModel(l,1);
	auto *att = mdl.GetAttachment(attId);
	if(att == nullptr)
		return;
	push_attachment(l, *att);
}
void Lua::Model::GetAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *att = mdl.GetAttachment(name);
	if(att == nullptr)
		return;
	push_attachment(l, *att);
}
void Lua::Model::GetAttachments(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &attachments = mdl.GetAttachments();
	auto t = CreateTable(l);
	for(auto i = decltype(attachments.size()) {0}; i < attachments.size(); ++i) {
		auto &att = attachments[i];

		PushInt(l, i + 1);
		push_attachment(l, att);

		SetTableValue(l, t);
	}
}

static void push_blend_controller(lua::State *l, const pragma::asset::BlendController &blendController)
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
void Lua::Model::LookupAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto id = mdl.LookupAnimation(name);
	PushInt(l, id);
}
void Lua::Model::LookupAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto attId = mdl.LookupAttachment(name);
	PushInt(l, attId);
}
void Lua::Model::LookupBone(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto boneId = mdl.LookupBone(name);
	PushInt(l, boneId);
}
void Lua::Model::AddAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name, const std::string &boneName, const Vector3 &offset, const EulerAngles &ang)
{
	//Lua::CheckModel(l,1);
	auto boneId = mdl.LookupBone(boneName);
	if(boneId < 0)
		return;
	AddAttachment(l, mdl, name, boneId, offset, ang);
}
void Lua::Model::AddAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name, uint32_t boneId, const Vector3 &offset, const EulerAngles &ang)
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
static void get_attachment(lua::State *l, LuaAttachmentData &att, int32_t t)
{
	Lua::PushString(l, "angles");
	Lua::GetTableValue(l, t);
	if(Lua::IsNil(l, -1) == false)
		att.angles = &Lua::Check<EulerAngles>(l, -1);
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
		att.offset = &Lua::Check<Vector3>(l, -1);
	Lua::Pop(l, 1);
}
void Lua::Model::SetAttachmentData(lua::State *l, pragma::asset::Model &mdl, const std::string &name, luabind::object data)
{
	//Lua::CheckModel(l,1);
	int32_t t = 3;
	CheckTable(l, t);
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
void Lua::Model::SetAttachmentData(lua::State *l, pragma::asset::Model &mdl, uint32_t attId, luabind::object data)
{
	//Lua::CheckModel(l,1);
	int32_t t = 3;
	CheckTable(l, t);
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
void Lua::Model::RemoveAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveAttachment(name);
}
void Lua::Model::RemoveAttachment(lua::State *l, pragma::asset::Model &mdl, uint32_t attId)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveAttachment(attId);
}
void Lua::Model::GetBlendControllerCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetBlendControllers().size());
}
void Lua::Model::GetBlendController(lua::State *l, pragma::asset::Model &mdl, int32_t blendControllerId)
{
	//Lua::CheckModel(l,1);
	auto *blendController = mdl.GetBlendController(blendControllerId);
	if(blendController == nullptr)
		return;
	push_blend_controller(l, *blendController);
}
void Lua::Model::GetBlendController(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto *blendController = mdl.GetBlendController(name);
	if(blendController == nullptr)
		return;
	push_blend_controller(l, *blendController);
}
void Lua::Model::GetBlendControllers(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto t = CreateTable(l);
	auto &blendControllers = mdl.GetBlendControllers();
	for(auto i = decltype(blendControllers.size()) {0}; i < blendControllers.size(); ++i) {
		auto &blendController = blendControllers[i];

		PushInt(l, i + 1);
		push_blend_controller(l, blendController);

		SetTableValue(l, t);
	}
}
void Lua::Model::LookupBlendController(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.LookupBlendController(name));
}

void Lua::Model::GetAnimationCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	std::unordered_map<std::string, uint32_t> *anims;
	mdl.GetAnimations(&anims);
	PushInt(l, anims->size());
}

void Lua::Model::GetAnimationNames(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	std::unordered_map<std::string, unsigned int> *anims;
	mdl.GetAnimations(&anims);
	std::unordered_map<std::string, unsigned int>::iterator i;
	CreateTable(l);
	int top = GetStackTop(l);
	int n = 1;
	for(i = anims->begin(); i != anims->end(); i++) {
		PushString(l, i->first.c_str());
		SetTableValue(l, top, n);
		n++;
	}
}

void Lua::Model::GetAnimations(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto t = CreateTable(l);
	auto &anims = mdl.GetAnimations();
	int32_t idx = 1;
	for(auto &anim : anims) {
		PushInt(l, idx++);
		Lua::Push<std::shared_ptr<pragma::animation::Animation>>(l, anim);
		SetTableValue(l, t);
	}
}

void Lua::Model::GetAnimationName(lua::State *l, pragma::asset::Model &mdl, unsigned int animID)
{
	//Lua::CheckModel(l,1);
	std::string name;
	if(mdl.GetAnimationName(animID, name) == false)
		return;
	PushString(l, name);
}

void Lua::Model::GetAnimation(lua::State *l, pragma::asset::Model &mdl, unsigned int animID)
{
	//Lua::CheckModel(l,1);
	auto anim = mdl.GetAnimation(animID);
	if(anim == nullptr)
		return;
	Lua::Push<std::shared_ptr<pragma::animation::Animation>>(l, anim);
}

void Lua::Model::GetAnimation(lua::State *l, pragma::asset::Model &mdl, const char *name)
{
	//Lua::CheckModel(l,1);
	int animID = mdl.LookupAnimation(name);
	if(animID == -1)
		return;
	GetAnimation(l, mdl, animID);
}

void Lua::Model::PrecacheTextureGroup(lua::State *, pragma::asset::Model &mdl, unsigned int group)
{
	//Lua::CheckModel(l,1);
	mdl.PrecacheTextureGroup(group);
}

void Lua::Model::PrecacheTextureGroups(lua::State *, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.PrecacheTextureGroups();
}

void Lua::Model::GetReference(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &ref = mdl.GetReference();
	Lua::Push<std::shared_ptr<::Frame>>(l, ref.shared_from_this());
}
/*void Lua::Model::GetReferenceBoneMatrix(lua::State *l,::Model &mdl,uint32_t boneId)
{
	//Lua::CheckModel(l,1);
	auto *mat = mdl.GetBindPoseBoneMatrix(boneId);
	if(mat == nullptr)
		return;
	Lua::Push<Mat4>(l,*mat);
}
void Lua::Model::SetReferenceBoneMatrix(lua::State *l,::Model &mdl,uint32_t boneId,const Mat4 &mat)
{
	//Lua::CheckModel(l,1);
	mdl.SetBindPoseBoneMatrix(boneId,mat);
}*/
void Lua::Model::GetLocalBonePosition(lua::State *l, pragma::asset::Model &mdl, UInt32 animId, UInt32 frameId, UInt32 boneId)
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
void Lua::Model::LookupBodyGroup(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto id = mdl.GetBodyGroupId(name);
	PushInt(l, id);
}
void Lua::Model::GetBaseMeshGroupIds(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &baseMeshes = mdl.GetBaseMeshes();
	auto t = CreateTable(l);
	int32_t n = 1;
	for(auto &meshId : baseMeshes) {
		PushInt(l, n);
		PushInt(l, meshId);
		SetTableValue(l, t);
		++n;
	}
}
void Lua::Model::SetBaseMeshGroupIds(lua::State *l, pragma::asset::Model &mdl, luabind::object o)
{
	//Lua::CheckModel(l,1);
	int32_t tIdx = 2;
	CheckTable(l, tIdx);
	std::vector<uint32_t> ids;
	auto numIds = GetObjectLength(l, tIdx);
	ids.reserve(numIds);
	for(auto i = decltype(numIds) {0}; i < numIds; ++i) {
		PushInt(l, i + 1);
		GetTableValue(l, tIdx);
		auto groupId = CheckInt(l, -1);
		ids.push_back(groupId);

		Pop(l, 1);
	}
	mdl.GetBaseMeshes() = ids;
}
void Lua::Model::AddBaseMeshGroupId(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto groupId = CheckInt(l, 1);
	auto &ids = mdl.GetBaseMeshes();
	auto it = std::find(ids.begin(), ids.end(), groupId);
	if(it != ids.end())
		return;
	ids.push_back(groupId);
}
void Lua::Model::GetMeshGroupId(lua::State *l, pragma::asset::Model &mdl, uint32_t bodyGroupId, uint32_t groupId)
{
	//Lua::CheckModel(l,1);
	uint32_t meshId = uint32_t(-1);
	auto r = mdl.GetMesh(bodyGroupId, groupId, meshId);
	// UNUSED(r);
	PushInt(l, static_cast<int32_t>(meshId));
}

void Lua::Model::GetMeshGroup(lua::State *l, pragma::asset::Model &mdl, const std::string &meshGroupName)
{
	//Lua::CheckModel(l,1);
	auto meshGroup = mdl.GetMeshGroup(meshGroupName);
	if(meshGroup == nullptr)
		return;
	Lua::Push<std::shared_ptr<pragma::asset::ModelMeshGroup>>(l, meshGroup);
}

void Lua::Model::GetMeshGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t mgId)
{
	//Lua::CheckModel(l,1);
	auto group = mdl.GetMeshGroup(mgId);
	if(group == nullptr)
		return;
	Lua::Push<decltype(group)>(l, group);
}

void Lua::Model::GetMeshes(lua::State *l, pragma::asset::Model &mdl, const std::string &meshGroup)
{
	//Lua::CheckModel(l,1);
	auto *meshes = mdl.GetMeshes(meshGroup);
	auto t = CreateTable(l);
	if(meshes != nullptr) {
		int32_t n = 1;
		for(auto &mesh : *meshes) {
			PushInt(l, n);
			Lua::Push<std::shared_ptr<pragma::geometry::ModelMesh>>(l, mesh);
			SetTableValue(l, t);
			++n;
		}
	}
}

void Lua::Model::GetMeshes(lua::State *l, pragma::asset::Model &mdl, luabind::object o)
{
	//Lua::CheckModel(l,1);
	CheckTable(l, 2);
	std::vector<uint32_t> meshIds;
	PushNil(l);
	while(GetNextPair(l, 2) != 0) {
		auto meshId = CheckInt(l, -1);
		meshIds.push_back(static_cast<uint32_t>(meshId));
		Pop(l, 1);
	}

	std::vector<std::shared_ptr<pragma::geometry::ModelMesh>> meshes;
	mdl.GetMeshes(meshIds, meshes);

	auto t = CreateTable(l);
	int32_t n = 1;
	for(auto &mesh : meshes) {
		PushInt(l, n);
		Lua::Push<std::shared_ptr<pragma::geometry::ModelMesh>>(l, mesh);
		SetTableValue(l, t);
		++n;
	}
}

void Lua::Model::AddMeshGroup(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto meshGroup = mdl.AddMeshGroup(name);
	Lua::Push<std::shared_ptr<pragma::asset::ModelMeshGroup>>(l, meshGroup);
}

void Lua::Model::AddMeshGroup(lua::State *, pragma::asset::Model &mdl, pragma::asset::ModelMeshGroup &meshGroup)
{
	//Lua::CheckModel(l,1);
	auto pMeshGroup = meshGroup.shared_from_this();
	mdl.AddMeshGroup(pMeshGroup);
}

void Lua::Model::UpdateCollisionBounds(lua::State *, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.CalculateCollisionBounds();
}
void Lua::Model::UpdateRenderBounds(lua::State *, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.CalculateRenderBounds();
}
void Lua::Model::Update(lua::State *, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.Update();
}
void Lua::Model::Update(lua::State *, pragma::asset::Model &mdl, uint32_t flags)
{
	//Lua::CheckModel(l,1);
	mdl.Update(static_cast<pragma::asset::ModelUpdateFlags>(flags));
}
void Lua::Model::GetName(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushString(l, mdl.GetName());
}
void Lua::Model::GetMass(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushNumber(l, mdl.GetMass());
}
void Lua::Model::SetMass(lua::State *, pragma::asset::Model &mdl, float mass)
{
	//Lua::CheckModel(l,1);
	mdl.SetMass(mass);
}
void Lua::Model::GetBoneCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetBoneCount());
}
void Lua::Model::GetCollisionBounds(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Vector3 min, max;
	mdl.GetCollisionBounds(min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}
void Lua::Model::GetRenderBounds(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Vector3 min, max;
	mdl.GetRenderBounds(min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}
void Lua::Model::SetCollisionBounds(lua::State *, pragma::asset::Model &mdl, const Vector3 &min, const Vector3 &max)
{
	//Lua::CheckModel(l,1);
	mdl.SetCollisionBounds(min, max);
}
void Lua::Model::SetRenderBounds(lua::State *, pragma::asset::Model &mdl, const Vector3 &min, const Vector3 &max)
{
	//Lua::CheckModel(l,1);
	mdl.SetRenderBounds(min, max);
}
void Lua::Model::AddCollisionMesh(lua::State *, pragma::asset::Model &mdl, pragma::physics::CollisionMesh &colMesh)
{
	//Lua::CheckModel(l,1);
	mdl.AddCollisionMesh(colMesh.shared_from_this());
}
void Lua::Model::AddMaterial(lua::State *l, pragma::asset::Model &mdl, uint32_t textureGroup, pragma::material::Material *mat)
{
	if(!mat)
		return;
	//Lua::CheckModel(l,1);
	std::optional<uint32_t> skinTexIdx {};
	auto r = mdl.AddMaterial(textureGroup, mat, {}, &skinTexIdx);
	PushInt(l, r);
	if(skinTexIdx.has_value())
		PushInt(l, *skinTexIdx);
}
void Lua::Model::SetMaterial(lua::State *l, pragma::asset::Model &mdl, uint32_t matId, pragma::material::Material *mat)
{
	//Lua::CheckModel(l,1);
	mdl.SetMaterial(matId, mat);
}
void Lua::Model::GetMaterials(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto &matManager = nw->GetMaterialManager();
	auto t = CreateTable(l);
	uint32_t idx = 1;
	auto &mats = mdl.GetMaterials();
	for(auto &mat : mats) {
		auto *pmat = mat.get();
		if(pmat == nullptr)
			pmat = matManager.GetErrorMaterial();
		PushInt(l, idx++);
		Lua::Push<pragma::material::Material *>(l, pmat);
		SetTableValue(l, t);
	}
}
void Lua::Model::GetMaterialCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetMaterials().size());
}
void Lua::Model::GetMeshGroupCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetMeshGroupCount());
}
void Lua::Model::GetMeshCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetMeshCount());
}
void Lua::Model::GetSubMeshCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetSubMeshCount());
}
void Lua::Model::GetCollisionMeshCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetCollisionMeshCount());
}
void Lua::Model::GetBodyGroupId(lua::State *l, pragma::asset::Model &mdl, const std::string &bodyGroupName)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetBodyGroupId(bodyGroupName));
}
void Lua::Model::GetBodyGroupCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetBodyGroupCount());
}
static void push_body_group(lua::State *l, const pragma::asset::BodyGroup &bg)
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
void Lua::Model::GetBodyGroups(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto t = CreateTable(l);
	auto &bodyGroups = mdl.GetBodyGroups();
	for(auto i = decltype(bodyGroups.size()) {0u}; i < bodyGroups.size(); ++i) {
		auto &bg = bodyGroups.at(i);
		PushInt(l, i + 1u);
		push_body_group(l, bg);
		SetTableValue(l, t);
	}
}
void Lua::Model::GetBodyGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t bgId)
{
	//Lua::CheckModel(l,1);
	auto *bg = mdl.GetBodyGroup(bgId);
	if(bg == nullptr)
		return;
	push_body_group(l, *bg);
}
void Lua::Model::AddHitbox(lua::State *, pragma::asset::Model &mdl, uint32_t boneId, uint32_t hitGroup, const Vector3 &min, const Vector3 &max)
{
	//Lua::CheckModel(l,1);
	mdl.AddHitbox(boneId, static_cast<pragma::physics::HitGroup>(hitGroup), min, max);
}
void Lua::Model::GetHitboxCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetHitboxCount());
}
void Lua::Model::GetHitboxGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetHitboxGroup(boneId));
}
void Lua::Model::GetHitboxBounds(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId)
{
	//Lua::CheckModel(l,1);
	Vector3 min {0.f, 0.f, 0.f};
	Vector3 max {0.f, 0.f, 0.f};
	mdl.GetHitboxBounds(boneId, min, max);
	Lua::Push<Vector3>(l, min);
	Lua::Push<Vector3>(l, max);
}
void Lua::Model::GetHitboxBones(lua::State *l, pragma::asset::Model &mdl, uint32_t hitGroup)
{
	//Lua::CheckModel(l,1);
	auto boneIds = mdl.GetHitboxBones(static_cast<pragma::physics::HitGroup>(hitGroup));
	auto t = CreateTable(l);
	for(auto i = decltype(boneIds.size()) {0}; i < boneIds.size(); ++i) {
		PushInt(l, i + 1);
		PushInt(l, boneIds[i]);
		SetTableValue(l, t);
	}
}
void Lua::Model::GetHitboxBones(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto boneIds = mdl.GetHitboxBones();
	auto t = CreateTable(l);
	for(auto i = decltype(boneIds.size()) {0}; i < boneIds.size(); ++i) {
		PushInt(l, i + 1);
		PushInt(l, boneIds[i]);
		SetTableValue(l, t);
	}
}
void Lua::Model::SetHitboxGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId, uint32_t hitGroup)
{
	//Lua::CheckModel(l,1);
	auto &hitboxes = mdl.GetHitboxes();
	auto it = hitboxes.find(boneId);
	if(it == hitboxes.end())
		return;
	it->second.group = static_cast<pragma::physics::HitGroup>(hitGroup);
}
void Lua::Model::SetHitboxBounds(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId, const Vector3 &min, const Vector3 &max)
{
	//Lua::CheckModel(l,1);
	auto &hitboxes = mdl.GetHitboxes();
	auto it = hitboxes.find(boneId);
	if(it == hitboxes.end())
		return;
	it->second.min = min;
	it->second.max = max;
}
void Lua::Model::RemoveHitbox(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId)
{
	//Lua::CheckModel(l,1);
	auto &hitboxes = mdl.GetHitboxes();
	auto it = hitboxes.find(boneId);
	if(it == hitboxes.end())
		return;
	hitboxes.erase(it);
}
void Lua::Model::GetTextureGroupCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetTextureGroups().size());
}
static void push_texture_group(lua::State *l, pragma::asset::TextureGroup &group)
{
	auto tGroup = Lua::CreateTable(l);
	for(auto j = decltype(group.textures.size()) {0}; j < group.textures.size(); ++j) {
		Lua::PushInt(l, j + 1);
		Lua::PushInt(l, group.textures[j]);
		Lua::SetTableValue(l, tGroup);
	}
}
void Lua::Model::GetTextureGroups(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &texGroups = mdl.GetTextureGroups();
	auto t = CreateTable(l);
	for(auto i = decltype(texGroups.size()) {0}; i < texGroups.size(); ++i) {
		auto &group = texGroups[i];

		PushInt(l, i + 1);
		push_texture_group(l, group);
		SetTableValue(l, t);
	}
}
void Lua::Model::GetTextureGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t id)
{
	//Lua::CheckModel(l,1);
	auto *group = mdl.GetTextureGroup(id);
	if(group == nullptr)
		return;
	push_texture_group(l, *group);
}

void Lua::Model::Save(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto mdlName = name;
	std::string rootPath;
	if(file::validate_write_operation(l, mdlName, rootPath) == false) {
		PushBool(l, false);
		return;
	}
	auto r = mdl.SaveLegacy(pragma::Engine::Get()->GetNetworkState(l)->GetGameState(), mdlName, rootPath);
	PushBool(l, r);
}

void Lua::Model::Copy(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto cpy = mdl.Copy(pragma::Engine::Get()->GetNetworkState(l)->GetGameState());
	Lua::Push<decltype(cpy)>(l, cpy);
}

void Lua::Model::Copy(lua::State *l, pragma::asset::Model &mdl, uint32_t copyFlags)
{
	//Lua::CheckModel(l,1);
	auto cpy = mdl.Copy(pragma::Engine::Get()->GetNetworkState(l)->GetGameState(), static_cast<pragma::asset::Model::CopyFlags>(copyFlags));
	Lua::Push<decltype(cpy)>(l, cpy);
}

void Lua::Model::GetVertexCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetVertexCount());
}
void Lua::Model::GetTriangleCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetTriangleCount());
}
void Lua::Model::GetTextures(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &meta = mdl.GetMetaInfo();
	auto tTextures = CreateTable(l);
	for(auto i = decltype(meta.textures.size()) {0}; i < meta.textures.size(); ++i) {
		PushInt(l, i + 1);
		PushString(l, meta.textures[i]);
		SetTableValue(l, tTextures);
	}
}
void Lua::Model::GetTexturePaths(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &meta = mdl.GetMetaInfo();
	auto tTexturePaths = CreateTable(l);
	for(auto i = decltype(meta.texturePaths.size()) {0}; i < meta.texturePaths.size(); ++i) {
		PushInt(l, i + 1);
		PushString(l, meta.texturePaths[i]);
		SetTableValue(l, tTexturePaths);
	}
}
void Lua::Model::LoadMaterials(lua::State *l, pragma::asset::Model &mdl) { LoadMaterials(l, mdl, false); }
void Lua::Model::LoadMaterials(lua::State *l, pragma::asset::Model &mdl, bool bReload)
{
	//Lua::CheckModel(l,1);
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	mdl.LoadMaterials(bReload);
}
void Lua::Model::AddTexturePath(lua::State *, pragma::asset::Model &mdl, const std::string &path)
{
	//Lua::CheckModel(l,1);
	mdl.AddTexturePath(path);
}

void Lua::Model::RemoveTexturePath(lua::State *, pragma::asset::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveTexturePath(idx);
}
void Lua::Model::SetTexturePaths(lua::State *l, pragma::asset::Model &mdl, luabind::object o)
{
	//Lua::CheckModel(l,1);
	CheckTable(l, 2);
	std::vector<std::string> texturePaths;
	auto num = GetObjectLength(l, 2);
	texturePaths.reserve(num);
	PushNil(l);
	while(GetNextPair(l, 2) != 0) {
		auto *path = CheckString(l, -1);
		texturePaths.push_back(path);
		Pop(l, 1);
	}
	mdl.SetTexturePaths(texturePaths);
}
void Lua::Model::RemoveTexture(lua::State *, pragma::asset::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveTexture(idx);
}
void Lua::Model::ClearTextures(lua::State *, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.ClearTextures();
}
void Lua::Model::Rotate(lua::State *, pragma::asset::Model &mdl, const Quat &rot)
{
	//Lua::CheckModel(l,1);
	mdl.Rotate(rot);
}
void Lua::Model::Translate(lua::State *, pragma::asset::Model &mdl, const Vector3 &t)
{
	//Lua::CheckModel(l,1);
	mdl.Translate(t);
}
void Lua::Model::Scale(lua::State *, pragma::asset::Model &mdl, const Vector3 &scale)
{
	//Lua::CheckModel(l,1);
	mdl.Scale(scale);
}
void Lua::Model::GetEyeOffset(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	Lua::Push<Vector3>(l, mdl.GetEyeOffset());
}
void Lua::Model::SetEyeOffset(lua::State *l, pragma::asset::Model &mdl, const Vector3 &offset)
{
	//Lua::CheckModel(l,1);
	mdl.SetEyeOffset(offset);
}
void Lua::Model::AddAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &name, pragma::animation::Animation &anim)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.AddAnimation(name, anim.shared_from_this()));
}
void Lua::Model::RemoveAnimation(lua::State *l, pragma::asset::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	auto &anims = mdl.GetAnimations();
	if(idx >= anims.size())
		return;
	anims.erase(anims.begin() + idx);
}
void Lua::Model::ClearAnimations(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.GetAnimations().clear();
	mdl.GetAnimationNames().clear();
}
void Lua::Model::ClipAgainstPlane(lua::State *l, pragma::asset::Model &mdl, const Vector3 &n, double d, pragma::asset::Model &clippedMdlA, pragma::asset::Model &clippedMdlB)
{
	//Lua::CheckModel(l,1);
	mdl.ClipAgainstPlane(n, d, clippedMdlA, clippedMdlB);
}
void Lua::Model::ClearMeshGroups(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.GetMeshGroups() = {};
	mdl.GetBaseMeshes() = {};
}
void Lua::Model::RemoveMeshGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t idx)
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
void Lua::Model::ClearBaseMeshGroupIds(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.GetBaseMeshes() = {};
}
void Lua::Model::AddTextureGroup(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	mdl.CreateTextureGroup();
	PushInt(l, mdl.GetTextureGroups().size() - 1);
}
void Lua::Model::Merge(lua::State *l, pragma::asset::Model &mdl, pragma::asset::Model &mdlOther)
{
	//Lua::CheckModel(l,1);
	mdl.Merge(mdlOther);
}
void Lua::Model::Merge(lua::State *l, pragma::asset::Model &mdl, pragma::asset::Model &mdlOther, uint32_t mergeFlags)
{
	//Lua::CheckModel(l,1);
	mdl.Merge(mdlOther, static_cast<pragma::asset::Model::MergeFlags>(mergeFlags));
}
void Lua::Model::GetLODCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetLODCount());
}
static void push_lod(lua::State *l, const pragma::asset::LODInfo &info)
{
	auto tLod = Lua::CreateTable(l);

	Lua::PushString(l, "lod");
	Lua::PushInt(l, info.lod);
	Lua::SetTableValue(l, tLod);

	Lua::PushString(l, "meshGroupReplacements");
	auto tGroupReplacements = Lua::CreateTable(l);
	for(auto &pair : info.meshReplacements) {
		Lua::PushInt(l, pair.first);
		int32_t meshId = (pair.second == pragma::asset::MODEL_NO_MESH) ? -1 : pair.second;
		Lua::PushInt(l, meshId);
		Lua::SetTableValue(l, tGroupReplacements);
	}
	Lua::SetTableValue(l, tLod);
}
void Lua::Model::GetLODData(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &lods = mdl.GetLODs();
	auto tLods = CreateTable(l);
	for(auto i = decltype(lods.size()) {0}; i < lods.size(); ++i) {
		auto &lodInfo = lods.at(i);
		PushInt(l, i + 1);
		push_lod(l, lodInfo);
		SetTableValue(l, tLods);
	}
}
void Lua::Model::GetLOD(lua::State *l, pragma::asset::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	auto lod = mdl.GetLOD(idx);
	PushInt(l, lod);
}
void Lua::Model::GetLODData(lua::State *l, pragma::asset::Model &mdl, uint32_t lod)
{
	//Lua::CheckModel(l,1);
	auto *lodInfo = mdl.GetLODInfo(lod);
	if(lodInfo == nullptr)
		return;
	push_lod(l, *lodInfo);
}
void Lua::Model::TranslateLODMeshes(lua::State *l, pragma::asset::Model &mdl, uint32_t lod)
{
	//Lua::CheckModel(l,1);
	std::vector<uint32_t> meshIds;
	auto numMeshGroups = mdl.GetMeshGroupCount();
	meshIds.reserve(numMeshGroups);
	for(auto i = decltype(numMeshGroups) {0}; i < numMeshGroups; ++i)
		meshIds.push_back(i);
	auto r = mdl.TranslateLODMeshes(lod, meshIds);
	PushBool(l, r);
	if(r == false)
		return;
	auto t = CreateTable(l);
	for(auto i = decltype(meshIds.size()) {0}; i < meshIds.size(); ++i) {
		auto id = meshIds.at(i);
		PushInt(l, i + 1);
		PushInt(l, (id == pragma::asset::MODEL_NO_MESH) ? -1 : static_cast<int32_t>(id));
		SetTableValue(l, t);
	}
}
void Lua::Model::TranslateLODMeshes(lua::State *l, pragma::asset::Model &mdl, uint32_t lod, luabind::object o)
{
	//Lua::CheckModel(l,1);
	int32_t t = 3;
	CheckTable(l, t);

	std::vector<uint32_t> meshIds;
	auto numMeshGroups = GetObjectLength(l, t);
	meshIds.reserve(numMeshGroups);
	for(auto i = decltype(numMeshGroups) {0}; i < numMeshGroups; ++i) {
		PushInt(l, i + 1);
		GetTableValue(l, t);
		meshIds.push_back(CheckInt(l, -1));

		Pop(l, 1);
	}

	auto r = mdl.TranslateLODMeshes(lod, meshIds);
	PushBool(l, r);
	if(r == false)
		return;
	auto tTranslated = CreateTable(l);
	for(auto i = decltype(meshIds.size()) {0}; i < meshIds.size(); ++i) {
		auto id = meshIds.at(i);
		PushInt(l, i + 1);
		PushInt(l, (id == pragma::asset::MODEL_NO_MESH) ? -1 : static_cast<int32_t>(id));
		SetTableValue(l, tTranslated);
	}
}
void Lua::Model::GetJoints(lua::State *l, pragma::asset::Model &mdl)
{
	auto &joints = mdl.GetJoints();
	auto t = CreateTable(l);
	uint32_t idx = 1;
	for(auto &joint : joints) {
		PushInt(l, idx++);
		Lua::Push<pragma::physics::JointInfo *>(l, &joint);
		SetTableValue(l, t);
	}
}
void Lua::Model::GetVertexAnimations(lua::State *l, pragma::asset::Model &mdl)
{
	auto &vertexAnims = mdl.GetVertexAnimations();
	auto t = CreateTable(l);
	auto animIdx = 1u;
	for(auto &anim : vertexAnims) {
		PushInt(l, animIdx++);
		Lua::Push<std::shared_ptr<pragma::animation::VertexAnimation>>(l, anim);
		SetTableValue(l, t);
	}
}
void Lua::Model::GetVertexAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	auto *anim = mdl.GetVertexAnimation(name);
	if(anim == nullptr)
		return;
	Lua::Push<std::shared_ptr<pragma::animation::VertexAnimation>>(l, *anim);
}
void Lua::Model::AddVertexAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	auto anim = mdl.AddVertexAnimation(name);
	Lua::Push<std::shared_ptr<pragma::animation::VertexAnimation>>(l, anim);
}
void Lua::Model::RemoveVertexAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &name) { mdl.RemoveVertexAnimation(name); }
static void get_body_group_meshes(lua::State *l, pragma::asset::Model &mdl, const std::vector<uint32_t> &bodyGroupIds, uint32_t lod)
{
	std::vector<std::shared_ptr<pragma::geometry::ModelMesh>> meshes {};
	mdl.GetBodyGroupMeshes(bodyGroupIds, lod, meshes);
	auto t = Lua::CreateTable(l);
	auto idx = 1;
	for(auto &mesh : meshes) {
		Lua::PushInt(l, idx++);
		Lua::Push(l, mesh);
		Lua::SetTableValue(l, t);
	}
}
void Lua::Model::GetBodyGroupMeshes(lua::State *l, pragma::asset::Model &mdl, luabind::object oBodygroups, uint32_t lod)
{
	auto tBodygroups = 2;
	CheckTable(l, tBodygroups);
	std::vector<uint32_t> bodyGroupIds;

	auto numBgs = GetObjectLength(l, tBodygroups);
	bodyGroupIds.reserve(numBgs);
	for(auto i = decltype(numBgs) {0u}; i < numBgs; ++i) {
		PushInt(l, i + 1);
		GetTableValue(l, tBodygroups);

		auto groupId = CheckInt(l, -1);
		bodyGroupIds.push_back(groupId);

		Pop(l, 1);
	}

	get_body_group_meshes(l, mdl, bodyGroupIds, lod);
}
void Lua::Model::GetBodyGroupMeshes(lua::State *l, pragma::asset::Model &mdl, uint32_t lod)
{
	std::vector<uint32_t> bodygroups {};
	bodygroups.resize(mdl.GetBodyGroupCount(), 0u);

	get_body_group_meshes(l, mdl, bodygroups, lod);
}
void Lua::Model::GetBodyGroupMeshes(lua::State *l, pragma::asset::Model &mdl) { GetBodyGroupMeshes(l, mdl, 0u); }
static void push_flex_controller(lua::State *l, const pragma::animation::FlexController &fc)
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
void Lua::Model::GetFlexController(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	auto *fc = mdl.GetFlexController(name);
	if(fc == nullptr)
		return;
	push_flex_controller(l, *fc);
}
void Lua::Model::GetFlexController(lua::State *l, pragma::asset::Model &mdl, uint32_t id)
{
	auto *fc = mdl.GetFlexController(id);
	if(fc == nullptr)
		return;
	push_flex_controller(l, *fc);
}
void Lua::Model::GetFlexControllers(lua::State *l, pragma::asset::Model &mdl)
{
	auto t = CreateTable(l);
	auto &flexControllers = mdl.GetFlexControllers();
	auto fcId = 1u;
	for(auto &fc : flexControllers) {
		PushInt(l, fcId++);
		push_flex_controller(l, fc);

		SetTableValue(l, t);
	}
}
void Lua::Model::GetFlexControllerId(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	auto id = 0u;
	if(mdl.GetFlexControllerId(name, id) == false)
		PushInt(l, -1);
	else
		PushInt(l, id);
}
void Lua::Model::GetFlexes(lua::State *l, pragma::asset::Model &mdl)
{
	auto &flexes = mdl.GetFlexes();
	auto t = CreateTable(l);
	auto flexIdx = 1u;
	for(auto &flex : flexes) {
		PushInt(l, flexIdx++);
		Lua::Push<pragma::animation::Flex *>(l, &flex);
		SetTableValue(l, t);
	}
}
void Lua::Model::GetFlexId(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	auto flexId = 0u;
	if(mdl.GetFlexId(name, flexId) == false)
		PushInt(l, -1);
	else
		PushInt(l, flexId);
}
void Lua::Model::GetFlexFormula(lua::State *l, pragma::asset::Model &mdl, uint32_t flexId)
{
	std::string formula;
	if(mdl.GetFlexFormula(flexId, formula) == false)
		return;
	PushString(l, formula);
}
void Lua::Model::GetFlexFormula(lua::State *l, pragma::asset::Model &mdl, const std::string &flexName)
{
	std::string formula;
	if(mdl.GetFlexFormula(flexName, formula) == false)
		return;
	PushString(l, formula);
}
void Lua::Model::GetIKControllers(lua::State *l, pragma::asset::Model &mdl)
{
	auto &ikControllers = mdl.GetIKControllers();
	auto t = CreateTable(l);
	auto ikControllerIdx = 1u;
	for(auto &ikController : ikControllers) {
		PushInt(l, ikControllerIdx++);
		Lua::Push<std::shared_ptr<pragma::physics::IKController>>(l, ikController);
		SetTableValue(l, t);
	}
}
void Lua::Model::GetIKController(lua::State *l, pragma::asset::Model &mdl, uint32_t id)
{
	auto *ikController = mdl.GetIKController(id);
	if(ikController == nullptr)
		return;
	Lua::Push<std::shared_ptr<pragma::physics::IKController>>(l, ikController->shared_from_this());
}
void Lua::Model::LookupIKController(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	auto ikControllerId = std::numeric_limits<uint32_t>::max();
	if(mdl.LookupIKController(name, ikControllerId) == false) {
		PushInt(l, -1);
		return;
	}
	PushInt(l, ikControllerId);
}
void Lua::Model::AddIKController(lua::State *l, pragma::asset::Model &mdl, const std::string &name, uint32_t chainLength, const std::string &type, uint32_t method)
{
	auto *ikController = mdl.AddIKController(name, chainLength, type, static_cast<pragma::physics::ik::Method>(method));
	if(ikController == nullptr)
		return;
	Lua::Push<std::shared_ptr<pragma::physics::IKController>>(l, ikController->shared_from_this());
}
void Lua::Model::AddIKController(lua::State *l, pragma::asset::Model &mdl, const std::string &name, uint32_t chainLength, const std::string &type) { AddIKController(l, mdl, name, chainLength, type, pragma::math::to_integral(pragma::physics::ik::Method::Default)); }
void Lua::Model::RemoveIKController(lua::State *l, pragma::asset::Model &mdl, uint32_t id) { mdl.RemoveIKController(id); }
void Lua::Model::RemoveIKController(lua::State *l, pragma::asset::Model &mdl, const std::string &name) { mdl.RemoveIKController(name); }
void Lua::Model::AddIncludeModel(lua::State *l, pragma::asset::Model &mdl, const std::string &modelName) { mdl.GetMetaInfo().includes.push_back(modelName); }
void Lua::Model::GetIncludeModels(lua::State *l, pragma::asset::Model &mdl)
{
	auto t = CreateTable(l);
	auto &includes = mdl.GetMetaInfo().includes;
	for(auto i = decltype(includes.size()) {0u}; i < includes.size(); ++i) {
		auto &inc = includes.at(i);
		PushInt(l, i + 1);
		PushString(l, inc);
		SetTableValue(l, t);
	}
}
void Lua::Model::GetPhonemeMap(lua::State *l, pragma::asset::Model &mdl)
{
	auto &phonemeMap = mdl.GetPhonemeMap();
	if(phonemeMap.phonemes.empty())
		return;
	auto t = CreateTable(l);
	for(auto &pair : phonemeMap.phonemes) {
		PushString(l, pair.first);
		auto tFlexControllers = CreateTable(l);
		for(auto &fc : pair.second.flexControllers) {
			PushString(l, fc.first);
			PushNumber(l, fc.second);
			SetTableValue(l, tFlexControllers);
		}
		SetTableValue(l, t);
	}
}
void Lua::Model::AssignDistinctMaterial(lua::State *l, pragma::asset::Model &mdl, pragma::asset::ModelMeshGroup &group, pragma::geometry::ModelMesh &mesh, pragma::geometry::ModelSubMesh &subMesh)
{
	auto idx = mdl.AssignDistinctMaterial(group, mesh, subMesh);
	if(idx.has_value() == false)
		return;
	PushInt(l, *idx);
}
void Lua::Model::SetPhonemeMap(lua::State *l, pragma::asset::Model &mdl, luabind::object o)
{
	auto t = 2;
	CheckTable(l, t);
	auto &phonemeMap = mdl.GetPhonemeMap();
	phonemeMap = {};

	PushNil(l);
	while(GetNextPair(l, t) != 0) {
		std::string phoneme = CheckString(l, -2);
		auto tPhoneme = GetStackTop(l);
		CheckTable(l, tPhoneme);

		auto it = phonemeMap.phonemes.find(phoneme);
		if(it == phonemeMap.phonemes.end())
			phonemeMap.phonemes.insert(std::make_pair(phoneme, PhonemeInfo {}));

		PushNil(l);
		while(GetNextPair(l, tPhoneme) != 0) {
			std::string flexController = CheckString(l, -2);
			auto value = CheckNumber(l, -1);

			auto itInfo = it->second.flexControllers.find(flexController);
			if(itInfo == it->second.flexControllers.end())
				itInfo = it->second.flexControllers.insert(std::make_pair(flexController, value)).first;
			itInfo->second = value;

			Pop(l, 1);
		}

		Pop(l, 1);
	}
}
static void push_object_attachment(lua::State *l, const pragma::asset::ObjectAttachment &att)
{
	auto tAtt = Lua::CreateTable(l);

	Lua::PushString(l, "name");
	Lua::PushString(l, att.name);
	Lua::SetTableValue(l, tAtt);

	Lua::PushString(l, "attachment");
	Lua::PushString(l, att.attachment);
	Lua::SetTableValue(l, tAtt);

	Lua::PushString(l, "type");
	Lua::PushInt(l, pragma::math::to_integral(att.type));
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
void Lua::Model::GetObjectAttachments(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	auto &objAttachments = mdl.GetObjectAttachments();
	auto t = CreateTable(l);
	auto idx = 1u;
	for(auto &objAttachment : objAttachments) {
		PushInt(l, idx++);
		push_object_attachment(l, objAttachment);
		SetTableValue(l, t);
	}
}
void Lua::Model::AddObjectAttachment(lua::State *l, pragma::asset::Model &mdl, uint32_t type, const std::string &name, const std::string &attachment, luabind::object oKeyValues)
{
	//Lua::CheckModel(l,1);
	std::unordered_map<std::string, std::string> keyValues;
	auto numKeyValues = GetObjectLength(l, 5);

	PushNil(l);
	while(GetNextPair(l, 5) != 0) {
		auto *key = CheckString(l, -2);
		auto *val = CheckString(l, -1);
		keyValues.insert(std::make_pair(key, val));
		Pop(l, 1);
	}

	auto attId = mdl.AddObjectAttachment(static_cast<pragma::asset::ObjectAttachment::Type>(type), name, attachment, keyValues);
	PushInt(l, attId);
}
void Lua::Model::GetObjectAttachmentCount(lua::State *l, pragma::asset::Model &mdl)
{
	//Lua::CheckModel(l,1);
	PushInt(l, mdl.GetObjectAttachmentCount());
}
void Lua::Model::GetObjectAttachment(lua::State *l, pragma::asset::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	auto *att = mdl.GetObjectAttachment(idx);
	if(att == nullptr)
		return;
	push_object_attachment(l, *att);
}
void Lua::Model::LookupObjectAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	auto attId = 0u;
	auto rAttId = -1;
	if(mdl.LookupObjectAttachment(name, attId) == true)
		rAttId = attId;
	PushInt(l, rAttId);
}
void Lua::Model::RemoveObjectAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveObjectAttachment(name);
}
void Lua::Model::RemoveObjectAttachment(lua::State *l, pragma::asset::Model &mdl, uint32_t idx)
{
	//Lua::CheckModel(l,1);
	mdl.RemoveObjectAttachment(idx);
}
