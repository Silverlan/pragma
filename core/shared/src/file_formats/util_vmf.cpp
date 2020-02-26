#include "stdafx_shared.h"
#include "pragma/file_formats/util_vmf.hpp"
#include "pragma/model/poly.h"
#include "pragma/model/polymesh.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/level/level_info.hpp"
#include <sharedutils/util_file.h>
#include <util_fgd.hpp>
#include <pragma/math/intersection.h>
#include <unordered_set>

static const double EPSILON = 4.9406564584125e-2;//4.9406564584125e-4;//4.94065645841247e-324;
static void build_poly_mesh_info(std::shared_ptr<PolyMesh> mesh,const std::function<void(const std::string&)> &messageLogger)
{
	static std::vector<std::string> matMissing;
	auto bDisp = mesh->HasDisplacements();
	auto &polys = mesh->GetPolys();
	auto &info = mesh->GetCompiledData();
	for(auto it=polys.begin();it!=polys.end();++it)
	{
		auto &poly = *it;
		auto &polyInfo = poly->GetCompiledData();
		if(bDisp == false || poly->IsDisplacement())
		{
			auto &polyVerts = poly->GetVertices();
			unsigned int numPolyVerts = static_cast<unsigned int>(polyVerts.size());
			for(unsigned int k=0;k<numPolyVerts;k++)
			{
				glm::vec3 &va = polyVerts[k].pos;
				bool bExists = false;
				for(unsigned int l=0;l<info.vertexList.size();l++)
				{
					glm::vec3 &vb = info.vertexList[l];
					if(fabsf(va.x -vb.x) <= EPSILON &&
						fabsf(va.y -vb.y) <= EPSILON &&
						fabsf(va.z -vb.z) <= EPSILON)
					{
						bExists = true;
						break;
					}
				}
				if(bExists == false)
					info.vertexList.push_back(va);
			}
		}
		auto *texData = poly->GetTextureData();
		auto *mat = poly->GetMaterial();
		auto *texInfo = (mat != nullptr) ? mat->GetDiffuseMap() : nullptr;
		if(texInfo == nullptr && mat != nullptr)
		{
			// Find first texture in material
			std::function<bool(const std::shared_ptr<ds::Block>&)> fFindTexture = nullptr;
			fFindTexture = [&texInfo,&fFindTexture](const std::shared_ptr<ds::Block> &block) -> bool {
				auto *data = block->GetData();
				if(data == nullptr)
					return false;
				for(auto &pair : *data)
				{
					if(pair.second->IsBlock())
					{
						if(fFindTexture(std::static_pointer_cast<ds::Block>(pair.second)) == true)
							return true;
						continue;
					}
					if(dynamic_cast<ds::Texture*>(pair.second.get()) != nullptr)
					{
						texInfo = &static_cast<ds::Texture*>(pair.second.get())->GetValue();
						return true;
					}
				}
				return false;
			};
			fFindTexture(mat->GetDataBlock());
		}
		if(texInfo == nullptr)
		{
			auto bExists = false;
			for(unsigned int i=0;i<matMissing.size();i++)
			{
				if(matMissing[i] == texData->texture)
				{
					bExists = true;
					break;
				}
			}
			if(bExists == false)
			{
				matMissing.push_back(texData->texture);
				if(messageLogger != nullptr)
				{
					std::stringstream ss;
					ss<<"WARNING: Unable to find material '"<<texData->texture<<"'! Any meshes with this material may be rendered incorrectly!";
					messageLogger(ss.str());
				}
			}
			polyInfo.nu = glm::vec3(0,0,0);
			polyInfo.nv = glm::vec3(0,0,0);
			polyInfo.width = 512.f;
			polyInfo.height = 512.f;
		}
		else
		{
			polyInfo.nu = texData->nu;
			polyInfo.nv = texData->nv;
			polyInfo.width = static_cast<float>(texInfo->width);
			polyInfo.height = static_cast<float>(texInfo->height);
		}

		// Displacements
		if(poly->IsDisplacement())
		{
			auto disp = poly->GetDisplacement();
			auto &vertices = poly->GetVertices();
			std::vector<glm::vec3> sideVerts;
			sideVerts.reserve(vertices.size());
			for(auto it=vertices.begin();it!=vertices.end();++it)
				sideVerts.push_back(it->pos);
			auto &dispInfo = *(polyInfo.displacement = std::make_unique<PolyDispInfo>()).get();
			dispInfo.power = disp->power;
			poly->BuildDisplacement(sideVerts,polyInfo.nu,polyInfo.nv,polyInfo.width,polyInfo.height,
				dispInfo.vertices,dispInfo.uvs,dispInfo.triangles,dispInfo.faceNormals,dispInfo.numAlpha,&dispInfo.alphas
			);
		}
	}
}

#define NEIGHBOR_THIS 0
#define NEIGHBOR_TOP_LEFT 1
#define NEIGHBOR_TOP 2
#define NEIGHBOR_TOP_RIGHT 3
#define NEIGHBOR_LEFT 4
#define NEIGHBOR_RIGHT 5
#define NEIGHBOR_BOTTOM_LEFT 6
#define NEIGHBOR_BOTTOM 7
#define NEIGHBOR_BOTTOM_RIGHT 8

static std::vector<glm::vec3> get_sorted_vertices(Poly &poly)
{
	auto &polyVerts = poly.GetVertices();
	auto disp = poly.GetDisplacement();
	auto &startPos = disp->startposition;
	std::vector<glm::vec3> sortedPolyVerts;
	sortedPolyVerts.reserve(polyVerts.size());
	auto start = -1;
	auto idx = 0;
	for(auto it=polyVerts.begin();it!=polyVerts.end();++it)
	{
		auto &v = it->pos;
		if(fabsf(startPos.x -v.x) <= EPSILON &&
				fabsf(startPos.y -v.y) <= EPSILON &&
				fabsf(startPos.z -v.z) <= EPSILON)
		{
			start = idx;
			break;
		}
		idx++;
	}
	assert(start != -1);
	for(auto it=polyVerts.begin() +start;it!=polyVerts.end();++it)
		sortedPolyVerts.push_back(it->pos);
	for(auto it=polyVerts.begin();it!=polyVerts.begin() +start;++it)
		sortedPolyVerts.push_back(it->pos);
	return sortedPolyVerts;
}

