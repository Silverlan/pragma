/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __MODEL_H__
#define __MODEL_H__
#include "pragma/networkdefinitions.h"
#include <vector>

#include "pragma/model/side.h"
#include "pragma/model/animation/animation.hpp"
#include <mathutil/uvec.h>
#include <pragma/math/intersection.h>
#include <pragma/console/conout.h>
#include "material.h"
#include "pragma/physics/hitboxes.h"
#include "pragma/math/surfacematerial.h"
#include "pragma/model/modelupdateflags.hpp"
#include "pragma/model/model_flexes.hpp"
#include "pragma/physics/jointinfo.h"
#include "pragma/physics/ik/ik_controller.hpp"
#include "pragma/phonememap.hpp"
#include "pragma/game/game_coordinate_system.hpp"
#include <udm_types.hpp>
#include <sharedutils/def_handle.h>
#include <memory>

#pragma warning(push)
#pragma warning(disable : 4251)
class ModelMesh;
class DLLNETWORK ModelMeshGroup : public std::enable_shared_from_this<ModelMeshGroup> {
  public:
	static std::shared_ptr<ModelMeshGroup> Create(const std::string &name);
	static std::shared_ptr<ModelMeshGroup> Create(const ModelMeshGroup &other);
	bool operator==(const ModelMeshGroup &other) const;
	bool operator!=(const ModelMeshGroup &other) const;
	uint32_t GetMeshCount() const;
	const std::string &GetName() const;
	std::vector<std::shared_ptr<ModelMesh>> &GetMeshes();
	void AddMesh(const std::shared_ptr<ModelMesh> &mesh);

	bool IsEqual(const ModelMeshGroup &other) const;
  private:
	ModelMeshGroup(const std::string &name);
	std::string m_name;
	std::vector<std::shared_ptr<ModelMesh>> m_meshes;
};

struct DLLNETWORK LODInfo {
	unsigned int lod;
	std::unordered_map<unsigned int, unsigned int> meshReplacements;
	float distance = 0.f;

	bool operator==(const LODInfo &other) const { return lod == other.lod && meshReplacements == other.meshReplacements && umath::abs(distance - other.distance) < 0.001f; }
	bool operator!=(const LODInfo &other) const { return !operator==(other); }
};

struct DLLNETWORK BlendController {
	std::string name;
	int min;
	int max;
	bool loop;

	bool operator==(const BlendController &other) const { return name == other.name && min == other.min && max == other.max && loop == other.loop; }
	bool operator!=(const BlendController &other) const { return !operator==(other); }
};

struct DLLNETWORK Attachment {
	Attachment() = default;
	std::string name;
	unsigned int bone = 0u;
	Vector3 offset = {};
	EulerAngles angles = {};

	bool operator==(const Attachment &other) const { return name == other.name && bone == other.bone && uvec::distance_sqr(offset, other.offset) < 0.001f && uvec::distance_sqr(Vector3 {angles.p, angles.y, angles.r}, Vector3 {other.angles.p, other.angles.y, other.angles.r}) < 0.001f; }
	bool operator!=(const Attachment &other) const { return !operator==(other); }
};

struct DLLNETWORK ObjectAttachment {
	enum class Type : uint32_t { Model = 0u, ParticleSystem, Count };
	Type type = Type::Model;
	std::string attachment;
	std::string name;
	std::unordered_map<std::string, std::string> keyValues;

	bool operator==(const ObjectAttachment &other) const { return type == other.type && attachment == other.attachment && name == other.name && keyValues == other.keyValues; }
	bool operator!=(const ObjectAttachment &other) const { return !operator==(other); }
};

struct DLLNETWORK TextureGroup {
	std::vector<unsigned int> textures;

	bool operator==(const TextureGroup &other) const { return textures == other.textures; }
	bool operator!=(const TextureGroup &other) const { return !operator==(other); }
};

struct DLLNETWORK BodyGroup {
	BodyGroup(const std::string &n) : name(n) {}
	BodyGroup() {}
	std::string name;
	std::vector<unsigned int> meshGroups;

	bool operator==(const BodyGroup &other) const { return name == other.name && meshGroups == other.meshGroups; }
	bool operator!=(const BodyGroup &other) const { return !operator==(other); }
};

#pragma pack(push, 1)
struct DLLNETWORK Eyeball {
	std::string name = "";
	int32_t boneIndex = -1;
	Vector3 origin = {};
	float zOffset = 0.f;
	float radius = 0.f;
	Vector3 up = {};
	Vector3 forward = {};
	int32_t irisMaterialIndex = -1;
	float maxDilationFactor = 1.f;
	float irisUvRadius = 0.2f;

