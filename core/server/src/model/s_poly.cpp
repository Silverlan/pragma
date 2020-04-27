/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include <pragma/serverstate/serverstate.h>
#include "pragma/model/s_poly.h"

extern ServerState *server;
SPoly::SPoly(NetworkState *nw)
	: Poly(nw)
{}
void SPoly::SetTextureData(std::string texture,Vector3 nu,Vector3 nv,float ou,float ov,float su,float sv,float rot)
{
	Poly::SetTextureData(texture,nu,nv,ou,ov,su,sv,rot);
	SetMaterial(server->LoadMaterial(texture.c_str()));
}