static void find_neighbor_normals(glm::vec3 &v,int rows,std::vector<glm::vec3> &verts,std::vector<unsigned int> &triangles,std::vector<glm::vec3> &faceNormals,std::vector<glm::vec3> &neighborNormals)
{
	for(auto col=0;col<(rows -1);col++)
	{
		for(auto row=0;row<(rows -1);row+=((col > 0 && col < (rows -2)) ? (rows -2) : 1))
		{
			unsigned int idx = col *(rows -1) *6 +row *6;
			unsigned int faceId = col *(rows -1) *2 +row *2;
			auto vertAId1 = triangles[idx];
			auto vertAId2 = triangles[idx +1];
			auto vertAId3 = triangles[idx +2];
			auto &nA = faceNormals[faceId];
			idx += 3;
			faceId++;

			auto &va1 = verts[vertAId1];
			auto &va2 = verts[vertAId2];
			auto &va3 = verts[vertAId3];
			auto hit = 0;
			if(
				(fabsf(va1.x -v.x) <= EPSILON && fabsf(va1.y -v.y) <= EPSILON && fabsf(va1.z -v.z) <= EPSILON) ||
				(fabsf(va2.x -v.x) <= EPSILON && fabsf(va2.y -v.y) <= EPSILON && fabsf(va2.z -v.z) <= EPSILON) ||
				(fabsf(va3.x -v.x) <= EPSILON && fabsf(va3.y -v.y) <= EPSILON && fabsf(va3.z -v.z) <= EPSILON)
			)
				hit |= 1;
			auto vertBId1 = triangles[idx];
			auto vertBId2 = triangles[idx +1];
			auto vertBId3 = triangles[idx +2];
			auto &nB = faceNormals[faceId];

			auto &vb1 = verts[vertBId1];
			auto &vb2 = verts[vertBId2];
			auto &vb3 = verts[vertBId3];
			if(
				(fabsf(vb1.x -v.x) <= EPSILON && fabsf(vb1.y -v.y) <= EPSILON && fabsf(vb1.z -v.z) <= EPSILON) ||
				(fabsf(vb2.x -v.x) <= EPSILON && fabsf(vb2.y -v.y) <= EPSILON && fabsf(vb2.z -v.z) <= EPSILON) ||
				(fabsf(vb3.x -v.x) <= EPSILON && fabsf(vb3.y -v.y) <= EPSILON && fabsf(vb3.z -v.z) <= EPSILON)
			)
				hit |= 2;
			if(hit > 0)
			{
				if(hit &1)
					neighborNormals.push_back(nA);
				if(hit &2)
					neighborNormals.push_back(nB);
				if(hit < 3)
					neighborNormals.push_back(neighborNormals.back()); // Weigh it double
			}
		}
	}
}

static void build_displacement_normals(std::vector<std::shared_ptr<PolyMesh>> &meshes,PolyMesh *mesh,Poly *poly,PolyInfo &info,std::vector<glm::vec3> &outNormals)
{
	auto &displacement = *info.displacement.get();
	auto &faceNormals = displacement.faceNormals;
	auto rows = umath::pow(2,CInt32(displacement.power)) +1;
	auto numVerts = rows *rows;
	outNormals.resize(numVerts);

	std::array<PolyDispInfo*,9> neighbors = {&displacement,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};
	auto &polyVerts = poly->GetVertices();
	assert(polyVerts.size() == 4);

	// Debug
	auto sortedPolyVerts = get_sorted_vertices(*poly);
	auto &disp = *poly->GetDisplacement();
	for(auto it=meshes.begin();it!=meshes.end();++it)
	{
		auto &polyMesh = *it;
		if(polyMesh->HasDisplacements())
		{
			auto &meshInfo = polyMesh->GetCompiledData();
			auto &polys = polyMesh->GetPolys();
			for(auto it=polys.begin();it!=polys.end();++it)
			{
				auto &polyOther = *it;
				if(polyOther->IsDisplacement() && polyOther.get() != poly)
				{
					auto &dispOther = *polyOther->GetDisplacement();
					if(dispOther.power == disp.power)
					{
						auto &otherPolyVerts = polyOther->GetVertices();
						assert(otherPolyVerts.size() == 4);
						auto touching = 0;
						auto numTouching = 0;
						for(auto it=otherPolyVerts.begin();it!=otherPolyVerts.end();++it)
						{
							auto &vOther = it->pos;
							auto idx = 0;
							for(auto it=sortedPolyVerts.begin();it!=sortedPolyVerts.end();++it)
							{
								auto &v = *it;
								if(fabsf(vOther.x -v.x) <= EPSILON &&
										fabsf(vOther.y -v.y) <= EPSILON &&
										fabsf(vOther.z -v.z) <= EPSILON)
								{
									numTouching++;
									touching |= 1<<idx;
								}
								idx++;
							}
						}
						if(touching > 0)
						{
							if(numTouching < 3) // Else displacement is overlapping; No smoothing possible
							{
								auto &polyInfo = polyOther->GetCompiledData();
								auto &dispInfo = *polyInfo.displacement.get();
								if(touching == 1)
									neighbors[NEIGHBOR_TOP_RIGHT] = &dispInfo;
								else if(touching == 3)
									neighbors[NEIGHBOR_TOP] = &dispInfo;
								else if(touching == 2)
									neighbors[NEIGHBOR_TOP_LEFT] = &dispInfo;
								else if(touching == 6)
									neighbors[NEIGHBOR_LEFT] = &dispInfo;
								else if(touching == 4)
									neighbors[NEIGHBOR_BOTTOM_LEFT] = &dispInfo;
								else if(touching == 12)
									neighbors[NEIGHBOR_BOTTOM] = &dispInfo;
								else if(touching == 8)
									neighbors[NEIGHBOR_BOTTOM_RIGHT] = &dispInfo;
								else if(touching == 9)
									neighbors[NEIGHBOR_RIGHT] = &dispInfo;
							}
							break;
						}
					}
				}
			}
		}
	}

	// Calculate Vertex Normals
	for(auto col=0;col<rows;col++)
	{
		for(auto row=0;row<rows;row++)
		{
			auto vertId = col *rows +row;
			std::vector<glm::vec3> neighborNormals;
			neighborNormals.reserve(8);
			if(col > 0 && row > 0 && col < (rows -1) && row < (rows -1))
			{
				auto a = (col -1) *(rows -1) +(row -1);
				auto b = col *(rows -1) +(row -1);
				auto c = (col -1) *(rows -1) +row;
				auto d = col *(rows -1) +row;
				neighborNormals.push_back(faceNormals[a *2 +1]);
				neighborNormals.push_back(neighborNormals.back());
				
				neighborNormals.push_back(faceNormals[b *2]);
				neighborNormals.push_back(faceNormals[b *2 +1]);
				
				neighborNormals.push_back(faceNormals[c *2]);
				neighborNormals.push_back(faceNormals[c *2 +1]);
				
				neighborNormals.push_back(faceNormals[d *2]);
				neighborNormals.push_back(neighborNormals.back());
			}
			else
			{
				auto &v = displacement.vertices[vertId];
				for(auto it=neighbors.begin();it!=neighbors.end();++it)
				{
					auto *neighbor = *it;
					if(neighbor != nullptr)
					{
						auto &nbVerts = neighbor->vertices;
						auto &nbTriangles = neighbor->triangles;
						auto &nbFaceNormals = neighbor->faceNormals;
						find_neighbor_normals(v,rows,nbVerts,nbTriangles,nbFaceNormals,neighborNormals);
					}
				}
			}
			auto &n = outNormals[vertId] = glm::vec3(0.f,0.f,0.f);
			for(auto it=neighborNormals.begin();it!=neighborNormals.end();++it)
				n += *it;
			if(!neighborNormals.empty())
				n /= neighborNormals.size();
			uvec::normalize(&n);
			// TODO: Fill up with "up" vectors when not touching other displacements?
		}
	}
	//
}

static void build_displacement_normals(std::vector<std::shared_ptr<PolyMesh>> &meshes)
{
	for(auto it=meshes.begin();it!=meshes.end();++it)
	{
		auto &mesh = *it;
		if(mesh->HasDisplacements())
		{
			auto &polys = mesh->GetPolys();
			for(auto it=polys.begin();it!=polys.end();++it)
			{
				auto &poly = *it;
				auto &polyInfo = poly->GetCompiledData();
				if(poly->IsDisplacement())
				{
					auto &displacement = *polyInfo.displacement.get();
					build_displacement_normals(meshes,mesh.get(),poly.get(),polyInfo,displacement.normals);
				}
			}
		}
	}
}

