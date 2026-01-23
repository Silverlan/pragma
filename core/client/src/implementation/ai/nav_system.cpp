// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "pragma/console/helper.hpp"

module pragma.client;

import :ai;
import :client_state;
import :console.register_commands;
import :debug;
import :game;

std::shared_ptr<pragma::nav::CMesh> pragma::nav::CMesh::Create(const std::shared_ptr<RcNavMesh> &rcMesh, const Config &config) { return Mesh::Create<CMesh>(rcMesh, config); }
std::shared_ptr<pragma::nav::CMesh> pragma::nav::CMesh::Load(Game &game, const std::string &fname) { return Mesh::Load<CMesh>(game, fname); }
void pragma::nav::CMesh::UpdateDebugPath(Vector3 &start, Vector3 &end)
{
	auto res = FindPath(start, end);
	if(res == nullptr)
		return;
	std::vector<Vector3> lineVerts;
	lineVerts.reserve(res->pathCount + 2);

	const auto lineOffset = Vector3(0.f, 10.f, 0.f);
	auto nextPoint = res->start;
	for(auto i = decltype(res->pathCount) {0}; i < res->pathCount; i++) {
		auto &pathPoint = nextPoint;
		res->GetNode(i, pathPoint, nextPoint);
		lineVerts.push_back(pathPoint + lineOffset);
	}
	m_dbgNavPath = debug::DebugRenderer::DrawLines(lineVerts, colors::Yellow);

	m_numPath = static_cast<uint32_t>(lineVerts.size());
}

void pragma::nav::CMesh::UpdateDepthPathTargets()
{
	if(m_dbgPathStart != nullptr)
		m_dbgPointLines[0] = debug::DebugRenderer::DrawLine(*m_dbgPathStart, *m_dbgPathStart + Vector3(0.f, 32.f, 0.f), colors::Magenta);
	if(m_dbgPathEnd != nullptr)
		m_dbgPointLines[0] = debug::DebugRenderer::DrawLine(*m_dbgPathEnd, *m_dbgPathEnd + Vector3(0.f, 32.f, 0.f), colors::Magenta);
}

void pragma::nav::CMesh::SetDebugPathStart(Vector3 &start)
{
	if(m_dbgPathStart != nullptr)
		*m_dbgPathStart = start;
	else
		m_dbgPathStart = std::make_unique<Vector3>(start);
	if(m_dbgPathEnd != nullptr)
		UpdateDebugPath(*m_dbgPathStart, *m_dbgPathEnd);
	UpdateDepthPathTargets();
}
void pragma::nav::CMesh::SetDebugPathEnd(Vector3 &end)
{
	if(m_dbgPathEnd != nullptr)
		*m_dbgPathEnd = end;
	else
		m_dbgPathEnd = std::make_unique<Vector3>(end);
	if(m_dbgPathStart != nullptr)
		UpdateDebugPath(*m_dbgPathStart, *m_dbgPathEnd);
	UpdateDepthPathTargets();
}

