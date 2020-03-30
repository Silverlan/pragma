#include <iostream>
#include "source_engine/vbsp/bsp_converter.hpp"
#include <pragma/model/brush/brushmesh.h>
#include <pragma/game/game.h>
#include <vmf_poly.hpp>
#include <vmf_poly_mesh.hpp>
#include <vmf_entity_data.hpp>
#include <memory>

std::vector<std::shared_ptr<CollisionMesh>> pragma::asset::vbsp::BSPConverter::GeneratePhysics(asset::EntityData &entData,const std::vector<std::shared_ptr<vmf::PolyMesh>> &polyMeshes,const std::vector<MaterialHandle> &materials)
{
	auto &game = m_game;
	auto numMeshes = polyMeshes.size();

	auto *physEnv = game.GetPhysicsEnvironment();
	auto &surfaceMaterials = game.GetSurfaceMaterials();

	std::vector<std::shared_ptr<BrushMesh>> meshes;
	meshes.reserve(polyMeshes.size());
	for(auto &polyMesh : polyMeshes)
	{
		auto &info = polyMesh->GetCompiledData();
		std::vector<vmf::Poly*> *polys;
		polyMesh->GetPolys(&polys);
		auto numPolys = polys->size();
		auto verts = info.vertexList;
		for(auto &v : verts)
			v = BSPVertexToPragma(v);

		bool bDisplacement = polyMesh->HasDisplacements();
		auto mesh = game.CreateBrushMesh();
		if(bDisplacement)
			mesh->SetConvex(false);

		for(auto *poly : *polys)
		{
			auto &polyInfo = poly->GetCompiledData();
			std::vector<uint16_t> lodTriangles;
			auto side = game.CreateSide();
			std::vector<Vector3> sideVerts {};
			auto &polyVerts = poly->GetVertices();
			for(auto &pv : polyVerts)
			{
				auto va = BSPVertexToPragma(pv.pos);

				auto dClosest = std::numeric_limits<float>::max();
				uint32_t closestVertexIdx = std::numeric_limits<uint32_t>::max();
				for(auto vertIdx=decltype(verts.size()){0u};vertIdx<verts.size();++vertIdx)
				{
					auto &v = verts.at(vertIdx);
					auto d = uvec::distance_sqr(v,va);
					if(d >= dClosest)
						continue;
					dClosest = d;
					closestVertexIdx = vertIdx;
				}
				sideVerts.push_back(verts.at(closestVertexIdx));
			}

			auto matId = poly->GetMaterialId();
			Material *mat = (matId < materials.size()) ? materials.at(matId).get() : nullptr;

			uint8_t numAlpha = 0;
			if(bDisplacement == true)
			{
				auto &dispInfo = *polyInfo.displacement;
				auto dispVerts = dispInfo.vertices;
				auto dispNormals = dispInfo.normals;

				for(auto &v : dispVerts)
					v = BSPVertexToPragma(v);
				for(auto &n : dispNormals)
					n = BSPVertexToPragma(n);

				side->GetVertices() = dispVerts;
				side->GetNormals() = dispNormals;
				side->GetUVMapping() = dispInfo.uvs;
				side->SetDisplacementAlpha(dispInfo.numAlpha);
				side->GetAlphas() = dispInfo.alphas;

				// Triangles
				auto *polyDispInfo = poly->GetDisplacement();
				auto power = polyDispInfo->power;
				auto rows = umath::pow(2,CInt32(power)) +1;
				auto numVerts = umath::pow(rows,2);
				auto numIndices = umath::pow(rows -1,2) *6;

				lodTriangles.resize(numIndices);
				UInt32 idx = 0;
				//UInt32 i = 0;
				UInt32 offset = 0;
				//for(auto it=dispRows.begin();it!=dispRows.end();++it)
				{
					//auto rows = *it;
					for(auto col=0;col<rows -1;col++)
					{
						for(auto row=0;row<rows -1;row++)
						{
							auto a = offset +col *rows +row;
							auto b = a +1;
							auto c = offset +(col +1) *rows +row;
							lodTriangles[idx] = static_cast<uint16_t>(a);
							lodTriangles[idx +1] = static_cast<uint16_t>(b);
							lodTriangles[idx +2] = static_cast<uint16_t>(c);
							idx += 3;

							auto d = offset +(col +1) *rows +row +1;
							lodTriangles[idx] = static_cast<uint16_t>(b);
							lodTriangles[idx +1] = static_cast<uint16_t>(d);
							lodTriangles[idx +2] = static_cast<uint16_t>(c);
							idx += 3;
						}
					}
					offset += umath::pow2(rows);
				}
			}
			else
			{
				auto &vertices = side->GetVertices();
				auto &uvs = side->GetUVMapping();
				auto &normals = side->GetNormals();
				auto numSideVerts = sideVerts.size();

				auto nu = polyInfo.nu;
				auto nv = polyInfo.nv;
				uvec::normalize(&nu);
				uvec::normalize(&nv);
				nu = BSPVertexToPragma(nu);
				nv = BSPVertexToPragma(nv);

				auto *texData = poly->GetTextureData();
				float ou = texData->ou;
				float ov = texData->ov;
				float su = texData->su;
				float sv = texData->sv;

				TextureInfo *tex = (mat != nullptr) ? mat->GetDiffuseMap() : nullptr;
				unsigned int w = 0;
				unsigned int h = 0;
				if(tex != NULL)
				{
					w = tex->width;
					h = tex->height;
				}
				if(w == 0)
					w = 512;
				if(h == 0)
					h = 512;

				float sw = 1.f /w;
				float sh = 1.f /h;

				vertices.resize(numSideVerts);
				uvs.resize(numSideVerts);
				for(unsigned int k=0;k<numSideVerts;k++)
				{
					Vector3 &v = sideVerts[k];
					vertices[k] = v;
					Vector2 uv;
					uv.x = (glm::dot(v,nu) *sw) /su +ou *sw;
					uv.y = 1.f -((glm::dot(v,nv) *sh) /sv +ov *sh);

					uvs[k] = uv;
				}
				normals.reserve(numSideVerts);
				for(auto &pv : polyVerts)
					normals.push_back(BSPVertexToPragma(pv.normal));

				// To triangles
				size_t pivot = 0;
				auto numVerts = vertices.size();
				if(numVerts > 0)
				{
					auto numVals = (numVerts -2) *3;
					lodTriangles.resize(numVals);
					auto idx = 0;
					for(auto i=pivot +2;i<numVerts;i++)
					{
						lodTriangles[idx] = static_cast<uint16_t>(pivot);
						lodTriangles[idx +1] = static_cast<uint16_t>(i -1);
						lodTriangles[idx +2] = static_cast<uint16_t>(i);
						idx += 3;
					}
				}
			}
			side->SetMaterial(mat);
			side->SetDisplacementAlpha(numAlpha);
			*side->GetTriangles() = lodTriangles;
			side->SetConvex(!bDisplacement);
			mesh->AddSide(side);
		}
		if(bDisplacement == false)
			mesh->Optimize();
		if(physEnv)
			mesh->Calculate(*physEnv,&surfaceMaterials);
		meshes.push_back(mesh);
	}
	return GenerateCollisionMeshes(game,meshes,surfaceMaterials);
}
