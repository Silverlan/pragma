// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/model/c_poly.h"

import pragma.client.client_state;

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
