#include "stdafx_shared.h"
#include "pragma/level/mapgeometry.h"
#include "pragma/model/modelmesh.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/model/model.h"
#include "pragma/util/util_bsp_tree.hpp"
#include <util_bsp.hpp>

static uint32_t get_next_row_neighbor(uint32_t vertId,uint32_t power,uint8_t decimateAmount)
{
	return vertId +power *decimateAmount;
}

static uint32_t get_next_column_neighbor(uint32_t vertId,uint32_t,uint8_t decimateAmount)
{
	return vertId +1 *decimateAmount;
}

static void get_vertex_quad(uint32_t vertId,uint32_t power,uint8_t decimateAmount,std::vector<uint16_t> &r)
{
	r.reserve(r.size() +6);

	// Triangle #1
	auto offset = r.size();
	r.push_back(static_cast<uint16_t>(vertId));
	r.push_back(static_cast<uint16_t>(get_next_column_neighbor(vertId,power,decimateAmount)));
	r.push_back(static_cast<uint16_t>(get_next_row_neighbor(vertId,power,decimateAmount)));

	// Triangle #2
	r.push_back(static_cast<uint16_t>(r[offset +2]));
	r.push_back(static_cast<uint16_t>(r[offset +1]));
	r.push_back(static_cast<uint16_t>(get_next_column_neighbor(r[offset +2],power,decimateAmount)));
}

bool decimate_displacement_geometry(const std::vector<uint16_t> &indices,uint32_t power,std::vector<uint16_t> &outIndices,uint8_t decimateAmount)
{
	UNUSED(indices);
	assert(decimateAmount > 1 && (indices.size() %decimateAmount) == 0);
	if(decimateAmount <= 1)
		return false;
	auto &indicesDecimated = outIndices;
	auto iterations = (power *(power -1)) -decimateAmount;
	indicesDecimated.reserve(iterations *6);
	for(auto i=decltype(iterations){0};i<iterations;)
	{
		get_vertex_quad(i,power,decimateAmount,indicesDecimated);
		if(((i +1 +decimateAmount) %power) == 0)
			i += decimateAmount +1 +power *(decimateAmount -1);
		else
			i += decimateAmount;
	}
	return true;
}

////////////////////////////

pragma::level::BSPInputData::BSPInputData()
	: bspTree{bspTree.Create()}
{}

void pragma::level::load_map_materials(NetworkState *state,VFilePtr f,std::vector<Material*> &materials)
{
	unsigned int numMaterials = f->Read<unsigned int>();
	for(unsigned int i=0;i<numMaterials;i++)
	{
		std::string mat = f->ReadString();
		materials.push_back(state->LoadMaterial(mat.c_str()));
	}
}

static Vector2 calc_disp_surf_coords(const std::array<Vector2,4> &texCoords,uint32_t power,uint32_t x,uint32_t y)
{
	// See CCoreDispInfo::CalcDispSurfCoords from source sdk code
	auto postSpacing = ((1<<power) +1);
    auto ooInt = 1.0f /static_cast<float>(postSpacing -1);
	std::array<Vector2,2> edgeInt = {
		texCoords.at(1) -texCoords.at(0),
		texCoords.at(2) -texCoords.at(3)
	};
	edgeInt.at(0) *= ooInt;
	edgeInt.at(1) *= ooInt;

	std::array<Vector2,2> endPts = {
		edgeInt.at(0) *static_cast<float>(x) +texCoords.at(0),
		edgeInt.at(1) *static_cast<float>(x) +texCoords.at(3)
	};
	auto seg = endPts.at(1) -endPts.at(0);
	auto segInt = seg *ooInt;
	seg = segInt *static_cast<float>(y);
	return endPts.at(0) +seg;
}

