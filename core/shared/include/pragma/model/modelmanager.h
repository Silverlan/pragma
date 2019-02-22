#ifndef __MODELMANAGER_H__
#define __MODELMANAGER_H__

#include <unordered_map>
#include <vector>
#include <string>
#include "pragma/networkdefinitions.h"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/file_formats/wmd.h"
#include <sharedutils/util_string.h>
#include "pragma/entities/baseworld.h"
#include <sharedutils/util_file.h>

template<class TModel,class TModelMesh,class TModelSubMesh>
	class TModelManager
{
protected:
	static std::unordered_map<std::string,std::shared_ptr<Model>> m_models;
	static std::vector<std::string> m_marked;
	static std::shared_ptr<Model> CreateModel(Game *game,bool bAddReference=true);
	static std::shared_ptr<Model> Load(Game *game,const std::string &mdlName,bool bReload=false,bool *newModel=nullptr);
	static std::shared_ptr<Model> Create(Game *game,const std::string &name);
	static std::shared_ptr<Model> GetModel(Game *game,const std::string &mdlName);
	static std::shared_ptr<Model> CreateFromBrushMeshes(Game *game,std::vector<std::shared_ptr<BrushMesh>> &meshes,const std::vector<SurfaceMaterial> &surfaceMaterials);
public:
	static std::string GetCanonicalizedName(const std::string &mdlName);
	static void MarkAllForDeletion();
	static void MarkForDeletion(const std::string &name);
	static void ClearMarkedModels();
	static std::unordered_map<std::string,std::shared_ptr<Model>> &GetModels();
	static void Clear();
};

template<class TModel,class TModelMesh,class TModelSubMesh>
	decltype(TModelManager<TModel,TModelMesh,TModelSubMesh>::m_models) TModelManager<TModel,TModelMesh,TModelSubMesh>::m_models;
template<class TModel,class TModelMesh,class TModelSubMesh>
	decltype(TModelManager<TModel,TModelMesh,TModelSubMesh>::m_marked) TModelManager<TModel,TModelMesh,TModelSubMesh>::m_marked;

template<class TModel,class TModelMesh,class TModelSubMesh>
	void TModelManager<TModel,TModelMesh,TModelSubMesh>::MarkAllForDeletion()
{
	m_marked.clear();
	for(auto it : m_models)
		m_marked.push_back(it.first);
}

template<class TModel,class TModelMesh,class TModelSubMesh>
	void TModelManager<TModel,TModelMesh,TModelSubMesh>::MarkForDeletion(const std::string &name)
{
	auto itMdl = m_models.find(name);
	if(itMdl == m_models.end())
		return;
	auto it = std::find_if(m_marked.begin(),m_marked.end(),[&itMdl](const std::string &other) {
		return (itMdl->first == other) ? true : false;
	});
	if(it != m_marked.end())
		return;
	m_marked.push_back(itMdl->first);
}

template<class TModel,class TModelMesh,class TModelSubMesh>
	void TModelManager<TModel,TModelMesh,TModelSubMesh>::ClearMarkedModels()
{
	for(auto &marked : m_marked)
	{
		auto it = m_models.find(marked);
		if(it != m_models.end())
			m_models.erase(it);
	}
	m_marked.clear();
}

template<class TModel,class TModelMesh,class TModelSubMesh>
	void TModelManager<TModel,TModelMesh,TModelSubMesh>::Clear()
{
	m_models.clear();
	m_marked.clear();
}

template<class TModel,class TModelMesh,class TModelSubMesh>
	std::unordered_map<std::string,std::shared_ptr<Model>> &TModelManager<TModel,TModelMesh,TModelSubMesh>::GetModels() {return m_models;}

template<class TModel,class TModelMesh,class TModelSubMesh>
	std::shared_ptr<Model> TModelManager<TModel,TModelMesh,TModelSubMesh>::CreateModel(Game *game,bool bAddReference)
{
	uint32_t boneCount = (bAddReference == true) ? 1 : 0;
	auto mdl = std::shared_ptr<TModel>(new TModel(game->GetNetworkState(),boneCount));
	auto &skeleton = mdl->GetSkeleton();
	auto reference = std::make_shared<Animation>();

	if(bAddReference == true)
	{
		auto frame = std::make_shared<Frame>(1);
		auto *root = new Bone;
		root->name = "root";
		auto rootID = skeleton.AddBone(root);
		mdl->SetBindPoseBoneMatrix(0,glm::inverse(umat::identity()));
		auto &rootBones = skeleton.GetRootBones();
		rootBones[0] = skeleton.GetBone(rootID).lock();
		reference->AddBoneId(0);

		frame->SetBonePosition(0,Vector3(0.f,0.f,0.f));
		frame->SetBoneOrientation(0,uquat::identity());

		auto refFrame = std::make_shared<Frame>(*frame);
		frame->Localize(*reference,skeleton);
		reference->AddFrame(frame);
		mdl->AddAnimation("reference",reference);
		mdl->SetReference(refFrame);

		auto &baseMeshes = mdl->GetBaseMeshes();
		baseMeshes.push_back(0);
		mdl->AddMeshGroup("reference");

		mdl->CreateTextureGroup();
	}
	return mdl;
}

