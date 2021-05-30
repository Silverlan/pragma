/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseworld.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/model/side.h"
#include "pragma/ai/navsystem.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include <fsys/filesystem.h>
#include <mathutil/umath.h>
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/model/model.h"
#include <sharedutils/scope_guard.h>

#define WNAV_VERSION 0x0002

RcNavMesh::RcNavMesh(
	const std::shared_ptr<rcPolyMesh> &polyMesh,
	const std::shared_ptr<rcPolyMeshDetail> &polyMeshDetail,
	const std::shared_ptr<dtNavMesh> &navMesh
) : m_polyMesh(polyMesh),
	m_polyMeshDetail(polyMeshDetail),
	m_navMesh(navMesh)
{}

rcPolyMesh &RcNavMesh::GetPolyMesh() {return *m_polyMesh;}
rcPolyMeshDetail &RcNavMesh::GetPolyMeshDetail() {return *m_polyMeshDetail;}
dtNavMesh &RcNavMesh::GetNavMesh() {return *m_navMesh;}

////////////////////////////////

pragma::nav::Config::Config(float walkableRadius,float characterHeight,float maxClimbHeight,float walkableSlopeAngle)
	: walkableRadius(walkableRadius),characterHeight(characterHeight),maxClimbHeight(maxClimbHeight),
	walkableSlopeAngle(walkableSlopeAngle),
	cellSize(walkableRadius /4.f),cellHeight(characterHeight /4.f),
	maxEdgeLength(walkableRadius *8.f)
{}

////////////////////////////////

static std::shared_ptr<dtNavMesh> initialize_detour_mesh(rcPolyMesh &polyMesh,rcPolyMeshDetail &polyMeshDetail,const pragma::nav::Config &config,std::string *err=nullptr)
{
	dtNavMeshCreateParams params;
	memset(&params,0,sizeof(params));
	params.verts = polyMesh.verts;
	params.vertCount = polyMesh.nverts;
	params.polys = polyMesh.polys;
	params.polyAreas = polyMesh.areas;
	params.polyFlags = polyMesh.flags;
	params.polyCount = polyMesh.npolys;
	params.nvp = polyMesh.nvp;
	params.detailMeshes = polyMeshDetail.meshes;
	params.detailVerts = polyMeshDetail.verts;
	params.detailVertsCount = polyMeshDetail.nverts;
	params.detailTris = polyMeshDetail.tris;
	params.detailTriCount = polyMeshDetail.ntris;

	params.offMeshConVerts = 0;
	params.offMeshConRad = 0;
	params.offMeshConDir = 0;
	params.offMeshConAreas = 0;
	params.offMeshConFlags = 0;
	params.offMeshConUserID = 0;
	params.offMeshConCount = 0;

	params.walkableHeight = config.characterHeight;
	params.walkableRadius = config.walkableRadius;
	params.walkableClimb = config.maxClimbHeight;
	rcVcopy(params.bmin,polyMesh.bmin);
	rcVcopy(params.bmax,polyMesh.bmax);
	params.cs = polyMesh.cs;
	params.ch = polyMesh.ch;

	uint8_t *navData;
	int32_t navDataSize;
	if(dtCreateNavMeshData(&params,&navData,&navDataSize) == false)
	{
		if(err != nullptr)
			*err = "Could not create detour navigation mesh!";
		return nullptr;
	}
	util::ScopeGuard sg([navData]() {
		dtFree(navData);
	});
	auto dtNav = std::shared_ptr<dtNavMesh>(dtAllocNavMesh(),[](dtNavMesh *dtNavMesh) {
		dtFreeNavMesh(dtNavMesh);
	});;
	if(dtNav == nullptr)
	{
		if(err != nullptr)
			*err = "Could not allocate detour navigation mesh!";
		return nullptr;
	}
	auto status = dtNav->init(navData,navDataSize,DT_TILE_FREE_DATA);
	if(dtStatusFailed(status))
	{
		if(err != nullptr)
			*err = "Could not initialize detour navigation mesh!";
		return nullptr;
	}
	sg.dismiss(); // TODO: Is this okay?
	return dtNav;
}

