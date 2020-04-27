/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __MODEL_H__
#define __MODEL_H__
#include "pragma/networkdefinitions.h"
#include <vector>

#include "pragma/model/side.h"
#include "pragma/model/animation/animation.h"
#include <mathutil/uvec.h>
#include <pragma/math/intersection.h>
#include <pragma/console/conout.h>
#include "material.h"
#include "pragma/physics/hitboxes.h"
#include "pragma/math/surfacematerial.h"
#include "pragma/model/modelupdateflags.hpp"
#include "pragma/model/model_flexes.hpp"
#include "pragma/physics/ik/ik_controller.hpp"
#include "pragma/phonememap.hpp"
#include <sharedutils/def_handle.h>
#include <memory>

#pragma warning(push)
#pragma warning(disable : 4251)
class ModelMesh;
class DLLNETWORK ModelMeshGroup
	: public std::enable_shared_from_this<ModelMeshGroup>
{
public:
	static std::shared_ptr<ModelMeshGroup> Create(const std::string &name);
	static std::shared_ptr<ModelMeshGroup> Create(const ModelMeshGroup &other);
	bool operator==(const ModelMeshGroup &other) const;
	bool operator!=(const ModelMeshGroup &other) const;
	uint32_t GetMeshCount() const;
	const std::string &GetName() const;
	std::vector<std::shared_ptr<ModelMesh>> &GetMeshes();
	void AddMesh(const std::shared_ptr<ModelMesh> &mesh);
private:
	ModelMeshGroup(const std::string &name);
	std::string m_name;
	std::vector<std::shared_ptr<ModelMesh>> m_meshes;
};

struct DLLNETWORK LODInfo
{
	unsigned int lod;
	std::unordered_map<unsigned int,unsigned int> meshReplacements;
};

struct DLLNETWORK BlendController
{
	std::string name;
	int min;
	int max;
	bool loop;
};

struct DLLNETWORK Attachment
{
	Attachment()=default;
	std::string name;
	unsigned int bone = 0u;
	Vector3 offset = {};
	EulerAngles angles = {};
};

struct DLLNETWORK ObjectAttachment
{
	enum class Type : uint32_t
	{
		Model = 0u,
		ParticleSystem
	};
	Type type = Type::Model;
	std::string attachment;
	std::string name;
	std::unordered_map<std::string,std::string> keyValues;
};

struct DLLNETWORK TextureGroup
{
	std::vector<unsigned int> textures;
};

struct DLLNETWORK BodyGroup
{
	BodyGroup(const std::string &n)
		: name(n)
	{}
	BodyGroup()
	{}
	std::string name;
	std::vector<unsigned int> meshGroups;
};

struct DLLNETWORK Eyeball
{
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
	std::array<int32_t,3> upperFlexDesc = {};
	std::array<int32_t,3> lowerFlexDesc = {};
	std::array<float,3> upperTarget; // Angle in radians of raised, neutral, and lowered lid positions
	std::array<float,3> lowerTarget;

	int32_t upperLidFlexDesc = -1; // Index of flex desc that actual lid flexes look to
	int32_t lowerLidFlexDesc = -1;
};

#define MODEL_NO_MESH (unsigned int)(-1)

