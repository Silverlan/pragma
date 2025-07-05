// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __SURFACE_TYPE_MANAGER_HPP__
#define __SURFACE_TYPE_MANAGER_HPP__

#include "pragma/util/util_typed_manager.hpp"

namespace pragma::physics {
	class DLLNETWORK SurfaceType : public BaseNamedType {
	  protected:
		template<class TType>
		friend class pragma::TTypeManager;
		SurfaceType(TypeId id, const std::string &name);
	};

	using SurfaceTypeManager = TTypeManager<SurfaceType>;
};

#endif
