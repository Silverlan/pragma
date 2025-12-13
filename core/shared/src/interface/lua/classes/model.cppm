// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.model;

export import :model;

export namespace Lua {
	namespace ModelMeshGroup {
		DLLNETWORK void register_class(luabind::class_<pragma::asset::ModelMeshGroup> &classDef);
		DLLNETWORK void Create(lua::State *l, const std::string &name);
		DLLNETWORK void GetName(lua::State *l, pragma::asset::ModelMeshGroup &meshGroup);
		DLLNETWORK void GetMeshes(lua::State *l, pragma::asset::ModelMeshGroup &meshGroup);
		DLLNETWORK void AddMesh(lua::State *l, pragma::asset::ModelMeshGroup &meshGroup, pragma::geometry::ModelMesh &mesh);
	};
	namespace Joint {
		DLLNETWORK void GetType(lua::State *l, pragma::physics::JointInfo &joint);
		DLLNETWORK void GetChildBoneId(lua::State *l, pragma::physics::JointInfo &joint);
		DLLNETWORK void GetParentBoneId(lua::State *l, pragma::physics::JointInfo &joint);
		DLLNETWORK void GetCollisionsEnabled(lua::State *l, pragma::physics::JointInfo &joint);
		DLLNETWORK void GetKeyValues(lua::State *l, pragma::physics::JointInfo &joint);

