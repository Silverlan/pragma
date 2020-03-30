#include <iostream>
#include "source_engine/vbsp/bsp_converter.hpp"
#include <pragma/networkstate/networkstate.h>
#include <util_bsp.hpp>
#include <util_vmf.hpp>
#include <vmf_dispinfo.hpp>
#include <vmf_poly.hpp>
#include <vmf_poly_mesh.hpp>
#include <vmf_entity_data.hpp>
#include <pragma/console/conout.h>

void pragma::asset::vbsp::BSPConverter::BuildDisplacement(bsp::File &bsp,NetworkState *nw,const bsp::dDisp &disp,std::vector<std::shared_ptr<vmf::PolyMesh>> &outMeshes)
{
	auto &texInfo = bsp.GetTexInfo().at(disp.face.texinfo);
	auto &texData = bsp.GetTexData().at(texInfo.texdata);
	auto &dispInfo = disp.dispInfo;

	vmf::DispInfo info {};
	info.power = dispInfo.power;
	info.startposition = Vector3(dispInfo.startPosition.x,dispInfo.startPosition.z,-dispInfo.startPosition.y);
	info.startpositionId = std::numeric_limits<uint32_t>::max();
	//info.flags = dispInfo.flags;
	//info.elevation = dispInfo.elevation;
	//info.subdiv = dispInfo.subdiv;

	auto rows = static_cast<uint32_t>(pow(2u,info.power)) +1u;
	auto bInvalid = false;

	if(disp.verts.size() != umath::pow2(rows))
		throw std::logic_error("Discrepancy between displacement vertex count and displacement power/row count!");
	auto planeDist = disp.plane.dist;
	auto planeNormal = Vector3{disp.plane.normal.x,-disp.plane.normal.z,-disp.plane.normal.y};
	for(auto i=decltype(rows){0};i<rows;++i)
	{
		info.normals.push_back({});
		info.distances.push_back({});
		info.offsets.push_back({});
		info.alphas.push_back({});
		auto &vNormals = info.normals.back();
		auto &vDistances = info.distances.back();
		auto &vOffsets = info.offsets.back();
		auto &vAlphas = info.alphas.back();

		vDistances.reserve(disp.verts.size());
		vNormals.reserve(disp.verts.size());
		vOffsets.reserve(vOffsets.size());
		vAlphas.reserve(vAlphas.size());
		for(auto j=decltype(rows){0};j<rows;++j)
		{
			auto &v = disp.verts.at(i *rows +j);
			vDistances.push_back(v.dist);

			vOffsets.push_back({});
			auto n = Vector3{v.vec.y,v.vec.z,-v.vec.x};
			if(glm::length(n) == 0.f)
				n = glm::normalize(planeNormal *((planeDist > 0.f) ? 1.f : -1.f));
			else
				n = glm::normalize(n);
			vNormals.push_back(n);
			//vOffsets.push_back({v.vec.x,-v.vec.z,-v.vec.y});
			//vNormals.push_back({disp.plane.normal.x,disp.plane.normal.z,-disp.plane.normal.y});//{0.f,1.f,0.f}); // TODO

			auto a = v.alpha;
			if(a < 0.f || a > 255.f)
				a = umath::clamp(a,0.f,255.f);
			vAlphas.push_back({a /255.f,0.f});
		}
	}

	//info.startposition
	auto &texStringTable = bsp.GetTexDataStringIndices();
	auto &texStringData = bsp.GetTranslatedTexDataStrings();
	auto brushTexStringId = texStringTable.at(texData.nameStringTableID);

	auto polyMesh = std::make_shared<vmf::PolyMesh>();
	polyMesh->SetHasDisplacements(true);
	auto poly = new vmf::Poly([nw](const std::string &mat) -> Material* {
		return nw->LoadMaterial(mat);
	});
	auto &s = texInfo.textureVecs.at(0);
	auto &t = texInfo.textureVecs.at(1);
	auto su = 1.f;
	auto sv = 1.f;
	auto rot = 0.f;
	poly->SetTextureData(
		texStringData.at(brushTexStringId),
		Vector3(s.at(0),s.at(2),-s.at(1)),
		Vector3(t.at(0),t.at(2),-t.at(1)),
		s.at(3),-t.at(3),
		su,sv,
		rot
	);
	poly->SetMaterialId(brushTexStringId);
	poly->SetNormal(planeNormal);
	poly->SetDistance(planeDist);

	static const auto EPSILON = 0.1f;

	auto &polyVerts = poly->GetVertices();
	for(uint8_t i=0;i<4;++i)
		polyVerts.push_back(vmf::Vertex{});
	// Determine face vertex positions
	auto &edges = bsp.GetEdges();
	auto &surfEdges = bsp.GetSurfEdges();
	auto &verts = bsp.GetVertices();
	auto &face = disp.face;
	auto vertIdx = 0u;
	for(auto i=face.firstedge;i<(face.firstedge +face.numedges);++i)
	{
		auto edgeIdx = umath::abs(surfEdges.at(i));
		auto &edge = edges.at(edgeIdx);

		for(auto vIdx : edge.v)
		{
			auto pos = verts.at(vIdx);
			umath::swap(pos.y,pos.z);
			umath::negate(pos.z);

			auto it = std::find_if(polyVerts.begin(),polyVerts.end(),[&pos](vmf::Vertex &v) {
				return umath::abs(v.pos.x -pos.x) <= EPSILON &&
					umath::abs(v.pos.y -pos.y) <= EPSILON &&
					umath::abs(v.pos.z -pos.z) <= EPSILON;
			});
			if(it != polyVerts.end())
				continue;
			if(vertIdx >= polyVerts.size())
				throw std::logic_error("Too many displacement face vertices!");
			polyVerts.at(vertIdx++).pos = pos;
		}
	}
	//auto tmp = polyVerts.at(1);
	//polyVerts.at(1) = polyVerts.at(3);
	//polyVerts.at(3) = tmp;
	poly->SortVertices();
	//

	auto dMin = std::numeric_limits<float>::max();
	auto vertexIndex = 0u;
	for(auto &v : polyVerts)
	{
		auto d = uvec::distance(v.pos,info.startposition);
		if(d < dMin)
		{
			dMin = d;
			info.startpositionId = vertexIndex;
		}
		++vertexIndex;
	}
	if(dMin >= 1.f)
		Con::cwar<<"WARNING: Displacement start position is "<<dMin<<" units away from closest polygon position! This may cause issues..."<<Con::endl;

	polyMesh->AddPoly(poly);

	/*auto err = polyMesh->BuildPolyMesh();
	if(err != 0)
	{
	if(err == -1)
	std::cout<<"WARNING: Displacement with incorrect vertex count! Skipping mesh..."<<std::endl;
	else if(err == -2)
	std::cout<<"WARNING: Displacement with invalid start position! Skipping mesh..."<<std::endl;
	else
	std::cout<<"WARNING: Unable to build mesh. Skipping..."<<std::endl;
	}*/

	polyMesh->SetHasDisplacements(true);
	poly->SetDisplacement(info);

	polyMesh->BuildPolyMeshInfo();
	polyMesh->Calculate();

	outMeshes.push_back(polyMesh);
}
