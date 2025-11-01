// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:model.poly_mesh;

export import :model.poly;

export {
	class DLLCLIENT CPolyMesh : public PolyMesh {
		friend Con::c_cout &operator<<(Con::c_cout &, const CPolyMesh &);
	  private:
		std::vector<CPoly *> m_polys;
	};

	inline Con::c_cout &operator<<(Con::c_cout &os, const CPolyMesh &mesh)
	{
		os << static_cast<PolyMesh>(mesh);
		return os;
	}
};