static Vector3 calculate_center(std::vector<std::shared_ptr<PolyMesh>> &meshes)
{
	unsigned int numVertsTotal = 0;
	Vector3 center {0.f,0.f,0.f};
	for(auto &mesh : meshes)
	{
		auto &polys = mesh->GetPolys();
		for(unsigned j=0;j<polys.size();j++)
		{
			auto &poly = polys[j];
			auto &vertices = poly->GetVertices();
			numVertsTotal += static_cast<unsigned int>(vertices.size());
			for(unsigned k=0;k<vertices.size();k++)
				center += vertices[k].pos;
		}
	}
	if(numVertsTotal > 0)
		center /= numVertsTotal;
	for(auto &mesh : meshes)
	{
		auto &polys = mesh->GetPolys();
		for(unsigned j=0;j<polys.size();j++)
		{
			auto poly = polys[j];
			auto &vertices = poly->GetVertices();
			for(unsigned k=0;k<vertices.size();k++)
				vertices[k].pos -= center;
		}
	}
	return center;
}

static void swap_yz_coordinates(Vector3 &v)
{
	auto y = v.y;
	v.y = v.z;
	v.z = -y;
}

static void write_mesh(VFilePtrReal fOut,PolyMesh *mesh,PolyMeshInfo &info,std::vector<std::string> &materials)
{
	unsigned int numMaterials = static_cast<unsigned int>(materials.size());
	bool bDisp = mesh->HasDisplacements();
	fOut->Write<bool>(bDisp);
	auto &polys = mesh->GetPolys();
	unsigned int numPolys = static_cast<unsigned int>(polys.size());
	auto &verts = info.vertexList;
	unsigned int numVerts = static_cast<unsigned int>(verts.size());
	fOut->Write<unsigned int>(numVerts);
	for(unsigned int j=0;j<numVerts;j++)
	{
		glm::vec3 &v = verts[j];
		fOut->Write<float>(v.x);
		fOut->Write<float>(v.y);
		fOut->Write<float>(v.z);
	}

	fOut->Write<unsigned int>(numPolys);
	for(unsigned int j=0;j<numPolys;j++)
	{
		auto &poly = polys[j];
		auto &polyInfo = poly->GetCompiledData();
		std::string &smat = poly->GetTextureData()->texture;
		for(unsigned int k=0;k<numMaterials;k++)
		{
			if(smat == materials[k])
			{
				fOut->Write<unsigned int>(k);
				break;
			}
		}
		auto texData = poly->GetTextureData();
		auto &nu = polyInfo.nu;
		auto &nv = polyInfo.nv;
		fOut->Write<float>(nu.x);
		fOut->Write<float>(nu.y);
		fOut->Write<float>(nu.z);
		fOut->Write<float>(nv.x);
		fOut->Write<float>(nv.y);
		fOut->Write<float>(nv.z);
		//fOut->Write<float>(1.0f /width);
		//fOut->Write<float>(1.0f /height);
		fOut->Write<float>(texData->ou);
		fOut->Write<float>(texData->ov);
		fOut->Write<float>(texData->su);
		fOut->Write<float>(texData->sv);
		//fOut->Write<float>(texData->rot);

		auto &polyVerts = poly->GetVertices();
		auto numPolyVerts = static_cast<unsigned int>(polyVerts.size());
		fOut->Write<unsigned int>(numPolyVerts);
		for(unsigned int k=0;k<numPolyVerts;k++)
		{
			auto &va = polyVerts[k].pos;
			auto bFound = false;
			for(unsigned int l=0;l<numVerts;l++)
			{
				auto &vb = verts[l];
				if(fabsf(va.x -vb.x) <= EPSILON &&
					fabsf(va.y -vb.y) <= EPSILON &&
					fabsf(va.z -vb.z) <= EPSILON)
				{
					fOut->Write<unsigned int>(l);
					bFound = true;
					break;
				}
			}
			assert(bFound == true);
		}

		if(bDisp)
		{
			auto &dispInfo = *polyInfo.displacement.get();
			//
			auto &dispVerts = dispInfo.vertices;
			auto &dispNormals = dispInfo.normals;
			auto &dispUvs = dispInfo.uvs;
			auto &dispTriangles = dispInfo.triangles;
			auto &dispAlphas = dispInfo.alphas;
			auto &numAlpha = dispInfo.numAlpha;

			fOut->Write<unsigned short>(CUInt16(dispInfo.powersMerged.size() +1));
			fOut->Write<unsigned char>(CUChar(dispInfo.power));
			for(auto it=dispInfo.powersMerged.begin();it!=dispInfo.powersMerged.end();++it)
				fOut->Write<unsigned char>(*it);
			assert(("Vertex and Normal count do not match!",dispVerts.size() == dispNormals.size()));
			assert(("Vertex and UV count do not match!",dispVerts.size() == dispUvs.size()));
			for(auto it=dispVerts.begin();it!=dispVerts.end();++it)
				fOut->Write<glm::vec3>(*it);
			for(auto it=dispNormals.begin();it!=dispNormals.end();++it)
				fOut->Write<glm::vec3>(*it);
			for(auto it=dispUvs.begin();it!=dispUvs.end();++it)
				fOut->Write<glm::vec2>(*it);
			fOut->Write<unsigned char>(numAlpha);
			auto bHasAlphaA = (numAlpha > 0) ? true : false;
			if(bHasAlphaA == true)
			{
				auto bHasAlphaB = (numAlpha > 1) ? true : false;
				for(auto it=dispAlphas.begin();it!=dispAlphas.end();++it)
				{
					auto &a = *it;
					auto x = umath::round(a.x *255.f);
					x = (x > 255) ? 255 : ((x < 0) ? 0 : x);
					fOut->Write<unsigned char>(CUChar(x));
					if(bHasAlphaB == true)
					{
						auto y = umath::round(a.y *255.f);
						y = (y > 255) ? 255 : ((y < 0) ? 0 : y);
						fOut->Write<unsigned char>(CUChar(y));
					}
				}
			}
			//
		}
		else
		{
			for(unsigned int k=0;k<numPolyVerts;k++)
			{
				auto &v = polyVerts[k];
				fOut->Write<float>(v.normal.x);
				fOut->Write<float>(v.normal.y);
				fOut->Write<float>(v.normal.z);
			}
		}
	}
}