void pragma::nav::CMesh::ShowNavMeshes(bool b)
{
	if(b == m_bShowNavMeshes)
		return;
	if(b == false) {
		m_bShowNavMeshes = b;
		m_dbgNavMesh = nullptr;
		return;
	}
	if(get_cgame() == nullptr)
		return;
	auto &navMesh = GetRcNavMesh();
	if(navMesh == nullptr)
		return;
	m_bShowNavMeshes = b;

	auto &polyMesh = navMesh->GetPolyMesh();
	std::vector<Vector3> triangleVerts;
	{
		const auto fDrawMeshTile = [&triangleVerts](const dtNavMesh &mesh, const dtMeshTile &tile) {
			auto base = mesh.getPolyRefBase(&tile);

			auto tileNum = mesh.decodePolyIdTile(base);

			for(auto i = 0; i < tile.header->polyCount; ++i) {
				const auto *p = &tile.polys[i];
				if(p->getType() == DT_POLYTYPE_OFFMESH_CONNECTION) // Skip off-mesh links.
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

				triangleVerts.reserve(triangleVerts.size() + pd->triCount * 3u);
				for(auto j = 0; j < pd->triCount; ++j) {
					const auto *t = &tile.detailTris[(pd->triBase + j) * 4];
					for(auto k = 0; k < 3; ++k) {
						triangleVerts.push_back({});
						auto &v = triangleVerts.back();
						float *dtVert = nullptr;
						if(t[k] < p->vertCount)
							dtVert = &tile.verts[p->verts[t[k]] * 3];
						else
							dtVert = &tile.detailVerts[(pd->vertBase + t[k] - p->vertCount) * 3];
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
		for(auto i = decltype(numTiles) {0}; i < numTiles; ++i) {
			auto *tile = dtNavMesh.getTile(i);
			if(tile == nullptr || tile->header == nullptr)
				continue;
			fDrawMeshTile(dtNavMesh, *tile);
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
	auto col = colors::Aqua;
	col.a = 32;
	m_dbgNavMesh = debug::DebugRenderer::DrawMesh(triangleVerts, {col, colors::Maroon});
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

static auto cvShowNavMeshes = pragma::console::get_client_con_var("debug_nav_show_meshes");
namespace {
	auto _ = pragma::console::client::register_variable_listener<bool>(
	  "debug_nav_show_meshes", +[](pragma::NetworkState *, const pragma::console::ConVar &, bool, bool val) {
		  if(pragma::get_cgame() == nullptr || pragma::get_cgame()->LoadNavMesh() == false)
			  return;
		  auto &navMesh = pragma::get_cgame()->GetNavMesh();
		  if(navMesh == nullptr)
			  return;
		  static_cast<pragma::nav::CMesh &>(*navMesh).ShowNavMeshes(val);
	  });
}

////////////////////////////////////

void CMD_debug_nav_path_start(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &)
{
	if(!check_cheats("debug_nav_path_start", state))
		return;
	if(pragma::get_cgame() == nullptr || pragma::get_cgame()->LoadNavMesh() == false || pl == nullptr)
		return;
	auto &navMesh = pragma::get_cgame()->GetNavMesh();
	if(navMesh == nullptr)
		return;
	auto &ent = pl->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(charComponent.expired() && pTrComponent == nullptr)
		return;
	auto origin = charComponent.valid() ? charComponent->GetEyePosition() : pTrComponent->GetPosition();
	auto dir = charComponent.valid() ? charComponent->GetViewForward() : pTrComponent->GetForward();

	pragma::physics::TraceData data {};
	data.SetFilter(ent);
	data.SetFlags(pragma::physics::RayCastFlags::Default | pragma::physics::RayCastFlags::InvertFilter);
	data.SetSource(origin);
	data.SetTarget(origin + dir * 65'536.f);
	auto r = pragma::get_cgame()->RayCast(data);
	if(r.hitType == pragma::physics::RayCastHitType::None)
		return;
	static_cast<pragma::nav::CMesh &>(*navMesh).SetDebugPathStart(r.position);
}

void CMD_debug_nav_path_end(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &)
{
	if(!check_cheats("debug_nav_path_end", state))
		return;
	if(pragma::get_cgame() == nullptr || pragma::get_cgame()->LoadNavMesh() == false || pl == nullptr)
		return;
	auto &navMesh = pragma::get_cgame()->GetNavMesh();
	if(navMesh == nullptr)
		return;
	auto &ent = pl->GetEntity();
	auto charComponent = ent.GetCharacterComponent();
	auto pTrComponent = ent.GetTransformComponent();
	if(charComponent.expired() && pTrComponent == nullptr)
		return;
	auto origin = charComponent.valid() ? charComponent->GetEyePosition() : pTrComponent->GetPosition();
	auto dir = charComponent.valid() ? charComponent->GetViewForward() : pTrComponent->GetForward();

	pragma::physics::TraceData data {};
	data.SetFilter(ent);
	data.SetFlags(pragma::physics::RayCastFlags::Default | pragma::physics::RayCastFlags::InvertFilter);
	data.SetSource(origin);
	data.SetTarget(origin + dir * 65'536.f);
	auto r = pragma::get_cgame()->RayCast(data);
	if(r.hitType == pragma::physics::RayCastHitType::None)
		return;
	static_cast<pragma::nav::CMesh &>(*navMesh).SetDebugPathEnd(r.position);
}
namespace {
	auto UVN = pragma::console::client::register_command("debug_nav_path_start", &CMD_debug_nav_path_start, pragma::console::ConVarFlags::None, "Sets a start point for a navigation path. Use debug_nav_path_end to set the end point.");
	auto UVN = pragma::console::client::register_command("debug_nav_path_end", &CMD_debug_nav_path_end, pragma::console::ConVarFlags::None,
	  "Sets an end point for a navigation path. Use debug_nav_path_start to set the start point. The path will be drawn in the scene once both points have been set.");
}
