/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_SCENE_SNAPSHOT_HPP__
#define __PRAGMA_SCENE_SNAPSHOT_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/transform.hpp>
#include <memory>
#include <functional>
#include <mathutil/uvec.h>
#include <material.h>

class Model;
namespace pragma {
	class DLLNETWORK SceneSnapshot : public std::enable_shared_from_this<SceneSnapshot> {
	  public:
		struct DLLNETWORK CreateInfo {
			enum class Flags : uint8_t { None = 0u, IncludeLevelGeometry = 1u, IncludeStaticObjects = IncludeLevelGeometry << 1u, IncludeDynamicObjects = IncludeStaticObjects << 1u, IncludeLightSources = IncludeDynamicObjects << 1u };
			Flags flags = Flags::None;
		};
		struct DLLNETWORK Vertex {
			Vector3 position;
			Vector3 normal;
			Vector2 uv;
		};
		struct DLLNETWORK Mesh {
			std::vector<Vertex> verts = {};
			std::vector<uint32_t> tris = {};
			std::vector<Vector2> lightmapUvs = {};
			msys::MaterialHandle material = {};
		};
		struct DLLNETWORK Object {
			umath::Transform pose = {};
		};
		static std::shared_ptr<SceneSnapshot> Create();
		SceneSnapshot(const SceneSnapshot &) = delete;
		SceneSnapshot &operator=(const SceneSnapshot &) = delete;

		void MergeMeshesByMaterial();
		void AddModel(Model &mdl, uint32_t skin = 0u);
		const std::vector<std::shared_ptr<Mesh>> &GetMeshes() const;
		std::vector<std::shared_ptr<Mesh>> &GetMeshes();
		const std::vector<std::shared_ptr<Object>> &GetObjects() const;
		std::vector<std::shared_ptr<Object>> &GetObjects();
		const std::vector<msys::MaterialHandle> &GetMaterials() const;
		std::vector<msys::MaterialHandle> &GetMaterials();
	  private:
		void AddMaterial(Material &mat);
		SceneSnapshot() = default;
		std::vector<std::shared_ptr<Mesh>> m_meshes = {};
		std::vector<std::shared_ptr<Object>> m_objects = {};
		std::vector<msys::MaterialHandle> m_materials = {};
	};
};

#endif
