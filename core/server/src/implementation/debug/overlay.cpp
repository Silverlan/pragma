// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/networking/enums.hpp>
#include <pragma/math/angle/wvangle.h>
#include <pragma/networking/nwm_util.h>
#include <mathutil/plane.hpp>

module pragma.server.debug.overlay;

import pragma.server.server_state;

extern ServerState *server;

void SDebugRenderer::DrawPoint(const Vector3 &pos, const Color &color, float duration)
{
	NetPacket p;
	p << pos << color << duration;
	server->SendPacket("debug_drawpoint", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration)
{
	NetPacket p;
	p << start << end << color << duration;
	server->SendPacket("debug_drawline", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawBox(const Vector3 &start, const Vector3 &end, const Color &color, float duration) { DrawBox(start, end, EulerAngles(0.f, 0.f, 0.f), color, duration); }
void SDebugRenderer::DrawBox(const Vector3 &center, const Vector3 &min, const Vector3 &max, const Color &color, float duration) { DrawBox(center, min, max, EulerAngles(0.f, 0.f, 0.f), color, duration); }
void SDebugRenderer::DrawBox(const Vector3 &center, const Vector3 &min, const Vector3 &max, const EulerAngles &ang, const Color &color, const Color &colorOutline, float duration)
{
	NetPacket p;
	p << center << min << max << ang << color << true << colorOutline << duration;
	server->SendPacket("debug_drawbox", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawBox(const Vector3 &center, const Vector3 &min, const Vector3 &max, const EulerAngles &ang, const Color &color, float duration)
{
	NetPacket p;
	p << center << min << max << ang << color << false << duration;
	server->SendPacket("debug_drawbox", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawBox(const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &color, const Color &colorOutline, float duration)
{
	auto center = (start + end) * 0.5f;
	DrawBox(center, start - center, end - center, ang, color, colorOutline, duration);
}
void SDebugRenderer::DrawBox(const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &color, float duration)
{
	auto center = (start + end) * 0.5f;
	DrawBox(center, start - center, end - center, ang, color, duration);
}
void SDebugRenderer::DrawText(const std::string &text, const Vector3 &pos, const Vector2 &worldSize, const Color &color, float duration)
{
	NetPacket p;
	p->WriteString(text);
	p << pos << true << worldSize << true << color << duration;
	server->SendPacket("debug_drawtext", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawText(const std::string &text, const Vector3 &pos, float sizeScale, const Color &color, float duration)
{
	NetPacket p;
	p->WriteString(text);
	p << pos << false << sizeScale << true << color << duration;
	server->SendPacket("debug_drawtext", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawText(const std::string &text, const Vector3 &pos, const Vector2 &worldSize, float duration)
{
	NetPacket p;
	p->WriteString(text);
	p << pos << true << worldSize << false << duration;
	server->SendPacket("debug_drawtext", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawText(const std::string &text, const Vector3 &pos, float sizeScale, float duration)
{
	NetPacket p;
	p->WriteString(text);
	p << pos << false << sizeScale << false << duration;
	server->SendPacket("debug_drawtext", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawSphere(const Vector3 &origin, float radius, const Color &color, const Color &outlineColor, float duration, uint32_t recursionLevel)
{
	NetPacket p;
	p << origin << radius << color << duration << recursionLevel << true << outlineColor;
	server->SendPacket("debug_drawsphere", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawSphere(const Vector3 &origin, float radius, const Color &color, float duration, uint32_t recursionLevel)
{
	NetPacket p;
	p << origin << radius << color << duration << recursionLevel << false;
	server->SendPacket("debug_drawsphere", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawTruncatedCone(const Vector3 &origin, float startRadius, const Vector3 &dir, float dist, float endRadius, const Color &color, const Color &outlineColor, float duration, uint32_t segmentCount)
{
	NetPacket p;
	p << origin << startRadius << dir << dist << endRadius << color << duration << segmentCount << true << outlineColor;
	server->SendPacket("debug_drawtruncatedcone", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawTruncatedCone(const Vector3 &origin, float startRadius, const Vector3 &dir, float dist, float endRadius, const Color &color, float duration, uint32_t segmentCount)
{
	NetPacket p;
	p << origin << startRadius << dir << dist << endRadius << color << duration << segmentCount << false;
	server->SendPacket("debug_drawtruncatedcone", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawCylinder(const Vector3 &origin, const Vector3 &dir, float dist, float radius, const Color &color, float duration, uint32_t segmentCount)
{
	NetPacket p;
	p << origin << dir << dist << radius << color << duration << segmentCount << false;
	server->SendPacket("debug_drawcylinder", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawCylinder(const Vector3 &origin, const Vector3 &dir, float dist, float radius, const Color &color, const Color &outlineColor, float duration, uint32_t segmentCount)
{
	NetPacket p;
	p << origin << dir << dist << radius << color << duration << segmentCount << true << outlineColor;
	server->SendPacket("debug_drawcylinder", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawCone(const Vector3 &origin, const Vector3 &dir, float dist, float angle, const Color &color, const Color &outlineColor, float duration, uint32_t segmentCount)
{
	NetPacket p;
	p << origin << dir << dist << angle << color << duration << segmentCount << true << outlineColor;
	server->SendPacket("debug_drawcone", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawCone(const Vector3 &origin, const Vector3 &dir, float dist, float angle, const Color &color, float duration, uint32_t segmentCount)
{
	NetPacket p;
	p << origin << dir << dist << angle << color << duration << segmentCount << false;
	server->SendPacket("debug_drawcone", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawAxis(const Vector3 &origin, const EulerAngles &ang, float duration)
{
	NetPacket p;
	p << origin << ang << duration;
	server->SendPacket("debug_drawaxis", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawAxis(const Vector3 &origin, float duration) { DrawAxis(origin, EulerAngles(0.f, 0.f, 0.f), duration); }
void SDebugRenderer::DrawPath(const std::vector<Vector3> &path, const Color &color, float duration)
{
	NetPacket p;
	p->Write<uint32_t>(static_cast<uint32_t>(path.size()));
	for(auto &v : path)
		p->Write<Vector3>(v);
	p << color << duration;
	server->SendPacket("debug_drawpath", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawSpline(const std::vector<Vector3> &path, const Color &color, uint32_t segmentCount, float curvature, float duration)
{
	NetPacket p;
	p->Write<uint32_t>(static_cast<uint32_t>(path.size()));
	for(auto &v : path)
		p->Write<Vector3>(v);
	p << color << segmentCount << curvature << duration;
	server->SendPacket("debug_drawspline", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawPlane(const Vector3 &n, float dist, const Color &color, float duration)
{
	NetPacket p;
	p->Write<Vector3>(n);
	p->Write<float>(dist);
	p->Write<Color>(color);
	p->Write<float>(duration);
	server->SendPacket("debug_drawplane", p, pragma::networking::Protocol::FastUnreliable);
}
void SDebugRenderer::DrawPlane(const umath::Plane &plane, const Color &color, float duration) { DrawPlane(const_cast<umath::Plane &>(plane).GetNormal(), static_cast<float>(plane.GetDistance()), color, duration); }
void SDebugRenderer::DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration)
{
	NetPacket p;
	p->Write<uint32_t>(meshVerts.size() / 3);
	p->Write(reinterpret_cast<const uint8_t *>(meshVerts.data()), util::size_of_container(meshVerts));
	p->Write<Color>(color);
	p->Write<Color>(colorOutline);
	p->Write<float>(duration);
	server->SendPacket("debug_draw_mesh", p, pragma::networking::Protocol::FastUnreliable);
}
