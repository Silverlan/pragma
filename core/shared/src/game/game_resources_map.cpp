#include "stdafx_shared.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/entities/prop/prop_base.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/environment/audio/env_sound.h"
#include "pragma/file_formats/util_vmf.hpp"
#include "pragma/level/level_info.hpp"
#include "pragma/util/util_bsp_tree.hpp"
#include <sharedutils/util_file.h>
#include <util_fgd.hpp>
#include <pragma/file_formats/util_vmf.hpp>
#include <vmf_poly.hpp>
#include <vmf_poly_mesh.hpp>
#include <vmf_entity_data.hpp>
#include <unordered_set>

#ifdef _WIN32
#define ENABLE_BSP_SUPPORT
#endif

#ifdef ENABLE_BSP_SUPPORT
#include <util_bsp.hpp>
#endif

extern DLLENGINE Engine *engine;

static std::string invert_x_axis(std::string str)
{
	if(str.empty())
		return str;
	if(str[0] != '-')
		str = "-" +str;
	else
		str = str.substr(1,str.length());
	return str;
}

static std::string transform_angles(std::string val)
{
	val = invert_x_axis(val);
	std::vector<std::string> vdat;
	ustring::explode(val,ustring::WHITESPACE.c_str(),vdat);
	if(vdat.size() > 0)
	{
		float f = static_cast<float>(-atof(vdat[0].c_str()));
		vdat[0] = std::to_string(f);
	}
	if(vdat.size() > 1)
	{
		float f = static_cast<float>(atof(vdat[1].c_str()));
		f += 90.f;
		vdat[1] = std::to_string(f);
		val = "";
		for(char i=0;i<vdat.size();i++)
		{
			if(i > 0)
				val += " ";
			val += vdat[i];
		}
	}
	auto ang = EulerAngles(val);
	ang.Normalize();
	return std::to_string(ang.p) +" " +std::to_string(ang.y) +" " +std::to_string(ang.r);
}

static std::string swap_yz_axes(std::string str)
{
	std::vector<std::string> vdat;
	ustring::explode(str,ustring::WHITESPACE.c_str(),vdat);
	size_t l = vdat.size();
	if(l <= 1)
		return str;
	if(l >= 2)
	{
		if(vdat[1][0] != '-')
			vdat[1] = "-" +vdat[1];
		else
			vdat[1] = vdat[1].substr(1,vdat[1].length());
	}
	if(l == 2)
		return vdat[0] +" 0 " +vdat[1];
	return vdat[0] +" " +vdat[2] +" "+vdat[1];
}

static std::string transform_model_path(std::string val)
{
	size_t br = val.find_first_of("/\\");
	if(br != size_t(-1))
	{
		std::string path = val.substr(0,br);
		StringToLower(path);
		if(path == "models")
			val = val.substr(br +1,val.length());
	}
	size_t pext = val.find_last_of('.');
	if(pext != size_t(-1))
		val = val.substr(0,pext) +".wmd";
}

static void write_mesh(VFilePtrReal &fOut,std::shared_ptr<vmf::PolyMesh> &brushMesh)
{
	std::vector<vmf::Poly*> *polys;
	brushMesh->GetPolys(&polys);
	auto bDisp = brushMesh->HasDisplacements();
	fOut->Write<pragma::level::MeshType>(bDisp ? pragma::level::MeshType::Displacement : pragma::level::MeshType::PolyMesh);

	auto &info = brushMesh->GetCompiledData();
	auto numPolys = polys->size();
	auto &verts = info.vertexList;
	auto numVerts = verts.size();

	fOut->Write<uint32_t>(numVerts);
	fOut->Write(verts.data(),sizeof(verts.front()) *verts.size());

	fOut->Write<uint32_t>(numPolys);
	for(auto *poly : *polys)
	{
		auto *texData = poly->GetTextureData();
		auto matId = poly->GetMaterialId();
		fOut->Write<uint32_t>(matId);

		auto &polyInfo = poly->GetCompiledData();
		auto nu = polyInfo.nu;
		auto nv = polyInfo.nv;
		uvec::normalize(&nu);
		uvec::normalize(&nv);
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

		auto *polyVerts = poly->GetVertices();
		auto numPolyVerts = static_cast<uint32_t>(polyVerts->size());
		fOut->Write<uint32_t>(numPolyVerts);
		for(unsigned int k=0;k<numPolyVerts;k++)
		{
			auto &va = (*polyVerts)[k]->pos;
			auto bFound = false;
			for(unsigned int l=0;l<numVerts;l++)
			{
				const auto EPSILON = 4.9406564584125e-2;
				auto &vb = verts[l];
				if(fabsf(va.x -vb.x) <= EPSILON &&
					fabsf(va.y -vb.y) <= EPSILON &&
					fabsf(va.z -vb.z) <= EPSILON)
				{
					fOut->Write<uint32_t>(l);
					bFound = true;
					break;
				}
			}
			assert(bFound == true);
			if(bFound == false)
				Con::cerr<<"ERROR: Missing vertex index."<<Con::endl;
		}

		if(bDisp)
		{
			auto &dispInfo = *polyInfo.displacement;
			fOut->Write<uint16_t>(dispInfo.powersMerged.size() +1);
			fOut->Write<uint8_t>(dispInfo.power);
			for(auto &powerMerged : dispInfo.powersMerged)
				fOut->Write<uint8_t>(powerMerged);
			if(dispInfo.vertices.size() != dispInfo.normals.size() || dispInfo.vertices.size() != dispInfo.uvs.size())
				throw std::logic_error("Displacement vertex count does not match with normal or uv count!");
			fOut->Write(dispInfo.vertices.data(),dispInfo.vertices.size() *sizeof(dispInfo.vertices.front()));
			fOut->Write(dispInfo.normals.data(),dispInfo.normals.size() *sizeof(dispInfo.normals.front()));
			fOut->Write(dispInfo.uvs.data(),dispInfo.uvs.size() *sizeof(dispInfo.uvs.front()));

			fOut->Write<uint8_t>(dispInfo.numAlpha);
			auto bHasAlphaA = (dispInfo.numAlpha > 0) ? true : false;
			if(bHasAlphaA == true)
			{
				auto bHasAlphaB = (dispInfo.numAlpha > 1) ? true : false;
				for(auto &a : dispInfo.alphas)
				{
					auto x = umath::round(a.x *255.f);
					x = (x > 255) ? 255 : ((x < 0) ? 0 : x);
					fOut->Write<uint8_t>(x);
					if(bHasAlphaB == true)
					{
						auto y = umath::round(a.y *255.f);
						y = (y > 255) ? 255 : ((y < 0) ? 0 : y);
						fOut->Write<uint8_t>(y);
					}
				}
			}
		}
		else
		{
			for(auto *v : *polyVerts)
			{
				fOut->Write<float>(v->normal.x);
				fOut->Write<float>(v->normal.y);
				fOut->Write<float>(v->normal.z);
			}
		}
	}
}
#ifdef ENABLE_BSP_SUPPORT
static void find_model_brushes(bsp::File &bsp,const bsp::dmodel_t &mdl,std::unordered_set<uint32_t> &brushIds)
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

