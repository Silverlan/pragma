// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"


export module pragma.shared:scripting.lua.classes.model;

export import :model;

export namespace Lua {
	namespace ModelMeshGroup {
		DLLNETWORK void register_class(luabind::class_<pragma::ModelMeshGroup> &classDef);
		DLLNETWORK void Create(lua_State *l, const std::string &name);
		DLLNETWORK void GetName(lua_State *l, pragma::ModelMeshGroup &meshGroup);
		DLLNETWORK void GetMeshes(lua_State *l, pragma::ModelMeshGroup &meshGroup);
		DLLNETWORK void AddMesh(lua_State *l, pragma::ModelMeshGroup &meshGroup, ::ModelMesh &mesh);
	};
	namespace Joint {
		DLLNETWORK void GetType(lua_State *l, JointInfo &joint);
		DLLNETWORK void GetChildBoneId(lua_State *l, JointInfo &joint);
		DLLNETWORK void GetParentBoneId(lua_State *l, JointInfo &joint);
		DLLNETWORK void GetCollisionsEnabled(lua_State *l, JointInfo &joint);
		DLLNETWORK void GetKeyValues(lua_State *l, JointInfo &joint);

		DLLNETWORK void SetType(lua_State *l, JointInfo &joint, uint32_t type);
		DLLNETWORK void SetCollisionMeshId(lua_State *l, JointInfo &joint, uint32_t meshId);
		DLLNETWORK void SetParentCollisionMeshId(lua_State *l, JointInfo &joint, uint32_t meshId);
		DLLNETWORK void SetCollisionsEnabled(lua_State *l, JointInfo &joint, bool bEnabled);
		DLLNETWORK void SetKeyValues(lua_State *l, JointInfo &joint, luabind::object keyValues);
		DLLNETWORK void SetKeyValue(lua_State *l, JointInfo &joint, const std::string &key, const std::string &val);
		DLLNETWORK void RemoveKeyValue(lua_State *l, JointInfo &joint, const std::string &key);
	};
	namespace Bone {
		DLLNETWORK void register_class(lua_State *l, luabind::class_<pragma::animation::Skeleton> &classDef);
	};
	namespace Model {
		DLLNETWORK void register_class(lua_State *l, luabind::class_<pragma::Model> &classDef, luabind::class_<::ModelMesh> &classDefModelMesh, luabind::class_<pragma::ModelSubMesh> &classDefModelSubMesh);
		DLLNETWORK void GetCollisionMeshes(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void ClearCollisionMeshes(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetSkeleton(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetAttachmentCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetAttachments(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetAttachment(lua_State *l, pragma::Model &mdl, int32_t attId);
		DLLNETWORK void GetAttachment(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void LookupAttachment(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void LookupBone(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void LookupAnimation(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void AddAttachment(lua_State *l, pragma::Model &mdl, const std::string &name, const std::string &boneName, const Vector3 &offset, const EulerAngles &ang);
		DLLNETWORK void AddAttachment(lua_State *l, pragma::Model &mdl, const std::string &name, uint32_t boneId, const Vector3 &offset, const EulerAngles &ang);
		DLLNETWORK void AddAttachment(lua_State *l, pragma::Model &mdl, luabind::object data);
		DLLNETWORK void SetAttachmentData(lua_State *l, pragma::Model &mdl, const std::string &name, luabind::object data);
		DLLNETWORK void SetAttachmentData(lua_State *l, pragma::Model &mdl, uint32_t attId, luabind::object data);
		DLLNETWORK void RemoveAttachment(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void RemoveAttachment(lua_State *l, pragma::Model &mdl, uint32_t attId);

		DLLNETWORK void GetObjectAttachments(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void AddObjectAttachment(lua_State *l, pragma::Model &mdl, uint32_t type, const std::string &name, const std::string &attachment, luabind::object oKeyValues);
		DLLNETWORK void GetObjectAttachmentCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetObjectAttachment(lua_State *l, pragma::Model &mdl, uint32_t idx);
		DLLNETWORK void LookupObjectAttachment(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void RemoveObjectAttachment(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void RemoveObjectAttachment(lua_State *l, pragma::Model &mdl, uint32_t idx);

		DLLNETWORK void GetBlendControllerCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetBlendControllers(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetBlendController(lua_State *l, pragma::Model &mdl, int32_t blendControllerId);
		DLLNETWORK void GetBlendController(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void LookupBlendController(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void GetAnimationCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetAnimationNames(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetAnimations(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetAnimation(lua_State *l, pragma::Model &mdl, const char *name);
		DLLNETWORK void GetAnimation(lua_State *l, pragma::Model &mdl, unsigned int animID);
		DLLNETWORK void GetAnimationName(lua_State *l, pragma::Model &mdl, unsigned int animID);
		DLLNETWORK void PrecacheTextureGroup(lua_State *l, pragma::Model &mdl, unsigned int group);
		DLLNETWORK void PrecacheTextureGroups(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetReference(lua_State *l, pragma::Model &mdl);
		//DLLNETWORK void GetReferenceBoneMatrix(lua_State *l,::Model &mdl,uint32_t boneId);
		//DLLNETWORK void SetReferenceBoneMatrix(lua_State *l,::Model &mdl,uint32_t boneId,const Mat4 &mat);
		DLLNETWORK void GetLocalBonePosition(lua_State *l, pragma::Model &mdl, UInt32 animId, UInt32 frameId, UInt32 boneId);
		DLLNETWORK void LookupBodyGroup(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void GetBaseMeshGroupIds(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void SetBaseMeshGroupIds(lua_State *l, pragma::Model &mdl, luabind::object o);
		DLLNETWORK void AddBaseMeshGroupId(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetMeshGroupId(lua_State *l, pragma::Model &mdl, uint32_t bodyGroupId, uint32_t groupId);
		DLLNETWORK void GetMeshGroup(lua_State *l, pragma::Model &mdl, const std::string &meshGroupName);
		DLLNETWORK void GetMeshGroup(lua_State *l, pragma::Model &mdl, uint32_t groupId);
		DLLNETWORK void GetMeshes(lua_State *l, pragma::Model &mdl, const std::string &meshGroup);
		DLLNETWORK void GetMeshes(lua_State *l, pragma::Model &mdl, luabind::object meshIds);
		DLLNETWORK void GetMeshGroups(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void AddMeshGroup(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void AddMeshGroup(lua_State *l, pragma::Model &mdl, pragma::ModelMeshGroup &meshGroup);
		DLLNETWORK void UpdateCollisionBounds(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void UpdateRenderBounds(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void Update(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void Update(lua_State *l, pragma::Model &mdl, uint32_t flags);
		DLLNETWORK void GetName(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetMass(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void SetMass(lua_State *l, pragma::Model &mdl, float mass);
		DLLNETWORK void GetBoneCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetCollisionBounds(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetRenderBounds(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void SetCollisionBounds(lua_State *l, pragma::Model &mdl, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void SetRenderBounds(lua_State *l, pragma::Model &mdl, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void AddCollisionMesh(lua_State *l, pragma::Model &mdl, pragma::physics::CollisionMesh &colMesh);
		DLLNETWORK void AddMaterial(lua_State *l, pragma::Model &mdl, uint32_t textureGroup, msys::Material *mat);
		DLLNETWORK void SetMaterial(lua_State *l, pragma::Model &mdl, uint32_t matId, msys::Material *mat);
		DLLNETWORK void GetMaterials(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetMaterialCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetMeshGroupCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetMeshCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetSubMeshCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetCollisionMeshCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetBodyGroupId(lua_State *l, pragma::Model &mdl, const std::string &bodyGroupName);
		DLLNETWORK void GetBodyGroupCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetBodyGroups(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetBodyGroup(lua_State *l, pragma::Model &mdl, uint32_t bgId);
		DLLNETWORK void AddHitbox(lua_State *l, pragma::Model &mdl, uint32_t boneId, uint32_t hitGroup, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void GetHitboxCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetHitboxGroup(lua_State *l, pragma::Model &mdl, uint32_t boneId);
		DLLNETWORK void GetHitboxBounds(lua_State *l, pragma::Model &mdl, uint32_t boneId);
		DLLNETWORK void GetHitboxBones(lua_State *l, pragma::Model &mdl, uint32_t hitGroup);
		DLLNETWORK void GetHitboxBones(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void SetHitboxGroup(lua_State *l, pragma::Model &mdl, uint32_t boneId, uint32_t hitGroup);
		DLLNETWORK void SetHitboxBounds(lua_State *l, pragma::Model &mdl, uint32_t boneId, const Vector3 &min, const Vector3 &max);
		DLLNETWORK void RemoveHitbox(lua_State *l, pragma::Model &mdl, uint32_t boneId);
		DLLNETWORK void GetTextureGroupCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetTextureGroups(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetTextureGroup(lua_State *l, pragma::Model &mdl, uint32_t id);
		DLLNETWORK void Save(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void Copy(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void Copy(lua_State *l, pragma::Model &mdl, uint32_t copyFlags);
		DLLNETWORK void GetVertexCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetTriangleCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetTextures(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetTexturePaths(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void LoadMaterials(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void LoadMaterials(lua_State *l, pragma::Model &mdl, bool bReload);
		DLLNETWORK void AddTexturePath(lua_State *l, pragma::Model &mdl, const std::string &path);
		DLLNETWORK void RemoveTexturePath(lua_State *l, pragma::Model &mdl, uint32_t idx);
		DLLNETWORK void SetTexturePaths(lua_State *l, pragma::Model &mdl, luabind::object o);
		DLLNETWORK void RemoveTexture(lua_State *l, pragma::Model &mdl, uint32_t idx);
		DLLNETWORK void ClearTextures(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void Rotate(lua_State *l, pragma::Model &mdl, const Quat &rot);
		DLLNETWORK void Translate(lua_State *l, pragma::Model &mdl, const Vector3 &t);
		DLLNETWORK void Scale(lua_State *l, pragma::Model &mdl, const Vector3 &scale);
		DLLNETWORK void GetEyeOffset(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void SetEyeOffset(lua_State *l, pragma::Model &mdl, const Vector3 &offset);
		DLLNETWORK void AddAnimation(lua_State *l, pragma::Model &mdl, const std::string &name, pragma::animation::Animation &anim);
		DLLNETWORK void RemoveAnimation(lua_State *l, pragma::Model &mdl, uint32_t idx);
		DLLNETWORK void ClearAnimations(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void ClipAgainstPlane(lua_State *l, pragma::Model &mdl, const Vector3 &n, double d, pragma::Model &clippedMdlA, pragma::Model &clippedMdlB);
		DLLNETWORK void ClearMeshGroups(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void RemoveMeshGroup(lua_State *l, pragma::Model &mdl, uint32_t idx);
		DLLNETWORK void ClearBaseMeshGroupIds(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void AddTextureGroup(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void Merge(lua_State *l, pragma::Model &mdl, pragma::Model &mdlOther);
		DLLNETWORK void Merge(lua_State *l, pragma::Model &mdl, pragma::Model &mdlOther, uint32_t mergeFlags);
		DLLNETWORK void GetLODCount(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetLODData(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetLOD(lua_State *l, pragma::Model &mdl, uint32_t idx);
		DLLNETWORK void GetLODData(lua_State *l, pragma::Model &mdl, uint32_t lod);
		DLLNETWORK void TranslateLODMeshes(lua_State *l, pragma::Model &mdl, uint32_t lod);
		DLLNETWORK void TranslateLODMeshes(lua_State *l, pragma::Model &mdl, uint32_t lod, luabind::object o);
		DLLNETWORK void GetJoints(lua_State *l, pragma::Model &mdl);

		DLLNETWORK void GetVertexAnimations(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetVertexAnimation(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void AddVertexAnimation(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void RemoveVertexAnimation(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void GetBodyGroupMeshes(lua_State *l, pragma::Model &mdl, luabind::object oBodygroups, uint32_t lod);
		DLLNETWORK void GetBodyGroupMeshes(lua_State *l, pragma::Model &mdl, uint32_t lod);
		DLLNETWORK void GetBodyGroupMeshes(lua_State *l, pragma::Model &mdl);

		DLLNETWORK void GetFlexController(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void GetFlexController(lua_State *l, pragma::Model &mdl, uint32_t id);
		DLLNETWORK void GetFlexControllers(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetFlexControllerId(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void GetFlexes(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetFlexId(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void GetFlexFormula(lua_State *l, pragma::Model &mdl, uint32_t flexId);
		DLLNETWORK void GetFlexFormula(lua_State *l, pragma::Model &mdl, const std::string &flexName);

		DLLNETWORK void GetIKControllers(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void GetIKController(lua_State *l, pragma::Model &mdl, uint32_t id);
		DLLNETWORK void LookupIKController(lua_State *l, pragma::Model &mdl, const std::string &name);
		DLLNETWORK void AddIKController(lua_State *l, pragma::Model &mdl, const std::string &name, uint32_t chainLength, const std::string &type, uint32_t method);
		DLLNETWORK void AddIKController(lua_State *l, pragma::Model &mdl, const std::string &name, uint32_t chainLength, const std::string &type);
		DLLNETWORK void RemoveIKController(lua_State *l, pragma::Model &mdl, uint32_t id);
		DLLNETWORK void RemoveIKController(lua_State *l, pragma::Model &mdl, const std::string &name);

		DLLNETWORK void AddIncludeModel(lua_State *l, pragma::Model &mdl, const std::string &modelName);
		DLLNETWORK void GetIncludeModels(lua_State *l, pragma::Model &mdl);

		DLLNETWORK void GetPhonemeMap(lua_State *l, pragma::Model &mdl);
		DLLNETWORK void SetPhonemeMap(lua_State *l, pragma::Model &mdl, luabind::object o);

		DLLNETWORK void AssignDistinctMaterial(lua_State *l, pragma::Model &mdl, pragma::ModelMeshGroup &group, ::ModelMesh &mesh, pragma::ModelSubMesh &subMesh);
	};
};