class CollisionMesh;
class Game;
class VertexAnimation;
class NetworkState;
namespace pragma::physics{class ScaledTransform;};
class DLLNETWORK Model
	: public std::enable_shared_from_this<Model>
{
	friend Con::c_cout& operator<<(Con::c_cout&,const Model&);
public:
	Model();
	virtual void Remove();
	enum class Flags : uint32_t
	{
		None = 0u,
		Static = 1u,
		Inanimate = Static<<1u, // This flag has no effect and is merely a hint that the model doesn't have any animations or skeleton (except for the root bone)
		Unused1 = Inanimate<<1u,
		Unused2 = Unused1<<1u,
		Unused3 = Unused2<<1u,
		Unused4 = Unused3<<1u,
		Unused5 = Unused4<<1u,
		DontPrecacheTextureGroups = Unused5<<1u
	};

	struct DLLNETWORK MetaInfo
	{
		MetaInfo();
		std::vector<std::string> includes;
		std::vector<std::string> texturePaths;
		std::vector<std::string> textures;
		Flags flags = Flags::None;
	};
	enum class CopyFlags : uint32_t
	{
		None = 0u,
		ShallowCopy = 0u,

		CopyMeshesBit = 1u,
		CopyAnimationsBit = CopyMeshesBit<<1u,
		CopyVertexAnimationsBit = CopyAnimationsBit<<1u,
		CopyCollisionMeshes = CopyVertexAnimationsBit<<1u,

		DeepCopy = CopyMeshesBit | CopyAnimationsBit | CopyVertexAnimationsBit | CopyCollisionMeshes
	};
public:
	template<class TModel>
		static std::shared_ptr<Model> Create(NetworkState *nw,uint32_t numBones,const std::string &name="");
	template<class TModel>
		static std::shared_ptr<Model> Create(const Model &other);
	enum class DLLNETWORK MergeFlags : uint32_t
	{
		None = 0,
		Animations = 1,
		Attachments = Animations<<1,
		BlendControllers = Attachments<<1,
		Hitboxes = BlendControllers<<1,
		Joints = Hitboxes<<1,
		CollisionMeshes = Joints<<1,
		Meshes = CollisionMeshes<<1,

		All = (Meshes<<1) -1
	};

	bool operator==(const Model &other) const;
	bool operator!=(const Model &other) const;
	virtual ~Model();
	bool Save(Game *game,const std::string &name,const std::string &rootPath="") const;
	std::shared_ptr<Model> Copy(Game *game,CopyFlags copyFlags=CopyFlags::ShallowCopy) const;
	bool FindMaterial(const std::string &texture,std::string &matPath) const;
	MetaInfo &GetMetaInfo() const;
	Vector3 GetOrigin() const;
	const Vector3 &GetEyeOffset() const;
	void SetEyeOffset(const Vector3 &offset);
	virtual void AddMesh(const std::string &meshGroup,const std::shared_ptr<ModelMesh> &mesh);
	bool IsValid() const;
	CallbackHandle CallOnMaterialsLoaded(const std::function<void(void)> &f);
	void GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups,std::vector<std::shared_ptr<ModelMesh>> &outMeshes) const;
	void GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups,uint32_t lod,std::vector<std::shared_ptr<ModelMesh>> &outMeshes) const;
	// Returns all existing meshes in this model (Including LOD meshes)
	std::vector<std::shared_ptr<ModelMesh>> *GetMeshes(const std::string &meshGroup);
	std::shared_ptr<ModelMeshGroup> GetMeshGroup(const std::string &meshGroup);
	std::shared_ptr<ModelMeshGroup> AddMeshGroup(const std::string &meshGroup,uint32_t &meshGroupId);
	std::shared_ptr<ModelMeshGroup> AddMeshGroup(const std::string &meshGroup);
	bool GetMeshGroupId(const std::string &meshGroup,uint32_t &groupId) const;
	std::shared_ptr<ModelMeshGroup> GetMeshGroup(uint32_t groupId);
	void AddMeshGroup(std::shared_ptr<ModelMeshGroup> &meshGroup);
	std::vector<std::shared_ptr<ModelMeshGroup>> &GetMeshGroups();
	const std::vector<std::shared_ptr<ModelMeshGroup>> &GetMeshGroups() const;
	std::vector<std::shared_ptr<CollisionMesh>> &GetCollisionMeshes();
	const std::vector<std::shared_ptr<CollisionMesh>> &GetCollisionMeshes() const;
	void AddCollisionMesh(const std::shared_ptr<CollisionMesh> &mesh);
	// Gets the level of detail info for the given lod, or the next best
	LODInfo *GetLODInfo(uint32_t lod);
	LODInfo *AddLODInfo(uint32_t lod,std::unordered_map<uint32_t,uint32_t> &replaceIds);
	uint32_t GetLODCount() const;
	uint32_t GetLOD(uint32_t id) const;
	const std::vector<LODInfo> &GetLODs() const;
	bool TranslateLODMeshes(uint32_t lod,std::vector<uint32_t> &meshIds);
	// Returns true if the bodygroup exists and sets 'outMeshId' to the mesh Id. If the bodygroup mesh is none/blank, 'outMeshId' will be (unsigned int)(-1)
	bool GetMesh(uint32_t bodyGroupId,uint32_t groupId,uint32_t &outMeshId);
	void GetMeshes(const std::vector<uint32_t> &meshIds,std::vector<std::shared_ptr<ModelMesh>> &outMeshes);
	//void GetWeights(std::vector<VertexWeight*> **weights);
	static void ClearCache();
	const std::string &GetName() const;
	uint32_t AddAnimation(const std::string &name,const std::shared_ptr<Animation> &anim);
	int LookupAnimation(const std::string &name) const;
	int SelectWeightedAnimation(Activity activity,int32_t animIgnore=-1);
	int SelectFirstAnimation(Activity activity) const;
	unsigned char GetAnimationActivityWeight(uint32_t animation) const;
	Activity GetAnimationActivity(uint32_t animation) const;
	float GetAnimationDuration(uint32_t animation);
	std::shared_ptr<Animation> GetAnimation(uint32_t ID);
	void GetAnimations(Activity activity,std::vector<uint32_t> &animations);
	void GetAnimations(std::unordered_map<std::string,uint32_t> **anims);
	const std::vector<std::shared_ptr<Animation>> &GetAnimations() const;
	std::vector<std::shared_ptr<Animation>> &GetAnimations();
	bool GetAnimationName(uint32_t animId,std::string &name) const;
	std::string GetAnimationName(uint32_t animId) const;
	uint32_t GetAnimationCount() const;
	bool HasVertexWeights() const;
	std::optional<float> CalcFlexWeight(
		uint32_t flexId,
		const std::function<std::optional<float>(uint32_t)> &fFetchFlexControllerWeight,
		const std::function<std::optional<float>(uint32_t)> &fFetchFlexWeight
	) const;

	// Vertex animations
	const std::vector<std::shared_ptr<VertexAnimation>> &GetVertexAnimations() const;
	std::vector<std::shared_ptr<VertexAnimation>> &GetVertexAnimations();
	const std::shared_ptr<VertexAnimation> *GetVertexAnimation(uint32_t vaIdx) const;
	bool GetVertexAnimationId(const std::string &name,uint32_t &id) const;
	std::shared_ptr<VertexAnimation> *GetVertexAnimation(uint32_t vaIdx);
	const std::shared_ptr<VertexAnimation> *GetVertexAnimation(const std::string &name) const;
	std::shared_ptr<VertexAnimation> *GetVertexAnimation(const std::string &name);
	std::shared_ptr<VertexAnimation> AddVertexAnimation(const std::string &name);
	void RemoveVertexAnimation(const std::string &name);

	// Flex controllers
	const std::vector<FlexController> &GetFlexControllers() const;
	bool GetFlexControllerId(const std::string &name,uint32_t &id) const;
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
	bool GetFlexId(const std::string &name,uint32_t &id) const;
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
	bool GetFlexFormula(uint32_t id,std::string &formula) const;
	bool GetFlexFormula(const std::string &name,std::string &formula) const;

	// Inverse kinematics
	const std::vector<std::shared_ptr<IKController>> &GetIKControllers() const;
	std::vector<std::shared_ptr<IKController>> &GetIKControllers();
	const IKController *GetIKController(uint32_t id) const;
	IKController *GetIKController(uint32_t id);
	bool LookupIKController(const std::string &name,uint32_t &id) const;
	IKController *AddIKController(const std::string &name,uint32_t chainLength,const std::string &type,util::ik::Method method=util::ik::Method::Default);
	void RemoveIKController(uint32_t id);
	void RemoveIKController(const std::string &name);

	// Phonemes
	const PhonemeMap &GetPhonemeMap() const;
	PhonemeMap &GetPhonemeMap();

	const Skeleton &GetSkeleton() const;
	Skeleton &GetSkeleton();
	uint32_t GetBoneCount() const;
	bool GetLocalBonePosition(uint32_t animId,uint32_t frameId,uint32_t boneId,Vector3 &rPos,Quat &rRot,Vector3 *scale=nullptr);
	bool IsRootBone(uint32_t boneId) const;
	bool IntersectAABB(Vector3 &min,Vector3 &max);
	void CalculateRenderBounds();
	void CalculateCollisionBounds();
	virtual void Update(ModelUpdateFlags flags=ModelUpdateFlags::AllData);
	void GetCollisionBounds(Vector3 &min,Vector3 &max);
	void GetRenderBounds(Vector3 &min,Vector3 &max);
	void SetCollisionBounds(const Vector3 &min,const Vector3 &max);
	void SetRenderBounds(const Vector3 &min,const Vector3 &max);
	Mat4 *GetBindPoseBoneMatrix(uint32_t boneID);
	void SetBindPoseBoneMatrix(uint32_t boneID,Mat4 mat);
	void GenerateBindPoseMatrices();
	float GetMass() const;
	void SetMass(float mass);
	void AddBlendController(const std::string &name,int32_t min,int32_t max,bool loop);
	BlendController *GetBlendController(uint32_t id);
	const BlendController *GetBlendController(uint32_t id) const;
	BlendController *GetBlendController(const std::string &name);
	const BlendController *GetBlendController(const std::string &name) const;
	int LookupBlendController(const std::string &name);
	const std::vector<BlendController> &GetBlendControllers() const;
	std::vector<BlendController> &GetBlendControllers();
	const std::vector<Attachment> &GetAttachments() const;
	std::vector<Attachment> &GetAttachments();
	void AddAttachment(const std::string &name,uint32_t boneID,Vector3 offset,EulerAngles angles);
	Attachment *GetAttachment(uint32_t attachmentID);
	Attachment *GetAttachment(const std::string &name);
	void RemoveAttachment(const std::string &name);
	void RemoveAttachment(uint32_t idx);
	int32_t LookupAttachment(const std::string &name);
	std::optional<pragma::physics::ScaledTransform> CalcReferenceAttachmentPose(int32_t attId) const;
	std::optional<pragma::physics::ScaledTransform> CalcReferenceBonePose(int32_t boneId) const;

	const std::vector<ObjectAttachment> &GetObjectAttachments() const;
	std::vector<ObjectAttachment> &GetObjectAttachments();
	uint32_t AddObjectAttachment(ObjectAttachment::Type type,const std::string &name,const std::string &attachment,const std::unordered_map<std::string,std::string> &keyValues);
	uint32_t GetObjectAttachmentCount() const;
	ObjectAttachment *GetObjectAttachment(uint32_t idx);
	bool LookupObjectAttachment(const std::string &name,uint32_t &attId) const;
	bool RemoveObjectAttachment(const std::string &name);
	bool RemoveObjectAttachment(uint32_t idx);

	int32_t LookupBone(const std::string &name) const;
	void Merge(const Model &other,MergeFlags flags=MergeFlags::All);

	// Hitboxes
	void AddHitbox(uint32_t boneId,HitGroup group,const Vector3 &min,const Vector3 &max);
	void AddHitbox(uint32_t boneId,const Hitbox &hitbox);
	uint32_t GetHitboxCount() const;
	const std::unordered_map<uint32_t,Hitbox> &GetHitboxes() const;
	std::unordered_map<uint32_t,Hitbox> &GetHitboxes();
	const Hitbox *GetHitbox(uint32_t boneId) const;
	HitGroup GetHitboxGroup(uint32_t boneId) const;
	bool GetHitboxBounds(uint32_t boneId,Vector3 &min,Vector3 &max) const;
	std::vector<uint32_t> GetHitboxBones(HitGroup group) const;
	void GetHitboxBones(HitGroup group,std::vector<uint32_t> &boneIds) const;
	std::vector<uint32_t> GetHitboxBones() const;
	void GetHitboxBones(std::vector<uint32_t> &boneIds) const;

	void UpdateShape(const std::vector<SurfaceMaterial> *materials=nullptr);
	const std::vector<uint32_t> &GetBaseMeshes() const;
	std::vector<uint32_t> &GetBaseMeshes();
	uint32_t GetMeshGroupCount() const;
	uint32_t GetMeshCount() const;
	uint32_t GetSubMeshCount() const;
	uint32_t GetCollisionMeshCount() const;
	uint32_t GetVertexCount() const;
	uint32_t GetTriangleCount() const;
	// Textures
	uint32_t AddTexture(const std::string &tex,Material *mat);
	bool SetTexture(uint32_t texIdx,const std::string &tex,Material *mat);
	uint32_t AddMaterial(uint32_t skin,Material *mat,std::optional<uint32_t> *optOutSkinTexIdx=nullptr);
	bool SetMaterial(uint32_t texIdx,Material *mat);
	void RemoveTexture(uint32_t idx);
	void ClearTextures();
	void LoadMaterials(const std::function<Material*(const std::string&,bool)> &loadMaterial,bool bReload=false);
	TextureGroup *CreateTextureGroup();
	std::vector<std::string> &GetTextures();
	std::vector<MaterialHandle> &GetMaterials();
	const std::vector<MaterialHandle> &GetMaterials() const;
	Material *GetMaterial(uint32_t texID);
	Material *GetMaterial(uint32_t texGroup,uint32_t texID);
	void PrecacheTexture(uint32_t texId,const std::function<Material*(const std::string&,bool)> &loadMaterial,bool bReload=false);
	virtual void PrecacheTexture(uint32_t texId,bool bReload=false);
	std::vector<TextureGroup> &GetTextureGroups();
	TextureGroup *GetTextureGroup(uint32_t i);
	const TextureGroup *GetTextureGroup(uint32_t i) const;
	virtual void PrecacheTextureGroup(uint32_t i);
	void PrecacheTextureGroups();
	std::vector<std::string> &GetTexturePaths();
	void AddTexturePath(const std::string &path);
	void RemoveTexturePath(uint32_t idx);
	void SetTexturePaths(const std::vector<std::string> &paths);
	std::optional<uint32_t> GetMaterialIndex(const ModelSubMesh &mesh,uint32_t skinId=0) const;
	void SetReference(std::shared_ptr<Frame> frame);
	const Frame &GetReference() const;
	Frame &GetReference();
	void Rotate(const Quat &rot);
	void Translate(const Vector3 &t);
	void Scale(const Vector3 &scale);

	// Merges meshes with same materials (Only within mesh groups)
	void Optimize();

	// BodyGroups
	BodyGroup *GetBodyGroup(uint32_t id);
	BodyGroup &AddBodyGroup(const std::string &name);
	Int32 GetBodyGroupId(const std::string &name);
	std::vector<BodyGroup> &GetBodyGroups();
	UInt32 GetBodyGroupCount() const;

	const std::vector<JointInfo> &GetJoints() const;
	std::vector<JointInfo> &GetJoints();
	JointInfo &AddJoint(uint8_t type,uint32_t src,uint32_t tgt);

	const std::vector<Eyeball> &GetEyeballs() const;
	std::vector<Eyeball> &GetEyeballs();
	uint32_t GetEyeballCount() const;
	const Eyeball *GetEyeball(uint32_t idx) const;
	Eyeball *GetEyeball(uint32_t idx);
	void AddEyeball(const Eyeball &eyeball);

	void SetMaxEyeDeflection(umath::Degree eyeDeflection);
	umath::Degree GetMaxEyeDeflection() const;

	util::WeakHandle<const Model> GetHandle() const;
	util::WeakHandle<Model> GetHandle();

	void ClipAgainstPlane(const Vector3 &n,double d,Model &mdlA,Model &mdlB,const std::vector<Mat4> *boneMatrices=nullptr);