static void build_displacement(bsp::File &bsp,NetworkState *nw,const bsp::dDisp &disp,std::vector<std::shared_ptr<vmf::PolyMesh>> &outMeshes)
{
	auto &texInfo = bsp.GetTexInfo().at(disp.face.texinfo);
	auto &texData = bsp.GetTexData().at(texInfo.texdata);
	auto &dispInfo = disp.dispInfo;

	auto *info = new vmf::DispInfo;
	info->power = dispInfo.power;
	info->startposition = Vector3(dispInfo.startPosition.x,dispInfo.startPosition.z,-dispInfo.startPosition.y);
	info->startpositionId = std::numeric_limits<uint32_t>::max();
	//info->flags = dispInfo.flags;
	//info->elevation = dispInfo.elevation;
	//info->subdiv = dispInfo.subdiv;

	auto rows = static_cast<uint32_t>(pow(2u,info->power)) +1u;
	auto bInvalid = false;

	if(disp.verts.size() != umath::pow2(rows))
		throw std::logic_error("Discrepancy between displacement vertex count and displacement power/row count!");
	auto planeDist = disp.plane.dist;
	auto planeNormal = Vector3{disp.plane.normal.x,-disp.plane.normal.z,-disp.plane.normal.y};
	for(auto i=decltype(rows){0};i<rows;++i)
	{
		info->normals.push_back({});
		info->distances.push_back({});
		info->offsets.push_back({});
		info->alphas.push_back({});
		auto &vNormals = info->normals.back();
		auto &vDistances = info->distances.back();
		auto &vOffsets = info->offsets.back();
		auto &vAlphas = info->alphas.back();

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

	//info->startposition
	auto &texStringTable = bsp.GetTexDataStringIndices();
	auto &texStringData = bsp.GetTranslatedTexDataStrings();
	auto brushTexStringId = texStringTable.at(texData.nameStringTableID);

	auto polyMesh = std::make_shared<vmf::PolyMesh>();
	polyMesh->SetHasDisplacements(true);
	auto poly = new vmf::Poly(nw->GetMaterialManager());
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

	auto &polyVerts = *poly->GetVertices();
	for(uint8_t i=0;i<4;++i)
		polyVerts.push_back(new vmf::Vertex());
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

			auto it = std::find_if(polyVerts.begin(),polyVerts.end(),[&pos](vmf::Vertex *v) {
				return umath::abs(v->pos.x -pos.x) <= EPSILON &&
					umath::abs(v->pos.y -pos.y) <= EPSILON &&
					umath::abs(v->pos.z -pos.z) <= EPSILON;
			});
			if(it != polyVerts.end())
				continue;
			if(vertIdx >= polyVerts.size())
				throw std::logic_error("Too many displacement face vertices!");
			polyVerts.at(vertIdx++)->pos = pos;
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
		auto d = uvec::distance(v->pos,info->startposition);
		if(d < dMin)
		{
			dMin = d;
			info->startpositionId = vertexIndex;
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

static void build_brush_mesh(bsp::File &bsp,NetworkState *nw,const std::unordered_set<uint32_t> &brushIds,std::vector<std::shared_ptr<vmf::PolyMesh>> &outMeshes)
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

			auto poly = new vmf::Poly(nw->GetMaterialManager());
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
#endif
#ifdef ENABLE_BSP_SUPPORT
static std::unique_ptr<bsp::File> open_bsp_map(NetworkState *nw,const std::string &path)
{
	auto bspPath = path;
	ufile::remove_extension_from_filename(bspPath);
	bspPath += ".bsp";
	static auto *ptrOpenArchiveFile = reinterpret_cast<void(*)(const std::string&,VFilePtr&)>(util::impl::get_module_func(nw,"open_archive_file"));
	if(ptrOpenArchiveFile == nullptr)
		return nullptr;
	VFilePtr f = nullptr;
	ptrOpenArchiveFile(bspPath,f);
	if(f == nullptr)
		return nullptr;
	bsp::ResultCode code;
	return bsp::File::Open(f,code);
}
#endif
bool util::port_hl2_map(NetworkState *nw,const std::string &path)
{
#ifdef ENABLE_BSP_SUPPORT
	auto bsp = open_bsp_map(nw,path);
	if(bsp == nullptr)
	{
		auto vmfPath = path;
		ufile::remove_extension_from_filename(vmfPath);
		vmfPath += ".vmf";
		if(FileManager::Exists(vmfPath) == false)
			return false;
		Con::cout<<"Found VMF version of map: '"<<vmfPath<<"'! Compiling..."<<Con::endl;
		Con::cwar<<"----------- VMF Compile LOG -----------"<<Con::endl;
		auto r = vmf::load(*nw,vmfPath,[](const std::string &msg) {
			Con::cout<<"> "<<msg<<Con::endl;
		});
		Con::cwar<<"---------------------------------------"<<Con::endl;
		return r == vmf::ResultCode::Success;
	}
	Con::cout<<"Found BSP version of map: '"<<path<<"'! Converting..."<<Con::endl;
	Con::cwar<<"----------- BSP Conversion LOG -----------"<<Con::endl;
	auto messageLogger = std::function<void(const std::string&)>([](const std::string &msg) {
		Con::cout<<"> "<<msg<<Con::endl;
	});
	ScopeGuard sg([]() {
		Con::cwar<<"----------- BSP Conversion LOG -----------"<<Con::endl;
	});

	auto entities = bsp->GetEntities();

	if(messageLogger != nullptr)
		messageLogger("Creating output file '" +path +"'...");
	auto fullPath = IMPORT_PATH +path;
	auto pathNoFile = ufile::get_path_from_filename(fullPath);
	if(FileManager::CreatePath(pathNoFile.c_str()) == false)
	{
		if(messageLogger != nullptr)
			messageLogger("ERROR: Unable to create path '" +pathNoFile +"'!");
		return 0;
	}
	auto fOut = FileManager::OpenFile<VFilePtrReal>(fullPath.c_str(),"wb");
	if(fOut == nullptr)
	{
		if(messageLogger != nullptr)
			messageLogger("ERROR: Unable to write file '" +path +"'!");
		return 0;
	}

	const std::array<char,3> header = {'W','L','D'};
	fOut->Write(header.data(),header.size());

	fOut->Write<uint32_t>(WLD_VERSION);
	static_assert(WLD_VERSION == 9);
	auto offsetMaterial = fOut->Tell();
	fOut->Write<uint64_t>(0ull);
	auto offsetEntities = fOut->Tell();
	fOut->Write<uint64_t>(0ull);
	auto offsetBSPTree = fOut->Tell();
	fOut->Write<uint64_t>(0ull);
	auto offsetLightMapData = fOut->Tell();
	fOut->Write<uint64_t>(0ull);
	auto offsetFaceVertexData = fOut->Tell();
	fOut->Write<uint64_t>(0ull);

	const auto fWriteOffset = [&fOut](uint64_t offsetToOffset) {
		auto offset = fOut->Tell();
		fOut->Seek(offsetToOffset);
		fOut->Write<uint64_t>(offset);
		fOut->Seek(offset);
	};

	auto &brushes = bsp->GetBrushes();
	auto &brushSides = bsp->GetBrushSides();
	auto &planes = bsp->GetPlanes();
	auto &texInfo = bsp->GetTexInfo();
	auto &texData = bsp->GetTexData();
	auto &texStringTable = bsp->GetTexDataStringIndices();
	auto &texStringData = bsp->GetTranslatedTexDataStrings();
	auto &models = bsp->GetModels();
	auto &faces = bsp->GetHDRFaces().empty() ? bsp->GetFaces() : bsp->GetHDRFaces();
	auto &origFaces = bsp->GetOriginalFaces();
	auto &nodes = bsp->GetNodes();
	auto &leaves = bsp->GetLeaves();
	auto &leafBrushes = bsp->GetLeafBrushes();

	std::unordered_map<uint32_t,std::pair<uint32_t,uint32_t>> staticPropLeafRanges {};
	auto &staticPropData = bsp->GetStaticPropData();
	for(auto &lump : staticPropData.staticPropLumps)
	{
		auto &name = staticPropData.dictionaryModelNames.at(lump.PropType);
		entities.push_back(std::make_shared<vmf::DataFileBlock>());
		auto &entData = entities.back();
		entData->keyvalues["classname"] = {"prop_static"};
		entData->keyvalues["origin"] = {std::to_string(lump.Origin.x) +" " +std::to_string(lump.Origin.y) +" " +std::to_string(lump.Origin.z)};
		entData->keyvalues["angles"] = {std::to_string(lump.Angles.p) +" " +std::to_string(lump.Angles.y) +" " +std::to_string(lump.Angles.r)};
		entData->keyvalues["skin"] = {std::to_string(lump.Skin)};
		entData->keyvalues["model"] = {name};
		auto idx = entities.size() -1;
		staticPropLeafRanges[idx] = {lump.FirstLeaf,lump.LeafCount};
	}

	auto itNodraw = std::find(texStringData.begin(),texStringData.end(),std::string("TOOLS/TOOLSNODRAW"));
	if(itNodraw == texStringData.end())
		const_cast<std::vector<std::string>&>(texStringData).push_back("TOOLS/TOOLSNODRAW");

	auto itSkybox = std::find(texStringData.begin(),texStringData.end(),std::string("TOOLS/TOOLSSKYBOX"));
	auto iSkybox = (itSkybox != texStringData.end()) ? (itSkybox -texStringData.begin()) : std::string::npos;
	auto itSkybox2D = std::find(texStringData.begin(),texStringData.end(),std::string("TOOLS/TOOLSSKYBOX2D"));
	auto iSkybox2D = (itSkybox2D != texStringData.end()) ? (itSkybox2D -texStringData.begin()) : std::string::npos;

	const std::array<std::string,10> materialsRemove = {
		"TOOLS/TOOLSHINT",
		"TOOLS/TOOLSSKIP",
		"TOOLS/TOOLSAREAPORTAL",
		"TOOLS/TOOLSBLOCKLIGHT",
		"TOOLS/TOOLSINVISIBLELADDER",
		"TOOLS/TOOLSOCCLUDER",
		"TOOLS/TOOLSBLOCK_LOS",
		"TOOLS/TOOLSBLOCKBULLETS",
		"TOOLS/TOOLSFOG",

		"TOOLS/TOOLSCLIP"
	};
	std::unordered_set<uint32_t> matRemoveIndices;
	for(auto &matName : materialsRemove)
	{
		auto it = std::find(texStringData.begin(),texStringData.end(),matName);
		if(it == texStringData.end())
			continue;
		matRemoveIndices.insert(it -texStringData.begin());
	}
	const std::unordered_map<std::string,std::string> materialsReplace = {
		//{"TOOLS/TOOLSCLIP","TOOLS/TOOLSNODRAW"}
	};
	for(auto &pair : materialsReplace)
	{
		auto it = std::find(texStringData.begin(),texStringData.end(),pair.first);
		if(it == texStringData.end())
			continue;
		const_cast<std::string&>(*it) = pair.second;
	}

	struct Output
	{
		std::string name;
		std::string target;
		std::string input;
		std::string param;
		float delay = 0.;
		int32_t times = -1;
	};
	struct EntityData
	{
		std::string className;
		std::unordered_map<std::string,std::string> keyvalues;
		std::vector<Output> outputs;
		int32_t brushModelId = -1;
		Vector3 origin = {};
		std::unordered_set<uint32_t> brushIds;
		std::vector<uint32_t> faceIndices = {};

		uint32_t firstLeaf = 0u;
		uint32_t numLeaves = 0u;
	};
	//entities.push_back(world);
	std::vector<std::shared_ptr<EntityData>> outEntities;
	outEntities.reserve(entities.size() +100);

	EntityData *entWater = nullptr;

	std::unordered_set<uint32_t> worldBrushes;
	find_model_brushes(*bsp,models.front(),worldBrushes);

	if(messageLogger != nullptr)
		messageLogger("Loading FGD data...");
	auto fgdData = pragma::level::load_fgds(*nw,messageLogger);
	const std::array<std::string,7> classIgnoreList = {
		//"worldspawn",
		"env_fog_controller", // TODO
		"func_brush", // TODO
		"info_node",

		"light", // TODO
		"light_spot", // TODO
		"light_environment" // TODO
	};
	if(messageLogger != nullptr)
		messageLogger("Transforming entity data...");
	auto idx = 1u;
	for(auto &entData : entities)
	{
		auto entIdx = idx++ -1;
		const auto fGetKeyValue = [&entData](const std::string &key) -> const std::string& {
			auto it = std::find_if(entData->keyvalues.begin(),entData->keyvalues.end(),[&key](const std::pair<std::string,std::vector<std::string>> &pair) {
				return ustring::compare(key,pair.first,false);
			});
			if(it == entData->keyvalues.end() || it->second.empty())
			{
				static std::string r = "";
				return r;
			}
			return it->second.front();
		};
		auto className = fGetKeyValue("classname");
		if(className.empty())
			continue;
		if(ustring::compare(className,"worldspawn",false) == true)
		{
			entData->keyvalues.insert(std::make_pair("color",std::vector<std::string>{pragma::level::WLD_DEFAULT_AMBIENT_COLOR.ToString()}));
			auto skybox = fGetKeyValue("skyname");
			if(skybox == "painted")
				skybox = "skybox/dawn2";
			else
				skybox = "skybox/" +skybox;
			auto wmiFileName = skybox +".wmi";
			if(FileManager::Exists(wmiFileName) == false)
			{
				if(FileManager::CreatePath("materials\\skybox") == true)
				{
					auto f = FileManager::OpenFile<VFilePtrReal>((IMPORT_PATH +"materials\\" +wmiFileName).c_str(),"w");
					if(f != nullptr)
					{
						f->WriteString("\"skybox\"\n{\n");
						f->WriteString("\t$cubemap skybox \"" +skybox +"\"\n");
						f->WriteString("\t$int mipmap_load_mode -1\n}");
						f = nullptr;
					}
				}
			}
			if(itSkybox != texStringData.end())
				const_cast<std::string&>(*itSkybox) = skybox;
			if(itSkybox2D != texStringData.end())
				const_cast<std::string&>(*itSkybox2D) = skybox;
			className = "world";
		}
		auto it = std::find_if(classIgnoreList.begin(),classIgnoreList.end(),[&className](const std::string &classNameOther) {
			return ustring::compare(className,classNameOther,false);
		});
		if(it != classIgnoreList.end())
			continue;

		int32_t brushModelId = -1;
		auto brushModel = fGetKeyValue("model");
		if(brushModel.size() > 1 && brushModel.front() == '*')
			brushModelId = util::to_int(ustring::sub(brushModel,1));

		if(ustring::compare(className,"func_detail",false) == true)
		{
			if(brushModelId != -1)
				find_model_brushes(*bsp,models.at(brushModelId),worldBrushes);
			continue;
		}

		auto bWorld = className == "world";
		if(bWorld == false)
			outEntities.push_back(std::make_shared<EntityData>());
		else
			outEntities.insert(outEntities.begin(),std::make_shared<EntityData>()); // World has to be the first entity in the list!
		auto &outEntity = bWorld ? *outEntities.front() : *outEntities.back();

		auto itLeafRange = staticPropLeafRanges.find(entIdx);
		if(itLeafRange != staticPropLeafRanges.end())
		{
			outEntity.firstLeaf = itLeafRange->second.first;
			outEntity.numLeaves = itLeafRange->second.second;
		}

		outEntity.brushModelId = brushModelId;

		auto lClassName = className;
		ustring::to_lower(lClassName);
		outEntity.outputs.reserve(entData->keyvalues.size());
		std::unordered_map<std::string,std::string> originalKeyValues {};
		for(auto &pair : entData->keyvalues)
		{
			if(pair.first.empty())
				continue;
			auto bKeyValue = true;
			for(auto &fgd : fgdData)
			{
				auto itClassDef = fgd.classDefinitions.find(lClassName);
				if(itClassDef == fgd.classDefinitions.end())
					continue;
				if(itClassDef->second->FindOutput(fgd,pair.first) != nullptr)
				{
					if(itClassDef->second->FindKeyValue(fgd,pair.first) == nullptr)
						bKeyValue = false;
					else if(messageLogger != nullptr)
						messageLogger("WARNING: Unable to determine whether '" +pair.first +"' of entity class '" +className +"' is keyvalue or output because definition could not be located in any FGD file! Treating as keyvalue...");
					break;
				}
			}
			if(bKeyValue == true)
			{
				if(pair.second.empty() == false)
					originalKeyValues.insert(std::make_pair(pair.first,pair.second.front()));
				continue;
			}
			outEntity.outputs.reserve(outEntity.outputs.size() +(pair.second.size() -1)); // We've already reserved 1 slot
			auto name = pair.first;
			ustring::to_lower(name);
			for(auto &val : pair.second)
			{
				outEntity.outputs.push_back({});
				auto &output = outEntity.outputs.back();
				output.name = name;
						
				std::vector<std::string> args;
				ustring::explode(val,",",args);
				auto numArgs = args.size();
				if(numArgs >= 2)
				{
					output.target = args.at(0);
					output.input = args.at(1);
					output.param = "";
					output.delay = 0.f;
					output.times = 1;
					if(numArgs > 2)
					{
						output.param = args.at(2);
						if(numArgs > 3)
						{
							output.delay = util::to_float(args.at(3));
							if(numArgs > 4)
								output.times = util::to_int(args.at(4));
						}
					}
				}
			}
		}

		auto originalClassName = className;
		pragma::level::transform_class(originalKeyValues,outEntity.keyvalues,className);
		auto bClassFound = false;
		for(auto &pair : outEntity.keyvalues)
		{
			pragma::level::transform_keyvalue(fgdData,originalClassName,pair.first,pair.second,[&bClassFound,&messageLogger](const std::string &msg,uint8_t msgType) {
				if(msgType == 0u)
				{
					if(bClassFound == true)
						return;
					bClassFound = true;
				}
				messageLogger(msg);
			});
		}

		outEntity.className = className;
	}

	enum class GeometryType : uint8_t
	{
		BrushGeometry = 0u,
		FaceGeometry
	};
	// Which geometry type to load from the BSP
	const auto geometryType = GeometryType::FaceGeometry;

	enum class ContentsFlags : uint64_t
	{
		Empty = 0,

		Solid = 0x1,
		Window = 0x2,
		Aux = 0x4,
		Grate = 0x8,
		Slime = 0x10,
		Water = 0x20,
		BlockLOS = 0x40,
		Opaque = 0x80,

		TestFogVolume = 0x100,
		Unused = 0x200,

		Unused6 = 0x400,

		Team1 = 0x800,
		Team2 = 0x1000,

		IgnoreNodrawOpaque = 0x2000,
		Moveable = 0x4000,
		AreaPortal = 0x8000,
		PlayerClip = 0x10000,
		MonsterClip = 0x20000,

		Current0 = 0x40000,
		Current90 = 0x80000,
		Current180 = 0x100000,
		Current270 = 0x200000,
		CurrentUp = 0x400000,
		CurrentDown = 0x800000,

		Origin = 0x1000000,

		Monster = 0x2000000,
		Debris = 0x4000000,
		Detail = 0x8000000,
		Translucent = 0x10000000,
		Ladder = 0x20000000,
		Hitbox = 0x40000000
	};
	enum class SurfFlags : uint32_t
	{
		Light = 0x0001,
		Sky2D = 0x0002,
		Sky = 0x0004,
		Warp = 0x0008,
		Trans = 0x0010,
		NoPortal = 0x0020,
		Trigger = 0x0040,
		Nodraw = 0x0080,

		Hint = 0x0100,

		Skip = 0x0200,
		NoLight = 0x0400,
		BumpLight = 0x0800,
		NoShadows = 0x1000,
		NoDecals = 0x2000,
		NoChop = 0x400,
		Hitbox = 0x8000
	};
	auto itWorld = std::find_if(outEntities.begin(),outEntities.end(),[](const std::shared_ptr<EntityData> entData) {
		return ustring::compare(entData->className,"world",false);
	});
	auto *entWorld = (itWorld != outEntities.end()) ? itWorld->get() : nullptr;

	auto itEntSkybox = std::find_if(outEntities.begin(),outEntities.end(),[](const std::shared_ptr<EntityData> entData) {
		return ustring::compare(entData->className,"skybox",false);
	});
	auto *entSkybox = (itEntSkybox != outEntities.end()) ? itEntSkybox->get() : nullptr;

	if(entWorld != nullptr)
	{
		entWorld->faceIndices.reserve(faces.size());
		entWorld->brushModelId = 0u; // First brush model is always worldspawn geometry
	}
	for(auto &outEntity : outEntities)
	{
		if(outEntity->brushModelId == -1)
			continue;
		//auto bWorld = outEntity.get() == entWorld;
		auto &brushModel = models.at(outEntity->brushModelId);
		outEntity->faceIndices.reserve(brushModel.numfaces);
		for(auto faceIndex=brushModel.firstface;faceIndex<(brushModel.firstface +brushModel.numfaces);++faceIndex)
		{
			auto &face = faces.at(faceIndex);
			auto &faceTexInfo = texInfo.at(face.texinfo);
			if(faceTexInfo.flags &umath::to_integral(SurfFlags::Sky))
			{
				if(entSkybox == nullptr)
				{
					outEntities.push_back(std::make_shared<EntityData>());
					entSkybox = outEntities.back().get();
					entSkybox->className = "skybox";
				}
				entSkybox->faceIndices.push_back(faceIndex);
			}
			else if(outEntity != nullptr)
				outEntity->faceIndices.push_back(faceIndex);
		}
	}

	std::vector<std::shared_ptr<vmf::PolyMesh>> outBrushes;
	util::bsp::GeometryData bspGeometryData {};
	//if(geometryType == GeometryType::BrushGeometry)
	//{
		// Build world geometry
		outBrushes.reserve(worldBrushes.size());
		for(auto it=worldBrushes.begin();it!=worldBrushes.end();)
		{
			auto idx = *it;
			auto &brush = brushes.at(idx);
			enum class BrushType : uint32_t
			{
				World = 0,
				Skybox,
				Water,
				Remove
			};
			auto type = BrushType::World;
			if(brush.contents &0x20)
				type = BrushType::Water;
			else
			{
				for(auto i=brush.firstside;i<brush.firstside +brush.numsides;++i)
				{
					auto &brushSide = brushSides.at(i);
					if(brushSide.texinfo == -1)
					{
						//brushSide.texinfo = iNodraw;
						//break;
						continue;
					}
					auto &brushTexInfo = texInfo.at(brushSide.texinfo);
					if(brushTexInfo.texdata == -1)
						continue;
					auto &brushTexData = texData.at(brushTexInfo.texdata);
					auto brushTexStringId = texStringTable.at(brushTexData.nameStringTableID);
					auto &brushTexString = texStringData.at(brushTexStringId);
				
					if(brushTexStringId == iSkybox || brushTexStringId == iSkybox2D)
					{
						type = BrushType::Skybox;
						break;
					}
					else
					{
						auto it = matRemoveIndices.find(brushTexStringId);
						if(it != matRemoveIndices.end())
						{
							type = BrushType::Remove;
							break;
						}
					}


				}
			}

			if(type == BrushType::Skybox)
			{
				if(entSkybox == nullptr)
				{
					outEntities.push_back(std::make_shared<EntityData>());
					entSkybox = outEntities.back().get();
					entSkybox->className = "skybox";
				}
				entSkybox->brushIds.insert(idx);

				it = worldBrushes.erase(it);
			}
			else if(type == BrushType::Water)
			{
				if(entWater == nullptr)
				{
					outEntities.push_back(std::make_shared<EntityData>());
					entWater = outEntities.back().get();
					entWater->className = "func_water";
				}
				entWater->brushIds.insert(idx);

				it = worldBrushes.erase(it);
			}
			else if(type == BrushType::Remove)
				it = worldBrushes.erase(it); // Remove all brushes of this type entirely
			else
				++it;
		}

		build_brush_mesh(*bsp,nw,worldBrushes,outBrushes);

		auto &displacements = bsp->GetDisplacements();
		for(auto &disp : displacements)
			build_displacement(*bsp,nw,disp,outBrushes);

		for(auto &mesh : outBrushes)
		{
			if(mesh->HasDisplacements() == false)
				continue;
			std::vector<vmf::Poly*> *polys;
			mesh->GetPolys(&polys);
			for(auto *poly : *polys)
			{
				auto &polyInfo = poly->GetCompiledData();
				if(poly->IsDisplacement() == false)
					continue;
				auto &displacement = *polyInfo.displacement.get();
				vmf::PolyMesh::BuildDisplacementNormals(outBrushes,mesh.get(),poly,polyInfo,displacement.normals);
			}
		}
		//
	//}
	//else
		bspGeometryData = util::bsp::load_bsp_geometry(*nw,*bsp);

	// Write materials
	if(messageLogger != nullptr)
		messageLogger("Writing materials...");
	fWriteOffset(offsetMaterial);
	fOut->Write<uint32_t>(texStringData.size());
	for(auto &str : texStringData)
	{
		auto texture = FileManager::GetCanonicalizedPath(str);
		ustring::to_lower(texture);
		if(ustring::substr(texture,0,10) == std::string("materials") +FileManager::GetDirectorySeparator())
			texture = texture.substr(10);
		fOut->WriteString(texture);
	}

	// Write BSP tree
	if(messageLogger != nullptr)
		messageLogger("Writing BSP Tree...");
	fWriteOffset(offsetBSPTree);
	auto bBSPTree = geometryType == GeometryType::FaceGeometry;
	fOut->Write<bool>(bBSPTree);
	if(bBSPTree == true)
	{
		auto bspTree = util::BSPTree::Create(*bsp);
		auto numClusters = bspTree.GetClusterCount();
		auto &bspNodes = bspTree.GetNodes();
		auto &clusterVisibility = bspTree.GetClusterVisibility();
		std::function<void(const util::BSPTree::Node&)> fWriteNode = nullptr;
		fWriteNode = [&fOut,&fWriteNode,&clusterVisibility,&bspNodes,&bspTree,numClusters](const util::BSPTree::Node &node) {
			fOut->Write<bool>(node.leaf);
			fOut->Write<Vector3>(node.min);
			fOut->Write<Vector3>(node.max);
			fOut->Write<int32_t>(node.firstFace);
			fOut->Write<int32_t>(node.numFaces);
			fOut->Write<int32_t>(node.originalNodeIndex);
			if(node.leaf)
			{
				fOut->Write<uint16_t>(node.cluster);
				auto itNode = std::find_if(bspNodes.begin(),bspNodes.end(),[&node](const std::shared_ptr<util::BSPTree::Node> &nodeOther) {
					return nodeOther.get() == &node;
				});
				// Calculate AABB encompassing all nodes visible by this node
				auto min = node.min;
				auto max = node.max;
				if(itNode != bspNodes.end())
				{
					for(auto clusterDst=decltype(numClusters){0u};clusterDst<numClusters;++clusterDst)
					{
						if(bspTree.IsClusterVisible(node.cluster,clusterDst) == false)
							continue;
						for(auto &nodeOther : bspNodes)
						{
							if(nodeOther->cluster != clusterDst)
								continue;
							uvec::to_min_max(min,max,nodeOther->min,nodeOther->max);
						}
					}
				}
				fOut->Write<Vector3>(min);
				fOut->Write<Vector3>(max);
				return;
			}
			fOut->Write<Vector3>(node.plane.GetNormal());
			fOut->Write<float>(node.plane.GetDistance());

			fWriteNode(*node.children.at(0));
			fWriteNode(*node.children.at(1));
		};
		fWriteNode(bspTree.GetRootNode());

		fOut->Write<uint64_t>(bspTree.GetClusterCount());
		fOut->Write(clusterVisibility.data(),clusterVisibility.size() *sizeof(clusterVisibility.front()));

		/*auto numClusters = clusterVisibility.size();
		auto numCompressedClusters = umath::pow2(numClusters);
		numCompressedClusters = numCompressedClusters /8u +((numCompressedClusters %8u) > 0u ? 1u : 0u);
		std::vector<uint8_t> compressedClusterVis(numCompressedClusters,0u);
		auto offset = 0ull; // Offset in bits(!)
		for(auto &cluster : clusterVisibility)
		{
			for(auto vis : cluster)
			{
				auto &visCompressed = compressedClusterVis.at(offset /8u);
				if(vis)
					visCompressed |= 1<<(offset %8u);
				++offset;
			}
		}*/

		// Validation
		/*for(auto clusterSrc=decltype(numClusters){0u};clusterSrc<numClusters;++clusterSrc)
		{
			for(auto clusterDst=decltype(numClusters){0u};clusterDst<numClusters;++clusterDst)
			{
				auto bit = clusterSrc *numClusters +clusterDst;
				auto offset = bit /8u;
				bit %= 8u;
				auto visibility = (compressedClusterVis.at(offset) &(1<<bit)) != 0;
				if(visibility != clusterVisibility.at(clusterSrc).at(clusterDst))
					throw std::runtime_error("Validation error!");
			}
		}

		std::vector<std::vector<uint8_t>> decompressedClusters(numClusters,std::vector<uint8_t>(numClusters,0u));
		auto cluster0 = 0ull;
		auto cluster1 = 0ull;
		for(auto i=decltype(compressedClusterVis.size()){0u};i<compressedClusterVis.size();++i)
		{
			auto vis = compressedClusterVis.at(i);
			for(auto j=0u;j<8u;++j)
			{
				if(vis &(1<<j))
					decompressedClusters.at(cluster0).at(cluster1) = 1u;
				if(++cluster1 == numClusters)
				{
					++cluster0;
					cluster1 = 0ull;
				}
			}
		}

		for(auto i=decltype(clusterVisibility.size()){0u};i<clusterVisibility.size();++i)
		{
			auto &cv0 = clusterVisibility.at(i);
			auto &cv1 = decompressedClusters.at(i);
			for(auto j=decltype(cv0.size()){0u};j<cv0.size();++j)
			{
				if(cv0.at(j) != cv1.at(j))
					throw std::runtime_error("Validation error!");
			}
		}*/
	}

	// Write Lightmap data
	if(messageLogger != nullptr)
		messageLogger("Writing light map data...");
	fWriteOffset(offsetLightMapData);
	fOut->Write<bool>(geometryType == GeometryType::FaceGeometry); // Light map data is only valid if we're using the map faces as geometry
	fOut->Write<uint32_t>(bspGeometryData.lightMapData.atlasSize);
	fOut->Write<uint8_t>(bspGeometryData.lightMapData.borderSize);
	auto &rects = bspGeometryData.lightMapData.lightmapAtlas;
	fOut->Write<uint32_t>(rects.size());
	for(auto &rect : rects)
	{
		fOut->Write<uint16_t>(rect.x);
		fOut->Write<uint16_t>(rect.y);
		fOut->Write<uint16_t>(rect.w);
		fOut->Write<uint16_t>(rect.h);
	}

	// Write luxel data (bsp::ColorRGBExp32)
	if(geometryType == GeometryType::BrushGeometry)
		fOut->Write<uint64_t>(0ull);
	else
	{
		fOut->Write<uint64_t>(bspGeometryData.lightMapData.luxelData.size());
		fOut->Write(bspGeometryData.lightMapData.luxelData.data(),bspGeometryData.lightMapData.luxelData.size() *sizeof(bspGeometryData.lightMapData.luxelData.front()));
	}

	// Write map face data
	if(messageLogger != nullptr)
		messageLogger("Writing map face data...");
	fWriteOffset(offsetFaceVertexData);
	// These are taken 1:1 from the source bsp file
	if(geometryType == GeometryType::BrushGeometry)
	{
		fOut->Write<uint32_t>(0u);
		fOut->Write<uint32_t>(0u);
		fOut->Write<uint32_t>(0u);
		fOut->Write<uint32_t>(0u);
		fOut->Write<uint64_t>(0ull);
	}
	else
	{
		auto &surfEdges = bsp->GetSurfEdges();
		fOut->Write<uint32_t>(surfEdges.size());
		fOut->Write(surfEdges.data(),surfEdges.size() *sizeof(surfEdges.front()));

		auto &edges = bsp->GetEdges();
		fOut->Write<uint32_t>(edges.size());
		fOut->Write(edges.data(),edges.size() *sizeof(edges.front()));

		auto &verts = bsp->GetVertices();
		fOut->Write<uint32_t>(verts.size());
		fOut->Write(verts.data(),verts.size() *sizeof(verts.front()));

		auto &texInfo = bsp->GetTexInfo();
		std::vector<bsp::TexInfo> simpleTexInfo {};
		simpleTexInfo.reserve(texInfo.size());
		for(auto &info : texInfo)
		{
			simpleTexInfo.push_back({});
			auto &simpleInfo = simpleTexInfo.back();
			simpleInfo.lightMapVecs = {
				Vector4{info.lightmapVecs.at(0).at(0),info.lightmapVecs.at(0).at(1),info.lightmapVecs.at(0).at(2),info.lightmapVecs.at(0).at(3)},
				Vector4{info.lightmapVecs.at(1).at(0),info.lightmapVecs.at(1).at(1),info.lightmapVecs.at(1).at(2),info.lightmapVecs.at(1).at(3)}
			};
			simpleInfo.textureVecs = {
				Vector4{info.textureVecs.at(0).at(0),info.textureVecs.at(0).at(1),info.textureVecs.at(0).at(2),info.textureVecs.at(0).at(3)},
				Vector4{info.textureVecs.at(1).at(0),info.textureVecs.at(1).at(1),info.textureVecs.at(1).at(2),info.textureVecs.at(1).at(3)}
			};
			simpleInfo.materialIndex = (info.texdata != -1) ? texStringTable.at(texData.at(info.texdata).nameStringTableID) : -1;
		}
		
		fOut->Write<uint32_t>(simpleTexInfo.size());
		fOut->Write(simpleTexInfo.data(),simpleTexInfo.size() *sizeof(simpleTexInfo.front()));

		fOut->Write<uint64_t>(bspGeometryData.lightMapData.faceInfos.size());
		for(auto &faceInfo : bspGeometryData.lightMapData.faceInfos)
		{
			fOut->Write<util::bsp::FaceLightMapInfo::Flags>(faceInfo.flags);
			if((faceInfo.flags &util::bsp::FaceLightMapInfo::Flags::Valid) == util::bsp::FaceLightMapInfo::Flags::None)
				continue;
			fOut->Write<int32_t>(faceInfo.lightMapSize.at(0));
			fOut->Write<int32_t>(faceInfo.lightMapSize.at(1));
			fOut->Write<int32_t>(faceInfo.lightMapMins.at(0));
			fOut->Write<int32_t>(faceInfo.lightMapMins.at(1));
			fOut->Write<uint32_t>(faceInfo.luxelDataOffset);
			fOut->Write<int16_t>(faceInfo.texInfoIndex);
			fOut->Write<int16_t>(faceInfo.dispInfoIndex);

			fOut->Write<uint32_t>(faceInfo.firstEdge);
			fOut->Write<uint16_t>(faceInfo.numEdges);

			auto &face = faces.at(faceInfo.faceIndex);
			auto &plane = planes.at(face.planenum);
			auto n = plane.normal;
			if(face.side)
				n = -n;
			fOut->Write<Vector3>(n);
		}

		auto &leafFaces = bsp->GetLeafFaces();
		fOut->Write<uint64_t>(leafFaces.size());
		fOut->Write(leafFaces.data(),leafFaces.size() *sizeof(leafFaces.front()));

		auto &displacements = bsp->GetDisplacements();
		fOut->Write<uint32_t>(displacements.size());
		for(auto &disp : displacements)
		{
			static_assert(sizeof(bsp::DispInfo::VertInfo) == sizeof(::bsp::dDispVert));
			fOut->Write<int32_t>(disp.dispInfo.power);
			fOut->Write<int32_t>(disp.dispInfo.DispVertStart);
			fOut->Write<int32_t>(disp.dispInfo.LightmapSamplePositionStart);
			fOut->Write<Vector3>(disp.dispInfo.startPosition);
			fOut->Write<uint32_t>(disp.verts.size());
			fOut->Write(disp.verts.data(),disp.verts.size() *sizeof(disp.verts.front()));
		}

		auto &dispLightmapSamplePositions = bsp->GetDispLightmapSamplePositions();
		fOut->Write<uint64_t>(dispLightmapSamplePositions.size());
		fOut->Write(dispLightmapSamplePositions.data(),dispLightmapSamplePositions.size() *sizeof(dispLightmapSamplePositions.front()));
	}

	// Write entities
	if(messageLogger != nullptr)
		messageLogger("Writing entity data...");
	fWriteOffset(offsetEntities);
	auto offsetEntityCount = fOut->Tell();
	fOut->Write<uint32_t>(outEntities.size());

	for(auto &outEntity : outEntities)
	{
		auto offsetEndOfEntity = fOut->Tell();
		fOut->Write<uint64_t>(0u); // Offset to end of entity
		auto offsetEntityMeshes = fOut->Tell();
		fOut->Write<uint64_t>(0u); // Offset to entity meshes
		auto offsetEntityLeaves = fOut->Tell();
		fOut->Write<uint64_t>(0u); // Offset to entity leaves

		fOut->Write<uint64_t>(0u); // Entity flags

		fOut->WriteString(outEntity->className);

		// Keyvalues
		fOut->Write<uint32_t>(outEntity->keyvalues.size());
		for(auto &pair : outEntity->keyvalues)
		{
			fOut->WriteString(pair.first);
			fOut->WriteString(pair.second);
		}

		// Outputs
		fOut->Write<uint32_t>(outEntity->outputs.size());
		for(auto &output : outEntity->outputs)
		{
			fOut->WriteString(output.name);
			fOut->WriteString(output.target);
			fOut->WriteString(output.input);
			fOut->WriteString(output.param);
			fOut->Write<float>(output.delay);
			fOut->Write<int32_t>(output.times);
		}
		
		// Custom Components
		std::unordered_set<std::string> components {};
		pragma::level::find_entity_components(outEntity->keyvalues,components);
		if(outEntity->outputs.empty() == false)
			components.insert("io");
		fOut->Write<uint32_t>(components.size());
		for(auto &name : components)
			fOut->WriteString(name);

		const ::bsp::dmodel_t *mdl = nullptr;
		if(outEntity->brushModelId != -1 && outEntity->brushModelId < models.size())
		{
			mdl = &models.at(outEntity->brushModelId);

			find_model_brushes(*bsp,*mdl,outEntity->brushIds);
			outEntity->origin = Vector3(mdl->origin.x,mdl->origin.z,-mdl->origin.y);
		}

		std::vector<std::shared_ptr<vmf::PolyMesh>> *polyMeshes = nullptr;
		std::vector<std::shared_ptr<vmf::PolyMesh>> tmpPolyMeshes {};
		auto bBrushEntity = false;
		//if(geometryType == GeometryType::BrushGeometry)
		{
			if(outEntity->className == "world")
				polyMeshes = &outBrushes;
			else
			{
				polyMeshes = &tmpPolyMeshes;
				build_brush_mesh(*bsp,nw,outEntity->brushIds,tmpPolyMeshes);
			}
			bBrushEntity = polyMeshes != nullptr && polyMeshes->empty() == false;
		}
		//else
		//	bBrushEntity = outEntity->className == "world"; // TODO

		// Entity origin
		Vector3 origin {};
		if(false)//bBrushEntity == true)
			origin = outEntity->origin;
		else
		{
			auto itOrigin = outEntity->keyvalues.find("origin");
			if(itOrigin != outEntity->keyvalues.end())
				origin = uvec::create(itOrigin->second);
			/*else if(mdl != nullptr && outEntity->className != "world")
			{
				auto mins = Vector3{mdl->mins.x,mdl->mins.z,-mdl->mins.y};
				auto maxs = Vector3{mdl->maxs.x,mdl->maxs.z,-mdl->maxs.y};
				origin = (mins +maxs) /2.f;
			}*/
			else
				origin = outEntity->origin;
		}
		fOut->Write<Vector3>(origin);

		// Leaves
		auto cur = fOut->Tell();
		fOut->Seek(offsetEntityLeaves);
			fOut->Write<uint64_t>(cur -offsetEntityLeaves);
		fOut->Seek(cur);

		fOut->Write<uint32_t>(outEntity->numLeaves);
		std::vector<uint16_t> leaves {};
		leaves.resize(outEntity->numLeaves);
		if(outEntity->numLeaves > 0u)
			memcpy(leaves.data(),staticPropData.leaves.data() +outEntity->firstLeaf,leaves.size() *sizeof(leaves.front()));
		fOut->Write(leaves.data(),leaves.size() *sizeof(leaves.front()));

		// Meshes
		cur = fOut->Tell();
		fOut->Seek(offsetEntityMeshes);
			fOut->Write<uint64_t>(cur -offsetEntityMeshes);
		fOut->Seek(cur);

		auto numPolys = (polyMeshes != nullptr) ? polyMeshes->size() : 0ull;
		fOut->Write<uint32_t>(numPolys);
		if(bBrushEntity == true)
		{
			for(auto i=decltype(numPolys){0};i<numPolys;++i)
			{
				auto &mesh = polyMeshes->at(i);
				write_mesh(fOut,mesh);
			}
		}
		
		auto &faceIndices = outEntity->faceIndices;
		if(faceIndices.empty())
			fOut->Write<uint32_t>(0u);
		else
		{
			fOut->Write<uint32_t>(faceIndices.size());
			fOut->Write(faceIndices.data(),faceIndices.size() *sizeof(faceIndices.front()));
		}

		cur = fOut->Tell();
		fOut->Seek(offsetEndOfEntity);
			fOut->Write<uint64_t>(cur -offsetEndOfEntity);
		fOut->Seek(cur);
	}

	if(messageLogger != nullptr)
	{
		messageLogger("Done!");
		messageLogger("All operations are complete!");
	}
	return true;
#else
	return false;
#endif
}
