// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <ofbx.h>

export module pragma.client:assets.fbx_loader;

import :assets.import_export;

export namespace pragma::asset::fbx {
	enum class RotationOrder : uint8_t {
		Xyz,
		Xzy,
		Yzx,
		Yxz,
		Zxy,
		Zyx,
	};

	enum class UpVector : uint8_t {
		X,
		Y,
		Z,
	};

	class FbxImporter {
	  public:
		FbxImporter(ofbx::IScene *scene, const std::string &mdlPath, const std::string &mdlName, const util::Path &outputPath) : m_fbxScene {scene}, m_mdlPath {mdlPath}, m_mdlName {mdlName}, m_outputPath {outputPath} {}
		~FbxImporter();
		std::optional<AssetImportResult> Load(std::string &outErr);
	  private:
		struct FbxMeshInfo {
			const ofbx::Mesh *mesh = nullptr;
			const ofbx::Material *material = nullptr;
			int32_t subMesh = -1;
			int32_t boneIndex = -1;
			int32_t lod = -1;
			bool skinned = false;
		};
		std::shared_ptr<Model> m_model;
		std::string m_mdlPath;
		std::string m_mdlName;
		util::Path m_outputPath;
		ofbx::IScene *m_fbxScene;
		float m_fbxScale = 1.f;
		std::vector<const ofbx::Object *> m_fbxBones;
		std::vector<FbxMeshInfo> m_fbxMeshes;
		UpVector m_upVector = UpVector::Y;

		static math::ScaledTransform GetPose(const ofbx::Object &o);
		static Vector3 GetTranslation(const ofbx::DVec3 &o);
		static Quat GetRotation(const ofbx::DVec3 &o, RotationOrder order);
		static Vector3 GetScale(const ofbx::DVec3 &o);
		static std::string GetImportMeshName(const FbxMeshInfo &mesh);
		static int DetectMeshLOD(const FbxMeshInfo &mesh);

		void GatherBones(bool force_skinned);
		void SortBones(bool force_skinned);
		void InsertHierarchy(std::vector<const ofbx::Object *> &bones, const ofbx::Object *node);
		void FillSkinInfo(std::vector<math::VertexWeight> &skinning, const ofbx::Mesh *mesh, int32_t boneId);
		std::optional<uint32_t> LoadMaterial(const ofbx::Material &mat, uint32_t partitionIdx, std::string &outErr);
		bool LoadMeshes(std::string &outErr);
		bool LoadAnimations(std::string &outErr);
		std::optional<std::string> Finalize(std::string &outErr);
		std::shared_ptr<animation::Bone> AddBone(const ofbx::Object &o);
	};
};
