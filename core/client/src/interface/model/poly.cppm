// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:model.poly;

export import pragma.shared;

export class DLLCLIENT CPoly : public Poly {
  private:
	void CalculateTextureAxes();
  public:
	CPoly(pragma::NetworkState *nw);
	virtual void SortVertices() override;
	virtual void SetTextureData(std::string texture, Vector3 nu, Vector3 nv, float ou, float ov, float su, float sv, float rot = 0) override;
};
