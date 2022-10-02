/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SURFACE_TYPE_MANAGER_HPP__
#define __SURFACE_TYPE_MANAGER_HPP__

#include "pragma/util/util_typed_manager.hpp"

namespace pragma::physics
{
	class DLLNETWORK SurfaceType
		: public BaseNamedType
	{
	protected:
		template<class TType>
            friend class pragma::TTypeManager;
		SurfaceType(TypeId id,const std::string &name);
	};

	using SurfaceTypeManager = TTypeManager<SurfaceType>;
};

#endif