template<class TModel,class TModelMesh,class TModelSubMesh>
	std::shared_ptr<Model> TModelManager<TModel,TModelMesh,TModelSubMesh>::Create(Game *game,const std::string &name)
{
	auto lname = GetCanonicalizedName(name);
	auto mdl = m_models[lname] = CreateModel(game);
	/*auto it = m_models.find(lname);
	if(it != m_models.end())
		return it->second;
	auto mdl = CreateModel();
	m_models.insert(decltype(m_models)::value_type(lname,mdl));*/
	return mdl;
}

template<class TModel,class TModelMesh,class TModelSubMesh>
	std::string TModelManager<TModel,TModelMesh,TModelSubMesh>::GetCanonicalizedName(const std::string &mdlName)
{
	auto lname = mdlName;
	ustring::to_lower(lname);
	std::string ext;
	if(ufile::get_extension(lname,&ext) == false)
		lname += ".wmd";
	return FileManager::GetCanonicalizedPath(lname);
}

template<class TModel,class TModelMesh,class TModelSubMesh>
	std::shared_ptr<Model> TModelManager<TModel,TModelMesh,TModelSubMesh>::GetModel(Game*,const std::string &mdlName)
{
	auto lname = GetCanonicalizedName(mdlName);
	auto it = m_models.find(lname);
	if(it == m_models.end())
		return nullptr;
	return it->second;
}

template<class TModel,class TModelMesh,class TModelSubMesh>
	std::shared_ptr<Model> TModelManager<TModel,TModelMesh,TModelSubMesh>::Load(Game *game,const std::string &mdlName,bool bReload,bool *newModel)
{
	if(newModel != nullptr)
		*newModel = false;
	auto lname = GetCanonicalizedName(mdlName);
	if(bReload == false)
	{
		auto it = m_models.find(lname);
		if(it != m_models.end())
		{
			auto &mdl = it->second;
			mdl->PrecacheTextureGroup(0); // TODO: What about other texture groups, if dontPrecache flag isn't set in the model?
			return mdl;
		}
		for(auto it=m_marked.begin();it!=m_marked.end();++it)
		{
			if(*it == lname)
			{
				m_marked.erase(it);
				break;
			}
		}
		it = m_models.find(lname);
		if(it != m_models.end())
		{
			auto &mdl = it->second;
			mdl->PrecacheTextureGroup(0); // TODO: What about other texture groups, if dontPrecache flag isn't set in the model?
			return mdl;
		}
	}
	FWMD wmd(game);
	auto mdl = std::shared_ptr<TModel>(
		wmd.Load<TModel,TModelMesh,TModelSubMesh>(
			game,lname.c_str(),
			std::bind(&NetworkState::LoadMaterial,game->GetNetworkState(),std::placeholders::_1,std::placeholders::_2),
			std::bind(&Game::LoadModel,game,std::placeholders::_1,false)
		)
	);
	if(mdl == nullptr)
		return nullptr;
	mdl->Update();
	m_models[lname] = mdl;
	if(newModel != nullptr)
		*newModel = true;
	return mdl;
}

