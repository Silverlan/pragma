// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_POLY_H__
#define __C_POLY_H__
#include "pragma/clientdefinitions.h"
#include "pragma/model/poly.h"
#include "cmaterialmanager.h"

class DLLCLIENT CPoly : public Poly {
  private:
	void CalculateTextureAxes();
  public:
	CPoly(NetworkState *nw);
	virtual void SortVertices() override;
	virtual void SetTextureData(std::string texture, Vector3 nu, Vector3 nv, float ou, float ov, float su, float sv, float rot = 0) override;
};
#endif
