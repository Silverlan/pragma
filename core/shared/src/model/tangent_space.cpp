/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/model/modelmesh.h"
#include "model/mikktspace/mikktspace.h"
#include <pragma/console/conout.h>

void umath::compute_tangent_basis(std::vector<Vertex> &verts,const std::vector<uint16_t> &triangles)
{
	struct MeshData
	{
		std::vector<Vertex> &verts;
		const std::vector<uint16_t> &triangles;
	};
	MeshData meshData {verts,triangles};
	SMikkTSpaceInterface iMikk {};
	iMikk.m_getNumFaces = [](const SMikkTSpaceContext *pContext) -> int {return static_cast<MeshData*>(pContext->m_pUserData)->triangles.size() /3;};
	iMikk.m_getNumVerticesOfFace = [](const SMikkTSpaceContext *pContext,const int iFace) -> int {return 3;};
	iMikk.m_getPosition = [](const SMikkTSpaceContext *pContext,float fvPosOut[],const int iFace,const int iVert) {
		auto &meshData = *static_cast<MeshData*>(pContext->m_pUserData);
		auto idx = meshData.triangles.at(iFace *3 +iVert);
		auto &v = meshData.verts.at(idx);
		for(uint8_t i=0;i<3;++i)
			fvPosOut[i] = v.position[i];
	};
	iMikk.m_getNormal = [](const SMikkTSpaceContext *pContext,float fvNormOut[],const int iFace,const int iVert)
	{
		auto &meshData = *static_cast<MeshData*>(pContext->m_pUserData);
		auto idx = meshData.triangles.at(iFace *3 +iVert);
		auto &v = meshData.verts.at(idx);
		for(uint8_t i=0;i<3;++i)
			fvNormOut[i] = v.normal[i];
	};
	iMikk.m_getTexCoord = [](const SMikkTSpaceContext *pContext,float fvTexcOut[],const int iFace,const int iVert)
	{
		auto &meshData = *static_cast<MeshData*>(pContext->m_pUserData);
		auto idx = meshData.triangles.at(iFace *3 +iVert);
		auto &v = meshData.verts.at(idx);
		for(uint8_t i=0;i<2;++i)
			fvTexcOut[i] = v.uv[i];
	};
	iMikk.m_setTSpaceBasic = [](const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
	{
		auto &meshData = *static_cast<MeshData*>(pContext->m_pUserData);
		auto idx = meshData.triangles.at(iFace *3 +iVert);
		auto &v = meshData.verts.at(idx);
		v.tangent = {fvTangent[0],fvTangent[1],fvTangent[2],fSign};
	};

	SMikkTSpaceContext context {};
	context.m_pInterface = &iMikk;
	context.m_pUserData = &meshData;
	auto result = genTangSpaceDefault(&context);
	if(result == false)
		Con::cwar<<"WARNING: Unable to calculate mesh tangents!"<<Con::endl;
}
