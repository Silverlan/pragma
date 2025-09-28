// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"

import pragma.client;

void NET_cl_debug_drawpoint(NetPacket packet)
{
	auto pos = packet->Read<Vector3>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(pos);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	DebugRenderer::DrawPoint(renderInfo);
}
void NET_cl_debug_drawline(NetPacket packet)
{
	auto start = packet->Read<Vector3>();
	auto end = packet->Read<Vector3>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	DebugRenderer::DrawLine(start, end, renderInfo);
}
void NET_cl_debug_drawbox(NetPacket packet)
{
	auto center = packet->Read<Vector3>();
	auto min = packet->Read<Vector3>();
	auto max = packet->Read<Vector3>();
	auto ang = packet->Read<EulerAngles>();
	auto col = packet->Read<Color>();
	auto bOutlineColor = packet->Read<bool>();
	Color colOutline = {};
	auto dur = packet->Read<float>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(center);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	renderInfo.SetRotation(uquat::create(ang));
	if(bOutlineColor == true) {
		colOutline = packet->Read<Color>();
		renderInfo.SetOutlineColor(colOutline);
	}
	DebugRenderer::DrawBox(min, max, renderInfo);
}
void NET_cl_debug_drawtext(NetPacket packet)
{
	auto text = packet->ReadString();
	auto pos = packet->Read<Vector3>();
	auto bSize = packet->Read<bool>();
	Vector2 size {};
	auto scale = 1.f;
	if(bSize == true)
		size = packet->Read<Vector2>();
	else
		scale = packet->Read<float>();
	auto bColor = packet->Read<bool>();
	Color col {};
	if(bColor == true)
		col = packet->Read<Color>();
	auto duration = packet->Read<float>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(pos);
	if(bColor == true)
		renderInfo.SetColor(col);
	renderInfo.SetDuration(duration);
	if(bSize == true)
		DebugRenderer::DrawText(renderInfo, text, size);
	else
		DebugRenderer::DrawText(renderInfo, text, scale);
}
void NET_cl_debug_drawsphere(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto radius = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	auto recursionLevel = packet->Read<uint32_t>();
	auto bOutline = packet->Read<bool>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	if(bOutline) {
		auto colOutline = packet->Read<Color>();
		renderInfo.SetOutlineColor(colOutline);
	}
	DebugRenderer::DrawSphere(renderInfo, radius, recursionLevel);
}
void NET_cl_debug_drawcone(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto dir = packet->Read<Vector3>();
	auto dist = packet->Read<float>();
	auto angle = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto duration = packet->Read<float>();
	auto segmentCount = packet->Read<uint32_t>();
	auto bOutline = packet->Read<bool>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(duration);
	if(bOutline) {
		auto colOutline = packet->Read<Color>();
		renderInfo.SetOutlineColor(colOutline);
	}
	DebugRenderer::DrawCone(renderInfo, dir, dist, angle, segmentCount);
}
void NET_cl_debug_drawaxis(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto ang = packet->Read<EulerAngles>();
	auto dur = packet->Read<float>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetRotation(uquat::create(ang));
	renderInfo.SetDuration(dur);
	DebugRenderer::DrawAxis(renderInfo);
}
void NET_cl_debug_drawpath(NetPacket packet)
{
	auto numPath = packet->Read<uint32_t>();
	std::vector<Vector3> path;
	path.reserve(numPath);
	for(auto i = decltype(numPath) {0}; i < numPath; ++i)
		path.push_back(packet->Read<Vector3>());
	auto col = packet->Read<Color>();
	auto duration = packet->Read<float>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetColor(col);
	renderInfo.SetDuration(duration);
	DebugRenderer::DrawPath(path, renderInfo);
}
void NET_cl_debug_drawspline(NetPacket packet)
{
	auto numPath = packet->Read<uint32_t>();
	std::vector<Vector3> path;
	path.reserve(numPath);
	for(auto i = decltype(numPath) {0}; i < numPath; ++i)
		path.push_back(packet->Read<Vector3>());
	auto col = packet->Read<Color>();
	auto numSegments = packet->Read<uint32_t>();
	auto curvature = packet->Read<float>();
	auto duration = packet->Read<float>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetColor(col);
	renderInfo.SetDuration(duration);
	DebugRenderer::DrawSpline(path, numSegments, curvature, renderInfo);
}
void NET_cl_debug_drawplane(NetPacket packet)
{
	auto n = packet->Read<Vector3>();
	auto d = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	DebugRenderer::DrawPlane(n, d, renderInfo);
}
void NET_cl_debug_draw_mesh(NetPacket packet)
{
	auto numTris = packet->Read<uint32_t>();
	std::vector<Vector3> verts;
	verts.resize(numTris * 3);
	packet->Read(verts.data(), util::size_of_container(verts));
	auto color = packet->Read<Color>();
	auto colorOutline = packet->Read<Color>();
	auto duration = packet->Read<float>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetColor(color);
	renderInfo.SetOutlineColor(colorOutline);
	renderInfo.SetDuration(duration);
	DebugRenderer::DrawMesh(verts, renderInfo);
}
void NET_cl_debug_drawtruncatedcone(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto startRadius = packet->Read<float>();
	auto dir = packet->Read<Vector3>();
	auto dist = packet->Read<float>();
	auto endRadius = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	auto segmentCount = packet->Read<uint32_t>();
	auto bOutline = packet->Read<bool>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	if(bOutline) {
		auto colOutline = packet->Read<Color>();
		renderInfo.SetOutlineColor(colOutline);
	}
	DebugRenderer::DrawTruncatedCone(renderInfo, startRadius, dir, dist, endRadius, segmentCount);
}
void NET_cl_debug_drawcylinder(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto dir = packet->Read<Vector3>();
	auto dist = packet->Read<float>();
	auto radius = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	auto segmentCount = packet->Read<uint32_t>();
	auto bOutline = packet->Read<bool>();
	DebugRenderInfo renderInfo {};
	renderInfo.SetOrigin(origin);
	renderInfo.SetColor(col);
	renderInfo.SetDuration(dur);
	if(bOutline) {
		auto colOutline = packet->Read<Color>();
		renderInfo.SetOutlineColor(colOutline);
	}
	DebugRenderer::DrawCylinder(renderInfo, dir, dist, radius, segmentCount);
}