	float irisScale = 0.f;
	struct LidFlexDesc {
		int32_t lidFlexIndex = -1;
		int32_t raiserFlexIndex = -1;
		int32_t neutralFlexIndex = -1;
		int32_t lowererFlexIndex = -1;

		umath::Radian raiserValue = 0.f;
		umath::Radian neutralValue = 0.f;
		umath::Radian lowererValue = 0.f;

		bool operator==(const LidFlexDesc &other) const;
		bool operator!=(const LidFlexDesc &other) const { return !operator==(other); }
	};
	LidFlexDesc upperLid {};
	LidFlexDesc lowerLid {};

	bool operator==(const Eyeball &other) const;
	bool operator!=(const Eyeball &other) const { return !operator==(other); }
};
#pragma pack(pop)

#define MODEL_NO_MESH (unsigned int)(-1)

class CollisionMesh;
class Game;
class VertexAnimation;
class FlexAnimation;
class NetworkState;
namespace pragma::animation {
	using BoneId = uint16_t;
	using FlexControllerId = uint32_t;
	struct MetaRig;
	enum class MetaRigBoneType : uint8_t;
};
namespace pragma::model {
	template<typename T>
	static void validate_value(const T &v)
	{
		if constexpr(std::is_arithmetic_v<T>) {
			if(isnan(v))
				throw std::runtime_error {"NaN value"};
			if(isinf(v))
				throw std::runtime_error {"inv value"};
		}
		else if constexpr(std::is_same_v<T, EulerAngles>) {
			for(int i = 0; i < 3; ++i)
				validate_value(v[i]);
		}
		else {
			for(size_t i = 0; i < T::length(); ++i)
				validate_value(v[i]);
		}
	}
	Vector3 get_mirror_transform_vector(pragma::Axis axis);
};
enum class JointType : uint8_t;
namespace umath {
	class ScaledTransform;
};
namespace udm {
	using Version = uint32_t;
};
class DLLNETWORK Model : public std::enable_shared_from_this<Model> {
	friend Con::c_cout &operator<<(Con::c_cout &, const Model &);
  public:
	Model();
	virtual void Remove();
	enum class Flags : uint32_t {
		None = 0u,
		Static = 1u,
		Inanimate = Static << 1u, // This flag has no effect and is merely a hint that the model doesn't have any animations or skeleton (except for the root bone)
		Unused1 = Inanimate << 1u,
		Unused2 = Unused1 << 1u,
		Unused3 = Unused2 << 1u,
		Unused4 = Unused3 << 1u,
		Unused5 = Unused4 << 1u,
		DontPrecacheTextureGroups = Unused5 << 1u,
		WorldGeometry = DontPrecacheTextureGroups << 1u,
		GeneratedHitboxes = WorldGeometry << 1u,
		GeneratedLODs = GeneratedHitboxes << 1u,
		GeneratedMetaRig = GeneratedLODs << 1u,
		GeneratedMetaBlendShapes = GeneratedMetaRig << 1u,

		Count = 13,
	};

	enum class StateFlags : uint32_t { None = 0u, Valid = 1u, AllMaterialsLoaded = Valid << 1u, MaterialsLoadInitiated = AllMaterialsLoaded << 1u };

	struct DLLNETWORK MetaInfo {
		MetaInfo();
		std::vector<std::string> includes;
		std::vector<std::string> texturePaths;
		std::vector<std::string> textures;
		Flags flags = Flags::None;

		bool operator==(const MetaInfo &other) const;
		bool operator!=(const MetaInfo &other) const { return !operator==(other); }
	};
	enum class CopyFlags : uint32_t {
		None = 0u,
		ShallowCopy = 0u,

		CopyMeshesBit = 1u,
		CopyAnimationsBit = CopyMeshesBit << 1u,
		CopyVertexAnimationsBit = CopyAnimationsBit << 1u,
		CopyCollisionMeshesBit = CopyVertexAnimationsBit << 1u,
		CopyFlexAnimationsBit = CopyCollisionMeshesBit << 1u,
		CopyUniqueIdsBit = CopyFlexAnimationsBit << 1u,
		CopyVertexData = CopyUniqueIdsBit << 1u,

