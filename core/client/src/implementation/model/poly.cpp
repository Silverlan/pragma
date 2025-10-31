// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client;

import :model.poly;

import :client_state;

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
	SetMaterial(pragma::get_client_state()->LoadMaterial(texture.c_str()));
}

void CPoly::CalculateTextureAxes()
{
	auto &texData = m_texData;
	if(texData == nullptr)
		return;

	msys::Material *mat = GetMaterial();
	TextureInfo *tex = nullptr;
	if(mat != nullptr)
		tex = mat->GetTextureInfo("diffusemap");
	int w, h;
	if(tex != nullptr) {
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