std::shared_ptr<RcNavMesh> pragma::nav::generate(Game &game,const Config &config,const BaseEntity &ent,std::string *err)
{
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr)
		return nullptr;
	auto numTris = hMdl->GetTriangleCount();
	std::vector<Vector3> vertices;
	std::vector<int32_t> triangles;
	vertices.reserve(hMdl->GetVertexCount());
	triangles.reserve(numTris *3u);
	auto &colMeshes = hMdl->GetCollisionMeshes();
	std::vector<ConvexArea> areas;
	areas.reserve(colMeshes.size());//numTris);
	for(auto &colMesh : colMeshes)
	{
		auto &meshVerts = colMesh->GetVertices();
		auto &meshTris = colMesh->GetTriangles();
		auto baseSurfMaterial = colMesh->GetSurfaceMaterial();
		auto &surfMaterials = colMesh->GetSurfaceMaterials();
		auto numMeshTris = meshTris.size() /3;
		auto idxOffset = vertices.size();
		vertices.reserve(vertices.size() +meshVerts.size());
		for(auto &v : meshVerts)
			vertices.push_back(v);

		triangles.reserve(triangles.size() +meshTris.size());
		for(auto idx : meshTris)
			triangles.push_back(idxOffset +idx);

		Vector3 min,max;
		colMesh->GetAABB(&min,&max);
		areas.push_back({});
		areas.back().verts.push_back(min);
		areas.back().verts.push_back(max);
		areas.back().area = baseSurfMaterial;
		/*areas.reserve(areas.size() +meshTris.size() /3);
		for(auto i=decltype(meshTris.size()){0u};i<meshTris.size();i+=3)
		{
			auto &v0 = meshVerts.at(meshTris.at(i));
			auto &v1 = meshVerts.at(meshTris.at(i +1));
			auto &v2 = meshVerts.at(meshTris.at(i +2));

			areas.push_back({});
			auto &area = areas.back();
			area.verts = {v0,v2,v1};
			area.area = baseSurfMaterial;
		}*/
	}

	return generate(game,config,vertices,triangles,&areas,err);
}
std::shared_ptr<RcNavMesh> pragma::nav::generate(Game &game,const Config &config,const std::vector<Vector3> &verts,const std::vector<int32_t> &indices,const std::vector<ConvexArea> *areas,std::string *err)
{
	//
	// Step 1. Initialize build config.
	//

	// See http://digestingduck.blogspot.com/2009/08/recast-settings-uncovered.html for more information
	auto cellSize = config.cellSize;
	auto cellHeight = config.cellHeight;
	auto agentHeight = config.characterHeight;
	auto agentRadius = config.walkableRadius;
	auto regionMinSize = config.minRegionSize;
	auto regionMergeSize = config.mergeRegionSize;
	auto edgeMaxLen = config.maxEdgeLength;
	auto edgeMaxError = config.maxSimplificationError;

	auto vertsPerPoly = config.vertsPerPoly;
	auto detailSampleDist = config.sampleDetailDist;
	auto detailSampleMaxError = config.sampleDetailMaxError;
	auto partitionType = config.partitionType;

	auto ctx = std::make_shared<rcContext>();
	auto keepInterResults = false;

	Vector3 min(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
	Vector3 max(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest());
	for(auto &v : verts)
	{
		uvec::min(&min,v);
		uvec::max(&max,v);
	}
	for(auto i=0;i<3;++i)
	{
		min[i] -= 0.01f;
		max[i] += 0.01f;
	}
	const auto *bmin = reinterpret_cast<float*>(&min);
	const auto *bmax =reinterpret_cast<float*>(&max);
	const auto *fverts = reinterpret_cast<const float*>(verts.data());
	const auto nverts = verts.size();
	const auto *tris = indices.data();
	const auto ntris = indices.size() /3;
	
	// Init build configuration from GUI
	rcConfig cfg;	
	memset(&cfg,0,sizeof(cfg));
	cfg.cs = cellSize;
	cfg.ch = cellHeight;
	cfg.walkableSlopeAngle = config.walkableSlopeAngle;
	cfg.walkableHeight = static_cast<int32_t>(ceilf(config.characterHeight /cfg.ch));
	cfg.walkableClimb = static_cast<int32_t>(floorf(config.maxClimbHeight /cfg.ch));
	cfg.walkableRadius = static_cast<int32_t>(ceilf(config.walkableRadius /cfg.cs));
	cfg.maxEdgeLen = static_cast<int32_t>(edgeMaxLen /cellSize);
	cfg.maxSimplificationError = edgeMaxError;
	cfg.minRegionArea = static_cast<int32_t>(rcSqr(regionMinSize));		// Note: area = size*size
	cfg.mergeRegionArea = static_cast<int32_t>(rcSqr(regionMergeSize));	// Note: area = size*size
	cfg.maxVertsPerPoly = static_cast<int32_t>(vertsPerPoly);
	cfg.detailSampleDist = detailSampleDist < 0.9f ? 0 : cellSize *detailSampleDist;
	cfg.detailSampleMaxError = cellHeight *detailSampleMaxError;
	
	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	rcVcopy(cfg.bmin,bmin);
	rcVcopy(cfg.bmax,bmax);
	rcCalcGridSize(cfg.bmin,cfg.bmax,cfg.cs,&cfg.width,&cfg.height);

	// Reset build times gathering.
	ctx->resetTimers();

	// Start the build process.	
	ctx->startTimer(RC_TIMER_TOTAL);
	
	ctx->log(RC_LOG_PROGRESS, "Building navigation:");
	ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", cfg.width, cfg.height);
	ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts/1000.0f, ntris/1000.0f);
	
	//
	// Step 2. Rasterize input polygon soup.
	//
	
	// Allocate voxel heightfield where we rasterize our input data to.
	auto m_solid = std::shared_ptr<rcHeightfield>(rcAllocHeightfield(),[](rcHeightfield *heightfield) {
		rcFreeHeightField(heightfield);
	});
	if(m_solid == nullptr)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return nullptr;
	}
	if(rcCreateHeightfield(ctx.get(),*m_solid,cfg.width,cfg.height,cfg.bmin,cfg.bmax,cfg.cs,cfg.ch) == false)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return nullptr;
	}
	
	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	std::vector<uint8_t> triAreas(ntris,0u);
	
	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	rcMarkWalkableTriangles(ctx.get(),cfg.walkableSlopeAngle,fverts,nverts,tris,ntris,triAreas.data());
	rcRasterizeTriangles(ctx.get(),fverts,nverts,tris,triAreas.data(),ntris,*m_solid,cfg.walkableClimb);

	if(keepInterResults == false)
		triAreas.clear();
	
	//
	// Step 3. Filter walkables surfaces.
	//
	
	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	rcFilterLowHangingWalkableObstacles(ctx.get(),cfg.walkableClimb,*m_solid);
	rcFilterLedgeSpans(ctx.get(),cfg.walkableHeight,cfg.walkableClimb,*m_solid);
	rcFilterWalkableLowHeightSpans(ctx.get(),cfg.walkableHeight,*m_solid);


	//
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	auto m_chf = std::shared_ptr<rcCompactHeightfield>(rcAllocCompactHeightfield(),[](rcCompactHeightfield *compactHeightfield) {
		rcFreeCompactHeightfield(compactHeightfield);
	});
	if(m_chf == nullptr)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return nullptr;
	}
	if(rcBuildCompactHeightfield(ctx.get(),cfg.walkableHeight,cfg.walkableClimb,*m_solid,*m_chf) == false)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return nullptr;
	}
	
	if(keepInterResults == false)
		m_solid = nullptr;
		
	// Erode the walkable area by agent radius.
	if(rcErodeWalkableArea(ctx.get(),cfg.walkableRadius,*m_chf) == false)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return nullptr;
	}

	// (Optional) Mark areas.
	if(areas != nullptr)
	{
		for(auto &convexArea : *areas)
		{
			if(convexArea.verts.empty())
				continue;
			/*auto hMin = std::numeric_limits<float>::max();
			auto hMax = std::numeric_limits<float>::lowest();
			Vector3 min(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
			Vector3 max(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest());
			for(auto &v : convexArea.verts)
			{
				hMin = umath::min(hMin,v.y);
				hMax = umath::max(hMax,v.y);

				uvec::min(&min,v);
				uvec::max(&max,v);
			}
			hMin = -10;//std::numeric_limits<float>::lowest();
			hMax = 10;//std::numeric_limits<float>::max();
			*/
			auto min = convexArea.verts.at(0);
			auto max = convexArea.verts.at(1);
			//rcMarkConvexPolyArea(ctx.get(),reinterpret_cast<const float*>(convexArea.verts.data()),convexArea.verts.size(),hMin,hMax,convexArea.area,*m_chf);
			rcMarkBoxArea(ctx.get(),reinterpret_cast<float*>(&min),reinterpret_cast<float*>(&max),convexArea.area,*m_chf);
		}
	}
	
	// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
	// There are 3 martitioning methods, each with some pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - partitions the heightfield into nice regions without holes or overlaps
	//   - the are some corner cases where this method creates produces holes and overlaps
	//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
	//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
	//   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
	// 2) Monotone partioning
	//   - fastest
	//   - partitions the heightfield into regions without holes and overlaps (guaranteed)
	//   - creates long thin polygons, which sometimes causes paths with detours
	//   * use this if you want fast navmesh generation
	// 3) Layer partitoining
	//   - quite fast
	//   - partitions the heighfield into non-overlapping regions
	//   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
	//   - produces better triangles than monotone partitioning
	//   - does not have the corner cases of watershed partitioning
	//   - can be slow and create a bit ugly tessellation (still better than monotone)
	//     if you have large open areas with small obstacles (not a problem if you use tiles)
	//   * good choice to use for tiled navmesh with medium and small sized tiles
	
	if(partitionType == Config::PartitionType::Watershed)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if(rcBuildDistanceField(ctx.get(),*m_chf) == false)
		{
			ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
			return nullptr;
		}
		
		// Partition the walkable surface into simple regions without holes.
		if(rcBuildRegions(ctx.get(),*m_chf,0,cfg.minRegionArea,cfg.mergeRegionArea) == false)
		{
			ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
			return nullptr;
		}
	}
	else if(partitionType == Config::PartitionType::Monotone)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if (rcBuildRegionsMonotone(ctx.get(),*m_chf,0,cfg.minRegionArea,cfg.mergeRegionArea) == false)
		{
			ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
			return nullptr;
		}
	}
	else // SAMPLE_PARTITION_LAYERS
	{
		// Partition the walkable surface into simple regions without holes.
		if(rcBuildLayerRegions(ctx.get(),*m_chf,0,cfg.minRegionArea) == false)
		{
			ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
			return nullptr;
		}
	}
	
	//
	// Step 5. Trace and simplify region contours.
	//
	
	// Create contours.
	auto m_cset = std::shared_ptr<rcContourSet>(rcAllocContourSet(),[](rcContourSet *contourSet) {
		rcFreeContourSet(contourSet);
	});
	if(m_cset == nullptr)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
		return nullptr;
	}
	if(rcBuildContours(ctx.get(),*m_chf,cfg.maxSimplificationError,cfg.maxEdgeLen,*m_cset) == false)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		return nullptr;
	}
	
	//
	// Step 6. Build polygons mesh from contours.
	//
	
	// Build polygon navmesh from the contours.
	auto m_pmesh = std::unique_ptr<rcPolyMesh,void(*)(rcPolyMesh*)>(rcAllocPolyMesh(),[](rcPolyMesh *polyMesh) {
		rcFreePolyMesh(polyMesh);
	});
	if(m_pmesh == nullptr)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
		return nullptr;
	}
	if(rcBuildPolyMesh(ctx.get(),*m_cset,cfg.maxVertsPerPoly,*m_pmesh) == false)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		return nullptr;
	}
	
	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//
	
	auto m_dmesh = std::unique_ptr<rcPolyMeshDetail,void(*)(rcPolyMeshDetail*)>(rcAllocPolyMeshDetail(),[](rcPolyMeshDetail *polyMesh) {
		rcFreePolyMeshDetail(polyMesh);
	});
	if(m_dmesh == nullptr)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
		return nullptr;
	}

	if(rcBuildPolyMeshDetail(ctx.get(),*m_pmesh,*m_chf,cfg.detailSampleDist,cfg.detailSampleMaxError,*m_dmesh) == false)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
		return nullptr;
	}

	if(keepInterResults == false)
	{
		m_chf = nullptr;
		m_cset = nullptr;
	}

	// At this point the navigation mesh data is ready, you can access it from m_pmesh.
	// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.
	
	//
	// (Optional) Step 8. Create Detour data from Recast poly mesh.
	//
	
	// The GUI may allow more max points per polygon than Detour can handle.
	// Only build the detour navmesh if we do not exceed the limit.
	std::unique_ptr<dtNavMesh,void(*)(dtNavMesh*)> m_navMesh = std::unique_ptr<dtNavMesh,void(*)(dtNavMesh*)>(nullptr,[](dtNavMesh*) {});
	if(cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = 0;
		int navDataSize = 0;

		// Update poly flags from areas.
		for(auto i=decltype(m_pmesh->npolys){0};i<m_pmesh->npolys;++i)
		{
			auto &area = m_pmesh->areas[i];
			if(area == RC_WALKABLE_AREA)
				area = 0u;
			if(area != 0u)
				std::cout<<"";
			auto *surfMat = game.GetSurfaceMaterial(area);
			if(surfMat != nullptr)
				m_pmesh->flags[i] = umath::to_integral(surfMat->GetNavigationFlags());
		}

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;
		// TODO
		/*params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
		params.offMeshConRad = m_geom->getOffMeshConnectionRads();
		params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
		params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
		params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
		params.offMeshConUserID = m_geom->getOffMeshConnectionId();
		params.offMeshConCount = m_geom->getOffMeshConnectionCount();*/
		params.walkableHeight = agentHeight;
		params.walkableRadius = agentRadius;
		params.walkableClimb = config.maxClimbHeight;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = cfg.cs;
		params.ch = cfg.ch;
		params.buildBvTree = true;
		
		if(dtCreateNavMeshData(&params,&navData,&navDataSize) == false)
		{
			ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			return nullptr;
		}

		util::ScopeGuard sg([navData]() {
			dtFree(navData);
		});
		
		m_navMesh = std::unique_ptr<dtNavMesh,void(*)(dtNavMesh*)>(dtAllocNavMesh(),[](dtNavMesh *navMesh) {
			dtFreeNavMesh(navMesh);
		});
		if(m_navMesh == nullptr)
		{
			ctx->log(RC_LOG_ERROR, "Could not create Detour navmesh");
			return nullptr;
		}
		
		dtStatus status;
		
		status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if(dtStatusFailed(status))
		{
			ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh");
			return nullptr;
		}
		sg.dismiss();
		/*status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			m_ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
			return nullptr;
		}*/
	}
	
	ctx->stopTimer(RC_TIMER_TOTAL);

	// Show performance stats.
	//duLogBuildTimes(*m_ctx, m_ctx->getAccumulatedTime(RC_TIMER_TOTAL));
	ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", m_pmesh->nverts, m_pmesh->npolys);
	
	auto m_totalBuildTimeMs = ctx->getAccumulatedTime(RC_TIMER_TOTAL) /1000.f;

	return std::make_shared<RcNavMesh>(std::move(m_pmesh),std::move(m_dmesh),std::move(m_navMesh));

	// Obsolete
	/*for(unsigned int i=0;i<meshes->size();i++)
	{
		BrushMesh *mesh = (*meshes)[i];
		std::vector<Side*> *sides;
		mesh->GetSides(&sides);
		for(unsigned int j=0;j<sides->size();j++)
		{
			Side *side = (*sides)[j];
			std::vector<Vector3> *sideVerts;
			side->GetVertices(&sideVerts);
			std::vector<uint16_t> *sideTris;
			side->GetTriangles(&sideTris);
			auto offset = vertices.size();
			for(auto &v : *sideVerts)
				vertices.push_back(v);
			for(auto i=decltype(sideTris->size()){0};i<sideTris->size();i+=3)
			{
				triangles.push_back(offset +sideTris->at(i));
				triangles.push_back(offset +sideTris->at(i +1));
				triangles.push_back(offset +sideTris->at(i +2));
			}
			//for(unsigned int k=0;k<sideTris->size();k++)
			//	triangles.push_back(base +(*sideTris)[k]);
		}
	}*/

	/*std::vector<Vector3> vertices;
	std::vector<int32_t> triangles;
	auto &mdl = ent->GetModel();
	vertices.reserve(mdl->GetVertexCount());
	triangles.reserve(mdl->GetTriangleCount());
	if(mdl != nullptr && mdl->GetMeshGroupCount() > 0)
	{
		auto meshGroup = mdl->GetMeshGroup(0);
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto &meshTriangles = subMesh->GetTriangles();
				auto &meshVerts = subMesh->GetVertices();
				triangles.reserve(triangles.size() +meshTriangles.size());
				vertices.reserve(vertices.size() +meshVerts.size());
				auto offset = vertices.size();
				for(auto &v : meshVerts)
					vertices.push_back(v.position);
				for(auto idx : meshTriangles)
					triangles.push_back(offset +idx);
			}
		}
	}*/
	/*auto &mdl = ent->GetModel();
	auto &colMeshes = mdl->GetCollisionMeshes();
	std::vector<Vector3> vertices;
	std::vector<int32_t> triangles;
	for(auto &colMesh : colMeshes)
	{
		auto &meshVerts = colMesh->GetVertices();
		auto &meshTriangles = colMesh->GetTriangles();
		vertices.reserve(vertices.size() +meshVerts.size());
		auto offset = vertices.size();
		for(auto &v : meshVerts)
			vertices.push_back(v);
		triangles.reserve(triangles.size() +meshTriangles.size());
		for(auto idx : meshTriangles)
			triangles.push_back(offset +idx);
	}*/