template<class TModel,class TModelMesh,class TModelSubMesh>
	std::shared_ptr<Model> TModelManager<TModel,TModelMesh,TModelSubMesh>::CreateFromBrushMeshes(Game *game,std::vector<std::shared_ptr<BrushMesh>> &meshes,const std::vector<SurfaceMaterial> &surfaceMaterials)
{
	auto mdl = CreateModel(game);
	auto group = mdl->GetMeshGroup("reference");
	auto *texGroup = mdl->GetTextureGroup(0);
	std::unordered_map<Material*,unsigned int> textures;
	unsigned int textureId = 0;
	for(unsigned int i=0;i<meshes.size();i++)
	{
		auto &mesh = meshes[i];
		auto &sides = mesh->GetSides();
		for(unsigned int j=0;j<sides.size();j++)
		{
			auto &side = sides[j];
			auto *mat = side->GetMaterial();
			if(mat != NULL)
			{
				auto it = textures.find(mat);
				if(it == textures.end())
				{
					const std::string &name = mat->GetName();
					mdl->AddTexture(name,mat);
					textures.insert(std::unordered_map<Material*,unsigned int>::value_type(mat,textureId));
					texGroup->textures.push_back(textureId);
					textureId++;
				}
			}
		}
	}
	std::vector<std::unordered_map<uint32_t,uint32_t>> lods;
	Vector3 min(std::numeric_limits<glm::mediump_float>::max(),std::numeric_limits<glm::mediump_float>::max(),std::numeric_limits<glm::mediump_float>::max());
	Vector3 max(std::numeric_limits<glm::mediump_float>::lowest(),std::numeric_limits<glm::mediump_float>::lowest(),std::numeric_limits<glm::mediump_float>::lowest());
	for(unsigned int i=0;i<meshes.size();i++)
	{
		auto colMesh = CollisionMesh::Create(game);
		std::vector<Vector3> &colVerts = colMesh->GetVertices();
		auto &colTriangles = colMesh->GetTriangles();
		auto &colMaterials = colMesh->GetSurfaceMaterials();
		colMesh->SetBoneParent(0);
		auto &mesh = meshes[i];
		auto bConvex = mesh->IsConvex();
		colMesh->SetConvex(bConvex);
		Vector3 brushMin;
		Vector3 brushMax;
		mesh->GetBounds(&brushMin,&brushMax);
		uvec::min(&min,brushMin);
		uvec::max(&max,brushMax);
		auto &sides = mesh->GetSides();
		struct SurfaceMaterialInfo
		{
			SurfaceMaterialInfo(int id)
				: surfaceMaterialId(id),count(0)
			{}
			int surfaceMaterialId;
			int count;
		};
		std::vector<SurfaceMaterialInfo> surfaceMaterialInfo;
		auto surfaceMaterialId = 0;
		auto maxSurfaceMaterialCount = 0;
		std::vector<std::shared_ptr<TModelMesh>> cMeshes;
		cMeshes.push_back(std::make_shared<TModelMesh>());
		auto cMesh = cMeshes.back();
		for(unsigned int j=0;j<sides.size();j++)
		{
			auto cSubMesh = std::make_shared<TModelSubMesh>();
			auto &side = sides[j];
			if(bConvex == side->IsConvex())
			{
				auto &vertices = cSubMesh->GetVertices();
				auto &alphas = cSubMesh->GetAlphas();
				auto &triangles = cSubMesh->GetTriangles();

				Material *mat = side->GetMaterial();
				auto surfaceMaterial = -1;
				if(mat != NULL)
				{
					std::unordered_map<Material*,unsigned int>::iterator it = textures.find(mat);
					if(it != textures.end())
						cSubMesh->SetTexture(it->second);
					auto &data = mat->GetDataBlock();
					if(data != nullptr)
					{
						std::string surfaceIdentifier;
						if(data->GetString("surfacematerial",&surfaceIdentifier) == true)
						{
							auto it = std::find_if(surfaceMaterials.begin(),surfaceMaterials.end(),[&surfaceIdentifier](const SurfaceMaterial &mat) {
								return (mat.GetIdentifier() == surfaceIdentifier) ? true : false;
							});
							if(it != surfaceMaterials.end())
								surfaceMaterial = static_cast<int>(it->GetIndex());
						}
					}
				}
				if(bConvex == true && surfaceMaterial != -1) // Determine most used surface material if this is a convex mesh
				{
					auto itInfo = std::find_if(surfaceMaterialInfo.begin(),surfaceMaterialInfo.end(),[surfaceMaterial](SurfaceMaterialInfo &info) {
						return (info.surfaceMaterialId == surfaceMaterial) ? true : false;
					});
					if(itInfo == surfaceMaterialInfo.end())
					{
						surfaceMaterialInfo.push_back(SurfaceMaterialInfo(surfaceMaterial));
						itInfo = surfaceMaterialInfo.end() -1;
					}
					itInfo->count++;
					if(itInfo->count > maxSurfaceMaterialCount)
					{
						maxSurfaceMaterialCount = itInfo->count;
						surfaceMaterialId = surfaceMaterial;
					}
				}

				auto &vertsSide = side->GetVertices();
				auto &normalsSide = side->GetNormals();
				auto &trianglesSide = *side->GetTriangles();
				auto &uvsSide = side->GetUVMapping();
				auto &alphasSide = side->GetAlphas();
				auto colVertOffset = colVerts.size();
				if(bConvex == false)
				{
					if(surfaceMaterial == -1)
						surfaceMaterial = 0;
					colVerts.reserve(colVerts.size() +trianglesSide.size());
					colMaterials.reserve(colMaterials.size() +trianglesSide.size());
					for(auto it=trianglesSide.begin();it!=trianglesSide.end();++it)
						colVerts.push_back(vertsSide[*it]);

					colTriangles.reserve(colTriangles.size() +trianglesSide.size());
					for(auto i=decltype(trianglesSide.size()){0};i<trianglesSide.size();++i)
						colTriangles.push_back(colVertOffset +i);

					colMaterials.reserve(colMaterials.size() +trianglesSide.size() /3);
					for(UInt i=0;i<trianglesSide.size();i+=3)
						colMaterials.push_back(surfaceMaterial);
				}
				else
					colVerts.reserve(colVerts.size() +vertsSide.size());
				auto numAlpha = side->GetAlphaCount();
				for(unsigned int k=0;k<vertsSide.size();k++)
				{
					if(bConvex == true)
						colVerts.push_back(vertsSide[k]);
					vertices.push_back(Vertex{
						vertsSide[k],uvsSide[k],normalsSide[k]
					});
					if(numAlpha > 0)
						alphas.push_back(alphasSide[k]);
				}
				cSubMesh->SetAlphaCount(numAlpha);
				if(bConvex == true)
				{
					colTriangles.reserve(colTriangles.size() +trianglesSide.size());
					for(auto i=decltype(trianglesSide.size()){0};i<trianglesSide.size();++i)
						colTriangles.push_back(colVertOffset +trianglesSide.at(i));
				}
				for(unsigned int k=0;k<trianglesSide.size();k++)
					triangles.push_back(trianglesSide[k]);
				cMesh->AddSubMesh(cSubMesh);

				uint32_t lod = 1;
				auto *trianglesLod = side->GetTriangles(lod);
				while(trianglesLod != nullptr)
				{
					auto lodMesh = std::make_shared<TModelSubMesh>(*cSubMesh);
					lodMesh->ClearTriangles();
					auto &triangles = lodMesh->GetTriangles();
					for(unsigned int k=0;k<trianglesLod->size();k++)
						triangles.push_back((*trianglesLod)[k]);
					trianglesLod = side->GetTriangles(lod);

					if(lod >= cMeshes.size())
						cMeshes.push_back(std::make_shared<TModelMesh>());
					auto cMesh = cMeshes.back();
					cMesh->AddSubMesh(lodMesh);
					++lod;
				}
			}
		}
		auto bHasLods = (cMeshes.size() > 1) ? true : false;
		auto numMeshes = cMeshes.size();
		if(numMeshes -1 > lods.size())
			lods.resize(numMeshes -1);
		auto &baseMeshes = mdl->GetBaseMeshes();
		auto baseMeshId = baseMeshes.size();
		for(auto i=decltype(numMeshes){0};i<numMeshes;++i)
		{
			auto &cMesh = cMeshes[i];
			if(bConvex == true)
				colMesh->SetSurfaceMaterial(surfaceMaterialId); // Global surface material for entire mesh
			//cMesh->Update(true);
			if(bHasLods == false)
				group->AddMesh(cMesh);
			else
			{
				auto numGroups = mdl->GetMeshGroupCount();
				auto lodGroup = mdl->AddMeshGroup("displacement" +std::to_string(baseMeshId) +"_lod" +std::to_string(i));
				lodGroup->AddMesh(cMesh);

				if(i > 0)
					lods[static_cast<uint32_t>(i -1)][static_cast<uint32_t>(baseMeshId)] = numGroups;
				else
				{
					baseMeshes.push_back(numGroups);
					baseMeshId = numGroups;
				}
			}
		}
		colMesh->CalculateBounds();
		colMesh->CalculateVolumeAndCom();
		mdl->AddCollisionMesh(colMesh);
	}
	for(auto i=decltype(lods.size()){0};i<lods.size();++i)
		mdl->AddLODInfo(static_cast<uint32_t>(i *WORLD_LOD_OFFSET +WORLD_BASE_LOD),lods[i]);
	if(meshes.empty())
	{
		min = Vector3(0.f,0.f,0.f);
		max = Vector3(0.f,0.f,0.f);
	}
	mdl->SetRenderBounds(min,max);
	mdl->SetCollisionBounds(min,max);
	mdl->UpdateShape(&surfaceMaterials);
	return mdl;
}

#endif
