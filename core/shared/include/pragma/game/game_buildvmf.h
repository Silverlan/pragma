#ifndef __GAME_BUILDVMF_H__
#define __GAME_BUILDVMF_H__
#include <pragma/game/game.h>
#include "pragma/encryption/md5.h"
#include "forgedata.h"
#include <pragma/util/datafile.h>

template<class TWorld,class TPolyMesh,class TPoly,class TBrushMesh>
	void Game::BuildVMF(const char *map)
{
	BaseWorld *bWrld = GetWorld();
	if(bWrld == NULL)
		return;
	std::string path = "maps\\";
	path += map;
	path += ".vmf";
	const char *cPath = path.c_str();
	Con::cout<<"Loading vmf '"<<map<<"'..."<<Con::endl;
	VFilePtr *f = FileManager::OpenFile(cPath,"rb");
	if(f == NULL)
	{
		Con::cwar<<"WARNING: Unable to open vmf '"<<map<<"'!"<<Con::endl;
		return;
	}

	// Calculating MD5
	char *buffer;
	size_t result;
	unsigned long long lSize = f->GetSize();
	buffer = (char*) malloc(sizeof(char) *lSize);
	if(buffer == NULL)
	{
		Con::cwar<<"WARNING: Unable to read vmf '"<<map<<"': Not enough memory."<<Con::endl;
		return;
	}
	result = f->Read(buffer,lSize);
	if(result != lSize)
	{
		Con::cwar<<"WARNING: Unable to read vmf '"<<map<<"': Reading error."<<Con::endl;
		return;
	}
	std::string md5 = MD5(std::string(buffer,lSize)).hexdigest();
	delete buffer;
	//
	f->Seek(0);

	DataFileBlock *vmf = DataFile::ReadBlock(f);
	if(vmf == NULL)
	{
		Con::cwar<<"WARNING: Unable to read vmf '"<<map<<"'!"<<Con::endl;
		return;
	}
	std::unordered_map<std::string,std::vector<DataFileBlock*>*>::iterator i = vmf->blocks.find("world");
	if(i == vmf->blocks.end())
	{
		Con::cout<<"WARNING: Unable to find block 'world' in vmf '"<<map<<"'!"<<Con::endl;
		FileManager::CloseFile(f);
		delete vmf;
		return;
	}
	m_mapInfo.md5 = md5;
	m_mapInfo.name = map;
	std::vector<DataFileBlock*> *blocks = i->second;
	DataFileBlock *world = (*blocks)[0];

	std::unordered_map<std::string,std::vector<DataFileBlock*>*>::iterator j = world->blocks.find("solid");
	if(j == world->blocks.end())
	{
		Con::cout<<"WANRING: Unable to parse world from vmf '"<<map<<"': No solids!"<<Con::endl;
		FileManager::CloseFile(f);
		delete vmf;
		return;
	}
	std::vector<DataFileBlock*> *solids = j->second;
	std::vector<TPolyMesh*> meshes;
	for(int k=0;k<solids->size();k++)
	{
		DataFileBlock *solid = (*solids)[k];
		std::unordered_map<std::string,std::vector<DataFileBlock*>*>::iterator l = solid->blocks.find("side");
		if(l == solid->blocks.end())
			Con::cout<<"WARNING: Solid with no sides! Skipping..."<<Con::endl;
		else
		{
			TPolyMesh *mesh = new TPolyMesh;
			std::vector<DataFileBlock*> *sides = l->second;
			for(int m=0;m<sides->size();m++)
			{
				DataFileBlock *side = (*sides)[m];
				std::string material = side->keyvalues["material"];
				std::string splane = side->keyvalues["plane"];
				std::string suaxis = side->keyvalues["uaxis"];
				std::string svaxis = side->keyvalues["vaxis"];
				std::string srot = side->keyvalues["rotation"];
				std::string slmapscale = side->keyvalues["lightmapscale"];
				std::string smoothinggroups = side->keyvalues["smoothing_groups"];

				int ufound = suaxis.find(']');
				int vfound = svaxis.find(']');
				if(ufound != -1 && vfound != -1)
				{
					std::string u = suaxis.substr(1,ufound -1);
					std::string v = svaxis.substr(1,vfound -1);

					std::vector<std::string> vu;
					std::vector<std::string> vv;
					SplitString(u,&vu);
					SplitString(v,&vv);
					if(vu.size() == 4 && vv.size() == 4)
					{
						Vector3 plane[3] = {{},{},{}};
						int st = splane.find_first_of('(');
						int en;
						for(int n=0;n<3;n++)
						{
							en = splane.find(')',st +1);
							std::vector<std::string> vec;
							SplitString(splane.substr(st +1,en -1),&vec);
							plane[n].x = atof(vec[0].c_str());
							plane[n].y = atof(vec[2].c_str());
							plane[n].z = atof(vec[1].c_str());
							st = splane.find('(',en +1);
						}
						float su = atof(suaxis.substr(ufound +1).c_str());
						float sv = atof(svaxis.substr(vfound +1).c_str());
						TPoly *poly = new TPoly;
						poly->SetTextureData(
							material,
							Vector3(atof(vu[0].c_str()),atof(vu[2].c_str()),atof(vu[1].c_str())),
							Vector3(atof(vv[0].c_str()),atof(vv[2].c_str()),atof(vv[1].c_str())),
							atof(vu[3].c_str()),atof(vv[3].c_str()),
							su,sv,
							atof(srot.c_str())
						);
						Vector3 normal = glm::normalize(glm::cross(plane[2] -plane[0],plane[1] -plane[2]));
						double d = -(glm::dot(normal,plane[0]));
						poly->SetNormal(normal);
						poly->SetDistance(d);
						mesh->AddPoly(poly);
					}
					else
						Con::cout<<"WANRING: Side with invalid texture information. Skipping..."<<Con::endl;
				}
			}
			mesh->BuildPolyMesh();
			mesh->Calculate();
			if(mesh->GetPolyCount() > 0)
				meshes.push_back(mesh);
			else
				Con::cout<<"WARNING: Unable to create brush: Mesh without polys("<<*mesh<<")"<<Con::endl;
		}
	}
	FileManager::CloseFile(f);
	delete vmf;
	std::vector<TBrushMesh*> brushMeshes;
	TPolyMesh::GenerateBrushMeshes(&brushMeshes,meshes);
	
	TWorld *wrld = static_cast<TWorld*>(bWrld);
	auto &entWorld = wrld->GetEntity();
	Vector3 min(0,0,0);
	Vector3 max(0,0,0);
	for(int i=0;i<brushMeshes.size();i++)
	{
		entWorld.AddBrushMesh(brushMeshes[i]);
		Vector3 minMesh = {};
		Vector3 maxMesh = {};
		brushMeshes[i]->GetBounds(&minMesh,&maxMesh);
		if(i == 0)
		{
			min = minMesh;
			max = maxMesh;
		}
		else
		{
			Vector3::min(&min,minMesh);
			Vector3::max(&max,maxMesh);
		}
	}
	entWorld.SetCollisionBounds(min,max);
	wrld->SetBrushes(meshes);
	entWorld.InitializePhysics(PHYSICSTYPE::STATIC);
	//wrld->m_brushes = new std::vector<PolyMesh*>(meshes);
	Con::cout<<"Successfully loaded map '"<<map<<"'!"<<Con::endl;
}

#endif