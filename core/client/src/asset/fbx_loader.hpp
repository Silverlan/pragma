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
};
namespace pragma::animation {
	struct Bone;
};
class Model;
namespace pragma::asset::fbx {
	class FbxImporter {
	  public:
		FbxImporter(ofbx::IScene *scene, const std::string &mdlPath, const std::string &mdlName) : m_fbxScene {scene}, m_mdlPath {mdlPath}, m_mdlName {mdlName} {}
		~FbxImporter();
		std::optional<AssetImportResult> Load(std::string &outErr);
	  private:
		std::shared_ptr<Model> m_model;
		std::string m_mdlPath;
		std::string m_mdlName;
		ofbx::IScene *m_fbxScene;

		void FillSkinInfo(std::vector<umath::VertexWeight> &skinning, const ofbx::Mesh *mesh, int32_t boneId);
		bool LoadMaterial(const ofbx::Material &mat, std::string &outErr);
		bool LoadMeshes(std::string &outErr);
		bool LoadAnimations(std::string &outErr);
		std::optional<std::string> Finalize(std::string &outErr);
		std::shared_ptr<pragma::animation::Bone> AddBone(const ofbx::Object &o);
	};
};

#endif