void pragma::level::load_map_faces(Game &game,VFilePtr f,BaseEntity &ent,const BSPInputData &bspInputData,const std::vector<Material*> &materials,std::vector<std::vector<Vector2>> *outMeshLightMapUvCoordinates)
{
	auto numFaces = f->Read<uint32_t>();
	if(numFaces == 0u)
		return;
	std::vector<uint32_t> faceIndices {};
	faceIndices.resize(numFaces);
	f->Read(faceIndices.data(),faceIndices.size() *sizeof(faceIndices.front()));

	auto &lightMapInfo = bspInputData.lightMapInfo;
	auto borderSize = lightMapInfo.borderSize;
	auto lightmapAtlasExtents = lightMapInfo.atlasSize;
	using MaterialIndex = int16_t;
	struct BSPTriangleInfo
	{
		MaterialIndex materialIndex = 0u;
		uint32_t faceIndex = 0u;
		std::array<Vertex,3u> vertices;
		std::array<Vector2,3u> lightMapUvs;
	};
	std::vector<BSPTriangleInfo> bspTriangles {};
	auto &faces = bspInputData.lightMapInfo.faceInfos;
	auto &texInfo = bspInputData.texInfo;
	auto &surfEdges = bspInputData.surfEdges;
	auto &verts = bspInputData.verts;
	auto &edges = bspInputData.edges;
	std::vector<std::pair<uint64_t,uint64_t>> displacementRanges {};
	for(auto faceIndex : faceIndices)
	{
		auto &face = faces.at(faceIndex);
		if(face.numEdges == 0u)
			continue;
		auto &faceTexInfo = texInfo.at(face.texInfoIndex); // TODO: face.texInfoIndex == -1?

		auto texId = faceTexInfo.materialIndex;
		auto *mat = materials.at(texId);
		auto *diffuseMap = (mat != nullptr) ? mat->GetDiffuseMap() : nullptr;
		if(diffuseMap == nullptr)
			; // TODO: Print warning
		auto texWidth = (diffuseMap != nullptr) ? diffuseMap->width : 0u;
		auto texHeight = (diffuseMap != nullptr) ? diffuseMap->height : 0u;
		if(texWidth == 0u)
			texWidth = 512u;
		if(texHeight == 0u)
			texHeight = 512u;

		Vector3 texVec0 = Vector3{faceTexInfo.textureVecs.at(0).x,faceTexInfo.textureVecs.at(0).y,faceTexInfo.textureVecs.at(0).z};
		umath::swap(texVec0.y,texVec0.z);
		umath::negate(texVec0.z);

		Vector3 texVec1 = Vector3{faceTexInfo.textureVecs.at(1).x,faceTexInfo.textureVecs.at(1).y,faceTexInfo.textureVecs.at(1).z};
		umath::swap(texVec1.y,texVec1.z);
		umath::negate(texVec1.z);

		const auto fCalculateUv = [&texVec0,&texVec1,&faceTexInfo,texWidth,texHeight](const Vector3 &pos) -> Vector2 {
			Vector2 vertUv{};
			vertUv.x = uvec::dot(texVec0,pos) +faceTexInfo.textureVecs.at(0).w;
			vertUv.y = uvec::dot(texVec1,pos) +faceTexInfo.textureVecs.at(1).w;
			vertUv.x /= static_cast<float>(texWidth);
			vertUv.y /= static_cast<float>(texHeight);
			return vertUv;
		};

		auto widthLightmap = face.lightMapSize.at(0);
		auto heightLightmap = face.lightMapSize.at(1);
		auto &lv0 = faceTexInfo.lightMapVecs.at(0);
		Vector3 lightVec0 = Vector3{lv0.x,lv0.y,lv0.z};
		umath::swap(lightVec0.y,lightVec0.z);
		umath::negate(lightVec0.z);

		auto &lv1 = faceTexInfo.lightMapVecs.at(1);
		Vector3 lightVec1 = Vector3{lv1.x,lv1.y,lv1.z};
		umath::swap(lightVec1.y,lightVec1.z);
		umath::negate(lightVec1.z);

		auto &lmInfo = lightMapInfo.faceInfos.at(faceIndex);

		const auto fLightmapUvToAtlasUv = [&lmInfo,widthLightmap,heightLightmap,borderSize,lightmapAtlasExtents](const Vector2 &lightMapUv) {
			auto bRotated = (lmInfo.flags &util::bsp::FaceLightMapInfo::Flags::Rotated) != util::bsp::FaceLightMapInfo::Flags::None;

			auto result = lightMapUv;
			if(bRotated)
				result = {1.f -result.y,result.x};
			result.x = result.x *widthLightmap +lmInfo.x +borderSize;
			result.y = result.y *heightLightmap +lmInfo.y +borderSize;

			result.x /= static_cast<float>(lightmapAtlasExtents);
			result.y /= static_cast<float>(lightmapAtlasExtents);
			return result;
		};

		const auto fCalcLightmapUv = [&lightVec0,&lightVec1,&face,&lv0,&lv1,&fLightmapUvToAtlasUv,&lmInfo,widthLightmap,heightLightmap](const Vector3 &pos,const std::function<void(Vector2&)> &uvModifier=nullptr) {
			auto lu = uvec::dot(lightVec0,pos) +lv0.w -(face.lightMapMins.at(0));
			auto lv = uvec::dot(lightVec1,pos) +lv1.w -(face.lightMapMins.at(1));

			auto bRotated = (lmInfo.flags &util::bsp::FaceLightMapInfo::Flags::Rotated) != util::bsp::FaceLightMapInfo::Flags::None;
			if(bRotated == false)
			{
				lu /= static_cast<float>(widthLightmap);
				lv /= static_cast<float>(heightLightmap);
			}
			else
			{
				// Divide by the ORIGINAL image bounds
				lu /= static_cast<float>(heightLightmap);
				lv /= static_cast<float>(widthLightmap);
			}

			return fLightmapUvToAtlasUv({lu,lv});
		};

		if(face.dispInfoIndex != -1)
		{
			if(face.numEdges == 0u) // Some faces have no edges? Unsure what these are for.
				continue;
			// This is a displacement
			if(face.numEdges != 4u)
			{
				// Displacements must only have exactly 4 edges
				Con::cwar<<"WARNING: Corrupted displacement with "<<face.numEdges<<" edges! Skipping..."<<Con::endl;
				continue;
			}
			auto &dispInfo = bspInputData.displacementInfo.at(face.dispInfoIndex);

			auto dispStartPos = dispInfo.startPosition;
			umath::swap(dispStartPos.y,dispStartPos.z);
			umath::negate(dispStartPos.z);
			// We need the face vertices to determine the bounds of the displacement
			auto dMax = std::numeric_limits<float>::max();
			auto startPosIdx = 0u;
			std::array<Vector3,4u> faceVerts {};
			for(auto i=decltype(face.numEdges){0u};i<face.numEdges;++i)
			{
				auto iEdge = surfEdges.at(face.firstEdge +i);
				Vector3 v;
				if(iEdge >= 0)
					v = verts.at(edges.at(iEdge).v.at(0));
				else
					v = verts.at(edges.at(-iEdge).v.at(1));
				umath::swap(v.y,v.z);
				umath::negate(v.z);
				faceVerts.at(i) = v;

				auto d = uvec::distance_sqr(dispStartPos,faceVerts.at(i));
				if(d < dMax)
				{
					// We need to determine which of the face vertices is the start position
					dMax = d;
					startPosIdx = i;
				}
			}

			// Find order for face vertices so that start position is first in the list
			std::array<uint8_t,4u> faceVertOrder {};
			auto idx = 0u;
			for(auto i=startPosIdx;i<(startPosIdx +4u);++i)
				faceVertOrder.at(idx++) = i %4;

			auto numRows = umath::pow(2,dispInfo.power) +1;
			auto numVerts = umath::pow2(numRows);
			std::vector<std::pair<Vector3,Vector3>> points {}; // Pairs of positions +offsets
			points.reserve(numVerts);
			for(auto x=decltype(numRows){0};x<numRows;++x)
			{
				auto u = x /static_cast<float>(numRows -1);
				for(auto y=decltype(numRows){0};y<numRows;++y)
				{
					auto v = y /static_cast<float>(numRows -1);
					// Calculate flat point of the displacement by interpolation the face vertices
					auto p0 = faceVerts.at(faceVertOrder.at(0)) +(faceVerts.at(faceVertOrder.at(1)) -faceVerts.at(faceVertOrder.at(0))) *u;
					auto p1 = faceVerts.at(faceVertOrder.at(3)) +(faceVerts.at(faceVertOrder.at(2)) -faceVerts.at(faceVertOrder.at(3))) *u;
					auto posCur = p0 +(p1 -p0) *v;

					// Calculate displacement offset relative to the flat point
					auto vertIdx = x *numRows +y;
					auto &dispVert = dispInfo.vertices.at(vertIdx);
					auto offset = dispVert.vec *dispVert.dist;
					umath::swap(offset.y,offset.z);
					umath::negate(offset.z);

					points.push_back({posCur,offset}); // Original position (without offset is required for UV calculation below)
				}
			}
			
			// Build the triangles
			// (Compare CCoreDispInfo::GenerateDispSurf in source-sdk source code)
			auto numTriangles = umath::pow2(numRows -1) *2u;
			displacementRanges.push_back({bspTriangles.size(),numTriangles});
			bspTriangles.reserve(bspTriangles.size() +numTriangles);

			const std::array<Vector2,4> lightMapUvBaseCoords = {
				fLightmapUvToAtlasUv(Vector2{0.f,0.f}),
				fLightmapUvToAtlasUv(Vector2{0.f,1.f}),
				fLightmapUvToAtlasUv(Vector2{1.f,1.f}),
				fLightmapUvToAtlasUv(Vector2{1.f,0.f})
			};

			for(auto x=decltype(numRows){0u};x<(numRows -1);++x)
			{
				for(auto y=decltype(numRows){0u};y<(numRows -1);++y)
				{
					auto vertIdx0 = x *numRows +y;
					auto vertIdx1 = vertIdx0 +1u;
					auto vertIdx2 = vertIdx0 +numRows;
					auto vertIdx3 = vertIdx2 +1u;

					auto &p0 = points.at(vertIdx0);
					auto &p1 = points.at(vertIdx1);
					auto &p2 = points.at(vertIdx2);
					auto &p3 = points.at(vertIdx3);

					Vector3 normal {0.f,1.f,0.f}; // TODO

					auto triIdx = y *(numRows -1) +x;
					bspTriangles.push_back({});
					auto &triInfo0 = bspTriangles.back();
					triInfo0.faceIndex = faceIndex;
					triInfo0.materialIndex = texId;
					triInfo0.vertices.at(0) = Vertex{p0.first +p0.second,fCalculateUv(p0.first),normal};
					triInfo0.vertices.at(1) = Vertex{p1.first +p1.second,fCalculateUv(p1.first),normal};
					triInfo0.vertices.at(2) = Vertex{p2.first +p2.second,fCalculateUv(p2.first),normal};

					auto *outLightMapUvs0 = (outMeshLightMapUvCoordinates != nullptr) ? &triInfo0.lightMapUvs : nullptr;
					if(outLightMapUvs0 != nullptr)
					{
						// Note: Usually fCalcLightmapUv should be used to calculate the lightmap uvs, however
						// for some reason that results in incorrect uv coordinates if the displacement brush has
						// been skewed. calc_disp_surf_coords is from the source sdk code and does not use
						// the lightmap vecs from the BSP, but seems to work for all cases. It simply
						// calculates interpolated lightmap uvs over the displacement from the base uv coordinates
						// in lightMapUvBaseCoords.
						outLightMapUvs0->at(0) = calc_disp_surf_coords(lightMapUvBaseCoords,dispInfo.power,x,y);
						outLightMapUvs0->at(1) = calc_disp_surf_coords(lightMapUvBaseCoords,dispInfo.power,x,y +1);
						outLightMapUvs0->at(2) = calc_disp_surf_coords(lightMapUvBaseCoords,dispInfo.power,x +1,y);
					}

					bspTriangles.push_back({});
					auto &triInfo1 = bspTriangles.back();
					triInfo1.faceIndex = faceIndex;
					triInfo1.materialIndex = texId;
					triInfo1.vertices.at(0) = Vertex{p1.first +p1.second,fCalculateUv(p1.first),normal};
					triInfo1.vertices.at(1) = Vertex{p3.first +p3.second,fCalculateUv(p3.first),normal};
					triInfo1.vertices.at(2) = Vertex{p2.first +p2.second,fCalculateUv(p2.first),normal};
					auto *outLightMapUvs1 = (outMeshLightMapUvCoordinates != nullptr) ? &triInfo1.lightMapUvs : nullptr;
					if(outLightMapUvs1 != nullptr)
					{
						outLightMapUvs1->at(0) = calc_disp_surf_coords(lightMapUvBaseCoords,dispInfo.power,x,y +1);
						outLightMapUvs1->at(1) = calc_disp_surf_coords(lightMapUvBaseCoords,dispInfo.power,x +1,y +1);
						outLightMapUvs1->at(2) = calc_disp_surf_coords(lightMapUvBaseCoords,dispInfo.power,x +1,y);
					}
				}
			}
			continue;
		}

		// See https://github.com/ajkhoury/OpenBSP-MinGW/blob/master/Bsp.cpp
		std::pair<int32_t,Vector2> vertPoint;
		std::pair<int32_t,Vector2> rootPoint; // if it is the first run through the first vertex is the "hub" index that all of the triangles in the plane will refer to
		std::pair<int32_t,Vector2> firstPoint; // the first point after the hub
		std::pair<int32_t,Vector2> secondPoint; // last point to create a full triangle

		for(auto j=decltype(face.numEdges){0u};j<face.numEdges;++j)
		{
			auto iEdge = surfEdges.at(face.firstEdge +j);
			Vector3 vertex;
			if(iEdge >= 0)
				vertex = verts.at(edges.at(iEdge).v.at(0));
			else
				vertex = verts.at(edges.at(-iEdge).v.at(1));

			auto i = face.firstEdge +j;
			auto edgeIndex = surfEdges.at(i);
			auto &edge = edges.at(umath::abs(edgeIndex));
			auto reverse = (edgeIndex >= 0);
			//auto vNormal = face.planeNormal;
			//umath::swap(vNormal.y,vNormal.z);
			//umath::negate(vNormal.z);
			if(i == face.firstEdge)
			{
				rootPoint = {edge.v.at(reverse ? 0 : 1),Vector2{}};
				vertPoint = {edge.v.at(reverse ? 1 : 0),Vector2{}};
			}
			else 
			{
				vertPoint = {edge.v.at(reverse ? 0 : 1),Vector2{}};
				if(vertPoint == rootPoint)
					continue;
				firstPoint = vertPoint;
				
				vertPoint = {edge.v.at(reverse ? 1 : 0),Vector2{}};
				if(vertPoint == rootPoint)
					continue;
				secondPoint = vertPoint;
				
				if(bspTriangles.size() == bspTriangles.capacity())
					bspTriangles.reserve(bspTriangles.size() +1'000); // TODO
				bspTriangles.push_back({});
				auto &triInfo = bspTriangles.back();
				triInfo.faceIndex = faceIndex;
				triInfo.materialIndex = texId;
				auto &outVerts = triInfo.vertices;
				auto *outLightMapUvs = (outMeshLightMapUvCoordinates != nullptr) ? &triInfo.lightMapUvs : nullptr;
				auto localVertIdx = 0u;

				std::array<Vector3,3> faceVerts = {verts.at(rootPoint.first),verts.at(secondPoint.first),verts.at(firstPoint.first)};
				for(auto &v : faceVerts)
				{
					umath::swap(v.y,v.z);
					umath::negate(v.z);
				}
				auto faceNormal = Geometry::CalcFaceNormal(faceVerts.at(0),faceVerts.at(1),faceVerts.at(2));
				for(auto &pos : faceVerts)
				{
					auto vertUv = fCalculateUv(pos);
					outVerts.at(localVertIdx) = Vertex{pos,vertUv,faceNormal};

					if(outLightMapUvs != nullptr)
						outLightMapUvs->at(localVertIdx) = fCalcLightmapUv(pos);
					++localVertIdx;
				}
			}
		}
	}

	// Split meshes by leaves
	auto &bspTree = bspInputData.bspTree;
	auto &bspNodes = bspTree.GetNodes();

	struct MaterialMesh
	{
		std::vector<BSPTriangleInfo*> triangles;
	};
	struct ClusterMesh
	{
		std::unordered_map<MaterialIndex,MaterialMesh> meshes;
	};
	using BSPClusterIndex = int32_t;

	auto &leafFaces = bspInputData.leafFaces;
	// Meshes split by clusters (world only)
	std::unordered_map<BSPClusterIndex,ClusterMesh> clusterMeshes {};

	auto numMeshes = 0u;
	if(ent.IsWorld())
	{
		for(auto nodeIdx=decltype(bspNodes.size()){0u};nodeIdx<bspNodes.size();++nodeIdx)
		{
			auto &bspNode = bspNodes.at(nodeIdx);
			if(bspNode->leaf == false)
				continue;
			auto itClusterMesh = clusterMeshes.find(bspNode->cluster);
			if(itClusterMesh == clusterMeshes.end())
				itClusterMesh = clusterMeshes.insert(std::make_pair(bspNode->cluster,ClusterMesh{})).first;
			auto &clusterMesh = itClusterMesh->second;
			for(auto leafFaceIndex=bspNode->firstFace;leafFaceIndex<(bspNode->firstFace +bspNode->numFaces);++leafFaceIndex)
			{
				auto faceIdx = leafFaces.at(leafFaceIndex);
				for(auto &triInfo : bspTriangles)
				{
					if(triInfo.faceIndex != faceIdx)
						continue;
					auto itMatMesh = clusterMesh.meshes.find(triInfo.materialIndex);
					if(itMatMesh == clusterMesh.meshes.end())
					{
						itMatMesh = clusterMesh.meshes.insert(std::make_pair(triInfo.materialIndex,MaterialMesh{})).first;
						++numMeshes;
					}
					auto &matMesh = itMatMesh->second;
					matMesh.triangles.push_back(&triInfo);
				}
			}
		}
	}
	else
	{
		// Not a world entity; Just add all faces belonging to this entity directly
		auto itClusterMesh = clusterMeshes.find(-1);
		if(itClusterMesh == clusterMeshes.end())
			itClusterMesh = clusterMeshes.insert(std::make_pair(-1,ClusterMesh{})).first;
		auto &clusterMesh = itClusterMesh->second;
		for(auto &triInfo : bspTriangles)
		{
			auto itMatMesh = clusterMesh.meshes.find(triInfo.materialIndex);
			if(itMatMesh == clusterMesh.meshes.end())
			{
				itMatMesh = clusterMesh.meshes.insert(std::make_pair(triInfo.materialIndex,MaterialMesh{})).first;
				++numMeshes;
			}
			auto &matMesh = itMatMesh->second;
			matMesh.triangles.push_back(&triInfo);
		}
	}

	// At this point 'clusterMesh' contains the triangles split by clusters and materials.
	// Build model
	auto mdl = game.CreateModel();
	auto meshGroup = mdl->GetMeshGroup(0u);
	meshGroup->GetMeshes().reserve(numMeshes);
	if(outMeshLightMapUvCoordinates != nullptr)
		outMeshLightMapUvCoordinates->reserve(outMeshLightMapUvCoordinates->size() +numMeshes);
	auto meshIndex = 0u;
	for(auto &pairCluster : clusterMeshes)
	{
		auto &clusterMesh = pairCluster.second;
		for(auto &pairMat : clusterMesh.meshes)
		{
			auto &matMesh = pairMat.second;
			auto matIdx = mdl->AddMaterial(0u,materials.at(pairMat.first));
			auto mesh = game.CreateModelMesh();
			auto subMesh = game.CreateModelSubMesh();
			std::vector<Vector2> *meshLightMapUvs = nullptr;
			if(outMeshLightMapUvCoordinates != nullptr)
			{
				outMeshLightMapUvCoordinates->push_back({});
				meshLightMapUvs = &outMeshLightMapUvCoordinates->back();
				meshLightMapUvs->reserve(matMesh.triangles.size() *3u);
			}
			auto &meshVerts = subMesh->GetVertices();
			auto &meshTris = subMesh->GetTriangles();
			meshVerts.reserve(matMesh.triangles.size() *3u);
			meshTris.reserve(matMesh.triangles.size() *3u);
			for(auto *tri : matMesh.triangles)
			{
				auto numVerts = meshVerts.size();
				meshTris.push_back(numVerts);
				meshTris.push_back(numVerts +1u);
				meshTris.push_back(numVerts +2u);

				meshVerts.push_back(tri->vertices.at(0));
				meshVerts.push_back(tri->vertices.at(1));
				meshVerts.push_back(tri->vertices.at(2));

				if(meshLightMapUvs != nullptr)
				{
					meshLightMapUvs->push_back(tri->lightMapUvs.at(0));
					meshLightMapUvs->push_back(tri->lightMapUvs.at(1));
					meshLightMapUvs->push_back(tri->lightMapUvs.at(2));
				}
			}
			subMesh->SetTexture(matIdx);
			subMesh->SetReferenceId(meshIndex++); // Mesh index; Needed to associate mesh with light map uv buffer
			mesh->AddSubMesh(subMesh);
			mesh->SetReferenceId(pairCluster.first); // Cluster index; Needed to determine visibility in BSP occlusion culling (only used for world entity)

			meshGroup->AddMesh(mesh);
		}
	}

	// For some reason displacement faces aren't located in any leaves, so we'll just add them separately
	for(auto &range : displacementRanges)
	{
		auto &triInfo = bspTriangles.at(range.first);
		auto matIdx = mdl->AddMaterial(0u,materials.at(triInfo.materialIndex));
		auto mesh = game.CreateModelMesh();
		auto subMesh = game.CreateModelSubMesh();
		std::vector<Vector2> *meshLightMapUvs = nullptr;
		if(outMeshLightMapUvCoordinates != nullptr)
		{
			outMeshLightMapUvCoordinates->push_back({});
			meshLightMapUvs = &outMeshLightMapUvCoordinates->back();
			meshLightMapUvs->reserve(range.second *3u);
		}
		auto &meshVerts = subMesh->GetVertices();
		auto &meshTris = subMesh->GetTriangles();
		meshVerts.reserve(range.second *3u);
		meshTris.reserve(range.second *3u);

		for(auto i=range.first;i<(range.first +range.second);++i)
		{
			auto &triInfo = bspTriangles.at(i);
			auto numVerts = meshVerts.size();
			meshTris.push_back(numVerts);
			meshTris.push_back(numVerts +1u);
			meshTris.push_back(numVerts +2u);

			meshVerts.push_back(triInfo.vertices.at(0));
			meshVerts.push_back(triInfo.vertices.at(1));
			meshVerts.push_back(triInfo.vertices.at(2));

			if(meshLightMapUvs != nullptr)
			{
				meshLightMapUvs->push_back(triInfo.lightMapUvs.at(0));
				meshLightMapUvs->push_back(triInfo.lightMapUvs.at(1));
				meshLightMapUvs->push_back(triInfo.lightMapUvs.at(2));
			}
		}
		subMesh->SetTexture(matIdx);
		subMesh->SetReferenceId(meshIndex++); // Mesh index; Needed to associate mesh with light map uv buffer
		mesh->AddSubMesh(subMesh);
		mesh->SetReferenceId(std::numeric_limits<uint32_t>::max());

		meshGroup->AddMesh(mesh);
	}
	//

	auto pMdlComponent = ent.GetModelComponent();
	mdl->GenerateBindPoseMatrices();
	if(pMdlComponent.valid())
	{
		auto oldMdl = pMdlComponent->GetModel();
		if(oldMdl != nullptr)
			mdl->GetCollisionMeshes() = oldMdl->GetCollisionMeshes(); // Hack: Use physics from poly-based geometry (which should already be loaded at this point)
	}
	mdl->Update(ModelUpdateFlags::All);
	
	if(pMdlComponent.valid())
		pMdlComponent->SetModel(mdl);
}

void pragma::level::load_map_brushes(
	Game &game,uint32_t version,VFilePtr f,BaseEntity *ent,std::vector<Material*> &materials,std::vector<SurfaceMaterial> &surfaceMaterials,
	const Vector3 &origin
)
{
	unsigned int numMeshes = f->Read<unsigned int>();
	if(numMeshes == 0)
		return;
	if(version < 7)
	{
		auto center = f->Read<Vector3>();
		if(ent != nullptr)
		{
			auto pTrComponent = ent->GetTransformComponent();
			if(pTrComponent.valid())
				pTrComponent->SetPosition(center);
		}
	}

	std::vector<std::shared_ptr<BrushMesh>> meshes(numMeshes);
	for(unsigned int i=0;i<numMeshes;i++)
	{
		bool bDisplacement = f->Read<bool>();
		auto mesh = game.CreateBrushMesh();
		if(bDisplacement)
			mesh->SetConvex(false);

		std::vector<Vector3> verts;
		unsigned int numVerts = f->Read<unsigned int>();
		verts.resize(numVerts);
		f->Read(&verts[0][0],sizeof(float) *3 *numVerts);
		for(auto &v : verts)
			v += origin;

		unsigned int numSides = f->Read<unsigned int>();
		for(unsigned int j=0;j<numSides;j++)
		{
			std::vector<uint16_t> lodTriangles;
			auto side = game.CreateSide();
			unsigned int matID = f->Read<unsigned int>();
			Material *mat = (matID < materials.size()) ? materials.at(matID) : nullptr;
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
			Vector3 nu,nv;
			f->Read(&nu[0],sizeof(float) *3);
			f->Read(&nv[0],sizeof(float) *3);
			float sw = 1.f /w;
			float sh = 1.f /h;
			float ou = f->Read<float>();
			float ov = f->Read<float>();
			float su = f->Read<float>();
			float sv = f->Read<float>();
			//float rot = f->Read<float>();
			unsigned char numAlpha = 0;

			unsigned int numSideVerts = f->Read<unsigned int>();
			std::vector<Vector3> sideVerts(numSideVerts);
			for(unsigned int k=0;k<numSideVerts;k++)
			{
				unsigned int vertID = f->Read<unsigned int>();
				sideVerts[k] = verts[vertID];
			}

			if(bDisplacement == true)
			{
				auto numMerged = f->Read<unsigned short>();
				std::vector<unsigned char> dispRows;
				dispRows.reserve(numMerged);
				UInt32 numVerts = 0;
				UInt32 numIndices = 0;
				for(UInt16 i=0;i<numMerged;i++)
				{
					auto power = f->Read<unsigned char>();
					auto rows = umath::pow(2,CInt32(power)) +1;
					dispRows.push_back(CUChar(rows));
					numVerts += umath::pow(rows,2);
					numIndices += umath::pow(rows -1,2) *6;
				}

				auto &vertices = side->GetVertices();
				vertices.resize(numVerts);
				f->Read(vertices.data(),sizeof(Vector3) *numVerts);
				
				auto &normals = side->GetNormals();
				normals.resize(numVerts);
				f->Read(normals.data(),sizeof(Vector3) *numVerts);

				auto &uvs = side->GetUVMapping();
				uvs.resize(numVerts);
				f->Read(uvs.data(),sizeof(Vector2) *numVerts);
				if(version <= 0x0002) // Version 2 and lower used OpenGL texture coordinates (lower left origin), they'll have to be flipped first
				{
					for(auto &uv : uvs)
						uv.y = 1.f -uv.y;
				}

				numAlpha = f->Read<unsigned char>();
				if(numAlpha > 0)
				{
					auto &alphas = side->GetAlphas();
					alphas.reserve(numVerts);
					for(UInt32 i=0;i<numVerts;i++)
					{
						auto a1 = f->Read<unsigned char>();
						auto a2 = (numAlpha > 1) ? f->Read<unsigned char>() : 0;
						alphas.push_back(Vector2(
							CFloat(a1) /255.f,
							CFloat(a2) /255.f
						));
					}
				}

				// Triangles
				lodTriangles.resize(numIndices);
				UInt32 idx = 0;
				//UInt32 i = 0;
				UInt32 offset = 0;
				for(auto it=dispRows.begin();it!=dispRows.end();++it)
				{
					auto rows = *it;
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
				/*if(dispRows.size() == 1) // Only works with non-merged displacements (Which have been deactivated as of 16-09-29)
				{
					auto numTriangles = triangles->size();
					uint8_t power = 2;
					while(numTriangles > 16 && (numTriangles %power) == 0 && power < 128)
					{
						lodTriangles.push_back(new std::vector<uint16_t>());
						if(decimate_displacement_geometry(*triangles,dispRows.front(),*lodTriangles.back(),power) == false)
						{
							delete lodTriangles.back();
							lodTriangles.erase(lodTriangles.end() -1);
							break;
						}
						numTriangles = lodTriangles.back()->size();
						power <<= 1;
					}
				}*/
				//

				// Obsolete
				/*unsigned int startPosition = f->Read<unsigned int>();

				unsigned char power = f->Read<unsigned char>();
				numAlpha = f->Read<unsigned char>();

				int rows = umath::pow(2,power) +1;
				std::vector<std::vector<Vector3>> normalList(rows);
				std::vector<std::vector<float>> distList(rows);
				std::vector<std::vector<Vector3>> offsetList(rows);

				vertices = new std::vector<Vector3>;
				triangles = new std::vector<unsigned int>;
				normals = new std::vector<Vector3>;
				uvs = new std::vector<Vector2>;

				for(int i=0;i<rows;i++)
				{
					std::vector<Vector3> &normals = normalList[i];
					std::vector<float> &dists = distList[i];
					std::vector<Vector3> &offsets = offsetList[i];
					normals.resize(rows);
					dists.resize(rows);
					offsets.resize(rows);
					f->Read(&normals[0],sizeof(float) *3 *rows);
					f->Read(&offsets[0],sizeof(float) *3 *rows);
					f->Read(&dists[0],sizeof(float) *rows);
				}
				std::vector<std::vector<Vector2>> alphaList((numAlpha > 0) ? rows : 0);
				if(numAlpha > 0)
				{
					alphas = new std::vector<Vector2>;
					for(int i=(rows -1);i>=0;i--)
					{
						std::vector<Vector2> &vAlphas = alphaList[i];
						vAlphas.resize(rows);
						for(int j=0;j<rows;j++)
							f->Read(&vAlphas[j][0],sizeof(float) *numAlpha);
					}
				}
				BuildDisplacementTriangles(
					sideVerts,startPosition,nu,nv,sw,sh,ou,ov,su,sv,power,normalList,offsetList,distList,numAlpha,alphaList,
					*vertices,*normals,*uvs,*triangles,alphas
				);*/
			}
			else
			{
				auto &vertices = side->GetVertices();
				auto &uvs = side->GetUVMapping();
				auto &normals = side->GetNormals();
				vertices.resize(numSideVerts);
				uvs.resize(numSideVerts);
				normals.resize(numSideVerts);
				for(unsigned int k=0;k<numSideVerts;k++)
				{
					Vector3 &v = sideVerts[k];
					vertices[k] = v;
					Vector2 uv;
					uv.x = (glm::dot(v,nu) *sw) /su +ou *sw;
					uv.y = 1.f -((glm::dot(v,nv) *sh) /sv +ov *sh);
					/*if(uv.x < 0)
					{
						uv.x = -uv.x;
						uv.x -= CInt32(uv.x);
						uv.x = 1 -uv.x;
					}
					else
						uv.x -= CInt32(uv.x);
					
					if(uv.y < 0)
					{
						uv.y = -uv.y;
						uv.y -= CInt32(uv.y);
						uv.y = 1 -uv.y;
					}
					else
						uv.y -= CInt32(uv.y);*/
					uvs[k] = uv;
				}
				f->Read(normals.data(),sizeof(float) *3 *numSideVerts);
				ToTriangles(vertices,lodTriangles);
			}
			side->SetMaterial(mat);
			side->SetDisplacementAlpha(numAlpha);
			*side->GetTriangles() = lodTriangles;
			side->SetConvex(!bDisplacement);
			mesh->AddSide(side);
		}
		if(bDisplacement == false)
			mesh->Optimize();
		mesh->Calculate(&surfaceMaterials);
		meshes[i] = mesh;
	}
	if(ent == NULL)
		return;
	Vector3 min(0,0,0);
	Vector3 max(0,0,0);
	auto pPhysComponent = ent->GetPhysicsComponent();
	for(int i=0;i<meshes.size();i++)
	{
		if(pPhysComponent.valid())
			pPhysComponent->AddBrushMesh(meshes[i]);
		Vector3 minMesh,maxMesh;
		meshes[i]->GetBounds(&minMesh,&maxMesh);
		if(i == 0)
		{
			min = minMesh;
			max = maxMesh;
		}
		else
		{
			uvec::min(&min,minMesh);
			uvec::max(&max,maxMesh);
		}
	}
	if(pPhysComponent.valid())
	{
		pPhysComponent->SetCollisionBounds(min,max);
		pPhysComponent->InitializeBrushGeometry();
	}
}

void pragma::level::load_optimized_map_geometry(
	Game &game,uint32_t version,VFilePtr f,BaseEntity *ent,std::vector<Material*> &materials,
	std::vector<SurfaceMaterial> &surfaceMaterials
)
{
	auto numMeshes = f->Read<uint32_t>();
	if(numMeshes == 0)
		return;
	auto center = f->Read<Vector3>();
	if(ent != nullptr)
	{
		auto pTrComponent = ent->GetTransformComponent();
		if(pTrComponent.valid())
			pTrComponent->SetPosition(center);
	}
	auto bOptimizedMeshes = false;
	if(version >= 0x0004)
		bOptimizedMeshes = f->Read<bool>();
	assert(bOptimizedMeshes);
	if(bOptimizedMeshes == true)
	{
		auto mdl = game.CreateModel(false);
		auto group = mdl->GetMeshGroup("reference");
		auto *texGroup = mdl->GetTextureGroup(0);
		for(auto i=decltype(numMeshes){0};i<numMeshes;++i)
		{
			auto matId = f->Read<uint32_t>();
			uint32_t width = 0;
			uint32_t height = 0;
			if(matId < materials.size())
			{
				auto *mat = materials.at(matId);
				auto texId = mdl->AddTexture(mat->GetName(),mat);
				texGroup->textures.push_back(texId);

				auto *diffuse = mat->GetDiffuseMap();
				if(diffuse != nullptr)
				{
					width = diffuse->width;
					height = diffuse->height;
				}
			}
			width = (width != 0) ? width : 512;
			height = (height != 0) ? height : 512;
			auto sw = 1.f /static_cast<float>(width);
			auto sh = 1.f /static_cast<float>(height);

			auto mesh = game.CreateModelMesh();
			auto subMeshCount = f->Read<uint32_t>();
			for(auto i=decltype(subMeshCount){0};i<subMeshCount;++i)
			{
				auto nu = f->Read<Vector3>();
				auto nv = f->Read<Vector3>();
				auto ou = f->Read<float>();
				auto ov = f->Read<float>();
				auto su = f->Read<float>();
				auto sv = f->Read<float>();

				auto subMesh = game.CreateModelSubMesh();
				auto vertCount = f->Read<uint32_t>();
				for(auto i=decltype(vertCount){0};i<vertCount;++i)
				{
					auto v = f->Read<Vector3>();
					Vector2 uv {
						(glm::dot(v,nu) *sw) /su +ou *sw,
						1.f -((glm::dot(v,nv) *sh) /sv +ov *sh)
					};
					Vector3 normal {}; // TODO
					subMesh->AddVertex(Vertex{v,uv,normal});
				}
				mesh->AddSubMesh(subMesh);
			}
			group->AddMesh(mesh);
		}
		// TODO: Collision meshes
		// TODO: Surface materials
		if(ent == nullptr)
		{
			mdl->Remove();
			return;
		}
		mdl->Update(ModelUpdateFlags::All);
		Vector3 min,max;
		mdl->GetCollisionBounds(min,max);
		auto pPhysComponent = ent->GetPhysicsComponent();
		if(pPhysComponent.valid())
			pPhysComponent->SetCollisionBounds(min,max);
		auto *pMdlComponent = static_cast<BaseModelComponent*>(ent->AddComponent("model").get());
		if(pMdlComponent != nullptr)
			pMdlComponent->SetModel(mdl);
	}
}
