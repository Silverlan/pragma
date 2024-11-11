/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#ifndef __PRAGMA_FBX_LOADER_HPP__
#define __PRAGMA_FBX_LOADER_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/asset/c_util_model.hpp"
#include <sharedutils/util_path.hpp>
#include <mathutil/vertex.hpp>
#include <string>
#include <memory>
#include <vector>
#include <cinttypes>

namespace ofbx {
	struct IScene;
	struct Object;
	struct Mesh;
	struct Material;
	struct DVec3;
};
namespace pragma::animation {
	struct Bone;
};
class Model;
namespace pragma::asset::fbx {
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

		static umath::ScaledTransform GetPose(const ofbx::Object &o);
		static Vector3 GetTranslation(const ofbx::DVec3 &o);
		static Quat GetRotation(const ofbx::DVec3 &o, RotationOrder order);
		static Vector3 GetScale(const ofbx::DVec3 &o);
		static std::string GetImportMeshName(const FbxMeshInfo &mesh);
		static int DetectMeshLOD(const FbxMeshInfo &mesh);

		void GatherBones(bool force_skinned);
		void SortBones(bool force_skinned);
		void InsertHierarchy(std::vector<const ofbx::Object *> &bones, const ofbx::Object *node);
		void FillSkinInfo(std::vector<umath::VertexWeight> &skinning, const ofbx::Mesh *mesh, int32_t boneId);
		std::optional<uint32_t> LoadMaterial(const ofbx::Material &mat, uint32_t partitionIdx, std::string &outErr);
		bool LoadMeshes(std::string &outErr);
		bool LoadAnimations(std::string &outErr);
		std::optional<std::string> Finalize(std::string &outErr);
		std::shared_ptr<pragma::animation::Bone> AddBone(const ofbx::Object &o);
	};
};

#endif
