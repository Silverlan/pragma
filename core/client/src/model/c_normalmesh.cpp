/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/model/c_normalmesh.h"
#include <pragma/math/vector/wvvector3.h>

extern DLLCENGINE CEngine *c_engine;

NormalMesh::NormalMesh()
{}

void NormalMesh::ComputeTangentBasis(std::vector<Vertex> &verts,std::vector<uint16_t> &triangles)
{
	auto numVerts = verts.size();
	for(unsigned int i=0;i<triangles.size();i+=3)
	{
		if(triangles[i] >= numVerts || triangles[i +1] >= numVerts || triangles[i +2] >= numVerts)
		{
			Con::cwar<<"WARNING: Triangle vertices ("<<triangles[i]<<","<<triangles[i +1]<<","<<triangles[i +2]<<") out of bounds for mesh "<<this<<" ("<<numVerts<<" vertices)!"<<Con::endl;
			return;
		}
		auto &v0 = verts[triangles[i]];
		auto &v1 = verts[triangles[i +1]];
		auto &v2 = verts[triangles[i +2]];

		auto deltaPos1 = v1.position -v0.position;
		auto deltaPos2 = v2.position -v0.position;

		auto deltaUV1 = v1.uv -v0.uv;
		auto deltaUV2 = v2.uv -v0.uv;
		//auto deltaUV1 = Vector2(v1.uv.x,1.f -v1.uv.y) -Vector2(v0.uv.x,1.f -v0.uv.y);
		//auto deltaUV2 = Vector2(v2.uv.x,1.f -v2.uv.y) -Vector2(v0.uv.x,1.f -v0.uv.y);
		//auto deltaUV1 = Vector2(1.f -v1.uv.x,1.f -v1.uv.y) -Vector2(1.f -v0.uv.x,1.f -v0.uv.y);
		//auto deltaUV2 = Vector2(1.f -v2.uv.x,1.f -v2.uv.y) -Vector2(1.f -v0.uv.x,1.f -v0.uv.y);

		auto d = deltaUV1.x *deltaUV2.y -deltaUV1.y *deltaUV2.x;
		auto r = (d != 0.f) ? (1.f /d) : 0.f;
		auto tangent = (deltaPos1 *deltaUV2.y -deltaPos2 *deltaUV1.y) *r;
		auto biTangent = (deltaPos2 *deltaUV1.x -deltaPos1 *deltaUV2.x) *r;
		uvec::normalize(&biTangent);

		v0.tangent = tangent;
		v1.tangent = tangent;
		v2.tangent = tangent;

		v0.biTangent = biTangent;
		v1.biTangent = biTangent;
		v2.biTangent = biTangent;
	}
	for(unsigned int i=0;i<triangles.size();i++)
	{
		auto &v = verts[triangles[i]];
		auto &t = v.tangent;//m_tangents[triangles[i]];
		auto &b = v.biTangent;//m_biTangents[triangles[i]];

		t = t -v.normal *glm::dot(v.normal,t);
		uvec::normalize(&t);

		if(glm::dot(glm::cross(v.normal,t),b) < 0.f)
			t *= -1.f;
	}
	//auto &context = c_engine->GetRenderContext();
	//m_bufTangent = Vulkan::Buffer::Create(context,m_tangents.size() *sizeof(Vector3),m_tangents.data());
	//m_bufBiTangent = Vulkan::Buffer::Create(context,m_biTangents.size() *sizeof(Vector3),m_biTangents.data());
}
