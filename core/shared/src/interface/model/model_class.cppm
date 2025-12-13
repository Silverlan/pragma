// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:model.model;

import :console.output;
export import :game.coordinate_system;
export import :model.animation.animation;
export import :model.animation.flex_animation;
export import :model.animation.meta_rig;
export import :model.animation.phoneme_map;
export import :model.animation.vertex_animation;
export import :model.flexes;
export import :model.model_mesh;
export import :physics.collision_mesh;
export import :physics.hitbox;
export import :physics.ik.controller;
export import :physics.joint_info;

export import pragma.materialsystem;

export {
#pragma warning(push)
#pragma warning(disable : 4251)
	namespace pragma {
		class NetworkState;
		namespace asset {
			class DLLNETWORK ModelMeshGroup : public std::enable_shared_from_this<ModelMeshGroup> {
			public:
				static constexpr std::uint32_t layout_version = 1;

				static std::shared_ptr<ModelMeshGroup> Create(const std::string &name);
				static std::shared_ptr<ModelMeshGroup> Create(const ModelMeshGroup &other);
				bool operator==(const ModelMeshGroup &other) const;
				bool operator!=(const ModelMeshGroup &other) const;
				uint32_t GetMeshCount() const;
				const std::string &GetName() const;
				std::vector<std::shared_ptr<geometry::ModelMesh>> &GetMeshes();
				void AddMesh(const std::shared_ptr<geometry::ModelMesh> &mesh);

				bool IsEqual(const ModelMeshGroup &other) const;
			private:
				ModelMeshGroup(const std::string &name);
				std::string m_name;
				std::vector<std::shared_ptr<geometry::ModelMesh>> m_meshes;
			};
		}
	}

	namespace pragma::asset {
		struct DLLNETWORK LODInfo {
			unsigned int lod;
			std::unordered_map<unsigned int, unsigned int> meshReplacements;
			float distance = 0.f;

			bool operator==(const LODInfo &other) const { return lod == other.lod && meshReplacements == other.meshReplacements && math::abs(distance - other.distance) < 0.001f; }
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
			static constexpr std::uint32_t layout_version = 1;

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
				static constexpr std::uint32_t layout_version = 1;

				int32_t lidFlexIndex = -1;
				int32_t raiserFlexIndex = -1;
				int32_t neutralFlexIndex = -1;
				int32_t lowererFlexIndex = -1;

				math::Radian raiserValue = 0.f;
				math::Radian neutralValue = 0.f;
				math::Radian lowererValue = 0.f;

				bool operator==(const LidFlexDesc &other) const;
				bool operator!=(const LidFlexDesc &other) const { return !operator==(other); }
			};
			LidFlexDesc upperLid {};
			LidFlexDesc lowerLid {};

			bool operator==(const Eyeball &other) const;
			bool operator!=(const Eyeball &other) const { return !operator==(other); }
		};
	#pragma pack(pop)

		constexpr auto MODEL_NO_MESH = std::numeric_limits<uint32_t>::max();

		template<typename T>
		void validate_value(const T &v)
		{
			if constexpr(std::is_arithmetic_v<T>) {
				if(std::isnan(v))
					throw std::runtime_error {"NaN value"};
				if(std::isinf(v))
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
		Vector3 get_mirror_transform_vector(Axis axis);

		class DLLNETWORK Model : public std::enable_shared_from_this<Model> {
		  public:
			static constexpr std::uint32_t layout_version = 1;

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
				static constexpr std::uint32_t layout_version = 1;

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
			static std::shared_ptr<Model> Create(NetworkState *nw, uint32_t numBones, const std::string &name = "")
			{
				return std::shared_ptr<Model> {new TModel {nw, numBones, name}};
			}
			template<class TModel>
			static std::shared_ptr<Model> Create(const Model &other)
			{
				return std::shared_ptr<Model> {new TModel {other}};
			}
			template<class TModel>
			static std::shared_ptr<Model> Load(NetworkState &nw, const udm::AssetData &data, std::string &outErr)
			{
				auto mdl = Create<TModel>(&nw, 0u);
				if(Load(*mdl, nw, data, outErr) == false)
					return nullptr;
				return mdl;
			}
			static void GenerateStandardMetaRigReferenceBonePoses(const animation::MetaRig &metaRig, const animation::Skeleton &skeleton, const Frame &refFrame, std::vector<math::ScaledTransform> &outPoses);
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
			virtual void AddMesh(const std::string &meshGroup, const std::shared_ptr<geometry::ModelMesh> &mesh);
			bool IsValid() const;
			CallbackHandle CallOnMaterialsLoaded(const std::function<void(void)> &f);
			void GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, std::vector<std::shared_ptr<geometry::ModelMesh>> &outMeshes) const;
			void GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, uint32_t lod, std::vector<std::shared_ptr<geometry::ModelMesh>> &outMeshes) const;
			void GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, std::vector<std::shared_ptr<geometry::ModelSubMesh>> &outMeshes) const;
			void GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups, uint32_t lod, std::vector<std::shared_ptr<geometry::ModelSubMesh>> &outMeshes) const;
			// Returns all existing meshes in this model (Including LOD meshes)
			std::vector<std::shared_ptr<geometry::ModelMesh>> *GetMeshes(const std::string &meshGroup);
			std::shared_ptr<ModelMeshGroup> GetMeshGroup(const std::string &meshGroup);
			std::shared_ptr<ModelMeshGroup> AddMeshGroup(const std::string &meshGroup, uint32_t &meshGroupId);
			std::shared_ptr<ModelMeshGroup> AddMeshGroup(const std::string &meshGroup);
			bool GetMeshGroupId(const std::string &meshGroup, uint32_t &groupId) const;
			std::shared_ptr<ModelMeshGroup> GetMeshGroup(uint32_t groupId);
			void AddMeshGroup(std::shared_ptr<ModelMeshGroup> &meshGroup);
			std::vector<std::shared_ptr<ModelMeshGroup>> &GetMeshGroups();
			const std::vector<std::shared_ptr<ModelMeshGroup>> &GetMeshGroups() const;
			std::vector<std::shared_ptr<physics::CollisionMesh>> &GetCollisionMeshes();
			const std::vector<std::shared_ptr<physics::CollisionMesh>> &GetCollisionMeshes() const;
			void AddCollisionMesh(const std::shared_ptr<physics::CollisionMesh> &mesh);
			// Gets the level of detail info for the given lod, or the next best
			LODInfo *GetLODInfo(uint32_t lod);
			LODInfo *AddLODInfo(uint32_t lod, float distance, std::unordered_map<uint32_t, uint32_t> &replaceIds);
			uint32_t GetLODCount() const;
			uint32_t GetLOD(uint32_t id) const;
			const std::vector<LODInfo> &GetLODs() const;
			bool TranslateLODMeshes(uint32_t lod, std::vector<uint32_t> &meshIds);
			// Returns true if the bodygroup exists and sets 'outMeshId' to the mesh Id. If the bodygroup mesh is none/blank, 'outMeshId' will be (unsigned int)(-1)
			bool GetMesh(uint32_t bodyGroupId, uint32_t groupId, uint32_t &outMeshId);
			geometry::ModelMesh *GetMesh(uint32_t meshGroupIdx, uint32_t meshIdx);
			const geometry::ModelMesh *GetMesh(uint32_t meshGroupIdx, uint32_t meshIdx) const { return const_cast<Model *>(this)->GetMesh(meshGroupIdx, meshIdx); }
			geometry::ModelSubMesh *GetSubMesh(uint32_t meshGroupIdx, uint32_t meshIdx, uint32_t subMeshIdx);
			const geometry::ModelSubMesh *GetSubMesh(uint32_t meshGroupIdx, uint32_t meshIdx, uint32_t subMeshIdx) const { return const_cast<Model *>(this)->GetSubMesh(meshGroupIdx, meshIdx, subMeshIdx); }
			void GetMeshes(const std::vector<uint32_t> &meshIds, std::vector<std::shared_ptr<geometry::ModelMesh>> &outMeshes);
			void GetSubMeshes(const std::vector<uint32_t> &meshIds, std::vector<std::shared_ptr<geometry::ModelSubMesh>> &outMeshes);
			//void GetWeights(std::vector<VertexWeight*> **weights);
			static void ClearCache();
			const std::string &GetName() const;
			void SetName(const std::string &name) { m_name = name; }
			uint32_t AddAnimation(const std::string &name, const std::shared_ptr<animation::Animation> &anim);
			int LookupAnimation(const std::string &name) const;
			int SelectWeightedAnimation(Activity activity, int32_t animIgnore = -1);
			int SelectFirstAnimation(Activity activity) const;
			unsigned char GetAnimationActivityWeight(uint32_t animation) const;
			Activity GetAnimationActivity(uint32_t animation) const;
			float GetAnimationDuration(uint32_t animation);
			std::shared_ptr<animation::Animation> GetAnimation(uint32_t ID) const;
			void GetAnimations(Activity activity, std::vector<uint32_t> &animations);
			void GetAnimations(std::unordered_map<std::string, uint32_t> **anims);
			const std::vector<std::shared_ptr<animation::Animation>> &GetAnimations() const;
			std::vector<std::shared_ptr<animation::Animation>> &GetAnimations();
			std::unordered_map<std::string, unsigned int> &GetAnimationNames() { return m_animationIDs; }
			const std::unordered_map<std::string, unsigned int> &GetAnimationNames() const { return const_cast<Model *>(this)->GetAnimationNames(); }
			bool GetAnimationName(uint32_t animId, std::string &name) const;
			std::string GetAnimationName(uint32_t animId) const;
			uint32_t GetAnimationCount() const;
			bool HasVertexWeights() const;
			std::optional<float> CalcFlexWeight(uint32_t flexId, const std::function<std::optional<float>(uint32_t)> &fFetchFlexControllerWeight, const std::function<std::optional<float>(uint32_t)> &fFetchFlexWeight) const;
			virtual std::shared_ptr<geometry::ModelMesh> CreateMesh() const;
			virtual std::shared_ptr<geometry::ModelSubMesh> CreateSubMesh() const;
			float CalcBoneLength(animation::BoneId boneId) const;
			void RemoveBone(animation::BoneId boneId);

			// Vertex animations
			const std::vector<std::shared_ptr<animation::VertexAnimation>> &GetVertexAnimations() const;
			std::vector<std::shared_ptr<animation::VertexAnimation>> &GetVertexAnimations();
			const std::shared_ptr<animation::VertexAnimation> *GetVertexAnimation(uint32_t vaIdx) const;
			bool GetVertexAnimationId(const std::string &name, uint32_t &id) const;
			std::shared_ptr<animation::VertexAnimation> *GetVertexAnimation(uint32_t vaIdx);
			const std::shared_ptr<animation::VertexAnimation> *GetVertexAnimation(const std::string &name) const;
			std::shared_ptr<animation::VertexAnimation> *GetVertexAnimation(const std::string &name);
			std::shared_ptr<animation::VertexAnimation> AddVertexAnimation(const std::string &name);
			void RemoveVertexAnimation(const std::string &name);

			// Flex controllers
			const std::vector<animation::FlexController> &GetFlexControllers() const;
			bool GetFlexControllerId(const std::string &name, uint32_t &id) const;
			std::vector<animation::FlexController> &GetFlexControllers();
			const animation::FlexController *GetFlexController(uint32_t id) const;
			animation::FlexController *GetFlexController(uint32_t id);
			const animation::FlexController *GetFlexController(const std::string &name) const;
			animation::FlexController *GetFlexController(const std::string &name);
			animation::FlexController &AddFlexController(const std::string &name);
			void RemoveFlexController(uint32_t id);
			void RemoveFlexController(const std::string &name);
			uint32_t GetFlexControllerCount() const;
			const std::string *GetFlexControllerName(uint32_t id) const;

			// Flexes
			const std::vector<animation::Flex> &GetFlexes() const;
			bool GetFlexId(const std::string &name, uint32_t &id) const;
			std::vector<animation::Flex> &GetFlexes();
			const animation::Flex *GetFlex(uint32_t id) const;
			animation::Flex *GetFlex(uint32_t id);
			const animation::Flex *GetFlex(const std::string &name) const;
			animation::Flex *GetFlex(const std::string &name);
			animation::Flex &AddFlex(const std::string &name);
			void RemoveFlex(uint32_t id);
			void RemoveFlex(const std::string &name);
			uint32_t GetFlexCount() const;
			const std::string *GetFlexName(uint32_t id) const;
			bool GetFlexFormula(uint32_t id, std::string &formula) const;
			bool GetFlexFormula(const std::string &name, std::string &formula) const;

			// Inverse kinematics
			const std::vector<std::shared_ptr<physics::IKController>> &GetIKControllers() const;
			std::vector<std::shared_ptr<physics::IKController>> &GetIKControllers();
			const physics::IKController *GetIKController(uint32_t id) const;
			physics::IKController *GetIKController(uint32_t id);
			bool LookupIKController(const std::string &name, uint32_t &id) const;
			physics::IKController *AddIKController(const std::string &name, uint32_t chainLength, const std::string &type, physics::ik::Method method = physics::ik::Method::Default);
			void RemoveIKController(uint32_t id);
			void RemoveIKController(const std::string &name);

			// Phonemes
			const PhonemeMap &GetPhonemeMap() const;
			PhonemeMap &GetPhonemeMap();

			bool FindSubMeshIndex(const ModelMeshGroup *optMeshGroup, const geometry::ModelMesh *optMesh, const geometry::ModelSubMesh *optSubMesh, uint32_t &outGroupIdx, uint32_t &outMeshIdx, uint32_t &outSubMeshIdx) const;

			const animation::Skeleton &GetSkeleton() const;
			animation::Skeleton &GetSkeleton();

			void TransformBone(animation::BoneId boneId, const math::Transform &t, math::CoordinateSpace space = math::CoordinateSpace::World);

			const std::shared_ptr<animation::MetaRig> &GetMetaRig() const;
			bool GenerateMetaRig();
			bool GenerateMetaBlendShapes();
			void ClearMetaRig();
			std::optional<animation::MetaRigBoneType> GetMetaRigBoneParentId(animation::MetaRigBoneType type) const;
			std::optional<math::ScaledTransform> GetMetaRigReferencePose(animation::MetaRigBoneType type) const;
			bool GenerateStandardMetaRigReferenceBonePoses(std::vector<math::ScaledTransform> &outPoses) const;
			Quat CalcNormalizedMetaBoneRotation(animation::MetaRigBoneType type, const Quat &metaReferenceRot, const Quat &posedRot) const;
			Quat RetargetMetaBoneRotation(animation::MetaRigBoneType type, const Quat &metaReferenceRot, const Quat &posedRot, const animation::MetaRigBone &targetBone, const Quat &targetMetaReferenceRot) const;
			void ApplyPostImportProcessing();

			uint32_t GetBoneCount() const;
			bool GetLocalBonePosition(uint32_t animId, uint32_t frameId, uint32_t boneId, Vector3 &rPos, Quat &rRot, Vector3 *scale = nullptr);

			bool GetReferenceBonePose(animation::BoneId boneId, math::Transform &outPose, math::CoordinateSpace space = math::CoordinateSpace::Object) const;
			bool GetReferenceBonePose(animation::BoneId boneId, math::ScaledTransform &outPose, math::CoordinateSpace space = math::CoordinateSpace::Object) const;
			bool GetReferenceBonePos(animation::BoneId boneId, Vector3 &outPos, math::CoordinateSpace space = math::CoordinateSpace::Object) const;
			bool GetReferenceBoneRot(animation::BoneId boneId, Quat &outRot, math::CoordinateSpace space = math::CoordinateSpace::Object) const;
			bool GetReferenceBoneScale(animation::BoneId boneId, Vector3 &outScale, math::CoordinateSpace space = math::CoordinateSpace::Object) const;
			bool GetReferenceBonePose(animation::BoneId boneId, Vector3 *optOutPos, Quat *optOutRot, Vector3 *optOutScale = nullptr, math::CoordinateSpace space = math::CoordinateSpace::Object) const;

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
			std::optional<math::ScaledTransform> CalcReferenceAttachmentPose(int32_t attId) const;
			std::optional<math::ScaledTransform> CalcReferenceBonePose(int32_t boneId) const;

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

			std::optional<uint32_t> AssignDistinctMaterial(const ModelMeshGroup &group, const geometry::ModelMesh &mesh, geometry::ModelSubMesh &subMesh);

			// Hitboxes
			void AddHitbox(uint32_t boneId, physics::HitGroup group, const Vector3 &min, const Vector3 &max);
			void AddHitbox(uint32_t boneId, const physics::Hitbox &hitbox);
			uint32_t GetHitboxCount() const;
			const std::unordered_map<uint32_t, physics::Hitbox> &GetHitboxes() const;
			std::unordered_map<uint32_t, physics::Hitbox> &GetHitboxes();
			const physics::Hitbox *GetHitbox(uint32_t boneId) const;
			physics::HitGroup GetHitboxGroup(uint32_t boneId) const;
			bool GetHitboxBounds(uint32_t boneId, Vector3 &min, Vector3 &max) const;
			std::vector<uint32_t> GetHitboxBones(physics::HitGroup group) const;
			void GetHitboxBones(physics::HitGroup group, std::vector<uint32_t> &boneIds) const;
			std::vector<uint32_t> GetHitboxBones() const;
			void GetHitboxBones(std::vector<uint32_t> &boneIds) const;
			std::unordered_map<animation::BoneId, physics::Hitbox> CalcHitboxes() const;
			bool GenerateHitboxes();

			void UpdateShape(const std::vector<physics::SurfaceMaterial> *materials = nullptr);
			const std::vector<uint32_t> &GetBaseMeshes() const;
			std::vector<uint32_t> &GetBaseMeshes();
			uint32_t GetMeshGroupCount() const;
			uint32_t GetMeshCount() const;
			uint32_t GetSubMeshCount() const;
			uint32_t GetCollisionMeshCount() const;
			uint32_t GetVertexCount() const;
			uint32_t GetTriangleCount() const;
			// Textures
			uint32_t AddTexture(const std::string &tex, material::Material *mat);
			bool SetTexture(uint32_t texIdx, const std::string &tex, material::Material *mat);
			uint32_t AddMaterial(uint32_t skin, material::Material *mat, const std::optional<std::string> &matName = {}, std::optional<uint32_t> *optOutSkinTexIdx = nullptr);
			bool SetMaterial(uint32_t texIdx, material::Material *mat);
			void RemoveTexture(uint32_t idx);
			void ClearTextures();
			void LoadMaterials(bool bReload = false);
			void PrecacheMaterials();
			TextureGroup *CreateTextureGroup();
			std::vector<std::string> &GetTextures();
			std::vector<material::MaterialHandle> &GetMaterials();
			const std::vector<material::MaterialHandle> &GetMaterials() const;
			material::Material *GetMaterial(uint32_t texID);
			material::Material *GetMaterial(uint32_t texGroup, uint32_t texID);
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
			std::optional<uint32_t> GetMaterialIndex(const geometry::ModelSubMesh &mesh, uint32_t skinId = 0) const;
			void SetReference(std::shared_ptr<Frame> frame);
			const Frame &GetReference() const;
			Frame &GetReference();
			bool SetReferencePoses(const std::vector<math::ScaledTransform> &poses, bool posesInParentSpace = false);
			void Rotate(const Quat &rot);
			void Translate(const Vector3 &t);
			void Scale(const Vector3 &scale);
			void Mirror(Axis axis);

			// Merges meshes with same materials (Only within mesh groups)
			void Optimize();
			void Validate();

			// BodyGroups
			BodyGroup *GetBodyGroup(uint32_t id);
			BodyGroup &AddBodyGroup(const std::string &name);
			Int32 GetBodyGroupId(const std::string &name);
			std::vector<BodyGroup> &GetBodyGroups();
			UInt32 GetBodyGroupCount() const;

			const std::vector<physics::JointInfo> &GetJoints() const;
			std::vector<physics::JointInfo> &GetJoints();
			physics::JointInfo &AddJoint(physics::JointType type, animation::BoneId child, animation::BoneId parent);

			const std::vector<Eyeball> &GetEyeballs() const;
			std::vector<Eyeball> &GetEyeballs();
			uint32_t GetEyeballCount() const;
			const Eyeball *GetEyeball(uint32_t idx) const;
			Eyeball *GetEyeball(uint32_t idx);
			void AddEyeball(const Eyeball &eyeball);

			void SetMaxEyeDeflection(math::Degree eyeDeflection);
			math::Degree GetMaxEyeDeflection() const;

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

			std::optional<math::ScaledTransform> GetReferenceBonePose(animation::BoneId boneId) const;
			std::optional<SignedAxis> FindBoneTwistAxis(animation::BoneId boneId) const;
			std::optional<SignedAxis> FindBoneAxisForDirection(animation::BoneId boneId, const Vector3 &dir) const;
			static Quat GetTwistAxisRotationOffset(SignedAxis axis);
		  protected:
			Model(NetworkState *nw, uint32_t numBones, const std::string &name = "");
			Model(const Model &other);
			bool LoadFromAssetData(Game &game, const udm::AssetData &data, std::string &outErr);
			virtual void OnMaterialMissing(const std::string &matName);
			void AddLoadingMaterial(material::Material &mat, std::optional<uint32_t> index = {});
			void LoadMaterials(const std::vector<uint32_t> &textureGroups, bool precache, bool bReload);
			void LoadMaterials(bool precache, bool bReload);
			bool FindMaterial(const std::string &texture, std::string &matPath, bool importIfNotFound) const;

			virtual std::shared_ptr<animation::VertexAnimation> CreateVertexAnimation(const std::string &name) const;
			std::vector<std::shared_ptr<animation::VertexAnimation>>::const_iterator FindVertexAnimation(const std::string &name) const;
			std::vector<std::shared_ptr<animation::VertexAnimation>>::iterator FindVertexAnimation(const std::string &name);

			std::vector<animation::FlexController>::const_iterator FindFlexController(const std::string &name) const;
			std::vector<animation::FlexController>::iterator FindFlexController(const std::string &name);

			std::vector<std::shared_ptr<physics::IKController>>::const_iterator FindIKController(const std::string &name) const;
			std::vector<std::shared_ptr<physics::IKController>>::iterator FindIKController(const std::string &name);

			std::vector<animation::Flex>::const_iterator FindFlex(const std::string &name) const;
			std::vector<animation::Flex>::iterator FindFlex(const std::string &name);
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
			math::Degree m_maxEyeDeflection = 30.f;
			PhonemeMap m_phonemeMap = {};
			std::vector<BlendController> m_blendControllers;
			std::vector<std::shared_ptr<ModelMeshGroup>> m_meshGroups;
			std::vector<BodyGroup> m_bodyGroups;
			std::unordered_map<uint32_t, physics::Hitbox> m_hitboxes;
			std::vector<Eyeball> m_eyeballs;
			//std::vector<std::vector<VertexWeight>*> m_weights;
			static std::unordered_map<std::string, std::shared_ptr<Model>> m_models;
			std::shared_ptr<Frame> m_reference = nullptr;
			std::string m_name;
			std::vector<std::shared_ptr<animation::Animation>> m_animations;
			std::vector<std::shared_ptr<animation::VertexAnimation>> m_vertexAnimations;
			std::unordered_map<std::string, unsigned int> m_animationIDs;
			std::shared_ptr<animation::Skeleton> m_skeleton = nullptr;
			std::shared_ptr<animation::MetaRig> m_metaRig = nullptr;

			std::vector<animation::FlexController> m_flexControllers;
			std::vector<animation::Flex> m_flexes;

			std::vector<std::shared_ptr<physics::IKController>> m_ikControllers;

			std::vector<std::shared_ptr<FlexAnimation>> m_flexAnimations;
			std::vector<std::string> m_flexAnimationNames;

			// Bind pose matrices are currently unused; Bind pose is extracted from reference pose instead!
			std::vector<Mat4> m_bindPose;
			Vector3 m_eyeOffset = {};
			Vector3 m_collisionMin = {};
			Vector3 m_collisionMax = {};
			Vector3 m_renderMin = {};
			Vector3 m_renderMax = {};
			std::vector<std::shared_ptr<physics::CollisionMesh>> m_collisionMeshes;
			std::vector<physics::JointInfo> m_joints;
			std::vector<unsigned int> m_baseMeshes; // Meshes in LOD 0
			std::vector<LODInfo> m_lods;            // LODs have to be in order!
			std::vector<Attachment> m_attachments;
			std::vector<ObjectAttachment> m_objectAttachments;
			std::vector<material::MaterialHandle> m_materials;
			std::vector<TextureGroup> m_textureGroups;
			std::vector<CallbackHandle> m_matLoadCallbacks;
			std::vector<CallbackHandle> m_onAllMatsLoadedCallbacks;
			void OnMaterialLoaded();
		};

		using namespace pragma::math::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::asset::Model::CopyFlags)
	REGISTER_ENUM_FLAGS(pragma::asset::Model::MergeFlags)
	REGISTER_ENUM_FLAGS(pragma::asset::Model::Flags)
	REGISTER_ENUM_FLAGS(pragma::asset::Model::StateFlags)

#pragma warning(pop)
};
