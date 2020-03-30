#include <iostream>
#include "source_engine/vbsp/bsp_converter.hpp"
#include <pragma/game/game.h>
#include <pragma/asset_types/world.hpp>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/math/intersection.h>
#include <util_vmf.hpp>
#include <memory>

std::shared_ptr<Model> pragma::asset::vbsp::BSPConverter::GenerateModel(EntityData &entData,LightmapData &lightMapInfo,const std::vector<MaterialHandle> &materials)
{
	auto &worldData = *m_outputWorldData;
	auto &game = m_game;
	auto &bsp = *m_bsp;

	auto borderSize = lightMapInfo.borderSize;
	auto lightmapAtlasExtents = lightMapInfo.atlasSize;
	using MaterialIndex = int16_t;
	struct BSPTriangleInfo
	{
		MaterialIndex materialIndex = 0u;
		uint32_t faceIndex = 0u;
		std::array<Vertex,3u> vertices;
		std::array<Vector2,3u> lightMapUvs;
		std::optional<std::array<float,3>> alphas = {};
	};
	std::vector<BSPTriangleInfo> bspTriangles {};
	auto &faces = lightMapInfo.faceInfos; // TODO
	auto &texInfo = bsp.GetTexInfo();
	auto &texData = m_bsp->GetTexData();
	auto &texStringTable = m_bsp->GetTexDataStringIndices();
	auto &texStringData = m_bsp->GetTranslatedTexDataStrings();
	auto &surfEdges = bsp.GetSurfEdges();
	auto &verts = bsp.GetVertices();
	auto &edges = bsp.GetEdges();
	auto *bspData = FindBSPEntityData(entData);
	if(bspData == nullptr)
		return nullptr;
	std::vector<std::pair<uint64_t,uint64_t>> displacementRanges {};
	auto useLightmaps = true;//entData.IsWorld(); // TODO: Add support for static props?
	if(bspData)
	{
		for(auto faceIndex : bspData->faceIndices)
		{
			auto &face = faces.at(faceIndex);
			if(face.numEdges == 0u)
				continue;
			auto &faceTexInfo = texInfo.at(face.texInfoIndex); // TODO: face.texInfoIndex == -1?
			auto texId = (faceTexInfo.texdata != -1) ? texStringTable.at(texData.at(faceTexInfo.texdata).nameStringTableID) : -1;
			auto hMat = materials.at(texId);
			auto *diffuseMap = hMat.IsValid() ? hMat.get()->GetDiffuseMap() : nullptr;
			if(diffuseMap == nullptr)
				; // TODO: Print warning
			auto texWidth = (diffuseMap != nullptr) ? diffuseMap->width : 0u;
			auto texHeight = (diffuseMap != nullptr) ? diffuseMap->height : 0u;
			if(texWidth == 0u)
				texWidth = 512u;
			if(texHeight == 0u)
				texHeight = 512u;

			auto v0 = BSPTextureVecToPragma(Vector3(faceTexInfo.textureVecs.at(0).at(0),faceTexInfo.textureVecs.at(0).at(1),faceTexInfo.textureVecs.at(0).at(2)));
			auto v1 = BSPTextureVecToPragma(Vector3(faceTexInfo.textureVecs.at(1).at(0),faceTexInfo.textureVecs.at(1).at(1),faceTexInfo.textureVecs.at(1).at(2)));
			std::array<Vector4,2> textureVecs = {
				Vector4{v0.x,v0.y,v0.z,faceTexInfo.textureVecs.at(0).at(3)},
				Vector4{v1.x,v1.y,v1.z,faceTexInfo.textureVecs.at(1).at(3)}
			};

			Vector3 texVec0 = Vector3{textureVecs.at(0).x,textureVecs.at(0).y,textureVecs.at(0).z};
			texVec0 = {-texVec0.y,texVec0.z,-texVec0.x};

			Vector3 texVec1 = Vector3{textureVecs.at(1).x,textureVecs.at(1).y,textureVecs.at(1).z};
			texVec1 = {-texVec1.y,texVec1.z,-texVec1.x};

			const auto fCalculateUv = [&texVec0,&texVec1,&faceTexInfo,&textureVecs,texWidth,texHeight](const Vector3 &pos) -> Vector2 {
				Vector2 vertUv{};
				vertUv.x = uvec::dot(texVec0,pos) +textureVecs.at(0).w;
				vertUv.y = uvec::dot(texVec1,pos) +textureVecs.at(1).w;
				vertUv.x /= static_cast<float>(texWidth);
				vertUv.y /= static_cast<float>(texHeight);
				return vertUv;
			};

			auto lv0 = BSPTextureVecToPragma(Vector3(faceTexInfo.lightmapVecs.at(0).at(0),faceTexInfo.lightmapVecs.at(0).at(1),faceTexInfo.lightmapVecs.at(0).at(2)));
			auto lv1 = BSPTextureVecToPragma(Vector3(faceTexInfo.lightmapVecs.at(1).at(0),faceTexInfo.lightmapVecs.at(1).at(1),faceTexInfo.lightmapVecs.at(1).at(2)));
			std::array<Vector4,2> lightMapVecs = {
				Vector4{lv0.x,lv0.y,lv0.z,faceTexInfo.lightmapVecs.at(0).at(3)},
				Vector4{lv1.x,lv1.y,lv1.z,faceTexInfo.lightmapVecs.at(1).at(3)}
			};

			auto widthLightmap = face.lightMapSize.at(0);
			auto heightLightmap = face.lightMapSize.at(1);
			auto &lmv0 = lightMapVecs.at(0);
			Vector3 lightVec0 = Vector3{lmv0.x,lmv0.y,lmv0.z};
			lightVec0 = {-lightVec0.y,lightVec0.z,-lightVec0.x};

			auto &lmv1 = lightMapVecs.at(1);
			Vector3 lightVec1 = Vector3{lmv1.x,lmv1.y,lmv1.z};
			lightVec1 = {-lightVec1.y,lightVec1.z,-lightVec1.x};

			auto &lmInfo = lightMapInfo.faceInfos.at(faceIndex);

			const auto fLightmapUvToAtlasUv = [&lmInfo,widthLightmap,heightLightmap,borderSize,lightmapAtlasExtents](const Vector2 &lightMapUv) {
				auto bRotated = (lmInfo.flags &FaceLightMapInfo::Flags::Rotated) != FaceLightMapInfo::Flags::None;

				auto result = lightMapUv;
				if(bRotated)
					result = {1.f -result.y,result.x};
				result.x = result.x *widthLightmap +lmInfo.x +borderSize;
				result.y = result.y *heightLightmap +lmInfo.y +borderSize;

				result.x /= static_cast<float>(lightmapAtlasExtents.x);
				result.y /= static_cast<float>(lightmapAtlasExtents.y);
				return result;
			};

			const auto fCalcLightmapUv = [&lightVec0,&lightVec1,&face,&lmv0,&lmv1,&fLightmapUvToAtlasUv,&lmInfo,widthLightmap,heightLightmap](const Vector3 &pos,const std::function<void(Vector2&)> &uvModifier=nullptr) {
				auto lu = uvec::dot(lightVec0,pos) +lmv0.w -(face.lightMapMins.at(0));
				auto lv = uvec::dot(lightVec1,pos) +lmv1.w -(face.lightMapMins.at(1));

				auto bRotated = (lmInfo.flags &FaceLightMapInfo::Flags::Rotated) != FaceLightMapInfo::Flags::None;
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
				auto &dispInfo = bsp.GetDisplacements().at(face.dispInfoIndex);

				auto dispStartPos = dispInfo.dispInfo.startPosition;
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

				auto numRows = umath::pow(2,dispInfo.dispInfo.power) +1;
				auto numVerts = umath::pow2(numRows);
				struct Point
				{
					Vector3 position = {};
					Vector3 offset = {};
					float alpha = 0.f;
				};
				std::vector<Point> points {};
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
						auto &dispVert = dispInfo.verts.at(vertIdx);
						auto offset = dispVert.vec *dispVert.dist;
						umath::swap(offset.y,offset.z);
						umath::negate(offset.z);
						points.push_back({posCur,offset,dispVert.alpha}); // Original position (without offset is required for UV calculation below)
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
						triInfo0.vertices.at(0) = Vertex{p0.position +p0.offset,fCalculateUv(p0.position),normal};
						triInfo0.vertices.at(1) = Vertex{p1.position +p1.offset,fCalculateUv(p1.position),normal};
						triInfo0.vertices.at(2) = Vertex{p2.position +p2.offset,fCalculateUv(p2.position),normal};
						triInfo0.alphas = {
							umath::clamp(p0.alpha /255.f,0.f,1.f),
							umath::clamp(p1.alpha /255.f,0.f,1.f),
							umath::clamp(p2.alpha /255.f,0.f,1.f)
						};

						auto &outLightMapUvs0 = triInfo0.lightMapUvs;
						if(useLightmaps)
						{
							// Note: Usually fCalcLightmapUv should be used to calculate the lightmap uvs, however
							// for some reason that results in incorrect uv coordinates if the displacement brush has
							// been skewed. calc_disp_surf_coords is from the source sdk code and does not use
							// the lightmap vecs from the BSP, but seems to work for all cases. It simply
							// calculates interpolated lightmap uvs over the displacement from the base uv coordinates
							// in lightMapUvBaseCoords.
							outLightMapUvs0.at(0) = CalcDispSurfCoords(lightMapUvBaseCoords,dispInfo.dispInfo.power,x,y);
							outLightMapUvs0.at(1) = CalcDispSurfCoords(lightMapUvBaseCoords,dispInfo.dispInfo.power,x,y +1);
							outLightMapUvs0.at(2) = CalcDispSurfCoords(lightMapUvBaseCoords,dispInfo.dispInfo.power,x +1,y);
						}

						bspTriangles.push_back({});
						auto &triInfo1 = bspTriangles.back();
						triInfo1.faceIndex = faceIndex;
						triInfo1.materialIndex = texId;
						triInfo1.vertices.at(0) = Vertex{p1.position +p1.offset,fCalculateUv(p1.position),normal};
						triInfo1.vertices.at(1) = Vertex{p3.position +p3.offset,fCalculateUv(p3.position),normal};
						triInfo1.vertices.at(2) = Vertex{p2.position +p2.offset,fCalculateUv(p2.position),normal};
						triInfo1.alphas = {
							umath::clamp(p1.alpha /255.f,0.f,1.f),
							umath::clamp(p3.alpha /255.f,0.f,1.f),
							umath::clamp(p2.alpha /255.f,0.f,1.f)
						};
						auto &outLightMapUvs1 = triInfo1.lightMapUvs;
						if(useLightmaps)
						{
							outLightMapUvs1.at(0) = CalcDispSurfCoords(lightMapUvBaseCoords,dispInfo.dispInfo.power,x,y +1);
							outLightMapUvs1.at(1) = CalcDispSurfCoords(lightMapUvBaseCoords,dispInfo.dispInfo.power,x +1,y +1);
							outLightMapUvs1.at(2) = CalcDispSurfCoords(lightMapUvBaseCoords,dispInfo.dispInfo.power,x +1,y);
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
				auto i = face.firstEdge +j;
				auto edgeIndex = surfEdges.at(i);
				auto &edge = edges.at(umath::abs(edgeIndex));
				auto reverse = (edgeIndex >= 0);
				//auto vNormal = face.planeNormal;

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
					auto &outLightMapUvs = triInfo.lightMapUvs;
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

						outLightMapUvs.at(localVertIdx) = fCalcLightmapUv(pos);
						++localVertIdx;
					}
				}
			}
		}
	}

	// Split meshes by leaves
	struct MaterialMesh
	{
		std::vector<BSPTriangleInfo*> triangles;
	};
	struct ClusterMesh
	{
		std::unordered_map<MaterialIndex,MaterialMesh> meshes;
	};
	using BSPClusterIndex = int32_t;

	auto &leafFaces = bsp.GetLeafFaces();
	// Meshes split by clusters (world only)
	std::unordered_map<BSPClusterIndex,ClusterMesh> clusterMeshes {};


	auto *bspTree = worldData.GetBSPTree();
	auto numMeshes = 0u;
	if(entData.IsWorld() && bspTree)
	{
		auto &bspNodes = bspTree->GetNodes();
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
	auto meshIndex = 0u;
	for(auto &pairCluster : clusterMeshes)
	{
		auto &clusterMesh = pairCluster.second;
		for(auto &pairMat : clusterMesh.meshes)
		{
			auto &matMesh = pairMat.second;
			std::optional<uint32_t> skinTexIdx {};
			auto matIdx = mdl->AddMaterial(0u,materials.at(pairMat.first).get(),&skinTexIdx);
			auto mesh = game.CreateModelMesh();
			auto subMesh = game.CreateModelSubMesh();

			auto &meshVerts = subMesh->GetVertices();
			auto &meshTris = subMesh->GetTriangles();
			auto &uvSets = subMesh->GetUVSets();
			std::vector<Vector2> *meshLightmapUvs = nullptr;
			if(useLightmaps)
			{
				meshLightmapUvs = &subMesh->AddUVSet("lightmap");
				meshLightmapUvs->reserve(matMesh.triangles.size() *3u);
			}
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

				if(meshLightmapUvs)
				{
					meshLightmapUvs->push_back(tri->lightMapUvs.at(0));
					meshLightmapUvs->push_back(tri->lightMapUvs.at(1));
					meshLightmapUvs->push_back(tri->lightMapUvs.at(2));
				}
			}
			subMesh->SetSkinTextureIndex(skinTexIdx.has_value() ? *skinTexIdx : 0);
			if(useLightmaps)
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
		std::optional<uint32_t> skinTexIdx {};
		auto matIdx = mdl->AddMaterial(0u,materials.at(triInfo.materialIndex).get(),&skinTexIdx);
		auto mesh = game.CreateModelMesh();
		auto subMesh = game.CreateModelSubMesh();
		auto &uvSets = subMesh->GetUVSets();
		std::vector<Vector2> *meshLightmapUvs = nullptr;
		if(useLightmaps)
		{
			meshLightmapUvs = &subMesh->AddUVSet("lightmap");
			meshLightmapUvs->reserve(range.second *3u);
		}
		auto &meshVerts = subMesh->GetVertices();
		auto &meshTris = subMesh->GetTriangles();
		auto &meshAlphas = subMesh->GetAlphas();
		meshVerts.reserve(range.second *3u);
		meshTris.reserve(range.second *3u);

		auto hasAlphas = false;
		for(auto i=range.first;i<(range.first +range.second);++i)
		{
			auto &triInfo = bspTriangles.at(i);

			if(i == range.first)
			{
				hasAlphas = triInfo.alphas.has_value();
				meshAlphas.reserve(range.second *3u);
				subMesh->SetAlphaCount(1u);
			}

			auto numVerts = meshVerts.size();
			meshTris.push_back(numVerts);
			meshTris.push_back(numVerts +1u);
			meshTris.push_back(numVerts +2u);

			auto cv = [](Vertex &v) -> Vertex& {
				//v.position.x = -v.position.x;
				return v;
			};
			meshVerts.push_back(cv(triInfo.vertices.at(0)));
			meshVerts.push_back(cv(triInfo.vertices.at(1)));
			meshVerts.push_back(cv(triInfo.vertices.at(2)));

			if(hasAlphas)
			{
				meshAlphas.push_back(Vector2{triInfo.alphas->at(0),0.f});
				meshAlphas.push_back(Vector2{triInfo.alphas->at(1),0.f});
				meshAlphas.push_back(Vector2{triInfo.alphas->at(2),0.f});
			}

			if(meshLightmapUvs)
			{
				meshLightmapUvs->push_back(triInfo.lightMapUvs.at(0));
				meshLightmapUvs->push_back(triInfo.lightMapUvs.at(1));
				meshLightmapUvs->push_back(triInfo.lightMapUvs.at(2));
			}
		}
		subMesh->SetSkinTextureIndex(skinTexIdx.has_value() ? *skinTexIdx : 0);
		subMesh->SetReferenceId(meshIndex++); // Mesh index; Needed to associate mesh with light map uv buffer
		mesh->AddSubMesh(subMesh);
		mesh->SetReferenceId(std::numeric_limits<uint32_t>::max());

		meshGroup->AddMesh(mesh);
	}
	//

	// Collisions
	if(bspData)
	{
		auto collisionMeshes = GeneratePhysics(entData,bspData->polyMeshes,materials);
		mdl->GetCollisionMeshes().reserve(collisionMeshes.size());
		for(auto &colMesh : collisionMeshes)
			mdl->AddCollisionMesh(colMesh);
	}
	return mdl;
}