#if 0
	rcContext *context = new rcContext;
	rcConfig cfg;
	cfg.cs = 9.f;
	cfg.ch = 6.f;
	cfg.walkableSlopeAngle = 45.f;
	cfg.walkableHeight = umath::ceil(64.f /cfg.ch);
	cfg.walkableClimb = umath::floor(16.f /cfg.ch);
	cfg.walkableRadius = umath::ceil(16.f /cfg.cs);
	cfg.maxEdgeLen = static_cast<int>(256.f /cfg.cs);
	cfg.maxSimplificationError = 1.f;
	cfg.minRegionArea = static_cast<int>(rcSqr(0.f));
	cfg.mergeRegionArea = static_cast<int>(rcSqr(10.f));
	cfg.maxVertsPerPoly = DT_VERTS_PER_POLYGON;
	cfg.detailSampleDist = (6.f < 0.9f) ? 0 : cfg.cs *6.f;
	cfg.detailSampleMaxError = cfg.ch *1.f;

	context->resetTimers();
	context->startTimer(RC_TIMER_TOTAL);

	Vector3 min(0.f,0.f,0.f);
	Vector3 max(0.f,0.f,0.f);
	ent->GetCollisionBounds(&min,&max);
	rcVcopy(cfg.bmin,&min[0]);
	rcVcopy(cfg.bmax,&max[0]);
	rcCalcGridSize(cfg.bmin,cfg.bmax,cfg.cs,&cfg.width,&cfg.height);

	rcHeightfield *hf = rcAllocHeightfield();
	if(hf == NULL)
	{
		delete context;
		if(err != NULL)
			*err = "Unable to allocate heightfield: Out of memory!";
		return NULL;
	}
	if(!rcCreateHeightfield(context,*hf,cfg.width,cfg.height,cfg.bmin,cfg.bmax,cfg.cs,cfg.ch))
	{
		delete context;
		rcFreeHeightField(hf);
		if(err != NULL)
			*err = "Unable to create solid heightfield!";
		return NULL;
	}
	unsigned int numVerts = static_cast<unsigned int>(vertices.size());
	unsigned int numTriangles = static_cast<unsigned int>(triangles.size()) /3;
	unsigned char *triAreas = new unsigned char[numTriangles];
	if(triAreas == NULL)
	{
		delete context;
		rcFreeHeightField(hf);
		if(err != NULL)
			*err = "Out of memory!";
		return NULL;
	}
	memset(&triAreas[0],0,numTriangles *sizeof(unsigned char));
	rcMarkWalkableTriangles(
		context,
		cfg.walkableSlopeAngle,
		&vertices[0][0],numVerts,
		&triangles[0],numTriangles,
		&triAreas[0]
	);
	rcRasterizeTriangles(
		context,
		&vertices[0][0],numVerts,
		&triangles[0],&triAreas[0],numTriangles,
		*hf,cfg.walkableClimb
	);
	delete[] triAreas;

	rcFilterLowHangingWalkableObstacles(context,cfg.walkableClimb,*hf);
	rcFilterLedgeSpans(context,cfg.walkableHeight,cfg.walkableClimb,*hf);
	rcFilterWalkableLowHeightSpans(context,cfg.walkableHeight,*hf);

	rcCompactHeightfield *chf = rcAllocCompactHeightfield();
	if(chf == NULL)
	{
		delete context;
		rcFreeHeightField(hf);
		if(err != NULL)
			*err = "Unable to allocate compact heightfield: Out of memory!";
		return NULL;
	}
	if(!rcBuildCompactHeightfield(context,cfg.walkableHeight,cfg.walkableClimb,*hf,*chf))
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		if(err != NULL)
			*err = "Unable to create compact heightfield!";
		return NULL;
	}
	if(!rcErodeWalkableArea(context,cfg.walkableRadius,*chf))
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		if(err != NULL)
			*err = "Unable to erode walkable area!";
		return NULL;
	}
	if(!rcBuildDistanceField(context,*chf))
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		if(err != NULL)
			*err = "Unable to build distance field!";
		return NULL;
	}
	if(!rcBuildRegions(context,*chf,cfg.borderSize,cfg.minRegionArea,cfg.mergeRegionArea))
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		if(err != NULL)
			*err = "Unable to build regions!";
		return NULL;
	}
	rcContourSet *cset = rcAllocContourSet();
	if(cset == NULL)
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		if(err != NULL)
			*err = "Unable to allocate contour set: Out of memory!";
		return NULL;
	}
	if(!rcBuildContours(context,*chf,cfg.maxSimplificationError,cfg.maxEdgeLen,*cset))
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		if(err != NULL)
			*err = "Unable to build contours!!";
		return NULL;
	}
	if(cset->nconts == 0)
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		if(err != NULL)
			*err = "No contours were built!";
		return NULL;
	}
	rcPolyMesh *polyMesh = rcAllocPolyMesh();
	if(polyMesh == NULL)
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		if(err != NULL)
			*err = "Unable to allocate poly mesh: Out of memory!";
		return NULL;
	}
	if(!rcBuildPolyMesh(context,*cset,cfg.maxVertsPerPoly,*polyMesh))
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		rcFreePolyMesh(polyMesh);
		if(err != NULL)
			*err = "Unable to build poly mesh!";
		return NULL;
	}
	rcPolyMeshDetail *detailMesh = rcAllocPolyMeshDetail();
	if(detailMesh == NULL)
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		rcFreePolyMesh(polyMesh);
		if(err != NULL)
			*err = "Unable to allocate detail mesh: Out of memory!";
		return NULL;
	}
	if(!rcBuildPolyMeshDetail(context,*polyMesh,*chf,cfg.detailSampleDist,cfg.detailSampleMaxError,*detailMesh))
	{
		delete context;
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		rcFreePolyMesh(polyMesh);
		rcFreePolyMeshDetail(detailMesh);
		if(err != NULL)
			*err = "Unable to build poly mesh detail!";
		return NULL;
	}
	context->stopTimer(RC_TIMER_TOTAL);
	delete context;

	for(size_t i=0;i<polyMesh->npolys;i++)
	{
		if(polyMesh->areas[i] == RC_WALKABLE_AREA)
		{
			polyMesh->areas[i] = 0; // SAMPLE_POLYAREA_GROUND // TODO
			polyMesh->flags[i] = 1; // SAMPLE_POLYFLAGS_WALK // TODO
		}
	}

	dtNavMesh *dtMesh = InitializeDetourMesh(polyMesh,detailMesh,err);
	if(dtMesh == NULL)
	{
		rcFreeHeightField(hf);
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		rcFreePolyMesh(polyMesh);
		rcFreePolyMeshDetail(detailMesh);
		return NULL;
	}

	rcFreeHeightField(hf);
	rcFreeCompactHeightfield(chf);
	rcFreeContourSet(cset);
	RcNavMesh *navMesh = new RcNavMesh(polyMesh,detailMesh,dtMesh);
	m_navMeshes.push_back(navMesh);
	return navMesh;