		DeepCopy = CopyMeshesBit | CopyVertexData | CopyAnimationsBit | CopyVertexAnimationsBit | CopyCollisionMeshesBit | CopyFlexAnimationsBit
	};
  public:
	static constexpr auto PMDL_IDENTIFIER = "PMDL";
	static constexpr udm::Version PMDL_VERSION = 1;
	static bool Load(Model &mdl, NetworkState &nw, const udm::AssetData &data, std::string &outErr);
	template<class TModel>
	static std::shared_ptr<Model> Create(NetworkState *nw, uint32_t numBones, const std::string &name = "");
	template<class TModel>
	static std::shared_ptr<Model> Create(const Model &other);
	template<class TModel>
	static std::shared_ptr<Model> Load(NetworkState &nw, const udm::AssetData &data, std::string &outErr)
	{
		auto mdl = Create<TModel>(&nw, 0u);
		if(Load(*mdl, nw, data, outErr) == false)
			return nullptr;
		return mdl;
	}
	static void GenerateStandardMetaRigReferenceBonePoses(const pragma::animation::MetaRig &metaRig, const pragma::animation::Skeleton &skeleton, const Frame &refFrame, std::vector<umath::ScaledTransform> &outPoses);
	enum class DLLNETWORK MergeFlags : uint32_t {
		None = 0,
		Animations = 1,
		Attachments = Animations << 1,
		BlendControllers = Attachments << 1,
		Hitboxes = BlendControllers << 1,
		Joints = Hitboxes << 1,
		CollisionMeshes = Joints << 1,
		Meshes = CollisionMeshes << 1,

		All = (Meshes << 1) - 1
	};

