/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __WMD_H__
#define __WMD_H__
#include "pragma/networkdefinitions.h"
#include "pragma/file_formats/wdf.h"
#include "pragma/model/model.h"
#include <fsys/filesystem.h>
#include "materialmanager.h"
#include "pragma/file_formats/wad.h"

// Note: Changing this version will directly affect model saving as well as loading, both have to be updated accordingly
#define WMD_VERSION 37

enum class CollisionMeshLoadFlags : uint64_t
{
	None = 0u,
	SoftBody = 1u,
	Convex = SoftBody<<1u
};
REGISTER_BASIC_BITWISE_OPERATORS(CollisionMeshLoadFlags)

struct DLLNETWORK FWMDBone
{
	std::string name;
	std::unordered_map<unsigned int,FWMDBone*> children;
};

struct DLLNETWORK FWMDSkeleton
{
	std::vector<FWMDBone> bones;
	std::unordered_map<unsigned int,FWMDBone*> hierarchy;
};

struct DLLNETWORK FWMDFaceVertex
{
	unsigned long long vertexID;
	Vector3 uvw = {};
};

struct DLLNETWORK FWMDFace
{
	FWMDFaceVertex verts[3];
	unsigned int matID;
};

struct DLLNETWORK FWMDTexture
{
	FWMDTexture() {};
	FWMDTexture(unsigned int ID,std::string name)
	{
		this->ID = ID;
		this->name = name;
	}
	unsigned int ID;
	std::string name;
};

struct DLLNETWORK FWMDMaterial
{
	FWMDMaterial(FWMDTexture diffusemap,FWMDTexture bumpmap)
	{
		this->diffusemap = diffusemap;
		this->bumpmap = bumpmap;
	}
	FWMDTexture diffusemap;
	FWMDTexture bumpmap;
};

struct DLLNETWORK FWMDMesh
{
	FWMDMesh(FWMDTexture diffusemap,FWMDTexture bumpmap) : material(diffusemap,bumpmap) {}
	std::vector<Vector3> vertexList;
	std::vector<FWMDFace> faces;
	FWMDMaterial material;
};

struct DLLNETWORK FWMDVertex
{
	Vector3 position = {};
	Vector3 normal = {};
	std::unordered_map<unsigned long long,float> weights;
};

class DLLNETWORK FWMD
	: FWDF
{
private:
	bool m_bStatic;
	Game *m_gameState;
	void ReadChildBones(const Skeleton &skeleton,std::shared_ptr<Bone> bone);
	std::vector<FWMDMesh*> m_meshes;

	void LoadBones(unsigned short version,unsigned int numBones,Model &mdl);
	void LoadAttachments(Model &mdl);
	void LoadObjectAttachments(Model &mdl);
	void LoadHitboxes(uint16_t version,Model &mdl);
	void LoadMeshes(unsigned short version,Model &mdl,const std::function<std::shared_ptr<ModelMesh>()> &meshFactory,const std::function<std::shared_ptr<ModelSubMesh>()> &subMeshFactory);
	void LoadLODData(unsigned short version,Model &mdl);
	void LoadBodygroups(Model &mdl);
	void LoadSoftBodyData(Model &mdl,CollisionMesh &colMesh);
	void LoadCollisionMeshes(Game *game,unsigned short version,Model &mdl,SurfaceMaterial *smDefault=nullptr);
	void LoadBlendControllers(Model &mdl);
	void LoadIKControllers(uint16_t version,Model &mdl);
	void LoadAnimations(unsigned short version,Model &mdl);
public:
	FWMD(Game *game);
	~FWMD();
	template<class TModel,class TModelMesh,class TModelSubMesh>
		std::shared_ptr<Model> Load(Game *game,const std::string &model,const std::function<Material*(const std::string&,bool)> &loadMaterial,const std::function<std::shared_ptr<Model>(const std::string&)> &loadModel);
	void GetMeshes(std::vector<FWMDMesh*> **meshes);
};

#endif