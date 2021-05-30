/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/game/c_game.h"
#include <pragma/entities/baseentity.h>
#include <pragma/entities/baseworld.h>
#include "pragma/ai/c_navsystem.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "pragma/console/c_cvar.h"
#include <pragma/console/convars.h>
#include <pragma/physics/raytraces.h>
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

std::shared_ptr<pragma::nav::CMesh> pragma::nav::CMesh::Create(const std::shared_ptr<RcNavMesh> &rcMesh,const Config &config) {return Mesh::Create<CMesh>(rcMesh,config);}
std::shared_ptr<pragma::nav::CMesh> pragma::nav::CMesh::Load(Game &game,const std::string &fname) {return Mesh::Load<CMesh>(game,fname);}
void pragma::nav::CMesh::UpdateDebugPath(Vector3 &start,Vector3 &end)
{
	auto res = FindPath(start,end);
	if(res == nullptr)
		return;
	std::vector<Vector3> lineVerts;
	lineVerts.reserve(res->pathCount +2);

	const auto lineOffset = Vector3(0.f,10.f,0.f);
	auto nextPoint = res->start;
	for(auto i=decltype(res->pathCount){0};i<res->pathCount;i++)
	{
		auto &pathPoint = nextPoint;
		res->GetNode(i,pathPoint,nextPoint);
		lineVerts.push_back(pathPoint +lineOffset);
	}
	m_dbgNavPath = DebugRenderer::DrawLines(lineVerts,Color::Yellow);

	m_numPath = static_cast<uint32_t>(lineVerts.size());
}

void pragma::nav::CMesh::UpdateDepthPathTargets()
{
	if(m_dbgPathStart != nullptr)
		m_dbgPointLines[0] = DebugRenderer::DrawLine(*m_dbgPathStart,*m_dbgPathStart +Vector3(0.f,32.f,0.f),Color::Magenta);
	if(m_dbgPathEnd != nullptr)
		m_dbgPointLines[0] = DebugRenderer::DrawLine(*m_dbgPathEnd,*m_dbgPathEnd +Vector3(0.f,32.f,0.f),Color::Magenta);
}

void pragma::nav::CMesh::SetDebugPathStart(Vector3 &start)
{
	if(m_dbgPathStart != nullptr)
		*m_dbgPathStart = start;
	else
		m_dbgPathStart = std::make_unique<Vector3>(start);
	if(m_dbgPathEnd != nullptr)
		UpdateDebugPath(*m_dbgPathStart,*m_dbgPathEnd);
	UpdateDepthPathTargets();
}
void pragma::nav::CMesh::SetDebugPathEnd(Vector3 &end)
{
	if(m_dbgPathEnd != nullptr)
		*m_dbgPathEnd = end;
	else
		m_dbgPathEnd = std::make_unique<Vector3>(end);
	if(m_dbgPathStart != nullptr)
		UpdateDebugPath(*m_dbgPathStart,*m_dbgPathEnd);
	UpdateDepthPathTargets();
}