	bool IsEqual(const Model &other) const;
	bool operator==(const Model &other) const;
	bool operator!=(const Model &other) const;
	Model &operator=(const Model &other);
	virtual ~Model();
	bool Save(Game &game, udm::AssetDataArg outData, std::string &outErr);
	bool Save(Game &game, const std::string &fileName, std::string &outErr);
	bool Save(Game &game, std::string &outErr);
	bool SaveLegacy(Game *game, const std::string &name, const std::string &rootPath = "") const;
	std::shared_ptr<Model> Copy(Game *game, CopyFlags copyFlags = CopyFlags::ShallowCopy) const;
	bool FindMaterial(const std::string &texture, std::string &matPath) const;
	bool GenerateLowLevelLODs(Game &game);
	bool GenerateCollisionMeshes(bool convex, float mass, const std::optional<std::string> &surfaceMaterial = {});
	MetaInfo &GetMetaInfo() const;
	Vector3 GetOrigin() const;
	const Vector3 &GetEyeOffset() const;
	void SetEyeOffset(const Vector3 &offset);
	virtual void AddMesh(const std::string &meshGroup, const std::shared_ptr<ModelMesh> &mesh);
	bool IsValid() const;
	CallbackHandle CallOnMaterialsLoaded(const std::function<void(void)> &f);
	void GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, std::vector<std::shared_ptr<ModelMesh>> &outMeshes) const;
	void GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, uint32_t lod, std::vector<std::shared_ptr<ModelMesh>> &outMeshes) const;
	void GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, std::vector<std::shared_ptr<ModelSubMesh>> &outMeshes) const;
	void GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, uint32_t lod, std::vector<std::shared_ptr<ModelSubMesh>> &outMeshes) const;
	// Returns all existing meshes in this model (Including LOD meshes)
	std::vector<std::shared_ptr<ModelMesh>> *GetMeshes(const std::string &meshGroup);
	std::shared_ptr<ModelMeshGroup> GetMeshGroup(const std::string &meshGroup);
	std::shared_ptr<ModelMeshGroup> AddMeshGroup(const std::string &meshGroup, uint32_t &meshGroupId);
	std::shared_ptr<ModelMeshGroup> AddMeshGroup(const std::string &meshGroup);
	bool GetMeshGroupId(const std::string &meshGroup, uint32_t &groupId) const;
	std::shared_ptr<ModelMeshGroup> GetMeshGroup(uint32_t groupId);
	void AddMeshGroup(std::shared_ptr<ModelMeshGroup> &meshGroup);
	std::vector<std::shared_ptr<ModelMeshGroup>> &GetMeshGroups();
	const std::vector<std::shared_ptr<ModelMeshGroup>> &GetMeshGroups() const;
	std::vector<std::shared_ptr<CollisionMesh>> &GetCollisionMeshes();
	const std::vector<std::shared_ptr<CollisionMesh>> &GetCollisionMeshes() const;
	void AddCollisionMesh(const std::shared_ptr<CollisionMesh> &mesh);
	// Gets the level of detail info for the given lod, or the next best
	LODInfo *GetLODInfo(uint32_t lod);
	LODInfo *AddLODInfo(uint32_t lod, float distance, std::unordered_map<uint32_t, uint32_t> &replaceIds);
	uint32_t GetLODCount() const;
	uint32_t GetLOD(uint32_t id) const;
	const std::vector<LODInfo> &GetLODs() const;
	bool TranslateLODMeshes(uint32_t lod, std::vector<uint32_t> &meshIds);
	// Returns true if the bodygroup exists and sets 'outMeshId' to the mesh Id. If the bodygroup mesh is none/blank, 'outMeshId' will be (unsigned int)(-1)
	bool GetMesh(uint32_t bodyGroupId, uint32_t groupId, uint32_t &outMeshId);
	ModelMesh *GetMesh(uint32_t meshGroupIdx, uint32_t meshIdx);
	const ModelMesh *GetMesh(uint32_t meshGroupIdx, uint32_t meshIdx) const { return const_cast<Model *>(this)->GetMesh(meshGroupIdx, meshIdx); }
	ModelSubMesh *GetSubMesh(uint32_t meshGroupIdx, uint32_t meshIdx, uint32_t subMeshIdx);
	const ModelSubMesh *GetSubMesh(uint32_t meshGroupIdx, uint32_t meshIdx, uint32_t subMeshIdx) const { return const_cast<Model *>(this)->GetSubMesh(meshGroupIdx, meshIdx, subMeshIdx); }
	void GetMeshes(const std::vector<uint32_t> &meshIds, std::vector<std::shared_ptr<ModelMesh>> &outMeshes);
	void GetSubMeshes(const std::vector<uint32_t> &meshIds, std::vector<std::shared_ptr<ModelSubMesh>> &outMeshes);
	//void GetWeights(std::vector<VertexWeight*> **weights);
	static void ClearCache();
	const std::string &GetName() const;
	void SetName(const std::string &name) { m_name = name; }
	uint32_t AddAnimation(const std::string &name, const std::shared_ptr<pragma::animation::Animation> &anim);
	int LookupAnimation(const std::string &name) const;
	int SelectWeightedAnimation(Activity activity, int32_t animIgnore = -1);
	int SelectFirstAnimation(Activity activity) const;
	unsigned char GetAnimationActivityWeight(uint32_t animation) const;
	Activity GetAnimationActivity(uint32_t animation) const;
	float GetAnimationDuration(uint32_t animation);
	std::shared_ptr<pragma::animation::Animation> GetAnimation(uint32_t ID) const;
	void GetAnimations(Activity activity, std::vector<uint32_t> &animations);
	void GetAnimations(std::unordered_map<std::string, uint32_t> **anims);
	const std::vector<std::shared_ptr<pragma::animation::Animation>> &GetAnimations() const;
	std::vector<std::shared_ptr<pragma::animation::Animation>> &GetAnimations();
	std::unordered_map<std::string, unsigned int> &GetAnimationNames() { return m_animationIDs; }
	const std::unordered_map<std::string, unsigned int> &GetAnimationNames() const { return const_cast<Model *>(this)->GetAnimationNames(); }
	bool GetAnimationName(uint32_t animId, std::string &name) const;
	std::string GetAnimationName(uint32_t animId) const;
	uint32_t GetAnimationCount() const;
	bool HasVertexWeights() const;
	std::optional<float> CalcFlexWeight(uint32_t flexId, const std::function<std::optional<float>(uint32_t)> &fFetchFlexControllerWeight, const std::function<std::optional<float>(uint32_t)> &fFetchFlexWeight) const;
	virtual std::shared_ptr<ModelMesh> CreateMesh() const;
	virtual std::shared_ptr<ModelSubMesh> CreateSubMesh() const;
	float CalcBoneLength(pragma::animation::BoneId boneId) const;
	void RemoveBone(pragma::animation::BoneId boneId);

	// Vertex animations
	const std::vector<std::shared_ptr<VertexAnimation>> &GetVertexAnimations() const;
	std::vector<std::shared_ptr<VertexAnimation>> &GetVertexAnimations();
	const std::shared_ptr<VertexAnimation> *GetVertexAnimation(uint32_t vaIdx) const;
	bool GetVertexAnimationId(const std::string &name, uint32_t &id) const;
	std::shared_ptr<VertexAnimation> *GetVertexAnimation(uint32_t vaIdx);
	const std::shared_ptr<VertexAnimation> *GetVertexAnimation(const std::string &name) const;
	std::shared_ptr<VertexAnimation> *GetVertexAnimation(const std::string &name);
	std::shared_ptr<VertexAnimation> AddVertexAnimation(const std::string &name);
	void RemoveVertexAnimation(const std::string &name);

	// Flex controllers
	const std::vector<FlexController> &GetFlexControllers() const;
	bool GetFlexControllerId(const std::string &name, uint32_t &id) const;
	std::vector<FlexController> &GetFlexControllers();
	const FlexController *GetFlexController(uint32_t id) const;
	FlexController *GetFlexController(uint32_t id);
	const FlexController *GetFlexController(const std::string &name) const;
	FlexController *GetFlexController(const std::string &name);
	FlexController &AddFlexController(const std::string &name);
	void RemoveFlexController(uint32_t id);
	void RemoveFlexController(const std::string &name);
	uint32_t GetFlexControllerCount() const;
	const std::string *GetFlexControllerName(uint32_t id) const;

	// Flexes
	const std::vector<Flex> &GetFlexes() const;
	bool GetFlexId(const std::string &name, uint32_t &id) const;
	std::vector<Flex> &GetFlexes();
	const Flex *GetFlex(uint32_t id) const;
	Flex *GetFlex(uint32_t id);
	const Flex *GetFlex(const std::string &name) const;
	Flex *GetFlex(const std::string &name);
	Flex &AddFlex(const std::string &name);
	void RemoveFlex(uint32_t id);
	void RemoveFlex(const std::string &name);
	uint32_t GetFlexCount() const;
	const std::string *GetFlexName(uint32_t id) const;
	bool GetFlexFormula(uint32_t id, std::string &formula) const;
	bool GetFlexFormula(const std::string &name, std::string &formula) const;

	// Inverse kinematics
	const std::vector<std::shared_ptr<IKController>> &GetIKControllers() const;
	std::vector<std::shared_ptr<IKController>> &GetIKControllers();
	const IKController *GetIKController(uint32_t id) const;
	IKController *GetIKController(uint32_t id);
	bool LookupIKController(const std::string &name, uint32_t &id) const;
	IKController *AddIKController(const std::string &name, uint32_t chainLength, const std::string &type, util::ik::Method method = util::ik::Method::Default);
	void RemoveIKController(uint32_t id);
	void RemoveIKController(const std::string &name);

	// Phonemes
	const PhonemeMap &GetPhonemeMap() const;
	PhonemeMap &GetPhonemeMap();

	bool FindSubMeshIndex(const ModelMeshGroup *optMeshGroup, const ModelMesh *optMesh, const ModelSubMesh *optSubMesh, uint32_t &outGroupIdx, uint32_t &outMeshIdx, uint32_t &outSubMeshIdx) const;

	const pragma::animation::Skeleton &GetSkeleton() const;
	pragma::animation::Skeleton &GetSkeleton();

	void TransformBone(pragma::animation::BoneId boneId, const umath::Transform &t, umath::CoordinateSpace space = umath::CoordinateSpace::World);

	const std::shared_ptr<pragma::animation::MetaRig> &GetMetaRig() const;
	bool GenerateMetaRig();
	bool GenerateMetaBlendShapes();
	void ClearMetaRig();
	std::optional<pragma::animation::MetaRigBoneType> GetMetaRigBoneParentId(pragma::animation::MetaRigBoneType type) const;
	std::optional<umath::ScaledTransform> GetMetaRigReferencePose(pragma::animation::MetaRigBoneType type) const;
	bool GenerateStandardMetaRigReferenceBonePoses(std::vector<umath::ScaledTransform> &outPoses) const;
	void ApplyPostImportProcessing();

	uint32_t GetBoneCount() const;
	bool GetLocalBonePosition(uint32_t animId, uint32_t frameId, uint32_t boneId, Vector3 &rPos, Quat &rRot, Vector3 *scale = nullptr);

	bool GetReferenceBonePose(pragma::animation::BoneId boneId, umath::Transform &outPose, umath::CoordinateSpace space = umath::CoordinateSpace::Object) const;
	bool GetReferenceBonePose(pragma::animation::BoneId boneId, umath::ScaledTransform &outPose, umath::CoordinateSpace space = umath::CoordinateSpace::Object) const;
	bool GetReferenceBonePos(pragma::animation::BoneId boneId, Vector3 &outPos, umath::CoordinateSpace space = umath::CoordinateSpace::Object) const;
	bool GetReferenceBoneRot(pragma::animation::BoneId boneId, Quat &outRot, umath::CoordinateSpace space = umath::CoordinateSpace::Object) const;
	bool GetReferenceBoneScale(pragma::animation::BoneId boneId, Vector3 &outScale, umath::CoordinateSpace space = umath::CoordinateSpace::Object) const;
	bool GetReferenceBonePose(pragma::animation::BoneId boneId, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale = nullptr, umath::CoordinateSpace space = umath::CoordinateSpace::Object) const;

	bool IsRootBone(uint32_t boneId) const;
	bool IntersectAABB(Vector3 &min, Vector3 &max);
	void CalculateRenderBounds();
	void CalculateCollisionBounds();
	virtual void Update(ModelUpdateFlags flags = ModelUpdateFlags::AllData);
	void GetCollisionBounds(Vector3 &min, Vector3 &max) const;
	void GetRenderBounds(Vector3 &min, Vector3 &max) const;
	void SetCollisionBounds(const Vector3 &min, const Vector3 &max);
	void SetRenderBounds(const Vector3 &min, const Vector3 &max);
	Mat4 *GetBindPoseBoneMatrix(uint32_t boneID);
	void SetBindPoseBoneMatrix(uint32_t boneID, Mat4 mat);
	void GenerateBindPoseMatrices();
	float GetMass() const;
	void SetMass(float mass);
	void AddBlendController(const std::string &name, int32_t min, int32_t max, bool loop);
	BlendController *GetBlendController(uint32_t id);
	const BlendController *GetBlendController(uint32_t id) const;
	BlendController *GetBlendController(const std::string &name);
	const BlendController *GetBlendController(const std::string &name) const;
	int LookupBlendController(const std::string &name);
	const std::vector<BlendController> &GetBlendControllers() const;
	std::vector<BlendController> &GetBlendControllers();
	const std::vector<Attachment> &GetAttachments() const;
	std::vector<Attachment> &GetAttachments();
	void AddAttachment(const std::string &name, uint32_t boneID, Vector3 offset, EulerAngles angles);
	Attachment *GetAttachment(uint32_t attachmentID);
	Attachment *GetAttachment(const std::string &name);
	void RemoveAttachment(const std::string &name);
	void RemoveAttachment(uint32_t idx);
	int32_t LookupAttachment(const std::string &name);
	std::optional<umath::ScaledTransform> CalcReferenceAttachmentPose(int32_t attId) const;
	std::optional<umath::ScaledTransform> CalcReferenceBonePose(int32_t boneId) const;

	const std::vector<ObjectAttachment> &GetObjectAttachments() const;
	std::vector<ObjectAttachment> &GetObjectAttachments();
	uint32_t AddObjectAttachment(ObjectAttachment::Type type, const std::string &name, const std::string &attachment, const std::unordered_map<std::string, std::string> &keyValues);
	uint32_t GetObjectAttachmentCount() const;
	ObjectAttachment *GetObjectAttachment(uint32_t idx);
	bool LookupObjectAttachment(const std::string &name, uint32_t &attId) const;
	bool RemoveObjectAttachment(const std::string &name);
	bool RemoveObjectAttachment(uint32_t idx);

	int32_t LookupBone(const std::string &name) const;
	void Merge(const Model &other, MergeFlags flags = MergeFlags::All);

	std::optional<uint32_t> AssignDistinctMaterial(const ModelMeshGroup &group, const ModelMesh &mesh, ModelSubMesh &subMesh);

	// Hitboxes
	void AddHitbox(uint32_t boneId, HitGroup group, const Vector3 &min, const Vector3 &max);
	void AddHitbox(uint32_t boneId, const Hitbox &hitbox);
	uint32_t GetHitboxCount() const;
	const std::unordered_map<uint32_t, Hitbox> &GetHitboxes() const;
	std::unordered_map<uint32_t, Hitbox> &GetHitboxes();
	const Hitbox *GetHitbox(uint32_t boneId) const;
	HitGroup GetHitboxGroup(uint32_t boneId) const;
	bool GetHitboxBounds(uint32_t boneId, Vector3 &min, Vector3 &max) const;
	std::vector<uint32_t> GetHitboxBones(HitGroup group) const;
	void GetHitboxBones(HitGroup group, std::vector<uint32_t> &boneIds) const;
	std::vector<uint32_t> GetHitboxBones() const;
	void GetHitboxBones(std::vector<uint32_t> &boneIds) const;
	std::unordered_map<pragma::animation::BoneId, Hitbox> CalcHitboxes() const;
	bool GenerateHitboxes();

	void UpdateShape(const std::vector<SurfaceMaterial> *materials = nullptr);
	const std::vector<uint32_t> &GetBaseMeshes() const;
	std::vector<uint32_t> &GetBaseMeshes();
	uint32_t GetMeshGroupCount() const;
	uint32_t GetMeshCount() const;
	uint32_t GetSubMeshCount() const;
	uint32_t GetCollisionMeshCount() const;
	uint32_t GetVertexCount() const;
	uint32_t GetTriangleCount() const;
	// Textures
	uint32_t AddTexture(const std::string &tex, Material *mat);
	bool SetTexture(uint32_t texIdx, const std::string &tex, Material *mat);
	uint32_t AddMaterial(uint32_t skin, Material *mat, const std::optional<std::string> &matName = {}, std::optional<uint32_t> *optOutSkinTexIdx = nullptr);
	bool SetMaterial(uint32_t texIdx, Material *mat);
	void RemoveTexture(uint32_t idx);
	void ClearTextures();
	void LoadMaterials(bool bReload = false);
	void PrecacheMaterials();
	TextureGroup *CreateTextureGroup();
	std::vector<std::string> &GetTextures();
	std::vector<msys::MaterialHandle> &GetMaterials();
	const std::vector<msys::MaterialHandle> &GetMaterials() const;
	Material *GetMaterial(uint32_t texID);
	Material *GetMaterial(uint32_t texGroup, uint32_t texID);
	virtual void PrecacheTexture(uint32_t texId, bool bReload = false);
	std::vector<TextureGroup> &GetTextureGroups();
	TextureGroup *GetTextureGroup(uint32_t i);
	const TextureGroup *GetTextureGroup(uint32_t i) const;
	virtual void PrecacheTextureGroup(uint32_t i);
	void PrecacheTextureGroups();
	std::vector<std::string> &GetTexturePaths();
	void AddTexturePath(const std::string &path);
	void RemoveTexturePath(uint32_t idx);
	void SetTexturePaths(const std::vector<std::string> &paths);
	std::optional<uint32_t> GetMaterialIndex(const ModelSubMesh &mesh, uint32_t skinId = 0) const;
	void SetReference(std::shared_ptr<Frame> frame);
	const Frame &GetReference() const;
	Frame &GetReference();
	bool SetReferencePoses(const std::vector<umath::ScaledTransform> &poses, bool posesInParentSpace = false);
	void Rotate(const Quat &rot);
	void Translate(const Vector3 &t);
	void Scale(const Vector3 &scale);
	void Mirror(pragma::Axis axis);

	// Merges meshes with same materials (Only within mesh groups)
	void Optimize();
	void Validate();

	// BodyGroups
	BodyGroup *GetBodyGroup(uint32_t id);
	BodyGroup &AddBodyGroup(const std::string &name);
	Int32 GetBodyGroupId(const std::string &name);
	std::vector<BodyGroup> &GetBodyGroups();
	UInt32 GetBodyGroupCount() const;

	const std::vector<JointInfo> &GetJoints() const;
	std::vector<JointInfo> &GetJoints();
	JointInfo &AddJoint(JointType type, pragma::animation::BoneId child, pragma::animation::BoneId parent);

	const std::vector<Eyeball> &GetEyeballs() const;
	std::vector<Eyeball> &GetEyeballs();
	uint32_t GetEyeballCount() const;
	const Eyeball *GetEyeball(uint32_t idx) const;
	Eyeball *GetEyeball(uint32_t idx);
	void AddEyeball(const Eyeball &eyeball);

	void SetMaxEyeDeflection(umath::Degree eyeDeflection);
	umath::Degree GetMaxEyeDeflection() const;

	udm::PropertyWrapper GetExtensionData() const;

	util::WeakHandle<const Model> GetHandle() const;
	util::WeakHandle<Model> GetHandle();

	void RemoveUnusedMaterialReferences();
	void ClipAgainstPlane(const Vector3 &n, double d, Model &mdlA, Model &mdlB, const std::vector<Mat4> *boneMatrices = nullptr);

	std::vector<std::shared_ptr<FlexAnimation>> &GetFlexAnimations() { return m_flexAnimations; }
	const std::vector<std::shared_ptr<FlexAnimation>> &GetFlexAnimations() const { return const_cast<Model *>(this)->GetFlexAnimations(); }
	std::vector<std::string> &GetFlexAnimationNames() { return m_flexAnimationNames; }
	const std::vector<std::string> &GetFlexAnimationNames() const { return const_cast<Model *>(this)->GetFlexAnimationNames(); }
	std::optional<uint32_t> LookupFlexAnimation(const std::string &name) const;
	uint32_t AddFlexAnimation(const std::string &name, FlexAnimation &anim);
	FlexAnimation *GetFlexAnimation(uint32_t idx);
	const FlexAnimation *GetFlexAnimation(uint32_t idx) const { return const_cast<Model *>(this)->GetFlexAnimation(idx); }
	const std::string *GetFlexAnimationName(uint32_t idx) const;

	std::optional<umath::ScaledTransform> GetReferenceBonePose(pragma::animation::BoneId boneId) const;
	std::optional<pragma::SignedAxis> FindBoneTwistAxis(pragma::animation::BoneId boneId) const;
	std::optional<pragma::SignedAxis> FindBoneAxisForDirection(pragma::animation::BoneId boneId, const Vector3 &dir) const;
	static Quat GetTwistAxisRotationOffset(pragma::SignedAxis axis);
  protected:
	Model(NetworkState *nw, uint32_t numBones, const std::string &name = "");
	Model(const Model &other);
	bool LoadFromAssetData(Game &game, const udm::AssetData &data, std::string &outErr);
	virtual void OnMaterialMissing(const std::string &matName);
	void AddLoadingMaterial(Material &mat, std::optional<uint32_t> index = {});
	void LoadMaterials(const std::vector<uint32_t> &textureGroups, bool precache, bool bReload);
	void LoadMaterials(bool precache, bool bReload);
	bool FindMaterial(const std::string &texture, std::string &matPath, bool importIfNotFound) const;

	virtual std::shared_ptr<VertexAnimation> CreateVertexAnimation(const std::string &name) const;
	std::vector<std::shared_ptr<VertexAnimation>>::const_iterator FindVertexAnimation(const std::string &name) const;
	std::vector<std::shared_ptr<VertexAnimation>>::iterator FindVertexAnimation(const std::string &name);

	std::vector<FlexController>::const_iterator FindFlexController(const std::string &name) const;
	std::vector<FlexController>::iterator FindFlexController(const std::string &name);

	std::vector<std::shared_ptr<IKController>>::const_iterator FindIKController(const std::string &name) const;
	std::vector<std::shared_ptr<IKController>>::iterator FindIKController(const std::string &name);

	std::vector<Flex>::const_iterator FindFlex(const std::string &name) const;
	std::vector<Flex>::iterator FindFlex(const std::string &name);
  private:
	void Construct();
	udm::PProperty m_extensions = nullptr;
	NetworkState *m_networkState = nullptr;
	mutable MetaInfo m_metaInfo = {};
	StateFlags m_stateFlags = StateFlags::None;
	float m_mass = 0.f;
	uint32_t m_meshCount = 0u;
	uint32_t m_subMeshCount = 0u;
	uint32_t m_vertexCount = 0u;
	uint32_t m_triangleCount = 0u;
	umath::Degree m_maxEyeDeflection = 30.f;
	PhonemeMap m_phonemeMap = {};
	std::vector<BlendController> m_blendControllers;
	std::vector<std::shared_ptr<ModelMeshGroup>> m_meshGroups;
	std::vector<BodyGroup> m_bodyGroups;
	std::unordered_map<uint32_t, Hitbox> m_hitboxes;
	std::vector<Eyeball> m_eyeballs;
	//std::vector<std::vector<VertexWeight>*> m_weights;
	static std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
	std::shared_ptr<Frame> m_reference = nullptr;
	std::string m_name;
	std::vector<std::shared_ptr<pragma::animation::Animation>> m_animations;
	std::vector<std::shared_ptr<VertexAnimation>> m_vertexAnimations;
	std::unordered_map<std::string, unsigned int> m_animationIDs;
	std::shared_ptr<pragma::animation::Skeleton> m_skeleton = nullptr;
	std::shared_ptr<pragma::animation::MetaRig> m_metaRig = nullptr;

	std::vector<FlexController> m_flexControllers;
	std::vector<Flex> m_flexes;

	std::vector<std::shared_ptr<IKController>> m_ikControllers;

	std::vector<std::shared_ptr<FlexAnimation>> m_flexAnimations;
	std::vector<std::string> m_flexAnimationNames;

	// Bind pose matrices are currently unused; Bind pose is extracted from reference pose instead!
	std::vector<Mat4> m_bindPose;
	Vector3 m_eyeOffset = {};
	Vector3 m_collisionMin = {};
	Vector3 m_collisionMax = {};
	Vector3 m_renderMin = {};
	Vector3 m_renderMax = {};
	std::vector<std::shared_ptr<CollisionMesh>> m_collisionMeshes;
	std::vector<JointInfo> m_joints;
	std::vector<unsigned int> m_baseMeshes; // Meshes in LOD 0
	std::vector<LODInfo> m_lods;            // LODs have to be in order!
	std::vector<Attachment> m_attachments;
	std::vector<ObjectAttachment> m_objectAttachments;
	std::vector<msys::MaterialHandle> m_materials;
	std::vector<TextureGroup> m_textureGroups;
	std::vector<CallbackHandle> m_matLoadCallbacks;
	std::vector<CallbackHandle> m_onAllMatsLoadedCallbacks;
	void OnMaterialLoaded();
};
REGISTER_BASIC_BITWISE_OPERATORS(Model::CopyFlags);
REGISTER_BASIC_BITWISE_OPERATORS(Model::MergeFlags);
REGISTER_BASIC_BITWISE_OPERATORS(Model::Flags);
REGISTER_BASIC_BITWISE_OPERATORS(Model::StateFlags);
#pragma warning(pop)

template<class TModel>
std::shared_ptr<Model> Model::Create(NetworkState *nw, uint32_t numBones, const std::string &name)
{
	return std::shared_ptr<Model> {new TModel {nw, numBones, name}};
}
template<class TModel>
std::shared_ptr<Model> Model::Create(const Model &other)
{
	return std::shared_ptr<Model> {new TModel {other}};
}

#endif // __MODEL_H__
