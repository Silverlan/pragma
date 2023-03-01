/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/model/modelmesh.h"
#include "model/mikktspace/mikktspace.h"
#include <pragma/console/conout.h>

template<typename TIndex>
void umath::compute_tangent_basis(std::vector<Vertex> &verts, const TIndex *indices, uint32_t numIndices)
{
	if(numIndices == 0)
		return;
	struct MeshData {
		std::vector<Vertex> &verts;
		const TIndex *indices;
		uint32_t numIndices;
	};
	MeshData meshData {verts, indices, numIndices};
	SMikkTSpaceInterface iMikk {};
	iMikk.m_getNumFaces = [](const SMikkTSpaceContext *pContext) -> int { return static_cast<MeshData *>(pContext->m_pUserData)->numIndices / 3; };
	iMikk.m_getNumVerticesOfFace = [](const SMikkTSpaceContext *pContext, const int iFace) -> int { return 3; };
	iMikk.m_getPosition = [](const SMikkTSpaceContext *pContext, float fvPosOut[], const int iFace, const int iVert) {
		auto &meshData = *static_cast<MeshData *>(pContext->m_pUserData);
		auto idx = meshData.indices[iFace * 3 + iVert];
		auto &v = meshData.verts.at(idx);
		for(uint8_t i = 0; i < 3; ++i)
			fvPosOut[i] = v.position[i];
	};
	iMikk.m_getNormal = [](const SMikkTSpaceContext *pContext, float fvNormOut[], const int iFace, const int iVert) {
		auto &meshData = *static_cast<MeshData *>(pContext->m_pUserData);
		auto idx = meshData.indices[iFace * 3 + iVert];
		auto &v = meshData.verts.at(idx);
		for(uint8_t i = 0; i < 3; ++i)
			fvNormOut[i] = v.normal[i];
	};
	iMikk.m_getTexCoord = [](const SMikkTSpaceContext *pContext, float fvTexcOut[], const int iFace, const int iVert) {
		auto &meshData = *static_cast<MeshData *>(pContext->m_pUserData);
		auto idx = meshData.indices[iFace * 3 + iVert];
		auto &v = meshData.verts.at(idx);
		for(uint8_t i = 0; i < 2; ++i)
			fvTexcOut[i] = v.uv[i];
	};
	iMikk.m_setTSpaceBasic = [](const SMikkTSpaceContext *pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert) {
		auto &meshData = *static_cast<MeshData *>(pContext->m_pUserData);
		auto idx = meshData.indices[iFace * 3 + iVert];
		auto &v = meshData.verts.at(idx);
		v.tangent = {fvTangent[0], fvTangent[1], fvTangent[2], fSign};
	};

	SMikkTSpaceContext context {};
	context.m_pInterface = &iMikk;
	context.m_pUserData = &meshData;
	auto result = genTangSpaceDefault(&context);
	if(result == false)
		Con::cwar << "Unable to calculate mesh tangents!" << Con::endl;
}
template DLLNETWORK void umath::compute_tangent_basis<uint16_t>(std::vector<Vertex> &verts, const uint16_t *indices, uint32_t numIndices);
template DLLNETWORK void umath::compute_tangent_basis<uint32_t>(std::vector<Vertex> &verts, const uint32_t *indices, uint32_t numIndices);