		DLLNETWORK void SetType(lua::State *l, pragma::physics::JointInfo &joint, uint32_t type);
		DLLNETWORK void SetCollisionMeshId(lua::State *l, pragma::physics::JointInfo &joint, uint32_t meshId);
		DLLNETWORK void SetParentCollisionMeshId(lua::State *l, pragma::physics::JointInfo &joint, uint32_t meshId);
		DLLNETWORK void SetCollisionsEnabled(lua::State *l, pragma::physics::JointInfo &joint, bool bEnabled);
		DLLNETWORK void SetKeyValues(lua::State *l, pragma::physics::JointInfo &joint, luabind::object keyValues);
		DLLNETWORK void SetKeyValue(lua::State *l, pragma::physics::JointInfo &joint, const std::string &key, const std::string &val);
		DLLNETWORK void RemoveKeyValue(lua::State *l, pragma::physics::JointInfo &joint, const std::string &key);
	};
	namespace Bone {
		DLLNETWORK void register_class(lua::State *l, luabind::class_<pragma::animation::Skeleton> &classDef);
	};
	namespace Model {
		DLLNETWORK void register_class(lua::State *l, luabind::class_<pragma::asset::Model> &classDef, luabind::class_<pragma::geometry::ModelMesh> &classDefModelMesh, luabind::class_<pragma::geometry::ModelSubMesh> &classDefModelSubMesh);
		DLLNETWORK void GetCollisionMeshes(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void ClearCollisionMeshes(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetSkeleton(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetAttachmentCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetAttachments(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetAttachment(lua::State *l, pragma::asset::Model &mdl, int32_t attId);
		DLLNETWORK void GetAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void LookupAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void LookupBone(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void LookupAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void AddAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name, const std::string &boneName, const Vector3 &offset, const EulerAngles &ang);
		DLLNETWORK void AddAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name, uint32_t boneId, const Vector3 &offset, const EulerAngles &ang);
		DLLNETWORK void AddAttachment(lua::State *l, pragma::asset::Model &mdl, luabind::object data);
		DLLNETWORK void SetAttachmentData(lua::State *l, pragma::asset::Model &mdl, const std::string &name, luabind::object data);
		DLLNETWORK void SetAttachmentData(lua::State *l, pragma::asset::Model &mdl, uint32_t attId, luabind::object data);
		DLLNETWORK void RemoveAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void RemoveAttachment(lua::State *l, pragma::asset::Model &mdl, uint32_t attId);

		DLLNETWORK void GetObjectAttachments(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void AddObjectAttachment(lua::State *l, pragma::asset::Model &mdl, uint32_t type, const std::string &name, const std::string &attachment, luabind::object oKeyValues);
		DLLNETWORK void GetObjectAttachmentCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetObjectAttachment(lua::State *l, pragma::asset::Model &mdl, uint32_t idx);
		DLLNETWORK void LookupObjectAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void RemoveObjectAttachment(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void RemoveObjectAttachment(lua::State *l, pragma::asset::Model &mdl, uint32_t idx);

		DLLNETWORK void GetBlendControllerCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetBlendControllers(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetBlendController(lua::State *l, pragma::asset::Model &mdl, int32_t blendControllerId);
		DLLNETWORK void GetBlendController(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void LookupBlendController(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void GetAnimationCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetAnimationNames(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetAnimations(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetAnimation(lua::State *l, pragma::asset::Model &mdl, const char *name);
		DLLNETWORK void GetAnimation(lua::State *l, pragma::asset::Model &mdl, unsigned int animID);
		DLLNETWORK void GetAnimationName(lua::State *l, pragma::asset::Model &mdl, unsigned int animID);
		DLLNETWORK void PrecacheTextureGroup(lua::State *l, pragma::asset::Model &mdl, unsigned int group);
		DLLNETWORK void PrecacheTextureGroups(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetReference(lua::State *l, pragma::asset::Model &mdl);
		//DLLNETWORK void GetReferenceBoneMatrix(lua::State *l,::Model &mdl,uint32_t boneId);
		//DLLNETWORK void SetReferenceBoneMatrix(lua::State *l,::Model &mdl,uint32_t boneId,const Mat4 &mat);
		DLLNETWORK void GetLocalBonePosition(lua::State *l, pragma::asset::Model &mdl, UInt32 animId, UInt32 frameId, UInt32 boneId);
		DLLNETWORK void LookupBodyGroup(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void GetBaseMeshGroupIds(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void SetBaseMeshGroupIds(lua::State *l, pragma::asset::Model &mdl, luabind::object o);
		DLLNETWORK void AddBaseMeshGroupId(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetMeshGroupId(lua::State *l, pragma::asset::Model &mdl, uint32_t bodyGroupId, uint32_t groupId);
		DLLNETWORK void GetMeshGroup(lua::State *l, pragma::asset::Model &mdl, const std::string &meshGroupName);
		DLLNETWORK void GetMeshGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t groupId);
		DLLNETWORK void GetMeshes(lua::State *l, pragma::asset::Model &mdl, const std::string &meshGroup);
		DLLNETWORK void GetMeshes(lua::State *l, pragma::asset::Model &mdl, luabind::object meshIds);
		DLLNETWORK void AddMeshGroup(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void AddMeshGroup(lua::State *l, pragma::asset::Model &mdl, pragma::asset::ModelMeshGroup &meshGroup);
		DLLNETWORK void UpdateCollisionBounds(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void UpdateRenderBounds(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void Update(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void Update(lua::State *l, pragma::asset::Model &mdl, uint32_t flags);
		DLLNETWORK void GetName(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetMass(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void SetMass(lua::State *l, pragma::asset::Model &mdl, float mass);
		DLLNETWORK void GetBoneCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetCollisionBounds(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetRenderBounds(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void SetCollisionBounds(lua::State *l, pragma::asset::Model &mdl, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void SetRenderBounds(lua::State *l, pragma::asset::Model &mdl, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void AddCollisionMesh(lua::State *l, pragma::asset::Model &mdl, pragma::physics::CollisionMesh &colMesh);
		DLLNETWORK void AddMaterial(lua::State *l, pragma::asset::Model &mdl, uint32_t textureGroup, pragma::material::Material *mat);
		DLLNETWORK void SetMaterial(lua::State *l, pragma::asset::Model &mdl, uint32_t matId, pragma::material::Material *mat);
		DLLNETWORK void GetMaterials(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetMaterialCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetMeshGroupCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetMeshCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetSubMeshCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetCollisionMeshCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetBodyGroupId(lua::State *l, pragma::asset::Model &mdl, const std::string &bodyGroupName);
		DLLNETWORK void GetBodyGroupCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetBodyGroups(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetBodyGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t bgId);
		DLLNETWORK void AddHitbox(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId, uint32_t hitGroup, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void GetHitboxCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetHitboxGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId);
		DLLNETWORK void GetHitboxBounds(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId);
		DLLNETWORK void GetHitboxBones(lua::State *l, pragma::asset::Model &mdl, uint32_t hitGroup);
		DLLNETWORK void GetHitboxBones(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void SetHitboxGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId, uint32_t hitGroup);
		DLLNETWORK void SetHitboxBounds(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void RemoveHitbox(lua::State *l, pragma::asset::Model &mdl, uint32_t boneId);
		DLLNETWORK void GetTextureGroupCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetTextureGroups(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetTextureGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t id);
		DLLNETWORK void Save(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void Copy(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void Copy(lua::State *l, pragma::asset::Model &mdl, uint32_t copyFlags);
		DLLNETWORK void GetVertexCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetTriangleCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetTextures(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetTexturePaths(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void LoadMaterials(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void LoadMaterials(lua::State *l, pragma::asset::Model &mdl, bool bReload);
		DLLNETWORK void AddTexturePath(lua::State *l, pragma::asset::Model &mdl, const std::string &path);
		DLLNETWORK void RemoveTexturePath(lua::State *l, pragma::asset::Model &mdl, uint32_t idx);
		DLLNETWORK void SetTexturePaths(lua::State *l, pragma::asset::Model &mdl, luabind::object o);
		DLLNETWORK void RemoveTexture(lua::State *l, pragma::asset::Model &mdl, uint32_t idx);
		DLLNETWORK void ClearTextures(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void Rotate(lua::State *l, pragma::asset::Model &mdl, const Quat &rot);
		DLLNETWORK void Translate(lua::State *l, pragma::asset::Model &mdl, const Vector3 &t);
		DLLNETWORK void Scale(lua::State *l, pragma::asset::Model &mdl, const Vector3 &scale);
		DLLNETWORK void GetEyeOffset(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void SetEyeOffset(lua::State *l, pragma::asset::Model &mdl, const Vector3 &offset);
		DLLNETWORK void AddAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &name, pragma::animation::Animation &anim);
		DLLNETWORK void RemoveAnimation(lua::State *l, pragma::asset::Model &mdl, uint32_t idx);
		DLLNETWORK void ClearAnimations(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void ClipAgainstPlane(lua::State *l, pragma::asset::Model &mdl, const Vector3 &n, double d, pragma::asset::Model &clippedMdlA, pragma::asset::Model &clippedMdlB);
		DLLNETWORK void ClearMeshGroups(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void RemoveMeshGroup(lua::State *l, pragma::asset::Model &mdl, uint32_t idx);
		DLLNETWORK void ClearBaseMeshGroupIds(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void AddTextureGroup(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void Merge(lua::State *l, pragma::asset::Model &mdl, pragma::asset::Model &mdlOther);
		DLLNETWORK void Merge(lua::State *l, pragma::asset::Model &mdl, pragma::asset::Model &mdlOther, uint32_t mergeFlags);
		DLLNETWORK void GetLODCount(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetLODData(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetLOD(lua::State *l, pragma::asset::Model &mdl, uint32_t idx);
		DLLNETWORK void GetLODData(lua::State *l, pragma::asset::Model &mdl, uint32_t lod);
		DLLNETWORK void TranslateLODMeshes(lua::State *l, pragma::asset::Model &mdl, uint32_t lod);
		DLLNETWORK void TranslateLODMeshes(lua::State *l, pragma::asset::Model &mdl, uint32_t lod, luabind::object o);
		DLLNETWORK void GetJoints(lua::State *l, pragma::asset::Model &mdl);

		DLLNETWORK void GetVertexAnimations(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetVertexAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void AddVertexAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void RemoveVertexAnimation(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void GetBodyGroupMeshes(lua::State *l, pragma::asset::Model &mdl, luabind::object oBodygroups, uint32_t lod);
		DLLNETWORK void GetBodyGroupMeshes(lua::State *l, pragma::asset::Model &mdl, uint32_t lod);
		DLLNETWORK void GetBodyGroupMeshes(lua::State *l, pragma::asset::Model &mdl);

		DLLNETWORK void GetFlexController(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void GetFlexController(lua::State *l, pragma::asset::Model &mdl, uint32_t id);
		DLLNETWORK void GetFlexControllers(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetFlexControllerId(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void GetFlexes(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetFlexId(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void GetFlexFormula(lua::State *l, pragma::asset::Model &mdl, uint32_t flexId);
		DLLNETWORK void GetFlexFormula(lua::State *l, pragma::asset::Model &mdl, const std::string &flexName);

		DLLNETWORK void GetIKControllers(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void GetIKController(lua::State *l, pragma::asset::Model &mdl, uint32_t id);
		DLLNETWORK void LookupIKController(lua::State *l, pragma::asset::Model &mdl, const std::string &name);
		DLLNETWORK void AddIKController(lua::State *l, pragma::asset::Model &mdl, const std::string &name, uint32_t chainLength, const std::string &type, uint32_t method);
		DLLNETWORK void AddIKController(lua::State *l, pragma::asset::Model &mdl, const std::string &name, uint32_t chainLength, const std::string &type);
		DLLNETWORK void RemoveIKController(lua::State *l, pragma::asset::Model &mdl, uint32_t id);
		DLLNETWORK void RemoveIKController(lua::State *l, pragma::asset::Model &mdl, const std::string &name);

		DLLNETWORK void AddIncludeModel(lua::State *l, pragma::asset::Model &mdl, const std::string &modelName);
		DLLNETWORK void GetIncludeModels(lua::State *l, pragma::asset::Model &mdl);

		DLLNETWORK void GetPhonemeMap(lua::State *l, pragma::asset::Model &mdl);
		DLLNETWORK void SetPhonemeMap(lua::State *l, pragma::asset::Model &mdl, luabind::object o);

		DLLNETWORK void AssignDistinctMaterial(lua::State *l, pragma::asset::Model &mdl, pragma::asset::ModelMeshGroup &group, pragma::geometry::ModelMesh &mesh, pragma::geometry::ModelSubMesh &subMesh);
	};
};
