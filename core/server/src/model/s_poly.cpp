// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include <pragma/serverstate/serverstate.h>
#include "pragma/model/s_poly.h"

extern ServerState *server;
SPoly::SPoly(NetworkState *nw) : Poly(nw) {}
void SPoly::SetTextureData(std::string texture, Vector3 nu, Vector3 nv, float ou, float ov, float su, float sv, float rot)
{
	Poly::SetTextureData(texture, nu, nv, ou, ov, su, sv, rot);
	SetMaterial(server->LoadMaterial(texture.c_str()));
}