#endif
}

std::shared_ptr<RcNavMesh> pragma::nav::generate(Game &game,const Config &config,std::string *err)
{
	auto *pWorld = game.GetWorld();
	if(pWorld == nullptr)
		return nullptr;
	return generate(game,config,pWorld->GetEntity(),err);
}

std::shared_ptr<pragma::nav::Mesh> pragma::nav::Mesh::Create(const std::shared_ptr<RcNavMesh> &rcMesh,const Config &config) {return Create<Mesh>(rcMesh,config);}
std::shared_ptr<pragma::nav::Mesh> pragma::nav::Mesh::Load(Game &game,const std::string &fname) {return Load<Mesh>(game,fname);}
pragma::nav::Mesh::Mesh(const std::shared_ptr<RcNavMesh> &rcMesh,const Config &config)
	: m_rcMesh(rcMesh),m_config(config)
{}
const pragma::nav::Config &pragma::nav::Mesh::GetConfig() const {return m_config;}
const std::shared_ptr<RcNavMesh> &pragma::nav::Mesh::GetRcNavMesh() const {return const_cast<Mesh*>(this)->GetRcNavMesh();}
std::shared_ptr<RcNavMesh> &pragma::nav::Mesh::GetRcNavMesh() {return m_rcMesh;}

static void write_poly_mesh(VFilePtrReal fOut,const rcPolyMesh &polyMesh,const std::unordered_map<uint32_t,uint32_t> &areaTranslationTable)
{
	fOut->Write(&polyMesh.nverts,sizeof(int32_t) *5 +sizeof(float) *8);

	fOut->Write(&polyMesh.verts[0],sizeof(uint16_t) *polyMesh.nverts *3);
	fOut->Write(&polyMesh.polys[0],sizeof(uint16_t) *polyMesh.maxpolys *2 *polyMesh.nvp);
	fOut->Write(&polyMesh.regs[0],sizeof(uint16_t) *polyMesh.maxpolys);
	fOut->Write(&polyMesh.flags[0],sizeof(uint16_t) *polyMesh.maxpolys);

	std::vector<uint8_t> areas(polyMesh.maxpolys);
	for(auto i=decltype(polyMesh.maxpolys){0};i<polyMesh.maxpolys;++i)
	{
		auto it = areaTranslationTable.find(polyMesh.areas[i]);
		if(it == areaTranslationTable.end())
			throw std::logic_error("Invalid nav mesh area!");
		areas.at(i) = it->second;
	}

	fOut->Write(areas.data(),areas.size() *sizeof(areas.front()));
}

