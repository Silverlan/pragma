// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <vector>
#include <queue>
#include <memory>
#include <pragma/entities/baseentity_handle.h>

export module pragma.client.rendering.sorted_render_info;

export {
	using CSubMeshIterator = std::vector<CModelSubMesh *>::iterator;
	#pragma warning(push)
	#pragma warning(disable : 4251)
	class DLLCLIENT EntityMeshContainer {
	private:
		std::queue<UInt32> m_freeIndices;
		std::vector<CModelSubMesh *> m_meshes;
		CSubMeshIterator m_end;
		void UpdateLastIterator();
	protected:
		friend SortedMaterialMeshContainer;
		void Insert(CModelSubMesh *mesh, std::vector<SortedEntityMeshIteratorInfo> &locations);
	public:
		EntityMeshContainer(CBaseEntity *ent);
		CBaseEntity *entity;
		CSubMeshIterator begin();
		CSubMeshIterator end();
		void Erase(UInt32 idx);
	};

	struct SortedEntityMeshIteratorInfo {
	private:
		UInt32 index;
		std::vector<CModelSubMesh *> *container;
	public:
		SortedEntityMeshIteratorInfo(std::vector<CModelSubMesh *> &_container, UInt32 idx) : container(&_container), index(idx) {}
		void Erase() { (*container)[index] = nullptr; }
	};

	using SortedEntityMeshIterator = std::vector<std::unique_ptr<EntityMeshContainer>>::iterator;
	class DLLCLIENT SortedMaterialMeshContainer {
	private:
		std::vector<std::unique_ptr<EntityMeshContainer>> m_ents;
	protected:
		friend SortedShaderMeshContainer;
		void Insert(CBaseEntity *ent, std::vector<SortedEntityMeshIteratorInfo> &locations, CModelSubMesh *mesh);
	public:
		SortedMaterialMeshContainer(Material *mat);
		SortedMaterialMeshContainer(SortedMaterialMeshContainer &) = delete;
		SortedMaterialMeshContainer &operator=(const SortedMaterialMeshContainer &other) = delete;
		Material *material;
		SortedEntityMeshIterator begin();
		SortedEntityMeshIterator end();
	};

	using SortedMaterialMeshIterator = std::vector<std::unique_ptr<SortedMaterialMeshContainer>>::iterator;
	class DLLCLIENT SortedShaderMeshContainer {
	private:
		std::vector<std::unique_ptr<SortedMaterialMeshContainer>> m_materials;
	protected:
		friend SortedRenderMeshContainer;
		void Insert(CBaseEntity *ent, std::vector<SortedEntityMeshIteratorInfo> &locations, CModelSubMesh *mesh, Material *mat);
	public:
		SortedShaderMeshContainer(Shader::TexturedBase3D *shader);
		SortedShaderMeshContainer(SortedShaderMeshContainer &) = delete;
		SortedShaderMeshContainer &operator=(const SortedShaderMeshContainer &other) = delete;
		Shader::TexturedBase3D *shader;
		SortedMaterialMeshIterator begin();
		SortedMaterialMeshIterator end();
	};

	using SortedShaderMeshIterator = std::vector<std::unique_ptr<SortedShaderMeshContainer>>::iterator;
	using SortedRenderInfoIterator = std::vector<SortedRenderInfo>::iterator;
	class DLLCLIENT SortedRenderMeshContainer {
	private:
		static std::vector<std::unique_ptr<SortedShaderMeshContainer>> m_shaders;
		static std::vector<SortedRenderInfo> m_sortedData;
		std::vector<SortedEntityMeshIteratorInfo> m_meshLocations;

		void Insert(CBaseEntity *ent, CModelSubMesh *mesh, Material *mat, Shader::TexturedBase3D *shader);
		UInt FindDataIndex(Shader::TexturedBase3D *shader, Material *mat);
	public:
		SortedRenderMeshContainer(CBaseEntity *ent, std::vector<std::shared_ptr<ModelMesh>> &meshes);
		~SortedRenderMeshContainer();
		static SortedRenderInfoIterator begin();
		static SortedRenderInfoIterator end();
	};

	struct DLLCLIENT SortedRenderInfo {
		SortedRenderInfo(Shader::TexturedBase3D *shader, Material *material, CBaseEntity *entity, CModelSubMesh *mesh);
		Shader::TexturedBase3D *shader;
		Material *material;
		CBaseEntity *entity;
		CModelSubMesh *mesh;
	};
	#pragma warning(pop)
};