static void merge_meshes(std::vector<std::shared_ptr<PolyMesh>> &meshes,MaterialManager &materialManager)
{
	auto *matNoDraw = materialManager.Load("tools/toolsnodraw");
	// Remove touching faces
	for(auto it=meshes.begin();it!=meshes.end();++it)
	{
		auto &polyMesh = *it;
		if(polyMesh->HasDisplacements() == true) // TODO Is this okay?
			continue;
		glm::vec3 min;
		glm::vec3 max;
		polyMesh->GetBounds(&min,&max);
		auto &polys = polyMesh->GetPolys();
		for(auto it=meshes.begin();it!=meshes.end();++it)
		{
			auto &polyMeshOther = *it;
			glm::vec3 minOther;
			glm::vec3 maxOther;
			polyMeshOther->GetBounds(&minOther,&maxOther);
			if(Intersection::AABBAABB(min,max,minOther,maxOther) != INTERSECT_OUTSIDE)
			{
				for(auto it=polys.begin();it!=polys.end();++it)
				{
					auto &poly = *it;
					auto &verts = poly->GetVertices();
					auto *mat = poly->GetMaterial();
					auto &polysOther = polyMeshOther->GetPolys();
					for(auto itOther=polysOther.begin();itOther!=polysOther.end();++itOther)
					{
						auto &polyOther = *itOther;
						auto &vertsOther = polyOther->GetVertices();
						if(poly != polyOther)
						{
							if(verts.size() == vertsOther.size())
							{
								auto bHasAllEqual = true;
								for(auto it=verts.begin();it!=verts.end();++it)
								{
									auto &v = it->pos;
									auto bHasEqual = false;
									for(auto it=vertsOther.begin();it!=vertsOther.end();++it)
									{
										auto &vFirst = it->pos;
										if(fabsf(v.x -vFirst.x) <= EPSILON &&
											fabsf(v.y -vFirst.y) <= EPSILON &&
											fabsf(v.z -vFirst.z) <= EPSILON)
										{
											bHasEqual = true;
											break;
										}
									}
									if(bHasEqual == false)
									{
										bHasAllEqual = false;
										break;
									}
								}
								if(bHasAllEqual == true) // TODO: Only remove if facing each other!; Remove smaller face if planes intersect (And all vertices are within bounds of larger face)
								{
									//report.facesRemoved++;
									polyOther->SetMaterial(matNoDraw);
									poly->SetMaterial(matNoDraw);
									break;
								}
							}
						}
					}
				}
			}
		}
	}
	//

	struct PolyData
	{
		PolyData(PolyMesh *_mesh,Poly *_poly,PolyInfo &_info)
			: mesh(_mesh),poly(_poly),info(_info)
		{}
		PolyMesh *mesh;
		Poly *poly;
		PolyInfo &info;
	};

	std::unordered_map<Material*,std::vector<std::shared_ptr<PolyData>>> matPolys;
	for(auto it=meshes.begin();it!=meshes.end();++it)
	{
		auto &mesh = *it;
		auto &polys = mesh->GetPolys();
		for(auto it=polys.begin();it!=polys.end();++it)
		{
			auto &poly = *it;
			auto &polyInfo = poly->GetCompiledData();
			auto *mat = poly->GetMaterial();
			auto itMat = matPolys.find(mat);
			if(itMat == matPolys.end())
				itMat = matPolys.insert(std::unordered_map<Material*,std::vector<std::shared_ptr<PolyData>>>::value_type(mat,std::vector<std::shared_ptr<PolyData>>())).first;
			auto &polys = itMat->second;
			polys.push_back(std::shared_ptr<PolyData>(new PolyData(mesh.get(),poly.get(),polyInfo)));
		}
	}
	//auto testCount = 0;
	for(auto it=matPolys.begin();it!=matPolys.end();++it)
	{
		//auto *mat = it->first;
		auto &polys = it->second;
		for(auto it=polys.begin();it!=polys.end();++it)
		{
			auto &polyDataFirst = *it;
			auto &polyFirst = polyDataFirst->poly;
			auto bDisplacement = polyFirst->IsDisplacement();
			if(bDisplacement == true) // First displacement
			{
				if(true)
					continue; // Works, but not needed anymore
				auto &vertsFirst = polyFirst->GetVertices();
				auto &polyInfoFirst = polyDataFirst->info;
				auto &dispFirst = *polyInfoFirst.displacement.get();
				for(auto it2=(it +1);it2!=polys.end();++it2)
				{
					auto &polyData = *it2;
					//if(polyDataFirst.poly->GetMaterial() == polyData.poly->GetMaterial())
					//{
						if(polyData->poly->IsDisplacement())
						{
							auto &polyInfo = polyData->info;
							auto &disp = *polyInfo.displacement.get();
							if(disp.numAlpha == dispFirst.numAlpha)
							{
								dispFirst.powersMerged.push_back(disp.power);
								dispFirst.vertices.reserve(dispFirst.vertices.size() +disp.vertices.size());
								dispFirst.vertices.insert(dispFirst.vertices.end(),disp.vertices.begin(),disp.vertices.end());
			
								dispFirst.normals.reserve(dispFirst.normals.size() +disp.normals.size());
								dispFirst.normals.insert(dispFirst.normals.end(),disp.normals.begin(),disp.normals.end());

								dispFirst.uvs.reserve(dispFirst.uvs.size() +disp.uvs.size());
								dispFirst.uvs.insert(dispFirst.uvs.end(),disp.uvs.begin(),disp.uvs.end());

								dispFirst.alphas.reserve(dispFirst.alphas.size() +disp.alphas.size());
								dispFirst.alphas.insert(dispFirst.alphas.end(),disp.alphas.begin(),disp.alphas.end());

								dispFirst.triangles.reserve(dispFirst.triangles.size() +disp.triangles.size());
								dispFirst.triangles.insert(dispFirst.triangles.end(),disp.triangles.begin(),disp.triangles.end());
								polyData->poly->RemoveDisplacement();
								//++report.displacementsMerged;
							}
						}
					//}
				}
				break;
			}
		}
		/*for(auto it=polys.begin();it!=polys.end();++it)
		{
			auto &polyDataFirst = *it;
			auto *polyFirst = polyDataFirst.poly;
			auto bDisplacement = polyFirst->IsDisplacement();
			if(bDisplacement == false) // First non-displacement
			{
				for(auto it2=(it +1);it2!=polys.end();++it2)
				{
					auto &polyData = *it2;
					if(!polyData.poly->IsDisplacement())
					{
						polyData.poly->SetMaterial(matNoDraw);
						testCount++;
					}
				}
				break;
			}
		}*/
	}

	// Mesh normal meshes
	/*for(auto &pair : matPolys)
	{
		auto *mat = pair.first;
		auto &polys = pair.second;
		if(polys.empty())
			continue;
		auto &polyFirst = *polys.front().poly;
		for(auto it=polys.begin() +1;it!=polys.end();++it)
		{
			auto &polyOther = *it->poly;
			for(auto *v : *polyOther.GetVertices())
			{
				polyFirst.AddVertex(v->pos,v->normal); // TODO: UV
			
			}
			polyOther.
		}
	}*/

	//std::cout<<testCount<<std::endl;
	// Validate displacements
	for(auto it=meshes.begin();it!=meshes.end();)
	{
		(*it)->Validate();
		if(!(*it)->IsValid())
			it = meshes.erase(it);
		else
			++it;
	}
	//
}

struct KeyMod
{
	unsigned char type;
	std::string target;
};

static void write_offset(VFilePtrReal &f,uint64_t locationOffset)
{
	auto offset = f->Tell();
	f->Seek(locationOffset);
		f->Write<uint64_t>(offset);
	f->Seek(offset);
}

