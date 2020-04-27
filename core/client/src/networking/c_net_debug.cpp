/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/networking/c_net_debug.h"
#include "pragma/debug/c_debugoverlay.h"
#include <pragma/networking/nwm_util.h>

void NET_cl_debug_drawpoint(NetPacket packet)
{
	auto pos = packet->Read<Vector3>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	DebugRenderer::DrawPoint(pos,col,dur);
}
void NET_cl_debug_drawline(NetPacket packet)
{
	auto start = packet->Read<Vector3>();
	auto end = packet->Read<Vector3>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	DebugRenderer::DrawLine(start,end,col,dur);
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
	if(bOutlineColor == true)
		colOutline = packet->Read<Color>();
	auto dur = packet->Read<float>();
	if(bOutlineColor == false)
		DebugRenderer::DrawBox(center,min,max,ang,col,dur);
	else
		DebugRenderer::DrawBox(center,min,max,ang,col,colOutline,dur);
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
	if(bSize == true)
	{
		if(bColor == true)
			DebugRenderer::DrawText(text,pos,size,col,duration);
		else
			DebugRenderer::DrawText(text,pos,size,duration);
	}
	else
	{
		if(bColor == true)
			DebugRenderer::DrawText(text,pos,scale,col,duration);
		else
			DebugRenderer::DrawText(text,pos,scale,duration);
	}
}
void NET_cl_debug_drawsphere(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto radius = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	auto recursionLevel = packet->Read<uint32_t>();
	auto bOutline = packet->Read<bool>();
	if(bOutline == false)
		DebugRenderer::DrawSphere(origin,radius,col,dur,recursionLevel);
	else
	{
		auto colOutline = packet->Read<Color>();
		DebugRenderer::DrawSphere(origin,radius,col,colOutline,dur,recursionLevel);
	}
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
	if(bOutline == false)
		DebugRenderer::DrawCone(origin,dir,dist,angle,col,duration,segmentCount);
	else
	{
		auto colOutline = packet->Read<Color>();
		DebugRenderer::DrawCone(origin,dir,dist,angle,col,colOutline,duration,segmentCount);
	}
}
void NET_cl_debug_drawaxis(NetPacket packet)
{
	auto origin = packet->Read<Vector3>();
	auto ang = packet->Read<EulerAngles>();
	auto dur = packet->Read<float>();
	DebugRenderer::DrawAxis(origin,ang,dur);
}
void NET_cl_debug_drawpath(NetPacket packet)
{
	auto numPath = packet->Read<uint32_t>();
	std::vector<Vector3> path;
	path.reserve(numPath);
	for(auto i=decltype(numPath){0};i<numPath;++i)
		path.push_back(packet->Read<Vector3>());
	auto col = packet->Read<Color>();
	auto duration = packet->Read<float>();
	DebugRenderer::DrawPath(path,col,duration);
}
void NET_cl_debug_drawspline(NetPacket packet)
{
	auto numPath = packet->Read<uint32_t>();
	std::vector<Vector3> path;
	path.reserve(numPath);
	for(auto i=decltype(numPath){0};i<numPath;++i)
		path.push_back(packet->Read<Vector3>());
	auto col = packet->Read<Color>();
	auto numSegments = packet->Read<uint32_t>();
	auto curvature = packet->Read<float>();
	auto duration = packet->Read<float>();
	DebugRenderer::DrawSpline(path,col,numSegments,curvature,duration);
}
void NET_cl_debug_drawplane(NetPacket packet)
{
	auto n = packet->Read<Vector3>();
	auto d = packet->Read<float>();
	auto col = packet->Read<Color>();
	auto dur = packet->Read<float>();
	DebugRenderer::DrawPlane(n,d,col,dur);
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
	if(bOutline == false)
		DebugRenderer::DrawTruncatedCone(origin,startRadius,dir,dist,endRadius,col,dur,segmentCount);
	else
	{
		auto outlineColor = packet->Read<Color>();
		DebugRenderer::DrawTruncatedCone(origin,startRadius,dir,dist,endRadius,col,outlineColor,dur,segmentCount);
	}
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
	if(bOutline == false)
		DebugRenderer::DrawCylinder(origin,dir,dist,radius,col,dur,segmentCount);
	else
	{
		auto outlineColor = packet->Read<Color>();
		DebugRenderer::DrawCylinder(origin,dir,dist,radius,col,outlineColor,dur,segmentCount);
	}
}
