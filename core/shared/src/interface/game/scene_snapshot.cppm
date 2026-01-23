// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:game.scene_snapshot;

export import :types;
export import pragma.materialsystem;

export {
	namespace pragma::game {
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
				material::MaterialHandle material = {};
			};
			struct DLLNETWORK Object {
				math::Transform pose = {};
			};
			static std::shared_ptr<SceneSnapshot> Create();
			SceneSnapshot(const SceneSnapshot &) = delete;
			SceneSnapshot &operator=(const SceneSnapshot &) = delete;

			void MergeMeshesByMaterial();
			void AddModel(asset::Model &mdl, uint32_t skin = 0u);
			const std::vector<std::shared_ptr<Mesh>> &GetMeshes() const;
			std::vector<std::shared_ptr<Mesh>> &GetMeshes();
			const std::vector<std::shared_ptr<Object>> &GetObjects() const;
			std::vector<std::shared_ptr<Object>> &GetObjects();
			const std::vector<material::MaterialHandle> &GetMaterials() const;
			std::vector<material::MaterialHandle> &GetMaterials();
		  private:
			void AddMaterial(material::Material &mat);
			SceneSnapshot() = default;
			std::vector<std::shared_ptr<Mesh>> m_meshes = {};
			std::vector<std::shared_ptr<Object>> m_objects = {};
			std::vector<material::MaterialHandle> m_materials = {};
		};
	};
};