static void smooth_touching_poly_normals(const std::vector<std::shared_ptr<PolyMesh>> &meshes)
{
	struct PolyInfo
	{
		std::shared_ptr<Poly> poly;
		std::vector<uint32_t> outline;
	};
	std::vector<PolyInfo> polys {};
	auto numPolys = 0ull;
	for(auto &mesh : meshes)
		numPolys += mesh->GetPolyCount();
	polys.reserve(numPolys);
	for(auto &mesh : meshes)
	{
		for(auto &poly : mesh->GetPolys())
		{
			polys.push_back({poly,{}});

			// Transform poly verts to 2D space (on poly plane)
			auto n = poly->GetNormal();
			auto rot = uvec::get_rotation(Vector3{0.f,1.f,0.f},n);
			uquat::normalize(rot);
			std::vector<Vector2> polyVerts2d {};
			auto &verts = poly->GetVertices();
			polyVerts2d.reserve(verts.size());
			for(auto &polyVert : verts)
			{
				auto v = polyVert.pos;
				uvec::rotate(&v,rot);
				polyVerts2d.push_back(Vector2{v.x,v.z});
			}
			auto outline = Geometry::get_outline_vertices(polyVerts2d);
			if(outline.has_value() == false)
				Con::cwar<<"WARNING: Unable to generate outline for poly! Skipping..."<<Con::endl;
			else
				polys.back().outline = *outline;
		}
	}
	using PolyIndex = std::size_t;
	using VertexIndex = std::size_t;
	std::unordered_map<PolyIndex,std::unordered_map<VertexIndex,std::vector<Vector3>>> polyNormals; // Contains the normals which touching vertices are affected by
	for(auto i=decltype(polys.size()){0u};i<polys.size();++i)
	{
		auto &polyInfo = polys.at(i);
		const auto margin = Vector3{1.f,1.f,1.f};
		Vector3 min,max;
		polyInfo.poly->GetBounds(&min,&max);
		min -= margin;
		max += margin;
		auto &polyVerts = polyInfo.poly->GetVertices();
		auto &polyOutline = polyInfo.outline;

		for(auto j=i +1u;j<polys.size();++j)
		{
			auto &polyInfoOther = polys.at(j);
			auto angle = umath::acos(uvec::dot(polyInfo.poly->GetNormal(),polyInfoOther.poly->GetNormal()));
			const auto maxAngle = umath::pi /3.9; // 45 degree (+margin to account for precision errors)
			if(angle > maxAngle || angle < -maxAngle)
				continue;
			Vector3 minOther,maxOther;
			polyInfoOther.poly->GetBounds(&minOther,&maxOther);
			minOther -= margin;
			maxOther += margin;
			if(Intersection::AABBAABB(min,max,minOther,maxOther) == INTERSECT_OUTSIDE)
				continue;
			auto &polyVertsOther = polyInfoOther.poly->GetVertices();
			auto &polyOutlineOther = polyInfoOther.outline;
			for(auto k=decltype(polyOutlineOther.size()){0u};k<polyOutlineOther.size();++k)
			{
				auto idxOther0 = polyOutlineOther.at(k);
				auto idxOther1 = (k == (polyOutlineOther.size() -1u)) ? 0u : k +1u;
				auto &vOther0 = polyVertsOther.at(idxOther0);
				auto &vOther1 = polyVertsOther.at(idxOther1);
				for(auto l=decltype(polyOutline.size()){0u};l<polyOutline.size();++l)
				{
					auto idx0 = polyOutline.at(l);
					auto idx1 = (l == (polyOutline.size() -1u)) ? 0u : l +1u;
					auto &v0 = polyVerts.at(idx0);
					auto &v1 = polyVerts.at(idx1);
					const auto COMPARE_EPSILON = Vector3{0.04f};
					if(
						(uvec::cmp(v0.pos,vOther0.pos,COMPARE_EPSILON) == true &&
						uvec::cmp(v1.pos,vOther1.pos,COMPARE_EPSILON) == true) ||
						(uvec::cmp(v0.pos,vOther1.pos,COMPARE_EPSILON) == true &&
						uvec::cmp(v1.pos,vOther0.pos,COMPARE_EPSILON) == true)
					)
					{
						// Polys are touching
						struct PolyVertexInfo
						{
							PolyVertexInfo(PolyIndex polyIndex,VertexIndex vertexIndex,const Vector3 &normal)
								: polyIndex{polyIndex},vertexIndex{vertexIndex},normal{normal}
							{}
							PolyIndex polyIndex;
							VertexIndex vertexIndex;
							const Vector3 &normal;
						};
						assert(uvec::cmp(v0.normal,v1.normal));
						assert(uvec::cmp(vOther0.normal,vOther1.normal));
						if(uvec::cmp(v0.normal,v1.normal) == false || uvec::cmp(vOther0.normal,vOther1.normal) == false)
							throw std::logic_error("Normal mismatch!");
						const std::array<PolyVertexInfo,4> polyVertInfos = {
							PolyVertexInfo{i,idx0,vOther0.normal},
							PolyVertexInfo{i,idx1,vOther0.normal},
							PolyVertexInfo{j,idxOther0,v0.normal},
							PolyVertexInfo{j,idxOther1,v0.normal}
						};
						for(auto &info : polyVertInfos)
						{
							auto itPoly = polyNormals.find(info.polyIndex);
							if(itPoly == polyNormals.end())
								itPoly = polyNormals.insert(std::make_pair(info.polyIndex,std::unordered_map<VertexIndex,std::vector<Vector3>>{})).first;
							auto itVert = itPoly->second.find(info.vertexIndex);
							if(itVert == itPoly->second.end())
								itVert = itPoly->second.insert(std::make_pair(info.vertexIndex,std::vector<Vector3>{})).first;
							itVert->second.push_back(info.normal);
						}
					}
				}
			}
		}
	}
	// Calculate smooth normals
	for(auto &polyPair : polyNormals)
	{
		auto &poly = *polys.at(polyPair.first).poly;
		auto &polyVerts = poly.GetVertices();
		for(auto &vertexPair : polyPair.second)
		{
			auto &normal = polyVerts.at(vertexPair.first).normal;
			for(auto &n : vertexPair.second)
				normal += n;
			normal /= static_cast<float>(vertexPair.second.size() +1u);
			uvec::normalize(&normal);
		}
	}
}

