#include <iostream>
#include "source_engine/vbsp/bsp_converter.hpp"
#include <source_engine/vmf/util_vmf.hpp>
#include <pragma/networkstate/networkstate.h>
#include <vmf_poly.hpp>
#include <vmf_poly_mesh.hpp>
#include <vmf_entity_data.hpp>

void pragma::asset::vbsp::BSPConverter::FindModelBrushes(bsp::File &bsp,const bsp::dmodel_t &mdl,std::unordered_set<uint32_t> &brushIds)
{
	auto &nodes = bsp.GetNodes();
	auto &leaves = bsp.GetLeaves();
	auto &brushes = bsp.GetBrushes();
	auto &leafBrushes = bsp.GetLeafBrushes();
	std::function<void(int32_t)> fIterateTree = nullptr;
	fIterateTree = [&brushIds,&fIterateTree,&nodes,&leaves,&brushes,&leafBrushes](int32_t nodeId) {
		if(nodeId < 0)
		{
			auto leafId = -(nodeId + 1);
			auto &leaf = leaves.at(leafId);
			for(auto i=leaf.firstleafbrush;i<(leaf.firstleafbrush +leaf.numleafbrushes);++i)
			{
				auto idx = leafBrushes.at(i);
				auto &brush = brushes.at(idx);
				brushIds.insert(idx);
			}
		}
		else
		{
			auto &node = nodes.at(nodeId);
			for(auto childIdx : node.children)
				fIterateTree(childIdx);
		}
	};
	fIterateTree(mdl.headnode);
}

void pragma::asset::vbsp::BSPConverter::BuildBrushMesh(bsp::File &bsp,NetworkState *nw,const std::unordered_set<uint32_t> &brushIds,std::vector<std::shared_ptr<vmf::PolyMesh>> &outMeshes)
{
	auto &brushes = bsp.GetBrushes();
	auto &brushSides = bsp.GetBrushSides();
	auto &planes = bsp.GetPlanes();
	auto &texInfo = bsp.GetTexInfo();
	auto &texData = bsp.GetTexData();
	auto &texStringTable = bsp.GetTexDataStringIndices();
	auto &texStringData = bsp.GetTranslatedTexDataStrings();
	outMeshes.reserve(outMeshes.size() +brushIds.size());
	for(auto &idx : brushIds)
	{
		auto &brush = brushes.at(idx);
		auto mesh = std::make_shared<vmf::PolyMesh>();
		for(auto i=brush.firstside;i<brush.firstside +brush.numsides;++i)
		{
			auto &brushSide = brushSides.at(i);
			auto &brushPlane = planes.at(brushSide.planenum);
			std::string brushTexString;
			std::array<float,4> s {};
			std::array<float,4> t {};

			auto poly = new vmf::Poly([nw](const std::string &mat) -> Material* {
				return nw->LoadMaterial(mat);
			});
			auto texId = brushSide.texinfo;
			auto validTex = texId != -1 && texInfo.at(brushSide.texinfo).texdata != -1;
			if(validTex)
			{
				auto &brushTexInfo = texInfo.at(brushSide.texinfo);

				const auto SURF_SKIP = 0x200;
				const auto SURF_HINT = 0x100;
				const auto SURF_NODRAW = 0x80;
				const auto SURF_TRIGGER = 0x40;
				if((brushTexInfo.flags &SURF_SKIP) || (brushTexInfo.flags &SURF_HINT) || (brushTexInfo.flags &SURF_NODRAW) || (brushTexInfo.flags &SURF_TRIGGER))
					texId = -1;
				else
				{
					auto &brushTexData = texData.at(brushTexInfo.texdata);
					auto brushTexStringId = texStringTable.at(brushTexData.nameStringTableID);
					brushTexString = texStringData.at(brushTexStringId);
					s = brushTexInfo.textureVecs.at(0);
					t = brushTexInfo.textureVecs.at(1);
					poly->SetMaterialId(brushTexStringId);
				}
			}
			if(validTex == false)
			{
				brushTexString = "TOOLS/TOOLSNODRAW";
				auto it = std::find(texStringData.begin(),texStringData.end(),brushTexString);
				if(it != texStringData.end())
					poly->SetMaterialId(it -texStringData.begin());
			}

			auto lu = uvec::length(Vector3(s.at(0),s.at(1),s.at(2)));
			auto su = (lu != 0.f) ? (1.f /lu) : 0.f;
			auto lv = uvec::length(Vector3(t.at(0),t.at(1),t.at(2)));
			auto sv = (lv != 0.f) ? (1.f /lv) : 0.f;
			auto rot = 0.f;
			poly->SetTextureData(
				brushTexString,
				Vector3(s.at(0),s.at(2),-s.at(1)), // nu
				Vector3(t.at(0),t.at(2),-t.at(1)), // nv
				s.at(3),-t.at(3), // ou, ov
				su,-sv, // su, sv
				rot
			);
			poly->SetNormal({-brushPlane.normal.x,-brushPlane.normal.z,brushPlane.normal.y});
			poly->SetDistance(brushPlane.dist);
			mesh->AddPoly(poly);
		}
		auto err = mesh->BuildPolyMesh();
		if(err != 0)
		{
			if(err == -1)
				std::cout<<"WARNING: Displacement with incorrect vertex count! Skipping mesh..."<<std::endl;
			else if(err == -2)
				std::cout<<"WARNING: Displacement with invalid start position! Skipping mesh..."<<std::endl;
			else
				std::cout<<"WARNING: Unable to build mesh. Skipping..."<<std::endl;
		}

		mesh->BuildPolyMeshInfo();
		mesh->Calculate();

		outMeshes.push_back(mesh);
	}
}