static void write_poly_mesh(VFilePtrReal &fOut,const rcPolyMeshDetail &polyMeshDetail)
{
	fOut->Write(&polyMeshDetail.nmeshes,sizeof(int32_t) *3);

	fOut->Write(&polyMeshDetail.meshes[0],sizeof(uint32_t) *polyMeshDetail.nmeshes *4);
	fOut->Write(&polyMeshDetail.verts[0],sizeof(float) *polyMeshDetail.nverts *3);
	fOut->Write(&polyMeshDetail.tris[0],sizeof(uint8_t) *polyMeshDetail.ntris *4);
}

static void read_poly_mesh(VFilePtr &fIn,rcPolyMesh &polyMesh,std::vector<uint32_t> &areaTranslationTable)
{
	fIn->Read(&polyMesh.nverts,sizeof(int32_t) *5 +sizeof(float) *8);

	polyMesh.verts = new uint16_t[polyMesh.nverts *3];
	polyMesh.polys = new uint16_t[polyMesh.maxpolys *2 *polyMesh.nvp];
	polyMesh.regs = new uint16_t[polyMesh.maxpolys];
	polyMesh.flags = new uint16_t[polyMesh.maxpolys];
	polyMesh.areas = new uint8_t[polyMesh.maxpolys];

	fIn->Read(&polyMesh.verts[0],sizeof(uint16_t) *polyMesh.nverts *3);
	fIn->Read(&polyMesh.polys[0],sizeof(uint16_t) *polyMesh.maxpolys *2 *polyMesh.nvp);
	fIn->Read(&polyMesh.regs[0],sizeof(uint16_t) *polyMesh.maxpolys);
	fIn->Read(&polyMesh.flags[0],sizeof(uint16_t) *polyMesh.maxpolys);
	fIn->Read(&polyMesh.areas[0],sizeof(uint8_t) *polyMesh.maxpolys);

	for(auto i=decltype(polyMesh.maxpolys){0};i<polyMesh.maxpolys;++i)
		polyMesh.areas[i] = (i < areaTranslationTable.size()) ? areaTranslationTable.at(i) : polyMesh.areas[i];
}