void pragma::nav::CMesh::ShowNavMeshes(bool b)
{
	if(b == m_bShowNavMeshes)
		return;
	if(b == false)
	{
		m_bShowNavMeshes = b;
		m_dbgNavMesh = nullptr;
		return;
	}
	if(c_game == nullptr)
		return;
	auto &navMesh = GetRcNavMesh();
	if(navMesh == nullptr)
		return;
	m_bShowNavMeshes = b;

	auto &polyMesh = navMesh->GetPolyMesh();
	std::vector<Vector3> triangleVerts;
	{
		const auto fDrawMeshTile = [&triangleVerts](const dtNavMesh &mesh,const dtMeshTile &tile) {
			auto base = mesh.getPolyRefBase(&tile);

			auto tileNum = mesh.decodePolyIdTile(base);
	
			for(auto i=0;i<tile.header->polyCount;++i)
			{
				const auto *p = &tile.polys[i];
				if(p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION)	// Skip off-mesh links.
					continue;
			
				const auto *pd = &tile.detailMeshes[i];

				/*unsigned int col;
				if(query && query->isInClosedList(base | (dtPolyRef)i))
					col = duRGBA(255,196,0,64);
				else
				{
					if (flags & DU_DRAWNAVMESH_COLOR_TILES)
					{
						col = duIntToCol(tileNum, 128);
					}
					else
					{
						if (p->getArea() == 0) // Treat zero area type as default.
							col = duRGBA(0,192,255,64);
						else
							col = duIntToCol(p->getArea(), 64);
					}
				}*/
		
				triangleVerts.reserve(triangleVerts.size() +pd->triCount *3u);
				for(auto j=0; j<pd->triCount;++j)
				{
					const auto *t = &tile.detailTris[(pd->triBase +j) *4];
					for(auto k=0;k<3;++k)
					{
						triangleVerts.push_back({});
						auto &v = triangleVerts.back();
						float *dtVert = nullptr;
						if(t[k] < p->vertCount)
							dtVert = &tile.verts[p->verts[t[k]] *3];
						else
							dtVert = &tile.detailVerts[(pd->vertBase +t[k] -p->vertCount) *3];
						v[0] = dtVert[0];
						v[1] = dtVert[1];
						v[2] = dtVert[2];
					}
				}
			}
	
			// Draw inter poly boundaries
			//drawPolyBoundaries(dd, tile, duRGBA(0,48,64,32), 1.5f, true);
	
			// Draw outer poly boundaries
			//drawPolyBoundaries(dd, tile, duRGBA(0,48,64,220), 2.5f, false);
			/*
			const unsigned int vcol = duRGBA(0,0,0,196);
			dd->begin(DU_DRAW_POINTS, 3.0f);
			for (int i = 0; i < tile->header->vertCount; ++i)
			{
				const float* v = &tile->verts[i*3];
				dd->vertex(v[0], v[1], v[2], vcol);
			}
			dd->end();

			dd->depthMask(true);*/
		};

		const auto &dtNavMesh = navMesh->GetNavMesh();
		auto numTiles = dtNavMesh.getMaxTiles();
		for(auto i=decltype(numTiles){0};i<numTiles;++i)
		{
			auto *tile = dtNavMesh.getTile(i);
			if(tile == nullptr || tile->header == nullptr)
				continue;
			fDrawMeshTile(dtNavMesh,*tile);
		}
	}

	/*const auto nvp = polyMesh.nvp;
	const auto cs = polyMesh.cs;
	const auto ch = polyMesh.ch;
	const auto *orig = polyMesh.bmin;
	triangleVerts.reserve(polyMesh.npolys *(nvp -2));
	for(auto i=decltype(polyMesh.npolys){0};i<polyMesh.npolys;++i)
	{
		const auto *p = &polyMesh.polys[i *nvp *2];
		uint32_t vi[3];
		for(auto j=decltype(nvp){2};j<nvp;++j)
		{
			if(p[j] == RC_MESH_NULL_IDX)
				break;
			vi[0] = p[0];
			vi[1] = p[j-1];
			vi[2] = p[j];
			for(uint8_t k=0;k<3;++k)
			{
				const auto *v = &polyMesh.verts[vi[k] *3];
				const auto x = orig[0] +v[0] *cs;
				const auto y = orig[1] +(v[1] +1) *ch;
				const auto z = orig[2] +v[2] *cs;
				triangleVerts.push_back({x,y,z});
			}
		}
	}*/
	auto col = Color::Aqua;
	col.a = 32;
	m_dbgNavMesh = DebugRenderer::DrawMesh(triangleVerts,col,Color::Maroon);
}

void pragma::nav::CMesh::Clear()
{
	ShowNavMeshes(false);
	for(auto &l : m_dbgPointLines)
		l = nullptr;
	m_dbgNavPath = nullptr;
	m_dbgNavMesh = nullptr;
	m_dbgPathStart = nullptr;
	m_dbgPathEnd = nullptr;
}

static auto cvShowNavMeshes = GetClientConVar("debug_nav_show_meshes");
REGISTER_CONVAR_CALLBACK_CL(debug_nav_show_meshes,[](NetworkState*,ConVar*,bool,bool val) {
	if(c_game == NULL || c_game->LoadNavMesh() == false)
		return;
	auto &navMesh = c_game->GetNavMesh();
	if(navMesh == nullptr)
		return;
	static_cast<pragma::nav::CMesh&>(*navMesh).ShowNavMeshes(val);
});

////////////////////////////////////

void CMD_debug_nav_path_start(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string>&)
{
	CHECK_CHEATS("debug_nav_path_start",state,);
	if(c_game == nullptr || c_game->LoadNavMesh() == false || pl == nullptr)
		return;
	auto &navMesh = c_game->GetNavMesh();
	if(navMesh == nullptr)
		return;
	auto &ent = pl->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(charComponent.expired() && pTrComponent == nullptr)
		return;
	auto origin = charComponent.valid() ? charComponent->GetEyePosition() : pTrComponent->GetPosition();
	auto dir = charComponent.valid() ? charComponent->GetViewForward() : pTrComponent->GetForward();

	TraceData data {};
	data.SetFilter(ent);
	data.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter);
	data.SetSource(origin);
	data.SetTarget(origin +dir *65'536.f);
	auto r = c_game->RayCast(data);
	if(r.hitType == RayCastHitType::None)
		return;
	static_cast<pragma::nav::CMesh&>(*navMesh).SetDebugPathStart(r.position);
}

void CMD_debug_nav_path_end(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string>&)
{
	CHECK_CHEATS("debug_nav_path_end",state,);
	if(c_game == nullptr || c_game->LoadNavMesh() == false || pl == nullptr)
		return;
	auto &navMesh = c_game->GetNavMesh();
	if(navMesh == nullptr)
		return;
	auto &ent = pl->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(charComponent.expired() && pTrComponent == nullptr)
		return;
	auto origin = charComponent.valid() ? charComponent->GetEyePosition() : pTrComponent->GetPosition();
	auto dir = charComponent.valid() ? charComponent->GetViewForward() : pTrComponent->GetForward();

	TraceData data {};
	data.SetFilter(ent);
	data.SetFlags(RayCastFlags::Default | RayCastFlags::InvertFilter);
	data.SetSource(origin);
	data.SetTarget(origin +dir *65'536.f);
	auto r = c_game->RayCast(data);
	if(r.hitType == RayCastHitType::None)
		return;
	static_cast<pragma::nav::CMesh&>(*navMesh).SetDebugPathEnd(r.position);
}