protected:
	Model(NetworkState *nw,uint32_t numBones,const std::string &name="");
	Model(const Model &other);
	virtual void OnMaterialMissing(const std::string &matName);
	void AddLoadingMaterial(Material &mat,std::optional<uint32_t> index={});
	void PrecacheTextureGroup(const std::function<Material*(const std::string&,bool)> &loadMaterial,unsigned int i);
	void LoadMaterials(const std::vector<uint32_t> &textureGroups,const std::function<Material*(const std::string&,bool)> &loadMaterial,bool bReload=false);
	bool FindMaterial(const std::string &texture,std::string &matPath,const std::function<Material*(const std::string&,bool)> &loadMaterial) const;

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
	NetworkState *m_networkState = nullptr;
	mutable MetaInfo m_metaInfo = {};
	bool m_bValid = false;
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
	std::unordered_map<uint32_t,Hitbox> m_hitboxes;
	std::vector<Eyeball> m_eyeballs;
	//std::vector<std::vector<VertexWeight>*> m_weights;
	static std::unordered_map<std::string,std::shared_ptr<Model>> m_models;
	std::shared_ptr<Frame> m_reference = nullptr;
	std::string m_name;
	bool m_bAllMaterialsLoaded = false;
	std::vector<std::shared_ptr<Animation>> m_animations;
	std::vector<std::shared_ptr<VertexAnimation>> m_vertexAnimations;
	std::unordered_map<std::string,unsigned int> m_animationIDs;
	std::unique_ptr<Skeleton> m_skeleton = nullptr;

	std::vector<FlexController> m_flexControllers;
	std::vector<Flex> m_flexes;

	std::vector<std::shared_ptr<IKController>> m_ikControllers;

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
	std::vector<LODInfo> m_lods; // LODs have to be in order!
	std::vector<Attachment> m_attachments;
	std::vector<ObjectAttachment> m_objectAttachments;
	std::vector<MaterialHandle> m_materials;
	std::vector<TextureGroup> m_textureGroups;
	std::vector<CallbackHandle> m_matLoadCallbacks;
	std::vector<CallbackHandle> m_onAllMatsLoadedCallbacks;
	void OnMaterialLoaded();
};
REGISTER_BASIC_BITWISE_OPERATORS(Model::CopyFlags);
REGISTER_BASIC_BITWISE_OPERATORS(Model::MergeFlags);
REGISTER_BASIC_BITWISE_OPERATORS(Model::Flags);
#pragma warning(pop)

template<class TModel>
	std::shared_ptr<Model> Model::Create(NetworkState *nw,uint32_t numBones,const std::string &name)
{
	return std::shared_ptr<Model>{new TModel{nw,numBones,name}};
}
template<class TModel>
	std::shared_ptr<Model> Model::Create(const Model &other)
{
	return std::shared_ptr<Model>{new TModel{other}};
}

#endif // __MODEL_H__