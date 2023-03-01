/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_POLY_H__
#define __S_POLY_H__
#include "pragma/serverdefinitions.h"
#include "pragma/model/poly.h"

class DLLSERVER SPoly : public Poly {
  public:
	SPoly(NetworkState *nw);
	virtual void SetTextureData(std::string texture, Vector3 nu, Vector3 nv, float ou, float ov, float su, float sv, float rot = 0) override;
};
#endif
