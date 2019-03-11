#include "fbx.h"
#include "fbxfile.h"
#include "wv_source.hpp"
#include <pragma/networkstate/networkstate.h>
#include <sharedutils/util_file.h>
#include <string>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>

#pragma comment(lib,"libfbxsdk-md.lib")

static bool get_diffuse_file_name(FbxSurfaceMaterial *material,std::string &fileName)
{
	auto prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
	auto layeredTextureCount = prop.GetSrcObjectCount<FbxLayeredTexture>();
	if(layeredTextureCount > 0)
	{
		for(auto i=decltype(layeredTextureCount){0};i<layeredTextureCount;++i)
		{
			auto *layeredTexture = FbxCast<FbxLayeredTexture>(prop.GetSrcObject<FbxLayeredTexture>(i));
			auto lcount = layeredTexture->GetSrcObjectCount<FbxFileTexture>();
			for(auto j=decltype(lcount){0};j<lcount;++j)
			{
				auto *texture = FbxCast<FbxFileTexture>(layeredTexture->GetSrcObject<FbxFileTexture>(j));
				auto *textureName = texture->GetFileName();
				fileName = ufile::get_file_from_filename(textureName);
				return true;
			}
		}
	}
	else
	{
		auto textureCount = prop.GetSrcObjectCount<FbxFileTexture>();
		for(auto i=decltype(textureCount){0};i<textureCount;++i)
		{
			auto *texture = FbxCast<FbxFileTexture>(prop.GetSrcObject<FbxFileTexture>(i));
			auto *textureName = texture->GetFileName();
			fileName = ufile::get_file_from_filename(textureName);
			return true;
		}
	}
	return false;
}

/*
for (int i = 0; i < lScene->GetSrcObjectCount(FbxMesh::ClassId); i++)
{
FbxMesh* lMesh = (FbxMesh*)lScene->GetSrcObject(FbxMesh::ClassId, i);
FbxLayerElementUV* lUVs = lMesh->GetLayer(0)->GetUVs();
ConvertMapping(lUVs, lMesh);
}

void ConvertMapping(FbxLayerElementUV* pUVs, FbxMesh* pMesh)
{
if (pUVs->GetMappingMode() == FbxLayerElement::eByPolygonVertex)
{
if (pUVs->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
{
FbxArray<int> lNewIndexToDirect;
FbxArray<FbxVector2> lNewUVs;

// keep track of the processed control points
FbxSet lProcessedCP(pMesh->GetControlPointsCount());
for (int i = 0; i < pMesh->GetControlPointsCount(); i++)
lProcessedCP.Add((FbxHandle)i, (FbxHandle)false);

// visit each polygon and polygon vertex
for (int p = 0; p < pMesh->GetPolygonCount(); p++)
{
for (int pv = 0; pv < pMesh->GetPolygonSize(p); pv++)
{
int lCP = pMesh->GetPolygonVertex(p, pv);

// check if we already processed this control point
if (lProcessedCP.Get((FbxHandle)lCP) == 0)
{
FbxVector2 uv = pUVs->GetDirectArray().GetAt(lCP);
lNewUVs.Add(uv);
lNewIndexToDirect.Add(lCP); 
lProcessedCP.SetItem((FbxHandle)lCP, (FbxHandle)true);
}
}
}

// change the content of the index array and its mapping
pUVs->SetMappingMode(FbxLayerElement::eByControlPoint);
pUVs->GetIndexArray().Clear();
pUVs->GetIndexArray().Resize(lNewIndexToDirect.GetCount());
int* lIndexArray = (int*)pUVs->GetIndexArray().GetLocked();
for (int i = 0; i < lNewIndexToDirect.GetCount(); i++)
lIndexArray = lNewIndexToDirect.GetAt(i);
pUVs->GetIndexArray().Release((void**)&lIndexArray);

// and the content of the direct array
pUVs->GetDirectArray().Clear();
pUVs->GetDirectArray().Resize(lNewUVs.GetCount());
FbxVector2* lDirectArray = (FbxVector2*)pUVs->GetDirectArray().GetLocked();
for (int j = 0; j < lNewUVs.GetCount(); j++)
lDirectArray = lNewUVs.GetAt(j);
pUVs->GetDirectArray().Release((void**)&lDirectArray);
}
}
}
*/

