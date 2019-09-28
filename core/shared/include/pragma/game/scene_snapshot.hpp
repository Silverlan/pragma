#ifndef __PRAGMA_SCENE_SNAPSHOT_HPP__
#define __PRAGMA_SCENE_SNAPSHOT_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/physics/transform.hpp"
#include <memory>
#include <functional>
#include <mathutil/uvec.h>
#include <material.h>

class Model;
namespace pragma
{
	class DLLNETWORK SceneSnapshot
		: public std::enable_shared_from_this<SceneSnapshot>
	{
	public:
		struct DLLNETWORK CreateInfo
		{
			enum class Flags : uint8_t
			{
				None = 0u,
				IncludeLevelGeometry = 1u,
				IncludeStaticObjects = IncludeLevelGeometry<<1u,
				IncludeDynamicObjects = IncludeStaticObjects<<1u,
				IncludeLightSources = IncludeDynamicObjects<<1u
			};
			Flags flags = Flags::None;
		};
		struct DLLNETWORK Vertex
		{
			Vector3 position;
			Vector3 normal;
			Vector2 uv;
		};
		struct DLLNETWORK Mesh
		{
			std::vector<Vertex> verts = {};
			std::vector<uint16_t> tris = {};
			std::vector<Vector2> lightmapUvs = {};
			MaterialHandle material = {};
		};
		struct DLLNETWORK Object
		{
			pragma::physics::Transform pose = {};
		};
		static std::shared_ptr<SceneSnapshot> Create();
		SceneSnapshot(const SceneSnapshot&)=delete;
		SceneSnapshot &operator=(const SceneSnapshot&)=delete;

		void MergeMeshesByMaterial();
		void AddModel(Model &mdl,uint32_t skin=0u,std::vector<std::vector<Vector2>> *inLightmapUvs=nullptr);
		const std::vector<std::shared_ptr<Mesh>> &GetMeshes() const;
		std::vector<std::shared_ptr<Mesh>> &GetMeshes();
		const std::vector<std::shared_ptr<Object>> &GetObjects() const;
		std::vector<std::shared_ptr<Object>> &GetObjects();
		const std::vector<MaterialHandle> &GetMaterials() const;
		std::vector<MaterialHandle> &GetMaterials();
	private:
		void AddMaterial(Material &mat);
		SceneSnapshot()=default;
		std::vector<std::shared_ptr<Mesh>> m_meshes = {};
		std::vector<std::shared_ptr<Object>> m_objects = {};
		std::vector<MaterialHandle> m_materials = {};
	};
};

#endif
