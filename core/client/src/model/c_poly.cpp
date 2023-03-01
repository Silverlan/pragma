/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/model/c_poly.h"

extern ClientState *client;
CPoly::CPoly(NetworkState *nw) : Poly(nw) {}

void CPoly::SortVertices()
{
	if(!IsValid())
		return;
	Poly::SortVertices();
	CalculateTextureAxes();
}

void CPoly::SetTextureData(std::string texture, Vector3 nu, Vector3 nv, float ou, float ov, float su, float sv, float rot)
{
	Poly::SetTextureData(texture, nu, nv, ou, ov, su, sv, rot);
	SetMaterial(client->LoadMaterial(texture.c_str()));
}

void CPoly::CalculateTextureAxes()
{
	auto &texData = m_texData;
	if(texData == NULL)
		return;

	Material *mat = GetMaterial();
	TextureInfo *tex = NULL;
	if(mat != NULL)
		tex = mat->GetTextureInfo("diffusemap");
	int w, h;
	if(tex != NULL) {
		w = tex->width;
		h = tex->height;
	}
	else {
		w = 512;
		h = 512;
	}
	for(int i = 0; i < m_vertices.size(); i++) {
		auto &v = m_vertices[i].pos;
		if(m_centerLocalized.has_value())
			uvec::add(&v, *m_centerLocalized);
		float du = glm::dot(v, texData->nu);
		float tu = (du / w) / texData->su + texData->ou / w;
		float dv = glm::dot(v, texData->nv);
		float tv = (dv / h) / texData->sv + texData->ov / h;
		m_vertices[i].u = tu;
		m_vertices[i].v = tv;
	}
}