bool import::load_fbx(NetworkState *nw,Model &mdl,VFilePtr &f,std::vector<std::string> &textures)
{
	auto fbxFile = FBXFile(f);

	auto *manager = FbxManager::Create();
	auto *ios = FbxIOSettings::Create(manager,IOSROOT);
	manager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	auto *importer = FbxImporter::Create(manager,"");
    
	// Use the first argument as the filename for the importer.
	if(!importer->Initialize(static_cast<fbxsdk::FbxStream*>(&fbxFile),nullptr,-1,manager->GetIOSettings()))
		return false;

	// Create a new scene so that it can be populated by the imported file.
	auto *scene = FbxScene::Create(manager,"FBXImportScene");

	// Import the contents of the file into the scene.
	importer->Import(scene);

	// The file is imported, so get rid of the importer.
	importer->Destroy();

	auto *rootNode = scene->GetRootNode();
	if(rootNode != nullptr)
	{
		auto numChildren = rootNode->GetChildCount();
		auto mdlMesh = std::shared_ptr<ModelMesh>(nw->CreateMesh());
		for(auto i=decltype(numChildren){0};i<numChildren;++i)
		{
			auto *child = rootNode->GetChild(i);
			auto *attr = child->GetNodeAttribute();
			if(attr != nullptr && attr->GetAttributeType() == fbxsdk::FbxNodeAttribute::EType::eMesh)
			{
				auto subMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
				auto name = child->GetNameOnly();
				auto *mesh = static_cast<FbxMesh*>(attr);
				fbxsdk::FbxStringList uvSets;
				mesh->GetUVSetNames(uvSets);
				auto uvSetName = (uvSets.GetCount() > 0) ? uvSets[0] : fbxsdk::FbxString{};

				auto *verts = mesh->GetControlPoints();
				auto &meshVerts = subMesh->GetVertices();
				auto &meshTriangles = subMesh->GetTriangles();
				auto numVerts = mesh->GetControlPointsCount();
				meshVerts.reserve(numVerts);
				std::unordered_map<uint32_t,bool> vertsInitialized;
				for(auto i=decltype(numVerts){0};i<numVerts;++i)
				{
					auto &v = verts[i];
					meshVerts.push_back({{v[0],v[1],v[2]},{},{}});
					vertsInitialized[meshVerts.size() -1] = false;
				}
				auto numPolys = mesh->GetPolygonCount();
				for(auto i=decltype(numPolys){0};i<numPolys;++i)
				{
					auto numVerts = mesh->GetPolygonSize(i);
					assert(numVerts == 3);
					meshTriangles.reserve(numVerts);
					for(auto j=decltype(numVerts){0};j<numVerts;++j)
					{
						auto idx = mesh->GetPolygonVertex(i,j);
						if(vertsInitialized[idx] == false)
						{
							fbxsdk::FbxVector4 n;
							mesh->GetPolygonVertexNormal(i,j,n);
							fbxsdk::FbxVector2 uv;
							auto bUnmapped = false;
							mesh->GetPolygonVertexUV(i,j,uvSetName,uv,bUnmapped);

							auto &v = meshVerts[idx];
							v.normal = {n[0],n[1],n[2]};
							v.uv = {uv[0],uv[1]};
							for(uint8_t i=0;i<2;++i)
							{
								if(v.uv[i] < 0.f || v.uv[i] > 1.f)
								{
									v.uv[i] = 0.5f;
									continue;
								}
								if(v.uv[i] < 0.f)
									v.uv[i] = 1.f -fmodf(-v.uv[i],1.f);
								else
									v.uv[i] = fmodf(v.uv[i],1.f);
							}
							v.uv[1] = 1.f -v.uv[1];
							vertsInitialized[idx] = (bUnmapped == false) ? true : false;
						}
						meshTriangles.push_back(idx);
					}
				}
				auto numMats = child->GetMaterialCount();
				for(auto i=decltype(numMats){0};i<numMats;++i)
				{
					auto *mat = child->GetMaterial(i);
					std::string texture;
					if(get_diffuse_file_name(mat,texture) == true)
					{
						textures.push_back(texture);
						auto texId = import::util::add_texture(*nw,mdl,texture);
						subMesh->SetTexture(texId);
						break;
					}
				}
				subMesh->Update();
				mdlMesh->AddSubMesh(subMesh);
			}
		}
		mdlMesh->Update();
		mdl.GetMeshGroup(0)->AddMesh(mdlMesh);
	}

	/*auto numStacks = scene->GetSrcObjectCount<fbxsdk::FbxAnimStack>();
	for(auto i=decltype(numStacks){0};i<numStacks;++i)
	{
		auto *pAnimStack = FbxCast<FbxAnimStack>(scene->GetSrcObject<FbxAnimStack>(i));
		auto numAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
		auto *lAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(i);
		auto* lAnimCurve = lNode->LclTranslation.GetCurve(lAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	}*/

	// Destroy the SDK manager and all the other objects it was handling.
	manager->Destroy();
	return true;
}