static void read_poly_mesh(VFilePtr &fIn,rcPolyMeshDetail &polyMeshDetail)
{
	fIn->Read(&polyMeshDetail.nmeshes,sizeof(int) *3);

	polyMeshDetail.meshes = new uint32_t[polyMeshDetail.nmeshes *4];
	polyMeshDetail.verts = new float[polyMeshDetail.nverts *3];
	polyMeshDetail.tris = new uint8_t[polyMeshDetail.ntris *4];

	fIn->Read(&polyMeshDetail.meshes[0],sizeof(uint32_t) *polyMeshDetail.nmeshes *4);
	fIn->Read(&polyMeshDetail.verts[0],sizeof(float) *polyMeshDetail.nverts *3);
	fIn->Read(&polyMeshDetail.tris[0],sizeof(uint8_t) *polyMeshDetail.ntris *4);
}

bool pragma::nav::Mesh::Save(Game &game,const std::string &fname)
{
	if(m_rcMesh == nullptr)
		return false;
	auto &navMesh = *m_rcMesh;
	auto fOut = FileManager::OpenFile<VFilePtrReal>(fname.c_str(),"wb");
	if(fOut == nullptr)
		return false;
	const auto *id = "WNAV";
	fOut->Write(&id[0],4);
	fOut->Write<uint16_t>(WNAV_VERSION); // Version

	fOut->Write<Config>(m_config);

	std::vector<std::string> surfaceMaterialNames;
	std::unordered_map<uint32_t,uint32_t> surfaceMaterialTable;
	auto &polyMesh = navMesh.GetPolyMesh();
	auto numAreas = polyMesh.maxpolys;
	for(auto i=decltype(numAreas){0};i<numAreas;++i)
	{
		auto areaIdx = polyMesh.areas[i];
		auto it = surfaceMaterialTable.find(areaIdx);
		if(it != surfaceMaterialTable.end())
			continue;
		surfaceMaterialTable.insert(std::make_pair(areaIdx,surfaceMaterialNames.size()));
		auto *surfMat = game.GetSurfaceMaterial(areaIdx);
		if(surfMat != nullptr)
			surfaceMaterialNames.push_back(surfMat->GetIdentifier());
		else
		{
			Con::cwar<<"WARNING: Nav mesh poly with unknown surface material index "<<+areaIdx<<"! Setting to 0..."<<Con::endl;
			surfaceMaterialNames.push_back("");
		}
	}

	// Write surface material names
	fOut->Write<uint32_t>(surfaceMaterialNames.size());
	for(auto &name : surfaceMaterialNames)
		fOut->WriteString(name);

	write_poly_mesh(fOut,polyMesh,surfaceMaterialTable);

	auto &polyMeshDetail = navMesh.GetPolyMeshDetail();
	write_poly_mesh(fOut,polyMeshDetail);
	/*
	rcHeightfield *heightField = navMesh->GetHeightField();
	rcCompactHeightfield *cHeightField = navMesh->GetCompactHeightField();
	rcContourSet *contourSet = navMesh->GetContourSet();
	rcPolyMesh *polyMesh = navMesh->GetPolyMesh();
	rcPolyMeshDetail *polyMeshDetail = navMesh->GetPolyMeshDetail();

	unsigned long long locHeightField = fOut->Tell();
	fOut->Write<unsigned long long>((unsigned long long)(0));

	unsigned long long locCompactHeightField = fOut->Tell();
	fOut->Write<unsigned long long>((unsigned long long)(0));

	unsigned long long locContourSet = fOut->Tell();
	fOut->Write<unsigned long long>((unsigned long long)(0));

	unsigned long long locPolyMesh = fOut->Tell();
	fOut->Write<unsigned long long>((unsigned long long)(0));

	unsigned long long locPolyMeshDetail = fOut->Tell();
	fOut->Write<unsigned long long>((unsigned long long)(0));

	unsigned long long offset = fOut->Tell();
	fOut->Seek(locHeightField);
	fOut->Write<unsigned long long>(offset);
	fOut->Seek(offset);
	WriteHeightField(fOut,heightField);

	offset = fOut->Tell();
	fOut->Seek(locCompactHeightField);
	fOut->Write<unsigned long long>(offset);
	fOut->Seek(offset);
	fOut->Write<unsigned long long>(sizeof(*cHeightField));
	fOut->Write(cHeightField,sizeof(*cHeightField));

	offset = fOut->Tell();
	fOut->Seek(locContourSet);
	fOut->Write<unsigned long long>(offset);
	fOut->Seek(offset);
	fOut->Write<unsigned long long>(sizeof(*contourSet));
	fOut->Write(contourSet,sizeof(*contourSet));

	offset = fOut->Tell();
	fOut->Seek(locPolyMesh);
	fOut->Write<unsigned long long>(offset);
	fOut->Seek(offset);
	fOut->Write<unsigned long long>(sizeof(*polyMesh));
	fOut->Write(polyMesh,sizeof(*polyMesh));

	offset = fOut->Tell();
	fOut->Seek(locPolyMeshDetail);
	fOut->Write<unsigned long long>(offset);
	fOut->Seek(offset);
	fOut->Write<unsigned long long>(sizeof(*polyMeshDetail));
	fOut->Write(polyMeshDetail,sizeof(*polyMeshDetail));
	*/
	return true;
}