vmf::ResultCode vmf::load(NetworkState &nwState,const std::string &fileName,const std::function<void(const std::string&)> &messageLogger)
{
	auto f = FileManager::OpenFile(fileName.c_str(),"rb");
	if(f == nullptr)
	{
		if(messageLogger != nullptr)
		{
			std::stringstream ss;
			ss<<"File '"<<fileName<<"' not found!";
			messageLogger(ss.str());
		}
		return ResultCode::ErrFileNotFound;
	}
	if(messageLogger != nullptr)
		messageLogger("Parsing map info...");
	auto vmf = impl::DataFile::ReadBlock(f);
	if(vmf == nullptr)
	{
		if(messageLogger != nullptr)
			messageLogger("Unable to parse file!");
		return ResultCode::ErrUnableToParseFile;
	}
	auto it = vmf->blocks.find("world");
	if(it == vmf->blocks.end())
	{
		if(messageLogger != nullptr)
			messageLogger("No world found!");
		return ResultCode::ErrNoWorldFound;
	}
	auto &blocks = it->second;
	auto &world = blocks.front();
	auto itSolid = world->blocks.find("solid");
	if(itSolid == world->blocks.end())
	{
		if(messageLogger != nullptr)
			messageLogger("No world solids found!");
		return ResultCode::ErrNoWorldSolidsFound;
	}
	auto &solids = itSolid->second;
	std::vector<std::shared_ptr<PolyMesh>> worldMeshes;
	std::vector<std::shared_ptr<PolyMesh>> entMeshes;
	std::vector<std::string> materials;
	auto &matManager = nwState.GetMaterialManager();
	for(auto &pSolid : solids)
	{
		auto itSide = pSolid->blocks.find("side");
		if(itSide == pSolid->blocks.end())
		{
			if(messageLogger != nullptr)
				messageLogger("WARNING: Solid with no sides! Skipping...");
		}
		else
		{
			auto mesh = impl::build_mesh(itSide->second,&nwState);
			if(mesh == nullptr)
			{
				if(messageLogger != nullptr)
					messageLogger("WARNING: Mesh without polys found. Removing...");
			}
			else
				worldMeshes.push_back(mesh);
		}
	}
	smooth_touching_poly_normals(worldMeshes);

	// Fetch all materials (Including brush entities)
	for(auto &solid : solids)
	{
		auto itSide = solid->blocks.find("side");
		if(itSide == solid->blocks.end())
			continue;
		auto &sides = itSide->second;
		for(auto &side : sides)
		{
			auto mat = side->KeyValue("material");
			auto itMat = std::find(materials.begin(),materials.end(),mat);
			if(itMat == materials.end())
				materials.push_back(mat);
		}
	}

	auto itEnt = vmf->blocks.find("entity");
	std::vector<std::shared_ptr<impl::DataFileBlock>> ents;
	std::unordered_map<std::shared_ptr<impl::DataFileBlock>,std::shared_ptr<PolyMesh>> entBrushMeshes;
	std::unordered_map<std::shared_ptr<impl::DataFileBlock>,Vector3> entCenterPositions;
	if(itEnt != vmf->blocks.end())
	{
		itEnt->second.insert(itEnt->second.begin(),world);
		auto numEnts = itEnt->second.size();
		for(auto i=decltype(numEnts){0u};i<numEnts;++i)
		{
			auto &entity = itEnt->second.at(i);
			std::string classname = "";
			for(auto itKeyValue=entity->keyvalues.begin();itKeyValue!=entity->keyvalues.end();++itKeyValue)
			{
				auto key = itKeyValue->first;
				ustring::to_lower(key);
				if(key == "classname")
				{
					auto &val = itKeyValue->second.front();
					if(val == "worldspawn")
						val = "world";
					classname = val;
					break;
				}
			}
			if(classname.empty() == false)
			{
				auto bWorld = classname == "world";
				if(bWorld == true)
					ents.insert(ents.begin(),entity);
				else
				{
					ents.push_back(entity);

					auto itSolid = (bWorld == false) ? entity->blocks.find("solid") : entity->blocks.end();
					if(itSolid != entity->blocks.end() && itSolid->second.empty() == false) // Brush entity
					{
						auto &solids = itSolid->second;
						std::vector<std::shared_ptr<PolyMesh>> solidMeshes;
						for(auto &solid : solids)
						{
							auto itSide = solid->blocks.find("side");
							if(itSide == solid->blocks.end())
							{
								if(messageLogger != nullptr)
									messageLogger("WARNING: Solid with no sides! Skipping...");
							}
							else
							{
								auto mesh = impl::build_mesh(itSide->second,&nwState);
								if(mesh == nullptr)
								{
									if(messageLogger != nullptr)
										messageLogger("WARNING: Unable to build brush: Mesh without polys!");
								}
								else
								{
									//if(mesh->HasDisplacements()) // Any reason for doing this?
									//	delete mesh;
									//else
									entMeshes.push_back(mesh);
									solidMeshes.push_back(mesh);
									entBrushMeshes.insert(std::unordered_map<std::shared_ptr<impl::DataFileBlock>,std::shared_ptr<PolyMesh>>::value_type(solid,mesh));
								}
							}
						}
						if(!solidMeshes.empty())
							entCenterPositions.insert(std::unordered_map<std::shared_ptr<impl::DataFileBlock>,Vector3>::value_type(entity,bWorld ? Vector3{} : calculate_center(solidMeshes)));
						for(auto it=solidMeshes.begin();it!=solidMeshes.end();++it)
							build_poly_mesh_info(*it,messageLogger);
					}
				}
			}

			auto itSolid = entity->blocks.find("solid");
			if(itSolid != entity->blocks.end())
			{
				auto &solids = itSolid->second;
				unsigned int numSolids = static_cast<unsigned int>(solids.size());
				for(unsigned int l=0;l<numSolids;l++)
				{
					auto &solid = solids[l];
					auto itSide = solid->blocks.find("side");
					if(itSide != solid->blocks.end())
					{
						auto &sides = itSide->second;
						for(int m=0;m<sides.size();m++)
						{
							auto &side = sides[m];
							std::string mat = side->KeyValue("material");
							bool bExists = false;
							for(unsigned int k=0;k<materials.size();k++)
							{
								if(mat == materials[k])
								{
									bExists = true;
									break;
								}
							}
							if(bExists == false)
								materials.push_back(mat);
						}
					}
				}
			}
		}
	}
	if(messageLogger != nullptr)
		messageLogger("Parsing complete!");
	//
	//std::vector<BrushMesh*> brushMeshes;
	//PolyMesh::GenerateBrushMeshes(&brushMeshes,meshes);

	// Build Mesh Info
	for(auto it=worldMeshes.begin();it!=worldMeshes.end();++it)
		build_poly_mesh_info(*it,messageLogger);
	for(auto it=entMeshes.begin();it!=entMeshes.end();++it)
		build_poly_mesh_info(*it,messageLogger);
	build_displacement_normals(worldMeshes);
	build_displacement_normals(entMeshes);
	//
	merge_meshes(worldMeshes,matManager);

	// Build meshes from planes
	//std::vector<std::shared_ptr<MapMesh>> finalWorldMeshes;
	//wlc::build_meshes(worldMeshes,finalWorldMeshes); // TODO: Group by space?

	std::vector<std::shared_ptr<PolyMesh>> meshes;
	meshes.reserve(worldMeshes.size() +entMeshes.size());
	for(auto it=worldMeshes.begin();it!=worldMeshes.end();++it)
		meshes.push_back(*it);
	for(auto it=entMeshes.begin();it!=entMeshes.end();++it)
		meshes.push_back(*it);

	auto wldFileName = fileName;
	ufile::remove_extension_from_filename(wldFileName);
	wldFileName += ".wld";
	if(messageLogger != nullptr)
		messageLogger("Creating output file '" +wldFileName +"'...");
	auto fOut = FileManager::OpenSystemFile(wldFileName.c_str(),"wb");
	if(fOut == nullptr)
	{
		if(messageLogger != nullptr)
			messageLogger("ERROR: Unable to write file '" +wldFileName +"'!");
		return ResultCode::ErrOpenOutputFile;
	}

	if(messageLogger != nullptr)
		messageLogger("Loading FGD data...");
	auto fgdData = pragma::level::load_fgds(nwState,messageLogger);

	fOut->Write<char>('W');
	fOut->Write<char>('L');
	fOut->Write<char>('D');
	fOut->Write<unsigned int>(WLD_VERSION);
	static_assert(WLD_VERSION == 10);

	auto offsetLocMaterials = fOut->Tell();
	fOut->Write<uint64_t>(0);

	auto offsetLocEntities = fOut->Tell();
	fOut->Write<uint64_t>(0);

	fOut->Write<uint64_t>(0ull); // offsetBSPTree
	fOut->Write<uint64_t>(0ull); // offsetLightMapData
	fOut->Write<uint64_t>(0ull); // offsetFaceVertexData

	unsigned int numMeshes = static_cast<unsigned int>(worldMeshes.size());
	for(unsigned int i=0;i<numMeshes;i++)
	{
		auto &mesh = worldMeshes[i];
		auto &polys = mesh->GetPolys();
		unsigned int numPolys = static_cast<unsigned int>(polys.size());
		for(unsigned int j=0;j<numPolys;j++)
		{
			auto &poly = polys[j];
			auto texture = poly->GetTextureData();
			bool bExists = false;
			for(unsigned int k=0;k<materials.size();k++)
			{
				if(texture->texture == materials[k])
				{
					bExists = true;
					break;
				}
			}
			if(bExists == false)
				materials.push_back(texture->texture);
		}
	}
	if(messageLogger != nullptr)
		messageLogger("Writing world data...");

	write_offset(fOut,offsetLocMaterials);
	unsigned int numMaterials = static_cast<unsigned int>(materials.size());
	fOut->Write<unsigned int>(numMaterials);
	for(unsigned int i=0;i<numMaterials;i++)
		fOut->WriteString(materials[i]);

	if(messageLogger != nullptr)
		messageLogger("Done!");
	// To write as brushmeshes instead: (GenerateBrushMeshes has to be uncommented as well)
	// Advantage is that less has to be calculated on runtime (Triangle calculation), downside is that the file size increases
	/*std::vector<std::string> materials;
	unsigned int numMeshes = brushMeshes.size();
	for(unsigned int i=0;i<numMeshes;i++)
	{
		BrushMesh *mesh = brushMeshes[i];
		std::vector<Side*> *sides;
		mesh->GetSides(&sides);
		unsigned int numSides = sides->size();
		for(unsigned int j=0;j<numSides;j++)
		{
			Side *side = (*sides)[j];
			std::string &mat = side->GetMaterial();
			bool bExists = false;
			for(unsigned int k=0;k<materials.size();k++)
			{
				if(mat == materials[k])
				{
					bExists = true;
					break;
				}
			}
			if(bExists == false)
				materials.push_back(mat);
		}
	}
	unsigned int numMaterials = materials.size();
	fOut->Write<unsigned int>(numMaterials);
	for(unsigned int i=0;i<numMaterials;i++)
		fOut->WriteString(materials[i]);
	fOut->Write<unsigned int>(numMeshes);
	for(unsigned int i=0;i<numMeshes;i++)
	{
		BrushMesh *mesh = brushMeshes[i];
		std::vector<Side*> *sides;
		mesh->GetSides(&sides);
		unsigned int numSides = sides->size();
		std::vector<glm::vec3> verts;
		for(unsigned j=0;j<numSides;j++)
		{
			Side *side = (*sides)[j];
			std::vector<glm::vec3> *sideVerts;
			side->GetVertices(&sideVerts);
			unsigned int numSideVerts = sideVerts->size();
			for(unsigned int k=0;k<numSideVerts;k++)
			{
				glm::vec3 &va = (*sideVerts)[k];
				bool bExists = false;
				for(unsigned int l=0;l<verts.size();l++)
				{
					glm::vec3 &vb = verts[l];
					if(fabsf(va.x -vb.x) <= EPSILON &&
						fabsf(va.y -vb.y) <= EPSILON &&
						fabsf(va.z -vb.z) <= EPSILON)
					{
						bExists = true;
						break;
					}
				}
				if(bExists == false)
					verts.push_back(va);
			}
		}
		unsigned int numVerts = verts.size();
		fOut->Write<unsigned int>(numVerts);
		for(unsigned int j=0;j<numVerts;j++)
		{
			glm::vec3 &v = verts[j];
			fOut->Write<float>(v.x);
			fOut->Write<float>(v.y);
			fOut->Write<float>(v.z);
		}
		fOut->Write<unsigned int>(numSides);
		for(unsigned int j=0;j<numSides;j++)
		{
			Side *side = (*sides)[j];
			std::string &mat = side->GetMaterial();
			for(unsigned int k=0;k<numMaterials;k++)
			{
				if(mat == materials[k])
				{
					fOut->Write<unsigned int>(k);
					break;
				}
			}
			std::vector<glm::vec3> *vertices;
			side->GetVertices(&vertices);
			unsigned int numVerts = vertices->size();
			fOut->Write<unsigned int>(numVerts);
			for(unsigned int k=0;k<numVerts;k++)
			{
				glm::vec3 &va = (*vertices)[k];
				for(unsigned int l=0;l<verts.size();l++)
				{
					glm::vec3 &vb = verts[l];
					if(fabsf(va.x -vb.x) <= EPSILON &&
						fabsf(va.y -vb.y) <= EPSILON &&
						fabsf(va.z -vb.z) <= EPSILON)
					{
						fOut->Write<unsigned int>(l);
						break;
					}
				}
			}
			std::vector<glm::vec2> *uvs;
			side->GetUVCoordinates(&uvs);
			for(unsigned int k=0;k<numVerts;k++)
			{
				glm::vec2 &uv = (*uvs)[k];
				fOut->Write<float>(uv.x);
				fOut->Write<float>(uv.y);
			}
		}
	}*/

	fOut->Write<bool>(false); // No BSP Tree
	fOut->Write<bool>(false); // No light map
	fOut->Write<Vector2i>(Vector2i{}); // Atlas size
	fOut->Write<uint8_t>(0u); // Border size
	fOut->Write<uint32_t>(0u); // Light map atlas rect count
	fOut->Write<uint64_t>(0ull); // Luxel data
	fOut->Write<uint32_t>(0u); // Face data
	fOut->Write<uint32_t>(0u); // Edges
	fOut->Write<uint32_t>(0u); // Verts
	fOut->Write<uint32_t>(0u); // Tex Info
	fOut->Write<uint64_t>(0ull); // Faces
	fOut->Write<uint64_t>(0ull); // Leaf Faces
	fOut->Write<uint32_t>(0u); // Disp Info
	fOut->Write<uint64_t>(0ull); // Sample Positions

	// Write Entities
	if(messageLogger != nullptr)
		messageLogger("Writing entity data...");
	write_offset(fOut,offsetLocEntities);
	if(ents.empty())
	{
		if(messageLogger != nullptr)
			messageLogger("WARNING: No entities found!");
		fOut->Write<uint32_t>(static_cast<uint32_t>(0));
	}
	else
	{
		auto offsetNumEnts = fOut->Tell();
		fOut->Write<uint32_t>(static_cast<uint32_t>(0));
		for(auto itEnt=ents.begin();itEnt!=ents.end();++itEnt)
		{
			auto &entity = *itEnt;
			std::string classname = "";
			std::string targetname = "";
			std::unordered_map<std::string,std::string> originalKeyValues {};
			originalKeyValues.reserve(entity->keyvalues.size());
			for(auto &pair : entity->keyvalues)
				originalKeyValues.insert(std::make_pair(pair.first,pair.second.front()));
			for(auto &pair : originalKeyValues)
			{
				auto key = pair.first;
				ustring::to_lower(key);
				if(key == "classname")
					classname = pair.second;
				else if(key == "targetname")
					targetname = pair.second;
			}
			if(classname == "world")
				originalKeyValues.insert(std::make_pair("color",pragma::level::WLD_DEFAULT_AMBIENT_COLOR.ToString()));
			if(messageLogger != nullptr)
			{
				if(targetname.empty())
					messageLogger("\tWriting entity '" +classname +"...");
				else
					messageLogger("\tWriting entity '" +classname +" (" +targetname +")...");
			}

			auto offsetLocEndOfEntity = fOut->Tell();
			fOut->Write<uint64_t>(static_cast<uint64_t>(0)); // Offset to end of entity

			auto offsetMeshes = fOut->Tell();
			fOut->Write<uint64_t>(static_cast<uint64_t>(0)); // Size of entity information until mesh data; We don't need any specific entity info on the client, so we'll need this to skip them.

			auto offsetLeaves = fOut->Tell();
			fOut->Write<uint64_t>(0ull); // Leaves offset

			enum class EntityFlags : uint64_t
			{
				None = 0ull,
				ClientsideOnly = 1ull
			};

			auto flags = EntityFlags::None;
			auto bClassFound = false;
			std::unordered_map<std::string,std::string> outKeyValues {};
			pragma::level::transform_class(originalKeyValues,outKeyValues,classname);
			for(auto &pair : outKeyValues)
			{
				pragma::level::transform_keyvalue(fgdData,classname,pair.first,pair.second,[&bClassFound,&messageLogger](const std::string &msg,uint8_t msgType) {
					if(msgType == 0u)
					{
						if(bClassFound == true)
							return;
						bClassFound = true;
					}
					messageLogger(msg);
				});
			}

			for(auto itKeyValue=outKeyValues.begin();itKeyValue!=outKeyValues.end();)
			{
				std::string key = itKeyValue->first;
				StringToLower(key);
				std::string &val = itKeyValue->second;
				if(key.substr(0,8) == "wv_hint_")
				{
					if(val != "0")
					{
						size_t hintEnd = key.find_first_of('_',8);
						std::string hint = key.substr(8,hintEnd -8);
						std::string valMod = key.substr(hintEnd +1,key.length());
						if(hint == "clientsideonly")
							flags = static_cast<EntityFlags>(umath::to_integral(flags) | umath::to_integral(EntityFlags::ClientsideOnly));
						else if(hint == "classoverride")
							classname = val;
					}
					auto itNext = itKeyValue;
					itNext++;
					outKeyValues.erase(itKeyValue);
					itKeyValue = itNext;
				}
				else
					itKeyValue++;
			}
			fOut->Write<uint64_t>(umath::to_integral(flags));
			fOut->WriteString(classname);
			unsigned int numKeyValues = static_cast<unsigned int>(outKeyValues.size());
			fOut->Write<unsigned int>(numKeyValues);
			std::unordered_set<std::string> components {};
			pragma::level::find_entity_components(outKeyValues,components);
			for(auto itKeyValue=outKeyValues.begin();itKeyValue!=outKeyValues.end();itKeyValue++)
			{
				std::string key = itKeyValue->first;
				StringToLower(key);
				std::string &val = itKeyValue->second;
				fOut->WriteString(key);
				fOut->WriteString(val);
			}
			// Write outputs
			auto it = entity->blocks.find("connections");
			if(it == entity->blocks.end() || it->second.size() == 0)
				fOut->Write<unsigned int>((unsigned int)(0));
			else
			{
				auto &data = it->second[0];
				unsigned long long offsetConnections = fOut->Tell();
				fOut->Write<unsigned int>((unsigned int)(0));
				unsigned int numConnections = 0;
				std::map<std::string,std::vector<std::string>>::iterator it;
				for(auto it=data->keyvalues.begin();it!=data->keyvalues.end();it++)
				{
					std::string output = it->first;
					for(unsigned int i=0;i<it->second.size();i++)
					{
						std::string val = it->second[i];
						std::vector<std::string> args;
						ustring::explode(val,",",args);
						auto numArgs = args.size();
						if(numArgs >= 2)
						{
							std::string target = args[0];
							std::string input = args[1];
							std::string param = "";
							float delay = 0.f;
							int times = 1;
							if(numArgs > 2)
							{
								param = args[2];
								if(numArgs > 3)
								{
									delay = static_cast<float>(atof(args[3].c_str()));
									if(numArgs > 4)
										times = atoi(args[4].c_str());
								}
							}
							numConnections++;
							fOut->WriteString(output);
							fOut->WriteString(target);
							fOut->WriteString(input);
							fOut->WriteString(param);
							fOut->Write<float>(delay);
							fOut->Write<int>(times);
						}
					}
					unsigned long long cur = fOut->Tell();
					fOut->Seek(offsetConnections);
					fOut->Write<unsigned int>(numConnections);
					fOut->Seek(cur);
				}
			}

			// Write custom components
			fOut->Write<uint32_t>(components.size());
			for(auto &componentName : components)
				fOut->WriteString(componentName);

			// Brush geometry
			std::vector<std::shared_ptr<PolyMesh>> *meshes = nullptr;
			std::vector<std::shared_ptr<PolyMesh>> tmpMeshes {};
			auto itSolid = entity->blocks.find("solid");
			if(itSolid != entity->blocks.end() && itSolid->second.size() > 0)
			{
				if(classname == "world")
					meshes = &worldMeshes;
				else
				{
					meshes = &tmpMeshes;
					auto &solids = itSolid->second;
					for(UInt l=0;l<solids.size();l++)
					{
						auto &solid = solids[l];
						auto itBrushMesh = entBrushMeshes.find(solid);
						if(itBrushMesh != entBrushMeshes.end())
						{
							auto &mesh = itBrushMesh->second;
							tmpMeshes.push_back(mesh);
						}
					}
				}
			}
			unsigned int numMeshes = (meshes != nullptr) ? static_cast<unsigned int>(meshes->size()) : 0u;

			// Entity origin
			Vector3 origin {};
			if(numMeshes > 0u)
			{
				auto itCenter = entCenterPositions.find(entity);
				origin = itCenter->second;
			}
			else
			{
				auto itOrigin = outKeyValues.find("origin");
				origin = (itOrigin != outKeyValues.end()) ? uvec::create(itOrigin->second) : Vector3{};
			}
			fOut->Write<Vector3>(origin);

			auto cur = fOut->Tell();
			fOut->Seek(offsetLeaves);
				fOut->Write<uint64_t>(cur -offsetLeaves);
			fOut->Seek(cur);
			fOut->Write<uint32_t>(0ull); // Number of leaves

			cur = fOut->Tell();
			fOut->Seek(offsetMeshes);
				fOut->Write<uint64_t>(cur -offsetMeshes); // Offset to the entity meshes
			fOut->Seek(cur);

			fOut->Write<unsigned int>(numMeshes);
			if(numMeshes > 0u)
			{
				//fOut->Write<bool>(false);
				for(unsigned int l=0;l<numMeshes;l++)
				{
					auto &mesh = (*meshes)[l];
					auto &info = mesh->GetCompiledData();
					write_mesh(fOut,mesh.get(),info,materials);
				}
			}

			fOut->Write<uint32_t>(0u); // Faces

			cur = fOut->Tell();
			fOut->Seek(offsetLocEndOfEntity);
				fOut->Write<uint64_t>(cur -offsetLocEndOfEntity); // Offset to the end of the entity
			fOut->Seek(cur);
		}
		unsigned long long pos = fOut->Tell();
		fOut->Seek(offsetNumEnts);
		fOut->Write<unsigned int>(static_cast<unsigned int>(ents.size()));
		fOut->Seek(pos);
	}
	if(messageLogger != nullptr)
	{
		messageLogger("Done!");
		messageLogger("All operations are complete!");
		messageLogger("---------------------------------");
		messageLogger("World Mesh Count: " +std::to_string(worldMeshes.size()));
		messageLogger("Entity Mesh Count: " +std::to_string(entMeshes.size()));
		messageLogger("Total Mesh Count: " +std::to_string(meshes.size()));
		//messageLogger("Faces removed: " +std::to_string(report.facesRemoved));
		//messageLogger("Displacements merged: " +std::to_string(report.displacementsMerged));
		//messageLogger("Meshes merged: " +std::to_string(report.meshesMerged));
		//messageLogger("Entity Count: " +std::to_string(((itEnt == vmf->blocks.end()) ? 0 : itEnt->second->size())));
		//messageLogger("Time passed: " +std::to_string((tDelta /1000.f)<<" seconds"));
		messageLogger("Successfully saved map as '" +wldFileName +"'!");
	}
	return ResultCode::Success;
}