std::shared_ptr<RcNavMesh> pragma::nav::load(Game &game,const std::string &fname,Config &outConfig)
{
	auto fIn = FileManager::OpenFile(fname.c_str(),"rb");
	if(fIn == nullptr)
		return nullptr;
	std::array<char,4> header;
	fIn->Read(header.data(),header.size() *sizeof(header.front()));
	if(header.at(0) != 'W' || header.at(1) != 'N' || header.at(2) != 'A' || header.at(3) != 'V')
		return nullptr;
	auto version = fIn->Read<uint16_t>();
	if(version < 1 || version > WNAV_VERSION)
	{
		Con::cwar<<"WARNING: Incompatible navigation mesh format version "<<+version<<"!"<<Con::endl;
		return nullptr;
	}

	outConfig = {};
	if(version >= 0x0002)
		outConfig = fIn->Read<Config>();
	else
	{
		outConfig = {
			32.f /* walkableRadius */,
			64.f, /* characterHeight */
			20.f, /* maxClimbHeight */
			45.f /* walkableSlopeAngle */
		};
	}

	std::vector<std::string> surfaceMaterialNames;
	if(version >= 0x0002)
	{
		// Read surface material names
		auto numSurfaceMaterials = fIn->Read<uint32_t>();
		surfaceMaterialNames.reserve(numSurfaceMaterials);
		for(auto i=decltype(numSurfaceMaterials){0};i<numSurfaceMaterials;++i)
			surfaceMaterialNames.push_back(fIn->ReadString());
	}

	std::vector<uint32_t> surfaceMaterialTable;
	surfaceMaterialTable.reserve(surfaceMaterialNames.size());
	for(auto &name : surfaceMaterialNames)
	{
		auto *surfMat = game.GetSurfaceMaterial(name);
		surfaceMaterialTable.push_back((surfMat != nullptr) ? surfMat->GetIndex() : 0u);
		if(surfMat == nullptr)
			Con::cwar<<"WARNING: Nav mesh poly with unknown surface material '"<<name<<"'! Setting to 0..."<<Con::endl;
	}

	auto polyMesh = std::shared_ptr<rcPolyMesh>(rcAllocPolyMesh(),[](rcPolyMesh *polyMesh) {
		rcFreePolyMesh(polyMesh);
	});
	if(polyMesh == nullptr)
		return nullptr;
	read_poly_mesh(fIn,*polyMesh,surfaceMaterialTable);

	auto polyMeshDetail = std::shared_ptr<rcPolyMeshDetail>(rcAllocPolyMeshDetail(),[](rcPolyMeshDetail *polyMeshDetail) {
		rcFreePolyMeshDetail(polyMeshDetail);
	});
	if(polyMeshDetail == nullptr)
		return nullptr;
	read_poly_mesh(fIn,*polyMeshDetail);

		
	/*
	fIn->Seek(fIn->Tell() +sizeof(unsigned long long) *5);

	rcHeightfield *heightField = new rcHeightfield;
	rcCompactHeightfield *cHeightField = new rcCompactHeightfield;
	rcContourSet *contourSet = new rcContourSet;
	rcPolyMesh *polyMesh = new rcPolyMesh;
	rcPolyMeshDetail *polyMeshDetail = new rcPolyMeshDetail;

	unsigned long long szHeightField = fIn->Read<unsigned long long>();
	fIn->Read(heightField,szHeightField);

	unsigned long long szCompactHeightField = fIn->Read<unsigned long long>();
	fIn->Read(cHeightField,szCompactHeightField);

	unsigned long long szContourSet = fIn->Read<unsigned long long>();
	fIn->Read(contourSet,szContourSet);

	unsigned long long szPolyMesh = fIn->Read<unsigned long long>();
	fIn->Read(polyMesh,szPolyMesh);

	unsigned long long szPolyMeshDetail = fIn->Read<unsigned long long>();
	fIn->Read(polyMeshDetail,szPolyMeshDetail);

	FileManager::CloseFile(fIn);
	
	RcNavMesh *navMesh = new RcNavMesh(NULL,heightField,cHeightField,contourSet,polyMesh,polyMeshDetail);
	*/
	fIn.reset();
	auto dtMesh = initialize_detour_mesh(*polyMesh,*polyMeshDetail,outConfig);
	if(dtMesh == nullptr)
		return nullptr;
	auto navMesh = std::make_shared<RcNavMesh>(polyMesh,polyMeshDetail,dtMesh);
	if(navMesh == nullptr)
		return nullptr;
	return navMesh;
}

bool pragma::nav::Mesh::FindNearestPoly(const Vector3 &pos,dtPolyRef &ref)
{
	if(m_rcMesh == nullptr)
		return false;
	auto &mesh = *m_rcMesh;
	auto navQuery = std::unique_ptr<dtNavMeshQuery,decltype(&dtFreeNavMeshQuery)>(dtAllocNavMeshQuery(),dtFreeNavMeshQuery);
	auto status = navQuery->init(&mesh.GetNavMesh(),2048); // TODO
	if(dtStatusFailed(status))
		return false;
	const Vector3 extents(256.f,256.f,256.f); // TODO
	dtQueryFilter filter;
	filter.setIncludeFlags(0xFFFF); // TODO
	filter.setExcludeFlags(0); // TODO
	Vector3 nearestPoint {};
	status = navQuery->findNearestPoly(&pos[0],&extents[0],&filter,&ref,&nearestPoint[0]);
	if(dtStatusFailed(status))
		return false;
	return true;
}

bool pragma::nav::Mesh::RayCast(const Vector3 &start,const Vector3 &end,Vector3 &hit)
{
	if(m_rcMesh == nullptr)
		return false;
	auto &mesh = *m_rcMesh;
	auto navQuery = std::unique_ptr<dtNavMeshQuery,decltype(&dtFreeNavMeshQuery)>(dtAllocNavMeshQuery(),dtFreeNavMeshQuery);
	auto status = navQuery->init(&mesh.GetNavMesh(),2048); // TODO
	if(dtStatusFailed(status))
		return false;
	dtPolyRef startRef;
	dtPolyRef endRef;
	if(FindNearestPoly(start,startRef) == false || FindNearestPoly(end,endRef) == false)
		return false;
	const Vector3 extents(256.f,256.f,256.f); // TODO
	dtQueryFilter filter;
	filter.setIncludeFlags(0xFFFF); // TODO
	filter.setExcludeFlags(0); // TODO

	static std::unique_ptr<dtRaycastHit> rayHit = nullptr;
	static std::array<dtPolyRef,2048> hitRefs;
	if(rayHit == nullptr)
	{
		rayHit = std::make_unique<dtRaycastHit>();
		rayHit->pathCount = static_cast<int32_t>(hitRefs.size());
		rayHit->path = hitRefs.data();
	}

	status = navQuery->raycast(startRef,&start[0],&end[0],&filter,0,rayHit.get());
	if(dtStatusFailed(status) || rayHit->t == 0.f)
		return false;
	if(rayHit->t > 1.f)
		hit = end;
	else
		hit = start +(end -start) *rayHit->t;
	return true;
}

std::shared_ptr<RcPathResult> pragma::nav::Mesh::FindPath(const Vector3 &start,const Vector3 &end)
{
	if(m_rcMesh == nullptr)
		return nullptr;
	auto &mesh = *m_rcMesh;
	auto navQuery = std::shared_ptr<dtNavMeshQuery>(dtAllocNavMeshQuery(),[](dtNavMeshQuery *navQuery) {
		dtFreeNavMeshQuery(navQuery);
	});
	auto status = navQuery->init(&mesh.GetNavMesh(),2048); // TODO
	if(dtStatusFailed(status))
	{
		//Con::cerr<<"FAILED #1: "<<start.x<<","<<start.y<<","<<start.z<<" => "<<end.x<<","<<end.y<<","<<end.z<<Con::endl;
		return nullptr;
	}
	const Vector3 extents(256.f,256.f,256.f); // TODO
	dtQueryFilter filter;
	filter.setIncludeFlags(0xFFFF); // TODO
	filter.setExcludeFlags(0); // TODO
	dtPolyRef startRef;
	Vector3 startPoint;
	auto statusStart = navQuery->findNearestPoly(
		&start[0],&extents[0],
		&filter,&startRef,
		&startPoint[0]
	);
	if(dtStatusFailed(statusStart) == false && startRef != 0)
	{
		dtPolyRef endRef;
		Vector3 endPoint;
		auto statusEnd = navQuery->findNearestPoly(
			&end[0],&extents[0],
			&filter,&endRef,
			&endPoint[0]
		);
		if(!dtStatusFailed(statusEnd) && endRef != 0)
		{
			int maxPath = 128; // TODO
			auto r = std::make_shared<RcPathResult>(mesh,navQuery,startPoint,endPoint,maxPath); // TODO
			int32_t pathCount = 0;
			auto findStatus = navQuery->findPath(
				startRef,endRef,
				&startPoint[0],&endPoint[0],
				&filter,&r->path[0],
				&pathCount,maxPath
			);
			r->pathCount = pathCount +2;
			return r;
		}
	}
	//Con::cerr<<"FAILED #2: "<<start.x<<","<<start.y<<","<<start.z<<" => "<<end.x<<","<<end.y<<","<<end.z<<Con::endl;
	return nullptr;
}

////////////////////////////////////

RcPathResult::RcPathResult(RcNavMesh &pNavMesh,const std::shared_ptr<dtNavMeshQuery> &pQuery,Vector3 &pStart,Vector3 &pEnd,unsigned int numResults)
	: navMesh(pNavMesh),query(pQuery),start(pStart),end(pEnd),pathCount(0)
{
	path.resize(numResults);
}
bool RcPathResult::GetNode(uint32_t nodeId,const Vector3 &closest,Vector3 &node) const
{
	if(nodeId == 0)
	{
		node = start;
		return true;
	}
	if(nodeId == pathCount -1)
	{
		node = end;
		return true;
	}
	if(nodeId >= pathCount)
		return false;
	--nodeId;
	query->closestPointOnPolyBoundary(
		path[nodeId],&closest[0],&node[0]
	);
	return true;
}
